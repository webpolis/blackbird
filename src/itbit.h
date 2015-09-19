#ifndef ITBIT_H
#define ITBIT_H

#include <curl/curl.h>
#include <string>
#include "parameters.h"

namespace ItBit {

  // get quote
  double getQuote(CURL *curl, bool isBid);

  // get the current availability for usd or btc
  double getAvail(CURL *curl, Parameters params, std::string currency);

  // get the bitcoin exposition
  double getActivePos(CURL *curl, Parameters params);

  // get the limit price according to the requested volume
  double getLimitPrice(CURL *curl, double volume, bool isBid);

}
#endif
