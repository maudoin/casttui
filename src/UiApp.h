#pragma once

#include "MouseEvent.h"

#include <optional>

class UiApp
{

public:
  explicit UiApp();

  ~UiApp();

  void run();
  static std::optional<MouseEvent> mouseHit(int k, WINDOW* win);

protected:
  virtual bool doHandleKey(int k) = 0;
  virtual void doDelWindows() = 0;
  virtual void doBuildWindows(int h, int w) = 0;
  virtual void doRender(int k, int height, int width) = 0;

  void buildWindows();
  void delWindows();

private:
  bool handleKey(int k);
  void render(int k);
protected:
  bool _isRunning = true;
};
