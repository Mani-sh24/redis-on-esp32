# Redis on ESP32 (esp-rediss)

A clean, modern, and lightweight C++ implementation of a Redis-compatible database server running on the ESP32 microcontroller using the **ESP-IDF** development framework.

---

## Features

- **RESP Deserialisation & Serialisation**: Fully parses and generates Redis Serialization Protocol (RESP) types:
  - Strings (`+`)
  - Errors (`-`)
  - Integers (`:`)
  - Bulk Strings (`$`)
  - Arrays (`*`)
- **Key-Value Store Cache**: Supports core key-value storage with:
  - Time-To-Live (TTL) expiration using `EX` (seconds) and `PX` (milliseconds) SET options.
  - Active and passive eviction of expired keys via a dedicated background cleaner FreeRTOS task.
- **Wi-Fi Connectivity**: Connects to your local Wi-Fi station asynchronously. Logs the server IP dynamically upon connection.
- **Multi-Client Handling**: Spawn FreeRTOS tasks to handle multiple TCP client connections concurrently over LwIP sockets on port `6379`.

---

## Codebase Architecture

The project has been refactored into domain-specific subdirectories under `main/` to separate interfaces and implementations, keep headers clean, and isolate type configurations:

```text
main/
├── cache/                  # Cache Component
│   ├── types.hpp           # CacheItem definition (value & expiry timepoint)
│   └── Cache.hpp           # Cache class declarations
├── resp/                   # RESP Component
│   ├── types.hpp           # RespType and RespValue definitions
│   ├── deserialiser.hpp    # Parser function declarations
│   └── serialiser.hpp      # Serialiser function declarations
├── commands/               # DB Commands Component
│   ├── types.hpp           # ParseResults (for options parsing)
│   ├── db_commands.hpp     # SET, GET, INCR, DECR, and INCRBY declarations
│   └── handler.hpp         # Command router interface
├── utils/                  # Utility Domain
│   ├── string_helpers.hpp  # to_upper and parse_int helper signatures
│   ├── wifi.hpp            # Wifi setup class declarations & IP helper
│   └── my_data.hpp         # Wi-Fi SSID and Password configuration
├── Cache.cpp               # Cache implementation (Clean Task & storage logic)
├── db_commands.cpp         # Command helper implementations
├── deserialiser.cpp        # RESP parsing and print functions
├── handler.cpp             # Command dispatching and routing
├── main.cpp                # App entry point (app_main), TCP socket listener
├── serialiser.cpp          # RESP serialization
├── server.cpp              # Mock/standalone server implementation
├── string_helpers.cpp      # String processing helpers (to_upper, parse_int)
└── wifi.cpp                # Wi-Fi driver init & event handlers
```

---

## Supported Redis Commands

| Command | Usage | Description |
|---|---|---|
| **PING** | `PING` | Responds with `+PONG\r\n` |
| **ECHO** | `ECHO <message>` | Echoes back the provided message. |
| **SET** | `SET <key> <value> [EX seconds \| PX milliseconds]` | Stores the string value. Optional expiry will evict the key after duration. |
| **GET** | `GET <key>` | Retrieves the value of the key. Returns `nil` if expired or non-existent. |
| **INCR** | `INCR <key>` | Increments the integer value of the key by 1. Creates it as `1` if non-existent. |
| **DECR** | `DECR <key>` | Decrements the integer value of the key by 1. Fails if the key doesn't exist. |
| **INCRBY** | `INCRBY <key> <increment>` | Increments the key value by the specified integer amount. |

---

## Building and Flashing

### Prerequisites
- ESP-IDF v6.0.1 (or compatible release) configured in your environment.

### 1. Configure Wi-Fi Credentials
Open `main/utils/my_data.hpp` and set your local network credentials:
```cpp
#define SSID "Your_WiFi_Name"
#define PASS "Your_WiFi_Password"
```

### 2. Clean and Build
Due to CMake source-globbing caches, perform a full clean before building if source files have been changed:
```bash
idf.py fullclean
idf.py build
```

### 3. Flash and Monitor
Flash the firmware onto your ESP32 board and launch the UART monitor to view logs:
```bash
idf.py flash monitor
```

---

## Internal Eviction Flow

The cache storage engine runs a FreeRTOS cleaner task at configurable intervals (default is 5 seconds). 
1. When keys are set with expiry parameters (`EX`/`PX`), they are stored in a priority queue (`std::priority_queue`) ordered by the soonest expiration timepoint.
2. The cleaner task wakes up, compares the current time against the head of the priority queue, and removes all expired items from storage.
3. Passively, if a client tries to `GET` a key that has expired but has not been cleaned up by the background cleaner task yet, the database will identify the expiration, delete it, and return a `nil` response.
