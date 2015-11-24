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

typedef double (*getQuoteType) (Parameters& params, bool isBid);
typedef double (*getAvailType) (Parameters& params, std::string currency);
typedef int (*sendOrderType) (Parameters& params, std::string direction, double quantity, double price);
typedef bool (*isOrderCompleteType) (Parameters& params, int orderId);
typedef double (*getActivePosType) (Parameters& params);
typedef double (*getLimitPriceType) (Parameters& params, double volume, bool isBid);

int main(int argc, char** argv) {
  std::cout << "Blackbird Bitcoin Arbitrage" << std::endl;
  std::cout << "DISCLAIMER: USE THE SOFTWARE AT YOUR OWN RISK\n" << std::endl;

  // read the config file (config.json)
  json_error_t error;
  json_t* root= json_load_file("config.json", 0, &error);
  if (!root) {
    std::cout << "ERROR: config.json incorrect (" << error.text << ")\n" << std::endl;
    return -1;
  }

  if (!json_object_get(root, "SpreadTarget")) {
    std::cout << "ERROR: config.json is too old. Please update your file with the latest version\n" << std::endl;
    return -1;
  }

  int gapSec = json_integer_value(json_object_get(root, "GapSec"));
  unsigned debugMaxIteration = json_integer_value(json_object_get(root, "DebugMaxIteration"));
  bool useFullCash = json_boolean_value(json_object_get(root, "UseFullCash"));
  double untouchedCash = json_real_value(json_object_get(root, "UntouchedCash"));
  double cashForTesting = json_real_value(json_object_get(root, "CashForTesting"));
  double maxExposure = json_real_value(json_object_get(root, "MaxExposure"));
  bool infoOnly = json_boolean_value(json_object_get(root, "infoOnly"));

  std::locale mylocale("");

  if (!infoOnly) {
    if (!useFullCash) {
      if (cashForTesting < 10.0) {
        std::cout << "WARNING: Minimum test cash recommended: $10.00\n" << std::endl;
      }
      if (cashForTesting > maxExposure) {
        std::cout << "ERROR: Test cash ($" << cashForTesting << ") is above max exposure ($" << maxExposure << ")\n" << std::endl;
        return -1;
      }
    }
  }

  // function arrays
  getQuoteType getQuote[10];
  getAvailType getAvail[10];
  sendOrderType sendOrder[10];
  isOrderCompleteType isOrderComplete[10];
  getActivePosType getActivePos[10];
  getLimitPriceType getLimitPrice[10];

 // create a parameters structure
  Parameters params(root);
  int index = 0;
  std::string tmp;
  tmp = json_string_value(json_object_get(root, "BitfinexApiHead"));
  if (tmp.empty() == false || infoOnly == true) {
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
  if (tmp.empty() == false || infoOnly == true) {
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
  if (tmp.empty() == false || infoOnly == true) {
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
  if (tmp.empty() == false || infoOnly == true) {
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
  if (tmp.empty() == false || infoOnly == true) {
    params.addExchange("Kraken", json_real_value(json_object_get(root, "KrakenFees")), json_boolean_value(json_object_get(root, "KrakenCanShort")), true);
    getQuote[index] = Kraken::getQuote;
    getAvail[index] = Kraken::getAvail;
    sendOrder[index] = Kraken::sendOrder;
    isOrderComplete[index] = Kraken::isOrderComplete;
    getActivePos[index] = Kraken::getActivePos;
    getLimitPrice[index] = Kraken::getLimitPrice;
    index++;
  }
  tmp = json_string_value(json_object_get(root, "ItBitApiKey"));
  if (tmp.empty() == false || infoOnly == true) {
    params.addExchange("ItBit", json_real_value(json_object_get(root, "ItBitFees")), json_boolean_value(json_object_get(root, "ItBitCanShort")), false);
    getQuote[index] = ItBit::getQuote;
    getAvail[index] = ItBit::getAvail;
    // sendOrder[index] = ItBit::sendOrder;
    // isOrderComplete[index] = ItBit::isOrderComplete;
    getActivePos[index] = ItBit::getActivePos;
    getLimitPrice[index] = ItBit::getLimitPrice;
    index++;
  }

  if (index < 2) {
    std::cout << "ERROR: Blackbird needs at least two Bitcoin exchanges. Please edit the config.json file to add new exchanges\n" << std::endl;
    return -1;
  }

  std::string currDateTime = printDateTimeFileName();
  std::string csvFileName = "blackbird_result_" + currDateTime + ".csv";
  std::ofstream csvFile;
  csvFile.open(csvFileName.c_str(), std::ofstream::trunc);
  csvFile << "TRADE_ID,EXCHANGE_LONG,EXHANGE_SHORT,ENTRY_TIME,EXIT_TIME,DURATION,TOTAL_EXPOSURE,BALANCE_BEFORE,BALANCE_AFTER,RETURN\n";
  csvFile.flush();

  std::string logFileName = "blackbird_log_" + currDateTime + ".log";
  std::ofstream logFile;
  logFile.open(logFileName.c_str(), std::ofstream::trunc);
  logFile.imbue(mylocale);
  logFile.precision(2);
  logFile << std::fixed;
  params.logFile = &logFile;
  logFile << "--------------------------------------------" << std::endl;
  logFile << "|   Blackbird Bitcoin Arbitrage Log File   |" << std::endl;
  logFile << "--------------------------------------------\n" << std::endl;
  logFile << "Blackbird started on " << printDateTime() << "\n" << std::endl;

  if (infoOnly) {
    logFile << "Demo mode: trades won't be generated\n" << std::endl;
  }

  std::cout << "Log file generated: " << logFileName << "\nBlackbird is running... (pid " << getpid() << ")\n" << std::endl;

  // Vector of Bitcoin objects
  std::vector<Bitcoin*> btcVec;
  int num_exchange = params.nbExch();
  // create Bitcoin objects
  for (int i = 0; i < num_exchange; ++i) {
    btcVec.push_back(new Bitcoin(i, params.exchName[i], params.fees[i], params.hasShort[i], params.isImplemented[i]));
  }
  curl_global_init(CURL_GLOBAL_ALL);
  params.curl = curl_easy_init();

  logFile << "[ Targets ]" << std::endl;
  logFile << "   Spread Entry:  " << params.spreadEntry * 100.0 << "%" << std::endl;
  logFile << "   Spread Target: " << params.spreadTarget * 100.0  << "%" << std::endl;
  if (params.spreadEntry <= 0.0) {
    logFile << "   WARNING: Spread Entry should be positive" << std::endl;
  }
  if (params.spreadTarget <= 0.0) {
    logFile << "   WARNING: Spread Target should be positive" << std::endl;
  }
  logFile << std::endl;
  // store current balances
  logFile << "[ Current balances ]" << std::endl;
  double* balanceUsd = (double*)malloc(sizeof(double) * num_exchange);
  double* balanceBtc = (double*)malloc(sizeof(double) * num_exchange);
  for (int i = 0; i < num_exchange; ++i) {
    if (infoOnly) {
      balanceUsd[i] = 0.0;
      balanceBtc[i] = 0.0;
    } else {
      balanceUsd[i] = getAvail[i](params, "usd");
      balanceBtc[i] = getAvail[i](params, "btc");
    }
  }
  // contains balances after a completed trade
  double* newBalUsd = (double*)malloc(sizeof(double) * num_exchange);
  double* newBalBtc = (double*)malloc(sizeof(double) * num_exchange);
  memset(newBalUsd, 0.0, sizeof(double) * num_exchange);
  memset(newBalBtc, 0.0, sizeof(double) * num_exchange);

  for (int i = 0; i < num_exchange; ++i) {
    logFile << "   " << params.exchName[i] << ":\t";
    logFile << balanceUsd[i] << " USD\t" << std::setprecision(6) << balanceBtc[i]  << std::setprecision(2) << " BTC" << std::endl;
    if (balanceBtc[i] > 0.0300) {
      logFile << "ERROR: All BTC accounts must be empty before starting Blackbird" << std::endl;
      return -1;
    }
  }
  logFile << std::endl;
  logFile << "[ Cash exposure ]" << std::endl;
  if (infoOnly) {
    logFile << "   No cash - Demo mode" << std::endl;
  } else {
    if (useFullCash) {
      logFile << "   FULL cash used!" << std::endl;
    } else {
      logFile << "   TEST cash used\n   Value: $" << cashForTesting << std::endl;
    }
  }
  logFile << std::endl;

  time_t rawtime;
  rawtime = time(NULL);
  struct tm* timeinfo;
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
    logFile << "Running..." << std::endl;
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
      logFile << "WARNING: " << diffTime << " second(s) too late at " << printDateTime(currTime) << std::endl;
      // unsigned skip = (unsigned)ceil(diffTime / gapSec);
      // go to next iteration
      timeinfo->tm_sec = timeinfo->tm_sec + (ceil(diffTime / gapSec) + 1) * gapSec;
      currTime = mktime(timeinfo);
      sleep(gapSec - (diffTime % gapSec));
      logFile << std::endl;
    }
    else if (diffTime < 0) {
      sleep(-difftime(rawtime, currTime));  // sleep until the next iteration
    }
    if (params.verbose) {
      if (!inMarket) {
        logFile << "[ " << printDateTime(currTime) << " ]" << std::endl;
      } else {
        logFile << "[ " << printDateTime(currTime) << " IN MARKET: Long " << res.exchNameLong << " / Short " << res.exchNameShort << " ]" << std::endl;
      }
    }
    for (int e = 0; e < num_exchange; ++e) {
      double bid = getQuote[e](params, true);
      double ask = getQuote[e](params, false);
      if (bid == 0.0) {
        logFile << "   WARNING: " << params.exchName[e] << " bid is null, use previous one" << std::endl;
      }
      if (ask == 0.0) {
        logFile << "   WARNING: " << params.exchName[e] << " ask is null, use previous one" << std::endl;
      }
      if (params.verbose) {
        logFile << "   " << params.exchName[e] << ": \t" << bid << " / " << ask << std::endl;
      }
      btcVec[e]->updateData(bid, ask, 0.0);
      curl_easy_reset(params.curl);
    }
    if (params.verbose) {
      logFile << "   ----------------------------" << std::endl;
    }
      
    // compute entry point
    if (!inMarket) {
      for (int i = 0; i < num_exchange; ++i) {
        for (int j = 0; j < num_exchange; ++j) {
          if (i != j) {
            if (checkEntry(btcVec[i], btcVec[j], res, params)) {
              // entry opportunity found
              res.exposure = std::min(balanceUsd[res.idExchLong], balanceUsd[res.idExchShort]);
              if (infoOnly) {
                logFile << "   INFO: Opportunity found but no trade will be generated (Demo mode)" << std::endl;
                break;
              }
              if (res.exposure == 0.0) {
                logFile << "   WARNING: Opportunity found but no cash available. Trade canceled" << std::endl;
                break;
              }
              if (useFullCash == false && res.exposure <= cashForTesting) {
                logFile << "   WARNING: Opportunity found but no enough cash. Need more than TEST cash (min. $" << cashForTesting << "). Trade canceled" << std::endl;
                break;
              }
              if (useFullCash) {
                res.exposure -= untouchedCash * res.exposure;  // leave untouchedCash
                if (res.exposure > maxExposure) {
                  logFile << "   WARNING: Opportunity found but exposure ($" << res.exposure << ") above the limit" << std::endl;
                  logFile << "            Max exposure will be used instead ($" << maxExposure << ")" << std::endl;
                  res.exposure = maxExposure;
                }
              } else {
                res.exposure = cashForTesting;  // use test money
              }
              double volumeLong = res.exposure / btcVec[res.idExchLong]->getAsk();
              double volumeShort = res.exposure / btcVec[res.idExchShort]->getBid();
              double limPriceLong = getLimitPrice[res.idExchLong](params, volumeLong, false);
              double limPriceShort = getLimitPrice[res.idExchShort](params, volumeShort, true);
              if (limPriceLong - res.priceLongIn > params.priceDeltaLim || res.priceShortIn - limPriceShort > params.priceDeltaLim) {
                logFile << "   WARNING: Opportunity found but not enough liquidity. Trade canceled" << std::endl;
                logFile << "            Target long price:  " << res.priceLongIn << ", Real long price:  " << limPriceLong << std::endl;
                logFile << "            Target short price: " << res.priceShortIn << ", Real short price: " << limPriceShort << std::endl;
                res.trailing[res.idExchLong][res.idExchShort] = -1.0;
                break;
              }
              inMarket = true;
              resultId++;
              // update result
              res.id = resultId;
              res.entryTime = currTime;
              res.printEntry(*params.logFile);
              res.maxSpread[res.idExchLong][res.idExchShort] = -1.0;
              res.minSpread[res.idExchLong][res.idExchShort] = 1.0;
              res.trailing[res.idExchLong][res.idExchShort] = 1.0;
              int longOrderId = 0;
              int shortOrderId = 0;
              // send orders
              longOrderId = sendOrder[res.idExchLong](params, "buy", volumeLong, btcVec[res.idExchLong]->getAsk());
              shortOrderId = sendOrder[res.idExchShort](params, "sell", volumeShort, btcVec[res.idExchShort]->getBid());
              // wait for the orders to be filled
              logFile << "Waiting for the two orders to be filled..." << std::endl;
              sleep(3.0);
              while (!isOrderComplete[res.idExchLong](params, longOrderId) || !isOrderComplete[res.idExchShort](params, shortOrderId)) {
                sleep(3.0);
              }
              logFile << "Done" << std::endl;
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
        logFile << std::endl;
      }
    }
    // in market, looking to exit
    else if (inMarket) {
      if (checkExit(btcVec[res.idExchLong], btcVec[res.idExchShort], res, params, currTime)) {
        // exit opportunity found
        // check current exposure
        double* btcUsed = (double*)malloc(sizeof(double) * num_exchange);
        for (int i = 0; i < num_exchange; ++i) {
          btcUsed[i] = getActivePos[i](params);
        }
        double volumeLong = btcUsed[res.idExchLong];
        double volumeShort = btcUsed[res.idExchShort];
        double limPriceLong = getLimitPrice[res.idExchLong](params, volumeLong, true);
        double limPriceShort = getLimitPrice[res.idExchShort](params, volumeShort, false);

        if (res.priceLongOut - limPriceLong > params.priceDeltaLim || limPriceShort - res.priceShortOut > params.priceDeltaLim) {
          logFile << "   WARNING: Opportunity found but not enough liquidity. Trade canceled" << std::endl;
          logFile << "            Target long price:  " << res.priceLongOut << ", Real long price:  " << limPriceLong << std::endl;
          logFile << "            Target short price: " << res.priceShortOut << ", Real short price: " << limPriceShort << std::endl;
          res.trailing[res.idExchLong][res.idExchShort] = 1.0;
        } else {
          res.exitTime = currTime;
          res.printExit(*params.logFile);
          int longOrderId = 0;
          int shortOrderId = 0;
          logFile << std::setprecision(6) << "BTC exposure on " << params.exchName[res.idExchLong] << ": " << volumeLong << std::setprecision(2) << std::endl;
          logFile << std::setprecision(6) << "BTC exposure on " << params.exchName[res.idExchShort] << ": " << volumeShort << std::setprecision(2) << std::endl;
          logFile << std::endl;
          // send orders
          longOrderId = sendOrder[res.idExchLong](params, "sell", fabs(btcUsed[res.idExchLong]), btcVec[res.idExchLong]->getBid());
          shortOrderId = sendOrder[res.idExchShort](params, "buy", fabs(btcUsed[res.idExchShort]), btcVec[res.idExchShort]->getAsk());
          // wait for the orders to be filled
          logFile << "Waiting for the two orders to be filled..." << std::endl;
          sleep(3.0);
          while (!isOrderComplete[res.idExchLong](params, longOrderId) || !isOrderComplete[res.idExchShort](params, shortOrderId)) {
            sleep(3.0);
          }
          logFile << "Done\n" << std::endl;
          longOrderId = 0;
          shortOrderId = 0;
          inMarket = false;
          // new balances
          for (int i = 0; i < num_exchange; ++i) {
            newBalUsd[i] = getAvail[i](params, "usd");
            newBalBtc[i] = getAvail[i](params, "btc");
          }
          for (int i = 0; i < num_exchange; ++i) {
            logFile << "New balance on " << params.exchName[i] << ":  \t";
            logFile << newBalUsd[i] << " USD (perf $" << newBalUsd[i] - balanceUsd[i] << "), ";
            logFile << std::setprecision(6) << newBalBtc[i]  << std::setprecision(2) << " BTC" << std::endl;
          }
          logFile << std::endl;
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
          logFile << "ACTUAL PERFORMANCE: " << "$" << res.aftBalUsd - res.befBalUsd << " (" << res.totPerf() * 100.0 << "%)\n" << std::endl;
          csvFile << res.id << "," << res.exchNameLong << "," << res.exchNameShort << "," << printDateTimeCsv(res.entryTime) << "," << printDateTimeCsv(res.exitTime);
          csvFile << "," << res.getLength() << "," << res.exposure * 2.0 << "," << res.befBalUsd << "," << res.aftBalUsd << "," << res.totPerf() << "\n";
          csvFile.flush();
          if (params.sendEmail) {
            sendEmail(res, params);
            logFile << "Email sent" << std::endl;
          }
          res.clear();
          std::ifstream infile("stop_after_exit");
          if (infile.good()) {
            logFile << "Exit after last trade (file stop_after_exit found)" << std::endl;
            stillRunning = false;
          }
        }
      }
      if (params.verbose) {
        logFile << std::endl;
      }
    }
    timeinfo->tm_sec = timeinfo->tm_sec + gapSec;
    currIteration++;
    if (currIteration >= debugMaxIteration) {
      logFile << "Max iteration reached (" << debugMaxIteration << ")" <<std::endl;
      stillRunning = false;
    }
  }
  for (int i = 0; i < num_exchange; ++i) {
    delete(btcVec[i]);
  }
  json_decref(root);
  curl_easy_cleanup(params.curl);
  curl_global_cleanup();
  csvFile.close();
  logFile.close();

  return 0;
}
