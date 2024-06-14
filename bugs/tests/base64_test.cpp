#include <iostream>
#include <string>
#include <random>
#include <bitset>
#include <vector>
#include <algorithm>
#include <cassert>
#include <functional>
#include <limits>
#include <filesystem>
#include <fstream>
#include <queue>
#include <type_traits>

static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string to_base64(const std::string &in) {
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
  if (valb > -6) out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
  while (out.size() % 4) out.push_back('=');
  return out;
}

std::string from_base64(const std::string &in) {
  std::string out;

  std::vector<int> T(256, -1);
  for (int i = 0; i < 64; i++) T[base64_chars[i]] = i;

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
std::string to_base64(const std::bitset<N>& bits) {
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

  return to_base64(bytes);
}

template<size_t N>
std::bitset<N> bitset_from_base64(const std::string& base64_string) {
  std::string bytes = from_base64(base64_string);
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
std::string to_base64(const std::vector<bool>& bits) {
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

  return to_base64(bytes);
}

std::vector<bool> vector_from_base64(const std::string& base64_string) {
  std::string bytes = from_base64(base64_string);
  std::vector<bool> bits(bytes.size() * 8);

  for (size_t i = 0; i < bytes.size() * 8; ++i) {
    if (bytes[i / 8] & (1 << (7 - (i % 8)))) {
      bits[i] = true;
    }
  }

  return bits;
}

// Генератор случайных строк
std::string generate_random_string(size_t min_length = 1, size_t max_length = 100) {
  static std::random_device rd;
  static std::mt19937 generator(rd());
  static std::uniform_int_distribution<char> distribution(' ', '~');

  std::string random_string;
  size_t length = std::uniform_int_distribution<size_t>(min_length, max_length)(generator);
  random_string.reserve(length);

  for (size_t i = 0; i < length; ++i) {
    random_string += distribution(generator);
  }

  return random_string;
}

// Генератор случайных bitset
template<size_t N>
std::bitset<N> generate_random_bitset() {
  static std::random_device rd;
  static std::mt19937 generator(rd());
  static std::uniform_int_distribution<int> distribution;

  std::bitset<N> bits;
  for (size_t i = 0; i < N; i += 31) {
    bits ^= (distribution(generator) << i);
  }
  return bits;
}

// Генератор случайных vector<bool>
std::vector<bool> generate_random_vector_bool(size_t min_size = 1, size_t max_size = 100) {
  static std::random_device rd;
  static std::mt19937 generator(rd());
  static std::uniform_int_distribution<int> distribution;

  size_t size = std::uniform_int_distribution<size_t>(min_size, max_size)(generator);
  std::vector<bool> bits(size);
  for (size_t i = 0; i < size; ++i) {
    bits[i] = distribution(generator) % 2;
  }

  return bits;
}

// Функция для проверки, что строка содержит только символы Base64
bool is_valid_base64(const std::string& str) {
  return std::all_of(str.begin(), str.end(), [](char c) { return is_base64(c) || c == '='; });
}

int main() {
  // Количество итераций тестов
  const int num_tests = 100000; 

  for (int i = 0; i < num_tests; ++i) {
    // Генерируем случайные данные
    std::string random_string = generate_random_string();
    std::bitset<128> random_bitset = generate_random_bitset<128>();
    std::vector<bool> random_vector_bool = generate_random_vector_bool();

    // Конвертируем в Base64
    std::string base64_string = to_base64(random_string);
    std::string base64_bitset = to_base64(random_bitset);
    std::string base64_vector_bool = to_base64(random_vector_bool);

    // Проверяем, что строки содержат только допустимые символы Base64
    assert(is_valid_base64(base64_string));
    assert(is_valid_base64(base64_bitset));
    assert(is_valid_base64(base64_vector_bool));

    // Декодируем обратно
    std::string decoded_string = from_base64(base64_string);
    std::bitset<128> decoded_bitset = bitset_from_base64<128>(base64_bitset);
    std::vector<bool> decoded_vector_bool = vector_from_base64(base64_vector_bool);

    // Сравниваем результаты

    if (decoded_string != random_string) {
      std::cout << "Error in line: " << random_string << std::endl;
      std::cout << "Expected: " << random_string << std::endl;
      std::cout << "Got: " << decoded_string << std::endl;
      std::cout << "Base64: " << base64_string << std::endl;
      return 1;
    }
    if (decoded_bitset != random_bitset) {
        std::cout << "Error in bitset" << std::endl;
        std::cout << "Expected: " << random_bitset << std::endl;
        std::cout << "Got: " << decoded_bitset << std::endl;
        std::cout << "Base64: " << base64_bitset << std::endl;
        return 1;
    }
    
    if (std::equal(decoded_vector_bool.begin(), decoded_vector_bool.end(), random_vector_bool.begin()) == false){
        std::cout << "Error in vector<bool>" << std::endl;
        std::cout << "Expected: ";
        for (bool b : random_vector_bool) {
            std::cout << b;
        }
        std::cout << std::endl;
        std::cout << "Got: ";
        for (bool b : decoded_vector_bool) {
            std::cout << b;
        }
        std::cout << std::endl;
        std::cout << "Base64: " << base64_vector_bool << std::endl;
        return 1;
    }
  }

  std::cout << "All base64 test passed!" << std::endl;

  return 0;
}