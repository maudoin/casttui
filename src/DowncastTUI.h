#pragma once


#include "TableWindow.h"
#include "DialogNew.h"
#include "Window.h"
#include "WindowLabel.h"
#include "DropDown.h"
#include "CloseDropDown.h"
#include "FilterButtons.h"
#include "TUIApp.h"

#include <string>

class Downcast
{
    static constexpr int FIRST_LINE_Y = 0;
    static constexpr int SECOND_LINE_Y = 1;
    static constexpr int MENU_ITEM_SPACE = 0;
    static const std::string viewingLabel;
    static const std::string statusFilterLabel;

    int _xMax, _yMax;
    WindowLabel _addWindow;
    DropDown _podcastMenuWindow;
    CloseDropDown _closeWindow;
    FilterButtons _filters;
    TableWindow _tableWindow;

public:
    Downcast(DropDown::Callbacks const& podcastCallbacks,
             TableWindow::Callbacks const& showsCallbacks,
             FilterButtons::Callback const& filterCallback);

    WINDOW* mainHandle();

    void drawBackground() const;

    void redrawAll() const;

    void resize();

    TUIApp::NextOp handleMouseEvent(MEVENT const& event);
};
