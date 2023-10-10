#ifndef crypto
#define crypto

#include <string>

namespace Base64 {
static const std::string CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz"
                                 "0123456789+/";

std::string encode(unsigned char const *bytes, unsigned int in_len);
} // namespace Base64

#endif // crypto
