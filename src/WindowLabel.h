#pragma once



#if defined(_WIN32)
#include <curses.h>
#else
#include <ncurses.h>
#endif
#include <functional>
#include <string>
#include <limits>
#include <string.h>

struct MouseEvent;

struct WindowLabel
{
    WindowLabel(int x, int y, std::string const& str, WINDOW* parent = nullptr);
    ~WindowLabel();

    void draw(bool bold=false)const;
    int nextX()const;
    bool handleMouseEvent(MouseEvent const& event);
    void set(std::string const&);
    int x()const;
    int y()const;

    WINDOW* win;
    std::string str;
    int maxx;
};
