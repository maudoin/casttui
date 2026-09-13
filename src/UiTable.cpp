#include "UiTable.h"
#include <algorithm>
#include <ranges>


// --------------------------------------------------------------------
// Safe write helpers
// --------------------------------------------------------------------
inline void safe_addstr(WINDOW* _win, int row, int col, const std::wstring& ch, int attrs)
{
    int h, w;
    getmaxyx(_win, h, w);

#ifdef DEBUG_UI
    if (row < 0 || row >= h || col < 0 || col >= w)
    {
        std::wcerr << "[OOB] row=" << row << " col=" << col
                    << " char=" << ch << " win_w=" << w << " win_h=" << h << "\n";
        std::wcerr << "=== CURSES ERROR TRACE ===\n";
        //StackTrace::print();
    }
#endif
    wattron(_win, attrs);
    mvwaddwstr(_win, row, col, ch.c_str());
    wattroff(_win, attrs);
}

inline void addstr_run(WINDOW* _win, int row, int col, const std::wstring& chars, int attrs)
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

inline void addstr_focus(WINDOW* _win, int row, int col, const std::wstring& chars, bool focused)
{
    addstr_run(_win, row, col, chars, attr_from_focus(focused));
}

// --------------------------------------------------------------------
// Border builders
// --------------------------------------------------------------------
inline void draw_bottom_scroll_border(
    WINDOW* _win,
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
            chars.append(L"▄");   // FIXED
        else
            chars.append(L"─");   // FIXED
    }

    addstr_focus(_win, row, col, std::wstring(L"╰") + chars, focused);
}

inline void draw_border_columns(
    WINDOW* _win,
    int row,
    int col,
    const std::vector<HeaderColumn>& cols,
    const std::wstring& start,
    const std::wstring& mid,
    const std::wstring& end,
    bool focused = false)
{
    addstr_focus(_win, row, col, start, focused);

    int x = 1;
    for (std::size_t i = 0; i < cols.size(); ++i)
    {
        std::wstring hrun;
        {
            hrun.reserve(cols[i].width * 3); // box chars are multi-byte
            for (int k = 0; k < cols[i].width; ++k)
            hrun.append(L"─");
        }
        addstr_focus(_win, row, col + x, hrun, focused);
        x += cols[i].width;
        if (i < cols.size() - 1)
        {
            addstr_focus(_win, row, col + x, mid, focused);
            x += 1;
        }
    }

    addstr_focus(_win, row, col + x, end, focused);
}

void draw_top_border_header(
    WINDOW* _win,
    int row,
    int col,
    const std::vector<HeaderColumn>& cols,
    bool focused)
{
    //box(_win, 0, 0);
    draw_border_columns(_win, row, col, cols, L"╭", L"┬", L"╮", focused);
}

void draw_mid_border_header(
    WINDOW* _win,
    int row,
    int col,
    const std::vector<HeaderColumn>& cols,
    bool focused)
{
    draw_border_columns(_win, row, col, cols, L"├", L"┼", L"┤", focused);
}

void draw_bottom_border_header(
    WINDOW* _win,
    int row,
    int col,
    const std::vector<HeaderColumn>& cols,
    bool focused)
{
    draw_border_columns(_win, row, col, cols, L"╰", L"┴", L"╯", focused);
}

void draw_top_border(
    WINDOW* _win,
    int row,
    int col,
    int inner_width,
    bool focused)
{
    draw_top_border_header(_win, row, col, { HeaderColumn{ inner_width } }, focused);
}

void draw_bottom_border(
    WINDOW* _win,
    int row,
    int col,
    int inner_width,
    bool focused)
{
    draw_bottom_border_header(_win, row, col, { HeaderColumn{ inner_width } }, focused);
}

inline std::wstring build_left_border()
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
    WINDOW* _win,
    int row,
    int col,
    const std::vector<HeaderColumn>& cols,
    UiHotspotGroup& hostHotspotGroup,
    bool focused = false)
{
    addstr_focus(_win, row, col, build_left_border(), focused);
    int x = 1;

    for (std::size_t i = 0; i < cols.size(); ++i)
    {
        const auto& header = cols[i];
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
            hostHotspotGroup.addLocalSpot(row, col + x, row+1, col + x + header.width, *(header.callback));
        }
        x += header.width;
    }

    addstr_focus(_win, row, col + x, build_right_border({ std::nullopt, std::nullopt }, 0), focused);
}

