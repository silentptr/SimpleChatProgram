#include "SCP/Client/ChatClient.h"

#include <iostream>

namespace SCP::Client
{
    void ChatClient::SilentSockClose()
    {
        try
        {
            m_Socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        }
        catch (...) { }

        try
        {
            m_Socket.close();
        }
        catch (...) { }
    }

    boost::asio::awaitable<void> ChatClient::DoConnect()
    {
        std::cout << "Connecting to " << m_IP << ':' << std::to_string(m_Port) << '\n';
        auto [connectError, endpoint] = co_await boost::asio::async_connect(m_Socket, co_await m_Resolver.async_resolve(m_IP, std::to_string(m_Port)), boost::asio::as_tuple(boost::asio::use_awaitable));

        if (connectError != boost::system::errc::success)
        {
            m_EventHandler.OnConnect("1: " + connectError.message());
            co_return;
        }

        char buffer[28];
        std::memcpy(buffer, m_Header, sizeof(m_Header));
        std::memset(buffer + sizeof(m_Header), 0, sizeof(buffer) - sizeof(m_Header));
        std::memcpy(buffer + sizeof(m_Header), m_Username.c_str(), m_Username.length());
        auto [handshakeError, written] = co_await boost::asio::async_write(m_Socket, boost::asio::buffer(buffer), boost::asio::as_tuple(boost::asio::use_awaitable));

        if (handshakeError != boost::system::errc::success || written != sizeof(buffer))
        {
            SilentSockClose();
            m_EventHandler.OnConnect("2: " + connectError.message());
            co_return;
        }

        std::uint8_t uuidBuffer[16];
        auto [readError, readLen] = co_await m_Socket.async_read_some(boost::asio::buffer(uuidBuffer), boost::asio::as_tuple(boost::asio::use_awaitable));

        if (readError != boost::system::errc::success || readLen != 16)
        {
            SilentSockClose();
            m_EventHandler.OnConnect("3: " + connectError.message());
            co_return;
        }

        m_UUID = boost::uuids::uuid(uuidBuffer);
        m_State.store(ChatClientState::Connected);
        m_EventHandler.OnConnect(std::nullopt);
    }

    bool ChatClient::Start(const std::string& ip, std::uint16_t port, const std::string& username)
    {
        ChatClientState expected = ChatClientState::Inactive;

        if (!m_State.compare_exchange_strong(expected, ChatClientState::Connecting))
        {
            return false;
        }
        
        m_IP = ip;
        m_Port = port;
        m_Username = username;
        boost::asio::co_spawn(m_IOCtx, DoConnect(), boost::asio::detached);
        return true;
    }

    bool ChatClient::Stop()
    {
        ChatClientState expected = ChatClientState::Connected;

        if (m_State.compare_exchange_strong(expected, ChatClientState::Inactive))
        {
            SilentSockClose();
            return true;
        }
        else
        {
            return false;
        }
    }

    ChatClient::ChatClient(ChatClientEventHandler& handler) : m_IOCtx(1), m_Resolver(m_IOCtx), m_Socket(m_IOCtx), m_EventHandler(handler), m_State(ChatClientState::Inactive)
    {
        m_Thread = std::thread([&](){ m_IOCtx.run(); });
    }

    ChatClient::~ChatClient()
    {
        m_IOCtx.stop();
        m_Thread.join();
    }
}