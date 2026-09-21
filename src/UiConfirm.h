#pragma once

#include "UiTable.h"

#include <functional>
#include <string>

class UiConfirm : public UiTable
{
public:
  UiConfirm(std::function<void()> const& cancel);

  void set(std::wstring const& actionLabel, std::function<void()> const& action, std::wstring const& title = {});

  void render(UiInput const& input);
  bool handleKey(UiInput const& input);
  int preferredWindowHeight();
private:
  void cancel();

  std::wstring _actionName;
  std::function<void()> _action;
  std::function<void()> _cancel;
  int _field = 1;//accept,cancel
  std::wstring _title;
};
