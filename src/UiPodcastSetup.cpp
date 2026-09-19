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
        int style = UiColors::highlightStyle(isCurrentField && !editing);
        if (ev){setRowNoEdit();}
        return Cell{.text=f.label, .style=style};
      }
      else
      {
        //std::wstring displayValue =
        //  editing ? to_wstring(modal_edit_buffer) + L"_" : f.value;
        std::wstring displayValue;
        if (editing)
        {
          std::wstring w = to_wstring(modal_edit_buffer);

          int cellWidth = cols.vec[1].width;   // UiTable gives you this
          int scroll = 0;
          if (modal_caret >= cellWidth)
              scroll = modal_caret - cellWidth + 1;

          std::wstring slice = w.substr(scroll, cellWidth);

          int caretPos = modal_caret - scroll;
          if (caretPos >= 0 && caretPos <= (int)slice.size())
              slice.insert(caretPos, L"_");

          displayValue = slice;
        } else {
          displayValue = f.value;
        }
        int style = UiColors::highlightStyle(isCurrentField && editing);
        if (!f.editable)
        {
          if (ev){setRowNoEdit();}
          return Cell{.text=displayValue, .style=style};
        }
        if (ev)
        {
          modal_field = row;
          startEdit();
          // Move caret to mouse X
          if (modal_editing) {
            int rel = ev->x - cols.vec[col].start;   // you already have cell start X in UiTable
            rel = std::max(0, rel);
            rel = std::min(rel, (int)modal_edit_buffer.size());
            modal_caret = rel;
          }
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

  // Editing _mode
  if (modal_editing)
  {
    if (input.keyLeft()) {
      modal_caret = std::max(0, modal_caret - 1);
      return true;
    }
    else if (input.keyRight()) {
      modal_caret = std::min((int)modal_edit_buffer.size(), modal_caret + 1);
      return true;
    }
    else if (input.keyBackSpace() || input.key == 127) {
      if (modal_caret > 0) {
        modal_edit_buffer.erase(modal_caret - 1, 1);
        modal_caret--;
      }
      return true;
    }
    // if (input.key == KEY_BACKSPACE || input.key == 127)
    // {
    //   if (!modal_edit_buffer.empty())
    //   modal_edit_buffer.pop_back();
    // }
    else if (input.key == 10 || input.key == 13) // ENTER commits edit
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
    // else
    // {
    //   if (k >= 32 && k <= 126)
    //   modal_edit_buffer.push_back(static_cast<char>(k));
    // }
    if (input.key >= 32 && input.key <= 126)
    {
      modal_edit_buffer.insert(modal_edit_buffer.begin() + modal_caret,
                               static_cast<char>(input.key));
      modal_caret++;
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
    startEdit();
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
  modal_caret = (int)modal_edit_buffer.size();   // caret at end
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