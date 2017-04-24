#include "gdax.h"
#include "parameters.h"
#include "utils/restapi.h"

#include "jansson.h"

namespace GDAX {

static RestApi& queryHandle(Parameters &params)
{
  static RestApi query ("https://api.gdax.com",
                        params.cacert.c_str(), *params.logFile);
  return query;
}

quote_t getQuote(Parameters &params)
{
  auto &exchange = queryHandle(params);
  json_t *root = exchange.getRequest("/products/BTC-USD/ticker");

  const char *bid, *ask;
  int unpack_fail = json_unpack(root, "{s:s, s:s}", "bid", &bid, "ask", &ask);
  if (unpack_fail)
  {
    bid = "0";
    ask = "0";
  }
  quote_t bidask{std::stod(bid), std::stod(ask)};

  json_decref(root);
  return bidask;
}

double getAvail(Parameters &params, std::string currency)
{
  // TODO
  return 0.0;
}

double getActivePos(Parameters &params)
{
  // TODO
  return 0.0;
}

double getLimitPrice(Parameters &params, double volume, bool isBid)
{
  // TODO
  return 0.0;
}

}
