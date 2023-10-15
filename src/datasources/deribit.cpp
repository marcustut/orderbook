#include "deribit.h"

#include <openssl/rand.h>
#include <openssl/sha.h>
#include <quickfix/FixFieldNumbers.h>
#include <quickfix/FixFields.h>
#include <quickfix/FixValues.h>
#include <quickfix/SocketInitiator.h>
#include <quickfix/fix44/MarketDataRequest.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>

#include "../crypto.h"

namespace Deribit
{
  Fix::~Fix()
  {
    if (this->m_initiator != nullptr)
    {
      this->m_initiator->stop();
      delete this->m_initiator;
    }

    this->m_settings.reset();
    this->m_synch.reset();
    this->m_store_factory.reset();
    this->m_log_factory.reset();
  }

  Fix::Fix(FIX::SessionSettings settings)
      : m_session_id(), m_request_id(0), m_client_order_id(0),
        m_initiator(nullptr), m_settings(), m_synch(), m_store_factory(),
        m_log_factory()
  {
    // Initializing quickfix engine
    this->m_settings = std::make_unique<FIX::SessionSettings>(settings);
    this->m_synch = std::make_unique<FIX::SynchronizedApplication>(*this);
    this->m_store_factory = std::make_unique<FIX::FileStoreFactory>(*m_settings);
    this->m_log_factory = std::make_unique<FIX::FileLogFactory>(*m_settings);
  }

  void Fix::run() EXCEPT(std::runtime_error)
  {
    try
    {
      m_initiator =
          new FIX::SocketInitiator(*this->m_synch, *this->m_store_factory,
                                   *this->m_settings, *this->m_log_factory);

      m_initiator->start();
      printf("[%s][run] Started socket initiator\n",
             this->m_session_id.toString().c_str());
    }
    catch (std::exception const &exception)
    {
      std::cout << exception.what() << std::endl;
      delete m_initiator;
      throw std::runtime_error("Error running Fix");
    }
  }

  void Fix::request_test()
  {
    FIX::Message message;
    FIX::Header &header = message.getHeader();

    header.setField(FIX::MsgType(FIX::MsgType_TestRequest));
    message.setField(FIX::TestReqID(std::to_string(this->m_request_id++)));

    FIX::Session::sendToTarget(message, this->m_session_id);
    printf("[%s][test_request] Sent test request %s \n",
           this->m_session_id.toString().c_str(),
           std::to_string(this->m_request_id).c_str());
  }

  void Fix::request_order_book(std::string const &symbol, int depth)
  {
    FIX::Message message;
    FIX::Header &header = message.getHeader();

    header.setField(FIX::MsgType(FIX::MsgType_MarketDataRequest));
    message.setField(FIX::Symbol(symbol));
    message.setField(FIX::MDReqID(std::to_string(this->m_request_id++)));
    message.setField(FIX::SubscriptionRequestType(
        FIX::SubscriptionRequestType_SNAPSHOT_AND_UPDATES));

    message.setField(FIX::MarketDepth(depth));
    message.setField(FIX::MDUpdateType(0));

    // Request only bid and ask prices
    message.setField(FIX::NoMDEntryTypes(2));
    FIX44::MarketDataRequest::NoMDEntryTypes entry_types;
    entry_types.set(FIX::MDEntryType_BID);
    message.addGroup(entry_types);
    entry_types.set(FIX::MDEntryType_OFFER);
    message.addGroup(entry_types);

    FIX::Session::sendToTarget(message, this->m_session_id);
    printf("[%s][request_order_book] Sent market data (orderbook) request %s for %s with depth %d\n",
           this->m_session_id.toString().c_str(),
           std::to_string(this->m_request_id).c_str(),
           symbol.c_str(),
           depth);
  }

  void Fix::request_symbol_info()
  {
    FIX::Message message;
    auto const request_id = std::to_string(this->m_request_id++);

    auto &header = message.getHeader();

    header.setField(FIX::MsgType(FIX::MsgType_SecurityListRequest));

    message.setField(FIX::FIELD::SecurityReqID, request_id);
    message.setField(FIX::FIELD::SecurityListRequestType, "0");

    FIX::Session::sendToTarget(message, this->m_session_id);
    printf("[%s][request_symbol_info] Sent security list request %s \n",
           this->m_session_id.toString().c_str(),
           std::to_string(this->m_request_id).c_str());
  }

  void Fix::onCreate(const FIX::SessionID &session_id)
  {
    this->m_session_id = session_id;
    printf("[%s][onCreate] FIX::Session created\n",
           this->m_session_id.toString().c_str());
  }

