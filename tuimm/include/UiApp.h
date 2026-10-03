#pragma once

#include <memory>
#include <optional>

struct UiInput;

class OfflineBuffers;  // forward declaration for pimpl class

class UiApp
{

public:
  explicit UiApp(int argc, char* argv[]);

  ~UiApp();

  void run();

protected:
  virtual bool doHandleKey(UiInput const& input) = 0;
  virtual void doDelWindows() = 0;
  virtual void doBuildWindows(int h, int w) = 0;
  virtual void doRender(UiInput const& input) = 0;

  void buildWindows();
  void delWindows();

  void exit() {_isRunning = false;}
private:
  bool handleKey(UiInput const& input);
  void render(UiInput const& input);
  bool _isRunning = true;

  std::unique_ptr<OfflineBuffers> _offline;
};
