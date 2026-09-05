#include "DropDown.h"

#include "MouseEvent.h"

DropDown::DropDown(int x, int y,
    Callbacks const& callbacks)
: WindowLabel(x, y, callbacks.stringAt(0))
, _xMax(getmaxx(stdscr))
, _yMax(getmaxy(stdscr))
, _callbacks(callbacks)
{}

TUIApp::NextOp DropDown::mayHandleMouseEvent(MouseEvent const& event)
{
    if(!this->handleMouseEvent(event))
    {
        return TUIApp::NextOp::NONE;
    }
    int const w = _xMax - 20;
    int const h = std::min(_yMax - 2, _callbacks.itemCount()+3);
    TableWindow podcastListWindow(x(), y()+1, w, h,
        {_callbacks.itemCount, _callbacks.colSizesReq,
        [&](int const col, int const line){return _callbacks.stringAt(line);}});
    loopWindowInput(podcastListWindow.handle(), [&](int ch)
    {
        constexpr int KEY_ESC = 0x1b;
        switch (ch) {
            case KEY_ESC:
                return WindowLoopControl::DONE;
            case KEY_MOUSE:
                if (auto event = getMouseEvent())
                {
                    if(this->handleMouseEvent(*event))
                    {
                        return WindowLoopControl::DONE;
                    }
                    if(podcastListWindow.handleMouseEvent(*event))
                    {
                        if(std::optional<int> newPod = podcastListWindow.currentSelectedItemIndex())
                        {
                            this->set(_callbacks.stringAt(*newPod));
                            _callbacks.itemChanged(*newPod);
                        }
                        return WindowLoopControl::DONE;
                    }
                }
                break;
        }
        return WindowLoopControl::CONTINUE;
    });
    return TUIApp::NextOp::REDRAW_ALL;
}
