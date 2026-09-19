#pragma once

#include "MouseEvent.h"
#include "UiApp.h"

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
  static constexpr int FILL=-1;
  static constexpr int FIT_LABEL=0;
  int width;
  std::optional<std::wstring> name = std::nullopt;
  SortDir sort = SortDir::NONE;
  std::optional<std::function<void()>> callback;
};
struct ColumnRange
{
  int start,width;
  std::optional<MouseEvent> getEvent(int k, WINDOW* win, int row, int col)const;
};
struct Columns
{
  template <typename C>
  Columns(int innerWidth, std::vector<C> const&headerCols);
  std::vector<ColumnRange> vec;
  bool drawHeader = false;
};

// --------------------------------------------------------------------
// Cell
// --------------------------------------------------------------------
struct Cell
{
  std::wstring text = L"";
  int style = A_NORMAL;
};


// --------------------------------------------------------------------
// UiTable
// --------------------------------------------------------------------
class UiTable
{
  struct TableRender
  {
    UiTable& table;
    int k;
    int col;
    int totalInnerW;
    int dataRowCount;
    const std::vector<ColumnRange> cols_def;
    int borderStyle;
    std::pair<std::optional<int>, std::optional<int>> vparams;
    int const dynamicIndex;
    int viewContentFirstRow;
    int viewContentDataSize;

    struct Row
    {
      TableRender &tableRender;
      int i;
      int x = 0;
      ~Row(){tableRender.endRow(*this);}
    };
    ~TableRender();
    int rowCount() const {return viewContentDataSize;}
    Row startRow(int i){return Row{*this, i};};
    void endRow(Row const& row);
    std::optional<MouseEvent> getEvent(Row& row, int col);
    void draw(Row& row, int col, Cell const& cell);
  };
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

  Columns renderHeader(int k, std::vector<int> const&headerCols,int borderStyle);
  Columns renderHeader(int k, std::vector<HeaderColumn> const&headerCols,int borderStyle);

  template<typename GetCell, typename WinSelOp=decltype([]{})>
  void render(
    int k,
    int dataRowCount,
    Columns const& header_cols,
    const GetCell &cellCallback,
    int borderStyle,
    WinSelOp const& winSelOp = {})
  {
    if (UiApp::mouseHit(k, _win))
    {
      winSelOp();
    }
    auto tableRender = renderStart(k, dataRowCount, header_cols, borderStyle);
    for (int i = 0; i < tableRender.rowCount(); ++i)
    {
      TableRender::Row r = tableRender.startRow(i);
      for (int c = 0 ; c < tableRender.cols_def.size();++c)
      {
        tableRender.draw(r, c, cellCallback(_firstVisibleDataRow+i, c, tableRender.getEvent(r, c)));
      }
    }
  }
  TableRender renderStart(
    int k,
    int dataRowCount,
    const Columns &headerCols,
    int borderStyle);

  void renderArray(
    int k,
    const std::vector<Cell> &array,
    int borderStyle,
    const std::optional<std::wstring> &title = std::nullopt);

  void renderHeaderOnly(
    int k,
    std::vector<HeaderColumn> const&headerCols,
    int borderStyle)
  {
    auto h = renderHeader(k, headerCols, borderStyle);
    render(k, 0, h, [](int, int, std::optional<MouseEvent> const&){return Cell{};}, borderStyle);
  }

  int cursor() const { return _cursor; }
  int firstVisibleDataRow() const { return _firstVisibleDataRow; }
  int dynamicColCurrentOffsetX() const { return _dynamicColCurrentOffsetX; }

  int getHeight() const
  {
    return getmaxy(_win);
  }

protected:
  template <typename C>
  Columns renderHeaderImpl(int k, std::vector<C> const&headerCols, int borderStyle);

  WINDOW *_win = nullptr;

private:
  std::function<void()> _callback;
  int _cursor;
  Mode _mode;
  int _firstVisibleDataRow;
  int _dynamicColCurrentOffsetX;

  int _dynamicColViewWidth;
  int _dynamicColMaxDataWidth;

  int _dataRowCount;
  int _lastKnownViewHeight;

  friend class TableRender;
};
