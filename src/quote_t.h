#ifndef QUOTE_T_H
#define QUOTE_T_H

#include <utility>

class quote_t {
  
  typedef double bid_t;
  typedef double ask_t;
  std::pair<bid_t, ask_t> quote_;

public:
  quote_t(bid_t bid, ask_t ask) : quote_(std::make_pair(bid, ask)) {}
  quote_t(std::pair<bid_t, ask_t> &&quote) : quote_(std::move(quote)) {}
  bid_t bid() const { return quote_.first; }
  ask_t ask() const { return quote_.second; }
};

#endif
