#include "UiApp.h"
#include "UiTable.h"
#include "UiColors.h"
#include <vector>
#include <string>

struct UiTableDemo : public UiApp
{
  int width;
  bool header;
  int style;
  UiTable table;
  bool running = true;
  UiTableDemo(int width, bool header, int style = UiColors::normalStyle())
  : width(width)
  , header(header)
  , style(style)
  , table(UiTable::Mode::SCROLL, []{}, 5, 0)
  {}


  bool doHandleKey(UiInput const& input) override
  {
    if (!table.handleKey(input))
    {
      if (input.keyEnterReturn() || input.keySpace() || input.keyEsc())
      {
        this->exit();
        return true;
      }
    }
    return false;
  }
  void doDelWindows() override
  {
    table.delWindow();
  };
  void doBuildWindows(int h, int w) override
  {
    int height = 14;
    int starty = 1;
    int startx = 3;
    table.buildWindow(height, width, starty, startx);
  }
  void doRender(UiInput const& input) override
  {
    Columns cols = [&]{
      if (header)
      {
        return table.renderHeader(input, std::array{
          HeaderColumn{ 3, std::wstring(L"A"), SortDir::UP },
          HeaderColumn{ HeaderColumn::FILL, std::wstring(L"B (fill) "), SortDir::NONE },
          HeaderColumn{ 4, std::wstring(L"C"), SortDir::NONE }
        }, style);
      }
      else
      {
        return table.renderHeader(input, std::array{
          HeaderColumn{ 10, std::nullopt, SortDir::NONE },
          HeaderColumn{ HeaderColumn::FILL, std::nullopt, SortDir::NONE },
          HeaderColumn{ 20, std::nullopt, SortDir::NONE }
        }, style);
      }
    }();

    int rowCount = 20;

    auto cellCallback = [&cols](int row, int col, std::optional<UiInput::MouseEvent> const&ev) -> Cell {
      std::wstring base = col==0?L"a":col==1?L"b":L"c";
      std::wstring text = base + std::to_wstring(row);
      int style = ev?UiColors::focusedStyle():(row % 2 == 0) ? UiColors::boldStyle() : UiColors::dimmedStyle();
      return Cell{ text, style };
    };

    table.render(input, rowCount, cols, cellCallback, style);
  }
};

int main()
{

  UiTableDemo(25, true).run();
  UiTableDemo(25, true, UiColors::focusedStyle()).run();
  UiTableDemo(50, false).run();
  UiTableDemo(50, false, UiColors::focusedStyle()).run();

  return 0;
}
