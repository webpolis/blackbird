#include <sstream>
#include <stdlib.h>
#include <iomanip>
#include <numeric>
#include <cmath>
#include "check_entry_exit.h"

std::string percToStr(double perc) {
  std::ostringstream s;
  if (perc < 0.0) {
    s << std::fixed << std::setprecision(2) << perc * 100.0 << "%";
  } else {
    s << " " << std::fixed << std::setprecision(2) << perc * 100.0 << "%";
  }
  return s.str();
}

bool checkEntry(Bitcoin* btcLong, Bitcoin* btcShort, Result& res, Parameters& params) {
  if (btcShort->getHasShort()) {
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
      *params.logFile << "   " << btcLong->getExchName() << "/" << btcShort->getExchName() << ":\t" << percToStr(res.spreadIn);
      *params.logFile << " [target " << percToStr(params.spreadEntry) << ", min " << percToStr(res.minSpread[longId][shortId]) << ", max " << percToStr(res.maxSpread[longId][shortId]) << "]";
      if (params.useVolatility) {
        if (res.volatility[longId][shortId].size() >= params.volatilityPeriod) {
          double sum = std::accumulate(res.volatility[longId][shortId].begin(), res.volatility[longId][shortId].end(), 0.0);
          double mean = sum / res.volatility[longId][shortId].size();
          double squareSum = std::inner_product(res.volatility[longId][shortId].begin(), res.volatility[longId][shortId].end(), res.volatility[longId][shortId].begin(), 0.0);
          double stdev = std::sqrt(squareSum / res.volatility[longId][shortId].size() - mean * mean);
          *params.logFile << "  volat. " << stdev * 100.0 << "%";
        } else {
          *params.logFile << "  volat. n/a " << res.volatility[longId][shortId].size() << "<" << params.volatilityPeriod << " ";
        }
      }
      if (res.trailing[longId][shortId] != -1.0) {
        *params.logFile << "   trailing " << percToStr(res.trailing[longId][shortId]) << "  " << res.trailingWaitCount[longId][shortId] << "/" << params.trailingCount;
      }
      if ((btcLong->getIsImplemented() == false || btcShort->getIsImplemented() == false) && params.demoMode == false) {
        *params.logFile << "   info only"  << std::endl;
      } else {
        *params.logFile << std::endl;
      }
    }
    if (btcLong->getIsImplemented() == true) {
      if (btcShort->getIsImplemented() == true) {
        if (priceLong > 0.0) {
          if (priceShort > 0.0) {
            if (res.spreadIn >= params.spreadEntry) {
              double newTrailValue = res.spreadIn - params.trailingLim;
              if (res.trailing[longId][shortId] == -1.0) {
                if (newTrailValue > params.spreadEntry) {
                  res.trailing[longId][shortId] = newTrailValue;
                } else {
                  res.trailing[longId][shortId] = params.spreadEntry;
                }
              } else {
                if (newTrailValue >= res.trailing[longId][shortId]) {
                  res.trailing[longId][shortId] = newTrailValue;
                  res.trailingWaitCount[longId][shortId] = 0;
                }
                if (res.spreadIn < res.trailing[longId][shortId]) {
                  if (res.trailingWaitCount[longId][shortId] < params.trailingCount) {
                    res.trailingWaitCount[longId][shortId]++;
                  } else {
                    res.idExchLong = longId;
                    res.idExchShort = shortId;
                    res.feesLong = btcLong->getFees();
                    res.feesShort = btcShort->getFees();
                    res.exchNameLong = btcLong->getExchName();
                    res.exchNameShort = btcShort->getExchName();
                    res.priceLongIn = priceLong;
                    res.priceShortIn = priceShort;
                    res.exitTarget = res.spreadIn - params.spreadTarget - (2.0 * btcLong->getFees() + 2.0 * btcShort->getFees());
                    res.trailingWaitCount[longId][shortId] = 0;
                    return true;
                  }
                } else {
                  res.trailingWaitCount[longId][shortId] = 0;
                }
              }
            } else {
              res.trailing[longId][shortId] = -1.0;
              res.trailingWaitCount[longId][shortId] = 0;
            }
          }
        }
      }
    }
  }
  return false;
}

bool checkExit(Bitcoin* btcLong, Bitcoin* btcShort, Result& res, Parameters& params, time_t period) {
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
    *params.logFile << "   " << btcLong->getExchName() << "/" << btcShort->getExchName() << ":\t" << percToStr(res.spreadOut);
    *params.logFile << " [target " << percToStr(res.exitTarget) << ", min " << percToStr(res.minSpread[longId][shortId]) << ", max " << percToStr(res.maxSpread[longId][shortId]) << "]";
    if (params.useVolatility) {
      if (res.volatility[longId][shortId].size() >= params.volatilityPeriod) {
        double sum = std::accumulate(res.volatility[longId][shortId].begin(), res.volatility[longId][shortId].end(), 0.0);
        double mean = sum / res.volatility[longId][shortId].size();
        double squareSum = std::inner_product(res.volatility[longId][shortId].begin(), res.volatility[longId][shortId].end(), res.volatility[longId][shortId].begin(), 0.0);
        double stdev = std::sqrt(squareSum / res.volatility[longId][shortId].size() - mean * mean);
        *params.logFile << "  volat. " << stdev * 100.0 << "%";
      } else {
        *params.logFile << "  volat. n/a " << res.volatility[longId][shortId].size() << "<" << params.volatilityPeriod << " ";
      }
    }
    if (res.trailing[longId][shortId] != 1.0) {
      *params.logFile << "   trailing " << percToStr(res.trailing[longId][shortId]) << "  " << res.trailingWaitCount[longId][shortId] << "/" << params.trailingCount;
    }
  }
  *params.logFile << std::endl;
  if (period - res.entryTime >= int(params.maxLength)) {
    res.priceLongOut  = priceLong;
    res.priceShortOut = priceShort;
    return true;
  }
  if (priceLong > 0.0) {
    if (priceShort > 0.0) {
      if (res.spreadOut <= res.exitTarget) {
        double newTrailValue = res.spreadOut + params.trailingLim;
        if (res.trailing[longId][shortId] == 1.0) {
          if (newTrailValue < res.exitTarget) {
            res.trailing[longId][shortId] = newTrailValue;
          } else {
            res.trailing[longId][shortId] = res.exitTarget;
          }
        } else {
          if (newTrailValue <= res.trailing[longId][shortId]) {
            res.trailing[longId][shortId] = newTrailValue;
            res.trailingWaitCount[longId][shortId] = 0;
          }
          if (res.spreadOut > res.trailing[longId][shortId]) {
            if (res.trailingWaitCount[longId][shortId] < params.trailingCount) {
              res.trailingWaitCount[longId][shortId]++;
            } else {
              res.priceLongOut  = priceLong;
              res.priceShortOut = priceShort;
              res.trailingWaitCount[longId][shortId] = 0;
              return true;
            }
          } else {
            res.trailingWaitCount[longId][shortId] = 0;
          }
        }
      } else {
        res.trailing[longId][shortId] = 1.0;
        res.trailingWaitCount[longId][shortId] = 0;
      }
    }
  }
  return false;
}

