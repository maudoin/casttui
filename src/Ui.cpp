
#if defined(_WIN32)
#include <curses.h>
#else
#include <ncursesw/curses.h>
#endif

#include "UiTable.h"
#include "DowncastLogic.h"
#include "HtmlToText.h"

#include <array>
#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <ctime>
#include <ranges>

// --------------------------------------------------------------------
// Helpers
// --------------------------------------------------------------------
inline std::wstring to_wstring(const std::string& s)
{
    return utf8_to_wstring(s);
}

// --------------------------------------------------------------------
// PodcastUI
// --------------------------------------------------------------------
class PodcastUI
{
    // ----------------------------------------------------------------
    // Rendering
    // ----------------------------------------------------------------
    void render_podcast_list(WINDOW* win)
    {
        bool focused = (focus == Focus::Podcasts);

        int count = logic.podcastCount() + 2;

        std::vector<std::wstring> titles;
        titles.reserve(count);
        titles.push_back(L"Add podcast...");
        titles.push_back(L"All");
        for (int i = 0; i < logic.podcastCount(); ++i)
        {
            titles.push_back(to_wstring(logic.podcastTitle(i)));
        }

        std::vector<HeaderColumn> cols{
            HeaderColumn{.width = -1, .name = std::nullopt, .sort = SortDir::NONE, .dynamic = true}
        };

        auto cell_cb = [&](int row, int /*col*/) -> Cell
        {
            const std::wstring& title = titles[row];
            bool isCursor   = (row == podcast_table.cursor() && focus == Focus::Podcasts);
            bool isSelected = (row >= 2 && logic.isCurrentPodcast(row - 2));

            int style;
            if (isCursor && isSelected)
                style = COLOR_PAIR(3);
            else if (isSelected)
                style = COLOR_PAIR(2);
            else if (isCursor)
                style = COLOR_PAIR(1);
            else
                style = A_NORMAL;

            return Cell{title, style};
        };

        podcast_table.render(win, count, cols, cell_cb, focused);
    }

    void render_status_filter_bar(WINDOW* win)
    {
        bool focused = (focus == Focus::Status);
        werase(win);

        int h, w;
        getmaxyx(win, h, w);
        int inner_w = w - 2;

        _statusHotspots.setWin(win);

        draw_top_border(win, 0, 0, inner_w, focused);

        std::vector<Cell> cells{{L"Status: "}};
        std::vector<HeaderColumn> cols{HeaderColumn{.width = inner_w, .name = std::nullopt, .sort = SortDir::NONE, .dynamic = false}};
        draw_row_assembled_cols(win, 1, 0, cells, cols, _statusHotspots, focused);
        int x = static_cast<int>(cells[0].text.size());

        for (int i = 0; i < static_cast<int>(status_labels.size()); ++i)
        {
            auto const& [label, status] = status_labels[i];
            bool isSelected = logic.isStatusActive(status);
            bool isCursor   = (i == cursor_status && focused);

            int style;
            if (isCursor && isSelected)
                style = COLOR_PAIR(3);
            else if (isSelected)
                style = COLOR_PAIR(2);
            else if (isCursor)
                style = COLOR_PAIR(1);
            else
                style = A_NORMAL;

            std::wstring text = label + L"  ";
            if (x + static_cast<int>(text.size()) >= inner_w)
                break;

            std::vector<Cell> cells{ {text, style} };
            std::vector<HeaderColumn> col_def{
                HeaderColumn{.width = static_cast<int>(text.size()), .name = std::nullopt, .sort = SortDir::NONE, .dynamic = false}
            };
            draw_row_assembled_cols(win, 1, x, cells, col_def, _statusHotspots, focused);
            x += static_cast<int>(text.size());
        }

        draw_bottom_border(win, 2, 0, inner_w, focused);
        wrefresh(win);
    }

