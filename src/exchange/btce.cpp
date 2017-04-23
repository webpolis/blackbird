#include "btce.h"
#include "parameters.h"
#include "utils/restapi.h"

#include "jansson.h"
#include <unistd.h>

namespace BTCe {

static RestApi& queryHandle(Parameters &params)
{
  static RestApi query ("https://btc-e.com",
                        params.cacert.c_str(), *params.logFile);
  return query;
}

quote_t getQuote(Parameters& params)
{
  auto &exchange = queryHandle(params);
  json_t *root = exchange.getRequest("/api/3/ticker/btc_usd");

  double bidValue = json_number_value(json_object_get(json_object_get(root, "btc_usd"), "sell"));
  double askValue = json_number_value(json_object_get(json_object_get(root, "btc_usd"), "buy"));

  json_decref(root);
  return std::make_pair(bidValue, askValue);
}

double getAvail(Parameters& params, std::string currency) {
  // TODO
  return 0.0;
}

double getActivePos(Parameters& params) {
  // TODO
  return 0.0;
}

double getLimitPrice(Parameters& params, double volume, bool isBid) {
  // TODO
  return 0.0;
}

}
