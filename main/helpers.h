#ifndef HELPERS_H
#define HELPERS_H
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include "Cache.hpp"
#include <string_view>
using namespace std;

enum class RespType
{
    STRING,
    ERROR,
    INTEGER,
    BULK,
    ARRAY,
    NIL,
    BULK_NULL
};

struct RespValue
{
    RespType type;
    string str;
    long long integer;
    vector<RespValue> array;

    bool is_null = false;
};

struct ParseResults
{
    bool has_expiry = false;
    std::chrono::steady_clock::duration ttl{0};
};

string to_upper(string s);
void print_value(const RespValue &val, int depth = 0);
pair<RespValue, int> parse_bulk_strings(string_view text, int pos);
pair<RespValue, int> parse_string(string_view text, int pos);
pair<RespValue, int> parse_simple_errors(string_view text, int pos);
pair<RespValue, int> parse_integers(string_view text, int pos);
pair<RespValue, int> prcoess_parser(string_view buffer, int offset);
string serialise(const RespValue &obj);

std::string handle_value(const RespValue &value);
ParseResults parse_set_options(const vector<RespValue> &args);

bool setKeys(const RespValue &value,Cache &storage, string &response);
bool getKeys(const RespValue &value,Cache &storage, string &response);

#endif