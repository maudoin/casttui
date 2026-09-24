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

  UiFileBrowser(std::wstring const& title, std::function<void()> const& doneCallback);

  void render(UiInput const& input, bool focused, std::function<void()> const& winSelection);

  bool handleKey(UiInput const& input);
  void setPwd(std::filesystem::path const& p);
  const std::filesystem::path& pwd() const { return _pwd; }
  const std::vector<FileRecord>& records() const { return _records; }
  bool hasSelected() const { return !_selected.empty(); }

  std::filesystem::path getSelected() const;

  std::vector<std::filesystem::path> getMultiSelected() const;

  void clearSelected();

  void toggleSelect(std::filesystem::path const& p);

  void activate(int idx);

  bool ok() const;
  void resetOk();

private:
  void updateRecords();

private:
  std::wstring _title;
  std::filesystem::path _pwd;
  std::vector<FileRecord> _records;
  std::set<std::filesystem::path> _selected;
  bool _ok = false;
  std::function<void()> _doneCallback;
};
