#include <gtest/gtest.h>

#include "../src/orderbook.h"

TEST(OrderBook, AddLevel) {
  auto ob = OrderBook();

  ob.add_level(Level{.price = 1.0, .quantity = 0.1}, Side::Bid);

  EXPECT_EQ(ob.best_bid().value().price, 1.0);
  EXPECT_EQ(ob.best_bid().value().quantity, 0.1);

  ob.add_level(Level{.price = 1.1, .quantity = 0.2}, Side::Ask);

  EXPECT_EQ(ob.best_ask().value().price, 1.1);
  EXPECT_EQ(ob.best_ask().value().quantity, 0.2);
}

TEST(OrderBook, Spread) {
  auto ob = OrderBook();

  EXPECT_EQ(ob.spread(), 0);

  ob.add_level(Level{.price = 1.0, .quantity = 0.1}, Side::Bid);

  EXPECT_EQ(ob.best_bid().value().price, 1.0);
  EXPECT_EQ(ob.best_bid().value().quantity, 0.1);

  ob.add_level(Level{.price = 1.1, .quantity = 0.2}, Side::Ask);

  EXPECT_EQ(ob.best_ask().value().price, 1.1);
  EXPECT_EQ(ob.best_ask().value().quantity, 0.2);

  EXPECT_EQ(ob.spread(), 1.1 - 1.0);
}
