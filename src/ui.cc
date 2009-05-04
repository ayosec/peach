/*
 * Peach Finder: Peach Finder
 * Copyright (c) 2009 Ayose Cazorla
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <sys/ioctl.h>
#include <signal.h>
#include "peach.h"

WINDOW *entry_window = NULL;
WINDOW *files_window = NULL;

namespace UI {

    void build_ui()
    {
        if(entry_window)
            delwin(entry_window);

        if(files_window)
            delwin(files_window);

        entry_window = newwin(1, 0, 0, 0);
        files_window = newwin(0, 0, 1, 0);

        keypad(entry_window, TRUE);
    }

    void update_windows()
    {
        Files::update_window();
        Entry::update_window();
    }

    void update_term_size()
    {
        struct winsize winsz;
        ioctl(0, TIOCGWINSZ, &winsz);
        resizeterm(winsz.ws_row, winsz.ws_col);

        /* Recreate the windows */
        build_ui();
        update_windows();
    }

    static void _exit()
    {
        endwin();
    }

    void int_hanlder(int)
    {
        exit(0);
    }

    void init()
    {
        /* ncurses stuff */
        atexit(_exit);
        initscr();
        nonl();
        cbreak();
        noecho();

        build_ui();

        signal(SIGINT, int_hanlder);
    }

}
