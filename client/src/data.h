#ifndef CUSTOM_DATA
#define CUSTOM_DATA

#define FIFO_LENGTH 10

typedef struct message_data_type {
	time_t timestamp;
	char *nickname;
	int attrs;
	char *text;
} message_data_instance;

typedef struct fifo_element_type {
	message_data_instance *data;
	struct fifo_element_type *next;
	struct fifo_element_type *prev;
} fifo_element_instance;

typedef struct fifo_type {
	fifo_element_instance *head;
	fifo_element_instance *tail;
	int total;
} fifo_instance;

fifo_element_instance *fifo_push(fifo_instance *fifo,
	message_data_instance *data);

void fifo_pop(fifo_instance *fifo);

#endif