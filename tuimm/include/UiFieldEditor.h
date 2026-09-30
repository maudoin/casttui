#pragma once

#include "UiInput.h"
#include "to_wstring.h"

#include <string>

struct UiFieldEditor
{
  bool _editing = false;
  std::string _buffer;
  int _caret = 0;
public:

bool editing()const{return _editing;}

  void begin(const std::string &initial, int caretPos)
  {
    _buffer = initial;
    _caret = std::max(0, std::min(caretPos, (int)_buffer.size()));
    _editing = true;
  }

  void commitTo(std::string &target)
  {
    target = _buffer;
    cancel();
  }

  void cancel()
  {
    _editing = false;
    _buffer.clear();
    _caret = 0;
  }

  void display(UiTable::CellRenderStr& str, int colWidth) const
  {
    if (!_editing)
    {
      return;
    }

    std::wstring buff = to_wstring(_buffer);

    if (_caret == _buffer.size())
    {
      str << UiColors::normalStyle() << buff;
      str << UiColors::highlightedStyle() << L" ";
      return;
    }

    int scroll = 0;
    if (colWidth > 0 && _caret >= colWidth)
      scroll = _caret - (colWidth - 1);

    std::wstring_view visible_slice = std::wstring_view(buff).substr(scroll);

    size_t caret_pos = static_cast<size_t>(std::max(0, _caret - scroll));
    caret_pos = std::min(caret_pos, visible_slice.size());

    str << UiColors::normalStyle() << visible_slice.substr(0, caret_pos);
    if (caret_pos<visible_slice.size())
      str << UiColors::highlightedStyle() << visible_slice.substr(caret_pos, 1);
    if ((caret_pos+1)<visible_slice.size())
      str << UiColors::normalStyle()<< visible_slice.substr(caret_pos+1);
  }

  bool handleKey(UiInput const& input)
  {
    if (_editing)
    {
        if (input.keyLeft())  { moveLeft(); return true; }
        if (input.keyRight()) { moveRight(); return true; }
        if (input.keyHome())  { moveBegin(); return true; }
        if (input.keyEnd()) { moveEnd(); return true; }
        if (input.keyBackSpace())
        {
          backspace();
          return true;
        }
        if (input.keyDel())
        {
          del();
          return true;
        }
        if (input.key >= 32 && input.key <= 126)
        {
            insertChar((char)input.key);
            return true;
        }
      }
      return false;
    }

private:
  void insertChar(char c)
  {
    _buffer.insert(_buffer.begin() + _caret, c);
    _caret++;
  }

  void backspace()
  {
    if (_caret > 0)
    {
      _buffer.erase(_buffer.begin() + _caret - 1);
      _caret--;
    }
  }

  void del()
  {
    if (_caret < (int)(_buffer.size()-1))
    {
      _buffer.erase(_buffer.begin() + _caret);
    }
  }

  void moveLeft()
  {
    _caret = std::max(0, _caret - 1);
  }

  void moveRight()
  {
    _caret = std::min((int)_buffer.size(), _caret + 1);
  }

  void moveBegin()
  {
    _caret = 0;
  }

  void moveEnd()
  {
    _caret = (int)_buffer.size();
  }
};
