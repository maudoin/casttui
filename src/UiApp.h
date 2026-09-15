#pragma once

class MouseEvent;

class UiApp
{

public:
  explicit UiApp();

  ~UiApp();

  void run();
  protected:

  virtual bool doHandleMouse(MouseEvent const& ev) = 0;
  virtual bool doHandleKey(int k) = 0;
  virtual void doDelWindows() = 0;
  virtual void doBuildWindows(int h, int w) = 0;
  virtual void doRender(int k) = 0;

  void buildWindows();
  void delWindows();
private:

  bool handleKey(int k);
  void render(int k);
protected:
  bool _isRunning = true;
};
