#ifndef RESTAPI_H
#define RESTAPI_H

#include "curl/curl.h"
#include <iostream>
#include <memory>
#include <string>

struct json_t;
class RestApi
{
  struct CURL_deleter
  {
    void operator () (CURL *);
    void operator () (curl_slist *);
  };

  typedef std::unique_ptr<CURL, CURL_deleter> unique_curl;
  typedef std::string string;

  unique_curl C;
  const string host;
  std::ostream &log;

public:
  using unique_slist = std::unique_ptr<curl_slist, CURL_deleter>;

  RestApi              (string host, const char *cacert = nullptr,
                        std::ostream &log = std::cerr);
  RestApi              (const RestApi &) = delete;
  RestApi& operator =  (const RestApi &) = delete;

  json_t* getRequest   (const string &uri, unique_slist headers = nullptr);
  json_t* postRequest  (const string &uri, unique_slist headers = nullptr,
                        const string &post_data = "");
  json_t* postRequest  (const string &uri, const string &post_data);
};

template <typename T>
RestApi::unique_slist make_slist(T begin, T end)
{
  RestApi::unique_slist::pointer res;
  for (res = nullptr; begin != end; ++begin)
    res = curl_slist_append(res, begin->c_str());

  return RestApi::unique_slist(res);  // unique_ptr constructor is explicit
}

#endif
