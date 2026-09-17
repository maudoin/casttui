#include "UiApp.h"
#include "UiConfirm.h"
#include "UiPodcastSetup.h"
#include "UiPodcastTable.h"
#include "UiShowsTable.h"
#include "UiStatusTable.h"
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

#ifndef _WIN32
#include <locale.h>
#endif

class Ui : public UiApp
{
  enum class Focus { Podcasts, Status, Shows };
  enum class ModalMode { None, AddEditPodcast, Info, Confirm };
  public:
  explicit Ui(const std::string& db_path)
  : _logic(db_path)
  , _focusedPanel(Focus::Podcasts)
  , _podcastUi(_logic, UiPodcastTable::Actions{
    .winSelection=[&]{_focusedPanel = Focus::Podcasts;},
    .add=[&]()
    {
      this->_addEditPodcastUi.setAdd();
      this->_modalPopup = ModalMode::AddEditPodcast;
    },
    .edit=[this](PodcastCols const&p)
    {
      this->_addEditPodcastUi.setEdit(p);
      this->_modalPopup = ModalMode::AddEditPodcast;
    },
    .del=[&](std::wstring const& title, std::function<void()> const& del)
    {
      this->_confirmUi.set(title, del);
      this->_modalPopup = ModalMode::Confirm;
    }
  })
  , _showsUi(_logic, [&]{_focusedPanel = Focus::Shows;})
  , _actionsUi(UiTable::Mode::SCROLL)
  , _bottomBarUi(UiTable::Mode::SCROLL)
  , _statusUi(_logic, [&]{_focusedPanel = Focus::Status;},[this]{
    this->_confirmUi.set(L"Quit", [this]{this->_isRunning = false;});
    this->_modalPopup = ModalMode::Confirm;
  })
  , _addEditPodcastUi(_logic, [this]{this->_modalPopup = ModalMode::None;})
  , _confirmUi([&]{
    this->_confirmUi.set(L"", []{});
    this->_modalPopup=ModalMode::None;
  })
  , _infoUi(UiTable::Mode::SCROLL, [&]{/*no _focusedPanel action*/})
  {
    buildWindows();
  }

  ~Ui()
  {
    delWindows();
  }

private:

