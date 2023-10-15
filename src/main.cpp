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

    // Run the FIX engine
    application.run();

    // Wait for FIX to logon
    std::this_thread::sleep_for(std::chrono::seconds(3));

    /*     // Request symbol info */
    /*     application.request_symbol_info(); */

    // Request market data
    application.request_order_book("BTC-PERPETUAL", 1);

    while (true)
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
  }
  catch (const std::exception &e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
