#ifndef COMMANDS_HANDLER_HPP
#define COMMANDS_HANDLER_HPP

#include <string>
#include "resp/types.hpp"

std::string handle_value(const RespValue &value , ClientState &client);
std::string execute_cmd(const RespValue &value);

#endif // COMMANDS_HANDLER_HPP
