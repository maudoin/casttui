#include "UiTable.h"
#include <algorithm>
#include <ranges>

namespace{
// --------------------------------------------------------------------
// Safe write helpers
// --------------------------------------------------------------------
#ifdef DEBUG_UI
inline void safe_addstr(WINDOW *_win, int row, int col, const std::wstring &ch, int attrs)
{
  int h, w;
  getmaxyx(_win, h, w);

  if (row < 0 || row >= h || col < 0 || col >= w)
  {
    std::wcerr << "[OOB] row=" << row << " col=" << col
               << " char=" << ch << " win_w=" << w << " win_h=" << h << "\n";
    std::wcerr << "=== CURSES ERROR TRACE ===\n";
    // StackTrace::print();
  }
  wattron(_win, attrs);
  mvwaddwstr(_win, row, col, ch.c_str());
  wattroff(_win, attrs);
}

inline void addstr_run(WINDOW *_win, int row, int col, const std::wstring &chars, int attrs)
{
  for (std::size_t i = 0; i < chars.size(); ++i)
  {
    safe_addstr(_win, row, col + static_cast<int>(i),
                std::wstring(1, chars[i]), attrs);
  }
}
#else
inline void addstr_run(WINDOW *_win, int row, int col, const std::wstring &chars, int attrs)
{
  wattron(_win, attrs);
  mvwaddwstr(_win, row, col, chars.c_str());
  wattroff(_win, attrs);
}
#endif

// --------------------------------------------------------------------
// Border builders
// --------------------------------------------------------------------
inline void draw_bottom_scroll_border(
    WINDOW *_win,
    int row,
    int col,
    int inner_width,
    std::pair<std::optional<int>, std::optional<int>> hparams,
    int borderStyle)
{
  auto [startOpt, endOpt] = hparams;
  std::wstring chars;
  chars.reserve(inner_width);

  for (int c = 0; c < inner_width; ++c)
  {
    if (startOpt && endOpt && *startOpt <= c && c < *endOpt)
      chars.append(L"▄"); // FIXED
    else
      chars.append(L"─"); // FIXED
  }

  addstr_run(_win, row, col, std::wstring(L"╰") + chars + std::wstring(L"╯"), borderStyle);
}

inline void draw_border_columns(
    WINDOW *_win,
    int row,
    int col,
    const std::vector<ColumnRange> &cols,
    const std::wstring &start,
    const std::wstring &mid,
    const std::wstring &end,
    int borderStyle)
{
  addstr_run(_win, row, col, start, borderStyle);
  for (std::size_t i = 0; i < cols.size(); ++i)
  {
    auto const& range = cols[i];
    addstr_run(_win, row, col + range.start, std::wstring(range.width, L'─'), borderStyle);
    addstr_run(_win, row, col + range.start + range.width, (i < cols.size() - 1)?mid:end, borderStyle);
  }
}

void draw_top_border_header(
    WINDOW *_win,
    int row,
    int col,
    const std::vector<ColumnRange> &cols,
    int borderStyle)
{
  // box(_win, 0, 0);
  draw_border_columns(_win, row, col, cols, L"╭", L"┬", L"╮", borderStyle);
}

void draw_mid_border_header(
    WINDOW *_win,
    int row,
    int col,
    const std::vector<ColumnRange> &cols,
    int borderStyle)
{
  draw_border_columns(_win, row, col, cols, L"├", L"┼", L"┤", borderStyle);
}

void draw_bottom_border_header(
    WINDOW *_win,
    int row,
    int col,
    const std::vector<ColumnRange> &cols,
    int borderStyle)
{
  draw_border_columns(_win, row, col, cols, L"╰", L"┴", L"╯", borderStyle);
}

void draw_top_border(
    WINDOW *_win,
    int row,
    int col,
    int inner_width,
    int borderStyle)
{
  draw_top_border_header(_win, row, col, {ColumnRange{0,inner_width}}, borderStyle);
}

void draw_bottom_border(
    WINDOW *_win,
    int row,
    int col,
    int inner_width,
    int borderStyle)
{
  draw_bottom_border_header(_win, row, col, {ColumnRange{0,inner_width}}, borderStyle);
}

inline std::wstring build_left_border()
{
  return L"│";
}

inline std::wstring build_separator()
{
  return L"│";
}

inline std::wstring build_right_border(
    std::pair<std::optional<int>, std::optional<int>> vparams,
    int row)
{
  auto [startOpt, endOpt] = vparams;
  if (startOpt && endOpt && *startOpt <= row && row < *endOpt)
    return L"█";
  return L"│";
}

namespace{
inline int width(int width){return std::max(0,width);}
inline int width(HeaderColumn const& c){return (c.width == HeaderColumn::FIT_LABEL && c.name)?c.name->size():width(c.width);}
inline bool fill(int width){return width == HeaderColumn::FILL;}
inline bool fill(HeaderColumn const& c){return fill(c.width);}
}
template<typename C>
inline void draw_header_row(
  int k,
  WINDOW *_win,
  int row,
  int col,
  const std::vector<C> &cols,
  const std::vector<ColumnRange> &colRanges,
  int borderStyle)
{
  addstr_run(_win, row, col, build_left_border(), borderStyle);

  for (std::size_t i = 0; i < cols.size(); ++i)
  {
    const auto &header = cols[i];
    const auto &headerRange = colRanges[i];
    std::wstring cell;

    if constexpr(requires (C c){c.name;})
    if (header.name)
    {
      cell = *header.name;
      if (header.sort == SortDir::UP)
        cell += L" ↑";
      else if (header.sort == SortDir::DOWN)
        cell += L" ↓";
    }

    if (static_cast<int>(cell.size()) < headerRange.width)
      cell.append(headerRange.width - cell.size(), ' ');
    else if (static_cast<int>(cell.size()) > headerRange.width)
      cell = cell.substr(0, headerRange.width);

    addstr_run(_win, row, col + headerRange.start, cell, borderStyle);
    addstr_run(_win, row, col + headerRange.start + headerRange.width,
      (i == cols.size()-1)?build_right_border({std::nullopt, std::nullopt}, 0):L"│", borderStyle);
    if constexpr(requires (C c){c.name;})
    if (header.callback)
    {
      if (headerRange.getEvent(k, _win, row, col))
      {
        (*header.callback)();
      }
    }
  }
}

// ------------------------------------------------------------
void draw_empty_border(
    WINDOW *_win,
    int row,
    int col,
    int width,
    int borderStyle,
    std::pair<std::optional<int>, std::optional<int>> vparams)
{
  addstr_run(_win, row, col, build_left_border(), borderStyle);
  addstr_run(_win, row, col + width, build_right_border(vparams, row), borderStyle);
}

// --------------------------------------------------------------------
inline std::pair<std::optional<int>, std::optional<int>> scrollbar_thumb(
    int dataOffset,
    int dataSize,
    int dataViewSize,
    int uiOffset,
    int uiSize)
{
  if (uiSize <= 0 || dataSize <= dataViewSize)
    return {std::nullopt, std::nullopt};

  double ratio = static_cast<double>(dataViewSize) / static_cast<double>(dataSize);
  int thumbSize = std::max(1, static_cast<int>(uiSize * ratio));
  int maxOffset = dataSize - dataViewSize;

  int thumbStart = uiOffset +
                   static_cast<int>((static_cast<double>(dataOffset) / static_cast<double>(maxOffset)) *
                                    static_cast<double>(uiSize - thumbSize));

  return {thumbStart, thumbStart + thumbSize};
}
}//namespace
// ------------------------------------------------------------
std::optional<MouseEvent> UiTable::TableRender::getEvent(Row& r, int c)
{
  return this->cols_def[c].getEvent(k, table._win, viewContentFirstRow+r.i, col);
}
void UiTable::TableRender::draw(Row& r, int i, Cell const& cell)
{
  int row = viewContentFirstRow+r.i;
  addstr_run(table._win, row, col + r.x, i==0?build_left_border():build_separator(), borderStyle);

  int col_text_offset = (dynamicIndex == i) ? table._dynamicColCurrentOffsetX : 0;
  int width = cols_def[i].width;

  std::wstring text = cell.text;
  if (dynamicIndex == i)
    table._dynamicColMaxDataWidth = std::max(table._dynamicColMaxDataWidth, static_cast<int>(cell.text.size()));
  // horizontal scroll
  if (col_text_offset > 0)
    text = text.substr(std::min((int)text.size(), col_text_offset));
  // fill/fit column
  text.resize(std::max(0, cols_def[i].width), ' ');

  if ((int)text.size() < width)
    text.append(width - text.size(), ' ');
  else if ((int)text.size() > width)
    text = text.substr(0, width);

  addstr_run(table._win, row, col + r.x + 1, text, cell.style);
  r.x += width + 1;
}
// ------------------------------------------------------------
void UiTable::TableRender::endRow(Row const& r)
{
  int row = viewContentFirstRow+r.i;
  addstr_run(table._win, row, col + r.x, build_right_border(vparams, row), borderStyle);
}
// ------------------------------------------------------------
std::optional<MouseEvent> ColumnRange::getEvent(int k, WINDOW* win, int row, int col)const
{
  if (k==KEY_MOUSE)
  {
    if (auto ev = getMouseEvent())
    {
      MouseEvent mev = ev->toLocal(win);
      if (mev.hit({row, col + start, row + 1, col + start + width}))
      {
        return ev;
      }
    }
  }
  return std::nullopt;
}
// ------------------------------------------------------------
template <typename C>
Columns::Columns(int totalWidth, std::vector<C> const&headerCols)
{
  int const totalInnerW = totalWidth - 2;
  int fixed = 0;
  int num_fill = 0;
  vec.reserve(headerCols.size());
  int x=1;
  for (auto const &hc : headerCols)
  {
    int w;
    if (fill(hc))
    {
      num_fill++;
      w = 0;//width pending
    }
    else
    {
      w = ::width(hc);
    }
    fixed += w;
    vec.emplace_back(x,w);
    x+=w+1;
  }

  if (num_fill)
  {
    int sep_space = static_cast<int>(headerCols.size()) - 1;
    int remaining = totalInnerW - fixed - sep_space;
    int fill_width = num_fill > 0 && remaining > 0? remaining / num_fill : 0;
    int x=1;
    for (std::size_t i = 0; i < headerCols.size(); ++i)
    {
      if (fill(headerCols[i]))
      {
        vec[i].width = fill_width;
      }
      vec[i].start = x;
      x+=vec[i].width+1;
    }
  }
  if constexpr (requires(C c) { c.name; })
  {
    drawHeader = std::ranges::any_of(headerCols,
      [](C const &hc){ return hc.name.has_value(); });
  }
}
// ------------------------------------------------------------
// CLASS
// ------------------------------------------------------------
UiTable::UiTable(Mode mode,
                 std::function<void()> const &callback,
                 int firstVisibleDataRow,
                 int dynamicColCurrentOffsetX)
  : _callback(callback)
  , _cursor(firstVisibleDataRow)
  , _mode(mode)
  , _firstVisibleDataRow(firstVisibleDataRow)
  , _dynamicColCurrentOffsetX(dynamicColCurrentOffsetX)
  , _dynamicColViewWidth(0)
  , _dynamicColMaxDataWidth(0)
  , _dataRowCount(0)
  , _lastKnownViewHeight(0)
{
}

