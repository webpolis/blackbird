#ifndef CEXIO_H
#define CEXIO_H

#include "quote_t.h"
#include <string>
#include <sstream>

struct json_t;
struct Parameters;

namespace Cexio {

quote_t getQuote(Parameters& params);

double getAvail(Parameters& params, std::string currency);

std::string sendLongOrder(Parameters& params, std::string direction, double quantity, double price);

std::string sendShortOrder(Parameters& params, std::string direction, double quantity, double price);

std::string sendOrder(Parameters& params, std::string direction, double quantity, double price);

std::string openPosition(Parameters& params,std::string direction, double quantity, double price);

std::string closePosition(Parameters& params);


bool isOrderComplete(Parameters& params, std::string orderId);

double getActivePos(Parameters& params);

double getLimitPrice(Parameters& params, double volume, bool isBid);

void testCexio();

}

#endif
