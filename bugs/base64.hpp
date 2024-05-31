#ifndef BASE64_HPP
#define BASE64_HPP

#include <string>
#include <bitset>
#include <vector>

namespace base64 {

  std::string encode(const std::string& in);
  std::string decode(const std::string& in);

  template<size_t N>
  std::string encode(const std::bitset<N>& bits);

  template<size_t N>
  std::bitset<N> decode_bitset(const std::string& base64_string);

  std::string encode(const std::vector<bool>& bits);

  std::vector<bool> decode_vector_bool(const std::string& base64_string);

}  // namespace base64

#endif  // BASE64_HPP