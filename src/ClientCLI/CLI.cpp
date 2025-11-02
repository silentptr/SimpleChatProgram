#include "SCP/ClientCLI/CLI.h"

#include <ncurses.h>

namespace SCP::ClientCLI
{
    CLI::CLI()
    {
        m_Messages.reserve(255);
        initscr();
    }

    CLI::~CLI()
    {
        endwin();
    }

    void CLI::DoStuff()
    {
        nodelay(stdscr, FALSE);
        nocbreak();
        std::uint16_t port = 0;
        std::string ip, username;
        char inputBuffer[255];

        addstr("test");
        move(0, 0);
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
        m_ConnectDone = false;
        Start(ip, port, username);

        clear();
        printw("Connecting to %s:%hu...\n", ip.c_str(), port);
        printw("Press C to cancel");
        refresh();
        nodelay(stdscr, TRUE);

        while (!m_ConnectDone)
        {
            int thing = getch();

            if (thing == 'C' || thing == 'c')
            {
                return;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }

        nodelay(stdscr, FALSE);
        nocbreak();
        clear();

        if (m_ErrMsg.has_value())
        {
            printw("Error when connecting: %s\nPress any key to continue...", m_ErrMsg.value().c_str());
            getch();
            return;
        }

        std::string currentMessage;
        halfdelay(2);
        clear();

        while (GetState() == SCP::Client::ChatClientState::Connected)
        {
            std::string msg;
            
            if (m_MsgQueue.try_dequeue(msg))
            {
                if (m_Messages.size() == 255)
                {
                    m_Messages.pop_back();
                }

                m_Messages.push_back(std::move(msg));
            }

            erase();
            move(0, 0);
            printw("Connected to %s:%hu as %s", ip.c_str(), port, username.c_str());
            int maxY = getmaxy(stdscr);
            int messageIndex = m_Messages.size() - 1;
            int chatRows = maxY - 4;

            for (int rowIndex = maxY - 3 - (m_Messages.size() >= chatRows ? 0 : chatRows - m_Messages.size()); rowIndex > 1; --rowIndex)
            {
                if (messageIndex > -1)
                {
                    mvprintw(rowIndex, 0, m_Messages[messageIndex].c_str());
                }

                --messageIndex;
            }

            mvprintw(maxY - 1, 0, "Enter message: %s", currentMessage.c_str());
            int c = getch();

            if (c != ERR)
            {
                switch (c)
                {
                case '\b':
                case 127:
                    if (!currentMessage.empty())
                    {
                        currentMessage.pop_back();
                    }
                    
                    break;
                case '\r':
                case '\n':
                case '\f':
                    if (!currentMessage.empty())
                    {
                        SendMessage(currentMessage);
                        currentMessage.clear();
                    }
                    
                    break;
                default:
                    currentMessage += static_cast<char>(c);
                    break;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    void CLI::Run()
    {
        bool exit = false;

        do
        {
            DoStuff();

            nodelay(stdscr, FALSE);
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
        m_ErrMsg = std::move(str);
        m_ConnectDone = true;
    }

    void CLI::OnChatMessage(std::string msg)
    {
        m_MsgQueue.enqueue(std::move(msg));
    }

    void CLI::OnDisconnect(std::optional<std::string> str)
    {
        m_ErrMsg = std::move(str);
    }
}