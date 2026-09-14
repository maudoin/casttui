#pragma once

#include "UiTable.h"

#include <functional>


class DowncastLogic;

class UiShowsTable : public UiTable
{
public:
  UiShowsTable(DowncastLogic& logic, std::function<void()> const& winSelection);

  void render(int k, bool focused);
  bool handleKey(int k);

private:
  DowncastLogic& _logic;
};
