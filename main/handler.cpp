#include "helpers.h"
#include "Cache.hpp"

Cache storage(true);

std::string handle_value(const RespValue &value)
{
  std::string response;

  if (value.type != RespType::ARRAY || value.array.empty())
    return response;

  std::string command = value.array[0].str;
  
  for (auto &c : command)
    c = toupper(c);

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
  else
  {
    RespValue err;
    err.type = RespType::ERROR;
    err.str = "ERR unknown command or wrong number of arguments";
    response = serialise(err);
  }

  return response;
}