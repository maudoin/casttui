#include "UiApp.h"

#include "UiColors.h"
#include "UiInput.h"

#if defined(_WIN32)
#include <curses.h>
#else
#include <ncursesw/curses.h>
#endif

#include <array>
#include <chrono>
#include <ctime>
#include <iostream>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

#ifndef _WIN32
#include <locale.h>
#endif


struct OfflineBuffers {
  int argc;
  char** argv;
  WINDOW* _virtual_buffer = nullptr;
  SCREEN* _virtual_scr = nullptr;
  FILE* _dummy_fp = nullptr;
  OfflineBuffers(int argc, char* argv[])
  : argc(argc)
  , argv(argv)
  {
    // Setup a detached mini dummy terminal stream to satisfy Curses init
    _dummy_fp = fopen(
  #ifdef _WIN32
      "NUL",
  #else
      "/dev/null",
  #endif
      "w"
    );

    _virtual_scr = newterm(nullptr, _dummy_fp, stdin);
    set_term(_virtual_scr);

    // Create a floating virtual layout buffer (Rows, Columns)
    _virtual_buffer = newwin(24, 80, 0, 0);
  }
  ~OfflineBuffers()
  {
    delwin(_virtual_buffer);
    delscreen(_virtual_scr);
    fclose(_dummy_fp);
  }
  void output()
  {
    int max_y{}, max_x{};
    getmaxyx(_virtual_buffer, max_y, max_x);
    for (int y = 0; y < max_y; ++y) {
        for (int x = 0; x < max_x; ++x) {
            chtype ch = mvwinch(_virtual_buffer, y, x);
            char c = static_cast<char>(ch & A_CHARTEXT);
            std::cout << (c ? c : ' ');
        }
        std::cout << '\n';
    }
  }
};

UiApp::UiApp(int argc, char* argv[])
{
  if (argc>1)
  {
    _offline.reset(new OfflineBuffers(argc, argv));

    start_color();
    use_default_colors();
    UiColors::init();

    return;
  }
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

  UiColors::init();
}

UiApp::~UiApp()
{
  if (!_offline)
  {
    endwin();
  }
}

// Interactive mode - normal user input
void UiApp::run()
{
  if (_offline)
  {
    for (int i = 1; i < _offline->argc; ++i)
    {
      std::string key = _offline->argv[i];
      // Convert string key to character code
      unsigned char ch = static_cast<unsigned char>(key[0]);

      // Create a synthetic UiInput with the programmatic key
      int h, w;
      getmaxyx(_offline->_virtual_buffer, h, w);
      UiInput input;
      input.height = h;
      input.width = w;
      input.key = ch;
      input.mev.reset(); // No mouse event

      // Process the key - update virtual buffer
      handleKey(input);
      render(input);
    }
    return;
  }
  render({ERR});

  std::chrono::steady_clock::time_point lastUpdate;
  std::optional<UiInput::MouseEvent> lastMouseEvent;

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
      render({ERR});

      // Refresh all windows
      wnoutrefresh(stdscr);
      doupdate();
    }
    else
    {
      auto now = std::chrono::steady_clock::now();
      if (k != KEY_MOUSE || now - lastUpdate >= std::chrono::milliseconds(16))
      {
        lastUpdate = now;
        UiInput input = UiInput::init(k);
        if (!lastMouseEvent ||
            !input.mev ||
            lastMouseEvent->x!=input.mev->x ||
            lastMouseEvent->y!=input.mev->y)
        {
          getmaxyx(stdscr, input.height, input.width);
          handleKey(input);
          render(input);
        }
      }
    }
  }
}

bool UiApp::handleKey(UiInput const& input)
{
  return doHandleKey(input);
}

void UiApp::delWindows()
{
  doDelWindows();
}

void UiApp::buildWindows()
{
  int h, w;
  if (_offline && _offline->_virtual_buffer)
  {
    getmaxyx(_offline->_virtual_buffer, h, w);
  }
  else
  {
    getmaxyx(stdscr, h, w);
  }

  doBuildWindows(h, w);
}

void UiApp::render(UiInput const& input)
{
  if (_offline && _offline->_virtual_buffer)
  {
    // In CLI mode, refresh the virtual buffer
    wrefresh(_offline->_virtual_buffer);
    doRender(input);
    _offline->output();
  }
  else
  {
    // Interactive mode - use stdscr
    wnoutrefresh(stdscr);
    doRender(input);
    doupdate();
  }


}