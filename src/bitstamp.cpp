#include <string.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <sys/time.h>
#include "base64.h"
#include <iomanip>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <jansson.h>
#include "bitstamp.h"
#include "curl_fun.h"

namespace Bitstamp {

double getQuote(Parameters& params, bool isBid) {
  json_t* root = getJsonFromUrl(params, "https://www.bitstamp.net/api/ticker/", "");
  const char* quote;
  double quoteValue;
  if (isBid) {
    quote = json_string_value(json_object_get(root, "bid"));
  } else {
    quote = json_string_value(json_object_get(root, "ask"));
  }
  if (quote != NULL) {
    quoteValue = atof(quote);
  } else {
    quoteValue = 0.0;
  }
  json_decref(root);
  return quoteValue;
}


double getAvail(Parameters& params, std::string currency) {
  json_t* root= authRequest(params, "https://www.bitstamp.net/api/balance/", "");
  while (json_object_get(root, "message") != NULL) {
    sleep(1.0);
    *params.logFile << "<Bitstamp> Error with JSON in getAvail: " << json_dumps(root, 0) << ". Retrying..." << std::endl;
    root = authRequest(params, "https://www.bitstamp.net/api/balance/", "");
  }

  double availability = 0.0;
  if (currency.compare("btc") == 0) {
    availability = atof(json_string_value(json_object_get(root, "btc_balance")));
  }
  else if (currency.compare("usd") == 0) {
    availability = atof(json_string_value(json_object_get(root, "usd_balance")));
  }
  json_decref(root);
  return availability;
}


int sendOrder(Parameters& params, std::string direction, double quantity, double price) {
  double limPrice;  // define limit price to be sure to be executed
  if (direction.compare("buy") == 0) {
    limPrice = getLimitPrice(params, quantity, false);
  }
  else if (direction.compare("sell") == 0) {
    limPrice = getLimitPrice(params, quantity, true);
  }

  *params.logFile << "<Bitstamp> Trying to send a \"" << direction << "\" limit order: " << quantity << "@$" << limPrice << "..." << std::endl;
  std::ostringstream oss;
  oss << "https://www.bitstamp.net/api/" << direction << "/";
  std::string url = oss.str();
  oss.clear();
  oss.str("");

  oss << "amount=" << quantity << "&price=" << std::fixed << std::setprecision(2) << limPrice;
  std::string options = oss.str();
  json_t* root= authRequest(params, url, options);

  int orderId = json_integer_value(json_object_get(root, "id"));
  if (orderId == 0) {
    *params.logFile << "<Bitstamp> Order ID = 0. Message: " << json_dumps(root, 0) << std::endl;
  }
  *params.logFile << "<Bitstamp> Done (order ID: " << orderId << ")\n" << std::endl;
  json_decref(root);
  return orderId;
}


bool isOrderComplete(Parameters& params, int orderId) {
  if (orderId == 0) {
    return true;
  }
  std::ostringstream oss;
  oss << "id=" << orderId;
  std::string options = oss.str();
  json_t* root= authRequest(params, "https://www.bitstamp.net/api/order_status/", options);
  std::string status = json_string_value(json_object_get(root, "status"));
  json_decref(root);
  if (status.compare("Finished") == 0) {
    return true;
  } else {
    return false;
  }
}


double getActivePos(Parameters& params) {
  return getAvail(params, "btc");
}


double getLimitPrice(Parameters& params, double volume, bool isBid) {
  json_t* root;
  if (isBid) {
    root = json_object_get(getJsonFromUrl(params, "https://www.bitstamp.net/api/order_book/", ""), "bids");
  } else {
    root = json_object_get(getJsonFromUrl(params, "https://www.bitstamp.net/api/order_book/", ""), "asks");
  }

  // loop on volume
  *params.logFile << "<Bitstamp> Looking for a limit price to fill " << volume << "BTC..." << std::endl;
  double tmpVol = 0.0;
  int i = 0;
  while (tmpVol < volume) {
    // volumes are added up until the requested volume is reached
    double p = atof(json_string_value(json_array_get(json_array_get(root, i), 0)));
    double v = atof(json_string_value(json_array_get(json_array_get(root, i), 1)));
    *params.logFile << "<Bitstamp> order book: " << v << "@$" << p << std::endl;
    tmpVol += v;
    i++;
  }
  // return the next offer
  double limPrice = 0.0;
  if (params.aggressiveVolume) {
    limPrice = atof(json_string_value(json_array_get(json_array_get(root, i), 0)));
  } else {
    limPrice = atof(json_string_value(json_array_get(json_array_get(root, i+1), 0)));
  }
  json_decref(root);
  return limPrice;
}


json_t* authRequest(Parameters& params, std::string url, std::string options) {
  // nonce
  struct timeval tv;
  gettimeofday(&tv, NULL);
  unsigned long long nonce = (tv.tv_sec * 1000.0) + (tv.tv_usec * 0.001) + 0.5;

  std::ostringstream oss;
  oss << nonce << params.bitstampClientId << params.bitstampApi;
  unsigned char* digest;

  // Using sha256 hash engine
  digest = HMAC(EVP_sha256(), params.bitstampSecret, strlen(params.bitstampSecret), (unsigned char*)oss.str().c_str(), strlen(oss.str().c_str()), NULL, NULL);

  char mdString[SHA256_DIGEST_LENGTH+100];  // FIXME +100
  for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
    sprintf(&mdString[i*2], "%02X", (unsigned int)digest[i]);
  }

  oss.clear();
  oss.str("");

  oss << "key=" << params.bitstampApi << "&signature=" << mdString << "&nonce=" << nonce << "&" << options;
  std::string postParams = oss.str().c_str();

  CURLcode resCurl;
  if (params.curl) {
    std::string readBuffer;
    curl_easy_setopt(params.curl, CURLOPT_POST,1L);
    curl_easy_setopt(params.curl, CURLOPT_POSTFIELDS, postParams.c_str());
    curl_easy_setopt(params.curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(params.curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(params.curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(params.curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(params.curl, CURLOPT_CONNECTTIMEOUT, 10L);
    resCurl = curl_easy_perform(params.curl);
    json_t* root;
    json_error_t error;

    while (resCurl != CURLE_OK) {
      *params.logFile << "<Bitstamp> Error with cURL. Retry in 2 sec..." << std::endl;
      sleep(2.0);
      readBuffer = "";
      resCurl = curl_easy_perform(params.curl);
    }
    root = json_loads(readBuffer.c_str(), 0, &error);

    while (!root) {
      *params.logFile << "<Bitstamp> Error with JSON in authRequest:\n" << "Error: : " << error.text << ".  Retrying..." << std::endl;
      readBuffer = "";
      resCurl = curl_easy_perform(params.curl);
      while (resCurl != CURLE_OK) {
        *params.logFile << "<Bitstamp> Error with cURL. Retry in 2 sec..." << std::endl;
        sleep(2.0);
        readBuffer = "";
        resCurl = curl_easy_perform(params.curl);
      }
      root = json_loads(readBuffer.c_str(), 0, &error);
    }
    curl_easy_reset(params.curl);
    return root;
  }
  else {
    *params.logFile << "<Bitstamp> Error with cURL init." << std::endl;
    return NULL;
  }
}

}
