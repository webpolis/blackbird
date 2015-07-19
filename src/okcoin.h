#ifndef OKCOIN_H
#define OKCOIN_H

#include <curl/curl.h>
#include <string>
#include "parameters.h"

namespace OkCoin {

// get quote
double getQuote(CURL *curl, bool isBid);

// get the current availability for usd or btc
double getAvail(CURL *curl, Parameters params, std::string currency);

// send order to the exchange and return order ID
int sendOrder(CURL *curl, Parameters params, std::string direction, double quantity, double price);

// check the status of the order
bool isOrderComplete(CURL *curl, Parameters params, int orderId);

// get the bitcoin exposition
double getActivePos(CURL *curl, Parameters params);

// get the limit price according to the requested volume
double getLimitPrice(CURL *curl, double volume, bool isBid);

// send a request to the exchange and return a json object
json_t* authRequest(CURL *curl, std::string url, std::string signature, std::string content);

}

#endif
