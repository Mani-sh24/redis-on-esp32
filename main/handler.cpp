#include "commands/handler.hpp"
#include "commands/db_commands.hpp"
#include "cache/Cache.hpp"
#include "resp/serialiser.hpp"
#include <cctype>

Cache storage(true);

std::string handle_value(const RespValue &value)
{
  std::string response;

  if (value.type != RespType::ARRAY || value.array.empty())
    return response;

  std::string command = value.array[0].str;
  
  for (auto &c : command)
    c = toupper((unsigned char)c);

  if (command == "PING")
  {
    response = "+PONG\r\n";
  }
  else if (command == "ECHO" && value.array.size() >= 2)
  {
    response = serialise(value.array[1]);
  }
  else if (command == "SET" && value.array.size() >= 3)
  {
    return setKeys(value, storage, response) ? response : response;
  }
  else if (command == "GET" && value.array.size() >= 2)
  {
    return getKeys(value, storage, response) ? response : response;
  }
  else if (command == "INCR" && value.array.size() >= 2)
  {
    return incr(value, storage, response) ? response : response;
  }
  else if (command == "DECR" && value.array.size() >= 2)
  {
    return decr(value, storage, response) ? response : response;
  }
  else if (command == "INCRBY" && value.array.size() >= 3)
  {
    return incr_by(value, storage, response) ? response : response;
  }
  else
  {
    RespValue err;
    err.type = RespType::ERROR;
    err.str.reserve(42 + command.size());
    err.str = "ERR unknown command ";
    err.str += command;
    err.str += " or wrong number of arguments";
    response = serialise(err);
  }

  return response;
}