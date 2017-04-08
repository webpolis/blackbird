#ifndef BITCOIN_H
#define BITCOIN_H

#include "quote_t.h"
#include <string>

// contains all the information for a given exchange
class Bitcoin
{
  private:
    unsigned id;
    std::string exchName;
    double fees;
    bool hasShort;
    bool isImplemented;
    double bid, ask;

  public:
    Bitcoin(unsigned id, std::string n, double f, bool h, bool m);
    void updateData(quote_t quote);
    unsigned getId() const;
    double getAsk() const;
    double getBid() const;
    double getMidPrice() const;
    std::string getExchName() const;
    double getFees() const;
    bool getHasShort() const;
    bool getIsImplemented() const;
};

#endif
