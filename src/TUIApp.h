#pragma once


#include "TableWindow.h"
#include "DialogNew.h"
#include "Window.h"
#include "WindowLabel.h"

#if defined(_WIN32)
#include <curses.h>
#else
#include <ncurses.h>
#endif
#include <functional>
#include <string>
#include <limits>
#include <string.h>

namespace TUIApp
{

enum NextOp : int{NONE = 0, REDRAW_ALL, QUIT};
/// @tparam T Exprected API :
/// default ctor
/// void redrawAll();
/// void resize();
/// WINDOWS* mainHandle();
/// bool handleMouseEvent(MEVENT const&);
template<typename T, typename... ARGS>
int main(int argc, char *argv[], ARGS&&... args)
{
    initscr();

    cbreak();
    noecho();

    T impl(std::forward<ARGS>(args)...);

    impl.drawBackground();

    start_color();

    impl.redrawAll();

    mousemask(ALL_MOUSE_EVENTS, NULL);
    mouseinterval(0);//CLICKED will not work but gives fast mouse event response
    set_escdelay(0);
    loopWindowInput(impl.mainHandle(), [&](int ch)
    {
        switch (ch) {
            case KEY_RESIZE :
                impl.resize();
                break;
            case KEY_MOUSE:
                MEVENT event;
                if (getmouse(&event) == OK )
                {
                    switch( impl.handleMouseEvent(event) )
                    {
                        case NextOp::QUIT:
                            return WindowLoopControl::DONE;
                        case NextOp::REDRAW_ALL:
                            impl.redrawAll();
                            //loop again
                            return WindowLoopControl::CONTINUE;
                    }
                }
                break;
            case 0x1b : //KEY_ESC
                return WindowLoopControl::DONE;
        }
        //loop again
        return WindowLoopControl::CONTINUE;
    });

    endwin();

    return 1;
}

}
