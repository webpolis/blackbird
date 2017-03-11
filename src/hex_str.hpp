#ifndef HEX_STR_HPP
#define HEX_STR_HPP

#include <string>

template <typename FwdIt>
std::string hex_str(FwdIt first, FwdIt last)
{
  static_assert(sizeof(typename std::iterator_traits<FwdIt>::value_type) == 1,
                "value_type must be 1 byte.");
  constexpr const char *bytemap = "0123456789abcdef";

  std::string result(std::distance(first, last) * 2, '0');

  auto pos = begin(result);
  while (first != last)
  {
    *pos++ = bytemap[ *first >> 4 & 0xF ];
    *pos++ = bytemap[ *first++ & 0xF ];
  }

  return result;
}

#endif
