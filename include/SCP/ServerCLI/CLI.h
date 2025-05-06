#ifndef SCP_SERVERCLI_CLI_H_
#define SCP_SERVERCLI_CLI_H_

#include <cstdint>
#include <string>
#include <cstring>

#include "SCP/Server/ChatServer.h"

namespace SCP::ServerCLI
{
    class CLI
    {
    private:
        bool m_Running;
        SCP::Server::ChatServer m_Server;
    public:
        CLI();
        ~CLI();

        void Run();
    };
}

#endif