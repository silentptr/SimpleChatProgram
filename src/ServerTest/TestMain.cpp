#include <iostream>

#include "SCP/Server/ChatServer.h"

int main()
{
    SCP::Server::ChatServer server;
    std::uint16_t port = 1000;
    std::cout << "Starting server on port " << std::to_string(port) << "...\n";
    server.Start(1000);
    std::cout << "Server started. Waiting 5 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(5));
    server.Stop();
    std::cout << "Server stopped.\n";
    return 0;
}