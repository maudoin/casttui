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
  std::wstring text = L"";
  int style = A_NORMAL;
  std::optional<std::function<void()>> callback;
};

// --------------------------------------------------------------------
// TableRender
// --------------------------------------------------------------------
class UiTable;
struct TableRender
{
  UiTable& table;
  WINDOW *win;
  int k;
  int col;
  int totalInnerW;
  int dataRowCount;
  const std::vector<HeaderColumn> cols_def;
  bool focused;
  std::pair<std::optional<int>, std::optional<int>> vparams;
  int const dynamicIndex;
  int viewContentFirstRow;
  int viewContentDataSize;

  ~TableRender();
  friend struct Row;
  struct Row
  {
    TableRender &tableRender;
    int i;

    int row()const;
    int dataRow()const;

    int x;
    int col_text_offset;
    int width;

    Row& operator*(){return *this;}
    bool operator==(Row const& c){return c.i==i;}
    bool operator!=(Row const& c){return c.i!=i;}
    Row& operator++(){++i;return *this;}
    int index(){return i;}

    ~Row();
    friend struct Col;
    struct Col
    {
      Row& context;
      int i;

      Col& operator*(){return *this;}
      bool operator==(Col const& c){return c.i==i;}
      bool operator!=(Col const& c){return c.i!=i;}
      Col& operator++(){++i;return *this;}
      int index(){return i;}

      std::optional<MouseEvent> draw(Cell const& cell)
      {
        return context.draw(i, cell);
      }
    };

    Col begin();
    Col end(){return {*this, static_cast<int>(tableRender.cols_def.size())};}
  private:
    std::optional<MouseEvent> draw(int i, Cell const& cell);
  };
  Row begin(){return {*this, 0};}
  Row end(){return {*this, viewContentDataSize};}
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

  bool handleKey(int key);

  friend class TableRender;
  TableRender renderLoop(
    int k,
    int dataRowCount,
    const std::vector<HeaderColumn> &header_cols,
    bool focused = false);
  void renderEnd(TableRender& tableRender);

  void renderArray(
    int k,
    const std::vector<Cell> &array,
    const std::optional<std::wstring> &title = std::nullopt,
    bool focused = false);
  void renderSingleLine(
    int k,
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
  Mode _mode;
  int _firstVisibleDataRow;
  int _dynamicColCurrentOffsetX;

  int _dynamicColViewWidth;
  int _dynamicColMaxDataWidth;

  int _dataRowCount;
  int _lastKnownViewHeight;
};
