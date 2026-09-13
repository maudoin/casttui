
#if defined(_WIN32)
#include <curses.h>
#else
#include <ncursesw/curses.h>
#endif

#include "UiPodcastTable.h"
#include "UiShowsTable.h"
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
// PodcastUI
// --------------------------------------------------------------------
class PodcastUI
{
    // ----------------------------------------------------------------
    // Rendering
    // ----------------------------------------------------------------

    void render_status_filter_bar(bool focused)
    {
        std::wstring title = L"Status: ";
        std::wstring quitLabel = L" X ";
        std::vector<HeaderColumn> cols;
        cols.reserve(status_labels.size()+3);//+title+spacer+quit
        // title
        cols.push_back(HeaderColumn{.width = static_cast<int>(title.size())});
        // statuses
        for (int i = 0; i < static_cast<int>(status_labels.size()); ++i)
        {
            auto const& [label, status] = status_labels[i];
            cols.push_back(HeaderColumn{.width = static_cast<int>(label.size())});
        };
        // spacer
        cols.push_back(HeaderColumn{.dynamic = true});
        // quit
        cols.push_back(HeaderColumn{.width =  static_cast<int>(quitLabel.size())});

        auto cell_cb = [&](int row, int col) -> Cell
        {
            if (col == 0)
            {
                // Title
                return Cell{title, A_NORMAL};
            }
            if (col == cols.size()-1)
            {
                // Quit
                return Cell{quitLabel, A_NORMAL, [this]{
                    this->confirm_field=1;//cancel
                    this->confirm_title = L"Quit";
                    this->confirm_action = [this]{this->running = false;};
                    this->modal_mode = ModalMode::ConfirmExit;
                }};
            }
            int statusIndex = col-1;
            if (statusIndex >= status_labels.size())
            {
                // Spacer
                return Cell{L"", A_NORMAL};
            }
            auto const& [label, status] = status_labels[statusIndex];
            bool isSelected = logic.isStatusActive(status);
            bool isCursor   = (statusIndex == cursor_status && focused);

            int style;
            if (isCursor && isSelected)
                style = COLOR_PAIR(3);
            else if (isSelected)
                style = COLOR_PAIR(2);
            else if (isCursor)
                style = COLOR_PAIR(1);
            else
                style = A_NORMAL;

            return Cell{label, style,  [this, statusIndex]{
                cursor_status = statusIndex;
                auto const& [_, status] = this->status_labels[statusIndex];
                this->logic.setCurrentPodcastRowIndex(
                    std::nullopt,
                    status,
                    DowncastLogic::SetPodcastOption::FORCE_REFRESH
                );
            }};
        };

        status_table.render(1, cols, cell_cb, focused);
        return;
    }

    void render_actions_bar()
    {
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

        _actionsUi.renderArray(lines);
    }

    void render_bottom_bar()
    {
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

        _bottomBarUi.renderArray(std::vector<std::wstring>{text});
    }

    void render_confirm_modal()
    {
        std::wstring cancel = L"Cancel";

        auto cell_cb = [&](int, int col) -> Cell {
            bool const isCursor = col==confirm_field;
            int style = A_NORMAL;
            if (isCursor)
                style = COLOR_PAIR(1);
            if (col == 0)
            {
                return Cell{
                    .text=confirm_title,
                    .style=style,
                    .callback=[this]{
                        confirm_action();
                        this->modal_mode = ModalMode::None;
                        this->confirm_action = []{};
                        this->confirm_title = L"";
                    }
                };
            }
            return Cell{
                .text=cancel,
                .style=style,
                .callback=[this]{
                    this->modal_mode = ModalMode::None;
                    this->confirm_action = []{};
                    this->confirm_title = L"";
                }
            };
        };

        std::vector<HeaderColumn> cols{
            HeaderColumn{ .width = 0, .dynamic=true },
            HeaderColumn{ .width = static_cast<int>(cancel.size()) }
        };

        confirm_table.render(1, cols, cell_cb, true);
    }

