#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"

#include "datasources/deribit.h"
#include "orderbook.h"

int main() {
  using namespace ftxui;

  OrderBook ob = OrderBook();

  try {
    FIX::SessionSettings settings("fix_settings.cfg");
    Deribit::Fix application(settings);

    // Attach handlers
    application.attach_bid_ask_snapshot_handler(
        [&ob](std::string const& symbol, BidAskSnapshot const& snapshot) {
          for (auto const& bid : snapshot.bids)
            ob.add_level({bid.price, bid.quantity}, Side::Bid);
          for (auto const& ask : snapshot.asks)
            ob.add_level({ask.price, ask.quantity}, Side::Ask);
        });

    application.attach_bid_ask_delta_handler(
        [&ob](std::string const& symbol, BidAskDelta const& delta) {
          for (auto const& bid : delta.bids)
            switch (bid.action) {
              case OfferAction::Add:
              case OfferAction::Update:
                ob.add_level({bid.offer.price, bid.offer.quantity}, Side::Bid);
                break;
              case OfferAction::Remove:
                ob.remove_level(bid.offer.price, Side::Bid);
                break;
            }

          for (auto const& ask : delta.asks)
            switch (ask.action) {
              case OfferAction::Add:
              case OfferAction::Update:
                ob.add_level({ask.offer.price, ask.offer.quantity}, Side::Ask);
                break;
              case OfferAction::Remove:
                ob.remove_level(ask.offer.price, Side::Ask);
                break;
            }
        });

    // Run the FIX engine
    application.run();

    // Wait for FIX to logon
    std::this_thread::sleep_for(std::chrono::seconds(3));

    /*     // Request symbol info */
    /*     application.request_symbol_info(); */

    // Request market data
    application.request_order_book("BTC-PERPETUAL");

    std::string reset_position;
    while (true) {
      auto [bids, asks] = ob.top_n(5);
      if (bids.size() < 5 || asks.size() < 5)
        continue;

      auto document = vbox({
                          text(L"Orderbook for BTC-PERPETUAL"),
                          separator(),
                          text(std::to_string(asks[4].price) + " " +
                               std::to_string(asks[4].quantity)),
                          text(std::to_string(asks[3].price) + " " +
                               std::to_string(asks[3].quantity)),
                          text(std::to_string(asks[2].price) + " " +
                               std::to_string(asks[2].quantity)),
                          text(std::to_string(asks[1].price) + " " +
                               std::to_string(asks[1].quantity)),
                          text(std::to_string(asks[0].price) + " " +
                               std::to_string(asks[0].quantity)),
                          separator(),
                          text(std::to_string(bids[0].price) + " " +
                               std::to_string(bids[0].quantity)),
                          text(std::to_string(bids[1].price) + " " +
                               std::to_string(bids[1].quantity)),
                          text(std::to_string(bids[2].price) + " " +
                               std::to_string(bids[2].quantity)),
                          text(std::to_string(bids[3].price) + " " +
                               std::to_string(bids[3].quantity)),
                          text(std::to_string(bids[4].price) + " " +
                               std::to_string(bids[4].quantity)),
                      }) |
                      border;

      document = vbox(filler(), document);

      auto screen = Screen::Create(Dimension::Full());
      Render(screen, document);
      std::cout << reset_position;
      screen.Print();
      reset_position = screen.ResetPosition();

      using namespace std::chrono_literals;
      std::this_thread::sleep_for(0.01s);  // Rerender every 0.01s.
    }

    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
