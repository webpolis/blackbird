#include <sstream>
#include <stdlib.h>
#include <iomanip>
#include "check_entry_exit.h"

std::string percToStr(double perc) {

  std::ostringstream s;
  if (perc < 0.0) {
    s << std::fixed << std::setprecision(2) <<  perc * 100.0 << "%";
  } else {
    s << " " << std::fixed << std::setprecision(2) << perc * 100.0 << "%";
  }
  return s.str();
}


bool checkEntry(Bitcoin *btcLong, Bitcoin *btcShort, Result &res, Parameters params) {
  // compute limit (fees + spreadEntry)
  double limit = 2.0 * btcLong->getFees() + 2.0 * btcShort->getFees() + params.spreadEntry;
  // check if btcLong is below btcShort
  if (btcShort->getHasShort()) {
    // btcLong:  buy looking to match ask
    // btcShort: sell looking to match bid
    double priceLong = btcLong->getAsk();
    double priceShort = btcShort->getBid();
    res.spreadIn = (priceShort - priceLong) / priceLong;
    // maxSpread and minSpread
    int longId = btcLong->getId();
    int shortId = btcShort->getId();
    if (res.spreadIn > res.maxSpread[longId][shortId]) {
      res.maxSpread[longId][shortId] = res.spreadIn;
    }
    if (res.spreadIn < res.minSpread[longId][shortId]) {
      res.minSpread[longId][shortId] = res.spreadIn;
    }
    if (params.verbose) {
      std::cout << "   " << btcLong->getExchName() << "/" << btcShort->getExchName() << ":\t" << percToStr(res.spreadIn);
      std::cout << " [target " << percToStr(limit) << ", min " << percToStr(res.minSpread[longId][shortId]) << ", max " << percToStr(res.maxSpread[longId][shortId]) << "]" << std::endl;
    }
    // TODO Kraken (id 3) and ItBit (id 4) are not ready to be traded on
    if (res.spreadIn >= limit) {
      if (longId != 3) {
        if (longId != 4) {
          if (shortId != 3) {
            if (shortId != 4) {
              if (priceLong > 0.0) {
                if (priceShort > 0.0) {
                  // opportunity found
                  res.idExchLong = longId;
                  res.idExchShort = shortId;
                  res.feesLong = btcLong->getFees();
                  res.feesShort = btcShort->getFees();
                  res.exchNameLong = btcLong->getExchName();
                  res.exchNameShort = btcShort->getExchName();
                  res.priceLongIn = priceLong;
                  res.priceShortIn = priceShort;
                  return true;
                }
              }
            }
          }
        }
      }
    }
  }
  return false;  // no opportunity
}


bool checkExit(Bitcoin *btcLong, Bitcoin *btcShort, Result &res, Parameters params, time_t period) {
  // close btcLong:  sell looking to match bid
  // close btcShort: buy looking to match ask
  double priceLong  = btcLong->getBid();
  double priceShort = btcShort->getAsk();
  res.spreadOut = (priceShort - priceLong) / priceLong;
  // maxSpread and minSpread
  int longId = btcLong->getId();
  int shortId = btcShort->getId();
  if (res.spreadOut > res.maxSpread[longId][shortId]) {
    res.maxSpread[longId][shortId] = res.spreadOut;
  }
  if (res.spreadOut < res.minSpread[longId][shortId]) {
    res.minSpread[longId][shortId] = res.spreadOut;
  }
  if (params.verbose) {
    std::cout << "   " << btcLong->getExchName() << "/" << btcShort->getExchName() << ":\t" << percToStr(res.spreadOut);
    std::cout << " [target " << percToStr(params.spreadExit) << ", min " << percToStr(res.minSpread[longId][shortId]) << ", max " << percToStr(res.maxSpread[longId][shortId]) << "]" << std::endl;
  }
  // check length
  if (period - res.entryTime >= params.maxLength) {
    res.priceLongOut  = priceLong;
    res.priceShortOut = priceShort;
    return true;
  }
  if (res.spreadOut <= params.spreadExit && priceLong > 0.0 && priceShort > 0.0) {
    // exit opportunity found
    res.priceLongOut  = priceLong;
    res.priceShortOut = priceShort;
    return true;
  }
  return false;  // no exit opportunity
}