  void Fix::onLogon(const FIX::SessionID &session_id)
  {
    printf("[%s][onLogon] Logged on\n", this->m_session_id.toString().c_str());
  }

  void Fix::onLogout(const FIX::SessionID &session_id)
  {
    printf("[%s][onLogout] Logged out\n", this->m_session_id.toString().c_str());
  }

  void Fix::fromAdmin(const FIX::Message &message,
                      const FIX::SessionID &session_id)
  {
    printf("[%s][fromAdmin] Received %s\n", this->m_session_id.toString().c_str(),
           message.getHeader().getField(FIX::FIELD::MsgType).c_str());
  }

  void Fix::fromApp(const FIX::Message &message,
                    const FIX::SessionID &session_id)
      EXCEPT(FieldNotFound, IncorrectDataFormat, IncorrectTagValue,
             UnsupportedMessageType)
  {
    printf("[%s][fromApp] Received %s\n", this->m_session_id.toString().c_str(),
           message.getHeader().getField(FIX::FIELD::MsgType).c_str());
    crack(message, session_id);
  }

  void Fix::toAdmin(FIX::Message &message, const FIX::SessionID &)
  {
    auto const &msg_type = message.getHeader().getField(FIX::FIELD::MsgType);

    // Create logon message
    if (msg_type == FIX::MsgType_Logon)
    {
      auto const settings = this->m_settings.get()->get(this->m_session_id);
      std::string username = settings.getString("Username");

      std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch());
      std::string timestamp_in_ms = std::to_string(ms.count());
      unsigned char nonce[32] = {};

      RAND_bytes(nonce, sizeof(nonce));
      std::string nonce64 = Base64::encode(nonce, sizeof(nonce));
      std::string secret = settings.getString("Password");
      std::string raw_data = timestamp_in_ms + "." + nonce64;
      std::string base_signature_string = raw_data + secret;

      unsigned char hash[SHA256_DIGEST_LENGTH];
      SHA256_CTX sha256;
      SHA256_Init(&sha256);
      SHA256_Update(&sha256, base_signature_string.c_str(),
                    base_signature_string.size());
      SHA256_Final(hash, &sha256);

      static std::string password_sha_base64 = Base64::encode(hash, sizeof(hash));

      message.setField(FIX::HeartBtInt(20));
      message.setField(FIX::Username(username));
      message.setField(FIX::Password(password_sha_base64));
      message.setField(FIX::RawData(raw_data));
      message.setField(FIX::ResetSeqNumFlag("N"));
    }

    printf("[%s][toAdmin] Sending %s...\n", this->m_session_id.toString().c_str(),
           msg_type.c_str());
  }

  void Fix::toApp(FIX::Message &message, const FIX::SessionID &session_id)
      EXCEPT(DoNotSend)
  {

    printf("[%s][toApp] Sending %s...\n", this->m_session_id.toString().c_str(),
           message.getHeader().getField(FIX::FIELD::MsgType).c_str());
  }

  void Fix::onMessage(FIX44::MarketDataSnapshotFullRefresh const &message, FIX::SessionID const &session_id)
  {
    FIX::Symbol symbol;
    FIX::NoMDEntries no_md_entries;
    FIX44::MarketDataSnapshotFullRefresh::NoMDEntries entries_group;

    message.get(symbol);
    message.get(no_md_entries);

    for (size_t i = 0; i < no_md_entries; i++)
    {
      message.getGroup(i + 1, entries_group);

      FIX::MDEntryType md_entry_type; // 0=bid, 1=ask, 2=trade, 3=indexValue, 6=settlementPrice
      FIX::MDEntryPx md_entry_price;
      FIX::MDEntrySize md_entry_size;
      FIX::MDEntryDate md_entry_date;

      entries_group.get(md_entry_type);
      entries_group.get(md_entry_price);
      entries_group.get(md_entry_size);
      entries_group.get(md_entry_date);

      // Ignore other types of market data except for bid and ask
      if (md_entry_type != '0' && md_entry_type != '1')
        continue;

      printf("[%s][onMessage] Received market data (orderbook) for %s: %s at %s for %s at %s\n",
             session_id.toString().c_str(),
             symbol.getString().c_str(),
             md_entry_type == '0' ? "bid" : "ask",
             md_entry_price.getString().c_str(),
             md_entry_size.getString().c_str(),
             md_entry_date.getString().c_str());
    }
  }
} // namespace Deribit
