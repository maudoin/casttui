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

class DowncastLogic;

// Class for cleanness but instantiated once, no need to have a separate compilation unit, compiled once anyway
class UiShowsTable : public UiTable
{
public:
  UiShowsTable(DowncastLogic& logic)
  : UiTable(UiTable::Mode::CURSOR)
  , _logic(logic)
  {}


  void render(UiInput const& input, bool focused, std::function<void()> const& winSelection)
  {
    int h = getHeight();
    int inner_h = h - 2;
    int data_h  = inner_h - 2;

    auto const& shows = _logic.showsInRankRange(firstVisibleDataRow(), data_h);

    // We don't have direct access to current sort column; keep arrows neutral or infer externally.
    auto const titleSort = _logic.getShowSorting(&MediaViewCols::title);
    auto const dateSort = _logic.getShowSorting(&MediaViewCols::date);
    auto const durationSort = _logic.getShowSorting(&MediaViewCols::duration);
    auto makeCallback = [&](auto const& col, auto const& sort)->std::function<void()>{
      return [=, this]{
        this->_logic.setShowSorting(col, DowncastLogic::cycle(sort));
      };
    };

    Columns cols = UiTable::renderHeader(input, {
      HeaderColumn{.width = HeaderColumn::FILL, .name = L"Title",   .sort = toSortDir(titleSort), .callback=makeCallback(&MediaViewCols::title, titleSort)},
      HeaderColumn{.width = 12, .name = L"Date",    .sort = toSortDir(dateSort),     .callback=makeCallback(&MediaViewCols::date, dateSort)},
      HeaderColumn{.width = 10, .name = L"Duration",.sort = toSortDir(durationSort), .callback=makeCallback(&MediaViewCols::duration, durationSort)},
    }, UiColors::focusStyle(focused));

    auto cellCallback = [&](int row, int col, std::optional<UiInput::MouseEvent> const& ev) -> Cell
    {
      int idx = row - firstVisibleDataRow();
      if (idx < 0 || idx >= static_cast<int>(shows.size()))
      return Cell{};

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

      if (ev)
      {
        scrollTo(row);
        _logic.showSelection(row, true, false);
      }
      return Cell{text, UiColors::getStyle(isCursor, isSelected)};
    };

    UiTable::render(input, _logic.showCount(), cols, cellCallback, UiColors::focusStyle(focused), winSelection);
  }


  bool handleKey(UiInput const& input)
  {
    if (UiTable::handleKey(input))
    return true;

    using MediaStatus = DowncastLogic::MediaStatus;


    if (input.key == 'u' || input.key == 'U')
    {
      if (_logic.isStatusActive(MediaStatus::New))
      _logic.refreshCurrentPodcast();
      return true;
    }

    if (_logic.anySelection())
    {
      if (input.key == 'q' || input.key == 'Q')
      {
        _logic.setSelectedShowsStatus(Status::QUEUED);
        return true;
      }
      if (input.key == 's' || input.key == 'S')
      {
        _logic.setSelectedShowsStatus(Status::SKIPPED);
        return true;
      }
      if (input.key == 'n' || input.key == 'N')
      {
        _logic.setSelectedShowsStatus(Status::NEW);
        return true;
      }
    }

    if (input.key == 'd' || input.key == 'D')
    {
      if (_logic.isStatusActive(MediaStatus::Queued))
      _logic.startDownload();
      return true;
    }

    if (input.key == 10 || input.key == 13) // ENTER
    {
      _logic.showSelection(cursor(), false, false);
      return true;
    }

    if (input.key == ' ')
    {
      _logic.showSelection(cursor(), true, false);
      return true;
    }

  #ifdef PDCURSES_WIN32
    if (input.key == PADMINUS)
  #else
    if (input.key == '-')
  #endif
    {
      _logic.selectShowRange(0, cursor(), true);
      return true;
    }

  #ifdef PDCURSES_WIN32
    if (input.key == PADPLUS)
  #else
    if (input.key == '+')
  #endif
    {
      _logic.selectShowRange(cursor(), this->_logic.showCount()-1, true);
      return true;
    }

    // Name sort toggle
    if (input.key == 'n' || input.key == 'N')
    {
      auto const sort = _logic.getShowSorting(&MediaViewCols::title);
      _logic.setShowSorting(&MediaViewCols::title, DowncastLogic::cycle(sort));
    }
    // Time sort toggle
    if (input.key == 't' || input.key == 'T')
    {
      auto const sort = _logic.getShowSorting(&MediaViewCols::date);
      _logic.setShowSorting(&MediaViewCols::date, DowncastLogic::cycle(sort));
      return true;
    }

    // Length sort toggle
    if (input.key == 'l' || input.key == 'L')
    {
      auto const sort = _logic.getShowSorting(&MediaViewCols::duration);
      _logic.setShowSorting(&MediaViewCols::duration, DowncastLogic::cycle(sort));
      return true;
    }

    return false;
  }


private:
  static inline SortDir toSortDir(std::optional<DowncastLogic::SortingOption> const& s)
  {
    if (s)
    {
      return (*s == DowncastLogic::SortingOption::ASCENDING) ? SortDir::DOWN : SortDir::UP;
    }
    else
    {
      return SortDir::NONE;
    }
  }


  DowncastLogic& _logic;
};
