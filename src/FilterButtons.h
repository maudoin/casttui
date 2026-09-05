#pragma once


#include "TableWindow.h"
#include "DialogNew.h"
#include "Window.h"
#include "WindowLabel.h"
#include "TUIApp.h"

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

class FilterButtons
{
public:
    enum class Filter{New, Queue, Skip, Done, All};
    using Callback = std::function<void(Filter)>;
    FilterButtons(int x, int y, int space, Callback const& callback);

    void redrawFilters() const;

    TUIApp::NextOp mayHandleMouseEvent(MouseEvent const& event);
private:

    void drawFilter(WindowLabel const& win, Filter f) const;

    bool handleFilter(MouseEvent const& event, WindowLabel& win, Filter f);

    Filter _activeFilter = Filter::All;
    Callback _callback;
    WindowLabel _filterNewWindow;
    WindowLabel _filterQueueWindow;
    WindowLabel _filterSkippedWindow;
    WindowLabel _filterDoneWindow;
    WindowLabel _filterAllWindow;
};
