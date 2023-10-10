#ifndef deribit
#define deribit

#include <quickfix/Application.h>
#include <quickfix/FileLog.h>
#include <quickfix/FileStore.h>
#include <quickfix/Initiator.h>
#include <quickfix/MessageCracker.h>
#include <sys/_types/_int64_t.h>

class DeribitFIX : public FIX::Application, public FIX::MessageCracker {
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
  ~DeribitFIX();

  // Constructor
  DeribitFIX(FIX::SessionSettings);

  /* Actions */
  void run() EXCEPT(std::runtime_error);
  void request_test();
  void request_market_data(std::string const &symbol);
  void request_symbol_info();

  /* Implementing Application interface */
  void onCreate(FIX::SessionID const &) override;
  void onLogon(FIX::SessionID const &) override;
  void onLogout(FIX::SessionID const &) override;
  void toAdmin(FIX::Message &, FIX::SessionID const &) override;
  void toApp(FIX::Message &, FIX::SessionID const &)
      EXCEPT(FIX::DoNotSend) override;
  void fromAdmin(FIX::Message const &, FIX::SessionID const &)
      EXCEPT(FIX::FieldNotFound, FIX::IncorrectDataFormat,
             FIX::IncorrectTagValue, FIX::RejectLogon) override;
  void fromApp(FIX::Message const &, FIX::SessionID const &)
      EXCEPT(FIX::FieldNotFound, FIX::IncorrectDataFormat,
             FIX::IncorrectTagValue, FIX::UnsupportedMessageType) override;
};

#endif // deribit
