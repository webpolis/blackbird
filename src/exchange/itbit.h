#ifndef ITBIT_H
#define ITBIT_H

#include "quote_t.h"
#include "parameters.h"

#include <curl/curl.h>
#include <string>

namespace ItBit {

quote_t getQuote(Parameters& params);

double getAvail(Parameters& params, std::string currency);

double getActivePos(Parameters& params);

double getLimitPrice(Parameters& params, double volume, bool isBid);

}

#endif

