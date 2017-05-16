#include "poloniex.h"
#include "parameters.h"
#include "utils/restapi.h"
#include "unique_json.hpp"

#include "openssl/sha.h"
#include "openssl/hmac.h"

namespace Poloniex {

static RestApi& queryHandle(Parameters &params)
{
  static RestApi query ("https://poloniex.com",
                        params.cacert.c_str(), *params.logFile);
  return query;
}

quote_t getQuote(Parameters &params)
{
  auto &exchange = queryHandle(params);
  unique_json root { exchange.getRequest("/public?command=returnTicker") };

  const char *quote = json_string_value(json_object_get(json_object_get(root.get(), "USDT_BTC"), "highestBid"));
  auto bidValue = quote ? std::stod(quote) : 0.0;

  quote = json_string_value(json_object_get(json_object_get(root.get(), "USDT_BTC"), "lowestAsk"));
  auto askValue = quote ? std::stod(quote) : 0.0;

  return std::make_pair(bidValue, askValue);
}

double getAvail(Parameters& params, std::string currency) {
  // TODO
  return 0.0;
}

std::string sendLongOrder(Parameters& params, std::string direction, double quantity, double price) {
  // TODO
  return "0";
}

std::string sendShortOrder(Parameters& params, std::string direction, double quantity, double price) {
  // TODO
  return "0";
}

bool isOrderComplete(Parameters& params, std::string orderId) {
  // TODO
  return false;
}

double getActivePos(Parameters& params) {
  // TODO
  return 0.0;
}

double getLimitPrice(Parameters& params, double volume, bool isBid) {
  // TODO
  return 0.0;
}

json_t* authRequest(Parameters& params, std::string url, std::string request, std::string options) {
  // TODO
  return NULL;
}
  
}
