


#pragma once

#if defined(_WIN32)
#include <curses.h>
#else
#include <ncurses.h>
#endif
#include <functional>
#include <string>
#include <limits>
#include <optional>
#include <set>
#include <variant>

class TableWindow
{
    using NoIndex = std::monostate;
    using SingleIndex = int;
    using IndexSet = std::set<int>;
    struct IndexRange{int first, last;};
    using Selection = std::variant<NoIndex, SingleIndex, IndexSet, IndexRange>;
public:
    struct ColProp
    {
        static constexpr int AutoLeft = std::numeric_limits<int>::min();
        static constexpr int AutoRight = std::numeric_limits<int>::max();
        inline bool isAutoLeft()const{return size==AutoLeft;}
        inline bool isAutoRight()const{return size==AutoRight;}
        inline bool isAuto()const{return isAutoLeft() || isAutoRight();}
        static ColProp colRight(std::string const& title, int size = AutoRight)
        {
            return {size, title};
        }
        static ColProp colLeft(std::string const& title, int size = AutoLeft)
        {
            return {-size, title};
        }
        int size;
        std::string const title;
    };
    struct Callbacks
    {
        std::function<int()> const itemCount;
        std::function<std::vector<TableWindow::ColProp>()> const colSizesReq;
        std::function<std::string(int const col, int const line)> const stringAt;
    };
    TableWindow(int x, int y, int w, int h, Callbacks const& callbacks);
    ~TableWindow();
    void resize(int w, int h)const;
    void draw(bool redrawBorder = false, bool refresh = true)const;
    bool handleMouseEvent(MEVENT const&);
    std::optional<int> currentSelectedItemIndex() const;
    WINDOW * handle()const{return window;}

private:

    struct SingleColInfo
    {
        std::string const title;
        int const index, start, size;
        const char* format()const;
    };
    using ColInfo = std::vector<SingleColInfo>;

    int rangeEnd()const;
    ColInfo drawColumns()const;
    void drawScrollbar()const;
    int scrollbarX()const;
    bool isSelected(int itemIndex)const;

    WINDOW * const window;
    Callbacks _callbacks;
    int rangeStart = 0;
    Selection selection = NoIndex{};
    static constexpr int NO_COL_SORT = std::numeric_limits<int>::min();
    int currentColSort = NO_COL_SORT;
};
