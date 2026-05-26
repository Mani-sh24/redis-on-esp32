#include "commands/db_commands.hpp"
#include "resp/serialiser.hpp"
#include "utils/string_helpers.hpp"

#include <stdexcept>
#include <chrono>

bool setKeys(const RespValue &value, Cache &storage, std::string &response)
{
    RespValue res;

    if (value.type != RespType::ARRAY || value.array.size() < 3)
    {
        res.type = RespType::ERROR;
        res.str = "ERR invalid arguments";
        response = serialise(res);
        return false;
    }

    if (value.array[1].type != RespType::BULK || value.array[2].type != RespType::BULK)
    {
        res.type = RespType::ERROR;
        res.str = "ERR invalid arguments";
        response = serialise(res);
        return false;
    }

    try
    {
        auto parsed = parse_set_options(value.array);

        if (parsed.has_expiry)
        {
            long long ttl_ms = std::chrono::duration_cast<std::chrono::milliseconds>(parsed.ttl).count();

            storage.put(
                value.array[1].str,
                value.array[2].str,
                ttl_ms
            );
        }
        else
        {
            storage.put(
                value.array[1].str,
                value.array[2].str
            );
        }

        res.type = RespType::STRING;
        res.str = "OK";
    }
    catch (const std::exception &e)
    {
        res.type = RespType::ERROR;
        res.str = e.what();
        response = serialise(res);
        return false;
    }

    response = serialise(res);
    return true;
}

bool getKeys(const RespValue &value, Cache &storage, std::string &response)
{
    RespValue res;

    if (value.type != RespType::ARRAY || value.array.size() < 2)
    {
        res.type = RespType::ERROR;
        res.str = "ERR invalid arguments";
        response = serialise(res);
        return false;
    }

    if (value.array[1].type != RespType::BULK)
    {
        res.type = RespType::ERROR;
        res.str = "ERR invalid arguments";
        response = serialise(res);
        return false;
    }

    if (!storage.exists(value.array[1].str))
    {
        res.type = RespType::BULK_NULL;
    }
    else
    {
        res.type = RespType::BULK;
        res.str = storage.get(value.array[1].str);
    }

    response = serialise(res);
    return true;
}

ParseResults parse_set_options(const std::vector<RespValue> &args)
{
    ParseResults result;

    int i = 3;

    while (i < args.size())
    {
        std::string token = to_upper(args[i].str);

        if (token == "EX" || token == "PX")
        {
            if (i + 1 >= args.size())
                throw std::runtime_error("ERR missing expiry");

            long long ttl = stoll(args[i+1].str);

            if (ttl < 0)
                throw std::runtime_error("ERR invalid ttl");

            if (result.has_expiry)
                throw std::runtime_error("ERR multiple expiry options");

            result.has_expiry = true;

            if (token == "EX")
                result.ttl = std::chrono::seconds(ttl);
            else
                result.ttl = std::chrono::milliseconds(ttl);

            i += 2;
        }
        else
        {
            throw std::runtime_error("ERR unknown option " + token);
        }
    }

    return result;
}

bool incr(const RespValue &value, Cache &storage, std::string &response)
{
    RespValue res;

    if (value.type != RespType::ARRAY || value.array.size() < 2)
    {
        res.type = RespType::ERROR;
        res.str = "Error invalid arguments!";
        response = serialise(res);
        return false;
    }

    if (value.array[1].type != RespType::BULK)
    {
        res.type = RespType::ERROR;
        res.str = "Error invalid arguments!";
        response = serialise(res);
        return false;
    }

    const auto &key = value.array[1].str;

    if (storage.exists(key))
    {
        auto parsed = parse_int(storage.get(key));
        if (!parsed)
        {
            res.type = RespType::ERROR;
            res.str = "ERR value is not an integer";
            response = serialise(res);
            return false;
        }

        int num = *parsed + 1;

        storage.put(key, std::to_string(num));
        res.type = RespType::INTEGER;
        res.integer = num;
        response = serialise(res);
        return true;
    }

    storage.put(key, "1");
    res.type = RespType::INTEGER;
    res.integer = 1;
    response = serialise(res);
    return true;
}

bool decr(const RespValue &value, Cache &storage, std::string &response)
{
    RespValue res;
    if (value.type != RespType::ARRAY || value.array.size() < 2)
    {
        res.type = RespType::ERROR;
        res.str = "Error invalid arguments!";
        response = serialise(res);
        return false;
    }
    
    if (value.array[1].type != RespType::BULK)
    {
        res.type = RespType::ERROR;
        res.str = "Error invalid arguments!";
        response = serialise(res);
        return false;
    }

    const auto &key = value.array[1].str;

    if (!storage.exists(key))
    {
        res.type = RespType::ERROR;
        res.str = "Key doesnt exist!";
        response = serialise(res);
        return false;        
    }

    auto parsed = parse_int(storage.get(key));

    if (!parsed)
    {
        res.type = RespType::ERROR;
        res.str = "ERR value is not an integer";
        response = serialise(res);
        return false;
    }

    int num = *parsed - 1;
    storage.put(key, std::to_string(num));
    res.type = RespType::INTEGER;
    res.integer = num;
    response = serialise(res);
    return true;
}

bool incr_by(const RespValue &value, Cache &storage, std::string &response)
{
    RespValue res;

    if (value.type != RespType::ARRAY || value.array.size() < 3)
    {
        res.type = RespType::ERROR;
        res.str = "Error invalid arguments!";
        response = serialise(res);
        return false;
    }

    if (value.array[1].type != RespType::BULK)
    {
        res.type = RespType::ERROR;
        res.str = "Error invalid arguments!";
        response = serialise(res);
        return false;
    }

    const auto &key = value.array[1].str;
    auto num_by = parse_int(value.array[2].str);

    if (!num_by)
    {
        res.type = RespType::ERROR;
        res.str = "ERR value is not an integer";
        response = serialise(res);
        return false;
    }

    if (storage.exists(key))
    {
        auto parsed = parse_int(storage.get(key));
        
        if (!parsed)
        {
            res.type = RespType::ERROR;
            res.str = "ERR value is not an integer";
            response = serialise(res);
            return false;
        }
        int num = *parsed + *num_by;

        storage.put(key, std::to_string(num));
        res.type = RespType::INTEGER;
        res.integer = num;
        response = serialise(res);
        return true;
    }

    storage.put(key, std::to_string(*num_by));
    res.type = RespType::INTEGER;
    res.integer = *num_by;
    response = serialise(res);
    return true;
}

