#ifndef RESULT_H
#define RESULT_H

#include <fstream>
#include <time.h>
#include <string>
#include <list>

// structure that represents the result of a complete long/short trade (i.e. 2 trades IN, 2 trades OUT)
// some information is filled after the first two trades IN
struct Result {

  unsigned id;
  unsigned idExchLong;
  unsigned idExchShort;
  double exposure;
  double feesLong;
  double feesShort;
  time_t entryTime;
  time_t exitTime;
  std::string exchNameLong;
  std::string exchNameShort;
  double priceLongIn;
  double priceShortIn;
  double priceLongOut;
  double priceShortOut;
  double spreadIn;
  double spreadOut;
  double exitTarget;

  double minSpread[10][10];              // FIXME size
  double maxSpread[10][10];              // FIXME size
  double trailing[10][10];               // FIXME size
  unsigned trailingWait[10][10];         // FIXME size
  std::list<double> volatility[10][10];  // FIXME size

  double befBalUsd;
  double aftBalUsd;

  // return the *target* performance
  // including exchange fees
  double perfLong();
  double perfShort();

  double totPerf();

  // get the length of the trade in minutes
  double getLength();

  void printEntry(std::ofstream& logFile);
  void printExit(std::ofstream& logFile);

  void clear();

};

#endif
