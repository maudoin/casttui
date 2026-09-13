#include "UiShowsTable.h"

#if defined(_WIN32)
#include <curses.h>
#else
#include <ncursesw/curses.h>
#endif

#include "UiTable.h"
#include "DowncastLogic.h"
#include "HtmlToText.h"

#include <array>
#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <ctime>
#include <ranges>


UiShowsTable::UiShowsTable(DowncastLogic& logic, std::function<void()> const& winSelection)
: UiTable(UiTable::Mode::CURSOR,winSelection)
, _logic(logic)
{}

void UiShowsTable::render(bool focused)
{
    int h = getHeight();
    int inner_h = h - 2;
    int data_h  = inner_h - 2;

    auto const& shows = _logic.showsInRankRange(firstVisibleDataRow(), data_h);

    // We don't have direct access to current sort column; keep arrows neutral or infer externally.
    SortDir sort_direction = SortDir::NONE;

    std::vector<HeaderColumn> cols{
        HeaderColumn{.width = -1, .name = std::make_optional<std::wstring>(L"Title"),   .sort = SortDir::NONE, .dynamic = true},
        HeaderColumn{.width = 12, .name = std::make_optional<std::wstring>(L"Date"),    .sort = sort_direction, .dynamic = false},
        HeaderColumn{.width = 10, .name = std::make_optional<std::wstring>(L"Duration"),.sort = sort_direction, .dynamic = false},
    };

    auto cell_cb = [&](int row, int col) -> Cell
    {
        int idx = row - firstVisibleDataRow();
        if (idx < 0 || idx >= static_cast<int>(shows.size()))
            return Cell{L"", A_NORMAL};

        auto const& s = shows[idx];

        std::wstring text;
        if (col == 0)
            text = to_wstring(s.title);
        else if (col == 1)
            text = to_wstring(s.dateStr());
        else
            text = to_wstring(s.durationStr());

        bool isSelected = (row >= 2 && _logic.isShowRankSelected(row));
        bool isCursor   = (row == cursor() && focused);

        int style;
        if (isCursor && isSelected)
            style = COLOR_PAIR(3);
        else if (isSelected)
            style = COLOR_PAIR(2);
        else if (isCursor)
            style = COLOR_PAIR(1);
        else
            style = A_NORMAL;

        return Cell{text, style, [this, row]{
            this->scrollTo(row);
            this->_logic.showSelection(row, true, false);
        }};
    };

    UiTable::render(_logic.showCount(), cols, cell_cb, focused);
}

bool UiShowsTable::handleKey(int k)
{
    if (UiTable::handleKeyCh(k))
        return true;

    using MediaStatus = DowncastLogic::MediaStatus;


    if (k == 'u' || k == 'U')
    {
        if (_logic.isStatusActive(MediaStatus::New))
            _logic.refreshCurrentPodcast();
        return true;
    }

    if (_logic.anySelection())
    {
        if (k == 'q' || k == 'Q')
        {
            _logic.setSelectedShowsStatus(Status::QUEUED);
            return true;
        }
        if (k == 's' || k == 'S')
        {
            _logic.setSelectedShowsStatus(Status::SKIPPED);
            return true;
        }
        if (k == 'n' || k == 'N')
        {
            _logic.setSelectedShowsStatus(Status::NEW);
            return true;
        }
    }

    if (k == 'd' || k == 'D')
    {
        if (_logic.isStatusActive(MediaStatus::Queued))
            _logic.startDownload();
        return true;
    }

    if (k == 10 || k == 13) // ENTER
    {
        _logic.showSelection(cursor(), false, false);
        return true;
    }

    if (k == ' ')
    {
        _logic.showSelection(cursor(), true, false);
        return true;
    }

    // Time sort toggle
    if (k == 't' || k == 'T')
    {
        _logic.setShowSorting(&MediaViewCols::date,
                            DowncastLogic::SortingOption::ASCENDING);
        return true;
    }

    // Duration sort toggle
    if (k == 'l' || k == 'L')
    {
        _logic.setShowSorting(&MediaViewCols::duration,
                            DowncastLogic::SortingOption::ASCENDING);
        return true;
    }

    return false;
}
