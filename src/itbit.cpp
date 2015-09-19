#include <string.h>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <jansson.h>
#include "itbit.h"
#include "curl_fun.h"

namespace ItBit {

double getQuote(CURL *curl, bool isBid) {
    
  double quote;
  json_t *root = getJsonFromUrl(curl, "https://api.itbit.com/v1/markets/XBTUSD/ticker", "");
  if (isBid) {
    quote = atof(json_string_value(json_object_get(root, "bid")));
  } else {
    quote = atof(json_string_value(json_object_get(root, "ask"))); 
  }
  json_decref(root);
  return quote;
}


double getAvail(CURL *curl, Parameters params, std::string currency) {
  // TODO
  return 0.0;
}

  
double getActivePos(CURL *curl, Parameters params) {
  // TODO
  return 0.0;
}

double getLimitPrice(CURL *curl, double volume, bool isBid) {
  // TODO
  return 0.0;
}

}
