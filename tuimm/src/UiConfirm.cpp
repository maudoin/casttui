#include "UiConfirm.h"
#include "UiColors.h"

#include <array>
#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <ctime>
#include <ranges>


UiConfirm::UiConfirm(std::function<void()> const& cancel)
: UiTable(UiTable::Mode::CURSOR, []{})
, _cancel(cancel)
{}

int UiConfirm::preferredWindowHeight()
{
  return 5; // 2 text rows + 3 borders
}

void UiConfirm::set(std::wstring const& actionName, std::function<void()> const& action, std::wstring const& title)
{
  _field = 1;//"Cancel"
  _action = action;
  _actionName = actionName;
  _title = title;
}

void UiConfirm::cancel()
{
  _cancel();
  set(L"", []{});
}

void UiConfirm::render(UiInput const& input)
{
  std::wstring cancel = L"Cancel";

  auto cellCallback = [&](UiTable::CellRenderStr& str){
    bool const isCursor = str.col==_field;
    str << UiColors::highlightStyle(isCursor);
    if (str.col == 0)
    {
      if (str.ev) _action();
      str <<_title;
    }
    else
    {
      if (str.ev) _cancel();
      str <<cancel;
    }
  };

  Columns titleCols = UiTable::renderHeader(input, std::array{
    HeaderColumn{ .width = HeaderColumn::FILL, .name=_actionName },
    HeaderColumn{ .width = HeaderColumn::FIT_LABEL, .name=L" X ", .callback=_cancel}
  }, UiColors::focusedStyle());

  Columns cols = UiTable::renderHeader(input, std::array{
    HeaderColumn{ .width = HeaderColumn::FILL },
    HeaderColumn{ .width = static_cast<int>(cancel.size()) }
  }, UiColors::focusedStyle(), titleCols);

  UiTable::render(input, 1, cols, cellCallback, UiColors::focusedStyle());
}

bool UiConfirm::handleKey(UiInput const& input)
{
  // Auto switching windows
  if (input.keyRight())
  {
    _field = std::max(1, _field+1);
    return true;
  }
  else if (input.keyLeft())
  {
    _field = std::min(0, _field-1);
    return true;
  }
  else if (input.keyEnterReturn())
  {
    if (_field == 1)
    {
      cancel();
      return true;
    }
    else if (_field == 0)
    {
      _action();
      return true;
    }

  }
  return false;
}
