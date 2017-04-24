#ifndef GDAX_H
#define GDAX_H

#include "quote_t.h"
#include <string>

struct Parameters;

namespace GDAX {

quote_t getQuote(Parameters &params);

double getAvail(Parameters &params, std::string currency);

double getActivePos(Parameters &params);

double getLimitPrice(Parameters &params, double volume, bool isBid);

}

#endif
