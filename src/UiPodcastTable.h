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
    Columns cols = UiTable::renderHeader(input, std::array{
      HeaderColumn{.width = HeaderColumn::FILL, .name = std::nullopt, .sort = SortDir::NONE}
    },UiColors::focusStyle(focused));

    auto cellCallback = [&](UiTable::CellRenderStr& str)
    {
      if (str.ev)
      {
        this->scrollTo(str.row);
        this->pickPodcast();
      }

      bool isCursor   = focused && str.row == cursor();
      if (str.row == 0)
      {
        str <<  UiColors::highlightStyle(isCursor) << L"Add podcast...";
        return;
      }
      if (str.row == 1)
      {
        str <<  UiColors::getStyle(isCursor, _logic.noPodcastFilter()) << L"All";
        return;
      }
      int const i = str.row - 2;
      str << UiColors::getStyle(isCursor, _logic.isCurrentPodcast(i)) << to_wstring(_logic.podcastTitle(i));
    };

    UiTable::render(input, _logic.podcastCount() + 2, cols, cellCallback, UiColors::focusStyle(focused), winSelection);
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
        _actions.del(to_wstring(p.title), [this, delete_id]{
          this->_logic.deletePodcast(delete_id);
          this->scrollVertical(-1);
        });
        return true;
      }
    }

    if (input.keyEnterReturn())
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
      std::optional<std::optional<int>> newPodcastNumber;
      newPodcastNumber.emplace();
      newPodcastNumber->reset();
      _logic.setCurrentPodcastRowIndex(newPodcastNumber, std::nullopt);
      return true;
    }
    else
    {
      _logic.setCurrentPodcastRowIndex(cursor() - 2, std::nullopt);
      return true;
    }
    return false;
  }

private:
  DowncastLogic& _logic;
  Actions _actions;
};
