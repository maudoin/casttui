#include "UiFileBrowser.h"

#include "UiTable.h"
#include "UiColors.h"

#if defined(_WIN32)
#include <windows.h>
#endif

#include <filesystem>
#include <format>
#include <optional>
#include <set>
#include <string>
#include <vector>

UiFileBrowser::UiFileBrowser(std::wstring const& title, std::function<void(bool)> const& doneCallback, bool multiSelection, FilterFn filter, bool skipFileInfo)
: UiTable(UiTable::Mode::CURSOR)
, _title(title)
, _multiSelection(multiSelection)
, _doneCallback(doneCallback)
, _filter(std::move(filter))
, _skipFileInfo(skipFileInfo)
{
  setPwd(std::filesystem::current_path());
}

namespace
{
  std::wstring strSize(std::filesystem::directory_entry const& e)
  {
    if (!e.is_regular_file())
      return L"";
    static const wchar_t* u[] = {L"B", L"KB", L"MB", L"GB", L"TB"};
    auto size = e.file_size();
    int i = 0;
    for (; size >= 1024 && i < 4; i++) size /= 1024;
    return std::format(L"{:d} {}", size, u[i]);
  }
}
void UiFileBrowser::render(UiInput const& input, bool focused, std::function<void()> const& winSelection)
{
  Columns titleCols = UiTable::renderHeader(input, std::array{
    HeaderColumn{.width = HeaderColumn::FILL, .name = _title},
    HeaderColumn{.width = HeaderColumn::FIT_LABEL, .name = L" X ", .callback=[this]{this->_doneCallback(false);}},
  }, UiColors::focusStyle(focused));
  Columns currentCols = UiTable::renderHeader(input, std::array{
    HeaderColumn{.width = HeaderColumn::FILL, .name = _pwd.wstring()},
    HeaderColumn{.width = HeaderColumn::FIT_LABEL, .name = L" ▲ ", .callback=[this]{this->up();}},
  }, UiColors::focusStyle(focused), titleCols);
  Columns cols =
    _skipFileInfo
    ? UiTable::renderHeader(input, std::array{HeaderColumn{.width = HeaderColumn::FILL}}, UiColors::focusStyle(focused), currentCols)
    : UiTable::renderHeader(input, std::array{
    HeaderColumn{.width = HeaderColumn::FILL, .name = L"Name"},
    HeaderColumn{.width = 6, .name = L"Type"},
    HeaderColumn{.width = HeaderColumn::FIT_LABEL, .name = L"Ext"},
    HeaderColumn{.width = HeaderColumn::FIT_LABEL, .name = L"Size"},
  }, UiColors::focusStyle(focused), currentCols);

  auto cellCallback = [&](int row, int col, std::optional<UiInput::MouseEvent> const& ev) -> Cell
  {
    if (row < 0 || row >= static_cast<int>(_records.size()))
      return Cell{};

    auto const& r = _records[row];

    std::wstring text;
    if (col == 0) text = r.path().filename().empty()?r.path().wstring() : r.path().filename().wstring();
    else if (col == 1) text = r.is_directory() ? L"Folder" : L"File";
    else if (col == 2) text = r.path().extension().wstring();
    else               text = strSize(r);

    bool isSelected = _selected.count(r) > 0;
    bool isCursor   = (row == cursor() && focused);

    if (col == 0 && ev)
    {
      if (ev->leftDouble)
      {
        activate(row);
        // otherwise table will mix prev rows from prev path with next path rows
        forceRefresh();
      }
      else
      {
        scrollTo(row);
        toggleSelect(r.path());
      }
    }

    return Cell{text, UiColors::getStyle(isCursor, isSelected)};
  };

  Columns tableCols = UiTable::render(input, _records.size(), cols, cellCallback, UiColors::focusStyle(focused), winSelection, RowReserve(2));

  UiTable::renderHeaderOnly(input, std::array{
    HeaderColumn{.width = HeaderColumn::FILL, .name = L"OK", .callback = [this]{this->_doneCallback(true);}},
    HeaderColumn{.width = HeaderColumn::FIT_LABEL, .name = L"Cancel", .callback = [this]{this->_doneCallback(false);}},
  }, UiColors::focusStyle(focused), tableCols);
}
bool UiFileBrowser::handleKey(UiInput const& input)
{
  if (input.keyEsc())
  {
    _doneCallback(false);
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
      toggleSelect(_records[idx]);
    return true;
  }

  return false;
}

void UiFileBrowser::up()
{
#if defined(_WIN32)
  static std::filesystem::path ROOT_MSG = L"Select drive:";
  if (_pwd == _pwd.root_path() || _pwd == ROOT_MSG)
  {
    _pwd = ROOT_MSG;
    _records.clear();
    DWORD mask = GetLogicalDrives();
    for (char letter = 'A'; letter <= 'Z'; ++letter)
    {
      if (mask & (1u << (letter - 'A')))
      {
        _records.emplace_back( std::filesystem::path(std::wstring(1, letter) + L":\\"));
      }
    }
  }
  else
#endif
  {
    setPwd(_pwd.parent_path());
  }
}

void UiFileBrowser::setPwd(std::filesystem::path const& p)
{
  _pwd = std::filesystem::absolute(p);
  updateRecords();
  _selected.clear();
}

std::filesystem::path UiFileBrowser::getSelected() const
{
  if (_selected.empty())
    return _pwd;

  return _pwd / *_selected.begin();
}

std::vector<std::filesystem::path> UiFileBrowser::getMultiSelected() const
{
  if (_selected.empty())
    return { _pwd };

  std::vector<std::filesystem::path> out;
  for (auto const& s : _selected)
    out.push_back(_pwd / s);
  return out;
}

void UiFileBrowser::clearSelected()
{
  _selected.clear();
}

void UiFileBrowser::toggleSelect(std::filesystem::path const& p)
{
  if (_multiSelection)
  {
    if (_selected.count(p))
      _selected.erase(p);
    else
      _selected.insert(p);
  }
  else
  {
    _selected.clear();
    _selected.insert(p);
  }
}

void UiFileBrowser::activate(int idx)
{
  if (idx < 0 || idx >= static_cast<int>(_records.size()))
    return;

  auto const& r = _records[idx];
  if (r.is_directory() && r.exists())
  {
    setPwd(r.path());
  }
  else
  {
    _selected = { r.path().filename() };
    _ok = true;
  }
}
void UiFileBrowser::updateRecords()
{
  _records.clear();

  for (auto const& p : std::filesystem::directory_iterator(_pwd))
  {
    if (_filter && !_filter(p))
      continue;

    _records.emplace_back(p);
  }

  std::sort(_records.begin(), _records.end(),
    [](auto const& a, auto const& b)
    {
      if (a.is_directory() != b.is_directory())
        return a.is_directory() > b.is_directory();
      return a.path().string() < b.path().string();
    });
}
