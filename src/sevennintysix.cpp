#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <sys/time.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <openssl/hmac.h>
#include "base64.h"
#include <jansson.h>
#include "sevennintysix.h"
#include "curl_fun.h"

namespace SevenNintySix {

double getQuote(Parameters& params, bool isBid) {
	json_t* root = getJsonFromUrl(params, "http://api.796.com/v3/futures/ticker.html?type=weekly", "");
	const char* quote;
	double quoteValue;
	if (isBid) {
	  quote = json_string_value(json_object_get(json_object_get(root, "ticker"), "buy"));
	} else {
	  quote = json_string_value(json_object_get(json_object_get(root, "ticker"), "sell"));
	}
	if (quote != NULL) {
		quoteValue = atof(quote);
	} else {
		quoteValue = 0.0;
	}
	json_decref(root);
	return quoteValue;
}


double getAvail(Parameters& params, std::string currency){
	  json_t* root= authRequest(params,"https://796.com/v2/user/get_balance","balances","");
	  while (json_object_get(root, "message") != NULL) {
	    sleep(1.0);
	    *params.logFile << "<SevenNintySix> Error with JSON in getAvail: " << json_dumps(root, 0) << ". Retrying..." << std::endl;
	    root = authRequest(params,"https://796.com/v2/user/get_balance","balances","");
	  }

	  double availability = 0.0;
	  if (currency.compare("btc") == 0) {
	    availability = atof(json_string_value(json_object_get(root, "btc_balance")));
	  }
	  else if (currency.compare("mri") == 0) {
	    availability = atof(json_string_value(json_object_get(root, "mri_balance")));
	  }
	  else if (currency.compare("asicminer") == 0) {
	 	    availability = atof(json_string_value(json_object_get(root, "asicminer_balance")));
	  }
	  json_decref(root);
	  return availability;
}

int sendOrder(Parameters& params, std::string direction, double quantity, double price){
	double limPrice;  // define limit price to be sure to be executed
	if (direction.compare("buy") == 0) {
		limPrice = getLimitPrice(params, quantity, false);
	}
	else if (direction.compare("sell") == 0) {
	  limPrice = getLimitPrice(params, quantity, true);
	}

	*params.logFile << "<SevenNintySix> Trying to send a \"" << direction << "\" limit order: " << quantity << "@$" << limPrice << "..." << std::endl;
	std::ostringstream oss;
	oss << "\"symbol\":\"btcusd\", \"amount\":\"" << quantity << "\", \"price\":\"" << limPrice << "\", \"exchange\":\"bitfinex\", \"side\":\"" << direction << "\", \"type\":\"limit\"";
	std::string options = oss.str();

	json_t* root= authRequest(params, "https://796.com/v2/weeklyfutures/orders", "order/new", options);
	int orderId = json_integer_value(json_object_get(root, "id"));
	*params.logFile << "<SevenNintySix> Done (order ID: " << orderId << ")\n" << std::endl;

	json_decref(root);
	return orderId;
}

// check the status of the order
bool isOrderComplete(Parameters& params, int orderId){
	if (orderId == 0) {
	    return true;
	}
	std::ostringstream oss;
	oss << "\"order_id\":" << orderId;
	std::string options = oss.str();

	json_t* root= authRequest(params, "https://796.com/v2/weeklyfutures/orders", "order/status", options);

	bool isComplete = false;
	std::string _str=json_string_value(json_object_get((json_object_get(root, "data")),"state"));
	if(0 == strcmp(_str.c_str(),"done"))
	{
		isComplete = true;
	}
	else if((0 == strcmp(_str.c_str(),"wait"))||(0 == strcmp(_str.c_str(),"not")))
	{
		isComplete = false;
	}
	json_decref(root);
	return isComplete;
}

// get the Bitcoin exposition
double getActivePos(Parameters& params){
	return getAvail(params, "btc");
}

// get the limit price according to the requested volume
double getLimitPrice(Parameters& params, double volume, bool isBid){
	json_t* root;
	  if (isBid) {
	    root = json_object_get(getJsonFromUrl(params, "http://api.796.com/v3/futures/depth.html?type=weekly", ""), "bids");
	  } else {
	    root = json_object_get(getJsonFromUrl(params, "http://api.796.com/v3/futures/depth.html?type=weekly", ""), "asks");
	  }
	  // loop on volume
	  *params.logFile << "<SevenNintySix> Looking for a limit price to fill " << volume << "BTC..." << std::endl;
	  double tmpVol = 0.0;
	  int i = 0;
	  while (tmpVol < volume) {
	    // volumes are added up until the requested volume is reached
	      double p = json_real_value(json_array_get(json_array_get(root, i), 0));
	      double v = json_real_value(json_array_get(json_array_get(root, i), 1));

	    *params.logFile << "<SevenNintySix> order book: " << v << "@$" << p << std::endl;
	    tmpVol += v;
	    i++;
	  }
	  // return the next offer
	  double limPrice = 0.0;
	  if (params.aggressiveVolume) {
	    limPrice = atof(json_string_value(json_array_get(json_array_get(root, i), 0)));
	  } else {
	    limPrice = atof(json_string_value(json_array_get(json_array_get(root, i+1), 0)));
	  }
	  json_decref(root);
	  return limPrice;
}

// send a request to the exchange and return a JSON object
json_t* authRequest(Parameters& params, std::string url, std::string signature, std::string content){

	  unsigned char digest[MD5_DIGEST_LENGTH];

	  MD5((unsigned char*)signature.c_str(), strlen(signature.c_str()), (unsigned char*)&digest);

	  char mdString[33];
	  for (int i = 0; i < 16; i++) {
	    sprintf(&mdString[i*2], "%02X", (unsigned int)digest[i]);
	  }

	  std::ostringstream oss;
	  oss << content << "&sign=" << mdString;

	  std::string postParameters = oss.str().c_str();

	  // cURL headers
	  struct curl_slist* headers = NULL;
	  headers = curl_slist_append(headers, "contentType: application/x-www-form-urlencoded");

	  // cURL request
	  CURLcode resCurl;
	  // curl = curl_easy_init();
	  if (params.curl) {
	    curl_easy_setopt(params.curl, CURLOPT_POST,1L);
	    // curl_easy_setopt(params.curl, CURLOPT_VERBOSE, 1L);
	    curl_easy_setopt(params.curl, CURLOPT_HTTPHEADER, headers);
	    curl_easy_setopt(params.curl, CURLOPT_POSTFIELDS, postParameters.c_str());
	    curl_easy_setopt(params.curl, CURLOPT_SSL_VERIFYPEER, 0L);
	    curl_easy_setopt(params.curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	    std::string readBuffer;
	    curl_easy_setopt(params.curl, CURLOPT_WRITEDATA, &readBuffer);
	    curl_easy_setopt(params.curl, CURLOPT_URL, url.c_str());
	    curl_easy_setopt(params.curl, CURLOPT_CONNECTTIMEOUT, 10L);
	    resCurl = curl_easy_perform(params.curl);
	    json_t* root;
	    json_error_t error;

	    while (resCurl != CURLE_OK) {
	      *params.logFile << "<SevenNintySix> Error with cURL. Retry in 2 sec..." << std::endl;
	      sleep(2.0);
	      resCurl = curl_easy_perform(params.curl);
	    }
	    root = json_loads(readBuffer.c_str(), 0, &error);

	    while (!root) {
	      *params.logFile << "<SevenNintySix> Error with JSON:\n" << error.text << ". Retrying..." << std::endl;
	      readBuffer = "";
	      resCurl = curl_easy_perform(params.curl);
	      while (resCurl != CURLE_OK) {
	        *params.logFile << "<SevenNintySix> Error with cURL. Retry in 2 sec..." << std::endl;
	        sleep(2.0);
	        readBuffer = "";
	        resCurl = curl_easy_perform(params.curl);
	      }
	      root = json_loads(readBuffer.c_str(), 0, &error);
	    }
	    curl_slist_free_all(headers);
	    curl_easy_reset(params.curl);
	    return root;
	  }
	  else {
	    *params.logFile << "<SevenNintySix> Error with cURL init." << std::endl;
	    return NULL;
	  }

	}
}
