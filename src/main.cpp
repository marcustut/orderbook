#include <chrono>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <quickfix/Application.h>
#include <quickfix/FileStore.h>
#include <quickfix/FixFields.h>
#include <quickfix/FixValues.h>
#include <quickfix/Session.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/SocketAcceptor.h>
#include <quickfix/SocketInitiator.h>
#include <thread>

#include "orderbook.h"
#include "quickfix/SessionID.h"
#include "quickfix/fix44/Message.h"
#include "quickfix/fix44/MessageCracker.h"

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789+/";

std::string base64_encode(unsigned char const *bytes_to_encode,
                          unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] =
          ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] =
          ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (i = 0; (i < 4); i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] =
        ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] =
        ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while ((i++ < 3))
      ret += '=';
  }

  return ret;
}

class MyFIXApplication : public FIX::Application {
private:
  FIX::SessionSettings settings;
  FIX::SessionID session_id;

public:
  MyFIXApplication(FIX::SessionSettings settings) : settings(settings) {}

  void run() {
    auto sessions = this->settings.getSessions();
    auto const &session = *sessions.begin();

    std::cout << "[DEBUG] Before 1\n";

    std::string user = this->settings.get().getString("Username");

    std::chrono::milliseconds ms = duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    std::string timestamp_in_ms = std::to_string(ms.count());
    unsigned char nonce[32] = {};
    RAND_bytes(nonce, sizeof(nonce));
    std::string nonce64 = base64_encode(nonce, sizeof(nonce));
    std::string secret = this->settings.get().getString("Password");
    std::string raw_data = timestamp_in_ms + "." + nonce64;
    std::string base_signature_string = raw_data + secret;

    std::cout << "[DEBUG] Before SHA\n";

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, base_signature_string.c_str(),
                  base_signature_string.size());
    SHA256_Final(hash, &sha256);

    std::cout << "[DEBUG] Before B64\n";

    static std::string password_sha_base64 = base64_encode(hash, sizeof(hash));

    std::cout << "[DEBUG] Before Logon\n";

    FIX::Message message;
    FIX::Header &header = message.getHeader();

    message.setField(FIX::HeartBtInt(20));
    message.setField(FIX::Username(user));
    message.setField(FIX::Password(password_sha_base64));
    message.setField(FIX::RawData(raw_data));

    FIX::Session::sendToTarget(message, this->session_id);
    std::cout << "Logon sent to " << this->session_id << std::endl;

    FIX::Message message2;
    FIX::Header &header2 = message2.getHeader();

    header2.setField(FIX::MsgType(FIX::MsgType_TestRequest));
    message2.setField(FIX::TestReqID("123"));

    FIX::Session::sendToTarget(message2, this->session_id);
    std::cout << "TestRequest sent to " << this->session_id << std::endl;
  }

  void onCreate(const FIX::SessionID &session_id) override {
    this->session_id = session_id;
    std::cout << "Session created: " << session_id << std::endl;
  }

  void onLogon(const FIX::SessionID &session_id) override {
    std::cout << "Logged on at " << session_id << std::endl;
  }

  void onLogout(const FIX::SessionID &session_id) override {
    std::cout << "Logged out at " << session_id << std::endl;
  }

  void fromApp(const FIX::Message &message,
               const FIX::SessionID &sessionID) override {
    std::cout << "Message received from " << session_id << std::endl;
  }

  void toAdmin(FIX::Message &message, const FIX::SessionID &) override {
    auto const &msg_type = message.getHeader().getField(FIX::FIELD::MsgType);

    // Create logon message
    if (msg_type == FIX::MsgType_Logon) {
      std::cout << "[DEBUG] Before 1\n";

      std::string user = this->settings.get().getString("Username");

      std::chrono::milliseconds ms = duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch());
      std::string timestamp_in_ms = std::to_string(ms.count());
      unsigned char nonce[32] = {};
      RAND_bytes(nonce, sizeof(nonce));
      std::string nonce64 = base64_encode(nonce, sizeof(nonce));
      std::string secret = this->settings.get().getString("Password");
      std::string raw_data = timestamp_in_ms + "." + nonce64;
      std::string base_signature_string = raw_data + secret;

      std::cout << "[DEBUG] Before SHA\n";

      unsigned char hash[SHA256_DIGEST_LENGTH];
      SHA256_CTX sha256;
      SHA256_Init(&sha256);
      SHA256_Update(&sha256, base_signature_string.c_str(),
                    base_signature_string.size());
      SHA256_Final(hash, &sha256);

      std::cout << "[DEBUG] Before B64\n";

      static std::string password_sha_base64 =
          base64_encode(hash, sizeof(hash));

      std::cout << "[DEBUG] Before Logon\n";

      message.setField(FIX::HeartBtInt(20));
      message.setField(FIX::Username(user));
      message.setField(FIX::Password(password_sha_base64));
      message.setField(FIX::RawData(raw_data));
      message.setField(FIX::ResetSeqNumFlag("N"));
    }
  }

  void fromAdmin(const FIX::Message &, const FIX::SessionID &) override {}

  void toApp(FIX::Message &, const FIX::SessionID &) override {}
};

int main() {
  OrderBook ob = OrderBook();

  try {
    FIX::SessionSettings settings("fix_settings.cfg");
    MyFIXApplication application(settings);
    FIX::FileStoreFactory storeFactory(settings);
    FIX::ScreenLogFactory logFactory(settings);

    std::unique_ptr<FIX::Initiator> initiator =
        std::unique_ptr<FIX::Initiator>(new FIX::SocketInitiator(
            application, storeFactory, settings, logFactory));

    initiator->start();
    application.run();

    std::this_thread::sleep_for(std::chrono::seconds(10));
    initiator->stop();

    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
