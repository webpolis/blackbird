#include <string.h>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <jansson.h>
#include "kraken.h"
#include "curl_fun.h"

namespace Kraken {

double getQuote(Parameters& params, bool isBid) {
  json_t* root= getJsonFromUrl(params, "https://api.kraken.com/0/public/Ticker", "pair=XXBTZUSD");
  const char* quote;
  double quoteValue;
  if (isBid) {
    quote = json_string_value(json_array_get(json_object_get(json_object_get(json_object_get(root, "result"), "XXBTZUSD"), "b"), 0));
  } else {
    quote = json_string_value(json_array_get(json_object_get(json_object_get(json_object_get(root, "result"), "XXBTZUSD"), "a"), 0));
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
