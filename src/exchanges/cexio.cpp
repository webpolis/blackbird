#include "cexio.h"
#include "parameters.h"
#include "utils/restapi.h"
#include "utils/base64.h"
#include "unique_json.hpp"
#include "hex_str.hpp"

#include "openssl/sha.h"
#include "openssl/hmac.h"
#include <vector>
#include <iomanip>
#include <array>
#include <chrono>
#include <cmath>
#include <algorithm>

namespace Cexio {

static json_t* authRequest(Parameters &, std::string, std::string);

static bool g_bShort= false;

static RestApi& queryHandle(Parameters &params)
{
  static RestApi query ("https://cex.io/api",
                        params.cacert.c_str(), *params.logFile);
  return query;
}

static json_t* checkResponse(std::ostream &logFile, json_t *root)
{
  auto errstatus = json_object_get(root, "error");

  if (errstatus)
  {
    auto errmsg = json_dumps(errstatus, JSON_ENCODE_ANY);
    logFile << "<Cexio> Error with response: "
            << errmsg << '\n';
    free(errmsg);
  }

  return root;
}

quote_t getQuote(Parameters &params)
{
  auto &exchange = queryHandle(params); 
  unique_json root { exchange.getRequest("/ticker/BTC/USD") };

  double bidValue = json_number_value(json_object_get(root.get(), "bid"));
  double askValue = json_number_value(json_object_get(root.get(), "ask"));

  return std::make_pair(bidValue, askValue);
}

double getAvail(Parameters& params, std::string currency)
{
  double available = 0.0;
  std::transform(currency.begin(), currency.end(), currency.begin(), ::toupper);
  const char * curr_ = currency.c_str();

  unique_json root { authRequest(params, "/balance/","") };

  const char * avail_str = json_string_value(json_object_get(json_object_get(root.get(), curr_), "available"));
  available = avail_str ? atof(avail_str) : 0.0;

  return available;
}

std::string sendLongOrder(Parameters& params, std::string direction, double quantity, double price)
{
    g_bShort = false;
    return sendOrder(params, direction, quantity, price);

}

std::string sendShortOrder(Parameters& params, std::string direction, double quantity, double price)
{
  //return sendOrder(params, direction, quantity, price);

  g_bShort = true;

  if(direction.compare("sell") == 0)
  {
     return openPosition(params, direction, quantity, price);

  }else if(direction.compare("buy")== 0){

    return closePosition(params);

  }

   return "0";
}


std::string openPosition(Parameters& params, std::string direction, double quantity, double price)
{
   using namespace std;
   string pair = "btc_usd";
   string orderId = "";
   *params.logFile << "<Cexio> Trying to open a " << pair << " " << direction << " position: " << quantity << "@" << price << endl;

   ostringstream oss;
   string ptype = "";
   
   if(direction.compare("sell") == 0)
   {
      ptype = "short";

   }else if(direction.compare("buy")== 0){
     ptype = "long";
   }
   

   oss << "symbol=" << "BTC" << "&amount=" <<  setprecision(8) << quantity << "&msymbol=" << "USD" << "&leverage=" 
    << "3" << "&ptype=" << ptype << "&anySlipppage=" << "true" << "&eoprice=" <<  setprecision(8) << price << "&stopLossPrice=" << "35000";

   string options = oss.str();

   unique_json root {authRequest(params,"open_position/BTC/USD/",options)};
   auto error = json_string_value(json_object_get(root.get(),"error"));
   if(error){

        orderId = "0";

    }else{

        orderId = json_string_value(json_object_get(root.get(),"id"));
        *params.logFile << "<Cexio>Open Position Done (positon ID): " << orderId << ")\n" << endl;

    }

    return orderId;
}

std::string closePosition(Parameters& params)
{

 using namespace std;
 string orderId = "";
 unique_json root {authRequest(params,"/open_positions/BTC/USD/","")};
 size_t arraySize = json_array_size(json_object_get(root.get(),"data"));
 auto data = json_object_get(root.get(),"data");
 for(size_t i=0; i< arraySize;i++){
 
     std::string tmpId = json_string_value(json_object_get(json_array_get(data,i),"id"));
     ostringstream oss;
     oss << "id=" << tmpId;
     string options = oss.str();
     
     unique_json root1 {authRequest(params,"close_position/BTC/USD/",options)};
     auto error = json_string_value(json_object_get(root1.get(),"error"));
     if(error){
           orderId ="0";

     }else{

         orderId = json_string_value(json_object_get(root1.get(),"id"));
       
     }

 }

return orderId;


}



std::string sendOrder(Parameters& params, std::string direction, double quantity, double price)
{
  using namespace std;
  string pair = "btc_usd";
  string orderId = "";
  *params.logFile << "<Cexio> Trying to send a " << pair << " " << direction << " limit order: " << quantity << "@" << price << endl;

  ostringstream oss;
  oss << "type=" << direction << "&amount=" << quantity << "&price=" << fixed << setprecision(2) << price;
  string options = oss.str();

  unique_json root { authRequest(params, "/place_order/BTC/USD/", options) };
  auto error = json_string_value(json_object_get(root.get(), "error"));
  if (error){
    // auto dump = json_dumps(root.get(), 0);
    // *params.logFile << "<Cexio> Error placing order: " << dump << ")\n" << endl;
    // free(dump);
    orderId = "0";
  } else {
    orderId = json_string_value(json_object_get(root.get(), "id"));
    *params.logFile << "<Cexio> Done (order ID): " << orderId << ")\n" << endl;
  }
  return orderId;
}

bool isOrderComplete(Parameters& params, std::string orderId)
{


  using namespace std;
  if (orderId == "0") return false;


  auto oId = stol(orderId);
  ostringstream oss;
  oss << "id=" << oId;
  string options = oss.str();

 if(g_bShort)
 {
   unique_json root { authRequest(params, "/get_position/", options) };
   auto slremains = atof(json_string_value(json_object_get(root.get(), "slremains")));
  if (slremains==0){
    return true;
  } else { 
    auto dump = json_dumps(root.get(), 0);
    *params.logFile << "<Cexio> Position Order Not Complete: " << dump << ")\n" << endl;
    free(dump);
    // cout << "REMAINS:" << remains << endl;
    return false; 
  }

 }else{

  unique_json root { authRequest(params, "/get_order/", options) };
  auto remains = atof(json_string_value(json_object_get(root.get(), "remains")));
  if (remains==0){
    return true;
  } else { 
    auto dump = json_dumps(root.get(), 0);
    *params.logFile << "<Cexio> Order Not Complete: " << dump << ")\n" << endl;
    free(dump);
    // cout << "REMAINS:" << remains << endl;
    return false; 
  }

 }
  
}

double getActivePos(Parameters& params) { return getAvail(params, "btc"); }

double getLimitPrice(Parameters &params, double volume, bool isBid)
{
  auto &exchange = queryHandle(params);
  auto root = unique_json(exchange.getRequest("/order_book/BTC/USD/"));
  auto branch = json_object_get(root.get(), isBid ? "bids" : "asks");

  // loop on volume
  double totVol = 0.0;
  double currPrice = 0.0;
  double currVol = 0.0;
  unsigned int i = 0;
  // // [[<price>, <volume>], [<price>, <volume>], ...]
  for(i = 0; i < (json_array_size(branch)); i++)
  {
    // volumes are added up until the requested volume is reached
    currVol = json_number_value(json_array_get(json_array_get(branch, i), 1));
    currPrice = json_number_value(json_array_get(json_array_get(branch, i), 0));
    totVol += currVol;
    if(totVol >= volume * params.orderBookFactor){
        break;
    }
  }

  return currPrice;
}

json_t* authRequest(Parameters &params, std::string request, std::string options)
{
  using namespace std;
  static uint64_t nonce = time(nullptr) * 4;
  auto msg = to_string(++nonce) + params.cexioClientId + params.cexioApi;
  uint8_t *digest = HMAC (EVP_sha256(),
                          params.cexioSecret.c_str(), params.cexioSecret.size(),
                          reinterpret_cast<const uint8_t *>(msg.data()), msg.size(),
                          nullptr, nullptr);

  string postParams = "key=" + params.cexioApi +
                           "&signature=" + hex_str<upperhex>(digest, digest + SHA256_DIGEST_LENGTH) +
                           "&nonce=" + to_string(nonce);
  
  if (!options.empty())
  {
    postParams += "&";
    postParams += options;
  }

  auto &exchange = queryHandle(params);
  return checkResponse(*params.logFile, exchange.postRequest(request, postParams));
}

void testCexio() {
  using namespace std;
  Parameters params("blackbird.conf");
  params.logFile = new ofstream("./test.log" , ofstream::trunc);

  string orderId;

  cout << "Current value BTC_USD bid: " << getQuote(params).bid() << endl;
  cout << "Current value BTC_USD ask: " << getQuote(params).ask() << endl;
  cout << "Current balance BTC: " << getAvail(params, "BTC") << endl;
  cout << "Current balance BCH: " << getAvail(params, "BCH") << endl;
  cout << "Current balance ETH: " << getAvail(params, "ETH") << endl;
  cout << "Current balance LTC: " << getAvail(params, "LTC") << endl;
  cout << "Current balance DASH: " << getAvail(params, "DASH") << endl;
  cout << "Current balance ZEC: " << getAvail(params, "ZEC") << endl;
  cout << "Current balance USD: " << getAvail(params, "USD") << endl;
  cout << "Current balance EUR: " << getAvail(params, "EUR")<< endl;
  cout << "Current balance GBP: " << getAvail(params, "GBP") << endl;
  cout << "Current balance RUB: " << getAvail(params, "RUB") << endl;
  cout << "Current balance GHS: " << getAvail(params, "GHS") << endl;
  cout << "Current bid limit price for 10 units: " << getLimitPrice(params, 10.0, true) << endl;
  cout << "Current ask limit price for 10 units: " << getLimitPrice(params, 10.0, false) << endl;

  // cout << "Sending buy order - TXID: " ;
  // orderId = sendLongOrder(params, "buy", 0.002, 9510);
  // cout << orderId << endl;
  // cout << "Buy order is complete: " << isOrderComplete(params, orderId) << endl;
  // cout << "Active positions (BTC): " << getActivePos(params) << endl;

  // cout << "Sending sell order - TXID: " ;
  // orderId = sendLongOrder(params, "sell", 0.002, 8800);
  // cout << orderId << endl;
  // cout << "Sell order is complete: " << isOrderComplete(params, orderId) << endl;

  // cout << "Active positions (BTC): " << getActivePos(params) << endl;

  // cout << "Sending short order - TXID: " ;
  // orderId = sendShortOrder(params, "sell", 0.002, 8800);
  // cout << orderId << endl;
  // cout << "Sell order is complete: " << isOrderComplete(params, orderId) << endl;

  // cout << "Active positions: " << getActivePos(params) << endl;

  /******* Cancel order functionality - for testing *******/
  // long oId = 0; //Add orderId here
  // ostringstream oss;
  // oss << "id=" << oId;
  // string options = oss.str();

  // unique_json root { authRequest(params, "/cancel_order/", options) };
  // auto dump = json_dumps(root.get(), 0);
  // *params.logFile << "<Cexio> Error canceling order: " << dump << ")\n" << endl;
  // free(dump);
  /*********************************************************/
  }

}
