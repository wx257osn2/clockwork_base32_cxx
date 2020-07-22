#include<cstdint>
#include<cstddef>
#include<algorithm>
#include<string>
#include<string_view>
#include<vector>
#include<stdexcept>

namespace clockwork{

namespace detail{

constexpr std::uint8_t read_bits(const std::uint8_t* src, std::size_t offset, std::size_t count = 5, std::uint8_t ret = 0){
  if(count == 0)
    return ret;
  const std::uint8_t *pos = src + offset / 8;
  const std::size_t bits = offset % 8;
  const std::size_t use = std::min(8 - bits, count);
  const std::uint8_t mask = (1 << use) - 1;
  const std::size_t off = 8 - use;
  return read_bits(src, offset + use, count - use, (ret << use) | (((*pos << bits) & (mask << off)) >> off));
}

}

static constexpr std::size_t calc_encoded_size(std::size_t input_size)noexcept{
  const std::size_t s = input_size * 8;
  return s / 5 + (s % 5 == 0 ? 0 : 1);
}

static constexpr void encode(const std::uint8_t* inputs, std::size_t input_size, char* outputs){
  constexpr char symbols[32] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K',
    'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'V', 'W', 'X',
    'Y', 'Z'
  };
  std::size_t offset = 0;
  std::size_t i = input_size * 8;
  for(; i >= 5; i -= 5){
    const auto b = detail::read_bits(inputs, offset);
    *outputs++ = symbols[b];
    offset += 5;
  }
  if(i > 0){
    const auto b = detail::read_bits(inputs, offset, i);
    *outputs++ = symbols[b << (5 - i)];
  }
}

static inline std::string encode(const std::uint8_t* inputs, std::size_t input_size){
  std::string str(calc_encoded_size(input_size), 0);
  encode(inputs, input_size, str.data());
  return str;
}

static inline void encode(const std::vector<std::uint8_t>& inputs, char* outputs){
  encode(inputs.data(), inputs.size(), outputs);
}

static inline std::string encode(const std::vector<std::uint8_t>& inputs){
  return encode(inputs.data(), inputs.size());
}

static inline void encode(const std::byte* inputs, std::size_t input_size, char* outputs){
  encode(reinterpret_cast<const std::uint8_t*>(inputs), input_size, outputs);
}

static inline std::string encode(const std::byte* inputs, std::size_t input_size){
  return encode(reinterpret_cast<const std::uint8_t*>(inputs), input_size);
}

static inline void encode(const std::vector<std::byte>& inputs, char* outputs){
  encode(reinterpret_cast<const std::uint8_t*>(inputs.data()), inputs.size(), outputs);
}

static inline std::string encode(const std::vector<std::byte>& inputs){
  return encode(reinterpret_cast<const std::uint8_t*>(inputs.data()), inputs.size());
}

static constexpr std::size_t calc_decoded_size(std::size_t input_size)noexcept{
  return input_size * 5 / 8;
}

namespace detail{

struct bits_writer{
  std::uint8_t* o;
  std::uint8_t v = 0;
  std::uint8_t n = 0;
  constexpr bits_writer& shift(){
    v = v << (8 - n);
    return *this;
  }
  constexpr void flush(){
    *o++ = v;
    n = 0;
    v = 0;
  }
  constexpr bits_writer& operator()(std::uint8_t bits, std::uint8_t data){ //data must have no bits on upper
    if(n + bits <= 8){
      v =  (v << bits) | data;
      n += bits;
      if(n == 8)
        flush();
      return *this;
    }
    else{
      const std::uint8_t lb = 8 - n;
      const std::uint8_t nb = bits - lb;
      v = (v << lb) | (data >> nb);
      flush();
      const std::uint8_t xb = 8 - nb;
      v = data << xb >> xb;
      n = nb;
    }
    return *this;
  }
};

}

static constexpr void decode(const char* inputs, std::size_t input_size, std::uint8_t* outputs){
  constexpr std::int8_t symbols[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 0-9 */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 10-19 */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 20-29 */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 30-39 */
    -1, -1, -1, -1, -1, -1, -1, -1,  0,  1, /* 40-49 */
     2,  3,  4,  5,  6,  7,  8,  9,  0, -1, /* 50-59 */
    -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, /* 60-69 */
    15, 16, 17,  1, 18, 19,  1, 20, 21,  0, /* 70-79 */
    22, 23, 24, 25, 26, -2, 27, 28, 29, 30, /* 80-89 */
    31, -1, -1, -1, -1, -1, -1, 10, 11, 12, /* 90-99 */
    13, 14, 15, 16, 17,  1, 18, 19,  1, 20, /* 100-109 */
    21,  0, 22, 23, 24, 25, 26, -1, 27, 28, /* 110-119 */
    29, 30, 31, -1, -1, -1, -1, -1, -1, -1, /* 120-129 */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 130-109 */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 140-109 */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 150-109 */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 160-109 */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 170-109 */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 180-109 */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 190-109 */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 200-209 */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 210-209 */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 220-209 */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 230-209 */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 240-209 */
    -1, -1, -1, -1, -1, -1                  /* 250-256 */
  };
  std::size_t total = 0;
  detail::bits_writer o{outputs};
  const std::size_t back = input_size - 1;
  for(std::size_t i = 0; i < back; ++i){
    const auto sym = symbols[static_cast<std::uint8_t>(inputs[i])];
    if(sym < 0)
      throw std::invalid_argument(std::string{"invalid_symbol value "} + static_cast<char>(sym));
    o(5, static_cast<std::uint8_t>(sym));
    total += 5;
  }
  {
    auto sym = symbols[static_cast<std::uint8_t>(inputs[back])];
    if(sym < 0)
      throw std::invalid_argument(std::string{"invalid_symbol value "} + static_cast<char>(sym));
    std::uint8_t n = 5;
    const std::uint8_t padding = static_cast<std::uint8_t>(input_size * 5 % 8);
    if(padding != 0){
      n = 5 - padding;
      sym >>= padding;
    }
    o(n, static_cast<std::uint8_t>(sym)).shift().flush();
    total += n;
  }
  if(total % 8 != 0)
    throw std::invalid_argument("invalid total number of decode bits " + std::to_string(total));
}

static inline void decode(std::string_view s, std::uint8_t* outputs){
  decode(s.data(), s.size(), outputs);
}

template<typename T = std::uint8_t, std::enable_if_t<sizeof(T) == 1, std::nullptr_t> = nullptr>
static inline std::vector<T> decode(const char* inputs, std::size_t input_size){
  std::vector<T> outputs(calc_decoded_size(input_size));
  decode(inputs, input_size, reinterpret_cast<std::uint8_t*>(outputs.data()));
  return outputs;
}

template<typename T = std::uint8_t, std::enable_if_t<sizeof(T) == 1, std::nullptr_t> = nullptr>
static inline std::vector<T> decode(std::string_view s){
  return decode<T>(s.data(), s.size());
}

static inline void decode(const char* inputs, std::size_t input_size, std::byte* outputs){
  decode(inputs, input_size, reinterpret_cast<std::uint8_t*>(outputs));
}

static inline void decode(std::string_view s, std::byte* outputs){
  decode(s, reinterpret_cast<std::uint8_t*>(outputs));
}

}
