#include "quadrigacx.h"
#include "parameters.h"
#include "utils/restapi.h"
#include "utils/base64.h"
#include "unique_json.hpp"

#include "openssl/sha.h"
#include "openssl/hmac.h"
#include <iomanip>
#include <vector>
#include <array>
#include <time.h>
#include <chrono>

namespace QuadrigaCX {

// Initialise internal variables
static unique_json quadrigaTicker = nullptr;
static bool quadrigaGotTicker = false;
static unique_json quadrigaLimPrice = nullptr;
static bool quadrigaGotLimPrice = false;

static RestApi& queryHandle(Parameters &params)
{
  static RestApi query ("https://api.quadrigacx.com",
                        params.cacert.c_str(), *params.logFile);
  return query;
}

quote_t getQuote(Parameters &params)
{
//todo: tested works
  if (quadrigaGotTicker) {
    quadrigaGotTicker = false;
  } else {
    auto &exchange = queryHandle(params);
    quadrigaTicker.reset(exchange.getRequest("/v2/ticker?book=btc_usd"));
    quadrigaGotTicker = true;
  }
  json_t *root = quadrigaTicker.get();
  const char *quote = json_string_value(json_object_get(root, "bid"));
  auto bidValue = quote ? std::stod(quote) : 0.0;

  quote = json_string_value(json_object_get(root, "ask"));
  auto askValue = quote ? std::stod(quote) : 0.0;

  return std::make_pair(bidValue, askValue);
}


double getAvail(Parameters& params, std::string currency)
{
  //todo: tested works
  unique_json root { authRequest(params, "/v2/balance") };
  json_t *result = root.get();
  if (json_object_size(result) == 0) {
    return 0.0;
  }
  double available = 0.0;
  if (currency.compare("usd") == 0) {
    const char * avail_str = json_string_value(json_object_get(result, "usd_available"));
    available = avail_str ? atof(avail_str) : 0.0;
  } else if (currency.compare("btc") == 0) {
    const char * avail_str = json_string_value(json_object_get(result, "btc_available"));
    available = avail_str ? atof(avail_str) : 0.0;
  } else if (currency.compare("eth") == 0) {
    const char * avail_str = json_string_value(json_object_get(result, "eth_available"));
    available = avail_str ? atof(avail_str) : 0.0;
  } else if (currency.compare("cad") == 0) {
    const char * avail_str = json_string_value(json_object_get(result, "cad_available"));
    available = avail_str ? atof(avail_str) : 0.0;
  } else {
    *params.logFile << "<QuadrigaCX> Currency " << currency << " not supported" << std::endl;
  }
  return available;
}


std::string sendLongOrder(Parameters& params, std::string direction, double quantity, double price) {
  //todo: buy tested works 
  //todo: sell tested works 
  if (direction.compare("buy") != 0 && direction.compare("sell") != 0) {
    *params.logFile  << "<QuadrigaCX> Error: Neither \"buy\" nor \"sell\" selected" << std::endl;
    return "0";
  }
  *params.logFile << "<QuadrigaCX> Trying to send a \"" << direction << "\" limit order: "
                  << std::setprecision(6) << quantity << " @ $"
                  << std::setprecision(2) << price << "...\n";
  std::string pricelimit = std::to_string(price);
  std::string volume = std::to_string(quantity);

  unique_json options {json_object()};
  json_object_set_new(options.get(), "book", json_string("btc_usd"));
  json_object_set_new(options.get(), "amount", json_real(quantity));
  json_object_set_new(options.get(), "price", json_real(price));

  unique_json root { authRequest(params, ("/v2/" + direction), options.get()) };
  json_t *res = json_object_get(root.get(), "id");
  if (json_is_object(res) == 0) {
    auto dump = json_dumps(root.get(), 0) ;
    *params.logFile << "<QuadrigaCX> order failed: " << dump << std::endl;
    free(dump);
    return "Order Failed";
  }
  std::string txid = json_string_value(res);
  *params.logFile << "<QuadrigaCX> Done (transaction ID: " << txid << ")\n" << std::endl;
  return txid;
}


bool isOrderComplete(Parameters& params, std::string orderId) {
  auto ret = false;

  //todo: implement
  unique_json options {json_object()};
  json_object_set_new(options.get(), "id", json_string(orderId.c_str()));

  unique_json root { authRequest(params, "/v2/lookup_order", options.get()) };
  
  auto res = json_object_get(json_array_get(root.get(),0), "status");
  if( json_is_string(res) ){
    auto value = std::string(json_string_value(res));
    if(value.compare("2") == 0){
      ret = true;
    }
  }
  else{
    auto dump = json_dumps(root.get(), 0);
    *params.logFile << "<QuadrigaCX> Error: failed to get order status for id: " << orderId 
    << ":" << dump << std::endl;
    free(dump);
  }

  return ret;
   
}

double getActivePos(Parameters& params) {
  return getAvail(params, "btc");
}


double getLimitPrice(Parameters &params, double volume, bool isBid)
{
  //todo: tested
  if (!quadrigaGotLimPrice)
  {
    auto &exchange = queryHandle(params);
    quadrigaLimPrice.reset(exchange.getRequest("/v2/order_book?book=btc_usd"));
  }
  quadrigaGotLimPrice = !quadrigaGotLimPrice;
  auto root = quadrigaLimPrice.get();
  auto branch = json_object_get(root, isBid ? "bids" : "asks");

  // loop on volume
  double totVol = 0.0;
  double currPrice = 0.0;
  double currVol = 0.0;
  unsigned int i = 0;
  // [[<price>, <volume>], [<price>, <volume>], ...]
  for(i = 0; i < (json_array_size(branch)); i++)
  {
    // volumes are added up until the requested volume is reached
    currVol = atof(json_string_value(json_array_get(json_array_get(branch, i), 1)));
    currPrice = atof(json_string_value(json_array_get(json_array_get(branch, i), 0)));
    totVol += currVol;
    if(totVol >= volume * params.orderBookFactor){
        break;
    }
  }

  return currPrice;
}


json_t* authRequest(Parameters& params, std::string request, json_t * options)
{

  uint64_t nonce = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

  //post data is json
  unique_json payload {json_object()};
  auto root = payload.get();
  json_object_set_new(root, "key", json_string(params.quadrigaApi.c_str()));
  json_object_set_new(root, "nonce", json_integer(json_int_t(nonce)));
  json_object_set_new(root, "signature", json_string(getSignature(params, nonce).c_str()));

  if(options != nullptr){
    const char * key;
    json_t * value;

    json_object_foreach(options, key, value){
      if(value != nullptr && key != nullptr)
        json_object_set_new(root, key, value);
    }
  }

  auto payload_string = json_dumps(root, 0);
  std::string post_data(payload_string);
  free(payload_string);

  std::string headers[2] = { 
      "Content-Type: application/json; charset=utf-8",
      "Content-Length: " + std::to_string(post_data.length())
  };

  // cURL request
  auto &exchange = queryHandle(params);
  auto ret = exchange.postRequest(request,
                              make_slist(std::begin(headers), std::end(headers)),
                              post_data);
  return ret;
}


std::string getSignature(Parameters& params, const uint64_t nonce)
{
  std::string sig_data_str = std::to_string(nonce) + params.quadrigaClientId + params.quadrigaApi;
  auto data_len = sig_data_str.length();
  std::vector<uint8_t> sig_data(data_len);

  copy(sig_data_str.begin(), sig_data_str.end(), sig_data.begin());

  uint8_t * hmac_digest = HMAC(EVP_sha256(),
                               params.quadrigaSecret.c_str(),
                               params.quadrigaSecret.length(),
                               sig_data.data(),
                               data_len,
                               NULL, NULL);

  //convert to hex string
  std::ostringstream hex_stream;
  for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
  {
      hex_stream << std::hex << std::setfill('0') << std::setw(2) << (int32_t) hmac_digest[i];
  }
  return hex_stream.str();
}


void testQuadriga(){

    Parameters params("blackbird.conf");
    params.quadrigaSecret = "";
    params.quadrigaClientId = "";
    params.quadrigaApi = "";
    params.logFile = new std::ofstream("./test.log" , std::ofstream::trunc);

    std::string orderId;

    std::cout << "Current value BTC_USD bid: " << getQuote(params).bid() << std::endl;
    std::cout << "Current value BTC_USD ask: " << getQuote(params).ask() << std::endl;
    std::cout << "Current balance BTC: " << getAvail(params, "btc") << std::endl;
    std::cout << "Current balance USD: " << getAvail(params, "usd")<< std::endl;
    std::cout << "Current balance ETH: " << getAvail(params, "eth")<< std::endl;
    std::cout << "Current balance CAD: " << getAvail(params, "cad")<< std::endl;
    std::cout << "current bid limit price for 10 units: " << getLimitPrice(params, 10.0, true) << std::endl;
    std::cout << "Current ask limit price for 10 units: " << getLimitPrice(params, 10.0, false) << std::endl;

    std::cout << "Sending buy order for 0.005 BTC @ 1000 USD - TXID: " ;
    orderId = sendLongOrder(params, "buy", 0.005, 1000);
    std:: cout << orderId << std::endl;
    std::cout << "Buy order is complete: " << isOrderComplete(params, orderId) << std::endl;

    std::cout << "Sending sell order for 0.005 BTC @ 5000 USD - TXID: " ;
    orderId = sendLongOrder(params, "sell", 0.005, 5000);
    std:: cout << orderId << std::endl;
    std::cout << "Sell order is complete: " << isOrderComplete(params, orderId) << std::endl;


}

}
