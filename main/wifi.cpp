#include <iostream>
#include <cstring>
#include <string>

extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/ip4_addr.h"
#include "utils/my_data.hpp"
}

#include "utils/wifi.hpp"

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch(event_id)
    {
        case WIFI_EVENT_STA_START:
            std::cout<<"WiFi starting..."<<std::endl;
            esp_wifi_connect();
            break;

        case WIFI_EVENT_STA_CONNECTED:
            std::cout<<"Connected to router"<<std::endl;
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            std::cout<<"Disconnected! Reconnecting..."<<std::endl;
            esp_wifi_connect();
            break;

        case IP_EVENT_STA_GOT_IP:
        {
            ip_event_got_ip_t *event=(ip_event_got_ip_t*)event_data;

            std::cout<<"Got IP: "
                     <<(int)ip4_addr1(&event->ip_info.ip)<<"."
                     <<(int)ip4_addr2(&event->ip_info.ip)<<"."
                     <<(int)ip4_addr3(&event->ip_info.ip)<<"."
                     <<(int)ip4_addr4(&event->ip_info.ip)
                     <<std::endl;

            break;
        }

        default:
            break;
    }
}

void Wifi::connect()
{
    nvs_flash_init();

    esp_netif_init();

    esp_event_loop_create_default();

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_init = WIFI_INIT_CONFIG_DEFAULT();

    esp_wifi_init(&wifi_init);

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);

    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);

    wifi_config_t wifi_configuration={};

    strcpy((char*)wifi_configuration.sta.ssid,SSID);
    strcpy((char*)wifi_configuration.sta.password,PASS);

    esp_wifi_set_mode(WIFI_MODE_STA);

    esp_wifi_set_config(WIFI_IF_STA,&wifi_configuration);

    esp_wifi_start();
}
