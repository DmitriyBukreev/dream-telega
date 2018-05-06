#include "precompiled.h"
#include "tui.h"
#include "data.h"

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

wnd_instance *make_window(int y, int x, int height, int width, char *label,
	char boxed)
{
	wnd_instance *wnd = malloc(sizeof(wnd_instance));
	int wnd_max_y, wnd_max_x;

	if (wnd == NULL)
		return NULL;

	// Making sure all pointers are initialized with NULL
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
	if (boxed) {
		box(wnd->box, 0, 0);

		// Making contents of the box
		getmaxyx(wnd->box, wnd_max_y, wnd_max_x);
		wnd->content = derwin(wnd->box, wnd_max_y-2, wnd_max_x-2, 1, 1);
		if (wnd->content == NULL) {
			free_window(wnd);
			return NULL;
		}
		wbkgd(wnd->box, COLOR_PAIR(C_WINDOW));
		keypad(wnd->content, TRUE);
	}

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

wnd_instance *make_window_centered(int height, int width,
	char *label, char boxed)
{
	int scr_max_x, scr_max_y;

	getmaxyx(stdscr, scr_max_y, scr_max_x);
	return make_window(scr_max_y/2-height/2, scr_max_x/2-width/2,
		height, width, label, boxed);
}

wnd_instance *make_window_relative(wnd_instance *orig, int y, int x,
	int height, int width, char *label, char boxed)
{
	int x0, y0;

	if (orig->content != NULL)
		getbegyx(orig->content, y0, x0);
	else
		getbegyx(orig->box, y0, x0);

	return make_window(y0+y, x0+x, height, width, label, boxed);
}

void delete_menu(MENU *del_menu)
{
	ITEM **del_items;
	ITEM **iter;

	if (del_menu == NULL)
		return;
	del_items = iter = menu_items(del_menu);
	unpost_menu(del_menu);
	free_menu(del_menu);
	while (*iter != NULL) {
		free_item(*iter);
		iter++;
	}
	free(del_items);
}

MENU *create_menu(char **opts, int opts_num)
{
	// Pointer to return
	MENU *res_menu;
	// Items to initialize the menu
	ITEM **items;

	// Initializing menu
	items = malloc(sizeof(ITEM *) * (opts_num + 1));
	if (items == NULL) {
		free_menu(res_menu);
		return NULL;
	}
	for (int i = 0; i < opts_num; i++)
		items[i] = new_item(opts[i], "");
	items[opts_num] = NULL;
	res_menu = new_menu(items);

	return res_menu;
}

void free_interface(tui_instance *tui)
{
	if (tui == NULL)
		return;
	if (tui->msg_wnd != NULL) {
		delwin((WINDOW *)panel_userptr(tui->msg_wnd->panel));
		free_window(tui->msg_wnd);
	}
	if (tui->users_wnd != NULL)
		free_window(tui->users_wnd);
	if (tui->input_wnd != NULL)
		free_window(tui->input_wnd);
	if (tui->menu_wnd != NULL) {
		delete_menu((MENU *)panel_userptr(tui->menu_wnd->panel));
		free_window(tui->menu_wnd);
	}
	free(tui);
}

#define BREAK_KEY(key) \
((key) == KEY_F(12) || (key) == KEY_RESIZE || (key) == ERR || (key) == '\n')

int form_handler(FORM *form, int *len, int maxlen)
{
	int pos, key;

	pos = *len;
	curs_set(1);
	form_driver(form, REQ_END_FIELD);
	while (key = wgetch(form_sub(form))) {
		if (BREAK_KEY(key) || key == '\t') {
			form_driver(form, REQ_VALIDATION);
			curs_set(0);
			return key;
		}
		switch (key) {
		case KEY_LEFT:
		if (pos > 0) {
			form_driver(form, REQ_PREV_CHAR);
			pos--;
		}
		break;

		case KEY_RIGHT:
		if (pos < (*len)) {
			form_driver(form, REQ_NEXT_CHAR);
			pos++;
		}
		break;

		case KEY_BACKSPACE:
		if (pos > 0) {
			form_driver(form, REQ_DEL_PREV);
			pos--;
			(*len)--;
		}
		break;

		default:
		if ((*len) < maxlen - 1 && form_driver(form, key) == E_OK) {
			pos++;
			(*len)++;
		}
		}
	}
}

#define COLOR_OPTS_NUM 6
#define SETTINGS_HEIGHT 13
#define SETTINGS_WIDTH 36
int get_settings(settings_instance * settings)
{
	wnd_instance *main_wnd, *form_wnd, *menu_wnd;
	int key, len;
	FIELD * field[2];
	FORM *form;
	MENU *menu;
	ITEM **items;
	char *options[COLOR_OPTS_NUM] = {
	"Black", "Red", "Green", "Yellow", "Magenta", "Cyan" };

	// Making window in the middle of screen
	main_wnd = make_window_centered(SETTINGS_HEIGHT, SETTINGS_WIDTH,
		"Settings", TRUE);
	// Making window for a form
	form_wnd = make_window_relative(main_wnd, 0, 0,
		3, SETTINGS_WIDTH-2, "Nickname", TRUE);
	// Making field and form for nickname
	field[0] = new_field(1, NAME_LENGTH, 0, 0, 0, 0);
	field[1] = NULL;
	set_field_back(field[0], COLOR_PAIR(C_WINDOW));
	set_field_fore(field[0], COLOR_PAIR(settings->color));
	set_field_buffer(field[0], 0, settings->nickname);
	set_field_type(field[0], TYPE_ALNUM);
	len = strlen(settings->nickname);
	form = new_form(field);
	set_form_sub(form, form_wnd->content);
	post_form(form);

	// Making window for a menu
	menu_wnd = make_window_relative(main_wnd, 3, 0,
		8, SETTINGS_WIDTH-2, "Color", TRUE);
	menu = create_menu(options, COLOR_OPTS_NUM);
	items = menu_items(menu);
	set_menu_mark(menu, "");
	set_menu_fore(menu, COLOR_PAIR(C_SELECT));
	set_menu_back(menu, COLOR_PAIR(C_WINDOW));
	set_menu_win(menu, menu_wnd->content);
	set_current_item(menu, items[settings->color - C_NICKBLACK]);
	post_menu(menu);
	wrefresh(menu_wnd->content);

	do {
		// Processing input from a form
		key = form_handler(form, &len, NAME_LENGTH);
		if (BREAK_KEY(key))
			break;

		// Input from the menu
		key = wgetch(menu_wnd->content);
		while (!BREAK_KEY(key) && key != '\t') {
			switch (key) {
			case KEY_UP:
			menu_driver(menu, REQ_UP_ITEM);
			break;

			case KEY_DOWN:
			menu_driver(menu, REQ_DOWN_ITEM);
			break;
			}
			set_field_fore(field[0],
			COLOR_PAIR(item_index(current_item(menu))
					+ C_NICKBLACK));
			touchwin(form_wnd->content);
			wrefresh(form_wnd->content);
			key = wgetch(menu_wnd->content);
		}
	} while (!BREAK_KEY(key));

	if (key == '\n') {
		// Getting data from the form
		strncpy(settings->nickname, field_buffer(field[0], 0), len);
		settings->nickname[len] = 0;
		// Getting data from menu
		settings->color = item_index(current_item(menu)) + C_NICKBLACK;
		save_settings(settings);
	}

	// Releasing memory
	delete_menu(menu);
	free_window(menu_wnd);
	unpost_form(form);
	free_form(form);
	free_field(field[0]);
	free_window(form_wnd);
	free_window(main_wnd);
	update_panels();
	doupdate();
	return key;
}

#define CONNECTION_HEIGHT 8
#define CONNECTION_WIDTH 36
int get_connection(settings_instance *settings)
{
	wnd_instance *main_wnd, *adress_wnd, *port_wnd;
	FIELD * adress_field[2], *port_field[2];
	FORM *adress_form, *port_form;
	int adress_len, port_len;
	int key = 0;

	// Making window in the middle of screen
	main_wnd = make_window_centered(CONNECTION_HEIGHT, CONNECTION_WIDTH,
		"Connection", TRUE);
	// Adress input GUI
	adress_wnd = make_window_relative(main_wnd, 0, 0,
		3, SETTINGS_WIDTH-2, "Adress", TRUE);
	adress_field[0] = new_field(1, ADRESS_LENGTH, 0, 0, 0, 0);
	adress_field[1] = NULL;
	set_field_back(adress_field[0], COLOR_PAIR(C_WINDOW));
	set_field_fore(adress_field[0], COLOR_PAIR(C_WINDOW));
	set_field_buffer(adress_field[0], 0, settings->adress);
	set_field_type(adress_field[0], TYPE_IPV4);
	adress_len = strlen(settings->adress);
	adress_form = new_form(adress_field);
	set_form_sub(adress_form, adress_wnd->content);
	post_form(adress_form);
	// Port input GUI
	port_wnd = make_window_relative(main_wnd, 3, 0,
		3, SETTINGS_WIDTH-2, "Port", TRUE);
	port_field[0] = new_field(1, PORT_LENGTH, 0, 0, 0, 0);
	port_field[1] = NULL;
	set_field_back(port_field[0], COLOR_PAIR(C_WINDOW));
	set_field_fore(port_field[0], COLOR_PAIR(C_WINDOW));
	set_field_buffer(port_field[0], 0, settings->port);
	set_field_type(port_field[0], TYPE_NUMERIC);
	port_len = strlen(settings->port);
	port_form = new_form(port_field);
	set_form_sub(port_form, port_wnd->content);
	post_form(port_form);
	wrefresh(port_wnd->content);

	do {
		key = form_handler(adress_form, &adress_len, ADRESS_LENGTH);
		if (BREAK_KEY(key))
			break;
		key = form_handler(port_form, &port_len, PORT_LENGTH);
	} while (!BREAK_KEY(key));

	if (key == '\n') {
		// Saving settings
		strncpy(settings->adress, field_buffer(adress_field[0], 0),
			adress_len);
		settings->adress[adress_len] = 0;
		strncpy(settings->port, field_buffer(port_field[0], 0),
			port_len);
		settings->port[port_len] = 0;
		save_settings(settings);

		// Implement connection here
	}

	unpost_form(adress_form);
	free_form(adress_form);
	free_field(adress_field[0]);
	unpost_form(port_form);
	free_form(port_form);
	free_field(port_field[0]);
	free_window(adress_wnd);
	free_window(port_wnd);
	free_window(main_wnd);
	update_panels();
	doupdate();
}

#define PADROWS 1024
#define MAIN_OPTS_NUM 4
tui_instance *make_interface(void)
{
	// Structure to return
	tui_instance *res_tui;
	// User pointers
	// res_tui->msg_window->panel
	WINDOW *msg_pad;
	MENU *main_menu;

	// Main menu options
	char *options[MAIN_OPTS_NUM] = {
		"Settings", "Connection", "Rooms", "Exit"
	};

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

	// Making window for main menu
	res_tui->menu_wnd = make_window(0, 0, 1, scr_max_x, "", FALSE);
	if (res_tui->menu_wnd == NULL) {
		free_interface(res_tui);
		return NULL;
	}
	keypad(res_tui->menu_wnd->box, TRUE);
	// Making menu
	main_menu = create_menu(options, MAIN_OPTS_NUM);
	if (main_menu == NULL) {
		free_interface(res_tui);
		return NULL;
	}
	// Changing default settings of menu
	set_menu_format(main_menu, 1, MAIN_OPTS_NUM);
	set_menu_mark(main_menu, "");
	set_menu_fore(main_menu, COLOR_PAIR(C_WINDOW));
	set_menu_back(main_menu, COLOR_PAIR(C_WINDOW));
	set_menu_spacing(main_menu, 0, 0, 4);
	set_menu_win(main_menu, res_tui->menu_wnd->box);
	// Outputing the result
	post_menu(main_menu);
	update_panels();
	doupdate();
	// Saving pointer
	set_panel_userptr(res_tui->menu_wnd->panel, main_menu);

	// Making windows
	res_tui->msg_wnd = make_window(1, 0, horizontal_sep,
		vertical_sep, "Messages", TRUE);
	if (res_tui->msg_wnd == NULL) {
		free_interface(res_tui);
		return NULL;
	}

	res_tui->users_wnd = make_window(1, vertical_sep, horizontal_sep,
		scr_max_x-vertical_sep, "Users list", TRUE);
	if (res_tui->users_wnd == NULL) {
		free_interface(res_tui);
		return NULL;
	}
	res_tui->input_wnd = make_window(horizontal_sep+1, 0,
		scr_max_y-horizontal_sep-1, scr_max_x, "Input", TRUE);
	if (res_tui->input_wnd == NULL) {
		free_interface(res_tui);
		return NULL;
	}

	// Adding pad to msg_wnd
	msg_pad = newpad(PADROWS, vertical_sep-2);
	// Initial settings of pad
	wbkgd(msg_pad, COLOR_PAIR(C_WINDOW));
	scrollok(msg_pad, TRUE);
	keypad(msg_pad, TRUE);
	set_panel_userptr(res_tui->msg_wnd->panel, msg_pad);
	if (msg_pad == NULL) {
		free_interface(res_tui);
		return NULL;
	}

	return res_tui;
}

void set_label(wnd_instance *wnd, char *label, int attrs)
{
	box(wnd->box, 0, 0);
	wattron(wnd->box, attrs);
	mvwaddstr(wnd->box, 0, 2, label);
	wattroff(wnd->box, attrs);
	wrefresh(wnd->box);
}

void pad_refresh(tui_instance *tui)
{
	WINDOW *pad = (WINDOW *)panel_userptr(tui->msg_wnd->panel);
	int beg_x, beg_y;
	int max_x, max_y;

	getbegyx(tui->msg_wnd->content, beg_y, beg_x);
	getmaxyx(tui->msg_wnd->content, max_y, max_x);
	max_x += beg_x;
	max_y += beg_y-1;

	prefresh(pad, PADROWS/2-max_y, 0, beg_y, beg_x, max_y, max_x);
}

void print_msg(tui_instance *tui, time_t raw_time,
	char *nickname, int color, char *msg)
{
	// Area of printing
	WINDOW *pad = (WINDOW *)panel_userptr(tui->msg_wnd->panel);
	// Variables to calculate number of rows to scroll
	int length;
	int rows;
	int max_x, max_y;
	// Structured time
	struct tm struct_time;
	// Formatted timestamp
	char format_time[32];

	// Getting hours, minutes, seconds
	localtime_r(&raw_time, &struct_time);
	sprintf(format_time, "[%02i:%02i:%02i] ",
		struct_time.tm_hour, struct_time.tm_min, struct_time.tm_sec);

	// Getting number of rows in message window
	length = strlen(msg) + strlen(format_time) + strlen(nickname) + 3;
	getmaxyx(tui->msg_wnd->content, max_y, max_x);
	rows = ceil((float)length/max_x);

	// Printing routines
	mvwaddstr(pad, PADROWS/2-1, 0, format_time);
	wattron(pad, color);
	waddstr(pad, nickname);
	wattroff(pad, color);
	waddstr(pad, ": ");
	waddstr(pad, msg);

	// Scrolling pad upwards and outputting
	wscrl(pad, rows);
	pad_refresh(tui);
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

void redraw_interface(tui_instance **tui)
{
	// Message to restore
	message_instance *message;

	// Removing old interface
	free_interface(*tui);
	// Making new one
	*tui = make_interface();
	// Restoring messages from history
	message = history.head;
	while (message != NULL) {
		print_msg(*tui, message->timestamp,
		message->nickname, COLOR_PAIR(message->color),
		message->text);
		message = message->next;
	}
}

#define IN_BUF_SIZE 256
int input_handler(tui_instance **tui)
{
	// Variable for restoring from history
	message_instance *message;
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

	// Making window look like an active
	set_label((*tui)->input_wnd, "Input", COLOR_PAIR(C_SELECT));
	// Making cursor visible
	curs_set(1);

	// Processing of keys
	buf[0] = 0;
	while ((key = wgetch(textbox)) != ERR) {
		switch (key) {

		case '\t':
		case KEY_F(12):
			curs_set(0);
			werase(textbox);
			set_label((*tui)->input_wnd, "Input",
				COLOR_PAIR(C_WINDOW));
			// Launching next handler
			// or exiting
			return key;

		case KEY_RESIZE:
			redraw_interface(tui);
			// Updating pointer to textbox
			textbox = (*tui)->input_wnd->content;
			// Reprinting buffer to textbox
			buf[len] = 0;
			waddstr(textbox, buf);
			// Restoring label attributes
			set_label((*tui)->input_wnd, "Input",
				COLOR_PAIR(C_SELECT));
			break;

		case KEY_BACKSPACE:
			if (len > 0) {
				if (buf_pos < len)
				// Removing character in the middle of line
					memmove(buf+buf_pos-1, buf+buf_pos,
						len-buf_pos);
				tboxmove(textbox, buf, -1, &buf_pos, len);
				wdelch(textbox);
				len--;
			}
			break;

		case KEY_LEFT:
			tboxmove(textbox, buf, -1, &buf_pos, len);
			break;

		case KEY_RIGHT:
			tboxmove(textbox, buf, 1, &buf_pos, len);
			break;

		case '\n':
			buf[len] = 0;

			message = fifo_push(&history, time(NULL),
				settings.nickname, settings.color, buf);
			print_msg(*tui, message->timestamp, message->nickname,
				COLOR_PAIR(message->color), message->text);
			werase(textbox);

			len = 0;
			buf_pos = 0;
			buf[0] = 0;
			break;

		default:
			// Last character for 0
			if (len < IN_BUF_SIZE - 1) {
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
	return key;
}

int msg_handler(tui_instance **tui)
{
	WINDOW *messages = (WINDOW *)panel_userptr((*tui)->msg_wnd->panel);
	int key;
	int position;

	position = 0;
	set_label((*tui)->msg_wnd, "Messages", COLOR_PAIR(C_SELECT));
	pad_refresh(*tui);
	while ((key = wgetch(messages)) != ERR) {
		switch (key) {
		case '\t':
		case KEY_F(12):
			set_label((*tui)->msg_wnd, "Messages",
				COLOR_PAIR(C_WINDOW));
			// Return to the initial position
			wscrl(messages, position);
			pad_refresh(*tui);
			return key;
		case KEY_RESIZE:
			redraw_interface(tui);
			set_label((*tui)->msg_wnd, "Messages",
				COLOR_PAIR(C_SELECT));
			pad_refresh(*tui);
			break;
		case KEY_UP:
			if (position > 0) {
				wscrl(messages, 1);
				pad_refresh(*tui);
				position--;
			}
			break;
		case KEY_DOWN:
			if (position < PADROWS/2) {
				wscrl(messages, -1);
				pad_refresh(*tui);
				position++;
			}
			break;
		}
	}
	return key;
}

int menu_handler(tui_instance **tui)
{
	MENU *menu = (MENU *)panel_userptr((*tui)->menu_wnd->panel);
	int key;
	int choice;

	// Displaying selection
	set_menu_fore(menu, COLOR_PAIR(C_SELECT));

	while (key = wgetch(menu_win(menu))) {
		if (key == '\n') {
			choice = item_index(current_item(menu));
			switch (choice) {
			case 0:
			key = get_settings(&settings);
			break;
			case 1:
			key = get_connection(&settings);
			break;
			case 3:
			key = KEY_F(12);
			break;
			}
			pad_refresh(*tui);
		}

		switch (key) {
		case ERR:
		case '\t':
		case KEY_F(12):
		// Removing selection
		set_menu_fore(menu, COLOR_PAIR(C_WINDOW));
		update_panels();
		doupdate();
		return key;

		case KEY_RIGHT:
		menu_driver(menu, REQ_RIGHT_ITEM);
		break;

		case KEY_LEFT:
		menu_driver(menu, REQ_LEFT_ITEM);
		break;

		case KEY_RESIZE:
		redraw_interface(tui);
		menu = (MENU *)panel_userptr((*tui)->menu_wnd->panel);
		set_menu_fore(menu, COLOR_PAIR(C_SELECT));
		break;
		}
	}
	return key;
}

void init_curses(void)
{
	// Regular initialization of ncurses
	initscr();

	// Input handling settings
	HDL_ERR_LOGGED(cbreak(), ERR, "Cbreak() failed", ERR);
	HDL_ERR_LOGGED(noecho(), ERR,
		"Couldn't enter noecho mode", ERR);

	// Color pairs initialization
	HDL_ERR_LOGGED(has_colors(), FALSE,
		"No colors in terminal", E_NOCOLOURS);
	HDL_ERR_LOGGED(start_color(), ERR,
		"The color table cannot be allocated", ERR);
	init_pair(C_WINDOW, COLOR_BLACK, COLOR_WHITE);
	init_pair(C_SELECT, COLOR_WHITE, COLOR_BLACK);

	init_pair(C_NICKBLACK,		COLOR_BLACK,	COLOR_WHITE);
	init_pair(C_NICKRED,		COLOR_RED,		COLOR_WHITE);
	init_pair(C_NICKGREEN,		COLOR_GREEN,	COLOR_WHITE);
	init_pair(C_NICKYELLOW,		COLOR_YELLOW,	COLOR_WHITE);
	init_pair(C_NICKMAGENTA,	COLOR_MAGENTA,	COLOR_WHITE);
	init_pair(C_NICKCYAN,		COLOR_CYAN,		COLOR_WHITE);
}

void inteface_handler(tui_instance **tui)
{
	int result;

	load_settings(&settings);
	while (1) {
		result = input_handler(tui);
		if (result == KEY_F(12))
			return;
		result = msg_handler(tui);
		if (result == KEY_F(12))
			return;
		result = menu_handler(tui);
		if (result == KEY_F(12))
			return;
	}
}