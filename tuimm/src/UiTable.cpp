#include "UiTable.h"

#if defined(_WIN32)
#include <curses.h>
#else
#include <ncursesw/curses.h>
#endif

#include <algorithm>
#include <ranges>

inline bool DEBUG_UI = false;

// PIMPL
struct UiTableData
{
  WINDOW *win = nullptr;
};

namespace{
struct HLineCharacters{std::wstring left, mid, right;};
static const std::wstring VLineCharacter(L"│");
static const std::wstring VScrollCharacter(L"█");
static const std::wstring HLineCharacter(L"─");
static const std::wstring HScrollCharacter(L"▄");
static const HLineCharacters TopLineCharacters{L"╭", L"┬", L"╮"};
static const HLineCharacters MidLineCharacters{L"├", L"┼", L"┤"};
static const HLineCharacters BottomLineCharacters{L"╰", L"┴", L"╯"};

// --------------------------------------------------------------------
// Safe write helpers
// --------------------------------------------------------------------
#ifdef DEBUG_UI
inline void safe_addstr(WINDOW *win, int row, int col, const std::wstring &ch, int attrs)
{
  int h, w;
  getmaxyx(win, h, w);

  if (row < 0 || row >= h || col < 0 || col >= w)
  {
    std::wcerr << "[OOB] row=" << row << " col=" << col
               << " char=" << ch << " win_w=" << w << " win_h=" << h << "\n";
    std::wcerr << "=== CURSES ERROR TRACE ===\n";
    // StackTrace::print();
  }
  wattron(win, attrs);
  mvwaddwstr(win, row, col, ch.c_str());
  wattroff(win, attrs);
}

inline void mvwaddwstr_watt(WINDOW *win, int row, int col, const std::wstring &chars, int attrs)
{
  for (std::size_t i = 0; i < chars.size(); ++i)
  {
    safe_addstr(win, row, col + static_cast<int>(i),
                std::wstring(1, chars[i]), attrs);
  }
}
#else
inline void mvwaddwstr_watt(WINDOW *win, int row, int col, const std::wstring &chars, int attrs)
{
  wattron(win, attrs);
  mvwaddwstr(win, row, col, chars.c_str());
  wattroff(win, attrs);
}
#endif

// --------------------------------------------------------------------
// Border builders
// --------------------------------------------------------------------
inline void draw_bottom_scroll_border(
    WINDOW *win,
    int row,
    int col,
    int inner_width,
    HLineCharacters const& hLineCharacters,
    std::pair<std::optional<int>, std::optional<int>> hparams,
    int borderStyle)
{
  if (inner_width<=0)
  {
    return;
  }
  auto [startOpt, endOpt] = hparams;
  mvwaddwstr_watt(win, row, col, hLineCharacters.left, borderStyle);
  for (int c = 0; c < inner_width; ++c)
  {
    auto const& midChar =  (startOpt && endOpt && *startOpt <= c && c < *endOpt)?HScrollCharacter:HLineCharacter;
    mvwaddwstr_watt(win, row, col+c, midChar, borderStyle);
  }
  mvwaddwstr_watt(win, row, col+inner_width-1, hLineCharacters.right, borderStyle);
}

inline void draw_border_columns(
    WINDOW *win,
    int row,
    int col,
    const std::vector<ColumnRange> &cols,
    HLineCharacters const& hLineCharacters,
    int borderStyle)
{
  mvwaddwstr_watt(win, row, col, hLineCharacters.left, borderStyle);
  for (std::size_t i = 0; i < cols.size(); ++i)
  {
    auto const& range = cols[i];
    mvwaddwstr_watt(win, row, col + range.start, std::wstring(range.width, L'─'), borderStyle);
    mvwaddwstr_watt(win, row, col + range.start + range.width, (i < cols.size() - 1)?hLineCharacters.mid:hLineCharacters.right, borderStyle);
  }
}

void draw_top_border_header(
    WINDOW *win,
    int row,
    int col,
    const std::vector<ColumnRange> &cols,
    int borderStyle)
{
  // box(win, 0, 0);
  draw_border_columns(win, row, col, cols, TopLineCharacters, borderStyle);
}

void draw_mid_border_header(
    WINDOW *win,
    int row,
    int col,
    const std::vector<ColumnRange> &cols,
    int borderStyle)
{
  draw_border_columns(win, row, col, cols, MidLineCharacters, borderStyle);
}

void draw_bottom_border_header(
    WINDOW *win,
    int row,
    int col,
    const std::vector<ColumnRange> &cols,
    int borderStyle)
{
  draw_border_columns(win, row, col, cols, BottomLineCharacters, borderStyle);
}

inline void draw_transition_separator(
    WINDOW *win,
    int row,
    int col,
    const std::vector<ColumnRange> &before,
    const std::vector<ColumnRange> &after,
    int borderStyle)
{
    mvwaddwstr_watt(win, row, col, MidLineCharacters.left, borderStyle);

    std::size_t i = 0, j = 0;

    auto nextPos = [&](std::size_t idx, const std::vector<ColumnRange> &v)
    {
        return (idx < v.size())
            ? col + v[idx].start + v[idx].width
            : INT_MAX;
    };

    int nextBefore = nextPos(i, before);
    int nextAfter  = nextPos(j, after);

    int x = col + 1;

    while (nextBefore != INT_MAX || nextAfter != INT_MAX)
    {
        int const next = std::min(nextBefore, nextAfter);

        while (x < next)
            mvwaddwstr_watt(win, row, x++, HLineCharacter, borderStyle);

        bool const b = (x == nextBefore);
        bool const a = (x == nextAfter);

        std::wstring const& str =
            (b && a && i == (before.size()-1)) ? MidLineCharacters.right :
            (b && a) ? MidLineCharacters.mid :
            (b)      ? BottomLineCharacters.mid :
            (a)      ? TopLineCharacters.mid :
                       HLineCharacter;

        mvwaddwstr_watt(win, row, x++, str, borderStyle);

        if (b) nextBefore = nextPos(++i, before);
        if (a) nextAfter  = nextPos(++j, after);
    }
}

void draw_top_border(
    WINDOW *win,
    int row,
    int col,
    int inner_width,
    int borderStyle)
{
  draw_top_border_header(win, row, col, {ColumnRange{0,inner_width}}, borderStyle);
}

void draw_bottom_border(
    WINDOW *win,
    int row,
    int col,
    int inner_width,
    int borderStyle)
{
  draw_bottom_border_header(win, row, col, {ColumnRange{0,inner_width}}, borderStyle);
}

inline std::wstring build_left_border()
{
  return VLineCharacter;
}

inline std::wstring build_separator()
{
  return VLineCharacter;
}

inline std::wstring build_right_border(
    std::pair<std::optional<int>, std::optional<int>> vparams,
    int row)
{
  auto [startOpt, endOpt] = vparams;
  if (startOpt && endOpt && *startOpt <= row && row < *endOpt)
    return VScrollCharacter;
  return VLineCharacter;
}

namespace{
inline int width(int width){return std::max(0,width);}
inline int width(HeaderColumn const& c){return (c.width == HeaderColumn::FIT_LABEL && c.name)?c.name->size():width(c.width);}
inline bool fill(int width){return width == HeaderColumn::FILL;}
inline bool fill(HeaderColumn const& c){return fill(c.width);}
}
// ------------------------------------------------------------
UiInput::MouseEvent toLocal(WINDOW* win, UiInput::MouseEvent const& ev)
{
    int beginRow, beginCol;
    getbegyx(win, beginRow, beginCol);
    UiInput::MouseEvent locaEv = ev;
    locaEv.x -= beginCol;
    locaEv.y -= beginRow;
    return locaEv;
}
}
// ------------------------------------------------------------
std::optional<UiInput::MouseEvent> getEvent(
  UiInput const& input,
  WINDOW *win,
  int row,
  int col,
  ColumnRange const& colRange)
{
  if (input.mev)
  {
    UiInput::MouseEvent mev = toLocal(win, *(input.mev));
    if (mev.hit({row, col + colRange.start, row + 1, col + colRange.start + colRange.width}))
    {
      return mev;
    }
  }
  return std::nullopt;
}
// ------------------------------------------------------------
template<typename C>
inline void draw_header_row(
  UiInput const& input,
  WINDOW *win,
  int row,
  int col,
  const std::vector<C> &cols,
  const std::vector<ColumnRange> &colRanges,
  int borderStyle)
{
  mvwaddwstr_watt(win, row, col, build_left_border(), borderStyle);

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

    mvwaddwstr_watt(win, row, col + headerRange.start, cell, borderStyle);
    mvwaddwstr_watt(win, row, col + headerRange.start + headerRange.width,
      (i == cols.size()-1)?build_right_border({std::nullopt, std::nullopt}, 0):VLineCharacter, borderStyle);
    if constexpr(requires (C c){c.name;})
    if (header.callback)
    {
      if (getEvent(input, win, row, col, headerRange))
      {
        (*header.callback)();
      }
    }
  }
}

