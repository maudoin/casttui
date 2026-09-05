


#include "DialogNew.h"

#if defined(_WIN32)
#include <curses.h>
#else
#include <ncurses.h>
#endif
#include <form.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


static void form_driver_improved(FORM* form, int ch)
{
	switch (ch)
	{
		case KEY_DOWN:
		case KEY_BTAB:
			form_driver(form, REQ_NEXT_FIELD);
			form_driver(form, REQ_END_LINE);
			break;

		case KEY_UP:
		case KEY_STAB:
			form_driver(form, REQ_PREV_FIELD);
			form_driver(form, REQ_END_LINE);
			break;

		case KEY_LEFT:
			form_driver(form, REQ_PREV_CHAR);
			break;

		case KEY_RIGHT:
			form_driver(form, REQ_NEXT_CHAR);
			break;

		case KEY_PPAGE:
			form_driver(form, REQ_BEG_FIELD);
			break;

		case KEY_NPAGE:
			form_driver(form, REQ_END_FIELD);
			break;

		// Delete the char before cursor
		case KEY_BACKSPACE:
		case 127:
			form_driver(form, REQ_DEL_PREV);
			break;

		// Delete the char under the cursor
		case KEY_DC:
			form_driver(form, REQ_DEL_CHAR);
			break;

		default:
			form_driver(form, ch);
			break;
	}

}

void popupFieldDialog(WINDOW* parent)
{
    static constexpr int KEY_ESC = 0x1b;

	noecho();
	cbreak();
	keypad(stdscr, TRUE);
    curs_set(2);

	WINDOW* win_body = newwin(24, 80, 0, 0);
	assert(win_body != NULL);
    werase(win_body);
	box(win_body, 0, 0);
	WINDOW* win_form = derwin(win_body, 20, 78, 3, 1);
	assert(win_form != NULL);
	box(win_form, 0, 0);
	mvwprintw(win_body, 1, 2, "New podcast");

	FIELD *fields[5];
	fields[0] = new_field(1, 10, 0, 0, 0, 0);
	fields[1] = new_field(1, 40, 0, 15, 0, 0);
	fields[2] = new_field(1, 10, 1, 0, 0, 0);
	fields[3] = new_field(1, 40, 1, 15, 0, 0);
	fields[4] = NULL;
	assert(fields[0] != NULL && fields[1] != NULL && fields[2] != NULL && fields[3] != NULL);

	set_field_buffer(fields[0], 0, "url:");
	set_field_buffer(fields[1], 0, "http://podacast.com");
	set_field_buffer(fields[2], 0, "folder:");
	set_field_buffer(fields[3], 0, "d:\\podcats\\newnewnew");

	set_field_opts(fields[0], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
	set_field_opts(fields[1], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE);
	set_field_opts(fields[2], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
	set_field_opts(fields[3], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE);

	set_field_back(fields[1], A_UNDERLINE);
	set_field_back(fields[3], A_UNDERLINE);

	FORM* form = new_form(fields);
	assert(form != NULL);
	set_form_win(form, win_form);
	set_form_sub(form, derwin(win_form, 18, 76, 1, 1));
	post_form(form);

	refresh();
	wrefresh(win_body);
	wrefresh(win_form);

	while (true)
	{
		int ch = getch();
		if(ch == 0x1b ) //KEY_ESC
		{
			break;
		}
		form_driver_improved(form, ch);
		wrefresh(win_form);
	}

	unpost_form(form);
	free_form(form);
	free_field(fields[0]);
	free_field(fields[1]);
	free_field(fields[2]);
	free_field(fields[3]);
	delwin(win_form);
	delwin(win_body);
}
