#ifndef CUSTOM_DATA
#define CUSTOM_DATA

#define FIFO_LENGTH 100

typedef struct message_type {
	time_t timestamp;
	char *nickname;
	int attrs;
	char *text;
	struct message_type *next;
	struct message_type *prev;
} message_instance;

typedef struct history_type {
	message_instance *head;
	message_instance *tail;
	int total;
} history_instance;

message_instance *fifo_push(history_instance *history,
	time_t timestamp, char *nickname, int attrs, char *text);

void fifo_pop(history_instance *fifo);

#endif