    void render_shows_table(WINDOW* win)
    {
        int h, w;
        getmaxyx(win, h, w);
        int inner_h = h - 2;
        int data_h  = inner_h - 2;

        auto const& shows = logic.showsInRankRange(shows_table.firstVisibleDataRow(), data_h);

        bool focused = (focus == Focus::Shows);

        // We don't have direct access to current sort column; keep arrows neutral or infer externally.
        SortDir sort_direction = SortDir::NONE;

        std::vector<HeaderColumn> cols{
            HeaderColumn{.width = -1, .name = std::make_optional<std::wstring>(L"Title"),   .sort = SortDir::NONE, .dynamic = true},
            HeaderColumn{.width = 12, .name = std::make_optional<std::wstring>(L"Date"),    .sort = sort_direction, .dynamic = false},
            HeaderColumn{.width = 10, .name = std::make_optional<std::wstring>(L"Duration"),.sort = sort_direction, .dynamic = false},
        };

        auto cell_cb = [&](int row, int col) -> Cell
        {
            int idx = row - shows_table.firstVisibleDataRow();
            if (idx < 0 || idx >= static_cast<int>(shows.size()))
                return Cell{L"", A_NORMAL};

            auto const& s = shows[idx];

            std::wstring text;
            if (col == 0)
                text = to_wstring(s.title);
            else if (col == 1)
                text = to_wstring(s.dateStr());
            else
                text = to_wstring(s.durationStr());

            bool isSelected = (row >= 2 && logic.isShowRankSelected(row));
            bool isCursor   = (row == shows_table.cursor() && focused);

            int style;
            if (isCursor && isSelected)
                style = COLOR_PAIR(3);
            else if (isSelected)
                style = COLOR_PAIR(2);
            else if (isCursor)
                style = COLOR_PAIR(1);
            else
                style = A_NORMAL;

            return Cell{text, style};
        };

        shows_table.render(win, logic.showCount(), cols, cell_cb, focused);
    }

    void render_actions_bar(WINDOW* win)
    {
        werase(win);

        std::vector<std::wstring> lines(2, L"");

        if (focus == Focus::Shows)
        {
            lines[0] = L"Info (i)   Time sort (t)   Length sort (l)   Select Above (+)   Select Below (-)";
        }
        if (focus == Focus::Podcasts)
        {
            lines[0] = L"Refresh (r)  Edit (e)  Delete (d)";
        }

        using MediaStatus = DowncastLogic::MediaStatus;

        if (logic.isStatusActive(MediaStatus::New))
        {
            lines[1] += L"Update (u)  ";
        }

        if (logic.isStatusActive(MediaStatus::Queued))
        {
            if (logic.isDownloading())
                lines[1] += L"Downloading…   ";
            else
                lines[1] += L"Start Download (d)  ";
        }

        if (logic.anySelection())
        {
            if (!logic.isStatusActive(MediaStatus::New))
                lines[1] += L"Set New (n)  ";
            if (!logic.isStatusActive(MediaStatus::Skipped))
                lines[1] += L"Skip (s)  ";
            if (!logic.isStatusActive(MediaStatus::Queued))
                lines[1] += L"Queue (q)  ";
        }

        UiTable action_table(UiTable::Mode::SCROLL, 0, 0);
        action_table.renderArray(win, lines);
    }

    void render_bottom_bar(WINDOW* win)
    {
        werase(win);

        std::wstring text;

        if (logic.isBusy())
        {
            char spinner_chars[] = "|/-\\";
            auto now = std::chrono::system_clock::now();
            auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
            int idx  = static_cast<int>((ms / 100) & 3);
            text = L"Updating " + std::wstring(1, spinner_chars[idx]);
        }
        else if (logic.isDownloading())
        {
            auto progress = logic.getCurrentDownloadProgress();
            wchar_t spinner_chars[] = L"|/-\\";
            auto now = std::chrono::system_clock::now();
            auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
            int idx  = static_cast<int>((ms / 100) & 3);

            text =
                L"Downloading " + std::to_wstring(progress.currentFile) + L"/" +
                std::to_wstring(progress.totalFiles) + L": " +
                to_wstring(progress.currentLabel) + L" " + spinner_chars[idx];
        }
        else if (auto err = logic.lastError())
        {
            text = L"Error: " + to_wstring(*err);
        }
        else
        {
            text = L"Ready";
        }

        UiTable table(UiTable::Mode::SCROLL, 0, 0);
        table.renderArray(win, std::vector<std::wstring>{text});
    }

