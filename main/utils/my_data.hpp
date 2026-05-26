#ifndef UTILS_MY_DATA_HPP
#define UTILS_MY_DATA_HPP

#include "sdkconfig.h"

#ifndef CONFIG_ESP_WIFI_SSID
#define SSID ""
#else
#define SSID CONFIG_ESP_WIFI_SSID
#endif

#ifndef CONFIG_ESP_WIFI_PASSWORD
#define PASS ""
#else
#define PASS CONFIG_ESP_WIFI_PASSWORD
#endif

#endif // UTILS_MY_DATA_HPP
