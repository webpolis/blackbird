#include <string.h>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <jansson.h>
#include "itbit.h"
#include "curl_fun.h"

namespace ItBit {

quote_t getQuote(Parameters& params)
{
  bool GETRequest = false;
  json_t* root = getJsonFromUrl(params, "https://api.itbit.com/v1/markets/XBTUSD/ticker", "", GETRequest);
  const char *quote = json_string_value(json_object_get(root, "bid"));
  auto bidValue = quote ? std::stod(quote) : 0.0;

  quote = json_string_value(json_object_get(root, "ask"));
  auto askValue = quote ? std::stod(quote) : 0.0;

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

