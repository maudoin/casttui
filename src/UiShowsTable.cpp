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


UiShowsTable::UiShowsTable(DowncastLogic& logic)
: UiTable(UiTable::Mode::CURSOR)
, _logic(logic)
{}

namespace{
  SortDir toSortDir(std::optional<DowncastLogic::SortingOption> const& s)
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
}

void UiShowsTable::render(int k, bool focused, std::function<void()> const& winSelection)
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

  Columns cols = UiTable::renderHeader(k, {
    HeaderColumn{.width = HeaderColumn::FILL, .name = L"Title",   .sort = toSortDir(titleSort), .callback=makeCallback(&MediaViewCols::title, titleSort)},
    HeaderColumn{.width = 12, .name = L"Date",    .sort = toSortDir(dateSort),     .callback=makeCallback(&MediaViewCols::date, dateSort)},
    HeaderColumn{.width = 10, .name = L"Duration",.sort = toSortDir(durationSort), .callback=makeCallback(&MediaViewCols::duration, durationSort)},
  }, focused);

  auto cell_cb = [&](int row, int col, std::optional<MouseEvent> const& ev) -> Cell
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
    if (ev)
    {
      scrollTo(row);
      _logic.showSelection(row, true, false);
    }
    return Cell{text, style};
  };

  UiTable::render(k, _logic.showCount(), cols, cell_cb, focused, winSelection);
}

bool UiShowsTable::handleKey(int k)
{
  if (UiTable::handleKey(k))
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

#ifdef PDCURSES_WIN32
  if (k == PADMINUS)
#else
  if (k == '-')
#endif
  {
    _logic.selectShowRange(0, cursor(), true);
    return true;
  }

#ifdef PDCURSES_WIN32
  if (k == PADPLUS)
#else
  if (k == '+')
#endif
  {
    _logic.selectShowRange(cursor(), this->_logic.showCount()-1, true);
    return true;
  }

  // Name sort toggle
  if (k == 'n' || k == 'N')
  {
    auto const sort = _logic.getShowSorting(&MediaViewCols::title);
    _logic.setShowSorting(&MediaViewCols::title, DowncastLogic::cycle(sort));
  }
  // Time sort toggle
  if (k == 't' || k == 'T')
  {
    auto const sort = _logic.getShowSorting(&MediaViewCols::date);
    _logic.setShowSorting(&MediaViewCols::date, DowncastLogic::cycle(sort));
    return true;
  }

  // Length sort toggle
  if (k == 'l' || k == 'L')
  {
    auto const sort = _logic.getShowSorting(&MediaViewCols::duration);
    _logic.setShowSorting(&MediaViewCols::duration, DowncastLogic::cycle(sort));
    return true;
  }

  return false;
}
