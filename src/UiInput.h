#pragma once

#include "MouseEvent.h"

#include <optional>

struct UiInput
{
  int key = ERR;
  std::optional<MouseEvent> mev;
  int height, width;
};
