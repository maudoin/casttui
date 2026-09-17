#include "UiStatusTable.h"
#include "DowncastLogic.h"

#include <array>
#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <ctime>
#include <ranges>


UiStatusTable::UiStatusTable(DowncastLogic& _logic, std::function<void()> const& winSelection, std::function<void()> const& exit)
: UiTable(UiTable::Mode::CURSOR, winSelection)
, _logic(_logic)
, _cursorPosition(0)
, _exit(exit)
{
  using MediaStatus = DowncastLogic::MediaStatus;
  _labels = {
    {L"New",    MediaStatus::New},
    {L"Queue",  MediaStatus::Queued},
    {L"Skipped",MediaStatus::Skipped},
    {L"Done",   MediaStatus::Done},
    {L"All",    std::nullopt}
  };
}

void UiStatusTable::render(int k, bool focused)
{
  std::wstring title = L"Status: ";
  std::wstring quitLabel = L" X ";
  std::vector<HeaderColumn> cols;
  cols.reserve(_labels.size()+3);//+title+spacer+quit
  // title
  cols.push_back(HeaderColumn{.width = static_cast<int>(title.size())});
  // statuses
  for (int i = 0; i < static_cast<int>(_labels.size()); ++i)
  {
    auto const& [label, status] = _labels[i];
    cols.push_back(HeaderColumn{.width = static_cast<int>(label.size())});
  };
  // spacer
  cols.push_back(HeaderColumn{.width = HeaderColumn::FILL});
  // quit
  cols.push_back(HeaderColumn{.width =  static_cast<int>(quitLabel.size())});

  auto cell_cb = [&](int row, int col, std::optional<MouseEvent> const& ev) -> Cell
  {
    if (col == 0)
    {
      // Title
      return Cell{title, A_NORMAL};
    }
    if (col == cols.size()-1)
    {
      // Quit
      if (ev){_exit();}
      return Cell{quitLabel, A_NORMAL};
    }
    int statusIndex = col-1;
    if (statusIndex >= _labels.size())
    {
      // Spacer
      return Cell{L"", A_NORMAL};
    }
    auto const& [label, status] = _labels[statusIndex];
    bool isSelected = _logic.isStatusActive(status);
    bool isCursor   = (statusIndex == _cursorPosition && focused);

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
      this->_cursorPosition = statusIndex;
      auto const& [_, status] = this->_labels[statusIndex];
      this->_logic.setCurrentPodcastRowIndex(
        std::nullopt,
        status,
        DowncastLogic::SetPodcastOption::FORCE_REFRESH
      );
    }
    return Cell{label, style};
  };

  UiTable::render(k, 1, cols, cell_cb, focused);
}

bool UiStatusTable::handleKey(int k)
{
  auto move_status_cursor = [&](int delta)
  {
    _cursorPosition = std::max(
      0,
      std::min(static_cast<int>(_labels.size()) - 1,
      _cursorPosition + delta)
    );
  };

  if (k == KEY_LEFT)
  {
    move_status_cursor(-1);
    return true;
  }
  if (k == KEY_RIGHT)
  {
    move_status_cursor(1);
    return true;
  }

  // ENTER selects status
  if (k == 10 || k == 13)
  {
    auto const& [_, status] = _labels[_cursorPosition];
    _logic.setCurrentPodcastRowIndex(
      std::nullopt,
      status,
      DowncastLogic::SetPodcastOption::FORCE_REFRESH
    );
    return true;
  }

  return false;
}