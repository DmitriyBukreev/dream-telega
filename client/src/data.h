#ifndef CUSTOM_DATA
#define CUSTOM_DATA

#define FIFO_LENGTH 100
#define NAME_LENGTH 32
#define ADRESS_LENGTH 16
#define PORT_LENGTH 6
#define SETTINGS_PATH "settings"

typedef struct message_type {
	time_t timestamp;
	char *nickname;
	int color;
	char *text;
	struct message_type *next;
	struct message_type *prev;
} message_instance;

typedef struct history_type {
	message_instance *head;
	message_instance *tail;
	int total;
} history_instance;

typedef struct settings_type {
	char nickname[NAME_LENGTH];
	int color;
	char adress[ADRESS_LENGTH];
	char port[PORT_LENGTH];
} settings_instance;

message_instance *fifo_push(history_instance *history,
	time_t timestamp, char *nickname, int attrs, char *text);

void fifo_pop(history_instance *fifo);

int save_settings(settings_instance *settings);

int load_settings(settings_instance *settings);

extern history_instance history;
extern settings_instance settings;

#endif