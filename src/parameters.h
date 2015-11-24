#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <fstream>
#include <time.h>
#include <string>
#include <vector>
#include <jansson.h>
#include <curl/curl.h>

// this structure contains the exchanges and
// the strategy parameters
struct Parameters {

  // exchanges information
  std::vector<std::string> exchName;
  std::vector<double> fees;
  std::vector<bool> hasShort;
  std::vector<bool> isImplemented;
  std::vector<std::string> tickerUrl;

  // general parameters
  CURL* curl;
  double spreadEntry;
  double spreadTarget;
  unsigned maxLength;
  double priceDeltaLim;
  bool aggressiveVolume;
  double trailingLim;
  bool infoOnly;
  bool verbose;
  std::ofstream* logFile;

  // credentials
  const char *bitfinexApi;
  const char *bitfinexSecret;
  const char *okCoinApi;
  const char *okCoinSecret;
  const char *bitstampClientId;
  const char *bitstampApi;
  const char *bitstampSecret;
  const char *geminiApi;
  const char *geminiSecret;
  const char *krakenApi;
  const char *krakenSecret;

  // email
  bool sendEmail;
  const char* senderAddress;
  const char* senderUsername;
  const char* senderPassword;
  const char* smtpServerAddress;
  const char* receiverAddress;

  // constructor
  Parameters(json_t* root);

  // adds a new exchange
  void addExchange(std::string n, double f, bool h, bool m);

  // returns the number of exchange analyzed
  int nbExch() const;

};

#endif
