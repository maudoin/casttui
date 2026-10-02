#pragma once

#include "UiApp.h"
#include "UiColors.h"
#include "UiInput.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <vector>

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
  Columns(int innerWidth, std::span<C const> headerCols, std::optional<Columns> const& prev = std::nullopt);
  Columns(std::vector<ColumnRange> const& vec, int rowOffset)
  : vec(vec)
  , rowOffset(rowOffset)
  {}
  std::vector<ColumnRange> vec;
  bool drawHeader = false;
  int rowOffset = 0;
  int dynamicIndex = -1;
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
    bool limitedHeight;

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
  };
public:
  struct CellRenderStr
  {
    int row, col;
    std::optional<UiInput::MouseEvent> ev;
    CellRenderStr(TableRender::Row& r, int c);
    friend CellRenderStr& operator<<(CellRenderStr& s, int attr);
    friend CellRenderStr& operator<<(CellRenderStr& s, std::wstring_view const& str);
    ~CellRenderStr();
  private:
    TableRender::Row& _row;
    int _attr;
    int _logicalSize = 0;
  };
  friend CellRenderStr& operator<<(CellRenderStr& s, std::wstring_view const& str);
  enum class Mode
  {
    SCROLL,
    CURSOR
  };

  UiTable(Mode mode = Mode::SCROLL, std::function<void()> const &callback = [] {}, int firstVisibleDataRow = 0, int dynamicColCurrentOffsetX = 0);
  ~UiTable();

  virtual void delWindow();
  virtual void buildWindow(int nlines, int ncols, int begy, int begx);
  void buildWindowCentered(int nlines, int ncols, int parentHeight, int parentWidth)
  {
      int y  = (parentHeight - nlines) / 2;
      int x  = (parentWidth - ncols) / 2;
      this->buildWindow(nlines, ncols, y, x);
  }

  void scrollTo(int newCursor);
  void scrollVertical(int amount);
  void scrollHorizontal(int amount);

  bool handleKey(UiInput const& input);

  class RowReserve{ std::optional<int> index;friend class UiTable;public: RowReserve(std::optional<int> index=std::nullopt):index(index){}};

  template <std::ranges::contiguous_range R>
  requires std::same_as<std::ranges::range_value_t<R>, int> || std::same_as<std::ranges::range_value_t<R>, HeaderColumn>
  Columns renderHeader(UiInput const& input, R&& headerCols,int borderStyle,
    std::optional<Columns> const& previousColumns = std::nullopt)
  {
    std::span<const std::ranges::range_value_t<R>> span{std::ranges::data(headerCols),std::ranges::size(headerCols)};
    return renderHeaderImpl(input, span, borderStyle, previousColumns);
  }

  template<typename GetCell, typename WinSelOp=decltype([]{})>
  Columns render(
    UiInput const& input,
    int dataRowCount,
    Columns const& header_cols,
    const GetCell &cellCallback,
    int borderStyle,
    WinSelOp const& winSelOp = {},
    RowReserve const& rowCount={})
  {
    if (mouseHit(input))
    {
      winSelOp();
    }
    auto tableRender = renderStart(input, dataRowCount, header_cols, borderStyle, rowCount);
    for (int i = 0; i < tableRender.rowCount(); ++i)
    {
      TableRender::Row r = tableRender.startRow(i);
      for (int c = 0 ; c < tableRender.cols_def.size();++c)
      {
        CellRenderStr cellRender(r, c);
        cellCallback(cellRender);
      }
    }
    return Columns(tableRender.cols_def,  tableRender.viewContentFirstRow+ _lastKnownViewHeight);
  }
  TableRender renderStart(
    UiInput const& input,
    int dataRowCount,
    const Columns &headerCols,
    int borderStyle,
    RowReserve const& rowCount={});


  template <std::ranges::contiguous_range R>
  requires std::same_as<std::ranges::range_value_t<R>, std::wstring>
  void renderSingleLineRange(
    UiInput const& input,
    R&& r,
    int borderStyle,
    RowReserve const& rowCount={},
    const std::optional<std::wstring>& title = std::nullopt,
    std::optional<Columns> const& previousColumns = std::nullopt)
  {
    std::span<std::wstring> span{std::ranges::data(r),std::ranges::size(r)};

    auto cellCallback = [&span](UiTable::CellRenderStr& str)
    {
      str << span[str.row];
    };

    Columns cols = renderHeader(input, std::array{
      HeaderColumn{.width=HeaderColumn::FILL, .name=title}}, borderStyle, previousColumns);

    render(input, static_cast<int>(span.size()), cols, cellCallback, borderStyle, []{}, rowCount);
  }

  template <std::ranges::contiguous_range R>
  requires std::same_as<std::ranges::range_value_t<R>, int> || std::same_as<std::ranges::range_value_t<R>, HeaderColumn>
  void renderHeaderOnly(
    UiInput const& input,
    R&& headerCols,
    int borderStyle,
    std::optional<Columns> const& previousColumns = std::nullopt)
  {
    auto h = renderHeader(input, headerCols, borderStyle, previousColumns);
    renderEmpty(input, h, borderStyle);
  }

  int cursor() const { return _cursor; }
  int firstVisibleDataRow() const { return _firstVisibleDataRow; }
  int dynamicColCurrentOffsetX() const { return _dynamicColCurrentOffsetX; }

  int getHeight() const;
  int getWidth() const;

  void forceRefresh();

protected:

  UiTableData *_pimpl;

private:
  template  <typename C>
  Columns renderHeaderImpl(UiInput const& input, std::span<C const> headerCols,int borderStyle,
    std::optional<Columns> const& previousColumns = std::nullopt);
  /// To finish after header(s)
  void renderEmpty(
    UiInput const& input,
    Columns const& header_cols,
    int borderStyle);

  bool mouseHit(UiInput const& input);
  std::pair<std::optional<int>, std::optional<int>> vScroll(UiInput const& input, int viewContentFirstRow, int scrollX);
  std::pair<std::optional<int>, std::optional<int>> hScroll(UiInput const& input, int scrollY, int totalInnerW);

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
