#include "SCP/ClientCLI/CLI.h"

#include <ncurses.h>

namespace SCP::ClientCLI
{
    CLI::CLI() : m_Client(*this), m_FinishedConnect(false)
    {
        initscr();
        m_Messages.reserve(255);
    }

    CLI::~CLI()
    {
        endwin();
    }

    void CLI::Run()
    {
        std::uint16_t port = 0;
        std::string ip, username;
        char inputBuffer[32];

        printw("Enter IP: ");
        std::memset(inputBuffer, 0, sizeof(inputBuffer));
        getnstr(inputBuffer, 31);
        ip = inputBuffer;

        if (ip.size() == 1 && (ip[0] == 'q' || ip[0] == 'Q'))
        {
            return;
        }

        while (port == 0)
        {
            clear();
            printw("Enter port: ");
            std::memset(inputBuffer, 0, sizeof(inputBuffer));
            getnstr(inputBuffer, 5);

            if (inputBuffer[0] == 'q' || inputBuffer[0] == 'Q')
            {
                return;
            }

            try
            {
                port = boost::lexical_cast<std::uint16_t>(inputBuffer);
            }
            catch (...)
            {
                port = 0;
                continue;
            }
        }

        clear();
        printw("Enter username: ");
        std::memset(inputBuffer, 0, sizeof(inputBuffer));
        getnstr(inputBuffer, 19);
        username = inputBuffer;

        m_Client.Start(ip, port, username);

        clear();
        printw("Connecting to %s:%hu...\n", ip.c_str(), port);
        refresh();

        while (!m_FinishedConnect.load())
        {
            printw("test");
            refresh();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        if (m_ConnectErr.has_value())
        {
            clear();
            printw("Error when connecting: %s\n", m_ConnectErr.value().c_str());
            printw("Press any key to continue...");
            getch();
            return;
        }

        while (1)
        {
            std::string msg;
            
            if (m_MsgQueue.try_dequeue(msg))
            {
                if (m_Messages.size() == 100)
                {
                    m_Messages.pop_back();
                }

                m_Messages.push_back(std::move(msg));
            }

            erase();
            move(0, 0);
            printw("Connected to %s:%hu as %s", ip.c_str(), port, username.c_str());

            for (auto& message : m_Messages)
            {
                printw("%s\n", message.c_str());
            }

            int y = getmaxy(stdscr);
            mvprintw(y - 1, 0, "Press q to quit");
            refresh();

            int c = getch();

            if (c == ERR)
            {
                continue;
            }

            if (c == 'q' || c == 'Q')
            {
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void CLI::OnConnect(std::optional<std::string> str)
    {
        m_ConnectErr = str;
        m_FinishedConnect.store(true);
    }

    void CLI::OnChatMessage(std::string msg)
    {
        m_MsgQueue.enqueue(std::move(msg));
    }
}