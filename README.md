# Clockwork Base32 for C++

Implementation of Clockwork Base32 for C++.
See [Clockwork Base32 Specification](https://gist.github.com/szktty/228f85794e4187882a77734c89c384a8)

## Usage

```cpp
#include<cstdint>
#include<string_view>
#include<vector>
#include<iostream>
#include "clockwork_base32.hpp"

std::vector<std::uint8_t> make_data(){
  const std::string_view hw = "Hello, world!";
  std::vector<std::uint8_t> data;
  data.reserve(hw.size());
  for(auto&& x : hw)
    data.emplace_back(x);
  return data;
}

void use_clockwork(){
  const auto data = make_data();
  const auto encoded = clockwork::encode(data);
  try{
    const auto decoded = clockwork::decode<char>(encoded);
    std::cout << "decoded => " << std::string(decoded.begin(), decoded.end()) << std::endl;
  }catch(std::invalid_argument& e){
    std::cout << "decode failed => " << e.what() << std::endl;
  }
}
```

### Playground

[Wandbox](https://wandbox.org/permlink/OIaiPE3zI6Vq1bJ2)
