#include "UiInput.h"

#if defined(_WIN32)
#include <curses.h>
#else
#include <ncurses.h>
#endif

#include <optional>

#if defined(PDCURSES)
#define getmouse nc_getmouse
#endif

bool UiInput::MouseEvent::hit(UiInput::MouseEvent::Loc const& loc) const
{
  return (x >= loc.beginCol && x < loc.endCol) &&
         (y >= loc.beginRow && y < loc.endRow);
}

UiInput UiInput::init(int key)
{
  UiInput input{key};

  if (key == KEY_MOUSE)
  {
    MEVENT ev;

    if (getmouse(&ev) == OK)
    {

      input.mev = std::make_optional<UiInput::MouseEvent>();
      input.mev->x = ev.x;
      input.mev->y = ev.y;

      // Buttons (same in ncurses + PDCurses)
      input.mev->left   = ev.bstate & BUTTON1_PRESSED || ev.bstate & BUTTON1_CLICKED;
      input.mev->middle = ev.bstate & BUTTON2_PRESSED || ev.bstate & BUTTON2_CLICKED;
      input.mev->right  = ev.bstate & BUTTON3_PRESSED || ev.bstate & BUTTON3_CLICKED;
      // --- Modifier keys ---
#if defined(PDCURSES)
      // Some PDCurses versions have ev.modifier, some do not.
      // We detect it using sizeof(MEVENT).
      //
      // Classic PDCurses MEVENT is 16 bytes.
      // Extended MEVENT (with modifier) is larger.
      //
      if (sizeof(MEVENT) > 16)
      {
          // reinterpret cast to access modifier safely
          struct MEVENT_EX { int id,x,y,z; mmask_t bstate; int modifier; };
          auto *ex = reinterpret_cast<MEVENT_EX*>(&ev);

          input.mev->ctrl  = ex->modifier & 0x02; // PDC_KEY_MODIFIER_CONTROL
          input.mev->shift = ex->modifier & 0x01; // PDC_KEY_MODIFIER_SHIFT
          input.mev->alt   = ex->modifier & 0x04; // PDC_KEY_MODIFIER_ALT
      }
      else
      {
          // No modifier support in this PDCurses build
          input.mev->ctrl  = false;
          input.mev->shift = false;
          input.mev->alt   = false;
      }
#else
      input.mev->prev  = ev.bstate & BUTTON4_PRESSED || ev.bstate & BUTTON4_CLICKED;
      input.mev->next  = ev.bstate & BUTTON5_PRESSED || ev.bstate & BUTTON5_CLICKED;
      // ncurses: modifiers are stored in bstate
      input.mev->ctrl  = ev.bstate & BUTTON_CTRL;
      input.mev->shift = ev.bstate & BUTTON_SHIFT;
      input.mev->alt   = ev.bstate & BUTTON_ALT;
#endif
    }
  }
#ifdef PDCURSES_WIN32
  if (key == 22)  // ^V = Ctrl-V
  {
    char *clip;
    long len;

    if (PDC_getclipboard(&clip, &len) == PDC_CLIP_SUCCESS)
    {
      for (long i = len - 1; i >= 0; i--)
      {
        ungetch(clip[i]);   // feed clipboard text into input queue
      }
      PDC_freeclipboard(clip);
    }
  }
#endif
  return input;
}

bool UiInput::keyLeft() const
{
  return key == KEY_LEFT;
}
bool UiInput::keyRight() const
{
  return key == KEY_RIGHT;
}
bool UiInput::keyUp() const
{
  return key == KEY_UP;
}
bool UiInput::keyDown() const
{
  return key == KEY_DOWN;
}
bool UiInput::keyBackSpace() const
{
  return key == KEY_BACKSPACE || key == '\b';
}
bool UiInput::keyDel() const
{
  return key == KEY_DC || key == '\b';
}
bool UiInput::keyTab() const
{
  return key == 9;
}
bool UiInput::keyBackTab() const
{
  return key == KEY_BTAB;
}
bool UiInput::keyEnterReturn() const
{
  return key == 10 || key == 13;
}
bool UiInput::keySpace() const
{
  return key == 32 || key == ' ';
}
bool UiInput::keyEsc() const
{
  return key == 27;
}
bool UiInput::keyMinus() const
{
  #ifdef PDCURSES_WIN32
    return key == PADMINUS;
  #else
    return key == '-';
  #endif
}
bool UiInput::keyPlus() const
{
  #ifdef PDCURSES_WIN32
    return key == PADPLUS;
  #else
    return key == '+';
  #endif
}
bool UiInput::keyPageUp() const
{
  return key == KEY_PPAGE;
}
bool UiInput::keyPageDown() const
{
  return key == KEY_NPAGE;
}
bool UiInput::keyHome() const
{
  return key == KEY_HOME;
}
bool UiInput::keyEnd() const
{
  return key == KEY_END;
}