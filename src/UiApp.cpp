#include "UiApp.h"
#include "MouseEvent.h"

#if defined(_WIN32)
#include <curses.h>
#else
#include <ncursesw/curses.h>
#endif

#include <array>
#include <array>
#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <ctime>
#include <ranges>

#ifndef _WIN32
#include <locale.h>
#endif

UiApp::UiApp()
{

  #ifndef _WIN32
  setlocale(LC_ALL, "");
  #endif
  initscr();

  cbreak();
  noecho();

  mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  mouseinterval(0);//CLICKED will not work but gives fast mouse event response
  // set_escdelay(0); // wgetch(win) -> wgetch_escdelay(win, delay)
  curs_set(0);
  nodelay(stdscr, FALSE);
  keypad(stdscr, TRUE);

  // Colors
  start_color();
  use_default_colors();

  init_pair(1, COLOR_BLACK, COLOR_WHITE); // cursor highlight
  init_pair(2, COLOR_BLUE, -1);           // active status
  init_pair(3, COLOR_BLUE, COLOR_WHITE);  // cursor + active
  init_pair(4, COLOR_BLUE, -1);           // active window
}

UiApp::~UiApp()
{
  endwin();
}

void UiApp::run()
{
  render(ERR);

  while (_isRunning)
  {

    int k = wgetch(stdscr);
    if (k == KEY_RESIZE)
    {
      // Update curses internal structures
      resize_term(0, 0);

      // Recreate your windows with new sizes
      delWindows();
      buildWindows();

      // Redraw everything
      render(ERR);

      // Refresh all windows
      wnoutrefresh(stdscr);
      doupdate();
    }
    else
    {
      handleKey(k);
      render(k);
    }
  }
}

bool UiApp::handleKey(int k)
{
  if (k == KEY_MOUSE)
  {
    if (auto event = getMouseEvent())
    {
      if (doHandleMouse(*event))
      {
        return true;
      }
    }
  }
  return doHandleKey(k);
}
void UiApp::delWindows()
{
  doDelWindows();
}
void UiApp::buildWindows()
{
  int h, w;
  getmaxyx(stdscr, h, w);

  doBuildWindows(h, w);
}
void UiApp::render(int k)
{
  wnoutrefresh(stdscr);

  doRender(k);

  doupdate();
}