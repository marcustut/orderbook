#include "orderbook.h"

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

std::string OrderBook::to_string(size_t level) {
  std::string res = "";
  auto bid_it = bids.rbegin();
  auto ask_it = asks.begin();
  for (size_t i = 0; i < level; i++) {
    auto bid = bid_it->second;
    auto ask = ask_it->second;
    res = res + "b: " + std::to_string(bid.price) + "/" + std::to_string(bid.quantity) + "\n";
    res = "a: " + std::to_string(ask.price) + "/" + std::to_string(ask.quantity) + "\n" + res;
    bid_it++;
    ask_it++;
  }
  return res;
}