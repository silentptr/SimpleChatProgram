#ifndef SCP_SERVER_CHATCLIENT_H_
#define SCP_SERVER_CHATCLIENT_H_

#include <cstdint>
#include <string>
#include <atomic>
#include <optional>
#include <thread>

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
    public:
        // empty if success, otherwise contains an error message
        virtual void OnConnect(std::optional<std::string>) = 0;
    };

    enum class ChatClientState : std::uint8_t
    {
        Inactive = 0, Connecting = 1, Connected = 2
    };

    class ChatClient
    {
    private:
        static constexpr char m_Header[8] = {2, 3, 1, 77, 30, 115, 33, 49};

        ChatClientEventHandler& m_EventHandler;
    
        boost::asio::io_context m_IOCtx;
        std::thread m_Thread;
        boost::asio::ip::tcp::resolver m_Resolver;
        boost::thread m_ServerThread;
        boost::asio::ip::tcp::socket m_Socket;
        std::atomic<ChatClientState> m_State;
        std::string m_IP;
        std::uint16_t m_Port;
        std::string m_Username;
        boost::uuids::uuid m_UUID;

        void SilentSockClose();
        boost::asio::awaitable<void> DoConnect();
    public:
        ChatClient(ChatClientEventHandler&);
        ~ChatClient();

        inline ChatClientState GetState() const noexcept { return m_State; }

        bool Start(const std::string&, std::uint16_t, const std::string&);
        bool Stop();
    };
}

#endif