    void render_confirm_modal(WINDOW* stdscr)
    {
        if (modal_mode != ModalMode::Confirm)
            return;

        int h, w;
        getmaxyx(stdscr, h, w);
        int mh = 6;
        int mw = std::min(w - 4, 70);
        int y  = (h - mh) / 2;
        int x  = (w - mw) / 2;

        WINDOW* win = newwin(mh, mw, y, x);
        werase(win);

        UiTable confirm_table(UiTable::Mode::CURSOR, 0, 0);

        std::wstring title = to_wstring(logic.podcastTitle(podcast_table.cursor() - 2));
        std::wstring wtitle = L"Delete '" + title + L"'?";
        std::vector<std::wstring> lines{wtitle, L"Cancel"};

        confirm_table.renderArray(win, lines, std::nullopt, true);

        wnoutrefresh(win);
        delwin(win);
    }

    void render_info_modal(WINDOW* stdscr)
    {
        if (modal_mode != ModalMode::Info)
            return;

        int h, w;
        getmaxyx(stdscr, h, w);
        int mh = std::min(h - 4, 20);
        int mw = std::min(w - 4, 170);
        int y  = (h - mh) / 2;
        int x  = (w - mw) / 2;

        WINDOW* win = newwin(mh, mw, y, x);
        werase(win);

        auto const& shows = logic.showsInRankRange(shows_table.cursor(), 1);

        std::wstring text = shows.empty() ? L"No show selected." : html_to_text(to_wstring(shows[0].summary));
        std::vector<std::wstring> lines;

        int max_lines = mh - 2;
        int count = 0;
        std::string current;
        for (char c : text)
        {
            if (c == '\n')
            {
                lines.push_back(to_wstring(current));
                current.clear();
                if (++count >= max_lines) break;
            }
            else
            {
                current.push_back(c);
            }
        }
        if (!current.empty() && count < max_lines)
            lines.push_back(to_wstring(current));

        std::optional<std::wstring> title =
            shows.empty() ? std::nullopt : std::make_optional(to_wstring(shows[0].title));

        info_table.renderArray(win, lines, title);

        wnoutrefresh(win);
        delwin(win);
    }

    void render_add_edit_modal(WINDOW* stdscr)
    {
        if (modal_mode != ModalMode::Add && modal_mode != ModalMode::Edit)
            return;

        int h, w;
        getmaxyx(stdscr, h, w);
        int mh = std::min(h - 4, 20);
        int mw = std::min(w - 4, 70);
        int y  = (h - mh) / 2;
        int x  = (w - mw) / 2;

        WINDOW* win = newwin(mh, mw, y, x);
        werase(win);

        _addEditPodcastModalHotspots.setWin(win);

        int inner_w = mw - 2;
        std::vector<HeaderColumn> cols{
            HeaderColumn{.width = inner_w, .name = std::nullopt, .sort = SortDir::NONE, .dynamic = false}
        };

        draw_top_border_header(win, 0, 0, cols, true);

        std::wstring mode_str = (modal_mode == ModalMode::Add ? L"ADD" : L"EDIT");
        std::vector<std::wstring> header_line{mode_str};
        draw_row_assembled_cols(win, 1, 0,
                                {{mode_str + std::wstring(inner_w - mode_str.size(), L' ')}},
                                cols, _addEditPodcastModalHotspots, true);

        draw_mid_border_header(win, 2, 0, cols, true);

        int row = 3;
        std::vector<std::pair<std::wstring, std::wstring>> fields{
            {L"URL",     to_wstring(modal_url)},
            {L"Title",   to_wstring(modal_title)},
            {L"Target",  to_wstring(modal_target)},
            {L"Pattern", to_wstring(modal_pattern)},
        };

        std::wstring caption = (modal_mode == ModalMode::Add ? L"add" : L"edit");
        std::wstring cap_line = caption + L" podcast";
        draw_row_assembled_cols(win, 1, 0,
                                {{cap_line.append(inner_w - cap_line.size(), L' ')}},
                                cols, _addEditPodcastModalHotspots, true);

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

            draw_row_assembled_cols(win, row, 0, {{wline}}, cols, _addEditPodcastModalHotspots, true);
            ++row;
        }
        draw_empty_border(win, row, 0, mw-1, true);
        ++row;
        std::wstring whelp = L"ENTER=edit/commit   S=save   ESC=cancel";
        if (static_cast<int>(whelp.size()) < inner_w)
            whelp.append(inner_w - whelp.size(), L' ');
        draw_row_assembled_cols(win, row, 0, {{whelp}}, cols, _addEditPodcastModalHotspots, true);

