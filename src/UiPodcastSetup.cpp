#include "UiPodcastSetup.h"

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

UiPodcastSetup::UiPodcastSetup(DowncastLogic &_logic, std::function<void()> const &doneCallback)
: UiTable(UiTable::Mode::CURSOR, []{})
, _logic(_logic)
, _doneCallback(doneCallback)
{
}

void UiPodcastSetup::setAdd()
{
  modal_edit_id.reset();

  modal_url = "";
  modal_title = "";
  modal_target = ".";
  modal_pattern = "{date}-{title}";
  modal_preview_description = "";
  modal_preview_shows.clear();

  _mode = Mode::AddPodcast;
  modal_field = 0;
  modal_editing = false;
  modal_edit_buffer = "";
}

void UiPodcastSetup::setEdit(PodcastCols const &p)
{
  modal_edit_id = p.id;

  modal_url = p.link;
  modal_title = p.title;
  modal_target = p.target;
  modal_pattern = p.pattern;
  modal_preview_description = p.summary;
  modal_preview_shows.clear();

  _mode = Mode::EditPodcast;
  modal_field = 0;
  modal_editing = false;
  modal_edit_buffer = "";
}

void UiPodcastSetup::render()
{
  int h = getHeight();
  int inner_h = h - 2;

  // Rows:
  // 0: caption ("add podcast" / "edit podcast")
  // 1..N: fields
  // last-1: footer buttons

  struct Field
  {
    std::wstring label;
    std::wstring value;
    bool editable = true;
  };
  std::array<Field, 5> fields{{
    Field{.label=L"URL",         .value=to_wstring(modal_url)},
    Field{.label=L"Title",       .value=to_wstring(modal_title)},
    Field{.label=L"Target",      .value=to_wstring(modal_target)},
    Field{.label=L"Pattern",     .value=to_wstring(modal_pattern)},
    Field{.label=L"Description", .value=to_wstring(modal_preview_description), .editable=false}
  }};

  int fieldCount = static_cast<int>(fields.size());

  // Columns: Label | Value
  std::vector<HeaderColumn> cols{
    HeaderColumn{.width = 20, .dynamic = false},
    HeaderColumn{.width = -1, .dynamic = true},
  };

  auto cell_cb = [=, this](int row, int col) -> Cell
  {
    if (row == inner_h - 1)
    {
      if (col == 0)
        return Cell{.text=_mode == Mode::AddPodcast ? L"Add podcast (s)" : L"Save podcast (s)", .style=A_BOLD, .callback=[this]{this->savePodcast();}};
      if (col == 1)
        return Cell{.text=L"Cancel (Esc)", .style=A_BOLD, .callback=_doneCallback};
    }

    auto setRowNoEdit = [this, row]{
        modal_field = row;
        modal_edit_buffer.clear();
        modal_editing = false;
      };
    // Field rows
    if (row >= 0 && row < fieldCount)
    {
      auto const& f = fields[row];

      bool isCurrentField = (row == modal_field);
      bool editing  = (modal_editing && isCurrentField);

      if (col == 0)
      {
        int style = (isCurrentField && !editing) ? COLOR_PAIR(1) : A_NORMAL;
        return Cell{.text=f.label, .style=style, .callback=setRowNoEdit};
      }
      else
      {
        std::wstring displayValue =
          editing ? to_wstring(modal_edit_buffer) + L"_" : f.value;
        int style = (isCurrentField && editing) ? COLOR_PAIR(1) : A_NORMAL;
        if (!f.editable)
          return Cell{.text=displayValue, .style=style, .callback=setRowNoEdit};
        return Cell{.text=displayValue, .style=style, .callback=[this, row, displayValue]{
          modal_field = row;
          startEdit();
        }};
      }
    }
    // Preview rows
    if (row >= fieldCount)
    {
      int preview = row-fieldCount;
      if (col==0 && preview==0)
      {
        return Cell{.text=L"Preview", .style=A_BOLD, .callback=setRowNoEdit};
      }
      if (col==1 && preview < modal_preview_shows.size())
      {
        return Cell{.text=to_wstring(modal_preview_shows[preview]), .style=A_NORMAL, .callback=setRowNoEdit};
      }
    }

    // Footer row
    return Cell{L"", A_NORMAL};
  };

  UiTable::render(inner_h, cols, cell_cb, true);
}

bool UiPodcastSetup::handleKey(int k)
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
        {
          modal_url = modal_edit_buffer;
          auto [pod, shows] = _logic.queryPodcast(modal_url);
          if (pod)
          {
            modal_title = pod->title;
            modal_preview_description = pod->summary;
            modal_preview_shows = shows;
          }
          break;
        }
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
  if (modal_field == 4 && (k == KEY_LEFT || k == KEY_RIGHT))
  {
    return UiTable::handleKeyCh(k);
  }

  // ENTER begins editing
  if (k == 10 || k == 13)
  {
    startEdit();
    return true;
  }

  // Save podcast
  if (k == 's' || k == 'S')
  {
    savePodcast();
    return true;
  }

  return false;
}

void UiPodcastSetup::startEdit()
{
  switch (modal_field)
  {
    case 0:
      modal_edit_buffer = modal_url;
      break;
    case 1:
      modal_edit_buffer = modal_title;
      break;
    case 2:
      modal_edit_buffer = modal_target;
      break;
    case 3:
      modal_edit_buffer = modal_pattern;
      break;
  }
  modal_editing = true;
}

void UiPodcastSetup::savePodcast()
{
  PodcastCols p;
  p.id = (_mode == Mode::AddPodcast ? 0 : modal_edit_id.value_or(0));
  p.title = (modal_title.empty() ? modal_url : modal_title);
  p.pattern = modal_pattern;
  p.link = modal_url;
  p.summary = modal_preview_description;
  p.image_url = "";
  p.target = modal_target;

  _logic.insertPodcast(p,
    _mode == Mode::AddPodcast
    ? DowncastLogic::InsertPodcastMode::ADD
    : DowncastLogic::InsertPodcastMode::UPDATE);

    _doneCallback();
  }