#pragma once

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

class UiPodcastSetup : public UiTable
{
public:
  UiPodcastSetup(DowncastLogic& logic, std::function<void()> const& doneCallback);

  void setAdd();
  void setEdit(PodcastCols const&p);

  void render();

  bool handleKey(int k);

private:
  void startEdit();
  void savePodcast();

  DowncastLogic& _logic;

  enum class Mode { AddPodcast, EditPodcast};
  Mode _mode = Mode::AddPodcast;
  int modal_field = 0;
  bool modal_editing = false;
  std::string modal_url;
  std::string modal_title;
  std::string modal_target = ".";
  std::string modal_pattern = "{date}-{title}";
  std::string modal_edit_buffer;
  std::string modal_preview_description;
  std::vector<std::string> modal_preview_shows;
  std::optional<int> modal_edit_id;
  std::function<void()> _doneCallback;
};
