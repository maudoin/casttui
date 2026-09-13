#pragma once

#include "UiTable.h"

#include <functional>
#include <string>

class DowncastLogic;
class PodcastCols;
class MouseEvent;

class UiPodcastTable
{
public:
    struct Actions
    {
        std::function<void()> winSelection;
        std::function<void()> add;
        std::function<void(PodcastCols const&)> edit;
        std::function<void(std::wstring const& title, std::function<void()> const& del)> del;
    };

    explicit UiPodcastTable(DowncastLogic& logic, Actions const& actions);
    ~UiPodcastTable();
    void render(bool focused);

    bool handleKey(int k);
    bool pickPodcast();

    void delWindow();
    void buildWindow(int nlines, int ncols, int begy, int begx);
    bool handleMouseEvent(MouseEvent const& ev);
    int cursor();

private:
    DowncastLogic& _logic;

    UiTable _table;
    WINDOW* _win = nullptr;
    Actions _actions;
};
