
#if defined(_WIN32)
#include <curses.h>
#else
#include <ncurses.h>
#endif
#include <string>

struct PythonLikeWindow {
    WINDOW* win;

    // Emulate: win.attron(attr)
    void attron(int attr) {
        ::wattron(win, attr);
    }

    // Emulate: win.attroff(attr)
    void attroff(int attr) {
        ::wattroff(win, attr);
    }

    // Emulate helper 1: win.addstr("text")
    void addstr(const std::string& text) {
        ::waddstr(win, text.c_str());
    }

    // Emulate helper 2: win.addstr(y, x, "text")
    void addstr(int y, int x, const std::string& text) {
        ::mvwaddstr(win, y, x, text.c_str());
    }

    // Emulate helper 3: win.addstr(y, x, "text", attr)
    void addstr(int y, int x, const std::string& text, int attr) {
        ::wattron(win, attr);
        ::mvwaddstr(win, y, x, text.c_str());
        ::wattroff(win, attr);
    }

    void refresh() {
        ::wrefresh(win);
    }
};
