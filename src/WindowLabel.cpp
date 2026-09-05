


#include "WindowLabel.h"
#include "Window.h"

#include <string>


WindowLabel::WindowLabel(int x, int y, std::string const& str, WINDOW* parent)
: win(parent?derwin(parent, 1, str.length(), y, x):
            newwin(1, str.length(), y, x))
, str(str)
, maxx(x+str.length())
{
    draw();
}
WindowLabel::~WindowLabel()
{
    delwin(win);
}
int WindowLabel::x()const
{
    return getbegx(win);
}
int WindowLabel::y()const
{
    return getbegy(win);
}

void WindowLabel::draw(bool const bold)const
{
    init_color(COLOR_CYAN, 20, 20, 255);
    init_pair(3, COLOR_WHITE, COLOR_CYAN);
    wattrset(win, COLOR_PAIR(3)|(bold*A_BOLD)|(bold*A_REVERSE));
    mvwprintw(win, 0, 0,
        (std::string("%")+std::to_string(windowWidth(win))+"s").c_str(),
        str.c_str());
    wattrset(win, COLOR_PAIR(0));
    wrefresh(win);
}
void WindowLabel::set(std::string const&s)
{
    str = s;
    wresize(win, 1, str.length());
    draw(true);
}

int WindowLabel::nextX()const
{
    return maxx+1;
    //return getmaxx(win)+1;
}

bool WindowLabel::handleMouseEvent(MEVENT event)
{
    return (event.bstate & BUTTON1_PRESSED || event.bstate & BUTTON1_CLICKED) &&
        windowContainsMouseEvent(win, event);
}