#include <iostream>
#include <numeric>
#include "result.h"
#include "time_fun.h"

double Result::targetPerfLong() {
  return (priceLongOut - priceLongIn) / priceLongIn - 2.0 * feesLong;
}

double Result::targetPerfShort() {
  return (priceShortIn - priceShortOut) / priceShortIn - 2.0 * feesShort;
}

double Result::actualPerf() {
  if (exposure == 0.0) {
    return 0.0;
  } else {
    return (usdTotBalanceAfter - usdTotBalanceBefore) / (exposure * 2.0);
  }
}

double Result::getTradeLengthInMinute() {
  if (entryTime > 0 && exitTime > 0) {
    return ((double)(exitTime - entryTime)) / 60.0;
  } else {
    return 0;
  }
}

void Result::printEntryInfo(std::ofstream& logFile) {
  logFile << "\n[ ENTRY FOUND ]" << std::endl;
  logFile << "   Date & Time:       "  << printDateTime(entryTime) << std::endl;
  logFile << "   Exchange Long:     "  << exchNameLong <<  " (id " << idExchLong  << ")" << std::endl;
  logFile << "   Exchange Short:    "  << exchNameShort << " (id " << idExchShort << ")" << std::endl;
  logFile << "   Fees:              "  << feesLong * 100.0 << "% / " << feesShort * 100.0 << "%" << std::endl;
  logFile << "   Price Long:        $" << priceLongIn << " (target)" << std::endl;
  logFile << "   Price Short:       $" << priceShortIn << " (target)" << std::endl;
  logFile << "   Spread:            "  << spreadIn * 100.0 << "%" << std::endl;
  logFile << "   Cash used:         $" << exposure << " on each exchange" << std::endl;
  logFile << "   Exit Target:       "  << exitTarget * 100.0 << "%" << std::endl;
  logFile << std::endl;
}

void Result::printExitInfo(std::ofstream& logFile) {
  logFile << "\n[ EXIT FOUND ]" << std::endl;
  logFile << "   Date & Time:       "  << printDateTime(exitTime) << std::endl;
  logFile << "   Duration:          "  << getTradeLengthInMinute() << " minutes" << std::endl;
  logFile << "   Price Long:        $" << priceLongOut << " (target)" << std::endl;
  logFile << "   Price Short:       $" << priceShortOut << " (target)" << std::endl;
  logFile << "   Spread:            "  << spreadOut * 100.0 << "%" << std::endl;
  logFile << "   ---------------------------"  << std::endl;
  logFile << "   Target Perf Long:  "  << targetPerfLong()  * 100.0 << "% (fees incl.)" << std::endl;
  logFile << "   Target Perf Short: "  << targetPerfShort() * 100.0 << "% (fees incl.)" << std::endl;
  logFile << "   ---------------------------\n"  << std::endl;
}

void Result::reset() {
  id = 0;
  idExchLong = 0;
  idExchShort = 0;
  exposure = 0.0;
  feesLong = 0.0;
  feesShort = 0.0;
  entryTime = 0;
  exitTime = 0;
  exchNameLong = "";
  exchNameShort = "";
  priceLongIn = 0.0;
  priceShortIn = 0.0;
  priceLongOut = 0.0;
  priceShortOut = 0.0;
  spreadIn = 0.0;
  spreadOut = 0.0;
  exitTarget = 0.0;
  usdTotBalanceBefore = 0.0;
  usdTotBalanceAfter = 0.0;
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      minSpread[i][j] = 1.0;
      maxSpread[i][j] = -1.0;
      trailing[i][j] = -1.0;
      trailingWaitCount[i][j] = 0;
    }
  }
}

