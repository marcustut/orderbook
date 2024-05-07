#ifndef orderbook
#define orderbook

#include <map>
#include <optional>
#include <vector>

// Determines whether it is a bid or ask level.
enum class Side { Bid, Ask };

// Represents a level in the order book.
typedef struct {
  double price;
  double quantity;
} Level;

class OrderBook {
 private:
  std::map<double, Level> bids;
  std::map<double, Level> asks;

 public:
  OrderBook();

  std::optional<Level> best_bid();
  std::optional<Level> best_ask();

  double spread();

  void reset();

  void add_level(Level level, Side side);
  void remove_level(double price, Side side);
  std::pair<std::vector<Level>, std::vector<Level>> top_n(size_t level);
};

#endif  // orderbook
