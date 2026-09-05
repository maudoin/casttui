


#include "DialogNew.h"

#if defined(_WIN32)
#include <curses.h>
#else
#include <ncurses.h>
#endif
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void popupFieldDialog(WINDOW* parent)
{
    static constexpr int KEY_ESC = 0x1b;

    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(1);

    WINDOW* win_body = newwin(24, 80, 0, 0);
    box(win_body, 0, 0);
    mvwprintw(win_body, 1, 2, "New podcast");

    WINDOW* win_form = derwin(win_body, 20, 78, 3, 1);
    box(win_form, 0, 0);

    wrefresh(win_body);
    wrefresh(win_form);

    // --- Editable fields ---
    char url[256]    = "http://podacast.com";
    char folder[256] = "d:\\podcats\\newnewnew";

    int field = 0; // 0=url, 1=folder
    int pos[2] = { (int)strlen(url), (int)strlen(folder) };

    while (true)
    {
        // Draw labels
        mvwprintw(win_form, 2, 2, "url:");
        mvwprintw(win_form, 3, 2, "folder:");

        // Draw editable fields
        mvwprintw(win_form, 2, 10, "%-60s", url);
        mvwprintw(win_form, 3, 10, "%-60s", folder);

        // Move cursor
        wmove(win_form, 2 + field, 10 + pos[field]);

        wrefresh(win_form);

        int ch = wgetch(win_form);
        if (ch == KEY_ESC)
            break;

        char* buf = (field == 0 ? url : folder);
        int& p = pos[field];

        switch (ch)
        {
            case KEY_UP:
                field = 0;
                break;

            case KEY_DOWN:
                field = 1;
                break;

            case KEY_LEFT:
                if (p > 0) p--;
                break;

            case KEY_RIGHT:
                if (p < (int)strlen(buf)) p++;
                break;

            case KEY_BACKSPACE:
            case 127:
                if (p > 0) {
                    memmove(buf + p - 1, buf + p, strlen(buf) - p + 1);
                    p--;
                }
                break;

            case KEY_DC:
                if (p < (int)strlen(buf)) {
                    memmove(buf + p, buf + p + 1, strlen(buf) - p);
                }
                break;

            default:
                if (isprint(ch)) {
                    int len = strlen(buf);
                    if (len < 255) {
                        memmove(buf + p + 1, buf + p, len - p + 1);
                        buf[p] = ch;
                        p++;
                    }
                }
                break;
        }
    }

    delwin(win_form);
    delwin(win_body);
}
