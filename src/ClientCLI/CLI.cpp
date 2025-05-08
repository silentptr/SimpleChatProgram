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

    void CLI::DoStuff()
    {
        nodelay(stdscr, false);
        nocbreak();
        std::uint16_t port = 0;
        std::string ip, username;
        char inputBuffer[255];

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
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        if (m_ErrMsg.has_value())
        {
            clear();
            printw("Error when connecting: %s\n", m_ErrMsg.value().c_str());
            printw("Press any key to continue...");
            getch();
            return;
        }

        m_Connected = true;
        std::string currentMessage;
        halfdelay(2);

        while (m_Connected)
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
            printw("Connected to %s:%hu as %s\n\n", ip.c_str(), port, username.c_str());
            int maxY = getmaxy(stdscr);
            int maxX = getmaxx(stdscr);
            int maxMessages = maxY - 10;

            for (auto& message : m_Messages)
            {
                printw("%s\n", message.c_str());
            }


            mvprintw(maxY - 1, 0, "Enter message: %s", currentMessage.c_str());
            int c = getch();

            if (c != ERR)
            {
                switch (c)
                {
                case '\b':
                case 127:
                    currentMessage.pop_back();
                    break;
                case '\r':
                case '\n':
                case '\f':
                    m_Client.SendMessage(currentMessage);
                    currentMessage.clear();
                    break;
                default:
                    currentMessage += static_cast<char>(c);
                    break;
                }
            }

            try
            {
                std::this_thread::yield();
            }
            catch (...) { }
        }
    }

    void CLI::Run()
    {
        bool exit = false;

        do
        {
            DoStuff();

            nodelay(stdscr, false);
            nocbreak();
            clear();
            move(0, 0);
    
            if (m_ErrMsg.has_value())
            {
                printw("Abnormal disconnect from server: %s\n", m_ErrMsg.value().c_str());
            }
            else
            {
                printw("Disconnected from server.\n");
            }
    
            int response;
    
            do
            {
                printw("Connect to another server (Y or N)? ");
                response = getch();
            }
            while (response != 'Y' && response != 'y' && response != 'N' && response != 'n');

            if (response == 'n' || response == 'N')
            {
                exit = true;
            }
        } while (!exit);
    }

    void CLI::OnConnect(std::optional<std::string> str)
    {
        m_ErrMsg = str;
        m_FinishedConnect.store(true);
    }

    void CLI::OnChatMessage(std::string msg)
    {
        m_MsgQueue.enqueue(std::move(msg));
    }

    void CLI::OnDisconnect(std::optional<std::string> str)
    {
        m_ErrMsg = str;
        m_Connected = false;
    }
}