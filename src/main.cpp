#include <curses.h>
#include <fmt/core.h>

int main() {
    initscr();
    printw("Hello from cross-platform curses!");
    refresh();
    getch();
    endwin();

    fmt::print("Exited curses.\n");
    return 0;
}
