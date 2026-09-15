#include "UiTable.h"
#include <vector>
#include <string>

void run_table(int width, bool header, bool focus = false)
{
  WINDOW* stdscr = initscr();
  clear();

  int height = 14;
  int starty = 1;
  int startx = 3;
  {
    UiTable table(UiTable::Mode::SCROLL, []{}, 5, 0);
    table.buildWindow(height, width, starty, startx);

    int k = ERR;
    while (true)
    {
      std::vector<HeaderColumn> cols;
      if (header)
      {
        cols = {
          HeaderColumn{ 3, std::wstring(L"A"), SortDir::UP, false },
          HeaderColumn{ 0, std::wstring(L"B (fill) "), SortDir::NONE, true },
          HeaderColumn{ 4, std::wstring(L"C"), SortDir::NONE, false }
        };
      }
      else
      {
        cols = {
          HeaderColumn{ 10, std::nullopt, SortDir::NONE, false },
          HeaderColumn{ 0, std::nullopt, SortDir::NONE, true },
          HeaderColumn{ 20, std::nullopt, SortDir::NONE, false }
        };
      }

      int rowCount = 20;

      for (TableRender::Row r : table.renderLoop(k, rowCount, cols, focus))
      {
        for (TableRender::Row::Col c : r)
        {
          std::wstring base = cols[c.index()].name.value_or(L"");
          std::wstring text = base + std::to_wstring(r.index());
          int style = (r.index() % 2 == 0) ? A_BOLD : A_DIM;
          c.draw({ text, style });
        }
      }

      k = wgetch(stdscr);

      if (!table.handleKey(k))
      {
        if (k == 10 || k == 13 || k == 32 || k == 27)
        break;
      }
    }
  }
  endwin();
}
#ifndef _WIN32
#include <locale.h>
#endif

int main3()
{
  #ifndef _WIN32
  setlocale(LC_ALL, "");
  #endif
  initscr();
  start_color();
  use_default_colors();
  init_pair(1, COLOR_BLACK, COLOR_WHITE);
  init_pair(2, COLOR_BLUE, -1);
  init_pair(3, COLOR_WHITE, -1);

  run_table(25, true);
  run_table(25, true, true);
  run_table(50, false);
  run_table(50, false, true);

  return 0;
}
