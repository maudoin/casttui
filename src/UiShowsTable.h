#pragma once

#include "UiTable.h"

#include <functional>


class DowncastLogic;

class UiShowsTable : public UiTable
{
public:
  UiShowsTable(DowncastLogic& logic);

  void render(int k, bool focused, std::function<void()> const& winSelection);
  bool handleKey(int k);

private:
  DowncastLogic& _logic;
};
