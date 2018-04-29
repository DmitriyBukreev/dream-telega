#include "precompiled.h"
#include "tui.h"

void free_window(wnd_instance *wnd)
{
	if (wnd == NULL)
		return;
	if (wnd->panel != NULL)
		del_panel(wnd->panel);
	if (wnd->content != NULL)
		delwin(wnd->content);
	if (wnd->box != NULL)
		delwin(wnd->box);
	free(wnd);
}

wnd_instance *make_window(int y, int x, int height, int width, char *label)
{
	wnd_instance *wnd = malloc(sizeof(wnd_instance));
	int wnd_max_y, wnd_max_x;

	if (wnd == NULL)
		return NULL;
	// Making sure all pointes initialized with NULL
	wnd->box = NULL;
	wnd->content = NULL;
	wnd->panel = NULL;

	// Making border for window
	wnd->box = newwin(height, width, y, x);
	if (wnd->box == NULL) {
		free_window(wnd);
		return NULL;
	}
	wbkgd(wnd->box, COLOR_PAIR(C_WINDOW));
	box(wnd->box, 0, 0);


	// Making contents of the window
	getmaxyx(wnd->box, wnd_max_y, wnd_max_x);
	wnd->content = derwin(wnd->box, wnd_max_y-2, wnd_max_x-2, 1, 1);
	if (wnd->content == NULL) {
		free_window(wnd);
		return NULL;
	}
	wbkgd(wnd->box, COLOR_PAIR(C_WINDOW));
	keypad(wnd->content, TRUE);

	// Adding panel
	wnd->panel = new_panel(wnd->box);
	if (wnd->panel == NULL) {
		free_window(wnd);
		return NULL;
	}

	// Adding label
	if (label != NULL)
		mvwaddnstr(wnd->box, 0, 2, label, wnd_max_x-3);

	// Output to screen
	update_panels();
	doupdate();
	return wnd;
}

void free_interface(tui_instance *tui)
{
	if (tui == NULL)
		return;
	if (tui->msg_wnd != NULL)
		free_window(tui->msg_wnd);
	if (tui->users_wnd != NULL)
		free_window(tui->users_wnd);
	if (tui->input_wnd)
		free_window(tui->input_wnd);
	free(tui);
}

tui_instance *make_interface(void)
{
	// Structure to return
	tui_instance *res_tui;

	res_tui = malloc(sizeof(tui_instance));
	if (res_tui == NULL)
		return NULL;
	// Info about stdscr size
	int scr_max_y, scr_max_x;
	// Relations between windows
	int vertical_sep, horizontal_sep;

	// Calculating sizes
	getmaxyx(stdscr, scr_max_y, scr_max_x);
	vertical_sep = 0.7 * scr_max_x;
	horizontal_sep = 0.7 * scr_max_y;

	// Making windows
	res_tui->msg_wnd = make_window(1, 0, horizontal_sep,
		vertical_sep, "Messages");
	if (res_tui->msg_wnd == NULL) {
		free_interface(res_tui);
		return NULL;
	}

	res_tui->users_wnd = make_window(1, vertical_sep, horizontal_sep,
		scr_max_x-vertical_sep, "Users list");
	if (res_tui->users_wnd == NULL) {
		free_interface(res_tui);
		return NULL;
	}

	res_tui->input_wnd = make_window(horizontal_sep+1, 0,
		scr_max_y-horizontal_sep-1, scr_max_x, "Input");

	if (res_tui->input_wnd == NULL) {
		free_interface(res_tui);
		return NULL;
	}

	return res_tui;
}

