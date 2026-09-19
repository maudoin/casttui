#pragma once

#include "MouseEvent.h"

#include <optional>

namespace UiColors
{
void init()
{
  init_pair(1, COLOR_BLACK, COLOR_WHITE); // cursor highlight
  init_pair(2, COLOR_BLUE, -1);           // active status
  init_pair(3, COLOR_BLUE, COLOR_WHITE);  // cursor + active
  init_pair(4, COLOR_BLUE, -1);           // active window
}
static inline int focusedStyle()
{
  return COLOR_PAIR(4);
}
static inline int normalStyle()
{
  return A_NORMAL;
}
static inline int focusStyle(bool focused)
{
  return focused ? focusedStyle() : normalStyle();
}
int getStyle(bool isCursor, bool isSelected)
{
  if (isCursor && isSelected)
    return COLOR_PAIR(3);
  else if (isSelected)
    return COLOR_PAIR(2);
  else if (isCursor)
    return COLOR_PAIR(1);
  else
    return A_NORMAL;
}
}