    void render_info_modal(WINDOW* stdscr)
    {
        int h, w;
        getmaxyx(stdscr, h, w);
        int mh = std::min(h - 4, 20);
        int mw = std::min(w - 4, 170);
        int y  = (h - mh) / 2;
        int x  = (w - mw) / 2;

        info_table.delWindow();
        info_table.buildWindow(mh, mw, y, x);

        auto const& shows = logic.showsInRankRange(_showsUi.cursor(), 1);

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

        info_table.renderArray(lines, title);

    }

    void render_add_edit_modal(WINDOW* stdscr)
    {
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

        std::wstring mode_str = (modal_mode == ModalMode::AddPodcast ? L"ADD" : L"EDIT");
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

        std::wstring caption = (modal_mode == ModalMode::AddPodcast ? L"add" : L"edit");
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
            modal_mode = ModalMode::None;
            confirm_action = []{};
            confirm_title = L"";
            return true;
        }
        // Auto switching windows
        if (k == KEY_RIGHT)
        {
            confirm_field = std::max(1, confirm_field+1);
            return true;
        }
        else if (k == KEY_LEFT)
        {
            confirm_field = std::min(0, confirm_field-1);
            return true;
        }
        else if (k == 10 || k == 13) // ENTER commits edit
        {
            if (confirm_field == 1)
            {
                modal_mode = ModalMode::None;
                confirm_action = []{};
                confirm_title = L"";
                return true;
            }
            else if (confirm_field == 0)
            {
                confirm_action();
                return true;
            }

        }
        return confirm_table.handleKeyCh(k);
    }

    bool handle_info_modal_key(int k)
    {
        if (k == 27) // ESC
        {
            modal_mode = ModalMode::None;
        }
        return info_table.handleKeyCh(k);
    }

    bool handle_add_edit_modal_key(int k)
    {
        if (k == 27) // ESC
        {
            modal_editing = false;
            modal_edit_buffer.clear();
            modal_mode = ModalMode::None;
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
            p.id      = (modal_mode == ModalMode::AddPodcast ? 0 : modal_edit_id.value_or(0));
            p.title   = (modal_title.empty() ? modal_url : modal_title);
            p.pattern = modal_pattern;
            p.link    = modal_url;
            p.summary = modal_preview_description;
            p.image_url = "";
            p.target  = modal_target;

            logic.insertPodcast(p,
                modal_mode == ModalMode::AddPodcast
                    ? DowncastLogic::InsertPodcastMode::ADD
                    : DowncastLogic::InsertPodcastMode::UPDATE);

            modal_mode = ModalMode::None;
            return true;
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

    bool handle_mouse(MouseEvent const& ev)
    {
        if (modal_mode == ModalMode::None)
        {
            // always active
            if (_podcastUi.handleMouseEvent(ev))
                return true;
            if (_showsUi.handleMouseEvent(ev))
                return true;
            if (status_table.hotspots().handleMouseEvent(ev))
                return true;
        }

        // Modal active only when visible
        if (modal_mode == ModalMode::ConfirmDeletePodcast ||
            modal_mode == ModalMode::ConfirmExit)
            return confirm_table.hotspots().handleMouseEvent(ev);

        if (modal_mode == ModalMode::Info)
            return info_table.hotspots().handleMouseEvent(ev);

        if (modal_mode == ModalMode::AddPodcast ||
            modal_mode == ModalMode::EditPodcast)
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
                modal_mode=ModalMode::ConfirmExit;
                confirm_title = L"Quit";
                confirm_action = [this]{this->running = false;};
                confirm_field=1;//cancel
                return true;
            }
        }

        // Modal dispatch
        if (modal_mode == ModalMode::ConfirmDeletePodcast ||
            modal_mode == ModalMode::ConfirmExit)
            return handle_confirm_modal_key(k);

        if (modal_mode == ModalMode::Info)
            return handle_info_modal_key(k);

        if (modal_mode == ModalMode::AddPodcast ||
            modal_mode == ModalMode::EditPodcast)
            return handle_add_edit_modal_key(k);

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
                if (_podcastUi.cursor() < 2)
                    focus = Focus::Status;
                else
                    focus = Focus::Shows;
                return true;
            }
        }

        if (k == KEY_LEFT)
        {
            if (focus == Focus::Shows &&
                _showsUi.dynamicColCurrentOffsetX() == 0)
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
                _showsUi.cursor() == 0)
            {
                focus = Focus::Status;
                return true;
            }
        }

        // Normal dispatch
        switch (focus)
        {
            case Focus::Podcasts:
                return _podcastUi.handleKey(k);
            case Focus::Status:
                return handle_status_key(k);
            case Focus::Shows:
                if (_showsUi.handleKey(k))
                {
                    return true;
                }
                if (k == 'i' || k == 'I')
                {
                    modal_mode = ModalMode::Info;
                    return true;
                }
                return false;
        }

        return false;
    }
    void delWindows()
    {
        _podcastUi.delWindow();
        _showsUi.delWindow();
        _actionsUi.delWindow();
        _bottomBarUi.delWindow();
        info_table.delWindow();
        status_table.delWindow();
        confirm_table.delWindow();
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

        _podcastUi.buildWindow(h,        left_w, 0,                 0);
        status_table.buildWindow(status_h, right_w, 0,                 left_w);
        _showsUi.buildWindow(shows_h,  right_w, status_h,          left_w);
        _actionsUi.buildWindow(actions_h,right_w, status_h + shows_h,left_w);
        _bottomBarUi.buildWindow(bottom_h, right_w, status_h + shows_h + actions_h, left_w);

        int mh = 3;
        int mw = std::min(w - 4, 70);
        int y  = (h - mh) / 2;
        int x  = (w - mw) / 2;
        confirm_table.buildWindow(mh, mw, y, x);
    }
    void render_layout(WINDOW* stdscr)
    {
        wnoutrefresh(stdscr);

        _podcastUi.render(focus == Focus::Podcasts);
        render_status_filter_bar(focus == Focus::Status);
        _showsUi.render(focus == Focus::Shows);
        render_actions_bar();
        render_bottom_bar();

        if (modal_mode == ModalMode::ConfirmDeletePodcast ||
            modal_mode == ModalMode::ConfirmExit)
        {
            render_confirm_modal();
        }
        else if (modal_mode == ModalMode::Info)
        {
            render_info_modal(stdscr);
        }
        else if (modal_mode == ModalMode::AddPodcast ||
                 modal_mode == ModalMode::EditPodcast)
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
        , _podcastUi(logic, UiPodcastTable::Actions{
            .winSelection=[&]{focus = Focus::Podcasts;},
            .add=[&]()
            {
                modal_delete_id.reset();
                modal_mode = ModalMode::AddPodcast;
            },
            .edit=[this](PodcastCols const&p)
            {
                this->modal_url = p.link;
                this->modal_title = p.title;
                this->modal_target = p.target;
                this->modal_pattern = p.pattern;
                this->modal_preview_description = p.summary;
                this->modal_field = 0;
                this->modal_delete_id.reset();
                this->modal_edit_id = p.id;
                this->modal_mode = ModalMode::EditPodcast;
            },
            .del=[&](std::wstring const& title, std::function<void()> const& del)
            {
                modal_mode = ModalMode::ConfirmDeletePodcast;
                confirm_title = title;
                confirm_action = del;
                confirm_field = 1;//cancel
            }
        })
        , _showsUi(logic, [&]{focus = Focus::Shows;})
        , _actionsUi(UiTable::Mode::SCROLL)
        , _bottomBarUi(UiTable::Mode::SCROLL)
        , info_table(UiTable::Mode::SCROLL, [&]{/*no focus action*/})
        , status_table(UiTable::Mode::CURSOR, [&]{focus = Focus::Status;})
        , confirm_table(UiTable::Mode::CURSOR, [&]{/*no focus action*/})
        , _addEditPodcastModalHotspots([]{/*no focus action*/})
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
    enum class ModalMode { None, AddPodcast, EditPodcast, Info, ConfirmDeletePodcast, ConfirmExit };

    struct StatusLabel
    {
        std::wstring label;
        std::optional<DowncastLogic::MediaStatus> status;
    };

    DowncastLogic logic;

    Focus focus;
    UiPodcastTable _podcastUi;
    UiShowsTable _showsUi;
    UiTable _actionsUi;
    UiTable _bottomBarUi;
    UiTable info_table;
    UiTable status_table;
    UiTable confirm_table;
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
    std::wstring confirm_title;
    std::function<void()> confirm_action;
    int confirm_field = 1;//accept,cancel

    bool running = true;
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
