#include "precompiled.h"
#include "tui.h"

int main(int argn, char **argv)
{
	openlog(NULL, LOG_PID, LOG_USER);
	init_curses();
	draw_interface();
	endwin();
	closelog();
	return 0;
}