// ------------------------------------------------------------
void UiTable::scrollTo(int newCursor)
{
  if (_mode == Mode::CURSOR)
  {
    _cursor = std::clamp(newCursor, 0, _dataRowCount - 1);
    int visible = _lastKnownViewHeight;

    if (_cursor < _firstVisibleDataRow)
      _firstVisibleDataRow = _cursor;
    else if (_cursor >= _firstVisibleDataRow + visible)
      _firstVisibleDataRow = _cursor - visible + 1;
  }
  else
  {
    _firstVisibleDataRow =
        std::clamp(newCursor, 0, _dataRowCount - 1);
  }
}

// ------------------------------------------------------------
void UiTable::scrollVertical(int amount)
{
  if (_mode == Mode::CURSOR)
  {
    scrollTo(_cursor + amount);
  }
  else
  {
    scrollTo(_firstVisibleDataRow + amount);
  }
}

// ------------------------------------------------------------
void UiTable::scrollHorizontal(int amount)
{
  _dynamicColCurrentOffsetX =
      std::clamp(_dynamicColCurrentOffsetX + amount,
                 0,
                 std::max(0, _dynamicColMaxDataWidth - (_dynamicColViewWidth - 1)));
}

// ------------------------------------------------------------
bool UiTable::handleKey(int key)
{
  if (key == KEY_UP)
  {
    scrollVertical(-1);
    return true;
  }
  if (key == KEY_DOWN)
  {
    scrollVertical(1);
    return true;
  }
  if (key == KEY_PPAGE)
  {
    scrollVertical(-_lastKnownViewHeight);
    return true;
  }
  if (key == KEY_NPAGE)
  {
    scrollVertical(_lastKnownViewHeight);
    return true;
  }
  if (key == KEY_LEFT)
  {
    scrollHorizontal(-5);
    return true;
  }
  if (key == KEY_RIGHT)
  {
    scrollHorizontal(5);
    return true;
  }
  return false;
}

