#include "UiApp.h"
#include "UiColors.h"
#include "UiConfirm.h"
#include "UiFileBrowser.h"
#include "UiTable.h"

#include <vector>
#include <string>

namespace{
struct UiTableDemo : public UiApp
{
  UiTable _titleTable;
  UiTable _table;
  UiFileBrowser _fileBrowser;
  UiConfirm _confirm;
  std::wstring _path = L"Click or press 's' to select path)";
  enum class Mode{MAIN, BROWSE, CONFIRM};
  Mode _mode = Mode::MAIN;
  bool _header = true;
  int _styleIndex = 0;
  static const int RowCount = 200000;
  struct StyleOpt{int style;std::wstring name;};
  static std::array<StyleOpt,3> _styleOpts;
  UiTableDemo()
  : _titleTable(UiTable::Mode::SCROLL)
  , _table(UiTable::Mode::CURSOR, [] {}, RowCount/2)
  , _fileBrowser(L"Select whatever", [this]{this->_path=this->_fileBrowser.getSelected().wstring();this->_mode = Mode::MAIN;})
  , _confirm([this]{this->_mode=Mode::MAIN;})
  {
    // confirm windows can be used for multiple purposes
    _confirm.set(L"Exit", [this]{this->exit();}, L"Shutdown");
  }


  bool doHandleKey(UiInput const& input) override
  {
    if (_mode == Mode::CONFIRM)
    {
      return _confirm.handleKey(input);
    }

    if (_mode == Mode::BROWSE)
    {
      return _fileBrowser.handleKey(input);
    }

    if (input.key == 's')
    {
      _mode = Mode::BROWSE;
    }
    if (input.key == 'h')
    {
      _header = !_header;
    }
    if (input.key == 'r')
    {
      _styleIndex = (_styleIndex+1)%_styleOpts.size();
    }
    if (!_table.handleKey(input) && !_titleTable.handleKey(input))
    {
      if (input.keyEsc())
      {
        _mode = Mode::BROWSE;
        return true;
      }
    }
    return false;
  }
  void doDelWindows() override
  {
    _titleTable.delWindow();
    _table.delWindow();
    _fileBrowser.delWindow();
    _confirm.delWindow();
  };
  void doBuildWindows(int height, int width) override
  {
    static constexpr int TitleHeight = 3;
    _titleTable.buildWindow(TitleHeight, width, 0, 0);
    _table.buildWindow(height-TitleHeight, width, TitleHeight, 0);
    _fileBrowser.buildWindowCentered(std::max(10,height-6), std::max(30,width-6), height, width);
    _confirm.buildWindowCentered(5, std::max(30,width-6), height, width);
  }
  void doRender(UiInput const& input) override
  {
    if (_mode == Mode::BROWSE)
    {
      _fileBrowser.render(input, true, []{});
      return;
    }
    if (_mode == Mode::CONFIRM)
    {
      _confirm.render(input);
      return;
    }
    // Title table
    Columns titleCols = _titleTable.renderHeader(input, std::array{
          HeaderColumn{ .width=HeaderColumn::FIT_LABEL, .name=_path, .callback=[this]{this->_mode = Mode::BROWSE;} },
          HeaderColumn{ .width=HeaderColumn::FILL },
          HeaderColumn{ .width=HeaderColumn::FIT_LABEL, .name=L" X ", .callback=[this]{this->_mode = Mode::CONFIRM;} },
    }, _styleOpts[_styleIndex].style);
    _titleTable.render(input, 0, titleCols, [](int row, int col, std::optional<UiInput::MouseEvent> const&ev){return Cell{};}, _styleOpts[_styleIndex].style);


    // Main table
    Columns cols = [&]{
      if (_header)
      {
        return _table.renderHeader(input, std::array{
          HeaderColumn{ 10, std::wstring(L"A"), SortDir::UP },
          HeaderColumn{ HeaderColumn::FILL, std::wstring(L"B (fill _table with column to enable horizontal scrolling and reveal content) "), SortDir::NONE },
          HeaderColumn{ 10, std::wstring(L"C"), SortDir::NONE }
        }, _styleOpts[_styleIndex].style);
      }
      else
      {
        return _table.renderHeader(input, std::array{
          HeaderColumn{ 10, std::nullopt, SortDir::NONE },
          HeaderColumn{ HeaderColumn::FILL, std::nullopt, SortDir::NONE },
          HeaderColumn{ 20, std::nullopt, SortDir::NONE }
        }, _styleOpts[_styleIndex].style);
      }
    }();

    auto cellCallback = [&](int row, int col, std::optional<UiInput::MouseEvent> const&ev) -> Cell {
      std::wstring base = col==0?L"a":col==1?(std::wstring(200, L'-')+L"b"):L"c";
      std::wstring text = base + std::to_wstring(row);
      return Cell{ .text=text, .style=UiColors::highlightStyle(_table.cursor()==row) };
    };

    Columns afterTable = _table.render(input, RowCount, cols, cellCallback, _styleOpts[_styleIndex].style, []{}, UiTable::RowReserve{2});

    _table.renderHeaderOnly(input, std::array{
          HeaderColumn{ .width=HeaderColumn::FIT_LABEL, .name=_header?L"Hide header (h)":L"Show header (h)", .callback=[this]{this->_header=!this->_header;} },
          HeaderColumn{ .width=HeaderColumn::FIT_LABEL, .name=_styleOpts[_styleIndex].name+L" (r)", .callback=[this]{this->_styleIndex = (this->_styleIndex+1)%_styleOpts.size();} },
          HeaderColumn{ .width=HeaderColumn::FILL }
    }, _styleOpts[_styleIndex].style, afterTable);
  }
};

std::array<UiTableDemo::StyleOpt,3> UiTableDemo::_styleOpts{{
  {UiColors::normalStyle(), L"normal"},
  {UiColors::focusedStyle(), L"focused"},
  {UiColors::highlightedStyle(), L"highlighted"} }};

}

int main()
{
  UiTableDemo().run();
  return 0;
}
