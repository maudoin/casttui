#include "UiHotspot.h"

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


bool UiHotspot::handleMouseEvent(MouseEvent const& ev)const
{
  if ((ev.x >= beginCol && ev.x < endCol) &&
  (ev.y >= beginRow && ev.y < endRow))
  {
    callback();
    return true;
  }
  return false;
}
void UiHotspot::setWin(WINDOW* win)
{
  getbegyx(win, beginRow, beginCol);
  getmaxyx(win, endRow, endCol);
  endRow += beginRow;
  endCol += beginCol;
}


UiHotspotGroup::UiHotspotGroup(std::function<void()> const& callback)
: UiHotspot{0, 0, 0, 0, callback}
{}
bool UiHotspotGroup::handleMouseEvent(MouseEvent const& ev)const
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
void UiHotspotGroup::setWin(WINDOW* win)
{
  UiHotspot::setWin(win);
  clear();
}

void UiHotspotGroup::clear()
{
  _items.clear();
}
void UiHotspotGroup::addLocalSpot(int beginRow, int beginCol, int endRow, int endCol, std::function<void()> const& callback)
{
  _items.emplace_back(beginRow, beginCol, endRow, endCol, callback);
}