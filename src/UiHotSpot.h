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
  bool handleMouseEvent(MouseEvent const& ev)const;
  void setWin(WINDOW* win);

};
class UiHotspotGroup : public UiHotspot
{
public:
  UiHotspotGroup(std::function<void()> const& callback);
  bool handleMouseEvent(MouseEvent const& ev)const;
  void setWin(WINDOW* win);

  void clear();
  void addLocalSpot(int beginRow, int beginCol, int endRow, int endCol, std::function<void()> const& callback);
private:
  std::vector<UiHotspot> _items;
};
