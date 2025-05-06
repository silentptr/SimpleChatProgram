#include "SCP/Server/ChatServer.h"

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

            m_ChatRoom->GetEventHandler().OnChatMessage(std::string(m_Buffer, msgLen));
        }

        m_ChatRoom->RemoveClient(m_UUID);
    }

    std::shared_ptr<Client> Client::Create(std::shared_ptr<ChatRoom> cr, boost::uuids::uuid uuid, std::string name, boost::asio::ip::tcp::socket s)
    {
        return std::make_shared<Client>(Private(), cr, uuid, name, std::move(s));
    }

    boost::asio::awaitable<void> ChatRoom::CreateClient(boost::asio::ip::tcp::socket sock, std::string name)
    {
        auto uuid = boost::uuids::random_generator()();
        auto [error, write] = co_await boost::asio::async_write(sock, boost::asio::buffer(uuid.data(), uuid.size()), boost::asio::as_tuple(boost::asio::use_awaitable));

        if (error != boost::system::errc::success || write != uuid.size())
        {
            try
            {
                sock.shutdown(boost::asio::socket_base::shutdown_both);
            }
            catch(...){ }
            try
            {
                sock.close();
            }
            catch(...){ }
            
            co_return;
        }

        auto client = Client::Create(shared_from_this(), uuid, name, std::move(sock));
        m_Clients[uuid] = client;
        co_await client->DoRead();
    }

    ChatServer::ChatServer(ChatServerEventHandler& h) : m_IOCtx(1), m_Running(false), m_EventHandler(h), m_ChatRoom(ChatRoom::Create(m_EventHandler))
    {
        
    }

    ChatServer::~ChatServer()
    {
        Stop();
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
        m_ServerThread = std::thread([&](){ m_IOCtx.restart(); m_IOCtx.run(); });
        return true;
    }

    bool ChatServer::Stop()
    {
        bool expected = true;

        if (m_Running.compare_exchange_strong(expected, false))
        {
            m_IOCtx.stop();
            m_ServerThread.join();
            return true;
        }
        else
        {
            return false;
        }
    }

    boost::asio::awaitable<void> ChatServer::ServerLoop()
    {
        try
        {
            auto executor = co_await boost::asio::this_coro::executor;
            boost::asio::ip::tcp::acceptor acceptor(executor, {boost::asio::ip::tcp::v4(), m_Port});

            while (m_Running)
            {
                auto [error, sock] = co_await acceptor.async_accept(boost::asio::as_tuple(boost::asio::use_awaitable));

                if (error == boost::asio::error::shut_down)
                {
                    break;
                }

                boost::asio::co_spawn(executor, HandleConn(std::move(sock)), boost::asio::detached);
            }
        }
        catch (const boost::system::system_error& e)
        {
            std::cerr << "Listening error: " << e.what() << '\n';
        }
    }

    boost::asio::awaitable<void> ChatServer::HandleConn(boost::asio::ip::tcp::socket sock)
    {
        char buffer[28];
        auto [error, read] = co_await sock.async_read_some(boost::asio::buffer(buffer), boost::asio::as_tuple(boost::asio::use_awaitable));

        if (error != boost::system::errc::success || read != 28 || std::memcmp(m_Header, buffer, 8) != 0)
        {
            try
            {
                sock.shutdown(boost::asio::socket_base::shutdown_both);
            }
            catch(...){ }
            try
            {
                sock.close();
            }
            catch(...){ }
            co_return;
        }

        std::string name(buffer + sizeof(m_Header));
        co_await m_ChatRoom->CreateClient(std::move(sock), name);
    }
}