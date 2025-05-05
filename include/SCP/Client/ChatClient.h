#ifndef SCP_SERVER_CHATCLIENT_H_
#define SCP_SERVER_CHATCLIENT_H_

#include <cstdint>
#include <string>

#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/uuid.hpp>
#include <boost/thread.hpp>

namespace SCP::Client
{
    class ChatClientEventHandler
    {
    
    };

    enum class ChatClientState
    {
        Inactive, Connecting, Connected
    };

    class ChatClient
    {
    private:
        static constexpr char m_Header[8] = {2, 3, 1, 77, 30, 115, 33, 49};

        ChatClientEventHandler& m_EventHandler;
    
        boost::asio::io_context m_IOCtx;
        boost::thread m_ServerThread;
        boost::asio::ip::tcp::socket m_Socket;
        ChatClientState m_State;
        std::uint16_t m_Port;

        boost::asio::awaitable<void> DoConnect(const std::string&, std::uint16_t);
    public:
        inline ChatClient(ChatClientEventHandler& handler) : m_IOCtx(1), m_Socket(m_IOCtx), m_EventHandler(handler), m_State(ChatClientState::Inactive)
        {
            m_IOCtx.run();
        }

        inline ~ChatClient()
        {
            m_IOCtx.stop();
        }

        inline ChatClientState GetState() const noexcept { return m_State; }

        bool Start(const std::string&, std::uint16_t);
        bool Stop();
    };
}

#endif