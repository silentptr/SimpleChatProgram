#include "SCP/Client/ChatClient.h"

namespace SCP::Client
{
    bool ChatClient::Start(const std::string& ip, std::uint16_t port)
    {
        if (m_State != ChatClientState::Inactive)
        {
            return false;
        }

        boost::asio::ip::tcp::resolver resolver(m_IOCtx);
        co_await boost::asio::async_connect(m_Socket, co_await resolver.async_resolve(ip, std::to_string(port)));
        return true;
    }

    bool ChatClient::Stop()
    {
        if (m_Connected)
        {
            m_Connected = false;
            m_IOCtx.stop();
            return true;
        }
        else
        {
            return false;
        }
    }
}