// ------------------------------------------------------------
void UiTable::delWindow()
{
  delwin(_win);
  _win = nullptr;
}

// ------------------------------------------------------------
void UiTable::buildWindow(int nlines, int ncols, int begy, int begx)
{
  _win = newwin(nlines, ncols, begy, begx);
}

// ------------------------------------------------------------
template<typename C>
Columns UiTable::renderHeaderImpl(
  int k,
  std::vector<C> const& headerCols,
  int borderStyle)
{
  werase(_win);
  keypad(_win, TRUE);

  int totalWidth = getmaxx(_win);
  Columns resolvedHeaderCols(totalWidth, headerCols);
  // ------------------------------------------------------------
  // Draw header + mid border
  // ------------------------------------------------------------
  draw_top_border_header(_win, 0, 0, resolvedHeaderCols.vec, borderStyle);

  if (resolvedHeaderCols.drawHeader)
  {
    draw_header_row(k, _win, 1, 0, headerCols, resolvedHeaderCols.vec, borderStyle);
  }
  return resolvedHeaderCols;
}
Columns UiTable::renderHeader(int k, std::vector<int> const&headerCols, int borderStyle){ return renderHeaderImpl(k, headerCols, borderStyle); }
Columns UiTable::renderHeader(int k, std::vector<HeaderColumn> const&headerCols, int borderStyle){ return renderHeaderImpl(k, headerCols, borderStyle); }
// ------------------------------------------------------------
UiTable::TableRender UiTable::renderStart(
  int k,
  int dataRowCount,
  const Columns &resolvedHeaderCols,
  int borderStyle)
{
  int h, w;
  getmaxyx(_win, h, w);

  if (k == KEY_MOUSE)
  {
    int beginRow, beginCol;
    getbegyx(_win, beginRow, beginCol);
    int endRow = h + beginRow;
    int endCol = w + beginCol;
    auto ev = getMouseEvent();
    if (ev && (ev->x >= beginCol && ev->x < endCol) &&
        (ev->y >= beginRow && ev->y < endRow))
    {
      _callback();
    }
  }

  _dataRowCount = dataRowCount;
  int totalInnerW = w - 2;
  // ------------------------------------------------------------
  // Resolve column widths
  // ------------------------------------------------------------
  auto it = std::ranges::find_if(resolvedHeaderCols.vec,
      [](auto const& hc){ return hc.width == HeaderColumn::FILL; });
  int dynamic_index = (it == resolvedHeaderCols.vec.end() ? -1 : it - resolvedHeaderCols.vec.begin());

  int viewContentFirstRow = resolvedHeaderCols.drawHeader ? 3 : 1;
  _lastKnownViewHeight = h - (resolvedHeaderCols.drawHeader? 4 : 2);

  _dynamicColViewWidth =
      (dynamic_index >= 0 ? resolvedHeaderCols.vec[dynamic_index].width : 0);
  // ------------------------------------------------------------
  // Vertical scrollbar
  // ------------------------------------------------------------
  auto vparams = scrollbar_thumb(
      _firstVisibleDataRow,
      dataRowCount,
      _lastKnownViewHeight,
      viewContentFirstRow,
      _lastKnownViewHeight);


  if (resolvedHeaderCols.drawHeader && h>3)
  {
    draw_mid_border_header(_win, 2, 0, resolvedHeaderCols.vec, borderStyle);
  }

  // ------------------------------------------------------------
  // Data rows
  // ------------------------------------------------------------
  _dynamicColMaxDataWidth = 0;
  return{*this,
          k,
          0, totalInnerW,
          dataRowCount,
          std::move(resolvedHeaderCols.vec),
          borderStyle,
          vparams,
          dynamic_index,
          viewContentFirstRow,
          std::min(dataRowCount,_lastKnownViewHeight)};
}
// ------------------------------------------------------------
UiTable::TableRender::~TableRender()
{
  for (int i = viewContentDataSize; i < table._lastKnownViewHeight; ++i)
  {
    TableRender::Row r = startRow(i);
    for (int c = 0 ; c < cols_def.size();++c)
    {
      draw(r, c, {});
    }
  }

  // ------------------------------------------------------------
  // Horizontal scrollbar
  // ------------------------------------------------------------
  auto hparams = scrollbar_thumb(
      table._dynamicColCurrentOffsetX,
      table._dynamicColMaxDataWidth,
      table._dynamicColViewWidth,
      1,
      totalInnerW);

  if (!hparams.first || !hparams.second ||
      (*hparams.first == 1 && *hparams.second == totalInnerW - 2))
  {
    draw_bottom_border_header(
        table._win,
        viewContentFirstRow + table._lastKnownViewHeight,
        0,
        cols_def,
        borderStyle);
  }
  else
  {
    draw_bottom_scroll_border(
        table._win,
        viewContentFirstRow + table._lastKnownViewHeight,
        0,
        totalInnerW,
        hparams,
        borderStyle);
  }

  wrefresh(table._win);
}

// ------------------------------------------------------------
void UiTable::renderArray(
    int k,
    const std::vector<Cell> &array,
    int borderStyle,
    const std::optional<std::wstring> &title)
{
  auto cellCallback = [&array](int row, int, std::optional<MouseEvent> const&) -> Cell
  {
    return array[row];
  };

  Columns cols = renderHeader(k, {
      HeaderColumn{.width=HeaderColumn::FILL, .name=title}}, borderStyle);

  render(k, static_cast<int>(array.size()), cols, cellCallback, borderStyle);
}
