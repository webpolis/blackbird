#include "kraken.h"
#include "parameters.h"
#include "curl_fun.h"
#include "utils/restapi.h"
#include "utils/base64.h"

#include "openssl/sha.h"
#include "openssl/hmac.h"
#include "jansson.h"
#include <sstream>
#include <iomanip>
#include <vector>
#include <unistd.h>
#include <sys/time.h>

namespace Kraken {

// Initialise internal variables
static json_t *krakenTicker = nullptr;
static bool krakenGotTicker = false;
static json_t *krakenLimPrice = nullptr;
static bool krakenGotLimPrice = false;

static RestApi& queryHandle(Parameters &params)
{
  static RestApi query ("https://api.kraken.com",
                        params.cacert.c_str(), *params.logFile);
  return query;
}

quote_t getQuote(Parameters &params)
{
  json_t *root;
  if (krakenGotTicker) {
    root = krakenTicker;
    krakenGotTicker = false;
  } else {
    auto &exchange = queryHandle(params);
    root = exchange.getRequest("/0/public/Ticker?pair=XXBTZUSD");
    krakenGotTicker = true;
    krakenTicker = root;
  }
  const char *quote = json_string_value(json_array_get(json_object_get(json_object_get(json_object_get(root, "result"), "XXBTZUSD"), "b"), 0));
  auto bidValue = quote ? std::stod(quote) : 0.0;

  quote = json_string_value(json_array_get(json_object_get(json_object_get(json_object_get(root, "result"), "XXBTZUSD"), "a"), 0));
  auto askValue = quote ? std::stod(quote) : 0.0;

  if (!krakenGotTicker) {
    json_decref(root);
  }
  return std::make_pair(bidValue, askValue);
}

double getAvail(Parameters& params, std::string currency) {
  json_t* root = authRequest(params, "https://api.kraken.com", "/0/private/Balance");
  json_t* result = json_object_get(root, "result");
  if (json_object_size(result) == 0) {
    return 0.0;
  }
  double available = 0.0;
  if (currency.compare("usd") == 0) {
    const char * avail_str = json_string_value(json_object_get(result, "ZUSD"));
    available = avail_str ? atof(avail_str) : 0.0;
  } else if (currency.compare("btc") == 0) {
    const char * avail_str = json_string_value(json_object_get(result, "XXBT"));
    available = avail_str ? atof(avail_str) : 0.0;
  } else {
    *params.logFile << "<Kraken> Currency not supported" << std::endl;
  }
  return available;
}

std::string sendLongOrder(Parameters& params, std::string direction, double quantity, double price) {
  if (direction.compare("buy") != 0 && direction.compare("sell") != 0) {
    *params.logFile  << "<Kraken> Error: Neither \"buy\" nor \"sell\" selected" << std::endl;
    return "0";
  }
  *params.logFile << "<Kraken> Trying to send a \"" << direction << "\" limit order: "
                  << std::setprecision(6) << quantity << " @ $"
                  << std::setprecision(2) << price << "...\n";
  std::string pair = "XXBTZUSD";
  std::string type = direction;
  std::string ordertype = "limit";
  std::string pricelimit = std::to_string(price);
  std::string volume = std::to_string(quantity);
  std::string options = "pair=" + pair + "&type=" + type + "&ordertype=" + ordertype + "&price=" + pricelimit + "&volume=" + volume;
  json_t* res = authRequest(params, "https://api.kraken.com", "/0/private/AddOrder", options);
  json_t* root = json_object_get(res, "result");
  if (json_is_object(root) == 0) {
    *params.logFile << json_dumps(res, 0) << std::endl;
    exit(0);
  }
  std::string txid = json_string_value(json_array_get(json_object_get(root, "txid"), 0));
  *params.logFile << "<Kraken> Done (transaction ID: " << txid << ")\n" << std::endl;
  json_decref(root);
  return txid;
}

bool isOrderComplete(Parameters& params, std::string orderId) {
  json_t* root = authRequest(params, "https://api.kraken.com", "/0/private/OpenOrders");
  // no open order: return true
  root = json_object_get(json_object_get(root, "result"), "open");
  if (json_object_size(root) == 0) {
    *params.logFile << "<Kraken> No order exists" << std::endl;
    return true;
  }
  *params.logFile << json_dumps(root, 0) << std::endl;
  root = json_object_get(root, orderId.c_str());
  // open orders exist but specific order not found: return true
  if (json_object_size(root) == 0) {
    *params.logFile << "<Kraken> Order " << orderId << " does not exist" << std::endl;
    return true;
  // open orders exist and specific order was found: return false
  } else {
    *params.logFile << "<Kraken> Order " << orderId << " still exists!" << std::endl;
    return false;
  }
}

double getActivePos(Parameters& params) {
  return getAvail(params, "btc");
}

double getLimitPrice(Parameters &params, double volume, bool isBid)
{
  if (!krakenGotLimPrice)
  {
    if (krakenLimPrice) json_decref(krakenLimPrice);
    auto &exchange = queryHandle(params);
    krakenLimPrice = exchange.getRequest("/0/public/Depth?pair=XXBTZUSD");
  }
  krakenGotLimPrice = !krakenGotLimPrice;
  auto root = krakenLimPrice;
  auto branch = json_object_get(json_object_get(root, "result"), "XXBTZUSD");
  branch = json_object_get(branch, isBid ? "bids" : "asks");

  // loop on volume
  double tmpVol = 0.0;
  int i = 0;
  // [[<price>, <volume>, <timestamp>], [<price>, <volume>, <timestamp>], ...]
  while (tmpVol < volume) {
    // volumes are added up until the requested volume is reached
    tmpVol += atof(json_string_value(json_array_get(json_array_get(branch, i), 1)));
    i++;
  }

  return atof(json_string_value(json_array_get(json_array_get(branch, i-1), 0)));
}

json_t* authRequest(Parameters& params, std::string url, std::string request, std::string options) {
  // create nonce and POST data
  struct timeval tv;
  gettimeofday(&tv, NULL);
  unsigned long long nonce = (tv.tv_sec * 1000.0) + (tv.tv_usec * 0.001) + 0.5;
  std::string post_data = "";
  if (options.empty()) {
    post_data = "nonce=" + std::to_string(nonce);
  } else {
    post_data = "nonce=" + std::to_string(nonce) + "&" + options;
  }
  // Message signature using HMAC-SHA512 of (URI path + SHA256(nonce + POST data))
  // and base64 decoded secret API key
  std::string payload_for_signature = std::to_string(nonce) + post_data;
  uint8_t digest[SHA256_DIGEST_LENGTH];
  SHA256((uint8_t *)payload_for_signature.c_str(), payload_for_signature.size(), &digest[0]);
  int signature_data_length = request.length() + SHA256_DIGEST_LENGTH;
  std::vector<unsigned char> signature_data(signature_data_length);
  auto signature_digest = std::copy(begin(request), end(request), begin(signature_data));
  std::copy(std::begin(digest), std::end(digest), signature_digest);
  std::string decoded_key = base64_decode(params.krakenSecret);
  unsigned char* hmac_digest;
  hmac_digest = HMAC(EVP_sha512(), decoded_key.c_str(), decoded_key.length(), signature_data.data(), signature_data.size(), NULL, NULL);
  std::string api_sign_header = base64_encode(reinterpret_cast<const unsigned char*>(hmac_digest), SHA512_DIGEST_LENGTH);
  // cURL header
  struct curl_slist* headers = NULL;
  std::string api = "API-KEY:" + std::string(params.krakenApi);
  std::string api_sig = "API-Sign:" + api_sign_header;
  headers = curl_slist_append(headers, api.c_str());
  headers = curl_slist_append(headers, api_sig.c_str());
  // cURL request
  CURLcode resCurl;
  if (params.curl) {
    std::string readBuffer;
    curl_easy_setopt(params.curl, CURLOPT_POST, 1L);
    curl_easy_setopt(params.curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(params.curl, CURLOPT_POSTFIELDS, post_data.c_str());
    curl_easy_setopt(params.curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(params.curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(params.curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(params.curl, CURLOPT_URL, (url+request).c_str());
    curl_easy_setopt(params.curl, CURLOPT_CONNECTTIMEOUT, 10L);
    resCurl = curl_easy_perform(params.curl);
    json_t* root;
    json_error_t error;
    while (resCurl != CURLE_OK) {
      *params.logFile << "<Kraken> Error with cURL. Retry in 2 sec...\n" << std::endl;
      sleep(2.0);
      readBuffer = "";
      resCurl = curl_easy_perform(params.curl);
    }
    root = json_loads(readBuffer.c_str(), 0, &error);
    while (!root) {
      *params.logFile << "<Kraken> Error with JSON:\n" << error.text << std::endl;
      *params.logFile << "<Kraken> Buffer:\n" << readBuffer.c_str() << std::endl;
      *params.logFile << "<Kraken> Retrying..." << std::endl;
      sleep(2.0);
      readBuffer = "";
      resCurl = curl_easy_perform(params.curl);
      while (resCurl != CURLE_OK) {
        *params.logFile << "<Kraken> Error with cURL. Retry in 2 sec...\n" << std::endl;
        sleep(2.0);
        readBuffer = "";
        resCurl = curl_easy_perform(params.curl);
      }
      root = json_loads(readBuffer.c_str(), 0, &error);
    }
    curl_slist_free_all(headers);
    curl_easy_reset(params.curl);
    return root;
  } else {
    *params.logFile << "<Kraken> Error with cURL init." << std::endl;
    return NULL;
  }
  return NULL;
}

}
