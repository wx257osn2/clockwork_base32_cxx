#include<iostream>
#include<chrono>
#include<vector>
#include<cstdint>
#include<random>
#include<numeric>
#include"clockwork_base32.hpp"

template<typename RNG>
std::vector<std::uint8_t> make_data(RNG& rand, std::size_t size){
  std::uniform_int_distribution<std::uint8_t> dist(0, 255);
  std::vector<std::uint8_t> vec(size);
  std::generate(vec.begin(), vec.end(), [&]{return dist(rand);});
  return vec;
}

int main(int argc, char** argv){
  std::size_t size = 1 << 20;
  if(argc >= 2)
    if(const auto override_size = std::stoul(argv[1]))
      size = override_size;
  std::size_t iteration = 1000;
  if(argc == 3)
    iteration = std::stoul(argv[2]);
  std::mt19937 rand(std::chrono::high_resolution_clock::now().time_since_epoch().count());
  const auto data = make_data(rand, size);
  std::vector<std::chrono::microseconds> encode_times(iteration);
  for(auto&& x : encode_times){
    const auto d = data;
    const auto start = std::chrono::high_resolution_clock::now();
    const auto encoded = clockwork::encode(d);
    const auto end = std::chrono::high_resolution_clock::now();
    x = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  }
  const auto encoded = clockwork::encode(data);
  std::vector<std::chrono::microseconds> decode_times(iteration);
  for(auto&& x : decode_times){
    const auto e = encoded;
    const auto start = std::chrono::high_resolution_clock::now();
    const auto decoded = clockwork::decode<std::uint8_t>(e);
    const auto end = std::chrono::high_resolution_clock::now();
    x = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  }
  const auto decoded = clockwork::decode<std::uint8_t>(encoded);
  if(data != decoded){
    std::cerr << "validation failed" << std::endl;
    return EXIT_FAILURE;
  }
  std::sort(encode_times.begin(), encode_times.end());
  std::sort(decode_times.begin(), decode_times.end());
  std::cout << "encode: " << static_cast<double>(std::accumulate(encode_times.begin(), encode_times.end(), 0ull, [](auto l, std::chrono::microseconds r){return l + r.count();})) / iteration << '(' << encode_times.front().count() << '-' << encode_times.back().count() << ")\n"
               "decode: " << static_cast<double>(std::accumulate(decode_times.begin(), decode_times.end(), 0ull, [](auto l, std::chrono::microseconds r){return l + r.count();})) / iteration << '(' << decode_times.front().count() << '-' << decode_times.back().count() << ')' << std::endl;
}
