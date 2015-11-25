#ifndef BTCE_H
#define BTCE_H

#include <curl/curl.h>
#include <string>
#include "parameters.h"

namespace BTCe {

  // get quote
  double getQuote(Parameters& params, bool isBid);

  // get the current availability for usd or btc
  double getAvail(Parameters& params, std::string currency);

  // get the bitcoin exposition
  double getActivePos(Parameters& params);

  // get the limit price according to the requested volume
  double getLimitPrice(Parameters& params, double volume, bool isBid);

}
#endif
