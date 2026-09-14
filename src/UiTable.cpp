#include "UiTable.h"
#include <algorithm>
#include <ranges>

namespace{
// --------------------------------------------------------------------
// Safe write helpers
// --------------------------------------------------------------------
inline void safe_addstr(WINDOW *_win, int row, int col, const std::wstring &ch, int attrs)
{
  int h, w;
  getmaxyx(_win, h, w);

#ifdef DEBUG_UI
  if (row < 0 || row >= h || col < 0 || col >= w)
  {
    std::wcerr << "[OOB] row=" << row << " col=" << col
               << " char=" << ch << " win_w=" << w << " win_h=" << h << "\n";
    std::wcerr << "=== CURSES ERROR TRACE ===\n";
    // StackTrace::print();
  }
#endif
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

inline int attr_from_focus(bool focused)
{
  return focused ? COLOR_PAIR(4) : A_NORMAL;
}

inline void addstr_focus(WINDOW *_win, int row, int col, const std::wstring &chars, bool focused)
{
  addstr_run(_win, row, col, chars, attr_from_focus(focused));
}

// --------------------------------------------------------------------
// Border builders
// --------------------------------------------------------------------
inline void draw_bottom_scroll_border(
    WINDOW *_win,
    int row,
    int col,
    int inner_width,
    std::pair<std::optional<int>, std::optional<int>> hparams,
    bool focused = false)
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

  addstr_focus(_win, row, col, std::wstring(L"╰") + chars + std::wstring(L"╯"), focused);
}

inline void draw_border_columns(
    WINDOW *_win,
    int row,
    int col,
    const std::vector<HeaderColumn> &cols,
    const std::wstring &start,
    const std::wstring &mid,
    const std::wstring &end,
    bool focused = false)
{
  addstr_focus(_win, row, col, start, focused);

  int x = 1;
  for (std::size_t i = 0; i < cols.size(); ++i)
  {
    int w = std::max(0, cols[i].width);
    std::wstring hrun;
    {
      hrun.reserve(w * 3); // box chars are multi-byte
      for (int k = 0; k < w; ++k)
        hrun.append(L"─");
    }
    addstr_focus(_win, row, col + x, hrun, focused);
    x += w;
    if (i < cols.size() - 1)
    {
      addstr_focus(_win, row, col + x, mid, focused);
      x += 1;
    }
  }

  addstr_focus(_win, row, col + x, end, focused);
}

void draw_top_border_header(
    WINDOW *_win,
    int row,
    int col,
    const std::vector<HeaderColumn> &cols,
    bool focused)
{
  // box(_win, 0, 0);
  draw_border_columns(_win, row, col, cols, L"╭", L"┬", L"╮", focused);
}

void draw_mid_border_header(
    WINDOW *_win,
    int row,
    int col,
    const std::vector<HeaderColumn> &cols,
    bool focused)
{
  draw_border_columns(_win, row, col, cols, L"├", L"┼", L"┤", focused);
}

void draw_bottom_border_header(
    WINDOW *_win,
    int row,
    int col,
    const std::vector<HeaderColumn> &cols,
    bool focused)
{
  draw_border_columns(_win, row, col, cols, L"╰", L"┴", L"╯", focused);
}

void draw_top_border(
    WINDOW *_win,
    int row,
    int col,
    int inner_width,
    bool focused)
{
  draw_top_border_header(_win, row, col, {HeaderColumn{inner_width}}, focused);
}

void draw_bottom_border(
    WINDOW *_win,
    int row,
    int col,
    int inner_width,
    bool focused)
{
  draw_bottom_border_header(_win, row, col, {HeaderColumn{inner_width}}, focused);
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

inline void draw_header_row(
    WINDOW *_win,
    int row,
    int col,
    const std::vector<HeaderColumn> &cols,
    UiHotspotGroup &hostHotspotGroup,
    bool focused = false)
{
  addstr_focus(_win, row, col, build_left_border(), focused);
  int x = 1;

  for (std::size_t i = 0; i < cols.size(); ++i)
  {
    const auto &header = cols[i];
    std::wstring cell;

    if (header.name)
    {
      cell = *header.name;
      if (header.sort == SortDir::UP)
        cell += L" ↑";
      else if (header.sort == SortDir::DOWN)
        cell += L" ↓";
    }

    if (i > 0)
    {
      addstr_focus(_win, row, col + x, L"│", focused);
      x += 1;
    }

    if (static_cast<int>(cell.size()) < header.width)
      cell.append(header.width - cell.size(), ' ');
    else if (static_cast<int>(cell.size()) > header.width)
      cell = cell.substr(0, header.width);

    addstr_focus(_win, row, col + x, cell, focused);
    if (header.callback)
    {
      hostHotspotGroup.addLocalSpot(row, col + x, row + 1, col + x + header.width, *(header.callback));
    }
    x += header.width;
  }

  addstr_focus(_win, row, col + x, build_right_border({std::nullopt, std::nullopt}, 0), focused);
}

// ------------------------------------------------------------
void draw_empty_border(
    WINDOW *_win,
    int row,
    int col,
    int width,
    bool focused,
    std::pair<std::optional<int>, std::optional<int>> vparams)
{
  addstr_focus(_win, row, col, build_left_border(), focused);
  addstr_focus(_win, row, col + width, build_right_border(vparams, row), focused);
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
std::optional<MouseEvent> UiTable::TableRender::getEvent(Row& r, int i)
{
  if (k == KEY_MOUSE)
  {
    if (auto ev = getMouseEvent())
    {
      MouseEvent mev = ev->toLocal(table._win);
      int row = viewContentFirstRow+r.i;
      if (mev.hit({row, col + r.x + 1, row + 1, col + r.x + 1 + cols_def[i].width}))
      {
        return ev;
      }
    }
  }
  return std::nullopt;
}
void UiTable::TableRender::draw(Row& r, int i, Cell const& cell)
{
  int row = viewContentFirstRow+r.i;
  addstr_focus(table._win, row, col + r.x, i==0?build_left_border():build_separator(), focused);

  int col_text_offset = cols_def[i].dynamic ? table._dynamicColCurrentOffsetX : 0;
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
  addstr_focus(table._win, row, col + r.x, build_right_border(vparams, row), focused);
}
// ------------------------------------------------------------
namespace{
std::vector<HeaderColumn>
resolveHeaderWidth(const std::vector<HeaderColumn> &headerCols, int totalInnerW)
{
  int fixed = 0;
  int num_fill = 0;
  for (std::size_t i = 0; i < headerCols.size(); ++i)
  {
    if (headerCols[i].width > 0 && !headerCols[i].dynamic)
      fixed += headerCols[i].width;
    else if (headerCols[i].fit && headerCols[i].name)
      fixed += headerCols[i].name->size();
    else
    {
      num_fill++;
    }
  }

  int sep_space = static_cast<int>(headerCols.size()) - 1;
  int remaining = totalInnerW - fixed - sep_space;
  int fill_width = num_fill > 0 ? remaining / num_fill : 0;


  std::vector<HeaderColumn> resolvedHeaderCols;
  resolvedHeaderCols.reserve(headerCols.size());
  for (auto const &hc : headerCols)
  {
    HeaderColumn r = hc;
    r.width = (hc.fit && hc.name) ? hc.name->size() : hc.width > 0 ? hc.width : fill_width;
    r.dynamic |= hc.width <= 0;
    resolvedHeaderCols.push_back(r);
  }
  return resolvedHeaderCols;
}
}
// ------------------------------------------------------------
// CLASS
// ------------------------------------------------------------
UiTable::UiTable(Mode mode,
                 std::function<void()> const &callback,
                 int firstVisibleDataRow,
                 int dynamicColCurrentOffsetX)
  : UiHotspotGroup(callback)
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
UiTable::TableRender UiTable::renderStart(
  int k,
  int dataRowCount,
  const std::vector<HeaderColumn> &headerCols,
  bool focused)
{
  setWin(_win);
  werase(_win);
  keypad(_win, TRUE);

  _dataRowCount = dataRowCount;

  int h, w;
  getmaxyx(_win, h, w);
  int totalInnerW = w - 2;

  // ------------------------------------------------------------
  // Resolve column widths
  // ------------------------------------------------------------
  auto it = std::ranges::find_if(headerCols,
      [](auto const& hc){ return hc.width <= 0 || hc.dynamic; });
  int dynamic_index = (it == headerCols.end() ? -1 : it - headerCols.begin());

  bool hasHeader = std::ranges::any_of(headerCols,
    [](auto const &hc){ return hc.name.has_value(); });
  int viewContentFirstRow = hasHeader ? 3 : 1;
  _lastKnownViewHeight = h - (hasHeader? 4 : 2);

  std::vector<HeaderColumn> resolvedHeaderCols = resolveHeaderWidth(headerCols, totalInnerW);

  _dynamicColViewWidth =
      (dynamic_index >= 0 ? resolvedHeaderCols[dynamic_index].width : 0);
  // ------------------------------------------------------------
  // Vertical scrollbar
  // ------------------------------------------------------------
  auto vparams = scrollbar_thumb(
      _firstVisibleDataRow,
      dataRowCount,
      _lastKnownViewHeight,
      viewContentFirstRow,
      _lastKnownViewHeight);

  // ------------------------------------------------------------
  // Draw header + mid border
  // ------------------------------------------------------------
  draw_top_border_header(_win, 0, 0, resolvedHeaderCols, focused);

  if (hasHeader)
  {
    draw_header_row(_win, 1, 0, resolvedHeaderCols, *this, focused);
    if (h>3)
    {
      draw_mid_border_header(_win, 2, 0, resolvedHeaderCols, focused);
    }
  }

  // ------------------------------------------------------------
  // Data rows
  // ------------------------------------------------------------
  _dynamicColMaxDataWidth = 0;
  return{*this,
          k,
          0, totalInnerW,
          dataRowCount,
          std::move(resolvedHeaderCols),
          focused,
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
        focused);
  }
  else
  {
    draw_bottom_scroll_border(
        table._win,
        viewContentFirstRow + table._lastKnownViewHeight,
        0,
        totalInnerW,
        hparams,
        focused);
  }

  wrefresh(table._win);
}

// ------------------------------------------------------------
void UiTable::renderArray(
    int k,
    const std::vector<Cell> &array,
    const std::optional<std::wstring> &title,
    bool focused)
{
  auto cell_cb = [&array](int row, int, std::optional<MouseEvent> const&) -> Cell
  {
    return array[row];
  };

  std::vector<HeaderColumn> cols{
      HeaderColumn{0, title, SortDir::NONE, true}};

  render(k, static_cast<int>(array.size()), cols, cell_cb, focused);
}

// ------------------------------------------------------------
void UiTable::renderSingleLine(
    int k,
    const std::vector<Cell> &array,
    bool focused)
{
  auto cell_cb = [&array](int, int col, std::optional<MouseEvent> const&) -> Cell
  {
    return array[col];
  };
  auto colsView = array | std::views::transform([](const Cell &c)
                                                { return HeaderColumn{
                                                      .width = static_cast<int>(c.text.size()),
                                                  }; });
  std::vector<HeaderColumn> cols(colsView.begin(), colsView.end());
  if (!cols.empty())
    cols.back().width = -1;
  render(k, 1, cols, cell_cb, focused);
}
