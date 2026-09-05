#pragma once


#include "WindowLabel.h"
#include "TUIApp.h"

class CloseDropDown : public WindowLabel
{
public:
    CloseDropDown(int x, int y);

    TUIApp::NextOp mayHandleMouseEvent(MEVENT event);
};
