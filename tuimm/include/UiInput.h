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

  bool keyLeft() const;
  bool keyRight() const;
  bool keyUp() const;
  bool keyDown() const;
  bool keyBackSpace() const;
  bool keyDel() const;
  bool keyTab() const;
  bool keyBackTab() const;
  bool keyEnterReturn() const;
  bool keySpace() const;
  bool keyEsc() const;
  bool keyPlus() const;
  bool keyMinus() const;
  bool keyPageUp() const;
  bool keyPageDown() const;
  bool keyHome() const;
  bool keyEnd() const;
};
