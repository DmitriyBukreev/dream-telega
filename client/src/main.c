#include "precompiled.h"
#include "tui.h"

void inteface_handler(tui_instance **tui)
{
	int result;

	while (1) {
		result = input_handler(tui);
		if (result == KEY_F(12))
			return;
	}
}

int main(int argn, char **argv)
{
	tui_instance *tui;

	openlog(NULL, LOG_PID, LOG_USER);
	init_curses();
	tui = make_interface();
	inteface_handler(&tui);
	free_interface(tui);
	endwin();
	closelog();
	return 0;
}