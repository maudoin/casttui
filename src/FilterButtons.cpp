#include "FilterButtons.h"

FilterButtons::FilterButtons(int x, int y, int space, Callback const& callback)
: _activeFilter( Filter::All )
, _callback( callback )
, _filterNewWindow( x, y, "New")
, _filterQueueWindow( space+_filterNewWindow.nextX(), y, "Download queue")
, _filterSkippedWindow( space+_filterQueueWindow.nextX(), y, "Skipped")
, _filterDoneWindow( space+_filterSkippedWindow.nextX(), y, "Done")
, _filterAllWindow( space+_filterDoneWindow.nextX(), y, "All")
{}

void FilterButtons::redrawFilters() const
{
    drawFilter(_filterNewWindow, Filter::New);
    drawFilter(_filterQueueWindow, Filter::Queue);
    drawFilter(_filterSkippedWindow, Filter::Skip);
    drawFilter(_filterDoneWindow, Filter::Done);
    drawFilter(_filterAllWindow, Filter::All);
}

TUIApp::NextOp FilterButtons::mayHandleMouseEvent(MEVENT event)
{
    return ( handleFilter(event, _filterNewWindow, Filter::New)
    || handleFilter(event, _filterQueueWindow, Filter::Queue)
    || handleFilter(event, _filterSkippedWindow, Filter::Skip)
    || handleFilter(event, _filterDoneWindow, Filter::Done)
    || handleFilter(event, _filterAllWindow, Filter::All) ) ?
        TUIApp::NextOp::REDRAW_ALL :
        TUIApp::NextOp::NONE;
}

void FilterButtons::drawFilter(WindowLabel const& win, Filter f) const
{
    win.draw(_activeFilter == f);
}

bool FilterButtons::handleFilter(MEVENT event, WindowLabel& win, Filter f)
{
    if(win.handleMouseEvent(event))
    {
        _activeFilter = f;
        redrawFilters();
        _callback(f);
        return true;
    }
    return false;
}
