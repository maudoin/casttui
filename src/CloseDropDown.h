#pragma once


#include "WindowLabel.h"
#include "TUIApp.h"

struct MouseEvent;

class CloseDropDown : public WindowLabel
{
public:
    CloseDropDown(int x, int y);

    TUIApp::NextOp mayHandleMouseEvent(MouseEvent const& event);
};
