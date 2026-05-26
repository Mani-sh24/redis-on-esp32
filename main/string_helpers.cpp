#include "utils/string_helpers.hpp"
#include <cctype>
#include <charconv>

std::string to_upper(std::string s)
{
    for(auto &c:s) c=toupper((unsigned char)c);
    return s;
}

bool parse_int(std::string_view text, int pos, int crlf, int &value)
{
    if (crlf == std::string_view::npos || crlf < pos)
        return false;

    auto num = text.substr(pos, crlf - pos);
    auto [ptr, ec] = std::from_chars(
        num.data(),
        num.data() + num.size(),
        value);
    return ec == std::errc() && ptr == num.data() + num.size();
}

std::optional<int> parse_int(std::string_view str)
{
    int val;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), val);
    if (ec == std::errc() && ptr == str.data() + str.size())
    {
        return val;
    }
    return std::nullopt;
}