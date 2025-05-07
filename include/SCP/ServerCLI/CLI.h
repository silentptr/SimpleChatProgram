#ifndef SCP_SERVERCLI_CLI_H_
#define SCP_SERVERCLI_CLI_H_

#include <cstdint>
#include <string>
#include <cstring>
#include <vector>

#include <boost/lexical_cast.hpp>

#include "moodycamel/concurrentqueue.h"

#include "SCP/Server/ChatServer.h"

namespace SCP::ServerCLI
{
    class CLI : public SCP::Server::ChatServerEventHandler
    {
    private:
        bool m_Running;
        moodycamel::ConcurrentQueue<std::string> m_MsgQueue;
        std::vector<std::string> m_Messages;
        SCP::Server::ChatServer m_Server;
    public:
        CLI();
        ~CLI();

        void Run();

        void OnChatMessage(std::string) override;
    };
}

#endif