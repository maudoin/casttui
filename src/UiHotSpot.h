#pragma once

#include "MouseEvent.h"

#if defined(_WIN32)
#include <curses.h>
#else
#include <ncursesw/curses.h>
#endif
#include <string>
#include <vector>
#include <functional>
#include <optional>
#include <iostream>
#include <ranges>
#include <algorithm>

struct UiHotspot
{
    int beginRow, beginCol, endRow, endCol;
    std::function<void()> callback;
    bool handleMouseEvent(MouseEvent const& ev)const
    {
        if ((ev.x >= beginCol && ev.x < endCol) &&
            (ev.y >= beginRow && ev.y < endRow))
        {
            callback();
            return true;
        }
        return false;
    }
    void setWin(WINDOW* win)
    {
        getbegyx(win, beginRow, beginCol);
        getmaxyx(win, endRow, endCol);
        endRow += beginRow;
        endCol += beginCol;
    }

};
class UiHotspotGroup : public UiHotspot
{
public:
    UiHotspotGroup(std::function<void()> const& callback)
    : UiHotspot{0, 0, 0, 0, callback}
    {}
    bool handleMouseEvent(MouseEvent const& ev)const
    {
        if (UiHotspot::handleMouseEvent(ev))
        {
            MouseEvent locaEv = ev;
            locaEv.x -= beginCol;
            locaEv.y -= beginRow;
            for (auto const i:_items)
            {
                if (i.handleMouseEvent(locaEv))
                {
                    break;
                }
            }
            return true;
        }
        return false;
    }
    void setWin(WINDOW* win)
    {
        UiHotspot::setWin(win);
        clear();
    }

    void clear()
    {
        _items.clear();
    }
    void addLocalSpot(int beginRow, int beginCol, int endRow, int endCol, std::function<void()> const& callback)
    {
        _items.emplace_back(beginRow, beginCol, endRow, endCol, callback);
    }
private:
    std::vector<UiHotspot> _items;
};
