#pragma once

#include <optional>

struct UiInput
{
  struct MouseEvent
  {
    int x;
    int y;
    bool left;
    bool right;
    bool middle;
    bool prev=0;//not under windows
    bool next=0;//not under windows
    bool ctrl;
    bool shift;
    bool alt;

    struct Loc{ int beginRow=0, beginCol=0, endRow=0, endCol=0; };
    bool hit(Loc const& loc) const;
  };

  static UiInput init(int);

  int key = -1;
  std::optional<MouseEvent> mev;
  int height, width;
};
