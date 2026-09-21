#pragma once

#include "UiTable.h"
#include "UiColors.h"

#include <filesystem>
#include <set>
#include <string>
#include <vector>
#include <optional>

class UiFileBrowser : public UiTable
{
public:
  struct FileRecord
  {
    bool isDir = false;
    std::filesystem::path name;
    std::wstring showName;
    std::filesystem::path extension;
  };

  UiFileBrowser(std::wstring const& title, std::function<void()> const& doneCallback)
  : UiTable(UiTable::Mode::CURSOR)
  , _title(title)
  , _doneCallback(doneCallback)
  {
    setPwd(std::filesystem::current_path());
  }

  // ---------------------------
  // Rendering
  // ---------------------------
  void render(UiInput const& input, bool focused, std::function<void()> const& winSelection)
  {
    Columns titleCols = UiTable::renderHeader(input, {
      HeaderColumn{.width = HeaderColumn::FILL, .name = _title},
      HeaderColumn{.width = HeaderColumn::FIT_LABEL, .name = L" X ", .callback=_doneCallback},
    }, UiColors::focusStyle(focused));
    Columns cols = UiTable::renderHeader(input, {
      HeaderColumn{.width = HeaderColumn::FILL, .name = L"Name"},
      HeaderColumn{.width = 10, .name = L"Type"},
      HeaderColumn{.width = 10, .name = L"Ext"},
    }, UiColors::focusStyle(focused), titleCols);

    auto cellCallback = [&](int row, int col, std::optional<UiInput::MouseEvent> const& ev) -> Cell
    {
      if (row < 0 || row >= static_cast<int>(_records.size()))
        return Cell{};

      auto const& r = _records[row];

      std::wstring text;
      if (col == 0) text = r.showName;
      else if (col == 1) text = r.isDir ? L"Directory" : L"File";
      else               text = r.extension.wstring();

      bool isSelected = _selected.count(r.name) > 0;
      bool isCursor   = (row == cursor() && focused);

      if (ev)
      {
        scrollTo(row);
        toggleSelect(r.name);
      }

      return Cell{text, UiColors::getStyle(isCursor, isSelected)};
    };

    Columns tableCols = UiTable::render(input, _records.size(), cols, cellCallback, UiColors::focusStyle(focused), winSelection, RowReserve(2));

    Columns footerCols = UiTable::renderHeaderOnly(input, {
      HeaderColumn{.width = HeaderColumn::FILL, .name = L"OK"},
      HeaderColumn{.width = HeaderColumn::FIT_LABEL, .name = L"Cancel", .callback = _doneCallback},
    }, UiColors::focusStyle(focused), tableCols);
  }

  // ---------------------------
  // Keyboard handling
  // ---------------------------
  bool handleKey(UiInput const& input)
  {
    if (input.keyEsc())
    {
      _doneCallback();
      return true;
    }

    if (UiTable::handleKey(input))
      return true;

    if (input.keyEnterReturn())
    {
      activate(cursor() - firstVisibleDataRow());
      return true;
    }

    if (input.keySpace())
    {
      int idx = cursor() - firstVisibleDataRow();
      if (idx >= 0 && idx < static_cast<int>(_records.size()))
        toggleSelect(_records[idx].name);
      return true;
    }

    return false;
  }

  // ---------------------------
  // Logic
  // ---------------------------
  void setPwd(std::filesystem::path const& p)
  {
    _pwd = std::filesystem::absolute(p);
    updateRecords();
    _selected.clear();
  }

  const std::filesystem::path& pwd() const { return _pwd; }

  const std::vector<FileRecord>& records() const { return _records; }

  bool hasSelected() const { return !_selected.empty(); }

  std::filesystem::path getSelected() const
  {
    if (_selected.empty())
      return _pwd;

    return _pwd / *_selected.begin();
  }

  std::vector<std::filesystem::path> getMultiSelected() const
  {
    if (_selected.empty())
      return { _pwd };

    std::vector<std::filesystem::path> out;
    for (auto const& s : _selected)
      out.push_back(_pwd / s);
    return out;
  }

  void clearSelected()
  {
    _selected.clear();
  }

  void toggleSelect(std::filesystem::path const& p)
  {
    if (_selected.count(p))
      _selected.erase(p);
    else
      _selected.insert(p);
  }

  void activate(int idx)
  {
    if (idx < 0 || idx >= static_cast<int>(_records.size()))
      return;

    auto const& r = _records[idx];

    if (r.isDir)
    {
      if (r.name == "..")
        setPwd(_pwd.parent_path());
      else
        setPwd(_pwd / r.name);
    }
    else
    {
      _selected = { r.name };
      _ok = true;
    }
  }

  bool ok() const { return _ok; }
  void resetOk() { _ok = false; }

private:
  void updateRecords()
  {
    _records.clear();
    _records.push_back(FileRecord{
      true,
      "..",
      L"[D] ..",
      ""
    });

    for (auto const& p : std::filesystem::directory_iterator(_pwd))
    {
      FileRecord r;

      if (p.is_directory())
        r.isDir = true;
      else if (p.is_regular_file())
        r.isDir = false;
      else
        continue;

      r.name = p.path().filename();
      if (r.name.empty())
        continue;

      r.extension = p.path().filename().extension();

      r.showName = (r.isDir ? L"[D] " : L"[F] ") + p.path().filename().wstring();

      _records.push_back(r);
    }

    std::sort(_records.begin(), _records.end(),
      [](auto const& a, auto const& b)
      {
        if (a.isDir != b.isDir)
          return a.isDir > b.isDir;
        return a.name < b.name;
      });
  }

private:
  std::wstring _title;
  std::filesystem::path _pwd;
  std::vector<FileRecord> _records;
  std::set<std::filesystem::path> _selected;
  bool _ok = false;
  std::function<void()> _doneCallback;
};
