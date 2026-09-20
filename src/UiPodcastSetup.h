#pragma once

#include "UiFieldEditor.h"
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

// Class for cleanness but instantiated once, no need to have a separate compilation unit, compiled once anyway
class UiPodcastSetup : public UiTable
{
public:
  UiPodcastSetup(DowncastLogic& logic, std::function<void()> const& doneCallback);

  void setAdd();
  void setEdit(PodcastCols const&p);

  void render(UiInput const& input);

  bool handleKey(UiInput const& input);

private:
  void startEdit(int caretPos);
  void savePodcast();

  DowncastLogic& _logic;

  enum class Mode { AddPodcast, EditPodcast};
  Mode _mode = Mode::AddPodcast;
  int _field = 0;
  UiFieldEditor _editor;        // the ONLY active _editor
  std::string _url;
  std::string _title;
  std::string _target = ".";
  std::string _pattern = "{date}-{title}";
  std::string _preview_description;
  std::vector<std::string> _preview_shows;
  std::optional<int> _edit_id;
  std::function<void()> _doneCallback;
  std::array<std::string*, 4> _fields;
};
