


#include "TableWindow.h"
#include "Window.h"

#include <limits>

TableWindow::TableWindow(int x, int y, int w, int h, Callbacks const& callbacks)
: window(newwin(h, w, y, x))
, _callbacks(callbacks)
{
    draw(true);
}

TableWindow:: ~TableWindow()
{
    delwin(window);
}

bool TableWindow::handleMouseEvent(MEVENT const& event)
{
    if (windowContainsMouseEvent(window, event))
    {
        int const count = _callbacks.itemCount();
        int const contentHeight =  windowContentHeight(window);
        int const minRangeStart = 0;
        int const maxRangeStart = count>contentHeight ? (count - 1 - contentHeight) : 0;
        int const scrollJump = contentHeight / 3;
        int const clickedIndex = rangeStart + event.y - getbegy(window) -1 ;
        if (event.bstate & BUTTON4_PRESSED)
        {
            rangeStart = std::max(minRangeStart, rangeStart-scrollJump);
        }
        else if (event.bstate & BUTTON5_PRESSED)
        {
            rangeStart = std::min(maxRangeStart, rangeStart+scrollJump);
        }
        else if ((event.bstate & BUTTON1_PRESSED)==BUTTON1_PRESSED)
        {
            if((event.x-getbegx(window)) == scrollbarX())
            {
                int localMouseY = event.y - getbegy(window);
                int const itemNumAtMouseY = (count * localMouseY) / contentHeight;
                rangeStart = std::max(minRangeStart, std::min(maxRangeStart, itemNumAtMouseY - contentHeight/2));
            }
            else
            {
                selection = clickedIndex;
            }
        }
        else if ((event.bstate & (BUTTON1_PRESSED|BUTTON_CTRL)) == (BUTTON1_PRESSED|BUTTON_CTRL))
        {
            std::visit([&](auto&& content)
            {
                using T = std::decay_t<decltype(content)>;
                if constexpr (std::is_same_v<T, NoIndex>)
                {
                    selection = clickedIndex;
                }
                else if constexpr (std::is_same_v<T, SingleIndex>)
                {
                    if(content == clickedIndex)
                    {
                        selection = NoIndex{};
                    }
                    else
                    {
                        selection = IndexSet{clickedIndex, content};
                    }
                }
                else if constexpr (std::is_same_v<T, IndexSet>)
                {
                    if(std::find(content.begin(), content.end(), clickedIndex)!=content.end())
                    {
                        content.erase(clickedIndex);
                    }
                    else
                    {
                        content.insert(clickedIndex);
                    }

                }
                else if constexpr (std::is_same_v<T, IndexRange>)
                {
                    selection = IndexSet{};
                    auto& indexSet = std::get<IndexSet>(selection);
                    for(int i=content.first;i<=content.last;++i)
                    {
                        indexSet.insert(i);
                    }
                    indexSet.insert(clickedIndex);
                }
                else
                {
                    static_assert(std::is_same_v<T, T>, "non-exhaustive visitor!");
                }
            }, selection);
        }
        else if (event.bstate & (BUTTON1_PRESSED|BUTTON_SHIFT))
        {
            std::visit([&](auto&& content)
            {
                using T = std::decay_t<decltype(content)>;
                if constexpr (std::is_same_v<T, NoIndex>)
                {
                    selection = clickedIndex;
                }
                else if constexpr (std::is_same_v<T, SingleIndex>)
                {
                    if(content == clickedIndex)
                    {
                        selection = NoIndex{};
                    }
                    else
                    {
                        selection = IndexRange{std::min(clickedIndex, content), std::max(clickedIndex, content)};
                    }
                }
                else if constexpr (std::is_same_v<T, IndexSet>)
                {
                    if(content.empty())
                    {
                        selection = clickedIndex;
                    }
                    else if(clickedIndex>=*content.crend())
                    {
                        for(int i=*content.crend();i<=clickedIndex;++i)
                        {
                            content.insert(i);
                        }
                    }
                    else if(clickedIndex<*content.crend())
                    {
                        for(int i=clickedIndex;i<=*content.crend();++i)
                        {
                            content.insert(i);
                        }
                    }
                }
                else if constexpr (std::is_same_v<T, IndexRange>)
                {
                    selection = IndexRange{std::min(clickedIndex, content.first), std::max(clickedIndex, content.last)};
                }
                else
                {
                    static_assert(std::is_same_v<T, T>, "non-exhaustive visitor!");
                }
            }, selection);
        }
        draw(false, false);
        return true;
    }
    return false;
}
std::optional<int> TableWindow::currentSelectedItemIndex() const
{
    if(const SingleIndex* selectedItem =std::get_if<SingleIndex>(&selection))
    {
        if(*selectedItem>=0 && *selectedItem<_callbacks.itemCount())
        {
            return {*selectedItem};
        }
    }
    return std::nullopt;
}

bool TableWindow::isSelected(int itemIndex)const
{
    return
        std::visit([&itemIndex](auto&& content)
        {
            using T = std::decay_t<decltype(content)>;
            if constexpr (std::is_same_v<T, NoIndex>)
                return false;
            else if constexpr (std::is_same_v<T, SingleIndex>)
                return itemIndex == content;
            else if constexpr (std::is_same_v<T, IndexSet>)
                return std::find(content.begin(), content.end(), itemIndex)!=content.end();
            else if constexpr (std::is_same_v<T, IndexRange>)
                return itemIndex >= content.first && itemIndex <= content.last;
            else
                static_assert(std::is_same_v<T, T>, "non-exhaustive visitor!");
        }, selection);

}
int TableWindow::rangeEnd()const
{
    return rangeStart + std::min(_callbacks.itemCount(), windowContentHeight(window)+1);
}


