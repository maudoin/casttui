#pragma once

#include "UiHotSpot.h"

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
    std::optional<std::wstring> name;
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

    UiTable(Mode mode = Mode::SCROLL,
            std::function<void()> const& callback = []{},
            int firstVisibleDataRow = 0,
            int dynamicColCurrentOffsetX = 0);

    void delWindow();
    void buildWindow(int nlines, int ncols, int begy, int begx);
    void scrollTo(int newCursor);
    void scrollVertical(int amount);
    void scrollHorizontal(int amount);

    bool handleKeyCh(int key);

    void render(
        int dataRowCount,
        const std::vector<HeaderColumn>& header_cols,
        const std::function<Cell(int, int)>& cell_cb,
        bool focused = false);

    void renderArray(
        const std::vector<std::wstring>& array,
        const std::optional<std::wstring>& title = std::nullopt,
        bool focused = false);

    int cursor() const {return _cursor;}
    int firstVisibleDataRow() const {return _firstVisibleDataRow;}
    int dynamicColCurrentOffsetX() const {return _dynamicColCurrentOffsetX;}

    int getHeight()const
    {
        return getmaxy(_win);
    }
protected:
    WINDOW* _win = nullptr;
private:
    int _cursor;
    Mode mode;
    int _firstVisibleDataRow;
    int _dynamicColCurrentOffsetX;

    int dynamicColViewWidth;
    int dynamicColMaxDataWidth;

    int data_row_count;
    int lastKnownViewHeight;
    int first_data_row;
};

// free functions
void draw_top_border(WINDOW* win, int row, int col, int width, bool focused = false);
void draw_bottom_border(WINDOW* win, int row, int col, int width, bool focused = false);
void draw_top_border_header(WINDOW* win, int row, int col,
                            const std::vector<HeaderColumn>& cols, bool focused = false);
void draw_mid_border_header(WINDOW* win, int row, int col,
                            const std::vector<HeaderColumn>& cols, bool focused = false);
void draw_bottom_border_header(WINDOW* win, int row, int col,
                               const std::vector<HeaderColumn>& cols, bool focused = false);
void draw_row_assembled_cols(WINDOW* win, int row, int col,
                             const std::vector<Cell>& cells,
                             const std::vector<HeaderColumn>& cols_def,
                             UiHotspotGroup& hostHotspotGroup,
                             bool focused = false,
                             int textOffset=0,
                             std::pair<std::optional<int>, std::optional<int>> vparam = {std::nullopt, std::nullopt});
void draw_empty_border(WINDOW* win, int row, int col, int width, bool focused = false, std::pair<std::optional<int>, std::optional<int>> vparams = {std::nullopt, std::nullopt});