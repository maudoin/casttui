#include "UiColors.h"

#if defined(_WIN32)
#include <curses.h>
#else
#include <ncursesw/curses.h>
#endif

namespace UiColors
{
void init()
{
  init_pair(1, COLOR_BLACK, COLOR_WHITE); // cursor highlight
  init_pair(2, COLOR_BLUE, -1);           // active status
  init_pair(3, COLOR_BLUE, COLOR_WHITE);  // cursor + active
  init_pair(4, COLOR_BLUE, -1);           // active window
}

int highlightedStyle()
{
  return COLOR_PAIR(1);
}

int focusedStyle()
{
  return COLOR_PAIR(4);
}

int normalStyle()
{
  return A_NORMAL;
}

int focusStyle(bool focused)
{
  return focused ? focusedStyle() : normalStyle();
}

int highlightStyle(bool highlighted)
{
  return highlighted ? highlightedStyle() : normalStyle();
}

int getStyle(bool isCursor, bool isSelected)
{
  if (isCursor && isSelected)
    return COLOR_PAIR(3);
  else if (isSelected)
    return COLOR_PAIR(2);
  else
    return highlightStyle(isCursor);
}
}
