#pragma once

#include "UiTable.h"

#include "UiColors.h"
#include "DowncastLogic.h"

#include <array>
#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <ctime>
#include <ranges>

// Class for cleanness but instantiated once, no need to have a separate compilation unit, compiled once anyway
class UiStatusTable : public UiTable
{
public:
  UiStatusTable(DowncastLogic& logic, std::function<void()> const& exit)
  : UiTable(UiTable::Mode::CURSOR)
  , _logic(logic)
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

  void render(UiInput const& input, bool focused, std::function<void()> const& winSelection)
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

    auto cellCallback = [&](int row, int col, std::optional<UiInput::MouseEvent> const& ev) -> Cell
    {
      if (col == 0)
      {
        // Title
        return Cell{title};
      }
      if (col == cols.size()-1)
      {
        // Quit
        if (ev){_exit();}
        return Cell{quitLabel};
      }
      int statusIndex = col-1;
      if (statusIndex >= _labels.size())
      {
        // Spacer
        return Cell{};
      }
      auto const& [label, status] = _labels[statusIndex];
      bool isSelected = _logic.isStatusActive(status);
      bool isCursor   = (statusIndex == _cursorPosition && focused);

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
      return Cell{label, UiColors::getStyle(isCursor, isSelected)};
    };

    auto colRanges = UiTable::renderHeader(input, cols, UiColors::focusStyle(focused));
    UiTable::render(input, 1, colRanges, cellCallback, UiColors::focusStyle(focused), winSelection);
  }

  bool handleKey(UiInput const& input)
  {
    auto move_status_cursor = [&](int delta)
    {
      _cursorPosition = std::max(
        0,
        std::min(static_cast<int>(_labels.size()) - 1,
        _cursorPosition + delta)
      );
    };

    if (input.keyLeft())
    {
      move_status_cursor(-1);
      return true;
    }
    if (input.keyRight())
    {
      move_status_cursor(1);
      return true;
    }

    if (input.keyEnterReturn())
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

  bool leftMostStatus() const{return _cursorPosition==0;}
private:
  struct StatusLabel
  {
    std::wstring label;
    std::optional<DowncastLogic::MediaStatus> status;
  };
  DowncastLogic& _logic;
  int _cursorPosition;
  std::function<void()> _exit;
  std::vector<StatusLabel> _labels;

};
