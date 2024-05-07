#include "orderbook.h"

#include <iostream>

OrderBook::OrderBook() {
  bids = std::map<double, Level>();
  asks = std::map<double, Level>();
}

std::optional<Level> OrderBook::best_bid() {
  if (bids.empty())
    return std::nullopt;
  return bids.rbegin()->second;
}

std::optional<Level> OrderBook::best_ask() {
  if (asks.empty())
    return std::nullopt;
  return asks.begin()->second;
}

double OrderBook::spread() {
  auto best_bid = this->best_bid();
  auto best_ask = this->best_ask();
  if (!best_bid.has_value() || !best_ask.has_value())
    return 0;
  return best_ask.value().price - best_bid.value().price;
}

void OrderBook::reset() {
  bids.clear();
  asks.clear();
}

void OrderBook::add_level(Level level, Side side) {
  switch (side) {
    case Side::Bid:
      bids[level.price] = level;
      break;
    case Side::Ask:
      asks[level.price] = level;
      break;
  }
}

void OrderBook::remove_level(double price, Side side) {
  switch (side) {
    case Side::Bid:
      bids.erase(price);
      break;
    case Side::Ask:
      asks.erase(price);
      break;
  }
}

std::pair<std::vector<Level>, std::vector<Level>> OrderBook::top_n(
    size_t level) {
  std::vector<Level> top_bids, top_asks;
  auto bid_it = bids.rbegin();
  auto ask_it = asks.begin();
  auto a = std::min(std::min(bids.size(), level), asks.size());
  for (size_t i = 0; i < a; i++) {
    auto bid = bid_it->second;
    auto ask = ask_it->second;
    top_bids.push_back(bid);
    top_asks.push_back(ask);
    bid_it++;
    ask_it++;
  }
  return std::make_pair(top_bids, top_asks);
}
