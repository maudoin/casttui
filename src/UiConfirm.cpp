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

void UiConfirm::set(std::wstring const& title, std::function<void()> const& action)
{
  _field = 1;//"Cancel"
  _action = action;
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

  auto cellCallback = [&](int, int col, std::optional<UiInput::MouseEvent> const& ev) -> Cell {
    bool const isCursor = col==_field;
    int style = UiColors::highlightStyle(isCursor);
    if (col == 0)
    {
      if (ev) _action();
      return Cell{
        .text=_title,
        .style=style
      };
    }
    if (ev) _cancel();
    return Cell{
      .text=cancel,
      .style=style
    };
  };

  Columns cols = UiTable::renderHeader(input, {
    HeaderColumn{ .width = HeaderColumn::FILL },
    HeaderColumn{ .width = static_cast<int>(cancel.size()) }
  }, UiColors::focusedStyle());

  UiTable::render(input, 1, cols, cellCallback, UiColors::focusedStyle());
}

bool UiConfirm::handleKey(UiInput const& input)
{
  // Auto switching windows
  if (input.key == KEY_RIGHT)
  {
    _field = std::max(1, _field+1);
    return true;
  }
  else if (input.key == KEY_LEFT)
  {
    _field = std::min(0, _field-1);
    return true;
  }
  else if (input.key == 10 || input.key == 13) // ENTER commits edit
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
