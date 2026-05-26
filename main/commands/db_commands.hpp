#ifndef COMMANDS_DB_COMMANDS_HPP
#define COMMANDS_DB_COMMANDS_HPP

#include <string>
#include <vector>
#include "resp/types.hpp"
#include "cache/Cache.hpp"
#include "commands/types.hpp"

bool setKeys(const RespValue &value, Cache &storage, std::string &response);
bool getKeys(const RespValue &value, Cache &storage, std::string &response);
ParseResults parse_set_options(const std::vector<RespValue> &args);

bool incr(const RespValue &value, Cache &storage, std::string &response);
bool decr(const RespValue &value, Cache &storage, std::string &response);
bool incr_by(const RespValue &value, Cache &storage, std::string &response);

#endif // COMMANDS_DB_COMMANDS_HPP
