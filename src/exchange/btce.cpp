#include "btce.h"
#include "parameters.h"
#include "utils/restapi.h"
#include "unique_json.hpp"
#include "hex_str.hpp"

#include "openssl/sha.h"
#include "openssl/hmac.h"
#include <array>
#include <time.h>
#include <unistd.h>

namespace BTCe {

static json_t* authRequest(Parameters &, const char *, const std::string & = "");

static RestApi& queryHandle(Parameters &params)
{
  static RestApi query ("https://btc-e.com",
                        params.cacert.c_str(), *params.logFile);
  return query;
}

static json_t* checkResponse(std::ostream &logFile, json_t *root)
{
  unique_json own { root };
  auto stat = json_object_get(root, "success");
  if (json_integer_value(stat) == 0)
  {
    auto errmsg = json_object_get(root, "error");
    logFile << "<BTC-e> Error with response: "
            << json_string_value(errmsg) << '\n';
  }

  auto result = json_object_get(root, "return");
  json_incref(result);
  return result;
}

quote_t getQuote(Parameters& params)
{
  auto &exchange = queryHandle(params);
  unique_json root { exchange.getRequest("/api/3/ticker/btc_usd") };

  double bidValue = json_number_value(json_object_get(json_object_get(root.get(), "btc_usd"), "sell"));
  double askValue = json_number_value(json_object_get(json_object_get(root.get(), "btc_usd"), "buy"));

  return std::make_pair(bidValue, askValue);
}

double getAvail(Parameters &params, std::string currency)
{
  unique_json root { authRequest(params, "getInfo") };
  auto funds = json_object_get(json_object_get(root.get(), "funds"), currency.c_str());
  return json_number_value(funds);
}

double getActivePos(Parameters& params) {
  // TODO
  return 0.0;
}

double getLimitPrice(Parameters& params, double volume, bool isBid) {
  // TODO
  return 0.0;
}

json_t* authRequest(Parameters &params, const char *request, const std::string &options)
{
  using namespace std;
  static uint64_t nonce = time(nullptr) * 4;
  string post_body = "nonce="   + to_string(++nonce) +
                     "&method=" + request;
  if (!options.empty())
  {
    post_body += '&';
    post_body += options;
  }

  uint8_t *sign = HMAC (EVP_sha512(),
                        params.btceSecret.data(), params.btceSecret.size(),
                        reinterpret_cast<const uint8_t *>(post_body.data()), post_body.size(),
                        nullptr, nullptr);
  auto &exchange = queryHandle(params);
  array<string, 2> headers
  {
    "Key:"  + params.btceApi,
    "Sign:" + hex_str(sign, sign + SHA512_DIGEST_LENGTH),
  };
  return checkResponse (*params.logFile,
                        exchange.postRequest ("/tapi",
                                              make_slist(begin(headers), end(headers)),
                                              post_body));
}

}
