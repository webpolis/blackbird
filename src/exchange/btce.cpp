#include "btce.h"
#include "parameters.h"
#include "utils/restapi.h"
#include "unique_json.hpp"

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
  unique_json root { exchange.getRequest("/api/3/ticker/btc_usd") };

  double bidValue = json_number_value(json_object_get(json_object_get(root.get(), "btc_usd"), "sell"));
  double askValue = json_number_value(json_object_get(json_object_get(root.get(), "btc_usd"), "buy"));

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
