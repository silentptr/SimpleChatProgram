#ifndef SCP_SERVERCLI_CLI_H_
#define SCP_SERVERCLI_CLI_H_

#include <cstdint>
#include <string>
#include <cstring>
#include <vector>
#include <atomic>

#include <boost/lexical_cast.hpp>

#include "moodycamel/concurrentqueue.h"

#include "SCP/Server/ChatServer.h"

namespace SCP::ServerCLI
{
    class CLI : public SCP::Server::ChatServer
    {
    private:
        std::atomic_bool m_EventFinished;
        std::optional<std::string> m_ErrMsg;
        
        SCP::Server::ChatServer m_Server;
        std::vector<std::string> m_Messages;
        moodycamel::ConcurrentQueue<std::string> m_MsgQueue;
    public:
        CLI();
        ~CLI();

        void Run();

        void OnServerStart(std::optional<std::string>) override;
        void OnServerStop(std::optional<std::string>) override;
        void OnChatMessage(std::string) override;
    };
}

#endif