// ------------------------------------------------------------
// draw_row_assembled_cols()
// ------------------------------------------------------------
void draw_empty_border(
    WINDOW* _win,
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
// Scrollbar thumb
// --------------------------------------------------------------------
inline std::pair<std::optional<int>, std::optional<int>> scrollbar_thumb(
    int dataOffset,
    int dataSize,
    int dataViewSize,
    int uiOffset,
    int uiSize)
{
    if (uiSize <= 0 || dataSize <= dataViewSize)
        return { std::nullopt, std::nullopt };

    double ratio = static_cast<double>(dataViewSize) / static_cast<double>(dataSize);
    int thumbSize = std::max(1, static_cast<int>(uiSize * ratio));
    int maxOffset = dataSize - dataViewSize;

    int thumbStart = uiOffset +
        static_cast<int>((static_cast<double>(dataOffset) / static_cast<double>(maxOffset)) *
                         static_cast<double>(uiSize - thumbSize));

    return { thumbStart, thumbStart + thumbSize };
}

// ------------------------------------------------------------
// draw_row_assembled_cols()
// ------------------------------------------------------------
void draw_row_assembled_cols(
    WINDOW* _win,
    int row,
    int col,
    const std::vector<Cell>& cells,
    const std::vector<HeaderColumn>& cols_def,
    UiHotspotGroup& hostHotspotGroup,
    bool focused,
    int textOffset,
    std::pair<std::optional<int>, std::optional<int>> vparams)
{
    std::wstring left = build_left_border();
    std::wstring right = build_right_border(vparams, row);

    int x = 0;
    addstr_focus(_win, row, col + x, left, focused);
    x += 1;

    for (std::size_t i = 0; i < cells.size(); ++i)
    {
        if (i > 0)
        {
            addstr_focus(_win, row, col + x, L"│", focused);
            x += 1;
        }

        int col_text_offset = cols_def[i].dynamic ? textOffset : 0;
        int width = cols_def[i].width;

        auto const& cell = cells[i];

        std::wstring text = cell.text;

        if (col_text_offset > 0 && col_text_offset < (int)text.size())
            text = text.substr(col_text_offset);

        if ((int)text.size() < width)
            text.append(width - text.size(), ' ');
        else if ((int)text.size() > width)
            text = text.substr(0, width);

        addstr_run(_win, row, col + x, text, cell.style);

        if (cell.callback)
        {
            hostHotspotGroup.addLocalSpot(row, col + x, row+1, col + x + (int)text.size(), *(cell.callback));
        }

        x += (int)text.size();
    }

    addstr_focus(_win, row, col + x, right, focused);
}
// ------------------------------------------------------------
// Constructor
// ------------------------------------------------------------
UiTable::UiTable(Mode mode,
                 std::function<void()> const& callback,
                 int firstVisibleDataRow,
                 int dynamicColCurrentOffsetX)
    : UiHotspotGroup(callback)
    , _cursor(firstVisibleDataRow)
    , mode(mode)
    , _firstVisibleDataRow(firstVisibleDataRow)
    , _dynamicColCurrentOffsetX(dynamicColCurrentOffsetX)
    , dynamicColViewWidth(0)
    , dynamicColMaxDataWidth(0)
    , data_row_count(0)
    , lastKnownViewHeight(0)
    , first_data_row(0)
{
}

// ------------------------------------------------------------
// Vertical scrolling
// ------------------------------------------------------------
void UiTable::scrollTo(int newCursor)
{
    if (mode == Mode::CURSOR)
    {
        _cursor = std::clamp(newCursor, 0, data_row_count - 1);
        int visible = lastKnownViewHeight;

        if (_cursor < _firstVisibleDataRow)
            _firstVisibleDataRow = _cursor;
        else if (_cursor >= _firstVisibleDataRow + visible)
            _firstVisibleDataRow = _cursor - visible + 1;
    }
    else
    {
        _firstVisibleDataRow =
            std::clamp(newCursor, 0, data_row_count - 1);
    }
}

// ------------------------------------------------------------
// Vertical scrolling
// ------------------------------------------------------------
void UiTable::scrollVertical(int amount)
{
    if (mode == Mode::CURSOR)
    {
        scrollTo(_cursor + amount);
    }
    else
    {
        scrollTo(_firstVisibleDataRow + amount);
    }
}

// ------------------------------------------------------------
// Horizontal scrolling
// ------------------------------------------------------------
void UiTable::scrollHorizontal(int amount)
{
    _dynamicColCurrentOffsetX =
        std::clamp(_dynamicColCurrentOffsetX + amount,
                   0,
                   std::max(0, dynamicColMaxDataWidth - (dynamicColViewWidth - 1)));
}

// ------------------------------------------------------------
// Key handling
// ------------------------------------------------------------
bool UiTable::handleKeyCh(int key)
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
        scrollVertical(-lastKnownViewHeight);
        return true;
    }
    if (key == KEY_NPAGE)
    {
        scrollVertical(lastKnownViewHeight);
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


void UiTable::delWindow()
{
    delwin(_win);
    _win = nullptr;
}
void UiTable::buildWindow(int nlines, int ncols, int begy, int begx)
{
    _win = newwin(nlines, ncols, begy, begx);
}
// ------------------------------------------------------------
// Main render()
// ------------------------------------------------------------
void UiTable::render(
    int dataRowCount,
    const std::vector<HeaderColumn>& header_cols,
    const std::function<Cell(int, int)>& cell_cb,
    bool focused)
{
    if (!_win)
    {
        return;
    }
    setWin(_win);
    data_row_count = dataRowCount;

    int h, w;
    getmaxyx(_win, h, w);
    werase(_win);
    keypad(_win, TRUE);

    // ------------------------------------------------------------
    // Resolve column widths
    // ------------------------------------------------------------
    std::vector<int> cols;
    cols.reserve(header_cols.size());

    int fixed = 0;
    int num_fill = 0;

    for (auto const& hc : header_cols)
    {
        if (hc.width > 0)
            fixed += hc.width;
        else
            num_fill++;
    }

    int total_inner = w - 2;
    int sep_space = static_cast<int>(header_cols.size()) - 1;
    int remaining = total_inner - fixed - sep_space;
    int fill_width = num_fill > 0 ? remaining / num_fill : 0;

    for (auto const& hc : header_cols)
        cols.push_back(hc.width > 0 ? hc.width : fill_width);

    bool has_header =
        std::ranges::any_of(header_cols,
            [](auto const& hc) { return hc.name.has_value(); });

    std::vector<HeaderColumn> resolved_header_cols;
    resolved_header_cols.reserve(header_cols.size());

    for (auto const& hc : header_cols)
    {
        HeaderColumn r = hc;
        r.width = hc.width > 0 ? hc.width : fill_width;
        r.dynamic = hc.width == 0;
        resolved_header_cols.push_back(r);
    }

    if (has_header)
    {
        lastKnownViewHeight = h - 4;
        first_data_row = 3;
    }
    else
    {
        lastKnownViewHeight = h - 2;
        first_data_row = 1;
    }

    // ------------------------------------------------------------
    // Vertical scrollbar
    // ------------------------------------------------------------
    auto vparams = scrollbar_thumb(
        _firstVisibleDataRow,
        dataRowCount,
        lastKnownViewHeight,
        has_header ? 3 : 1,
        lastKnownViewHeight);

    int inner_width = 0;
    for (auto c : cols) inner_width += c;
    inner_width += static_cast<int>(cols.size()) - 1;

    int dynamic_index = -1;
    for (std::size_t i = 0; i < header_cols.size(); ++i)
    {
        if (header_cols[i].dynamic)
        {
            dynamic_index = static_cast<int>(i);
            break;
        }
    }

    dynamicColViewWidth =
        (dynamic_index >= 0 ? resolved_header_cols[dynamic_index].width : 0);

    // ------------------------------------------------------------
    // Draw header + mid border
    // ------------------------------------------------------------
    draw_top_border_header(_win, 0, 0, resolved_header_cols, focused);

    if (has_header)
    {
        draw_header_row(_win, 1, 0, resolved_header_cols, *this, focused);
        draw_mid_border_header(_win, 2, 0, resolved_header_cols, focused);
    }

    // ------------------------------------------------------------
    // Data rows
    // ------------------------------------------------------------
    dynamicColMaxDataWidth = 0;
    std::vector<Cell> cell_objs;
    std::vector<Cell> cells;

    for (int i = 0; i < lastKnownViewHeight; ++i)
    {
        int data_row = _firstVisibleDataRow + i;
        if (data_row >= dataRowCount)
        {
            draw_empty_border(_win, first_data_row + i, 0, w-1, focused, vparams);
            continue;
        }

        cells.clear();
        cells.reserve(cols.size());

        for (int col_index = 0; col_index < static_cast<int>(cols.size()); ++col_index)
        {
            Cell cell = cell_cb(data_row, col_index);
            if ((int)cell.text.size() < cols[col_index])
                cell.text.append(cols[col_index] - cell.text.size(), ' ');
            cells.emplace_back(cell);
        }

        if (dynamic_index >= 0)
            dynamicColMaxDataWidth =
                std::max(dynamicColMaxDataWidth,
                         static_cast<int>(cells[dynamic_index].text.size()));

        draw_row_assembled_cols(
            _win,
            first_data_row + i,
            0,
            cells,
            resolved_header_cols,
            *this,
            focused,
            _dynamicColCurrentOffsetX,
            vparams);

        wnoutrefresh(_win);
    }

    // ------------------------------------------------------------
    // Horizontal scrollbar
    // ------------------------------------------------------------
    auto hparams = scrollbar_thumb(
        _dynamicColCurrentOffsetX,
        dynamicColMaxDataWidth,
        dynamicColViewWidth,
        1,
        inner_width);

    if (!hparams.first || !hparams.second ||
        (*hparams.first == 1 && *hparams.second == inner_width - 2))
    {
        draw_bottom_border_header(
            _win,
            first_data_row + lastKnownViewHeight,
            0,
            resolved_header_cols,
            focused);
    }
    else
    {
        draw_bottom_scroll_border(
            _win,
            first_data_row + lastKnownViewHeight,
            0,
            inner_width,
            hparams,
            focused);
    }

    wrefresh(_win);
}

// ------------------------------------------------------------
// renderArray()
// ------------------------------------------------------------
void UiTable::renderArray(
    const std::vector<std::wstring>& array,
    const std::optional<std::wstring>& title,
    bool focused)
{
    setWin(_win);
    auto cell_cb = [&array](int row, int) -> Cell {
        return Cell{ array[row], A_NORMAL };
    };

    std::vector<HeaderColumn> cols{
        HeaderColumn{ 0, title, SortDir::NONE, true }
    };

    render(static_cast<int>(array.size()), cols, cell_cb, focused);
}

