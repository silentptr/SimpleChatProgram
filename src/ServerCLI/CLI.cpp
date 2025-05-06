#include "SCP/ServerCLI/CLI.h"

#include <ncurses.h>

#include <boost/lexical_cast.hpp>

namespace SCP::ServerCLI
{
    CLI::CLI() : m_Running(false)
    {

    }

    CLI::~CLI()
    {
        if (m_Running)
        {
            endwin();
        }
    }

    void CLI::Run()
    {
        m_Running = true;
        initscr();
        std::uint16_t port = 0;
        char inputBuffer[32];

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
        printw("Starting server on port %hu...\n", port);
        refresh();
        m_Server.Start(port);

        while (1)
        {
            clear();
            printw("Server listening on port %hu.\n", port);
            int y = getmaxy(stdscr);
            mvprintw(y - 1, 0, "Press q to quit");

            int c = getch();

            if (c == ERR)
            {
                continue;
            }

            if (c == 'q' || c == 'Q')
            {
                break;
            }
        }
    }
}