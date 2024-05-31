#include "base64.hpp"
#include <algorithm>

namespace base64 {

static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string encode(const std::string &in) {
  std::string out;

  int val = 0, valb = -6;
  for (unsigned char c : in) {
    val = (val << 8) + c;
    valb += 8;
    while (valb >= 0) {
      out.push_back(base64_chars[(val >> valb) & 0x3F]);
      valb -= 6;
    }
  }
  if (valb > -6) {
    out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
  }
  while (out.size() % 4) {
    out.push_back('=');
  }
  return out;
}

std::string decode(const std::string &in) {
  std::string out;

  std::vector<int> T(256, -1);
  for (int i = 0; i < 64; i++) {
    T[base64_chars[i]] = i;
  }

  int val = 0, valb = -8;
  for (unsigned char c : in) {
    if (T[c] == -1) break;
    val = (val << 6) + T[c];
    valb += 6;
    if (valb >= 0) {
      out.push_back(char((val >> valb) & 0xFF));
      valb -= 8;
    }
  }
  return out;
}

// Перегрузка для std::bitset
template<size_t N>
std::string encode(const std::bitset<N>& bits) {
  std::string bytes;
  bytes.reserve((bits.size() + 7) / 8);

  for (size_t i = 0; i < bits.size(); i += 8) {
    char byte = 0;
    for (size_t j = 0; j < 8 && i + j < bits.size(); ++j) {
      if (bits[i + j]) {
        byte |= 1 << (7 - j);
      }
    }
    bytes.push_back(byte);
  }

  return encode(bytes);
}

template<size_t N>
std::bitset<N> decode_bitset(const std::string& base64_string) {
  std::string bytes = decode(base64_string);
  std::bitset<N> bits;

  size_t min_size = std::min(bytes.size() * 8, bits.size());
  for (size_t i = 0; i < min_size; ++i) {
    if (bytes[i / 8] & (1 << (7 - (i % 8)))) {
      bits.set(i);
    }
  }

  return bits;
}

// Перегрузка для std::vector<bool>
std::string encode(const std::vector<bool>& bits) {
  std::string bytes;
  bytes.reserve((bits.size() + 7) / 8);

  for (size_t i = 0; i < bits.size(); i += 8) {
    char byte = 0;
    for (size_t j = 0; j < 8 && i + j < bits.size(); ++j) {
      if (bits[i + j]) {
        byte |= 1 << (7 - j);
      }
    }
    bytes.push_back(byte);
  }

  return encode(bytes);
}

std::vector<bool> decode_vector_bool(const std::string& base64_string) {
  std::string bytes = decode(base64_string);
  std::vector<bool> bits(bytes.size() * 8);

  for (size_t i = 0; i < bytes.size() * 8; ++i) {
    if (bytes[i / 8] & (1 << (7 - (i % 8)))) {
      bits[i] = true;
    }
  }

  return bits;
}

}  // namespace base64