const char* TableWindow::SingleColInfo::format()const
{
    //if size is negative it formats left, see printf documentation
    return (std::string("%")+std::to_string(size)+"s").c_str();
}
TableWindow::ColInfo TableWindow::drawColumns()const
{
    std::vector<ColProp> colSizes = _callbacks.colSizesReq();
    int total = windowContentWidth(window)-3;
    int toFill = total;
    int autoWidthCount = 0;

    for(ColProp const& c:colSizes)
    {
        if(c.isAuto())
        {
            ++autoWidthCount;
        }
        else
        {
            toFill-=std::abs(c.size);
        }
    }
    int autoWidth = toFill/autoWidthCount;
    bool first = true;
    for(ColProp& c:colSizes)
    {
        auto computed = [&]{first=false;return first ? toFill - autoWidth*(autoWidthCount-1) : autoWidth;};
        if(c.isAutoLeft())
        {
            c.size = -computed();
        }
        else if(c.isAutoRight())
        {
            c.size = computed();
        }
    }

    auto isSortedCol = [&](int col){ return !colSizes[col].title.empty() &&
            currentColSort!=NO_COL_SORT &&
            std::abs(currentColSort)==col; };
    static constexpr int BORDER_SIZE = 1;
    ColInfo starts;
    starts.reserve(colSizes.size());
    if(!colSizes.empty())
    {
        starts.push_back({colSizes[0].title, 0, BORDER_SIZE, colSizes[0].size});

        mvwprintw(window, 0, 1, "%s", colSizes[0].title.c_str());
        if(isSortedCol(0))
        {
            waddch(window, currentColSort>0?ACS_UARROW:ACS_DARROW);
        }
    }
    for(int x=0, colIndex=0;colIndex<colSizes.size()-1;++colIndex)
    {
        x += BORDER_SIZE+std::abs(colSizes[colIndex].size);

        mvwprintw(window, 0, x+1, "%s", colSizes[colIndex+1].title.c_str());
        if(isSortedCol(colIndex+1))
        {
            waddch(window, currentColSort>0?ACS_UARROW:ACS_DARROW);
        }

        mvwaddch(window, 0,x,ACS_TTEE);
        mvwaddch(window, windowHeight(window)+1,x,ACS_BTEE);
        for (int y = BORDER_SIZE;y < getmaxy(window)-BORDER_SIZE ;++y)
        {
            mvwaddch(window, y, x, ACS_VLINE);
        }
        starts.push_back({colSizes[colIndex+1].title, colIndex+1, x+BORDER_SIZE, colSizes[colIndex+1].size});
    }
    return starts;
}

void TableWindow::resize(int xMax, int yMax)const
{
    wresize(window, yMax - 2, xMax - 2);
    draw(true);
}

int TableWindow::scrollbarX()const
{
    return 1+windowContentWidth(window);
}

void TableWindow::drawScrollbar()const
{

    int const itemCount = _callbacks.itemCount();
    int h = windowContentHeight(window)+1;
    if(itemCount<=h)
    {
        return;
    }
    int x = getbegx(window)-1+scrollbarX();

    int yStart = 1+(h*rangeStart)/itemCount;
    int yEnd = 1+(h*rangeEnd())/itemCount;

    mvwvline(window, 1, x, ACS_CKBOARD, yStart);
    wattron(window, A_REVERSE);
    mvwvline(window, yStart, x, ACS_CKBOARD, yEnd - yStart);
    wattroff(window, A_REVERSE);
    mvwvline(window, yEnd, x, ACS_CKBOARD, h+1 - yEnd);
}

void TableWindow::draw(bool const redrawBorder, bool const refresh)const
{
    init_color(COLOR_BLUE, 100, 100, 100);
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLUE);

    if(redrawBorder)
    {
        box(window, 0, 0);
    }
    ColInfo const columns = drawColumns();
    int const rangeEnd = this->rangeEnd();
    int y = 1;
    for (int line=rangeStart;line<rangeEnd;++line, ++y)
    {
        curs_set(0);
        bool const odd = line%2!=0;
        short const col = odd?1:2;
        bool const isSel = isSelected(line);
        wattrset(window, COLOR_PAIR(col)|(A_REVERSE*isSel));
        for(SingleColInfo const& c:columns)
        {
            mvwprintw(window, y, c.start, c.format(), _callbacks.stringAt(c.index, line).c_str());
        }
    }
    //draw empty lines
    int const tableEnd = rangeStart + windowContentHeight(window);
    for (int line=rangeEnd;line<tableEnd;++line, ++y)
    {
        curs_set(0);
        bool const odd = line%2!=0;
        short const col = odd?1:2;
        bool const isSel = isSelected(line);
        wattrset(window, COLOR_PAIR(col)|(A_REVERSE*isSel));
        for(SingleColInfo const& c:columns)
        {
            mvwprintw(window, y, c.start, c.format(), "");
        }
    }

    wattrset(window, COLOR_PAIR(0));

    drawScrollbar();

    if(refresh)
    {
        wrefresh(window);
    }
}
