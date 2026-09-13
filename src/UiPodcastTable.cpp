#include "UiPodcastTable.h"

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

UiPodcastTable::UiPodcastTable(DowncastLogic& logic, Actions const& actions)
: UiTable(UiTable::Mode::CURSOR, actions.winSelection)
, _logic(logic)
, _actions(actions)
{
}

void UiPodcastTable::render(bool focused)
{
    int count = _logic.podcastCount() + 2;

    std::vector<std::wstring> titles;
    titles.reserve(count);
    titles.push_back(L"Add podcast...");
    titles.push_back(L"All");
    for (int i = 0; i < _logic.podcastCount(); ++i)
    {
        titles.push_back(to_wstring(_logic.podcastTitle(i)));
    }

    std::vector<HeaderColumn> cols{
        HeaderColumn{.width = -1, .name = std::nullopt, .sort = SortDir::NONE, .dynamic = true}
    };

    auto cell_cb = [&](int row, int /*col*/) -> Cell
    {
        const std::wstring& title = titles[row];
        bool isCursor   = (row == cursor() && focused);
        bool isSelected = (row >= 2 && _logic.isCurrentPodcast(row - 2));

        int style;
        if (isCursor && isSelected)
            style = COLOR_PAIR(3);
        else if (isSelected)
            style = COLOR_PAIR(2);
        else if (isCursor)
            style = COLOR_PAIR(1);
        else
            style = A_NORMAL;

        return Cell{title, style, [this, row]{
            this->scrollTo(row);
            this->pickPodcast();
        }};
    };

    UiTable::render(count, cols, cell_cb, focused);
}

bool UiPodcastTable::handleKey(int k)
{
    if (UiTable::handleKeyCh(k))
        return true;

    // Only valid podcast rows (skip Add/All)
    if (cursor() >= 2 &&
        cursor() < _logic.podcastCount() + 2)
    {
        auto const& p = _logic.podcast(cursor() - 2);

        if (k == 'r' || k == 'R')
        {
            _logic.refreshPodcastAtIndex(cursor() - 2);
            return true;
        }
        if (k == 'e' || k == 'E')
        {
            _actions.edit(p);
            return true;
        }
        if (k == 'd' || k == 'D')
        {
            int delete_id = p.id;
            _actions.del(L"Delete '" + to_wstring(p.title) + L"'?", [this, delete_id]{
                this->_logic.deletePodcast(delete_id);
                this->scrollVertical(-1);
            });
            return true;
        }
    }

    // ENTER behavior
    if (k == 10 || k == 13)
    {
        if (pickPodcast())
        {
            return true;
        }
    }

    return false;
}

bool UiPodcastTable::pickPodcast()
{
    if (cursor() == 0)
    {
        _actions.add();
        return true;
    }
    else if (cursor() == 1)
    {
        _logic.setCurrentPodcastRowIndex(std::optional<std::optional<int>>{std::nullopt}, std::nullopt);
        //TODO firstVisibleDataRow() = 0;
        //TODO shows_table.cursor() = 0;
        return true;
    }
    else
    {
        _logic.setCurrentPodcastRowIndex(cursor() - 2, std::nullopt);
        //TODO firstVisibleDataRow() = 0;
        //TODO shows_table.cursor() = 0;
        return true;
    }
    return false;
}