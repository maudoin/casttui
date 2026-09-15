#pragma once

#if defined(_WIN32)
#include <curses.h>
#else
#include <ncurses.h>
#endif

#include <optional>

#if defined(PDCURSES)
#define getmouse nc_getmouse
#endif

struct MouseEvent
{
    int x;
    int y;
    bool left;
    bool right;
    bool middle;
    bool prev=0;//not under windows
    bool next=0;//not under windows
    bool ctrl;
    bool shift;
    bool alt;
    mmask_t raw_bstate;

    MouseEvent toLocal(WINDOW* win)
    {
        int beginRow, beginCol;
        getbegyx(win, beginRow, beginCol);
        MouseEvent locaEv = *this;
        locaEv.x -= beginCol;
        locaEv.y -= beginRow;
        return locaEv;
    }

    struct Loc{ int beginRow=0, beginCol=0, endRow=0, endCol=0; };
    bool hit(Loc const& loc)
    {
        return (x >= loc.beginCol && x < loc.endCol) &&
               (y >= loc.beginRow && y < loc.endRow);
    }
};

inline std::optional<MouseEvent> getMouseEvent()
{
    MEVENT ev;

    if (getmouse(&ev) != OK)
    {
        return std::nullopt;
    }

    std::optional<MouseEvent> out = std::make_optional<MouseEvent>();
    out->x = ev.x;
    out->y = ev.y;
    out->raw_bstate = ev.bstate;

    // Buttons (same in ncurses + PDCurses)
    out->left   = ev.bstate & BUTTON1_PRESSED || ev.bstate & BUTTON1_CLICKED;
    out->middle = ev.bstate & BUTTON2_PRESSED || ev.bstate & BUTTON2_CLICKED;
    out->right  = ev.bstate & BUTTON3_PRESSED || ev.bstate & BUTTON3_CLICKED;
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

        out->ctrl  = ex->modifier & 0x02; // PDC_KEY_MODIFIER_CONTROL
        out->shift = ex->modifier & 0x01; // PDC_KEY_MODIFIER_SHIFT
        out->alt   = ex->modifier & 0x04; // PDC_KEY_MODIFIER_ALT
    }
    else
    {
        // No modifier support in this PDCurses build
        out->ctrl  = false;
        out->shift = false;
        out->alt   = false;
    }

#else
    out->prev  = ev.bstate & BUTTON4_PRESSED || ev.bstate & BUTTON4_CLICKED;
    out->next  = ev.bstate & BUTTON5_PRESSED || ev.bstate & BUTTON5_CLICKED;
    // ncurses: modifiers are stored in bstate
    out->ctrl  = ev.bstate & BUTTON_CTRL;
    out->shift = ev.bstate & BUTTON_SHIFT;
    out->alt   = ev.bstate & BUTTON_ALT;
#endif

    return out;
}
