#include "utils/wifi.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "resp/deserialiser.hpp"
#include "commands/handler.hpp"

void handle_commands(int clientfd)
{
  ClientState client;
  client.fd = clientfd;
  char buffer[1024];

  while (true)
  {
    ssize_t bytes = recv(clientfd, buffer, sizeof(buffer), 0);
    if (bytes <= 0)
      break;
    client.accumulator.append(buffer, bytes);
    // Parse all complete frames from the client.accumulator
    int offset = 0;
    while (offset < (int)client.accumulator.size())
    {
      auto [value, consumed] = prcoess_parser(client.accumulator, offset);
      if (consumed < 0)
        break; // incomplete frame — wait for more data
      offset += consumed;
      std::string response = handle_value(value , client);
      if (!response.empty())
        send(clientfd, response.c_str(), response.length(), 0);
    }
    // Remove all fully processed bytes from the front
    client.accumulator.erase(0, offset);
  }

  close(clientfd);
}

void client_task(void *arg)
{
    int clientfd = (int)(intptr_t)arg;

    handle_commands(clientfd);

    vTaskDelete(NULL);
}

extern "C" void app_main()
{
    Wifi::connect();

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        std::cerr << "Socket creation failed\n";
        return;
    }

    int reuse = 1;

    setsockopt( server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in server_addr{};

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(6379);

    if (bind( server_fd, (sockaddr *)&server_addr, sizeof(server_addr)) != 0)
    {
        std::cerr << "Bind failed\n";
        close(server_fd);
        return;
    }

    if (listen(server_fd, 5) != 0)
    {
        std::cerr << "Listen failed\n";
        close(server_fd);
        return;
    }

    while (true)
    {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd, (sockaddr *)&client_addr, &client_len);

        if (client_fd < 0) continue;

        std::cout << "Client connected\n";
        xTaskCreate(
            client_task,
            "client_task",
            4096,
            (void *)(intptr_t)client_fd,
            5,
            NULL);
    }
}