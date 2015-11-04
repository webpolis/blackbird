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
  double limit = 2.0 * btcLong->getFees() + 2.0 * btcShort->getFees() + params.spreadEntry;
  if (btcShort->getHasShort()) {
    // btcLong:  buy looking to match ask
    // btcShort: sell looking to match bid
    double priceLong = btcLong->getAsk();
    double priceShort = btcShort->getBid();
    if (priceLong > 0.0 && priceShort > 0.0) {
      res.spreadIn = (priceShort - priceLong) / priceLong;
    } else {
      res.spreadIn = 0.0;
    }
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
      std::cout << " [target " << percToStr(limit) << ", min " << percToStr(res.minSpread[longId][shortId]) << ", max " << percToStr(res.maxSpread[longId][shortId]) << "]";

      if (res.trailing[longId][shortId] != -1.0) {
        std::cout << "   (trailing " << res.trailing[longId][shortId] * 100.0 << "%)";
      }
      if (btcLong->getIsImplemented() == false || btcShort->getIsImplemented() == false) {
        std::cout << "   (info only)"  << std::endl;
      } else {
        std::cout << std::endl;
      }
    }

    if (btcLong->getIsImplemented() == true) {
      if (btcShort->getIsImplemented() == true) {
        if (priceLong > 0.0) {
          if (priceShort > 0.0) {
            if (res.spreadIn >= limit) {
              double newTrailValue = res.spreadIn - params.trailingLim;
              if (res.trailing[longId][shortId] == -1.0) {
                if (newTrailValue > limit) {
                  res.trailing[longId][shortId] = newTrailValue;
                } else {
                  res.trailing[longId][shortId] = limit;
                }
              } else {
                if (newTrailValue >= res.trailing[longId][shortId]) {
                  res.trailing[longId][shortId] = newTrailValue;
                }
                if (res.spreadIn < res.trailing[longId][shortId]) {
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
            } else {
              res.trailing[longId][shortId] = -1.0;
            }
          }
        }
      }
    }
  }
  return false;
}


bool checkExit(Bitcoin *btcLong, Bitcoin *btcShort, Result &res, Parameters params, time_t period) {
  // close btcLong:  sell looking to match bid
  // close btcShort: buy looking to match ask
  double priceLong  = btcLong->getBid();
  double priceShort = btcShort->getAsk();
  if (priceLong > 0.0 && priceShort > 0.0) {
    res.spreadOut = (priceShort - priceLong) / priceLong;
  } else {
    res.spreadOut = 0.0;
  }
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
    std::cout << " [target " << percToStr(params.spreadExit) << ", min " << percToStr(res.minSpread[longId][shortId]) << ", max " << percToStr(res.maxSpread[longId][shortId]) << "]";
    if (res.trailing[longId][shortId] != 1.0) {
      std::cout << "   (trailing " << res.trailing[longId][shortId] * 100.0 << "%)";
    }
  }
  std::cout << std::endl;
  if (period - res.entryTime >= params.maxLength) {
    res.priceLongOut  = priceLong;
    res.priceShortOut = priceShort;
    return true;
  }
  if (priceLong > 0.0) {
    if (priceShort > 0.0) {
      if (res.spreadOut <= params.spreadExit) {
        double newTrailValue = res.spreadOut + params.trailingLim;
        if (res.trailing[longId][shortId] == 1.0) {
          if (newTrailValue < params.spreadExit) {
            res.trailing[longId][shortId] = newTrailValue;
          } else {
            res.trailing[longId][shortId] = params.spreadExit;
          }
        } else {
          if (newTrailValue <= res.trailing[longId][shortId]) {
            res.trailing[longId][shortId] = newTrailValue;
          }
          if (res.spreadOut > res.trailing[longId][shortId]) {
            res.priceLongOut  = priceLong;
            res.priceShortOut = priceShort;
            return true;
          }
        }
      } else {
        res.trailing[longId][shortId] = 1.0;
      }
    }
  }
  return false;
}
