#include <fstream>
#include <iomanip>
#include <unistd.h>
#include <numeric>
#include <math.h>
#include <algorithm>
#include <jansson.h>
#include <curl/curl.h>
#include <string.h>

#include "base64.h"
#include "bitcoin.h"
#include "result.h"
#include "time_fun.h"
#include "curl_fun.h"
#include "parameters.h"
#include "check_entry_exit.h"
#include "bitfinex.h"
#include "okcoin.h"
#include "bitstamp.h"
#include "gemini.h"
#include "kraken.h"
#include "itbit.h"
#include "send_email.h"

typedef double (*getQuoteType) (CURL *curl, bool isBid);
typedef double (*getAvailType) (CURL *curl, Parameters params, std::string currency);
typedef int (*sendOrderType) (CURL *curl, Parameters params, std::string direction, double quantity, double price);
typedef bool (*isOrderCompleteType) (CURL *curl, Parameters params, int orderId);
typedef double (*getActivePosType) (CURL *curl, Parameters params);
typedef double (*getLimitPriceType) (CURL *curl, double volume, bool isBid);

int main(int argc, char **argv) {
  std::cout << "Blackbird Bitcoin Arbitrage" << std::endl;
  std::cout << "DISCLAIMER: USE THE SOFTWARE AT YOUR OWN RISK.\n" << std::endl;

  // read the config file (config.json)
  json_error_t error;
  json_t *root = json_load_file("config.json", 0, &error);
  if (!root) {
    std::cout << "ERROR: config.json incorrect (" << error.text << ")\n" << std::endl;
    return -1;
  }

  int gapSec = json_integer_value(json_object_get(root, "GapSec"));
  unsigned debugMaxIteration = json_integer_value(json_object_get(root, "DebugMaxIteration"));
  bool useFullCash = json_boolean_value(json_object_get(root, "UseFullCash"));
  double untouchedCash = json_real_value(json_object_get(root, "UntouchedCash"));
  double cashForTesting = json_real_value(json_object_get(root, "CashForTesting"));
  double volPriceDelta = 10.0; // 0.30;

  if (!useFullCash && cashForTesting < 15.0) {
    std::cout << "ERROR: Minimum test cash is $15.00.\n" << std::endl;
      return -1;
  }

  // function arrays
  getQuoteType getQuote[10];
  getAvailType getAvail[10];
  sendOrderType sendOrder[10];
  isOrderCompleteType isOrderComplete[10];
  getActivePosType getActivePos[10];
  getLimitPriceType getLimitPrice[10];

  // thousand separator
  std::locale mylocale("");
  std::cout.imbue(mylocale);
  // print precision of two digits
  std::cout.precision(2);
  std::cout << std::fixed;
  // create a parameters structure
  Parameters params(root);
  int index = 0;
  std::string tmp;
  tmp = json_string_value(json_object_get(root, "BitfinexApiHead"));
  if (tmp.empty() == false) {
    params.addExchange("Bitfinex", json_real_value(json_object_get(root, "BitfinexFees")), json_boolean_value(json_object_get(root, "BitfinexCanShort")), true);
    getQuote[index] = Bitfinex::getQuote;
    getAvail[index] = Bitfinex::getAvail;
    sendOrder[index] = Bitfinex::sendOrder;
    isOrderComplete[index] = Bitfinex::isOrderComplete;
    getActivePos[index] = Bitfinex::getActivePos;
    getLimitPrice[index] = Bitfinex::getLimitPrice;  
    index++;
  }
  tmp = json_string_value(json_object_get(root, "OkCoinApiKey"));
  if (tmp.empty() == false) {
    params.addExchange("OKCoin", json_real_value(json_object_get(root, "OkCoinFees")), json_boolean_value(json_object_get(root, "OkCoinCanShort")), true);
    getQuote[index] = OkCoin::getQuote;
    getAvail[index] = OkCoin::getAvail;
    sendOrder[index] = OkCoin::sendOrder;
    isOrderComplete[index] = OkCoin::isOrderComplete;
    getActivePos[index] = OkCoin::getActivePos;
    getLimitPrice[index] = OkCoin::getLimitPrice;  
    index++;
  }
  tmp = json_string_value(json_object_get(root, "BitstampClientId"));
  if (tmp.empty() == false) {
    params.addExchange("Bitstamp", json_real_value(json_object_get(root, "BitstampFees")), json_boolean_value(json_object_get(root, "BitstampCanShort")), true);
    getQuote[index] = Bitstamp::getQuote;
    getAvail[index] = Bitstamp::getAvail;
    sendOrder[index] = Bitstamp::sendOrder;
    isOrderComplete[index] = Bitstamp::isOrderComplete;
    getActivePos[index] = Bitstamp::getActivePos;
    getLimitPrice[index] = Bitstamp::getLimitPrice;  
    index++;
  }
  tmp = json_string_value(json_object_get(root, "GeminiApiKey")); 
  if (tmp.empty() == false) {
    params.addExchange("Gemini", json_real_value(json_object_get(root, "GeminiFees")), json_boolean_value(json_object_get(root, "GeminiCanShort")), true);
    getQuote[index] = Gemini::getQuote;
    getAvail[index] = Gemini::getAvail;
    sendOrder[index] = Gemini::sendOrder;
    isOrderComplete[index] = Gemini::isOrderComplete;
    getActivePos[index] = Gemini::getActivePos;
    getLimitPrice[index] = Gemini::getLimitPrice;  
    index++;
  }
  tmp = json_string_value(json_object_get(root, "KrakenApiKey")); 
  if (tmp.empty() == false) {
    params.addExchange("Kraken", json_real_value(json_object_get(root, "KrakenFees")), json_boolean_value(json_object_get(root, "KrakenCanShort")), false);
    getQuote[index] = Kraken::getQuote;
    getAvail[index] = Kraken::getAvail;
    // sendOrder[index] = Kraken::sendOrder;
    // isOrderComplete[index] = Kraken::isOrderComplete;
    getActivePos[index] = Kraken::getActivePos;
    getLimitPrice[index] = Kraken::getLimitPrice;  
    index++;
  }
  tmp = json_string_value(json_object_get(root, "ItBitApiKey"));
  if (tmp.empty() == false) {
    params.addExchange("ItBit", json_real_value(json_object_get(root, "ItBitFees")), json_boolean_value(json_object_get(root, "ItBitCanShort")), false);
    getQuote[index] = ItBit::getQuote;
    getAvail[index] = ItBit::getAvail;
    // sendOrder[index] = ItBit::sendOrder;
    // isOrderComplete[index] = ItBit::isOrderComplete;
    getActivePos[index] = ItBit::getActivePos;
    getLimitPrice[index] = ItBit::getLimitPrice;  
    index++;
  }
  // CSV file
  std::string csvFileName;
  csvFileName = "result_" + printDateTimeFileName() + ".csv";
  std::ofstream csvFile;
  csvFile.open(csvFileName.c_str(), std::ofstream::trunc);
  csvFile << "TRADE_ID,EXCHANGE_LONG,EXHANGE_SHORT,ENTRY_TIME,EXIT_TIME,DURATION,TOTAL_EXPOSURE,BALANCE_BEFORE,BALANCE_AFTER,RETURN\n";
  csvFile.flush();
  // Vector of Bitcoin
  std::vector<Bitcoin*> btcVec;
  int num_exchange = params.nbExch();
  // create Bitcoin objects
  for (int i = 0; i < num_exchange; ++i) {
    btcVec.push_back(new Bitcoin(i, params.exchName[i], params.fees[i], params.hasShort[i], params.isImplemented[i]));
  }
  CURL* curl;
  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();

  std::cout << "[ Targets ]" << std::endl;
  std::cout << "   Spread to enter: " << params.spreadEntry * 100.0 << "%" << std::endl;
  std::cout << "   Spread to exit: " << params.spreadExit * 100.0  << "%" << std::endl;
  std::cout << std::endl;
  // store current balances
  std::cout << "[ Current balances ]" << std::endl;
  double *balanceUsd = (double*)malloc(sizeof(double) * num_exchange);
  double *balanceBtc = (double*)malloc(sizeof(double) * num_exchange);
  for (int i = 0; i < num_exchange; ++i) {
    balanceUsd[i] = getAvail[i](curl, params, "usd");
    balanceBtc[i] = getAvail[i](curl, params, "btc");
  }
  // contains balances after a completed trade
  double *newBalUsd = (double*)malloc(sizeof(double) * num_exchange);
  double *newBalBtc = (double*)malloc(sizeof(double) * num_exchange);
  memset(newBalUsd, 0.0, sizeof(double) * num_exchange);
  memset(newBalBtc, 0.0, sizeof(double) * num_exchange);

  for (int i = 0; i < num_exchange; ++i) {
    std::cout << "   " << params.exchName[i] << ":\t";
    std::cout << balanceUsd[i] << " USD\t" << std::setprecision(6) << balanceBtc[i]  << std::setprecision(2) << " BTC" << std::endl;
  }
  std::cout << std::endl;
  std::cout << "[ Cash exposure ]" << std::endl;
  if (useFullCash) {
    std::cout << "   FULL cash used!" << std::endl;
  } else {
    std::cout << "   TEST cash used\n   Value: $" << cashForTesting << std::endl;
  }
  std::cout << std::endl;

  time_t rawtime;
  rawtime = time(NULL);
  struct tm * timeinfo;
  timeinfo = localtime(&rawtime);
  // wait the next gapSec seconds before starting
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  while ((int)timeinfo->tm_sec % gapSec != 0) {
    sleep(0.01);
    time(&rawtime);
    timeinfo = localtime(&rawtime);
  }
  // main loop
  if (!params.verbose) {
    std::cout << "Running..." << std::endl;
  }
  bool inMarket = false;
  int resultId = 0;
  Result res;
  res.clear();
  unsigned currIteration = 0;
  bool stillRunning = true;
  time_t currTime;
  time_t diffTime;
  while (stillRunning) {

    currTime = mktime(timeinfo);
    time(&rawtime);
    diffTime = difftime(rawtime, currTime);
    // check if we are already too late
    if (diffTime > 0) {
      std::cout << "WARNING: " << diffTime << " second(s) too late at " << printDateTime(currTime) << std::endl;
      // unsigned skip = (unsigned)ceil(diffTime / gapSec);
      // go to next iteration
      timeinfo->tm_sec = timeinfo->tm_sec + (ceil(diffTime / gapSec) + 1) * gapSec;
      currTime = mktime(timeinfo);
      sleep(gapSec - (diffTime % gapSec));
      std::cout << std::endl;
    }
    else if (diffTime < 0) {
      sleep(-difftime(rawtime, currTime));  // sleep until the next iteration
    }
    if (params.verbose) {
      if (!inMarket) {
        std::cout << "[ " << printDateTime(currTime) << " ]" << std::endl;
      }
      else {
        std::cout << "[ " << printDateTime(currTime) << " IN MARKET: Long " << res.exchNameLong << " / Short " << res.exchNameShort << " ]" << std::endl;
      }
    }
    for (int e = 0; e < num_exchange; ++e) {
      double bid = getQuote[e](curl, true);
      double ask = getQuote[e](curl, false);
      if (bid == 0.0) {
        std::cout << "   WARNING: " << params.exchName[e] << " bid is null, use previous one" << std::endl;
      }
      if (ask == 0.0) {
        std::cout << "   WARNING: " << params.exchName[e] << " ask is null, use previous one" << std::endl;
      }
      if (params.verbose) {
        std::cout << "   " << params.exchName[e] << ": \t" << bid << " / " << ask << std::endl;
      }
      btcVec[e]->updateData(bid, ask, 0.0);
      curl_easy_reset(curl);
    }
    if (params.verbose) {
      std::cout << "   ----------------------------" << std::endl;
    }
    // compute entry point
    if (!inMarket) {
      for (int i = 0; i < num_exchange; ++i) {
        for (int j = 0; j < num_exchange; ++j) {
          if (i != j) {
            if (checkEntry(btcVec[i], btcVec[j], res, params)) {
              // entry opportunity found
              res.exposure = std::min(balanceUsd[res.idExchLong], balanceUsd[res.idExchShort]);  // compute exposure
              if (res.exposure == 0.0) {
                std::cout << "   WARNING: Opportunity found but no cash available. Trade canceled." << std::endl;
                break;
              }
              if (useFullCash == false && res.exposure <= cashForTesting) {
                std::cout << "   WARNING: Opportunity found but no enough cash. Need more than TEST cash (min. $" << cashForTesting << "). Trade canceled." << std::endl;
                break;
              }
              if (useFullCash) {
                res.exposure -= untouchedCash * res.exposure;  // leave untouchedCash
              } else {
                res.exposure = cashForTesting;  // use test money
              }
              double volumeLong = res.exposure / btcVec[res.idExchLong]->getAsk();
              double volumeShort = res.exposure / btcVec[res.idExchShort]->getBid();
              double limPriceLong = getLimitPrice[res.idExchLong](curl, volumeLong, false);
              double limPriceShort = getLimitPrice[res.idExchShort](curl, volumeShort, true);
              if (limPriceLong - res.priceLongIn > volPriceDelta || res.priceShortIn - limPriceShort > volPriceDelta) {
                std::cout << "   WARNING: Opportunity found but not enough volume. Trade canceled." << std::endl;
                std::cout << "            Target long price:  " << res.priceLongIn << ", Real long price:  " << limPriceLong << std::endl;
                std::cout << "            Target short price: " << res.priceShortIn << ", Real short price: " << limPriceShort << std::endl; 
                break;
              }
              inMarket = true;
              resultId++;
              // update result
              res.id = resultId;
              res.entryTime = currTime;
              res.printEntry();
              res.maxSpread[res.idExchLong][res.idExchShort] = -1.0;
              res.minSpread[res.idExchLong][res.idExchShort] = 1.0;
              int longOrderId = 0;
              int shortOrderId = 0;
              // send orders
              longOrderId = sendOrder[res.idExchLong](curl, params, "buy", volumeLong, btcVec[res.idExchLong]->getAsk());
              shortOrderId = sendOrder[res.idExchShort](curl, params, "sell", volumeShort, btcVec[res.idExchShort]->getBid());
              // wait for the orders to be filled
              std::cout << "Waiting for the two orders to be filled..." << std::endl;
              sleep(3.0);
              while (!isOrderComplete[res.idExchLong](curl, params, longOrderId) || !isOrderComplete[res.idExchShort](curl, params, shortOrderId)) {
                sleep(3.0);
              }
              std::cout << "Done" << std::endl;
              longOrderId = 0;
              shortOrderId = 0;
              break;
            }
          }
        }
        if (inMarket) {
          break;
        }
      }
      if (params.verbose) {
        std::cout << std::endl;
      }
    }
    // in market, looking to exit
    else if (inMarket) {
      if (checkExit(btcVec[res.idExchLong], btcVec[res.idExchShort], res, params, currTime)) {
        // exit opportunity found
        // check current exposure
        double *btcUsed = (double*)malloc(sizeof(double) * num_exchange);
        for (int i = 0; i < num_exchange; ++i) {
          btcUsed[i] = getActivePos[i](curl, params);
        }
        double volumeLong = btcUsed[res.idExchLong];
        double volumeShort = btcUsed[res.idExchShort];
        double limPriceLong = getLimitPrice[res.idExchLong](curl, volumeLong, true);
        double limPriceShort = getLimitPrice[res.idExchShort](curl, volumeShort, false);

        if (res.priceLongOut - limPriceLong > volPriceDelta || limPriceShort - res.priceShortOut > volPriceDelta) {
          std::cout << "   WARNING: Opportunity found but not enough volume. Trade canceled." << std::endl;
          std::cout << "            Target long price:  " << res.priceLongOut << ", Real long price:  " << limPriceLong << std::endl;
          std::cout << "            Target short price: " << res.priceShortOut << ", Real short price: " << limPriceShort << std::endl; 
        }
        else {
          res.exitTime = currTime;
          res.printExit();
          int longOrderId = 0;
          int shortOrderId = 0;
          std::cout << std::setprecision(6) << "BTC exposure on " << params.exchName[res.idExchLong] << ": " << volumeLong << std::setprecision(2) << std::endl;
          std::cout << std::setprecision(6) << "BTC exposure on " << params.exchName[res.idExchShort] << ": " << volumeShort << std::setprecision(2) << std::endl;
          std::cout << std::endl;
          // send orders
          longOrderId = sendOrder[res.idExchLong](curl, params, "sell", fabs(btcUsed[res.idExchLong]), btcVec[res.idExchLong]->getBid());
          shortOrderId = sendOrder[res.idExchShort](curl, params, "buy", fabs(btcUsed[res.idExchShort]), btcVec[res.idExchShort]->getAsk());
          // wait for the orders to be filled
          std::cout << "Waiting for the two orders to be filled..." << std::endl;
          sleep(3.0);
          while (!isOrderComplete[res.idExchLong](curl, params, longOrderId) || !isOrderComplete[res.idExchShort](curl, params, shortOrderId)) {
            sleep(3.0);
          }
          std::cout << "Done\n" << std::endl;
          longOrderId = 0;
          shortOrderId = 0;
          inMarket = false;
          // new balances
          for (int i = 0; i < num_exchange; ++i) {
            newBalUsd[i] = getAvail[i](curl, params, "usd");
            newBalBtc[i] = getAvail[i](curl, params, "btc");
          }
          for (int i = 0; i < num_exchange; ++i) {
            std::cout << "New balance on " << params.exchName[i] << ":  \t";
            std::cout << newBalUsd[i] << " USD (perf $" << newBalUsd[i] - balanceUsd[i] << "), ";
            std::cout << std::setprecision(6) << newBalBtc[i]  << std::setprecision(2) << " BTC" << std::endl;
          }
          std::cout << std::endl;
          // update res with total balance
          for (int i = 0; i < num_exchange; ++i) {
            res.befBalUsd += balanceUsd[i];
            res.aftBalUsd += newBalUsd[i];
          }
          // update current balances with new values
          for (int i = 0; i < num_exchange; ++i) {
            balanceUsd[i] = newBalUsd[i];
            balanceBtc[i] = newBalBtc[i];
          }
          std::cout << "ACTUAL PERFORMANCE: " << "$" << res.aftBalUsd - res.befBalUsd << " (" << res.totPerf() * 100.0 << "%)\n" << std::endl;
          csvFile << res.id << "," << res.exchNameLong << "," << res.exchNameShort << "," << printDateTimeCsv(res.entryTime) << "," << printDateTimeCsv(res.exitTime);
          csvFile << "," << res.getLength() << "," << res.exposure * 2.0 << "," << res.befBalUsd << "," << res.aftBalUsd << "," << res.totPerf() << "\n";
          csvFile.flush();
          if (params.sendEmail) {
            sendEmail(res, params);
            std::cout << "Email sent" << std::endl;
          }
          res.clear();
          std::ifstream infile("stop_after_exit");
          if (infile.good()) {
            std::cout << "Exit after last trade (file stop_after_exit found)" << std::endl;
            stillRunning = false;
          }
        }
      }
      if (params.verbose) {
        std::cout << std::endl;
      }
    }
    timeinfo->tm_sec = timeinfo->tm_sec + gapSec;
    currIteration++;
    if (currIteration >= debugMaxIteration) {
      std::cout << "Max iteration reached (" << debugMaxIteration << ")" <<std::endl;
      stillRunning = false;
    }
  }
  for (int i = 0; i < num_exchange; ++i) {
    delete(btcVec[i]);
  }
  json_decref(root);
  curl_easy_cleanup(curl);
  curl_global_cleanup();
  csvFile.close();

  return 0;
}
