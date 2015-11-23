#ifndef BITCOIN_H
#define BITCOIN_H

#include <iostream>
#include <string>
#include <vector>


// This class contains all the information
// (fees, bid, ask, etc) for a given exchange
class Bitcoin {

  private:
    unsigned id;
    std::string exchName;
    double fees;
    bool hasShort;
    bool isImplemented;
    double bid;
    double ask;
    double volume;

  public:
    Bitcoin(unsigned id, std::string n, double f, bool h, bool m);
    void updateData(double b, double a, double v);

    unsigned getId() const;
    double getAsk() const;
    double getBid() const;
    double getVolume() const;
    std::string getExchName() const;
    double getFees() const;
    bool getHasShort() const;
    bool getIsImplemented() const;
 };

#endif
