#include "UiConfirm.h"

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

void UiConfirm::render()
{
    std::wstring cancel = L"Cancel";

    auto cell_cb = [&](int, int col) -> Cell {
        bool const isCursor = col==_field;
        int style = A_NORMAL;
        if (isCursor)
            style = COLOR_PAIR(1);
        if (col == 0)
        {
            return Cell{
                .text=_title,
                .style=style,
                .callback=_action
            };
        }
        return Cell{
            .text=cancel,
            .style=style,
            .callback=_cancel
        };
    };

    std::vector<HeaderColumn> cols{
        HeaderColumn{ .width = 0, .dynamic=true },
        HeaderColumn{ .width = static_cast<int>(cancel.size()) }
    };

    UiTable::render(1, cols, cell_cb, true);
}

bool UiConfirm::handleKey(int k)
{
    // Auto switching windows
    if (k == KEY_RIGHT)
    {
        _field = std::max(1, _field+1);
        return true;
    }
    else if (k == KEY_LEFT)
    {
        _field = std::min(0, _field-1);
        return true;
    }
    else if (k == 10 || k == 13) // ENTER commits edit
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
