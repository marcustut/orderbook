#include <chrono>
#include <iostream>
#include <thread>

#include "datasources/deribit.h"

#include "orderbook.h"

int main()
{
  OrderBook ob = OrderBook();

  try
  {
    FIX::SessionSettings settings("fix_settings.cfg");
    Deribit::Fix application(settings);

    // Attach handlers
    application.attach_bid_ask_snapshot_handler([&ob](std::string const &symbol, BidAskSnapshot const &snapshot)
                                                {
      for (auto const &bid : snapshot.bids)
        ob.add_level({bid.price, bid.quantity}, Side::Bid);
      for (auto const &ask : snapshot.asks)
        ob.add_level({ask.price, ask.quantity}, Side::Ask); });
    application.attach_bid_ask_delta_handler([&ob](std::string const &symbol, BidAskDelta const &delta)
                                             {
      for (auto const &bid : delta.bids)
        switch (bid.action)
        {
        case OfferAction::Add:
        case OfferAction::Update:
          ob.add_level({bid.offer.price, bid.offer.quantity}, Side::Bid);
          break;
        case OfferAction::Remove:
          ob.remove_level(bid.offer.price, Side::Bid);
          break;
        }

      for (auto const &ask : delta.asks)
        switch (ask.action)
        {
        case OfferAction::Add:
        case OfferAction::Update:
          ob.add_level({ask.offer.price, ask.offer.quantity}, Side::Ask);
          break;
        case OfferAction::Remove:
          ob.remove_level(ask.offer.price, Side::Ask);
          break;
        } });

    // Run the FIX engine
    application.run();

    // Wait for FIX to logon
    std::this_thread::sleep_for(std::chrono::seconds(3));

    /*     // Request symbol info */
    /*     application.request_symbol_info(); */

    // Request market data
    application.request_order_book("BTC-PERPETUAL");

    while (true)
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      std::cout << "[OB] Best bid: " << ob.best_bid().value_or(Level{0, 0}).price << ", Best ask: " << ob.best_ask().value_or(Level{0, 0}).price << std::endl;
    }

    return 0;
  }
  catch (const std::exception &e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
