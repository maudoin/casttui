#pragma once

#include "TableWindow.h"
#include "TUIApp.h"
#include "WindowLabel.h"

#include <functional>
#include <vector>


class DropDown : public WindowLabel
{
public:
    struct Callbacks
    {
        std::function<int()> const itemCount;
        std::function<std::vector<TableWindow::ColProp>()> const colSizesReq;
        std::function<std::string(int const line)> const stringAt;
        std::function<void(int)> const itemChanged;
    };

    DropDown(int x, int y, Callbacks const& callbacks);

    TUIApp::NextOp mayHandleMouseEvent(MEVENT event);
private:
    int _xMax, _yMax;
    Callbacks _callbacks;
};
