#pragma once

#include "UiTable.h"

#include <functional>
#include <string>

class DowncastLogic;
class PodcastCols;
class MouseEvent;

class UiPodcastTable : public UiTable
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

    void render(bool focused);
    bool handleKey(int k);
    bool pickPodcast();

private:
    DowncastLogic& _logic;
    Actions _actions;
};
