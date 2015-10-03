#include "bitcoin.h"
#include <math.h>

Bitcoin::Bitcoin(unsigned i, std::string n, double f, bool h) {
  id = i;
  exchName = n;
  fees = f;
  hasShort = h;
  bid = 0.0;
  ask = 0.0;
  volume = 0;
}

void Bitcoin::updateData(double b, double a, double v) {
  bid = b;
  ask = a;
  volume = v;
}

unsigned Bitcoin::getId() const {
  return id;
}

double Bitcoin::getBid() const {
  return bid;
}

double Bitcoin::getAsk() const {
  return ask;
}

double Bitcoin::getVolume() const {
  return volume;
}

std::string Bitcoin::getExchName() const {
  return exchName;
}

double Bitcoin::getFees() const {
  return fees;
}

bool Bitcoin::getHasShort() const {
  return hasShort;
}

