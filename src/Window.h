


#pragma once

#if defined(_WIN32)
#include <curses.h>
#else
#include <ncurses.h>
#endif

#if defined(PDCURSES)
#define getmouse nc_getmouse
#endif

inline int windowHeight(WINDOW *window){return getmaxy(window) - getbegy(window);}
inline int windowWidth(WINDOW *window){return getmaxx(window) - getbegx(window);}
inline int windowContentHeight(WINDOW *window){return windowHeight(window) -1;}
inline int windowContentWidth(WINDOW *window){return windowWidth(window) -1;}

inline bool windowContainsMouseEvent(WINDOW *window, int event_x, int event_y)
{
    return (event_y >= getbegy(window) && event_y < (getbegy(window)+getmaxy(window)) &&
            event_x >= getbegx(window) && event_x < (getbegx(window)+getmaxx(window)));
}


inline int wgetch_escdelay(WINDOW* window,int delay_ms)
{
    int ch = wgetch(window);

    if (ch != 27)   // not ESC
        return ch;

    // ESC received — check if more keys follow
    timeout(delay_ms);
    int next = wgetch(window);
    timeout(-1);    // restore blocking mode

    if (next == ERR)
        return 27;  // real ESC key

    // Put back the second key into your own buffer or handle it
    return next;     // part of an escape sequence
}

enum class WindowLoopControl{CONTINUE, DONE};
/// @param handleAndContinue (int ch)->WindowLoopControl, return false to stop listening input
template<typename OP>
inline void loopWindowInput(WINDOW *window, OP const& handleAndContinue)
{
    keypad(window, true);
    while(WindowLoopControl::CONTINUE == handleAndContinue(wgetch_escdelay(window,0)));
}