#include <iostream>
#include <mutex>
#include <condition_variable>

#include "SCP/Client/ChatClient.h"

std::atomic_bool g_Flag(false);

class MyClient : public SCP::Client::ChatClientEventHandler
{
private:
    SCP::Client::ChatClient m_Client;
public:
    MyClient() : m_Client(*this)
    {
        std::cout << "Connecting...\n";
        std::cout << "Start returned " << std::boolalpha << m_Client.Start("localhost", 2048, "test") << '\n';
    }

    void OnConnect(std::optional<std::string> msg) override
    {
        if (msg.has_value())
        {
            std::cout << "Connect failed: " << msg.value() << '\n';
        }
        else
        {
            std::cout << "Connect worked!\n";
        }

        g_Flag.store(true);
    }
};

int main()
{
    MyClient client;

    while (!g_Flag.load())
    {

    }
    
    std::cout << "Flag.\n";
    return 0;
}