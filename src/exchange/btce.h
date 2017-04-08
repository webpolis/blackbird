#ifndef BTCE_H
#define BTCE_H

#include "quote_t.h"
#include "parameters.h"

#include <curl/curl.h>
#include <string>

namespace BTCe {

quote_t getQuote(Parameters& params);

double getAvail(Parameters& params, std::string currency);

double getActivePos(Parameters& params);

double getLimitPrice(Parameters& params, double volume, bool isBid);

}

#endif

