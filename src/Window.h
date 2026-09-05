


#pragma once

#if defined(_WIN32)
#include <curses.h>
#else
#include <ncurses.h>
#endif

inline int windowHeight(WINDOW *window){return getmaxy(window) - getbegy(window);}
inline int windowWidth(WINDOW *window){return getmaxx(window) - getbegx(window);}
inline int windowContentHeight(WINDOW *window){return windowHeight(window) -1;}
inline int windowContentWidth(WINDOW *window){return windowWidth(window) -1;}

inline bool windowContainsMouseEvent(WINDOW *window, MEVENT const& event)
{
    return (event.y >= getbegy(window) && event.y < (getbegy(window)+getmaxy(window)) &&
            event.x >= getbegx(window) && event.x < (getbegx(window)+getmaxx(window)));
}
enum class WindowLoopControl{CONTINUE, DONE};
/// @param handleAndContinue (int ch)->WindowLoopControl, return false to stop listening input
template<typename OP>
inline void loopWindowInput(WINDOW *window, OP const& handleAndContinue)
{
    keypad(window, true);
    while(WindowLoopControl::CONTINUE == handleAndContinue(wgetch(window)));
}