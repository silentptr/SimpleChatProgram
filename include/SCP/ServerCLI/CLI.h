#ifndef SCP_SERVERCLI_CLI_H_
#define SCP_SERVERCLI_CLI_H_

#include <cstdint>
#include <string>
#include <cstring>
#include <vector>
#include <condition_variable>
#include <mutex>

#include <boost/lexical_cast.hpp>

#include "moodycamel/concurrentqueue.h"

#include "SCP/Server/ChatServer.h"

namespace SCP::ServerCLI
{
    class CLI : public SCP::Server::ChatServer
    {
    private:
        std::condition_variable m_CondVar;
        std::mutex m_Mutex;
        bool m_Waiting;
        std::optional<std::string> m_ErrMsg;

        void WaitForCallback();
        
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