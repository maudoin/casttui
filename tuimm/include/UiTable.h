#pragma once

#include "UiApp.h"
#include "UiColors.h"
#include "UiInput.h"

#include <string>
#include <vector>
#include <functional>
#include <optional>
#include <iostream>
#include <ranges>
#include <algorithm>

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
  int style = UiColors::normalStyle();
};


// PIMPL
struct UiTableData;
// --------------------------------------------------------------------
// UiTable
// --------------------------------------------------------------------
class UiTable
{
  struct TableRender
  {
    UiTable& table;
    UiInput input;
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
    std::optional<UiInput::MouseEvent> getEvent(Row& row, int col);
    void draw(Row& row, int col, Cell const& cell);
  };
public:
  enum class Mode
  {
    SCROLL,
    CURSOR
  };

  UiTable(Mode mode = Mode::SCROLL, std::function<void()> const &callback = [] {}, int firstVisibleDataRow = 0, int dynamicColCurrentOffsetX = 0);
  ~UiTable();

  void delWindow();
  void buildWindow(int nlines, int ncols, int begy, int begx);
  void scrollTo(int newCursor);
  void scrollVertical(int amount);
  void scrollHorizontal(int amount);

  bool handleKey(UiInput const& input);

  Columns renderHeader(UiInput const& input, std::vector<int> const&headerCols,int borderStyle);
  Columns renderHeader(UiInput const& input, std::vector<HeaderColumn> const&headerCols,int borderStyle);

  template<typename GetCell, typename WinSelOp=decltype([]{})>
  void render(
    UiInput const& input,
    int dataRowCount,
    Columns const& header_cols,
    const GetCell &cellCallback,
    int borderStyle,
    WinSelOp const& winSelOp = {})
  {
    if (mouseHit(input))
    {
      winSelOp();
    }
    auto tableRender = renderStart(input, dataRowCount, header_cols, borderStyle);
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
    UiInput const& input,
    int dataRowCount,
    const Columns &headerCols,
    int borderStyle);

  void renderArray(
    UiInput const& input,
    const std::vector<Cell> &array,
    int borderStyle,
    const std::optional<std::wstring> &title = std::nullopt);

  void renderHeaderOnly(
    UiInput const& input,
    std::vector<HeaderColumn> const&headerCols,
    int borderStyle)
  {
    auto h = renderHeader(input, headerCols, borderStyle);
    render(input, 0, h, [](int, int, std::optional<UiInput::MouseEvent> const&){return Cell{};}, borderStyle);
  }

  int cursor() const { return _cursor; }
  int firstVisibleDataRow() const { return _firstVisibleDataRow; }
  int dynamicColCurrentOffsetX() const { return _dynamicColCurrentOffsetX; }

  int getHeight() const;

protected:
  template <typename C>
  Columns renderHeaderImpl(UiInput const& input, std::vector<C> const&headerCols, int borderStyle);

  UiTableData *_pimpl;

private:
  bool mouseHit(UiInput const& input);

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
