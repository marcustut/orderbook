#ifndef deribit
#define deribit

#include <quickfix/Application.h>
#include <quickfix/FileLog.h>
#include <quickfix/FileStore.h>
#include <quickfix/Initiator.h>
#include <quickfix/MessageCracker.h>
#include <quickfix/Field.h>
#include <sys/_types/_int64_t.h>

namespace Deribit
{
  class Fix : public FIX::Application, public FIX::MessageCracker
  {
  private:
    // To identify the FIX session
    FIX::SessionID m_session_id;

    // To identify each request sent from the client
    int64_t m_request_id;

    // To identify each order sent from the client
    // TODO: Perhaps need something truly random
    int64_t m_client_order_id;

    // QuickFIX specific stuff
    FIX::Initiator *m_initiator;
    std::unique_ptr<FIX::SessionSettings> m_settings;
    std::unique_ptr<FIX::SynchronizedApplication> m_synch;
    std::unique_ptr<FIX::FileStoreFactory> m_store_factory;
    std::unique_ptr<FIX::FileLogFactory> m_log_factory;

  public:
    // Destructor
    ~Fix();

    // Constructor
    Fix(FIX::SessionSettings);

    /* Actions */
    void run() EXCEPT(std::runtime_error);
    void request_test();
    void request_order_book(std::string const &symbol, int depth);
    void request_symbol_info();

    /* Implementing Application interface */
    void onCreate(FIX::SessionID const &) override;
    void onLogon(FIX::SessionID const &) override;
    void onLogout(FIX::SessionID const &) override;
    void toAdmin(FIX::Message &, FIX::SessionID const &) override;
    void toApp(FIX::Message &, FIX::SessionID const &)
        EXCEPT(DoNotSend) override;
    void fromAdmin(FIX::Message const &, FIX::SessionID const &)
        EXCEPT(FieldNotFound, IncorrectDataFormat,
               IncorrectTagValue, RejectLogon) override;
    void fromApp(FIX::Message const &, FIX::SessionID const &)
        EXCEPT(FieldNotFound, IncorrectDataFormat,
               IncorrectTagValue, UnsupportedMessageType) override;

    /* Implementing MessageCracker interface */
    virtual void onMessage(FIX44::MarketDataSnapshotFullRefresh const &, FIX::SessionID const &) override;

    // // Following are the custom fields that Deribit uses, these can be found in their FIX documentation.
    // // https://docs.deribit.com/#market-data-request-v

    // // To skip block trades. If 9011=Y then block trades will not be reported. Default is N
    // USER_DEFINE_BOOLEAN(DeribitSkipBlockTrades, 9011);

    // // To show block trade id. If 9012=Y and 9012=N then block trades will include BlockTrade ID as TrdMatchID (880). Default is N
    // USER_DEFINE_BOOLEAN(DeribitShowBlockTradeId, 9012);

    // // Amount of trades returned in the snapshot response to request for snapshot of recent trades, default 20, maximum 1000
    // USER_DEFINE_INT(DeribitTradeAmount, 100007);

    // // UTC Timestamp in milliseconds (integer number of milliseconds), if specified, the response returns the trades happened since that timestamp, applicable to the request for recent trades snapshot
    // USER_DEFINE_INT(DeribitSinceTimestamp, 100008);

    // USER_DEFINE_QTY(TradeVolume24h, 100087);
    // USER_DEFINE_PRICE(MarkPrice, 100090);
    // USER_DEFINE_PRICE(CurrentFunding, 100092);
    // USER_DEFINE_PRICE(Funding8h, 100093);
    // USER_DEFINE_STRING(DeribitTradeId, 100009);
    // USER_DEFINE_STRING(DeribitLabel, 100010);
    // USER_DEFINE_STRING(DeribitLiquidation, 100091);
  };
} // namespace Deribit

#endif // deribit
