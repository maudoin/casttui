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
  int modal_field = 0;
  UiFieldEditor editor;        // the ONLY active editor
  std::string modal_url;
  std::string modal_title;
  std::string modal_target = ".";
  std::string modal_pattern = "{date}-{title}";
  std::string modal_preview_description;
  std::vector<std::string> modal_preview_shows;
  std::optional<int> modal_edit_id;
  std::function<void()> _doneCallback;
};
