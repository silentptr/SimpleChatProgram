#ifndef SCP_SERVER_CHATSERVER_H_
#define SCP_SERVER_CHATSERVER_H_

#include <cstdint>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/unordered_map.hpp>
#include <boost/uuid.hpp>
#include <boost/endian.hpp>

namespace SCP::Server
{
    class ChatServerEventHandler
    {
    public:
        ChatServerEventHandler() noexcept;
        virtual ~ChatServerEventHandler() noexcept;
        
        virtual void OnChatMessage(std::string);
    };

    class ChatRoom;
    
    class Client : public std::enable_shared_from_this<Client>
    {
        struct Private{ explicit Private() = default; };
    private:
        std::shared_ptr<ChatRoom> m_ChatRoom;
        boost::uuids::uuid m_UUID;
        std::string m_Username;
        boost::asio::ip::tcp::socket m_Socket;
        unsigned char m_Buffer[65535];
    public:
        inline Client(Private, std::shared_ptr<ChatRoom> cr, boost::uuids::uuid uuid, std::string username, boost::asio::ip::tcp::socket s) : m_ChatRoom(cr), m_UUID(uuid), m_Username(std::move(username)), m_Socket(std::move(s)) { }
        ~Client();

        static std::shared_ptr<Client> Create(std::shared_ptr<ChatRoom>, boost::uuids::uuid, std::string, boost::asio::ip::tcp::socket);

        boost::asio::awaitable<void> DoRead();
        boost::asio::awaitable<void> SendMessage(std::string);

        inline const std::string& GetUsername() const noexcept { return m_Username; }
    };

    class ChatRoom : public std::enable_shared_from_this<ChatRoom>
    {
        struct Private{ explicit Private() = default; };
    private:
        boost::unordered_map<boost::uuids::uuid, std::shared_ptr<Client>> m_Clients;
        ChatServerEventHandler& m_EventHandler;
    public:
        inline ChatRoom(Private, ChatServerEventHandler& h) : m_EventHandler(h) { }

        inline static std::shared_ptr<ChatRoom> Create(ChatServerEventHandler& h) { return std::make_shared<ChatRoom>(Private(), h); }

        boost::asio::awaitable<void> CreateClient(boost::asio::ip::tcp::socket, std::string);
        boost::asio::awaitable<void> RemoveClient(const boost::uuids::uuid&);
        boost::asio::awaitable<void> BroadcastMessage(std::string);

        inline ChatServerEventHandler& GetEventHandler() noexcept { return m_EventHandler; }
    };

    class ChatServer
    {
    private:
        static constexpr char m_Header[8] = {2, 3, 1, 77, 30, 115, 33, 49};

        boost::asio::io_context m_IOCtx;
        boost::asio::ip::tcp::acceptor m_Acceptor;
        std::jthread m_ServerThread;
        std::atomic_bool m_Running;
        std::uint16_t m_Port;

        ChatServerEventHandler& m_EventHandler;

        std::shared_ptr<ChatRoom> m_ChatRoom;

        void DoAccept();
        boost::asio::awaitable<void> HandleConn(boost::asio::ip::tcp::socket);
        
        void StopWithError(std::string);
    public:
        ChatServer(ChatServerEventHandler&);
        ~ChatServer();

        inline bool IsRunning() const noexcept { return m_Running; }
        inline std::uint16_t Port() const noexcept { return m_Port; }

        bool Start(std::uint16_t);
        bool Stop();
    };
}

#endif