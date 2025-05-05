#include "SCP/Server/ChatServer.h"

using boost::asio::co_spawn;

// references:
// https://www.boost.org/doc/libs/1_88_0/doc/html/boost_asio/example/cpp20/coroutines/echo_server.cpp

namespace SCP::Server
{
    Client::~Client()
    {
        try
        {
            m_Socket.shutdown(boost::asio::socket_base::shutdown_both);
            m_Socket.close();
        }
        catch (...) { }
    }

    boost::asio::awaitable<void> Client::DoRead()
    {
        while (true)
        {
            auto [lenError, lenRead] = co_await m_Socket.async_read_some(boost::asio::buffer(m_Buffer, 1), boost::asio::as_tuple(boost::asio::use_awaitable));

            if (lenError != boost::system::errc::success || lenRead != 1)
            {
                break;
            }

            std::uint8_t msgLen = static_cast<std::uint8_t>(m_Buffer[0]);
            auto [error, read] = co_await m_Socket.async_read_some(boost::asio::buffer(m_Buffer, msgLen), boost::asio::as_tuple(boost::asio::use_awaitable));

            if (error != boost::system::errc::success || read != msgLen)
            {
                break;
            }
        }

        m_ChatRoom->RemoveClient(m_UUID);
    }

    boost::asio::awaitable<void> ChatRoom::CreateClient(boost::asio::ip::tcp::socket sock)
    {
        auto uuid = boost::uuids::random_generator()();
        //auto pair = m_Clients.emplace(uuid, std::make_shared<Client>(shared_from_this(), uuid, std::move(sock)));
        auto pair = m_Clients.emplace(uuid, Client::Create(shared_from_this(), uuid, std::move(sock)));
        auto executor = co_await boost::asio::this_coro::executor;
        boost::asio::co_spawn(executor, (*pair.first).second->DoRead(), boost::asio::detached);
    }

    ChatServer::ChatServer() : m_IOCtx(1), m_Running(false)
    {
        
    }

    ChatServer::~ChatServer()
    {
        m_IOCtx.stop();
    }

    bool ChatServer::Start(std::uint16_t port)
    {
        if (m_Running)
        {
            return false;
        }

        m_Running = true;
        m_Port = port;
        boost::asio::co_spawn(m_IOCtx, ServerLoop(), boost::asio::detached);
        m_ServerThread = boost::thread(boost::bind(&ChatServer::ServerLoop, this));
        m_IOCtx.restart();
        m_IOCtx.run();
        return true;
    }

    bool ChatServer::Stop()
    {
        if (m_Running)
        {
            m_Running = false;
            m_IOCtx.stop();
            return true;
        }
        else
        {
            return false;
        }
    }

    boost::asio::awaitable<void> ChatServer::ServerLoop()
    {
        auto executor = co_await boost::asio::this_coro::executor;
        boost::asio::ip::tcp::acceptor acceptor(executor, {boost::asio::ip::tcp::v4(), m_Port});

        while (true)
        {
            auto [error, sock] = co_await acceptor.async_accept(boost::asio::as_tuple(boost::asio::use_awaitable));

            if (error == boost::asio::error::shut_down)
            {
                break;
            }

            boost::asio::co_spawn(executor, HandleConn(std::move(sock)), boost::asio::detached);
        }
    }

    boost::asio::awaitable<void> ChatServer::HandleConn(boost::asio::ip::tcp::socket sock)
    {
        char buffer[28];
        auto [error, read] = co_await sock.async_read_some(boost::asio::buffer(buffer), boost::asio::as_tuple(boost::asio::use_awaitable));

        if (error != boost::system::errc::success || read != 28 || std::memcmp(m_Header, buffer, 8) != 0)
        {
            sock.shutdown(boost::asio::socket_base::shutdown_both);
            sock.close();
            co_return;
        }

        co_await m_ChatRoom->CreateClient(std::move(sock));
    }
}