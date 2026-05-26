#ifndef COMMANDS_TYPES_HPP
#define COMMANDS_TYPES_HPP

#include <chrono>

struct ParseResults
{
    bool has_expiry = false;
    std::chrono::steady_clock::duration ttl{0};
};

#endif // COMMANDS_TYPES_HPP
