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

void UiPodcastTable::render(int k, bool focused)
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

  for (TableRender::Row r : renderLoop(k, count, cols, focused))
  {
    for (TableRender::Row::Col c : r)
    {
      const std::wstring& title = titles[r.index()];
      bool isCursor   = (r.index() == cursor() && focused);
      bool isSelected = (r.index() >= 2 && _logic.isCurrentPodcast(r.index() - 2));

      int style;
      if (isCursor && isSelected)
      style = COLOR_PAIR(3);
      else if (isSelected)
      style = COLOR_PAIR(2);
      else if (isCursor)
      style = COLOR_PAIR(1);
      else
      style = A_NORMAL;

      if (c.draw({title, style}))
      {
        this->scrollTo(r.index());
        this->pickPodcast();
      }
    }
  }
}

std::optional<int> UiPodcastTable::getPodcastIndex()const
{
  return (cursor() >= 2 &&
  cursor() < _logic.podcastCount() + 2) ? std::make_optional(cursor() - 2) : std::nullopt;
}

bool UiPodcastTable::handleKey(int k)
{
  if (UiTable::handleKey(k))
  return true;

  // Only valid podcast rows (skip Add/All)
  if (auto index = getPodcastIndex())
  {
    auto const& p = _logic.podcast(*index);

    if (k == 'r' || k == 'R')
    {
      _logic.refreshPodcastAtIndex(*index);
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