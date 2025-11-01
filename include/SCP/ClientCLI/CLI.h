#ifndef SCP_CLIENTCLI_CLI_H_
#define SCP_CLIENTCLI_CLI_H_

#include <cstdint>
#include <string>
#include <cstring>
#include <vector>

#include <boost/lexical_cast.hpp>

#include "moodycamel/concurrentqueue.h"

#include "SCP/Client/ChatClient.h"

namespace SCP::ClientCLI
{
    class CLI : public SCP::Client::ChatClient
    {
    private:
        std::atomic_bool m_EventFinished;
        std::optional<std::string> m_ErrMsg;

        std::vector<std::string> m_Messages;
        moodycamel::ConcurrentQueue<std::string> m_MsgQueue;
        
        void DoStuff();
    public:
        CLI();
        ~CLI();

        void Run();

        void OnConnect(std::expected<void, std::string>) override;
        void OnChatMessage(std::string) override;
        void OnDisconnect(std::expected<void, std::string>) override;
    };
}

#endif