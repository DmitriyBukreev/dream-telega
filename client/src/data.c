#include "data.h"

history_instance history = { NULL, NULL, 0 };

message_instance *fifo_push(history_instance *history,
	time_t timestamp, char *nickname, int attrs, char *text)
{
	// Creating new message instance
	message_instance *message;

	message = malloc(sizeof(message_instance));
	if (message == NULL)
		return NULL;

	message->nickname = malloc(strlen(nickname)+1);
	if (message->nickname == NULL) {
		free(message);
		return NULL;
	}

	message->text = malloc(strlen(text)+1);
	if (message->text == NULL) {
		free(message->nickname);
		free(message);
		return NULL;
	}

	// Adding data to message
	message->timestamp = timestamp;
	message->attrs = attrs;
	strcpy(message->nickname, nickname);
	strcpy(message->text, text);

	// Setting pointers of element
	message->prev = history->tail;
	message->next = NULL;

	// Setting pointers of descriptor
	if (history->tail != NULL)
		history->tail->next = message;
	else
		history->head = message;
	history->tail = message;

	history->total += 1;

	if (history->total > FIFO_LENGTH)
		fifo_pop(history);

	return message;
}

void fifo_pop(history_instance *history)
{
	message_instance *message;

	message = history->head;
	// Checking if fifo is empty
	if (message == NULL)
		return;

	// Setting pointers of history
	if (message->next != NULL)
		message->next->prev = NULL;
	else
		history->tail = NULL;
	history->head = message->next;

	// Releasing the memory
	free(message->nickname);
	free(message->text);
	free(message);

	history->total -= 1;
}