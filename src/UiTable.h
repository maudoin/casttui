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
  static constexpr int FILL=-1;
  static constexpr int FIT_LABEL=0;
  int width;
  std::optional<std::wstring> name = std::nullopt;
  SortDir sort = SortDir::NONE;
  std::optional<std::function<void()>> callback;
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
class UiTable : public UiHotspotGroup
{
  struct TableRender
  {
    UiTable& table;
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

  template<typename GetCell>
  void render(
    int k,
    int dataRowCount,
    const std::vector<HeaderColumn> &header_cols,
    const GetCell &cell_cb,
    bool focused = false)
  {
    auto tableRender = renderStart(k, dataRowCount, header_cols, focused);
    for (int i = 0; i < tableRender.rowCount(); ++i)
    {
      TableRender::Row r = tableRender.startRow(i);
      for (int c = 0 ; c < tableRender.cols_def.size();++c)
      {
        tableRender.draw(r, c, cell_cb(_firstVisibleDataRow+i, c, tableRender.getEvent(r, c)));
      }
    }
  }
  TableRender renderStart(
    int k,
    int dataRowCount,
    const std::vector<HeaderColumn> &headerCols,
    bool focused);

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

  friend class TableRender;
};
