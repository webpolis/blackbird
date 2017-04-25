#ifndef KRAKEN_H
#define KRAKEN_H

#include "quote_t.h"
#include "parameters.h"

#include <curl/curl.h>
#include <string>

namespace Kraken {

quote_t getQuote(Parameters& params);

double getAvail(Parameters& params, std::string currency);

std::string sendLongOrder(Parameters& params, std::string direction, double quantity, double price);

bool isOrderComplete(Parameters& params, std::string orderId);

double getActivePos(Parameters& params);

double getLimitPrice(Parameters& params, double volume, bool isBid);

json_t* authRequest(Parameters& params, std::string url, std::string request, std::string options="");

}

#endif
