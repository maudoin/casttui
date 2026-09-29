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
  using FilterFn = std::function<bool(const std::filesystem::directory_entry&)>;

  UiFileBrowser(std::wstring const& title, std::function<void(bool)> const& doneCallback, bool multiSelection = false, FilterFn filter = {}, bool skipFileInfo = false);

  void render(UiInput const& input, bool focused, std::function<void()> const& winSelection);

  bool handleKey(UiInput const& input);
  void setPwd(std::filesystem::path const& p);
  const std::filesystem::path& pwd() const { return _pwd; }
  bool hasSelected() const { return !_selected.empty(); }

  std::filesystem::path getSelected() const;

  std::vector<std::filesystem::path> getMultiSelected() const;

  void clearSelected();

  bool ok() const;
  void resetOk();

private:

  void toggleSelect(std::filesystem::path const& p);

  void activate(int idx);
  const std::vector<std::filesystem::directory_entry>& records() const { return _records; }
  void updateRecords();

private:
  std::wstring _title;
  std::filesystem::path _pwd;
  std::vector<std::filesystem::directory_entry> _records;
  bool _multiSelection = false;
  std::set<std::filesystem::path> _selected;
  bool _ok = false;
  std::function<void(bool)> _doneCallback;
  FilterFn _filter;
  bool _skipFileInfo;
};