  void renderActionsBar(int k)
  {
    std::vector<HeaderColumn> items;

    if (_focusedPanel == Focus::Shows)
    {
      auto sortCallback = [&](auto const& col)->std::function<void()>{
        return [=, this]{
          this->_logic.setShowSorting(col, DowncastLogic::cycle(_logic.getShowSorting(col)));
        };
      };
      items.push_back({ .width=HeaderColumn::FIT_LABEL, .name = L"Info (i)",          .callback = [this]{this->_modalPopup = ModalMode::Info;} });
      items.push_back({ .width=HeaderColumn::FIT_LABEL, .name = L"Name sort (n)",     .callback = sortCallback(&MediaViewCols::title)});
      items.push_back({ .width=HeaderColumn::FIT_LABEL, .name = L"Time sort (t)",     .callback = sortCallback(&MediaViewCols::date)});
      items.push_back({ .width=HeaderColumn::FIT_LABEL, .name = L"Length sort (l)",   .callback = sortCallback(&MediaViewCols::duration)});
      items.push_back({ .width=HeaderColumn::FIT_LABEL, .name = L"Select Above (-)",  .callback = [this]{
        this->_logic.selectShowRange(0, this->_showsUi.cursor(), true);}});
        items.push_back({ .width=HeaderColumn::FIT_LABEL, .name = L"Select Below (+)", .callback = [this]{
          this->_logic.selectShowRange(this->_showsUi.cursor(), this->_logic.showCount()-1, true);}});
      }

      auto podcastIndexOpt = _podcastUi.getPodcastIndex();
      if (_focusedPanel == Focus::Podcasts && podcastIndexOpt)
      {
        int podcastIndex = *podcastIndexOpt;
        items.push_back({ .width=HeaderColumn::FIT_LABEL, .name = L"Refresh (r)", .callback = [this, podcastIndex]{
          if (podcastIndex<_logic.podcastCount())
          {
            this->_logic.refreshPodcastAtIndex(podcastIndex);}
          }
        });
        items.push_back({ .width=HeaderColumn::FIT_LABEL, .name = L"Edit (e)",    .callback = [this, podcastIndex]{
          if (podcastIndex<_logic.podcastCount())
          {
            this->_addEditPodcastUi.setEdit(_logic.podcast(podcastIndex));
            this->_modalPopup = ModalMode::AddEditPodcast;
          }
        } });
        items.push_back({ .width=HeaderColumn::FIT_LABEL, .name = L"Delete (d)",  .callback = [this, podcastIndex]{
          this->_modalPopup = ModalMode::Confirm;
          this->_confirmUi.set(L"Delete '" + to_wstring(_logic.podcast(podcastIndex).title) + L"'?", [this, podcastIndex]{
            if (podcastIndex<_logic.podcastCount())
            {
              auto const& podcast = _logic.podcast(podcastIndex);
              this->_logic.deletePodcast(podcast.id);
              this->_podcastUi.scrollVertical(-1);
            }
          });
      } });
    }
    using MediaStatus = DowncastLogic::MediaStatus;
    if (_logic.isStatusActive(MediaStatus::New))
    {
      items.push_back({ .width=HeaderColumn::FIT_LABEL, .name = L"Update (u)", .callback = [this]{this->_logic.refreshCurrentPodcast();} });
    }

    if (_logic.isStatusActive(MediaStatus::Queued))
    {
      if (_logic.isDownloading())
      {
        items.push_back({ .width=HeaderColumn::FIT_LABEL, .name = L"Downloading…", .callback = []{} });
      }
      else
      {
        items.push_back({ .width=HeaderColumn::FIT_LABEL, .name = L"Start Download (d)", .callback = [this]{this->_logic.startDownload();} });
      }
    }

    if (_logic.anySelection())
    {
      if (!_logic.isStatusActive(MediaStatus::New))
      items.push_back({ .width=HeaderColumn::FIT_LABEL, .name = L"Set New (n)", .callback = [this]{this->_logic.setSelectedShowsStatus(Status::NEW);} });

      if (!_logic.isStatusActive(MediaStatus::Skipped))
      items.push_back({ .width=HeaderColumn::FIT_LABEL, .name = L"Skip (s)", .callback = [this]{this->_logic.setSelectedShowsStatus(Status::SKIPPED);} });

      if (!_logic.isStatusActive(MediaStatus::Queued))
      items.push_back({ .width=HeaderColumn::FIT_LABEL, .name = L"Queue (q)", .callback = [this]{this->_logic.setSelectedShowsStatus(Status::QUEUED);} });
    }

    items.push_back({ .width=HeaderColumn::FILL, .name=L""});
    _actionsUi.render(k, 0, items, [](int, int, std::optional<MouseEvent> const&){return Cell{};});
  }

  void renderBottomBar(int k)
  {
    std::wstring text;

    if (_logic.isBusy())
    {
      char spinner_chars[] = "|/-\\";
      auto now = std::chrono::system_clock::now();
      auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
      int idx  = static_cast<int>((ms / 100) & 3);
      text = L"Updating " + std::wstring(1, spinner_chars[idx]);
    }
    else if (_logic.isDownloading())
    {
      auto progress = _logic.getCurrentDownloadProgress();
      wchar_t spinner_chars[] = L"|/-\\";
      auto now = std::chrono::system_clock::now();
      auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
      int idx  = static_cast<int>((ms / 100) & 3);

      text =
      L"Downloading " + std::to_wstring(progress.currentFile) + L"/" +
      std::to_wstring(progress.totalFiles) + L": " +
      to_wstring(progress.currentLabel) + L" " + spinner_chars[idx];
    }
    else if (auto err = _logic.lastError())
    {
      text = L"Error: " + to_wstring(*err);
    }
    else
    {
      text = L"Ready";
    }

    _bottomBarUi.renderArray(k, std::vector<Cell>{Cell{text}});
  }


