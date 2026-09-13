#pragma once

#include "UiTable.h"
#include "DowncastLogic.h"

#include <array>
#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <ctime>
#include <ranges>

class UiStatusTable : public UiTable
{
public:
  UiStatusTable(DowncastLogic& logic, std::function<void()> const& winSelection, std::function<void()> const& exit);

  void render(bool focused);

  bool handleKey(int k);

  bool leftMostStatus() const{return _cursorPosition==0;}
private:
  struct StatusLabel
  {
    std::wstring label;
    std::optional<DowncastLogic::MediaStatus> status;
  };
  DowncastLogic& _logic;
  int _cursorPosition;
  std::function<void()> _exit;
  std::vector<StatusLabel> _labels;

};
