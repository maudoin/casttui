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
, _fields({{&_url, &_title, &_target, &_pattern}})
, _fileBrowser([this]{this->_showFileBrowser=false;this->_fileBrowser.delWindow();this->_target=this->_fileBrowser.getSelected().string();})
{
}

void UiPodcastSetup::setAdd()
{
  _edit_id.reset();

  _url = "";
  _title = "";
  _target = ".";
  _pattern = "{date}-{title}";
  _preview_description = "";
  _preview_shows.clear();

  _mode = Mode::AddPodcast;
  _field = 0;

  if (_showFileBrowser)_fileBrowser.delWindow();
  _showFileBrowser=false;
}

void UiPodcastSetup::setEdit(PodcastCols const &p)
{
  _edit_id = p.id;

  _url = p.link;
  _title = p.title;
  _target = p.target;
  _pattern = p.pattern;
  _preview_description = p.summary;
  _preview_shows.clear();

  _mode = Mode::EditPodcast;
  _field = 0;

  if (_showFileBrowser)_fileBrowser.delWindow();
  _showFileBrowser=false;
}

void UiPodcastSetup::render(UiInput const& input)
{
  if (_showFileBrowser)
  {
    _fileBrowser.render(input, true, []{});
    return;
  }
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
    Field{.label=L"URL",         .value=to_wstring(_url)},
    Field{.label=L"Title",       .value=to_wstring(_title)},
    Field{.label=L"Target",      .value=to_wstring(_target)},
    Field{.label=L"Pattern",     .value=to_wstring(_pattern)},
    Field{.label=L"Description", .value=to_wstring(_preview_description), .editable=false}
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
        _field = row;
        this->_editor.cancel();};
    // Field rows
    if (row >= 0 && row < fieldCount)
    {
      auto const& f = fields[row];

      bool isCurrentField = (row == _field);
      bool editing  = (this->_editor.editing && isCurrentField);

      if (col == 0)
      {
        int style = UiColors::highlightStyle(isCurrentField && !editing);
        if (ev)
        {
          if (_fields[_field] == &_target)
          {
            _showFileBrowser = true;
            {
              int mh = std::min(input.height - 4, 20);
              int mw = std::min(input.width - 4, 70);
              int y  = (input.height - mh) / 2;
              int x  = (input.width - mw) / 2;
              _fileBrowser.buildWindow(mh, mw, y, x);
            }
          }
          setRowNoEdit();
        }
        return Cell{.text=f.label, .style=style};
      }
      else
      {
        std::wstring displayValue;

        if (_editor.editing && _field == row)
            displayValue = to_wstring(_editor.display(cols.vec[1].width));
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
          _field = row;
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
      if (col==1 && preview < _preview_shows.size())
      {
        if (ev){setRowNoEdit();}
        return Cell{.text=to_wstring(_preview_shows[preview]), .style=UiColors::normalStyle()};
      }
    }

    // Footer row
    return Cell{};
  };

  UiTable::render(input, inner_h, cols, cellCallback, UiColors::focusedStyle());
}

bool UiPodcastSetup::handleKey(UiInput const& input)
{
  if (_showFileBrowser)
  {
    return _fileBrowser.handleKey(input);
  }
  if (_editor.handleKey(input))
  {
    return true;
  }
  if (_editor.editing)
  {
      if (input.keyEnterReturn())
      {
        _editor.commitTo(*_fields[_field]);
        if (_fields[_field] == &_url)
        {
          auto [pod, shows] = _logic.queryPodcast(_url);
          if (pod)
          {
            _title = pod->title;
            _preview_description = pod->summary;
            _preview_shows = shows;
          }
        }
        return true;
      }
      return true;
  }

  // Navigation between fields
  if (input.keyUp())
  {
    _field = std::max(0, _field - 1);
    return true;
  }
  if (input.keyDown())
  {
    _field = std::min(3, _field + 1);
    return true;
  }
  if (_field == 4 && (input.keyLeft() || input.keyRight()))
  {
    return UiTable::handleKey(input);
  }

  if (input.keyEnterReturn())
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
  _editor.begin(*_fields[_field], caretPos);
}

void UiPodcastSetup::savePodcast()
{
  PodcastCols p;
  p.id = (_mode == Mode::AddPodcast ? 0 : _edit_id.value_or(0));
  p.title = (_title.empty() ? _url : _title);
  p.pattern = _pattern;
  p.link = _url;
  p.summary = _preview_description;
  p.image_url = "";
  p.target = _target;

  _logic.insertPodcast(p,
    _mode == Mode::AddPodcast
    ? DowncastLogic::InsertPodcastMode::ADD
    : DowncastLogic::InsertPodcastMode::UPDATE);

  _doneCallback();
}

void UiPodcastSetup::delWindow()
{
  if (_showFileBrowser)
  {
    _fileBrowser.delWindow();
  }
  UiTable::delWindow();
}

void UiPodcastSetup::buildWindow(int nlines, int ncols, int begy, int begx)
{
  if (_showFileBrowser)
  {
    int mh = std::min(nlines - 4, 20);
    int mw = std::min(ncols - 4, 70);
    int y  = (nlines - mh) / 2;
    int x  = (ncols - mw) / 2;
    _fileBrowser.buildWindow(mh, mw, begy+y, begx+x);
  }
  UiTable::buildWindow(nlines, ncols, begy, begx);
}