#include <iostream>
#include "parameters.h"
#include "time_fun.h"


Parameters::Parameters(std::string fileName) {

  std::ifstream configFile(fileName.c_str());
  spreadEntry = getDouble(getParameter("SpreadEntry", configFile));
  spreadTarget = getDouble(getParameter("SpreadTarget", configFile));
  maxLength = getUnsigned(getParameter("MaxLength", configFile));
  priceDeltaLim = getDouble(getParameter("PriceDeltaLimit", configFile));
  aggressiveVolume = getBool(getParameter("AggressiveVolume", configFile));
  trailingLim = getDouble(getParameter("TrailingSpreadLim", configFile));
  orderBookFactor = getDouble(getParameter("OrderBookFactor", configFile));
  demoMode = getBool(getParameter("DemoMode", configFile));
  verbose = getBool(getParameter("Verbose", configFile));
  gapSec = getUnsigned(getParameter("GapSec", configFile));
  debugMaxIteration = getUnsigned(getParameter("DebugMaxIteration", configFile));
  useFullCash = getBool(getParameter("UseFullCash", configFile));
  untouchedCash = getDouble(getParameter("UntouchedCash", configFile));
  cashForTesting = getDouble(getParameter("CashForTesting", configFile));
  maxExposure = getDouble(getParameter("MaxExposure", configFile));

  bitfinexApi = getParameter("BitfinexApiKey", configFile);
  bitfinexSecret = getParameter("BitfinexSecretKey", configFile);
  bitfinexFees = getDouble(getParameter("BitfinexFees", configFile));
  bitfinexCanShort = getBool(getParameter("BitfinexCanShort", configFile));

  okcoinApi = getParameter("OkCoinApiKey", configFile);
  okcoinSecret = getParameter("OkCoinSecretKey", configFile);
  okcoinFees = getDouble(getParameter("OkCoinFees", configFile));
  okcoinCanShort = getBool(getParameter("OkCoinCanShort", configFile));

  bitstampClientId = getParameter("BitstampClientId", configFile);
  bitstampApi = getParameter("BitstampApiKey", configFile);
  bitstampSecret = getParameter("BitstampSecretKey", configFile);
  bitstampFees = getDouble(getParameter("BitstampFees", configFile));
  bitstampCanShort = getBool(getParameter("BitstampCanShort", configFile));

  geminiApi = getParameter("GeminiApiKey", configFile);
  geminiSecret = getParameter("GeminiSecretKey", configFile);
  geminiFees = getDouble(getParameter("GeminiFees", configFile));
  geminiCanShort = getBool(getParameter("GeminiCanShort", configFile));

  krakenApi = getParameter("KrakenApiKey", configFile);
  krakenSecret = getParameter("KrakenSecretKey", configFile);
  krakenFees = getDouble(getParameter("KrakenFees", configFile));
  krakenCanShort = getBool(getParameter("KrakenCanShort", configFile));

  itbitApi = getParameter("ItBitApiKey", configFile);
  itbitSecret = getParameter("ItBitSecretKey", configFile);
  itbitFees = getDouble(getParameter("ItBitFees", configFile));
  itbitCanShort = getBool(getParameter("ItBitCanShort", configFile));

  btceApi = getParameter("BTCeApiKey", configFile);
  btceSecret = getParameter("BTCeSecretKey", configFile);
  btceFees = getDouble(getParameter("BTCeFees", configFile));
  btceCanShort = getBool(getParameter("BTCeCanShort", configFile));

  sevennintysixApi = getParameter("SevenNintySixApiKey", configFile);
  sevennintysixSecret = getParameter("SevenNintySixSecretKey", configFile);
  sevennintysixFees = getDouble(getParameter("SevenNintySixFees", configFile));
  sevennintysixCanShort = getBool(getParameter("SevenNintySixCanShort", configFile));

  sendEmail = getBool(getParameter("SendEmail", configFile));
  senderAddress = getParameter("SenderAddress", configFile);
  senderUsername = getParameter("SenderUsername", configFile);
  senderPassword = getParameter("SenderPassword", configFile);
  smtpServerAddress = getParameter("SmtpServerAddress", configFile);
  receiverAddress = getParameter("ReceiverAddress", configFile);

  useDatabase = getBool(getParameter("UseDatabase", configFile));
  dbHost = getParameter("DBHost", configFile);
  dbName = getParameter("DBName", configFile);
  dbUser = getParameter("DBUser", configFile);
  dbPassword = getParameter("DBPassword", configFile);

  configFile.close();
}


void Parameters::addExchange(std::string n, double f, bool h, bool m) {
  exchName.push_back(n);
  fees.push_back(f);
  canShort.push_back(h);
  isImplemented.push_back(m);
}


int Parameters::nbExch() const {
  return exchName.size();
}


std::string getParameter(std::string parameter, std::ifstream& configFile) {

  std::string line;
  configFile.clear();
  configFile.seekg(0);
  if (configFile.is_open()) {
    while (getline(configFile, line)) {
      if (line.length() > 0 && line.at(0) != '#') {
        std::string key = line.substr(0, line.find('='));
        std::string value = line.substr(line.find('=') + 1, line.length());
        if (key == parameter) {
          return value;
        }
      }
    }
    std::cout << "ERROR: parameter " << parameter << " not found" << std::endl;
    return "error";
  } else {
    std::cout << "ERROR: file cannot be open" << std::endl;
    return "error";
  }
}


bool getBool(std::string value) {
  if (value == "true") {
    return true;
  } else {
    return false;
  }
}


double getDouble(std::string value) {
  return atof(value.c_str());
}


unsigned getUnsigned(std::string value) {
  return atoi(value.c_str());
}
