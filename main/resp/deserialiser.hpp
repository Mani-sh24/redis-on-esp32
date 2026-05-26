#ifndef RESP_DESERIALISER_HPP
#define RESP_DESERIALISER_HPP

#include <string_view>
#include <utility>
#include "resp/types.hpp"

std::pair<RespValue, int> parse_bulk_strings(std::string_view text, int pos);
std::pair<RespValue, int> parse_string(std::string_view text, int pos);
std::pair<RespValue, int> parse_simple_errors(std::string_view text, int pos);
std::pair<RespValue, int> parse_integers(std::string_view text, int pos);
std::pair<RespValue, int> parse_array(std::string_view text, int pos);
std::pair<RespValue, int> prcoess_parser(std::string_view buffer, int offset);
void print_value(const RespValue &val, int depth = 0);

#endif // RESP_DESERIALISER_HPP