        for (;row<mh-2; ++row)
        {
            draw_empty_border(win, row, 0, mw-1, true);
        }
        std::wstring wfooter = L"[ OK ]   [ Cancel ]";
        if (static_cast<int>(wfooter.size()) < inner_w)
            wfooter.append(inner_w - wfooter.size(), L' ');
        draw_row_assembled_cols(win, mh - 2, 0, {{wfooter}}, cols, _addEditPodcastModalHotspots, true);
        draw_bottom_border_header(win, mh - 1, 0, cols, true);

        wnoutrefresh(win);
        delwin(win);
    }
    // --------------------------------------------------------------------
    // Key handling
    // --------------------------------------------------------------------

    bool handle_confirm_modal_key(int k)
    {
        if (k == 27) // ESC
        {
            if (modal_mode != ModalMode::None)
                modal_mode = ModalMode::None;
        }

        if (k == 'y' || k == 'Y')
        {
            if (modal_delete_id)
            {
                logic.deletePodcastAtIndex(*modal_delete_id);
            }
            modal_mode = ModalMode::None;
            modal_delete_id.reset();

            podcast_table.scrollVertical(1);
        }
        else if (k == 'n' || k == 'N' || k == 27)
        {
            modal_mode = ModalMode::None;
            modal_delete_id.reset();
        }
        return true;
    }

    bool handle_info_modal_key(int k)
    {
        if (k == 27) // ESC
        {
            if (modal_mode != ModalMode::None)
                modal_mode = ModalMode::None;
        }
        return info_table.handleKeyCh(k);
    }

    bool handle_add_edit_modal_key(int k)
    {
        if (k == 27) // ESC
        {
            if (modal_editing)
            {
                modal_editing = false;
                modal_edit_buffer.clear();
            }
            else
            {
                modal_mode = ModalMode::None;
            }
            return true;
        }

        // Editing mode
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
                            auto [pod, shows] = logic.queryPodcast(modal_url);
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
            p.id      = (modal_mode == ModalMode::Add ? 0 : modal_edit_id.value_or(0));
            p.title   = (modal_title.empty() ? modal_url : modal_title);
            p.pattern = modal_pattern;
            p.link    = modal_url;
            p.summary = modal_preview_description;
            p.image_url = "";
            p.target  = modal_target;

            logic.insertPodcast(p,
                modal_mode == ModalMode::Add
                    ? DowncastLogic::InsertPodcastMode::ADD
                    : DowncastLogic::InsertPodcastMode::UPDATE);

            modal_mode = ModalMode::None;
            return true;
        }

        return false;
    }

    bool handle_podcast_key(int k)
    {
        if (podcast_table.handleKeyCh(k))
            return true;

        if (k == KEY_LEFT)
        {
            shows_table.scrollHorizontal(-5);
            return true;
        }

        // Only valid podcast rows (skip Add/All)
        if (podcast_table.cursor() >= 2 &&
            podcast_table.cursor() < logic.podcastCount() + 2)
        {
            auto const& p = logic.podcast(podcast_table.cursor() - 2);

            if (k == 'r' || k == 'R')
            {
                logic.refreshPodcastAtIndex(podcast_table.cursor() - 2);
                return true;
            }
            if (k == 'e' || k == 'E')
            {
                modal_url = p.link;
                modal_title = p.title;
                modal_target = p.target;
                modal_pattern = p.pattern;
                modal_preview_description = p.summary;
                modal_field = 0;
                modal_delete_id.reset();
                modal_edit_id = p.id;
                modal_mode = ModalMode::Edit;
                return true;
            }
            if (k == 'd' || k == 'D')
            {
                modal_delete_id = p.id;
                modal_mode = ModalMode::Confirm;
                return true;
            }
        }

        // ENTER behavior
        if (k == 10 || k == 13)
        {
            if (podcast_table.cursor() == 0)
            {
                modal_delete_id.reset();
                modal_mode = ModalMode::Add;
                return true;
            }
            else if (podcast_table.cursor() == 1)
            {
                logic.setCurrentPodcastRowIndex(std::optional<std::optional<int>>{std::nullopt}, std::nullopt);
                //TODO podcast_table.firstVisibleDataRow() = 0;
                //TODO shows_table.cursor() = 0;
                return true;
            }
            else
            {
                logic.setCurrentPodcastRowIndex(podcast_table.cursor() - 2, std::nullopt);
                //TODO podcast_table.firstVisibleDataRow() = 0;
                //TODO shows_table.cursor() = 0;
                return true;
            }
        }

        return false;
    }

    bool handle_status_key(int k)
    {
        auto move_status_cursor = [&](int delta)
        {
            cursor_status = std::max(
                0,
                std::min(static_cast<int>(status_labels.size()) - 1,
                        cursor_status + delta)
            );
        };

        if (k == KEY_LEFT)
        {
            move_status_cursor(-1);
            return true;
        }
        if (k == KEY_RIGHT)
        {
            move_status_cursor(1);
            return true;
        }

        // ENTER selects status
        if (k == 10 || k == 13)
        {
            auto const& [_, status] = status_labels[cursor_status];
            logic.setCurrentPodcastRowIndex(
                std::nullopt,
                status,
                DowncastLogic::SetPodcastOption::FORCE_REFRESH
            );
            return true;
        }

        return false;
    }

    bool handle_shows_key(int k)
    {
        if (shows_table.handleKeyCh(k))
            return true;

        using MediaStatus = DowncastLogic::MediaStatus;

        if (k == 'i' || k == 'I')
        {
            modal_mode = ModalMode::Info;
            return true;
        }

        if (k == 'u' || k == 'U')
        {
            if (logic.isStatusActive(MediaStatus::New))
                logic.refreshCurrentPodcast();
            return true;
        }

        if (logic.anySelection())
        {
            if (k == 'q' || k == 'Q')
            {
                logic.setSelectedShowsStatus(Status::QUEUED);
                return true;
            }
            if (k == 's' || k == 'S')
            {
                logic.setSelectedShowsStatus(Status::SKIPPED);
                return true;
            }
            if (k == 'n' || k == 'N')
            {
                logic.setSelectedShowsStatus(Status::NEW);
                return true;
            }
        }

        if (k == 'd' || k == 'D')
        {
            if (logic.isStatusActive(MediaStatus::Queued))
                logic.startDownload();
            return true;
        }

        if (k == 10 || k == 13) // ENTER
        {
            logic.showSelection(shows_table.cursor(), false, false);
            return true;
        }

        if (k == ' ')
        {
            logic.showSelection(shows_table.cursor(), true, false);
            return true;
        }

        // Time sort toggle
        if (k == 't' || k == 'T')
        {
            logic.setShowSorting(&MediaViewCols::date,
                                DowncastLogic::SortingOption::ASCENDING);
            return true;
        }

        // Duration sort toggle
        if (k == 'l' || k == 'L')
        {
            logic.setShowSorting(&MediaViewCols::duration,
                                DowncastLogic::SortingOption::ASCENDING);
            return true;
        }

        return false;
    }

    bool handle_mouse(MouseEvent const& ev)
    {
        // always active
        if (podcast_table.hotspots().handleMouseEvent(ev))
            return true;
        if (shows_table.hotspots().handleMouseEvent(ev))
            return true;
        if (_statusHotspots.handleMouseEvent(ev))
            return true;

        // Modal active only when visible
        if (modal_mode == ModalMode::Confirm)
            return _confirmModalHotspots.handleMouseEvent(ev);

        if (modal_mode == ModalMode::Info)
            return info_table.hotspots().handleMouseEvent(ev);

        if (modal_mode == ModalMode::Add ||
            modal_mode == ModalMode::Edit)
            return _addEditPodcastModalHotspots.handleMouseEvent(ev);
        return false;
    }

    bool handle_key(WINDOW* stdscr, int k)
    {
        if (k == KEY_RESIZE)
        {
            render_layout(stdscr);
            return true;
        }
        if (k == KEY_MOUSE)
        {
            if (auto event = getMouseEvent())
            {
                if (handle_mouse(*event))
                {
                    return true;
                }
            }
        }
        // ESC closes app only when no modal is open
        if (k == 27)
        {
            if (modal_mode == ModalMode::None)
            {
                running = false;
                return true;
            }
        }

        // Focus cycling order
        static const std::array<Focus,3> focus_order{
            Focus::Podcasts,
            Focus::Status,
            Focus::Shows
        };

        auto status_cycle = [&](int amount)
        {
            auto it = std::find(focus_order.begin(), focus_order.end(), focus);
            int idx = std::distance(focus_order.begin(), it);
            idx = (idx + amount + focus_order.size()) % focus_order.size();
            return focus_order[idx];
        };

        // TAB → forward
        if (k == 9)
        {
            focus = status_cycle(1);
            return true;
        }

        // SHIFT+TAB
        if (k == KEY_BTAB)
        {
            focus = status_cycle(-1);
            return true;
        }

        // Auto switching windows
        if (k == KEY_RIGHT)
        {
            if (focus == Focus::Podcasts)
            {
                if (podcast_table.cursor() < 2)
                    focus = Focus::Status;
                else
                    focus = Focus::Shows;
                return true;
            }
        }

        if (k == KEY_LEFT)
        {
            if (focus == Focus::Shows &&
                shows_table.dynamicColCurrentOffsetX() == 0)
            {
                focus = Focus::Podcasts;
                return true;
            }
            if (focus == Focus::Status &&
                cursor_status == 0)
            {
                focus = Focus::Podcasts;
                return true;
            }
        }

        if (k == KEY_DOWN)
        {
            if (focus == Focus::Status)
            {
                focus = Focus::Shows;
                return true;
            }
        }

        if (k == KEY_UP)
        {
            if (focus == Focus::Shows &&
                shows_table.cursor() == 0)
            {
                focus = Focus::Status;
                return true;
            }
        }

        // Modal dispatch
        if (modal_mode == ModalMode::Confirm)
            return handle_confirm_modal_key(k);

        if (modal_mode == ModalMode::Info)
            return handle_info_modal_key(k);

        if (modal_mode == ModalMode::Add ||
            modal_mode == ModalMode::Edit)
            return handle_add_edit_modal_key(k);

        // Normal dispatch
        switch (focus)
        {
            case Focus::Podcasts:
                return handle_podcast_key(k);
            case Focus::Status:
                return handle_status_key(k);
            case Focus::Shows:
                return handle_shows_key(k);
        }

        return false;
    }
    void delWindows()
    {
        delwin(win_left);
        delwin(win_status);
        delwin(win_shows);
        delwin(win_actions);
        delwin(win_bottom);
    }
    void buildWindows()
    {
        int h, w;
        getmaxyx(stdscr, h, w);

        int left_w = std::max(20, w / 4);
        int right_w = w - left_w;

        int min_shows_h = 3;
        int status_h = 3;
        int actions_h = 4;
        int bottom_h = 3;

        status_h = std::max(min_shows_h, status_h);
        actions_h = std::max(min_shows_h, actions_h);
        bottom_h = std::max(min_shows_h, bottom_h);

        int shows_h = std::max(min_shows_h, h - status_h - actions_h - bottom_h);

        win_left   = newwin(h,        left_w, 0,                 0);
        win_status = newwin(status_h, right_w, 0,                 left_w);
        win_shows  = newwin(shows_h,  right_w, status_h,          left_w);
        win_actions= newwin(actions_h,right_w, status_h + shows_h,left_w);
        win_bottom = newwin(bottom_h, right_w, status_h + shows_h + actions_h, left_w);
    }
    void render_layout(WINDOW* stdscr)
    {
        render_podcast_list(win_left);
        render_status_filter_bar(win_status);
        render_shows_table(win_shows);
        render_actions_bar(win_actions);
        render_bottom_bar(win_bottom);

        wnoutrefresh(stdscr);
        wnoutrefresh(win_left);
        wnoutrefresh(win_status);
        wnoutrefresh(win_shows);
        wnoutrefresh(win_actions);
        wnoutrefresh(win_bottom);

        if (modal_mode == ModalMode::Confirm)
        {
            render_confirm_modal(stdscr);
        }
        else if (modal_mode == ModalMode::Info)
        {
            render_info_modal(stdscr);
        }
        else if (modal_mode == ModalMode::Add || modal_mode == ModalMode::Edit)
        {
            render_add_edit_modal(stdscr);
        }

        doupdate();

    }

