#pragma once

#include "UiTable.h"

#include "UiColors.h"
#include "DowncastLogic.h"
#include "HtmlToText.h"

#include <array>
#include <chrono>
#include <ctime>
#include <functional>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

// Class for cleaness but instantiated once, no need to have a separate compilation unit, compiled once anyway
class UiPodcastTable : public UiTable
{
public:
  struct Actions
  {
    std::function<void()> add;
    std::function<void(PodcastCols const&)> edit;
    std::function<void(std::wstring const& title, std::function<void()> const& del)> del;
  };


  explicit UiPodcastTable(DowncastLogic& logic, Actions const& actions)
  : UiTable(UiTable::Mode::CURSOR)
  , _logic(logic)
  , _actions(actions)
  {}


  std::optional<int> getPodcastIndex()const
  {
    return (cursor() >= 2 &&
    cursor() < _logic.podcastCount() + 2) ? std::make_optional(cursor() - 2) : std::nullopt;
  }


  void render(UiInput const& input, bool focused, std::function<void()> const& winSelection)
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

    Columns cols = UiTable::renderHeader(input, {
      HeaderColumn{.width = HeaderColumn::FILL, .name = std::nullopt, .sort = SortDir::NONE}
    },UiColors::focusStyle(focused));

    auto cellCallback = [&](int row, int /*col*/, std::optional<MouseEvent> const& ev) -> Cell
    {
      const std::wstring& title = titles[row];
      bool isCursor   = focused && row == cursor();
      bool isSelected = row >= 2 && _logic.isCurrentPodcast(row - 2);

      if (ev)
      {
        this->scrollTo(row);
        this->pickPodcast();
      }
      return Cell{title, UiColors::getStyle(isCursor, isSelected)};
    };

    UiTable::render(input, count, cols, cellCallback, UiColors::focusStyle(focused), winSelection);
  }


  bool handleKey(UiInput const& input)
  {
    if (UiTable::handleKey(input))
    return true;

    // Only valid podcast rows (skip Add/All)
    if (auto index = getPodcastIndex())
    {
      auto const& p = _logic.podcast(*index);

      if (input.key == 'r' || input.key == 'R')
      {
        _logic.refreshPodcastAtIndex(*index);
        return true;
      }
      if (input.key == 'e' || input.key == 'E')
      {
        _actions.edit(p);
        return true;
      }
      if (input.key == 'd' || input.key == 'D')
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
    if (input.key == 10 || input.key == 13)
    {
      if (pickPodcast())
      {
        return true;
      }
    }

    return false;
  }


  bool pickPodcast()
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

private:
  DowncastLogic& _logic;
  Actions _actions;
};
