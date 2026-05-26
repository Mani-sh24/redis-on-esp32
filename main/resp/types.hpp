#ifndef RESP_TYPES_HPP
#define RESP_TYPES_HPP

#include <string>
#include <vector>

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
    std::string str;
    long long integer;
    std::vector<RespValue> array;

    bool is_null = false;
};

#endif // RESP_TYPES_HPP
