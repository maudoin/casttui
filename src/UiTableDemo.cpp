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

    UiInput input;
    while (true)
    {
      Columns cols = [&]{
        if (header)
        {
          return table.renderHeader(input, {
            HeaderColumn{ 3, std::wstring(L"A"), SortDir::UP },
            HeaderColumn{ HeaderColumn::FILL, std::wstring(L"B (fill) "), SortDir::NONE },
            HeaderColumn{ 4, std::wstring(L"C"), SortDir::NONE }
          }, focus);
        }
        else
        {
          return table.renderHeader(input, {
            HeaderColumn{ 10, std::nullopt, SortDir::NONE },
            HeaderColumn{ HeaderColumn::FILL, std::nullopt, SortDir::NONE },
            HeaderColumn{ 20, std::nullopt, SortDir::NONE }
          }, focus);
        }
      }();

      int rowCount = 20;

      auto cellCallback = [&cols](int row, int col, std::optional<MouseEvent> const&) -> Cell {
        std::wstring base = col==0?L"a":col==1?L"b":L"c";
        std::wstring text = base + std::to_wstring(row);
        int style = (row % 2 == 0) ? A_BOLD : A_DIM;
        return Cell{ text, style };
      };

      table.render(input, rowCount, cols, cellCallback, focus);

      input.key = wgetch(stdscr);

      if (!table.handleKey(input))
      {
        if (input.key == 10 || input.key == 13 || input.key == 32 || input.key == 27)
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
