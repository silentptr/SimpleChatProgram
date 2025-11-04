#ifndef SCP_CLIENTGUI_CONNECTINFO_H_
#define SCP_CLIENTGUI_CONNECTINFO_H_

#include <cstdint>
#include <string>

namespace SCP::ClientGUI
{
    struct ConnectInfo
    {
        std::string m_IP;
        std::uint16_t m_Port;
        std::string m_Username;
    };
}

#endif