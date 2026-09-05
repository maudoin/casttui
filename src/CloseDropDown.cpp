#include "CloseDropDown.h"

#include "MouseEvent.h"

CloseDropDown::CloseDropDown(int x, int y)
: WindowLabel(x, y, "Close")
{}

TUIApp::NextOp CloseDropDown::mayHandleMouseEvent(MouseEvent const& event)
{
    if(!this->handleMouseEvent(event))
    {
        return TUIApp::NextOp::NONE;
    }
    TUIApp::NextOp nextOp = TUIApp::NextOp::NONE;
    WINDOW* menu = newwin(4, 1+6+1, getbegy(this->win)+1, getbegx(this->win)-3);
    box(menu, 0, 0);
    WindowLabel cancelWindow( 1, 1, "Cancel", menu);
    WindowLabel confirmWindow( 1, 2, "Exit  ", menu);
    wrefresh(menu);
    loopWindowInput(menu, [&](int ch)
    {
        switch (ch) {
            case 0x1b : //KEY_ESC
                nextOp = TUIApp::NextOp::REDRAW_ALL;
                return WindowLoopControl::DONE;
            case KEY_MOUSE:
                MouseEvent event;
                if (getMouseEvent(event) == OK )
                {
                    if(confirmWindow.handleMouseEvent(event))
                    {
                        nextOp = TUIApp::NextOp::QUIT;
                        return WindowLoopControl::DONE;
                    }
                    if(cancelWindow.handleMouseEvent(event)
                        || this->handleMouseEvent(event))
                    {
                        nextOp = TUIApp::NextOp::REDRAW_ALL;
                        return WindowLoopControl::DONE;
                    }
                }
                break;
        }
        return WindowLoopControl::CONTINUE;
    });
    delwin(menu);
    return nextOp;
}
