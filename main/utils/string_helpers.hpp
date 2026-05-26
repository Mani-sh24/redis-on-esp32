#ifndef UTILS_STRING_HELPERS_HPP
#define UTILS_STRING_HELPERS_HPP

#include <string>
#include <string_view>
#include <optional>

std::string to_upper(std::string s);

bool parse_int(std::string_view text, int pos, int crlf, int &value);
std::optional<int> parse_int(std::string_view str);

#endif // UTILS_STRING_HELPERS_HPP
