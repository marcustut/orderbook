#include "deribit.h"

#include <openssl/rand.h>
#include <openssl/sha.h>
#include <quickfix/FixFieldNumbers.h>
#include <quickfix/FixFields.h>
#include <quickfix/FixValues.h>
#include <quickfix/SocketInitiator.h>
#include <quickfix/fix44/MarketDataRequest.h>

#include "../crypto.h"

DeribitFIX::~DeribitFIX() {
  if (this->m_initiator != nullptr) {
    this->m_initiator->stop();
    delete this->m_initiator;
  }

  this->m_settings.reset();
  this->m_synch.reset();
  this->m_store_factory.reset();
  this->m_log_factory.reset();
}

DeribitFIX::DeribitFIX(FIX::SessionSettings settings)
    : m_session_id(), m_request_id(0), m_client_order_id(0),
      m_initiator(nullptr), m_settings(), m_synch(), m_store_factory(),
      m_log_factory() {
  // Initializing quickfix engine
  this->m_settings = std::make_unique<FIX::SessionSettings>(settings);
  this->m_synch = std::make_unique<FIX::SynchronizedApplication>(*this);
  this->m_store_factory = std::make_unique<FIX::FileStoreFactory>(*m_settings);
  this->m_log_factory = std::make_unique<FIX::FileLogFactory>(*m_settings);
}

void DeribitFIX::run() EXCEPT(std::runtime_error) {
  try {
    m_initiator =
        new FIX::SocketInitiator(*this->m_synch, *this->m_store_factory,
                                 *this->m_settings, *this->m_log_factory);

    m_initiator->start();
    printf("[%s][run] Started socket initiator\n",
           this->m_session_id.toString().c_str());

  } catch (std::exception const &exception) {
    std::cout << exception.what() << std::endl;
    delete m_initiator;
    throw std::runtime_error("Error running DeribitFIX");
  }
}

void DeribitFIX::request_test() {
  FIX::Message message;
  FIX::Header &header = message.getHeader();

  header.setField(FIX::MsgType(FIX::MsgType_TestRequest));
  message.setField(FIX::TestReqID(std::to_string(this->m_request_id++)));

  FIX::Session::sendToTarget(message, this->m_session_id);
  printf("[%s][test_request] Sent test request %s \n",
         this->m_session_id.toString().c_str(),
         std::to_string(this->m_request_id).c_str());
}

void DeribitFIX::request_market_data(std::string const &symbol) {
  FIX::Message message;
  FIX::Header &header = message.getHeader();

  header.setField(FIX::MsgType(FIX::MsgType_MarketDataRequest));
  message.setField(FIX::Symbol(symbol));
  message.setField(FIX::MDReqID(std::to_string(this->m_request_id++)));
  message.setField(FIX::SubscriptionRequestType(
      FIX::SubscriptionRequestType_SNAPSHOT_AND_UPDATES));

  message.setField(FIX::MarketDepth(1));
  message.setField(FIX::MDUpdateType(0));

  // Request only bid and ask prices
  message.setField(FIX::NoMDEntryTypes(2));
  FIX44::MarketDataRequest::NoMDEntryTypes entry_types;
  entry_types.set(FIX::MDEntryType_BID);
  message.addGroup(entry_types);
  entry_types.set(FIX::MDEntryType_OFFER);
  message.addGroup(entry_types);

  FIX::Session::sendToTarget(message, this->m_session_id);
  printf("[%s][request_market_data] Sent market data request %s \n",
         this->m_session_id.toString().c_str(),
         std::to_string(this->m_request_id).c_str());
}

void DeribitFIX::request_symbol_info() {
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

void DeribitFIX::onCreate(const FIX::SessionID &session_id) {
  this->m_session_id = session_id;
  printf("[%s][onCreate] Session created\n",
         this->m_session_id.toString().c_str());
}

void DeribitFIX::onLogon(const FIX::SessionID &session_id) {
  printf("[%s][onLogon] Logged on\n", this->m_session_id.toString().c_str());
}

void DeribitFIX::onLogout(const FIX::SessionID &session_id) {
  printf("[%s][onLogout] Logged out\n", this->m_session_id.toString().c_str());
}

void DeribitFIX::fromAdmin(const FIX::Message &message,
                           const FIX::SessionID &session_id) {
  printf("[%s][fromAdmin] Received %s\n", this->m_session_id.toString().c_str(),
         message.getHeader().getField(FIX::FIELD::MsgType).c_str());
}

void DeribitFIX::fromApp(const FIX::Message &message,
                         const FIX::SessionID &session_id)
    EXCEPT(FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue,
           FIX::UnsupportedMessageType) {
  printf("[%s][fromApp] Received %s\n", this->m_session_id.toString().c_str(),
         message.getHeader().getField(FIX::FIELD::MsgType).c_str());
}

void DeribitFIX::toAdmin(FIX::Message &message, const FIX::SessionID &) {
  auto const &msg_type = message.getHeader().getField(FIX::FIELD::MsgType);

  // Create logon message
  if (msg_type == FIX::MsgType_Logon) {
    auto const settings = this->m_settings.get();
    std::string username = settings->get().getString("Username");

    std::chrono::milliseconds ms = duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    std::string timestamp_in_ms = std::to_string(ms.count());
    unsigned char nonce[32] = {};

    RAND_bytes(nonce, sizeof(nonce));
    std::string nonce64 = Base64::encode(nonce, sizeof(nonce));
    std::string secret = settings->get().getString("Password");
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

void DeribitFIX::toApp(FIX::Message &message, const FIX::SessionID &session_id)
    EXCEPT(FIX::DoNotSend) {

  printf("[%s][toApp] Sending %s...\n", this->m_session_id.toString().c_str(),
         message.getHeader().getField(FIX::FIELD::MsgType).c_str());
}
