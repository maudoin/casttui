#include "UiPodcastSetup.h"

#include "DowncastLogic.h"
#include "HtmlToText.h"

#include <array>
#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <ctime>
#include <ranges>

UiPodcastSetup::UiPodcastSetup(DowncastLogic& _logic, std::function<void()> const& winSelection)
: UiTable(UiTable::Mode::CURSOR, winSelection)
, _logic(_logic)
{}

void UiPodcastSetup::setAdd()
{
    modal_edit_id.reset();
    _mode = Mode::AddPodcast;
}

void UiPodcastSetup::setEdit(PodcastCols const&p)
{
    modal_url = p.link;
    modal_title = p.title;
    modal_target = p.target;
    modal_pattern = p.pattern;
    modal_preview_description = p.summary;
    modal_field = 0;
    modal_edit_id = p.id;
    _mode = Mode::EditPodcast;
}

void UiPodcastSetup::render()
{
    int h, w;
    getmaxyx(stdscr, h, w);
    int mh = std::min(h - 4, 20);
    int mw = std::min(w - 4, 70);
    int y  = (h - mh) / 2;
    int x  = (w - mw) / 2;

    WINDOW* win = newwin(mh, mw, y, x);
    werase(win);

    _win = (win);

    int inner_w = mw - 2;
    std::vector<HeaderColumn> cols{
        HeaderColumn{.width = inner_w, .name = std::nullopt, .sort = SortDir::NONE, .dynamic = false}
    };

    draw_top_border_header(win, 0, 0, cols, true);

    std::wstring mode_str = (_mode == Mode::AddPodcast ? L"ADD" : L"EDIT");
    std::vector<std::wstring> header_line{mode_str};
    draw_row_assembled_cols(win, 1, 0,
                            {{mode_str + std::wstring(inner_w - mode_str.size(), L' ')}},
                            cols, *this, true);

    draw_mid_border_header(win, 2, 0, cols, true);

    int row = 3;
    std::vector<std::pair<std::wstring, std::wstring>> fields{
        {L"URL",     to_wstring(modal_url)},
        {L"Title",   to_wstring(modal_title)},
        {L"Target",  to_wstring(modal_target)},
        {L"Pattern", to_wstring(modal_pattern)},
    };

    std::wstring caption = (_mode == Mode::AddPodcast ? L"add" : L"edit");
    std::wstring cap_line = caption + L" podcast";
    draw_row_assembled_cols(win, 1, 0,
                            {{cap_line.append(inner_w - cap_line.size(), L' ')}},
                            cols, *this, true);

    row = 3;
    for (int i = 0; i < static_cast<int>(fields.size()); ++i)
    {
        auto const& [label, value] = fields[i];
        char cursor = (i == modal_field ? '>' : ' ');
        std::wstring display_value;
        if (modal_editing && i == modal_field)
            display_value = to_wstring(modal_edit_buffer) + L"_";
        else
            display_value = value;

        std::wstring wline = std::wstring(1, cursor) + L" " + label + L": " + display_value;
        if (static_cast<int>(wline.size()) > mw - 4)
            wline.resize(mw - 4);

        if (static_cast<int>(wline.size()) < inner_w)
            wline.append(inner_w - wline.size(), L' ');

        draw_row_assembled_cols(win, row, 0, {{wline}}, cols, *this, true);
        ++row;
    }
    draw_empty_border(win, row, 0, mw-1, true);
    ++row;
    std::wstring whelp = L"ENTER=edit/commit   S=save   ESC=cancel";
    if (static_cast<int>(whelp.size()) < inner_w)
        whelp.append(inner_w - whelp.size(), L' ');
    draw_row_assembled_cols(win, row, 0, {{whelp}}, cols, *this, true);

    for (;row<mh-2; ++row)
    {
        draw_empty_border(win, row, 0, mw-1, true);
    }
    std::wstring wfooter = L"[ OK ]   [ Cancel ]";
    if (static_cast<int>(wfooter.size()) < inner_w)
        wfooter.append(inner_w - wfooter.size(), L' ');
    draw_row_assembled_cols(win, mh - 2, 0, {{wfooter}}, cols, *this, true);
    draw_bottom_border_header(win, mh - 1, 0, cols, true);

    wnoutrefresh(win);
    delwin(win);
}

bool UiPodcastSetup::handleKey(int k, bool& done)
{

    // Editing _mode
    if (modal_editing)
    {
        if (k == KEY_BACKSPACE || k == 127)
        {
            if (!modal_edit_buffer.empty())
                modal_edit_buffer.pop_back();
        }
        else if (k == 10 || k == 13) // ENTER commits edit
        {
            switch (modal_field)
            {
                case 0:
                    modal_url = modal_edit_buffer;
                    {
                        auto [pod, shows] = _logic.queryPodcast(modal_url);
                        if (pod)
                        {
                            modal_title = pod->title;
                            modal_preview_description = pod->summary;
                            modal_preview_shows = shows;
                        }
                    }
                    break;
                case 1:
                    modal_title = modal_edit_buffer;
                    break;
                case 2:
                    modal_target = modal_edit_buffer;
                    break;
                case 3:
                    modal_pattern = modal_edit_buffer;
                    break;
            }
            modal_editing = false;
            modal_edit_buffer.clear();
        }
        else
        {
            if (k >= 32 && k <= 126)
                modal_edit_buffer.push_back(static_cast<char>(k));
        }
        return true;
    }

    // Navigation between fields
    if (k == KEY_UP)
    {
        modal_field = std::max(0, modal_field - 1);
        return true;
    }
    if (k == KEY_DOWN)
    {
        modal_field = std::min(3, modal_field + 1);
        return true;
    }

    // ENTER begins editing
    if (k == 10 || k == 13)
    {
        switch (modal_field)
        {
            case 0: modal_edit_buffer = modal_url; break;
            case 1: modal_edit_buffer = modal_title; break;
            case 2: modal_edit_buffer = modal_target; break;
            case 3: modal_edit_buffer = modal_pattern; break;
        }
        modal_editing = true;
        return true;
    }

    // Save podcast
    if (k == 's' || k == 'S')
    {
        PodcastCols p;
        p.id      = (_mode == Mode::AddPodcast ? 0 : modal_edit_id.value_or(0));
        p.title   = (modal_title.empty() ? modal_url : modal_title);
        p.pattern = modal_pattern;
        p.link    = modal_url;
        p.summary = modal_preview_description;
        p.image_url = "";
        p.target  = modal_target;

        _logic.insertPodcast(p,
            _mode == Mode::AddPodcast
                ? DowncastLogic::InsertPodcastMode::ADD
                : DowncastLogic::InsertPodcastMode::UPDATE);

        done=true;
        return true;
    }

    return false;
}
