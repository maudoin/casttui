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
  int beginRow=0, beginCol=0, endRow=0, endCol=0;
  std::function<void()> callback=[]{};
  bool handleMouseEvent(MouseEvent const& ev)const;
  void setWin(WINDOW* win);
  operator bool()const{return beginRow&&beginCol&&endRow&&endCol;}
};
class UiHotspotGroup : public UiHotspot
{
public:
  UiHotspotGroup(std::function<void()> const& callback);
  bool handleMouseEvent(MouseEvent const& ev)const;
  void setWin(WINDOW* win);

  void clear();
  void addLocalSpot(int beginRow, int beginCol, int endRow, int endCol, std::function<void()> const& callback);
  void add(UiHotspot const& h){_items.push_back(h);}
private:
  std::vector<UiHotspot> _items;
};
