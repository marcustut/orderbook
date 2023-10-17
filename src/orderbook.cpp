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
      if (bids.contains(level.price)) {
        bids[level.price].quantity += level.quantity;
        return;
      }
      bids[level.price] = level;
      break;
    case Side::Ask:
      if (asks.contains(level.price)) {
        asks[level.price].quantity += level.quantity;
        return;
      }
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
