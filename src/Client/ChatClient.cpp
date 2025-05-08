#include "SCP/Client/ChatClient.h"

namespace SCP::Client
{
    ChatClientEventHandler::ChatClientEventHandler() noexcept { }
    ChatClientEventHandler::~ChatClientEventHandler() noexcept { }
    void ChatClientEventHandler::OnConnect(std::optional<std::string>) { }
    void ChatClientEventHandler::OnChatMessage(std::string) { }
    void ChatClientEventHandler::OnDisconnect(std::optional<std::string>) { }

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
        try
        {
            auto [connectError, endpoint] = co_await boost::asio::async_connect(m_Socket, co_await m_Resolver.async_resolve(m_IP, std::to_string(m_Port)), boost::asio::as_tuple(boost::asio::use_awaitable));

            if (connectError != boost::system::errc::success)
            {
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
                co_return;
            }

            std::uint8_t uuidBuffer[16];
            auto [readError, readLen] = co_await m_Socket.async_read_some(boost::asio::buffer(uuidBuffer), boost::asio::as_tuple(boost::asio::use_awaitable));

            if (readError != boost::system::errc::success || readLen != 16)
            {
                SilentSockClose();
                co_return;
            }

            m_UUID = boost::uuids::uuid(uuidBuffer);
            m_State.store(ChatClientState::Connected);
            m_EventHandler.OnConnect(std::nullopt);
            co_await DoRead();
        }
        catch (const boost::system::system_error& e)
        {
            std::cerr << e.what() << '\n';
        }
    }

    boost::asio::awaitable<void> ChatClient::DoRead()
    {
        while (m_State.load() == ChatClientState::Connected)
        {
            auto [sizeError, sizeLen] = co_await m_Socket.async_read_some(boost::asio::buffer(m_Buffer, 2), boost::asio::as_tuple(boost::asio::use_awaitable));

            if (sizeError != boost::system::errc::success || sizeLen != 2)
            {
                StopWithError(sizeError.message());
                co_return;
            }

            std::uint16_t msgLen = boost::endian::load_big_u16(m_Buffer);
            auto [msgError, msgReadLen] = co_await m_Socket.async_read_some(boost::asio::buffer(m_Buffer, msgLen), boost::asio::as_tuple(boost::asio::use_awaitable));

            if (msgError != boost::system::errc::success || msgReadLen != msgLen)
            {
                StopWithError(msgError.message());
                co_return;
            }

            m_EventHandler.OnChatMessage(std::string(reinterpret_cast<char*>(m_Buffer), msgLen));
        }

        Stop();
    }

    boost::asio::awaitable<void> ChatClient::DoWrite(std::string msg)
    {
        unsigned char writeBuffer[65535+2];
        boost::endian::store_big_u16(writeBuffer, msg.size());
        std::memcpy(writeBuffer+2, msg.c_str(), msg.size());
        co_await boost::asio::async_write(m_Socket, boost::asio::buffer(writeBuffer, msg.size() + 2), boost::asio::use_awaitable);
    }

    void ChatClient::SendMessage(const std::string& msg)
    {
        boost::asio::co_spawn(m_IOCtx, DoWrite(msg), boost::asio::detached);
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
        m_Thread = std::jthread([&](){ m_IOCtx.run(); });
        return true;
    }

    bool ChatClient::StopWithError(std::string err)
    {
        ChatClientState expected = ChatClientState::Connected;

        if (m_State.compare_exchange_strong(expected, ChatClientState::Inactive))
        {
            SilentSockClose();
            m_EventHandler.OnDisconnect(err);
            try { m_IOCtx.stop(); } catch (...) { }
            return true;
        }
        else
        {
            return false;
        }
    }

    bool ChatClient::Stop()
    {
        ChatClientState expected = ChatClientState::Connected;

        if (m_State.compare_exchange_strong(expected, ChatClientState::Inactive))
        {
            SilentSockClose();
            m_EventHandler.OnDisconnect(std::nullopt);
            try { m_IOCtx.stop(); } catch (...) { }
            return true;
        }
        else
        {
            return false;
        }
    }

    ChatClient::ChatClient(ChatClientEventHandler& handler) : m_IOCtx(1), m_Resolver(m_IOCtx), m_Socket(m_IOCtx), m_EventHandler(handler), m_State(ChatClientState::Inactive)
    {
        
    }

    ChatClient::~ChatClient()
    {
        
    }
}