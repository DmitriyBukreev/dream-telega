#include "precompiled.h"
#include "tui.h"

int main(int argn, char **argv)
{
	tui_instance *tui;

	openlog(NULL, LOG_PID, LOG_USER);
	setlocale(LC_ALL, "Russian");
	init_curses();
	tui = make_interface();
	inteface_handler(&tui);
	free_interface(tui);
	endwin();
	closelog();
	return 0;
}