public:
    void run()
    {
        while (running)
        {
            render_layout(stdscr);

            int k = wgetch(stdscr);
            if (k == KEY_RESIZE)
            {
                // Update curses internal structures
                resize_term(0, 0);

                // Recreate your windows with new sizes
                delWindows();
                buildWindows();

                // Redraw everything
                render_layout(stdscr);

                // Refresh all windows
                wnoutrefresh(stdscr);
                doupdate();
            }
            else
            {
                handle_key(stdscr, k);
            }
        }
    }
    ~PodcastUI()
    {
        delWindows();
        endwin();
    }
    explicit PodcastUI(const std::string& db_path)
        : logic(db_path)
        , focus(Focus::Podcasts)
        , podcast_table(UiTable::Mode::CURSOR, [&]{focus = Focus::Podcasts;})
        , shows_table(UiTable::Mode::CURSOR, [&]{focus = Focus::Shows;})
        , info_table(UiTable::Mode::SCROLL, [&]{})
        , _statusHotspots([&]{focus = Focus::Status;})
        , _confirmModalHotspots([]{})
        , _addEditPodcastModalHotspots([]{})
        , cursor_status(0)
    {
        using MediaStatus = DowncastLogic::MediaStatus;
        status_labels = {
            {L"New",    MediaStatus::New},
            {L"Queue",  MediaStatus::Queued},
            {L"Skipped",MediaStatus::Skipped},
            {L"Done",   MediaStatus::Done},
            {L"All",    std::nullopt}
        };
        initscr();

        cbreak();
        noecho();

        mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
        mouseinterval(0);//CLICKED will not work but gives fast mouse event response
        // set_escdelay(0); // wgetch(win) -> wgetch_escdelay(win, delay)
        curs_set(0);
        nodelay(stdscr, FALSE);
        keypad(stdscr, TRUE);

        // Colors
        start_color();
        use_default_colors();

        init_pair(1, COLOR_BLACK, COLOR_WHITE); // cursor highlight
        init_pair(2, COLOR_BLUE, -1);           // active status
        init_pair(3, COLOR_BLUE, COLOR_WHITE);  // cursor + active
        init_pair(4, COLOR_BLUE, -1);           // active window

        buildWindows();
    }

    enum class Focus { Podcasts, Status, Shows };
    enum class ModalMode { None, Add, Edit, Info, Confirm };

    struct StatusLabel
    {
        std::wstring label;
        std::optional<DowncastLogic::MediaStatus> status;
    };

    DowncastLogic logic;

    Focus focus;
    UiTable podcast_table;
    UiTable shows_table;
    UiTable info_table;
    UiHotspotGoup _statusHotspots;
    UiHotspotGoup _confirmModalHotspots;
    UiHotspotGoup _addEditPodcastModalHotspots;
    int cursor_status;

    std::vector<StatusLabel> status_labels;

    ModalMode modal_mode = ModalMode::None;
    int modal_field = 0;
    std::string modal_url;
    std::string modal_title;
    std::string modal_target = ".";
    std::string modal_pattern = "{date}-{title}";
    bool modal_editing = false;
    std::string modal_edit_buffer;
    std::string modal_preview_description;
    std::vector<std::string> modal_preview_shows;
    std::optional<int> modal_delete_id;
    std::optional<int> modal_edit_id;

    bool running = true;

    WINDOW* win_left   = nullptr;
    WINDOW* win_status = nullptr;
    WINDOW* win_shows  = nullptr;
    WINDOW* win_actions= nullptr;
    WINDOW* win_bottom = nullptr;
};

#ifndef _WIN32
#include <locale.h>
#endif

int main()
{
    #ifndef _WIN32
        setlocale(LC_ALL, "");
    #endif

    PodcastUI ui("castapod.db3");
    ui.run();

    return 0;
}
