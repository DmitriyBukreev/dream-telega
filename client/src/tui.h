#ifndef TUI_HEADER
#define TUI_HEADER

#include "data.h"

#define E_NOCOLOURS	  304
#define E_NOWINDOW 	  305

#define C_WINDOW	  1
#define C_SELECT	  2
#define C_NICKBLACK	  3
#define C_NICKRED	  4
#define C_NICKGREEN	  5
#define C_NICKYELLOW  6
#define C_NICKMAGENTA 7
#define C_NICKCYAN	  8

#define HDL_ERR_LOGGED(statement, error, message, code) \
do { \
	if ((statement) == (error)) { \
		syslog(LOG_ERR, "[ERR] %s", message); \
		endwin(); \
		exit(code); \
	} \
} while (0)

#define HDL_NOTERR_LOGGED(statement, error, message, code) \
do { \
	if ((statement) != (error)) { \
		syslog(LOG_ERR, "[ERR] %s", message); \
		endwin(); \
		exit(code); \
	} \
} while (0)

#define DEBUG_MSG(message) \
syslog(LOG_INFO, "[DEBUG] %s", message)

typedef struct wnd_type {
	WINDOW *box;
	WINDOW *content;
	PANEL *panel;
} wnd_instance;

typedef struct menu_type
{
	WINDOW *background;
	PANEL *panel;
	MENU *menu;	
} menu_instance;

typedef struct  tui_type {
	wnd_instance* msg_wnd;
	wnd_instance* input_wnd;
	wnd_instance* users_wnd;
	menu_instance* main_menu;
} tui_instance;

tui_instance *make_interface(void);

void free_interface(tui_instance *tui);

int input_handler(tui_instance **tui);

int msg_handler(tui_instance **tui);

int menu_handler(tui_instance **tui);

void init_curses(void);

extern history_instance history;

#endif