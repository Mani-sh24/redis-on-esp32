#include "helpers.h"
#include <vector>
#include <string>
#include <iostream>
#include <charconv>
using namespace std;
pair<RespValue, int> parse_bulk_strings(string_view text, int pos)
{
    // $5\r\nhello\r\n
    int original = pos - 1;
    int crlf = text.find("\r\n", pos);
    if (crlf == string::npos)
        return {{}, -1};
    int len;
    auto num = text.substr(pos, crlf - pos);

    auto [ptr, ec] =
        std::from_chars(num.data(),
                        num.data() + num.size(),
                        len);

    if (ec != std::errc())
        return {{}, -1};
    if (len == -1)
    {
        RespValue result;
        result.type = RespType::NIL;
        result.is_null = true;
        return {result, (crlf + 2) - original};
    }
    int start = crlf + 2;
    if (start + len + 2 > text.size()) // check for full frame
        return {{}, -1};
    RespValue result;
    result.type = RespType::BULK;
    result.str = text.substr(start, len);
    if (text.substr(start + len, 2) != "\r\n")
        return {{}, -1};
    int end = start + len + 2;
    // cout << start << endl;
    return {result, end - original};
}
pair<RespValue, int> parse_string(string_view text, int pos)
{
    int original = pos - 1;
    int crlf = text.find("\r\n", pos);
    if (crlf == string::npos)
        return {{}, -1};
    int len = crlf - pos;
    int start = pos;
    if (start + len + 2 > text.size()) // check for full frame
        return {{}, -1};
    RespValue result;
    result.type = RespType::STRING;
    result.str = text.substr(start, len);
    // string result = text.substr(start, len);
    if (text.substr(start + len, 2) != "\r\n")
        return {{}, -1};
    int end = start + len + 2;
    return {result, end - original};
}
pair<RespValue, int> parse_simple_errors(string_view text, int pos)
{
    int original = pos - 1;
    int crlf = text.find("\r\n", pos);
    if (crlf == string::npos)
        return {{}, -1};
    int start = pos;
    int len = crlf - pos;
    if (start + len + 2 > text.size()) // check for full frame
        return {{}, -1};
    RespValue result;
    result.type = RespType::ERROR;
    result.str = text.substr(start, len);
    if (text.substr(start + len, 2) != "\r\n")
        return {{}, -1};
    int end = start + len + 2;
    return {result, end - original};
}
pair<RespValue, int> parse_integers(string_view text, int pos)
{
    int original = pos - 1;

    int crlf = text.find("\r\n", pos);
    if (crlf == string::npos)
        return {{}, -1};

    if (crlf == pos)
        return {{}, -1}; // empty integer

    RespValue result;
    result.type = RespType::INTEGER;
    auto num = text.substr(pos, crlf - pos);
    auto [ptr, ec] = std::from_chars(
        num.data(),
        num.data() + num.size(),
        result.integer);
    if (ec != std::errc())
        return {{}, -1};

    int end = crlf + 2;
    return {result, end - original};
}
pair<RespValue, int> parse_array(string_view text, int pos)
{
    int original = pos - 1;

    int crlf = text.find("\r\n", pos);
    if (crlf == string::npos)
        return {{}, -1};

    int len;
    auto num = text.substr(pos, crlf - pos);

    auto [ptr, ec] = std::from_chars(
        num.data(),
        num.data() + num.size(),
        len);

    if (ec != std::errc() ||
        ptr != num.data() + num.size())
        return {{}, -1};
    int cursor = crlf + 2;

    // null array
    if (len == -1)
    {
        RespValue result;
        result.type = RespType::NIL;
        return {result, cursor - original};
    }

    RespValue result;
    result.type = RespType::ARRAY;

    for (int i = 0; i < len; i++)
    {
        auto [value, consumed] = prcoess_parser(text, cursor);

        if (consumed < 0)
            return {{}, -1};

        result.array.push_back(value);
        cursor += consumed;
    }

    return {result, cursor - original};
}
pair<RespValue, int> prcoess_parser(string_view buffer, int offset)
{
    if (offset >= buffer.size())
        return {{}, -1};
    switch (buffer[offset])
    {
    case '+':
        return parse_string(buffer, offset + 1);
    case '$':
        return parse_bulk_strings(buffer, offset + 1);
    case '-':
        return parse_simple_errors(buffer, offset + 1);
    case ':':
        return parse_integers(buffer, offset + 1);
    case '*':
        return parse_array(buffer, offset + 1);
    default:
        return {{}, -1};
    }
}
void print_value(const RespValue &val, int depth)
{
    string indent(depth * 2, ' ');

    switch (val.type)
    {
    case RespType::STRING:
        cout << indent << "STRING: " << val.str << "\n";
        break;

    case RespType::ERROR:
        cout << indent << "ERROR: " << val.str << "\n";
        break;

    case RespType::INTEGER:
        cout << indent << "INTEGER: " << val.integer << "\n";
        break;

    case RespType::BULK:
        cout << indent << "BULK: " << val.str << "\n";
        break;

    case RespType::NIL:
        cout << indent << "NIL\n";
        break;
    case RespType::BULK_NULL:
        cout << indent << "NIL\n";
        break;
    case RespType::ARRAY:
        cout << indent << "ARRAY (" << val.array.size() << " elements):\n";
        for (const auto &elem : val.array)
        {
            print_value(elem, depth + 1);
        }
        break;
    }
}