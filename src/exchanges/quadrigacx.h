#ifndef QUADRIGACX_H
#define QUADRIGACX_H

#include "quote_t.h"
#include <string>
#include <sstream>

struct json_t;
struct Parameters;

namespace QuadrigaCX {

quote_t getQuote(Parameters& params);

double getAvail(Parameters& params, std::string currency);

std::string sendLongOrder(Parameters& params, std::string direction, double quantity, double price);

bool isOrderComplete(Parameters& params, std::string orderId);

double getActivePos(Parameters& params);

double getLimitPrice(Parameters& params, double volume, bool isBid);

json_t* authRequest(Parameters& params, std::string request, json_t * options = nullptr);

std::string getSignature(Parameters& params, const uint64_t  nonce);

void testQuadriga();
}

#endif
