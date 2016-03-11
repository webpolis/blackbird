#ifndef CURL_FUN_H
#define CURL_FUN_H

#include <string>
#include <jansson.h>
#include <curl/curl.h>
#include "parameters.h"

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

json_t* getJsonFromUrl(Parameters& params, std::string url, std::string postField, bool getRequest);

#endif