// Moves cursor within the textbox
int tboxmove(WINDOW *tbox, const char *buf, char dir, int *pos, const int max)
{
	// Position in the box
	int x, y;
	// Sizes of the box
	int max_x, max_y;

	// Checking if argument is correct
	if (dir < 0)
		dir = -1;
	else if (dir > 0)
		dir = 1;
	else
		return 0;

	// Checking if moving withing the line
	if ((*pos) + dir < 0 || (*pos) + dir > max)
		return 0;

	// Getting position in the box
	getyx(tbox, y, x);
	// Getting sizes of the box
	getmaxyx(tbox, max_y, max_x);

	if (x + dir > max_x - 1) {
		y++;
		x = 0;
	} else if (x + dir < 0) {
		y--;
		x = max_x - 1;
	} else
		x += dir;

	*pos += dir;

	wmove(tbox, y, x);
	return x;
}

#define IN_BUF_SIZE 256
int input_handler(tui_instance **tui)
{
	// Getting pointer to textbox
	WINDOW *textbox = (*tui)->input_wnd->content;
	// Key to handle
	int key;
	// Input buffer
	char buf[IN_BUF_SIZE];
	// Position in the buffer
	int buf_pos = 0;
	// Length of the message
	int len = 0;

	buf[0] = 0;
	while ((key = wgetch(textbox)) != ERR) {
		switch (key) {

		case KEY_UP:
		case KEY_DOWN:
		// Not implemented
			break;

		case KEY_STAB:
		case KEY_F(12):
			// Launching next handler
			// or exiting
			return key;

		case KEY_RESIZE:
			// Redrawing interface
			endwin();
			refresh();
			clear();
			free_interface(*tui);
			// Updating pointers
			*tui = make_interface();
			textbox = (*tui)->input_wnd->content;
			// Restoring buffer to textbox
			buf[len] = 0;
			waddstr(textbox, buf);
			break;

		case KEY_BACKSPACE:
			if (buf_pos < len)
				// Removing character in the middle of line
				memmove(buf+buf_pos, buf+buf_pos+1,
					len-buf_pos);
			tboxmove(textbox, buf, -1, &buf_pos, len);
			wdelch(textbox);
			len--;
			break;

		case KEY_LEFT:
			tboxmove(textbox, buf, -1, &buf_pos, len);
			break;

		case KEY_RIGHT:
			tboxmove(textbox, buf, 1, &buf_pos, len);
			break;

		case '\n':
			buf[len] = '\n';
			buf[len+1] = 0;

			// TODO: fancy printing to msg_wnd
			HDL_ERR_LOGGED(waddstr((*tui)->msg_wnd->content, buf),
				ERR, "waddstr() failed", ERR);
			HDL_ERR_LOGGED(wrefresh((*tui)->msg_wnd->content),
				ERR, "wrefresh() failed", ERR);
			HDL_ERR_LOGGED(werase(textbox), ERR,
				"werase() failed", ERR);

			len = 0;
			buf_pos = 0;
			buf[0] = 0;
			break;

		default:
			// 2 charecters for '\n' and 0 in the end of the buffer
			if (len < IN_BUF_SIZE - 2) {
				if (buf_pos < len) {
					// Adding character in the middle
					memmove(buf+buf_pos+1, buf+buf_pos,
						len-buf_pos);
					buf[buf_pos] = key;
					winsch(textbox, key);
					tboxmove(textbox, buf, 1,
						&buf_pos, len);
				} else {
					// Adding character in the end of buffer
					buf[buf_pos] = key;
					waddch(textbox, key);
					buf_pos++;
				}
				len++;
			}
		}
	}
}

void init_curses(void)
{
	// Regular initialization of ncurses
	initscr();

	// Input handling settings
	HDL_ERR_LOGGED(cbreak(), ERR, "Cbreak() failed", ERR);
	// HDL_ERR_LOGGED(raw(), ERR, "Raw() failed", ERR);
	HDL_ERR_LOGGED(noecho(), ERR,
		"Couldn't enter noecho mode", ERR);

	// Color pairs initialization
	HDL_ERR_LOGGED(has_colors(), FALSE,
		"No colors in terminal", E_NOCOLOURS);
	HDL_ERR_LOGGED(start_color(), ERR,
		"The color table cannot be allocated", ERR);
	init_pair(C_WINDOW, COLOR_BLACK, COLOR_WHITE);
}