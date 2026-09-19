#include "UiPodcastSetup.h"

#include "UiColors.h"
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
}

void UiPodcastSetup::render(UiInput const& input)
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
  Columns cols = UiTable::renderHeader(input, {20,HeaderColumn::FILL}, UiColors::focusedStyle());

  auto cellCallback = [=, this](int row, int col, std::optional<UiInput::MouseEvent> const& ev) -> Cell
  {
    if (row == inner_h - 1)
    {
      if (col == 0)
      {
        if (ev){savePodcast();}
        return Cell{.text=_mode == Mode::AddPodcast ? L"Add podcast (s)" : L"Save podcast (s)", .style=UiColors::boldStyle()};
      }
      if (col == 1)
      {
        if (ev){_doneCallback();}
        return Cell{.text=L"Cancel (Esc)", .style=UiColors::boldStyle()};
      }
    }

    auto setRowNoEdit = [this, row]{
        modal_field = row;
        this->editor.cancel();};
    // Field rows
    if (row >= 0 && row < fieldCount)
    {
      auto const& f = fields[row];

      bool isCurrentField = (row == modal_field);
      bool editing  = (this->editor.editing && isCurrentField);

      if (col == 0)
      {
        int style = UiColors::highlightStyle(isCurrentField && !editing);
        if (ev){setRowNoEdit();}
        return Cell{.text=f.label, .style=style};
      }
      else
      {
        std::wstring displayValue;

        if (editor.editing && modal_field == row)
            displayValue = to_wstring(editor.display(cols.vec[1].width));
        else
            displayValue = f.value;

        int style = UiColors::highlightStyle(isCurrentField && editing);
        if (!f.editable)
        {
          if (ev){setRowNoEdit();}
          return Cell{.text=displayValue, .style=style};
        }
        if (ev)
        {
          modal_field = row;
          startEdit(ev->x - cols.vec[col].start);
        }
        return Cell{.text=displayValue, .style=style};
      }
    }
    // Preview rows
    if (row >= fieldCount)
    {
      int preview = row-fieldCount;
      if (col==0 && preview==0)
      {
        if (ev){setRowNoEdit();}
        return Cell{.text=L"Preview", .style=UiColors::boldStyle()};
      }
      if (col==1 && preview < modal_preview_shows.size())
      {
        if (ev){setRowNoEdit();}
        return Cell{.text=to_wstring(modal_preview_shows[preview]), .style=UiColors::normalStyle()};
      }
    }

    // Footer row
    return Cell{};
  };

  UiTable::render(input, inner_h, cols, cellCallback, UiColors::focusedStyle());
}

bool UiPodcastSetup::handleKey(UiInput const& input)
{
  if (editor.handleKey(input))
  {
    return true;
  }
  if (editor.editing)
  {
      if (input.key == 10 || input.key == 13)
      {
        switch (modal_field)
        {
          case 0:
          {
            editor.commitTo(modal_url);
            auto [pod, shows] = _logic.queryPodcast(modal_url);
            if (pod)
            {
              modal_title = pod->title;
              modal_preview_description = pod->summary;
              modal_preview_shows = shows;
            }
            break;
          }
          case 1: editor.commitTo(modal_title); break;
          case 2: editor.commitTo(modal_target); break;
          case 3: editor.commitTo(modal_pattern); break;
        }
        return true;
      }
      return true;
  }

  // Navigation between fields
  if (input.keyUp())
  {
    modal_field = std::max(0, modal_field - 1);
    return true;
  }
  if (input.keyDown())
  {
    modal_field = std::min(3, modal_field + 1);
    return true;
  }
  if (modal_field == 4 && (input.keyLeft() || input.keyRight()))
  {
    return UiTable::handleKey(input);
  }

  // ENTER begins editing
  if (input.key == 10 || input.key == 13)
  {
    startEdit(0);
    return true;
  }

  // Save podcast
  if (input.key == 's' || input.key == 'S')
  {
    savePodcast();
    return true;
  }

  return false;
}

void UiPodcastSetup::startEdit(int caretPos)
{
  switch (modal_field)
  {
    case 0: editor.begin(modal_url, caretPos); break;
    case 1: editor.begin(modal_title, caretPos); break;
    case 2: editor.begin(modal_target, caretPos); break;
    case 3: editor.begin(modal_pattern, caretPos); break;
  }
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