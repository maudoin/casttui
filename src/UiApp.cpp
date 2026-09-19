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
}

std::optional<MouseEvent> UiApp::mouseHit(int k, WINDOW* win)
{
  if (k == KEY_MOUSE)
  {
    int beginRow, beginCol;
    int h, w;
    getbegyx(win, beginRow, beginCol);
    getmaxyx(win, h, w);
    int endRow = h + beginRow;
    int endCol = w + beginCol;
    auto ev = getMouseEvent();
    if (ev && (ev->x >= beginCol && ev->x < endCol) &&
        (ev->y >= beginRow && ev->y < endRow))
    {
      return ev;
    }
  }
  return std::nullopt;
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

  int h, w;
  getmaxyx(stdscr, h, w);
  doRender(k, h, w);

  doupdate();
}