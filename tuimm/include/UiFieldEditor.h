#pragma once

#include "UiInput.h"

#include <string>

struct UiFieldEditor
{
public:
  bool editing = false;
  std::string buffer;
  int caret = 0;

  void begin(const std::string &initial, int caretPos)
  {
    buffer = initial;
    caret = std::max(0, std::min(caretPos, (int)buffer.size()));
    editing = true;
  }

  void commitTo(std::string &target)
  {
    target = buffer;
    cancel();
  }

  void cancel()
  {
    editing = false;
    buffer.clear();
    caret = 0;
  }

  void insertChar(char c)
  {
    buffer.insert(buffer.begin() + caret, c);
    caret++;
  }

  void backspace()
  {
    if (caret > 0)
    {
      buffer.erase(buffer.begin() + caret - 1);
      caret--;
    }
  }

  void moveLeft()
  {
    caret = std::max(0, caret - 1);
  }

  void moveRight()
  {
    caret = std::min((int)buffer.size(), caret + 1);
  }

  std::string display(int cellWidth) const
  {
    if (!editing)
      return ""; // caller uses committed value

    int scroll = 0;
    if (caret >= cellWidth)
      scroll = caret - cellWidth + 1;

    std::string slice = buffer.substr(scroll, cellWidth);

    int caretPos = caret - scroll;
    if (caretPos >= 0 && caretPos <= (int)slice.size())
      slice.insert(caretPos, "_");

    return slice;
  }

  bool handleKey(UiInput const& input)
  {
    if (editing)
    {
        if (input.keyLeft())  { moveLeft(); return true; }
        if (input.keyRight()) { moveRight(); return true; }
        if (input.keyBackSpace()) { backspace(); return true; }


        if (input.key >= 32 && input.key <= 126)
        {
            insertChar((char)input.key);
            return true;
        }

        return true;
      }
      return false;
    }
};
