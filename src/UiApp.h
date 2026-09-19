#pragma once

#if defined(_WIN32)
#include <curses.h>
#else
#include <ncursesw/curses.h>
#endif

#include <optional>

struct UiInput;
class UiApp
{

public:
  explicit UiApp();

  ~UiApp();

  void run();
  static bool mouseHit(UiInput const& input, WINDOW* win);

protected:
  virtual bool doHandleKey(UiInput const& input) = 0;
  virtual void doDelWindows() = 0;
  virtual void doBuildWindows(int h, int w) = 0;
  virtual void doRender(UiInput const& input) = 0;

  void buildWindows();
  void delWindows();

private:
  bool handleKey(UiInput const& input);
  void render(UiInput const& input);
protected:
  bool _isRunning = true;
};
