#pragma once

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

inline bool DEBUG_UI = true;

// --------------------------------------------------------------------
// SortDir / HeaderColumn
// --------------------------------------------------------------------
enum class SortDir
{
  NONE,
  UP,
  DOWN
};

struct HeaderColumn
{
  int width;
  std::optional<std::wstring> name = std::nullopt;
  SortDir sort = SortDir::NONE;
  bool dynamic = false;
  std::optional<std::function<void()>> callback;
};

// --------------------------------------------------------------------
// Cell
// --------------------------------------------------------------------
struct Cell
{
  std::wstring text;
  int style = A_NORMAL;
  std::optional<std::function<void()>> callback;
};

// --------------------------------------------------------------------
// UiTable
// --------------------------------------------------------------------
class UiTable : public UiHotspotGroup
{
public:
  enum class Mode
  {
    SCROLL,
    CURSOR
  };

  UiTable(Mode mode = Mode::SCROLL, std::function<void()> const &callback = [] {}, int firstVisibleDataRow = 0, int dynamicColCurrentOffsetX = 0);

  void delWindow();
  void buildWindow(int nlines, int ncols, int begy, int begx);
  void scrollTo(int newCursor);
  void scrollVertical(int amount);
  void scrollHorizontal(int amount);

  bool handleKeyCh(int key);

  void render(
      int dataRowCount,
      const std::vector<HeaderColumn> &header_cols,
      const std::function<Cell(int, int)> &cell_cb,
      bool focused = false);

  void renderArray(
      const std::vector<Cell> &array,
      const std::optional<std::wstring> &title = std::nullopt,
      bool focused = false);
  void renderSingleLine(
      const std::vector<Cell> &array,
      bool focused = false);

  int cursor() const { return _cursor; }
  int firstVisibleDataRow() const { return _firstVisibleDataRow; }
  int dynamicColCurrentOffsetX() const { return _dynamicColCurrentOffsetX; }

  int getHeight() const
  {
    return getmaxy(_win);
  }

protected:
  WINDOW *_win = nullptr;

private:
  int _cursor;
  Mode mode;
  int _firstVisibleDataRow;
  int _dynamicColCurrentOffsetX;

  int dynamicColViewWidth;
  int dynamicColMaxDataWidth;

  int data_row_count;
  int lastKnownViewHeight;
};