// ------------------------------------------------------------
void draw_empty_border(
    WINDOW *win,
    int row,
    int col,
    int width,
    int borderStyle,
    std::pair<std::optional<int>, std::optional<int>> vparams)
{
  mvwaddwstr_watt(win, row, col, build_left_border(), borderStyle);
  mvwaddwstr_watt(win, row, col + width, build_right_border(vparams, row), borderStyle);
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
// ------------------------------------------------------------
std::optional<UiInput::MouseEvent> UiTable::TableRender::getEvent(Row& r, int c)
{
  return ::getEvent(input, table._pimpl->win, viewContentFirstRow+r.i, col, this->cols_def[c]);
}
void UiTable::TableRender::draw(Row& r, int i, Cell const& cell)
{
  int row = viewContentFirstRow+r.i;
  mvwaddwstr_watt(table._pimpl->win, row, col + r.x, i==0?build_left_border():build_separator(), borderStyle);

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

  mvwaddwstr_watt(table._pimpl->win, row, col + r.x + 1, text, cell.style);
  r.x += width + 1;
}
// ------------------------------------------------------------
void UiTable::TableRender::endRow(Row const& r)
{
  int row = viewContentFirstRow+r.i;
  mvwaddwstr_watt(table._pimpl->win, row, col + r.x, build_right_border(vparams, row), borderStyle);
}
// ------------------------------------------------------------
template <typename C>
Columns::Columns(int totalWidth, std::vector<C> const&headerCols, std::optional<Columns> const& prev)
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
  rowOffset = (prev?prev->rowOffset:0) + (drawHeader?2:0);
}
// ------------------------------------------------------------
// CLASS
// ------------------------------------------------------------
UiTable::UiTable(Mode mode,
                 std::function<void()> const &callback,
                 int firstVisibleDataRow,
                 int dynamicColCurrentOffsetX)
  : _pimpl(new UiTableData())
  , _callback(callback)
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
UiTable::~UiTable()
{
  delete _pimpl;
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
bool UiTable::handleKey(UiInput const& input)
{
  if (input.keyUp())
  {
    scrollVertical(-1);
    return true;
  }
  if (input.keyDown())
  {
    scrollVertical(1);
    return true;
  }
  if (input.keyPageUp())
  {
    scrollVertical(-_lastKnownViewHeight);
    return true;
  }
  if (input.keyPageDown())
  {
    scrollVertical(_lastKnownViewHeight);
    return true;
  }
  if (input.keyLeft())
  {
    scrollHorizontal(-5);
    return true;
  }
  if (input.keyRight())
  {
    scrollHorizontal(5);
    return true;
  }
  return false;
}

// ------------------------------------------------------------
void UiTable::delWindow()
{
  delwin(_pimpl->win);
  _pimpl->win = nullptr;
}

// ------------------------------------------------------------
void UiTable::buildWindow(int nlines, int ncols, int begy, int begx)
{
  _pimpl->win = newwin(nlines, ncols, begy, begx);
}

// ------------------------------------------------------------
template<typename C>
Columns UiTable::renderHeaderImpl(
  UiInput const& input,
  std::vector<C> const& headerCols,
  int borderStyle,
  std::optional<Columns> const& previousColumns)
{
  if (!previousColumns)
  {
    werase(_pimpl->win);
    keypad(_pimpl->win, TRUE);
  }

  int totalWidth = getmaxx(_pimpl->win);
  Columns resolvedHeaderCols(totalWidth, headerCols, previousColumns);
  // ------------------------------------------------------------
  // Draw header + mid border
  // ------------------------------------------------------------
  if (previousColumns)
  {
    draw_transition_separator(_pimpl->win, previousColumns->rowOffset, 0, previousColumns->vec, resolvedHeaderCols.vec, borderStyle);
  }
  else
  {
    draw_top_border_header(_pimpl->win, 0, 0, resolvedHeaderCols.vec, borderStyle);
  }

  if (resolvedHeaderCols.drawHeader)
  {
    draw_header_row(input, _pimpl->win, (previousColumns?previousColumns->rowOffset:0)+1, 0, headerCols, resolvedHeaderCols.vec, borderStyle);
  }
  return resolvedHeaderCols;
}
Columns UiTable::renderHeader(UiInput const& input, std::vector<int> const&headerCols, int borderStyle,
   std::optional<Columns> const& previousColumns)
    { return renderHeaderImpl(input, headerCols, borderStyle, previousColumns); }
Columns UiTable::renderHeader(UiInput const& input, std::vector<HeaderColumn> const&headerCols, int borderStyle,
   std::optional<Columns> const& previousColumns)
    { return renderHeaderImpl(input, headerCols, borderStyle, previousColumns); }
// ------------------------------------------------------------
bool UiTable::mouseHit(UiInput const& input)
{
  if (input.mev)
  {
    int beginRow, beginCol;
    int h, w;
    getbegyx(_pimpl->win, beginRow, beginCol);
    getmaxyx(_pimpl->win, h, w);
    int endRow = h + beginRow;
    int endCol = w + beginCol;
    if ((input.mev->x >= beginCol && input.mev->x < endCol) &&
        (input.mev->y >= beginRow && input.mev->y < endRow))
    {
      return true;
    }
  }
  return false;
}
// ------------------------------------------------------------
UiTable::TableRender UiTable::renderStart(
  UiInput const& input,
  int dataRowCount,
  const Columns &resolvedHeaderCols,
  int borderStyle,
  UiTable::RowReserve const& rowReserve)
{
  int h = getmaxy(_pimpl->win) - (rowReserve.index ? *rowReserve.index : 0);
  int w = getmaxx(_pimpl->win);

  if (input.mev)
  {
    int beginRow = getbegy(_pimpl->win) ;
    int beginCol = getbegx(_pimpl->win);
    int endRow = h + beginRow;
    int endCol = w + beginCol;
    if (input.mev && (input.mev->x >= beginCol && input.mev->x < endCol) &&
        (input.mev->y >= beginRow && input.mev->y < endRow))
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

  int viewContentFirstRow = 1 + resolvedHeaderCols.rowOffset;
  _lastKnownViewHeight = h - (2 + resolvedHeaderCols.rowOffset); // 2 = reserve top and bottom lines

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
    draw_mid_border_header(_pimpl->win, resolvedHeaderCols.rowOffset, 0, resolvedHeaderCols.vec, borderStyle);
  }

  // ------------------------------------------------------------
  // Data rows
  // ------------------------------------------------------------
  _dynamicColMaxDataWidth = 0;
  return
  {
    *this,
    input,
    0, totalInnerW,
    dataRowCount,
    std::move(resolvedHeaderCols.vec),
    borderStyle,
    vparams,
    dynamic_index,
    viewContentFirstRow,
    std::min(dataRowCount,_lastKnownViewHeight),
    rowReserve.index.has_value()
  };
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
    if (!limitedHeight)
    {
      draw_border_columns(
        table._pimpl->win,
        viewContentFirstRow + table._lastKnownViewHeight,
        0,
        cols_def,
        limitedHeight?MidLineCharacters:BottomLineCharacters,
        borderStyle);
    }
  }
  else
  {
    draw_bottom_scroll_border(
      table._pimpl->win,
      viewContentFirstRow + table._lastKnownViewHeight,
      0,
      totalInnerW,
      limitedHeight?MidLineCharacters:BottomLineCharacters,
      hparams,
      borderStyle);
  }

  wrefresh(table._pimpl->win);
}

// ------------------------------------------------------------
int UiTable::getHeight() const
{
  return getmaxy(_pimpl->win);
}