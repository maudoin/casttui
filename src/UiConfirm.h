#pragma once

#include "UiTable.h"

#include <functional>
#include <string>

class UiConfirm : public UiTable
{
public:
  UiConfirm(std::function<void()> const& cancel);

  void set(std::wstring const& title, std::function<void()> const& action);

  void render(int k);
  bool handleKey(int k);
private:
  void cancel();

  std::wstring _title;
  std::function<void()> _action;
  std::function<void()> _cancel;
  int _field = 1;//accept,cancel
};
