#include "precompiled.h"
#include "tui.h"

wnd_instance *draw_window(int y, int x, int height, int width, char *label)
{
	wnd_instance *wnd = malloc(sizeof(wnd_instance));
	int wnd_max_y, wnd_max_x;

	// Making sure all pointes initialized with NULL
	if (wnd == NULL)
		return NULL;
	wnd->box = NULL;
	wnd->content = NULL;
	wnd->panel = NULL;

	// Making border for window
	wnd->box = newwin(height, width, y, x);
	if (wnd->box == NULL)
		return NULL;
	wbkgd(wnd->box, COLOR_PAIR(C_WINDOW));
	box(wnd->box, 0, 0);


	// Making contents of the window
	getmaxyx(wnd->box, wnd_max_y, wnd_max_x);
	wnd->content = derwin(wnd->box, wnd_max_y-2, wnd_max_x-2, 1, 1);
	if (wnd->content == NULL)
		return NULL;
	wbkgd(wnd->box, COLOR_PAIR(C_WINDOW));

	// Adding panel
	wnd->panel = new_panel(wnd->box);
	if (wnd->panel == NULL)
		return NULL;

	// Adding label
	if (label != NULL)
		mvwaddnstr(wnd->box, 0, 2, label, wnd_max_x-3);

	// Output to screen
	update_panels();
	doupdate();
	return wnd;
}

void free_window(wnd_instance *wnd)
{
	if (wnd != NULL) {
		if (wnd->panel != NULL)
			del_panel(wnd->panel);
		if (wnd->content != NULL)
			delwin(wnd->content);
		if (wnd->box != NULL)
			delwin(wnd->box);
		free(wnd);
	}
}

void draw_interface(void)
{
	wnd_instance *msg_wnd;

	msg_wnd = draw_window(5, 5, 15, 15, "TESTING CUSTOM WINDOS");
	getch();

	free_window(msg_wnd);
}

void init_curses(void)
{
	initscr();

	HDL_ERR_LOGGED(cbreak(), ERR, "Cbreak() failed", ERR);
	HDL_ERR_LOGGED(keypad(stdscr, TRUE), ERR, "Keypad() failed", ERR);
	HDL_ERR_LOGGED(has_colors(), FALSE,
		"No colors in terminal", E_NOCOLOURS);
	HDL_ERR_LOGGED(start_color(), ERR,
		"The color table cannot be allocated", ERR);
	HDL_ERR_LOGGED(noecho(), ERR,
		"Couldn't enter noecho mode", ERR);
	use_default_colors();
	init_pair(C_WINDOW, COLOR_WHITE, COLOR_RED);
}