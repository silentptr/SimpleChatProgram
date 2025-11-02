#include "SCP/ServerCLI/CLI.h"

#include <ncurses.h>

namespace SCP::ServerCLI
{
    CLI::CLI()
    {
        initscr();
        m_Messages.reserve(255);
    }

    CLI::~CLI()
    {
        endwin();
    }

    void CLI::WaitForCallback()
    {
        std::unique_lock lock(m_Mutex);
        m_CondVar.wait(lock, [this]{return !m_Waiting;});
    }

    void CLI::Run()
    {
        std::uint16_t port = 0;
        char inputBuffer[32];

        while (port == 0)
        {
            clear();
            printw("Enter port (q to quit): ");
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
        printw("Starting server on port %hu...\n", port);
        refresh();

        m_Waiting = true;
        Start(port);
        WaitForCallback();

        if (m_ErrMsg.has_value())
        {
            clear();
            printw("An error occured while starting server: %s\nPress any key to continue...", m_ErrMsg.value().c_str());
            getch();
            return;
        }

        halfdelay(2);

        while (IsRunning())
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
            printw("Server listening on port %hu.\n\n", port);

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

            mvprintw(maxY - 1, 0, "Press q to quit");

            int c = getch();

            if (c == 'q' || c == 'Q')
            {
                break;
            }

            try
            {
                std::this_thread::yield();
            }
            catch (...) { }
        }

        Stop();
        WaitForCallback();

        if (m_ErrMsg.has_value())
        {
            nodelay(stdscr, FALSE);
            nocbreak();
            clear();
            printw("Server stopped abnormally: %s\nPress any key to continue...", m_ErrMsg.value().c_str());
            getch();
        }
    }

    void CLI::OnServerStart(std::optional<std::string> err)
    {
        m_ErrMsg = std::move(err);
        {
            std::scoped_lock lock(m_Mutex);
            m_Waiting = false;
        }
        m_CondVar.notify_one();
    }

    void CLI::OnServerStop(std::optional<std::string> err)
    {
        m_ErrMsg = std::move(err);
        {
            std::scoped_lock lock(m_Mutex);
            m_Waiting = false;
        }
        m_CondVar.notify_one();
    }

    void CLI::OnChatMessage(std::string msg)
    {
        m_MsgQueue.enqueue(std::move(msg));
    }
}