#include<iostream>
#include"clockwork_base32.hpp"

struct test_case{
  std::string plain;
  std::string encoded;
};

bool test_encode(const test_case& t){
  const auto encoded = clockwork::encode(reinterpret_cast<const std::byte*>(t.plain.data()), t.plain.size());
  const bool ret = encoded == t.encoded;
  if(!ret){
    std::cout << "encoded '" << t.plain << "', expected '" << t.encoded << "', actual '" << encoded << "'\n";
  }
  return ret;
}

bool test_decode(const test_case& t){
  const auto decoded = clockwork::decode<char>(t.encoded);
  std::string d(decoded.begin(), decoded.end());
  const bool ret = d == t.plain;
  if(!ret){
    std::cout << "decoded '" << t.encoded << "', expected '" << t.plain << "', actual '" << d << "'\n";
  }
  return ret;
}

int main(){
  const test_case test_cases[] = {
    {"foobar", "CSQPYRK1E8"},
    {"Hello, world!", "91JPRV3F5GG7EVVJDHJ22"},
    {"The quick brown fox jumps over the lazy dog.", "AHM6A83HENMP6TS0C9S6YXVE41K6YY10D9TPTW3K41QQCSBJ41T6GS90DHGQMY90CHQPEBG"},
    {"Wow, it really works!", "AXQQEB10D5T20WK5C5P6RY90EXQQ4TVK44"}
  };
  for(auto&& x : test_cases)
    test_encode(x);
  for(auto&& x : test_cases)
    test_decode(x);
}