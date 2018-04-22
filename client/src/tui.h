#ifndef TUI_HEADER
#define TUI_HEADER

#define E_NOCOLOURS 304
#define E_NOWINDOW 305

#define C_WINDOW 1

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

typedef struct wnd_type {
	WINDOW *box;
	WINDOW *content;
	PANEL *panel;
} wnd_instance;

wnd_instance *draw_window(int y, int x, int height, int width, char *label);

void draw_interface(void);

void init_curses(void);

#endif