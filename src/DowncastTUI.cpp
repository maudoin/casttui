#include "DowncastTUI.h"

Downcast::Downcast(DropDown::Callbacks const& podcastCallbacks,
                    TableWindow::Callbacks const& showsCallbacks,
                    FilterButtons::Callback const& filterCallback)
: _xMax(getmaxx(stdscr))
, _yMax(getmaxy(stdscr))
, _addWindow( MENU_ITEM_SPACE, FIRST_LINE_Y, "Add...")
, _podcastMenuWindow(MENU_ITEM_SPACE+_addWindow.nextX()+1+strlen(viewingLabel.c_str()), FIRST_LINE_Y,
    podcastCallbacks)
, _closeWindow( _xMax-5, FIRST_LINE_Y)
, _filters(MENU_ITEM_SPACE+strlen(statusFilterLabel.c_str()), SECOND_LINE_Y, MENU_ITEM_SPACE, filterCallback)
, _tableWindow(1, 2, _xMax - 2,  _yMax - 3, showsCallbacks)
{}

WINDOW* Downcast::mainHandle()
{
    return _tableWindow.handle();
}

void Downcast::drawBackground() const
{
    mvprintw(FIRST_LINE_Y, 8, "%s", viewingLabel.c_str());
    mvprintw(SECOND_LINE_Y, 0, "%s",statusFilterLabel.c_str());
    refresh();
}

void Downcast::redrawAll() const
{
    erase();
    this->drawBackground();

    _addWindow.draw();
    _podcastMenuWindow.draw();

    _filters.redrawFilters();

    _closeWindow.draw();
    _tableWindow.draw(true);
}

void Downcast::resize()
{
    refresh();
    _addWindow.draw();
    _podcastMenuWindow.draw();
    _closeWindow.draw();
    _tableWindow.resize(getmaxx(stdscr), getmaxy(stdscr));
}

TUIApp::NextOp Downcast::handleMouseEvent(MEVENT const& event)
{
    if(_tableWindow.handleMouseEvent(event))
    {
        return TUIApp::NextOp::NONE;
    }
    if(_addWindow.handleMouseEvent(event))
    {
        popupFieldDialog(_tableWindow.handle());
        return TUIApp::NextOp::REDRAW_ALL;
    }
    if(TUIApp::NextOp nextOp = _podcastMenuWindow.mayHandleMouseEvent(event))
    {
        return nextOp;
    }
    if(TUIApp::NextOp nextOp = _closeWindow.mayHandleMouseEvent(event))
    {
        return nextOp;
    }
    return _filters.mayHandleMouseEvent(event);
}

const std::string Downcast::viewingLabel = "Viewing podcast : ";
const std::string Downcast::statusFilterLabel =  "Status filter : ";