  void renderInfoModal(int k)
  {
    int h, w;
    getmaxyx(stdscr, h, w);
    int mh = std::min(h - 4, 20);
    int mw = std::min(w - 4, 170);
    int y  = (h - mh) / 2;
    int x  = (w - mw) / 2;

    _infoUi.delWindow();
    _infoUi.buildWindow(mh, mw, y, x);

    auto const& shows = _logic.showsInRankRange(_showsUi.cursor(), 1);

    std::wstring text = shows.empty() ? L"No show selected." : html_to_text(to_wstring(shows[0].summary));
    auto lines  = std::views::split(text, '\n');


    std::optional<std::wstring> title =
      shows.empty() ? std::nullopt : std::make_optional(to_wstring(shows[0].title));
    std::vector<HeaderColumn> cols{
        HeaderColumn{.width=HeaderColumn::FILL, .name=title}};

    auto getCell = [&](int row, int col, std::optional<MouseEvent> const&)
    {
      auto range=*std::next(lines.begin(), row);
      return Cell{.text={range.begin(), range.end()}};
    };
    _infoUi.render(k, static_cast<int>(std::ranges::distance(lines)), cols, getCell, true);

  }

  // --------------------------------------------------------------------

  bool doHandleMouse(MouseEvent const& ev) override
  {
    if (_modalPopup == ModalMode::None)
    {
      // always active
      if (_podcastUi.handleMouseEvent(ev))
      return true;
      if (_showsUi.handleMouseEvent(ev))
      return true;
      if (_statusUi.handleMouseEvent(ev))
      return true;
    }

    // Modal active only when visible
    if (_modalPopup == ModalMode::Confirm)
    return _confirmUi.handleMouseEvent(ev);
    else if (_modalPopup == ModalMode::Info)
    return _infoUi.handleMouseEvent(ev);
    else if (_modalPopup == ModalMode::AddEditPodcast)
    return _addEditPodcastUi.handleMouseEvent(ev);

    _actionsUi.handleMouseEvent(ev);
    return false;
  }

bool doHandleKey(int k) override
{
  // ESC closes app only when no modal is open
  if (k == 27)
  {
    if (_modalPopup == ModalMode::None)
    {
      _confirmUi.set(L"Quit", [this]{this->_isRunning = false;});
      _modalPopup=ModalMode::Confirm;
      return true;
    }
  }
  // exit modals
  if (_modalPopup != ModalMode::None)
  {
    if (k == 27) // ESC
    {
      _modalPopup = ModalMode::None;
      return true;
    }
  }

  // Modal dispatch
  if (_modalPopup == ModalMode::Confirm)
  return _confirmUi.handleKey(k);

  if (_modalPopup == ModalMode::AddEditPodcast)
  {
    return _addEditPodcastUi.handleKey(k);
  }

  // Focus cycling order
  static const std::array<Focus,3> focus_order{
    Focus::Podcasts,
    Focus::Status,
    Focus::Shows
  };

  auto status_cycle = [&](int amount)
  {
    auto it = std::find(focus_order.begin(), focus_order.end(), _focusedPanel);
    int idx = std::distance(focus_order.begin(), it);
    idx = (idx + amount + focus_order.size()) % focus_order.size();
    return focus_order[idx];
  };

  // TAB → forward
  if (k == 9)
  {
    _focusedPanel = status_cycle(1);
    return true;
  }

  // SHIFT+TAB
  if (k == KEY_BTAB)
  {
    _focusedPanel = status_cycle(-1);
    return true;
  }

  // Auto switching windows
  if (k == KEY_RIGHT)
  {
    if (_focusedPanel == Focus::Podcasts)
    {
      if (_podcastUi.cursor() < 2)
      _focusedPanel = Focus::Status;
      else
      _focusedPanel = Focus::Shows;
      return true;
    }
  }

  if (k == KEY_LEFT)
  {
    if (_focusedPanel == Focus::Shows &&
      _showsUi.dynamicColCurrentOffsetX() == 0)
    {
      _focusedPanel = Focus::Podcasts;
      return true;
    }
    if (_focusedPanel == Focus::Status &&
      _statusUi.leftMostStatus())
      {
        _focusedPanel = Focus::Podcasts;
        return true;
      }
    }

    if (k == KEY_DOWN)
    {
      if (_focusedPanel == Focus::Status)
      {
        _focusedPanel = Focus::Shows;
        return true;
      }
    }

    if (k == KEY_UP)
    {
      if (_focusedPanel == Focus::Shows &&
        _showsUi.cursor() == 0)
      {
        _focusedPanel = Focus::Status;
        return true;
      }
    }

    // Normal dispatch
    switch (_focusedPanel)
    {
      case Focus::Podcasts:
      return _podcastUi.handleKey(k);
      case Focus::Status:
      return _statusUi.handleKey(k);
      case Focus::Shows:
      if (_showsUi.handleKey(k))
      {
        return true;
      }
      if (k == 'i' || k == 'I')
      {
        _modalPopup = ModalMode::Info;
        return true;
      }
      return false;
    }

    return false;
  }
  // --------------------------------------------------------------------
  void doDelWindows() override
  {
    _podcastUi.delWindow();
    _showsUi.delWindow();
    _actionsUi.delWindow();
    _bottomBarUi.delWindow();
    _statusUi.delWindow();
    _infoUi.delWindow();
    _confirmUi.delWindow();
  }
  void doBuildWindows(int h, int w) override
  {
    int left_w = std::max(20, w / 4);
    int right_w = w - left_w;

    int min_shows_h = 3;
    int status_h = 3;
    int actions_h = 3;
    int bottom_h = 3;


    status_h = std::max(min_shows_h, status_h);
    actions_h = std::max(min_shows_h, actions_h);
    bottom_h = std::max(min_shows_h, bottom_h);

    int shows_h = std::max(min_shows_h, h - status_h - actions_h - bottom_h);

    _podcastUi.buildWindow(h,        left_w, 0,                 0);
    _statusUi.buildWindow(status_h, right_w, 0,                 left_w);
    _showsUi.buildWindow(shows_h,  right_w, status_h,          left_w);
    _actionsUi.buildWindow(actions_h,right_w, status_h + shows_h,left_w);
    _bottomBarUi.buildWindow(bottom_h, right_w, status_h + shows_h + actions_h, left_w);

    {
      int mh = 3;
      int mw = std::min(w - 4, 70);
      int y  = (h - mh) / 2;
      int x  = (w - mw) / 2;
      _confirmUi.buildWindow(mh, mw, y, x);
    }

    {
      int mh = std::min(h - 4, 20);
      int mw = std::min(w - 4, 70);
      int y  = (h - mh) / 2;
      int x  = (w - mw) / 2;
      _addEditPodcastUi.buildWindow(mh, mw, y, x);
    }
  }
  void doRender(int k) override
  {
    _podcastUi.render(k, _focusedPanel == Focus::Podcasts);
    _statusUi.render(k, _focusedPanel == Focus::Status);
    _showsUi.render(k, _focusedPanel == Focus::Shows);
    renderActionsBar(k);
    renderBottomBar(k);

    if (_modalPopup == ModalMode::Confirm)
    {
      _confirmUi.render(k);
    }
    else if (_modalPopup == ModalMode::Info)
    {
      renderInfoModal(k);
    }
    else if (_modalPopup == ModalMode::AddEditPodcast)
    {
      _addEditPodcastUi.render(k);
    }

  }
  // --------------------------------------------------------------------

  DowncastLogic _logic;

  Focus _focusedPanel;
  UiPodcastTable _podcastUi;
  UiShowsTable _showsUi;
  UiTable _actionsUi;
  UiTable _bottomBarUi;
  UiStatusTable _statusUi;
  UiPodcastSetup _addEditPodcastUi;
  UiConfirm _confirmUi;
  UiTable _infoUi;

  ModalMode _modalPopup = ModalMode::None;

};

// --------------------------------------------------------------------
int main()
{

  Ui ui("castapod.db3");
  ui.run();
  return 0;
}
