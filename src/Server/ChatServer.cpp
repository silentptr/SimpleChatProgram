#include "SCP/Server/ChatServer.h"

// references:
// https://www.boost.org/doc/libs/1_88_0/doc/html/boost_asio/example/cpp20/coroutines/echo_server.cpp

namespace SCP::Server
{
    ChatServerEventHandler::ChatServerEventHandler() noexcept { }
    ChatServerEventHandler::~ChatServerEventHandler() noexcept { }
    void ChatServerEventHandler::OnChatMessage(std::string) { }

    Client::~Client()
    {
        try
        {
            m_Socket.shutdown(boost::asio::socket_base::shutdown_both);
        }
        catch (...) { }
        try
        {
            m_Socket.close();
        }
        catch (...) { }
    }

    boost::asio::awaitable<void> Client::DoRead()
    {
        while (true)
        {
            auto [lenError, lenRead] = co_await m_Socket.async_read_some(boost::asio::buffer(m_Buffer, 2), boost::asio::as_tuple(boost::asio::use_awaitable));

            if (lenError != boost::system::errc::success || lenRead != 2)
            {
                break;
            }

            std::uint16_t msgLen = boost::endian::load_big_u16(m_Buffer);

            if (msgLen == 0)
            {
                break;
            }

            auto [error, read] = co_await m_Socket.async_read_some(boost::asio::buffer(m_Buffer, msgLen), boost::asio::as_tuple(boost::asio::use_awaitable));

            if (error != boost::system::errc::success || read != msgLen)
            {
                break;
            }

            std::string message = m_Username + ": " + std::string(reinterpret_cast<char*>(m_Buffer), msgLen);
            m_ChatRoom->GetEventHandler().OnChatMessage(message);
            co_await m_ChatRoom->BroadcastMessage(message);
        }

        co_await m_ChatRoom->RemoveClient(m_UUID);
    }

    boost::asio::awaitable<void> Client::SendMessage(std::string msg)
    {
        unsigned char buffer[65535+2];
        boost::endian::store_big_u16(buffer, msg.size());
        std::memcpy(buffer + 2, msg.c_str(), msg.size());
        co_await boost::asio::async_write(m_Socket, boost::asio::buffer(buffer, msg.size() + 2), boost::asio::use_awaitable);
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

        auto client = Client::Create(shared_from_this(), uuid, std::move(name), std::move(sock));
        m_Clients[uuid] = client;
        auto executor = co_await boost::asio::this_coro::executor;
        boost::asio::co_spawn(executor, BroadcastMessage(client->GetUsername() + " connected"), boost::asio::detached);
        co_await client->DoRead();
    }

    boost::asio::awaitable<void> ChatRoom::RemoveClient(const boost::uuids::uuid& uuid)
    {
        auto client = m_Clients[uuid];
        m_Clients.erase(uuid);
        auto executor = co_await boost::asio::this_coro::executor;
        boost::asio::co_spawn(executor, BroadcastMessage(client->GetUsername() + " disconnected"), boost::asio::detached);
    }

    boost::asio::awaitable<void> ChatRoom::BroadcastMessage(std::string msg)
    {
        m_EventHandler.OnChatMessage(msg);
        
        for (auto& c : m_Clients)
        {
            co_await c.second->SendMessage(msg);
        }
    }

    ChatServer::ChatServer(ChatServerEventHandler& h) : m_IOCtx(1), m_Running(false), m_EventHandler(h), m_ChatRoom(ChatRoom::Create(m_EventHandler)), m_Acceptor(m_IOCtx)
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
        boost::asio::ip::tcp::endpoint endpoint = {boost::asio::ip::tcp::v4(), m_Port};
        m_Acceptor.open(endpoint.protocol());
        m_Acceptor.bind(endpoint);
        m_Acceptor.listen();
        DoAccept();
        m_ServerThread = std::jthread([&](){ m_IOCtx.restart(); m_IOCtx.run(); });
        return true;
    }

    bool ChatServer::Stop()
    {
        bool expected = true;

        if (m_Running.compare_exchange_strong(expected, false))
        {
            m_Acceptor.close();
            m_IOCtx.stop();
            return true;
        }
        else
        {
            return false;
        }
    }

    void ChatServer::DoAccept()
    {
        m_Acceptor.async_accept([this](const boost::system::error_code& error, boost::asio::ip::tcp::socket sock)
        {
            if (!error)
            {
                boost::asio::co_spawn(m_IOCtx, HandleConn(std::move(sock)), boost::asio::detached);
            }

            if (m_Running)
            {
                DoAccept();
            }
        });
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