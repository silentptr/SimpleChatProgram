#ifndef SCP_SERVER_CHATSERVER_H_
#define SCP_SERVER_CHATSERVER_H_

#include <cstdint>
#include <vector>
#include <memory>

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
#include <boost/thread.hpp>

namespace SCP::Server
{
    class ChatRoom;
    
    class Client : public std::enable_shared_from_this<Client>
    {
        struct Private{ explicit Private() = default; };
    private:
        std::shared_ptr<ChatRoom> m_ChatRoom;
        boost::uuids::uuid m_UUID;
        boost::asio::ip::tcp::socket m_Socket;
        char m_Buffer[1024];
    public:
        inline Client(Private, std::shared_ptr<ChatRoom> cr, boost::uuids::uuid uuid, boost::asio::ip::tcp::socket s) : m_ChatRoom(cr), m_UUID(uuid), m_Socket(std::move(s)) { }
        ~Client();

        inline static std::shared_ptr<Client> Create(std::shared_ptr<ChatRoom> cr, boost::uuids::uuid uuid, boost::asio::ip::tcp::socket s)
        {
            return std::make_shared<Client>(Private(), cr, uuid, std::move(s));
        }

        boost::asio::awaitable<void> DoRead();
    };

    class ChatRoom : public std::enable_shared_from_this<ChatRoom>
    {
        struct Private{ explicit Private() = default; };
    private:
        boost::unordered_map<boost::uuids::uuid, std::shared_ptr<Client>> m_Clients;
    public:
        inline ChatRoom(Private) { }

        inline static std::shared_ptr<ChatRoom> Create() { return std::make_shared<ChatRoom>(Private()); }

        boost::asio::awaitable<void> CreateClient(boost::asio::ip::tcp::socket);
        inline void RemoveClient(const boost::uuids::uuid& uuid) { m_Clients.erase(uuid); }
    };

    class ChatServer
    {
    private:
        static constexpr char m_Header[8] = {2, 3, 1, 77, 30, 115, 33, 49};

        boost::asio::io_context m_IOCtx;
        boost::thread m_ServerThread;
        bool m_Running;
        std::uint16_t m_Port;

        std::shared_ptr<ChatRoom> m_ChatRoom;

        boost::asio::awaitable<void> ServerLoop();
        boost::asio::awaitable<void> HandleConn(boost::asio::ip::tcp::socket);
    public:
        ChatServer();
        ~ChatServer();

        inline bool IsRunning() const noexcept { return m_Running; }
        inline std::uint16_t Port() const noexcept { return m_Port; }

        bool Start(std::uint16_t);
        bool Stop();
    };
}

#endif