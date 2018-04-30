#include "data.h"

fifo_instance history = { NULL, NULL, 0 };

fifo_element_instance *fifo_push(fifo_instance *fifo,
	message_data_instance *data)
{
	// Creating new element instance
	fifo_element_instance *element =
		malloc(sizeof(fifo_element_instance));
	if (element == NULL)
		return NULL;

	// Adding data to element
	element->data = malloc(sizeof(message_data_instance));
	*(element->data) = *(data);
	element->data->nickname = malloc(strlen(data->nickname));
	strcpy(element->data->nickname, data->nickname);
	element->data->text = malloc(strlen(data->text));
	strcpy(element->data->text, data->text);

	// Setting pointers of element
	element->prev = fifo->tail;
	element->next = NULL;

	// Setting pointers of descriptor
	if (fifo->tail != NULL)
		fifo->tail->next = element;
	else
		fifo->head = element;
	fifo->tail = element;

	fifo->total += 1;

	if (fifo->total > FIFO_LENGTH)
		fifo_pop(fifo);

	return element;
}

void fifo_pop(fifo_instance *fifo)
{
	fifo_element_instance *element;

	element = fifo->head;
	// Checking if fifo is empty
	if (element == NULL)
		return;

	// Setting pointers of fifo
	if (element->next != NULL)
		element->next->prev = NULL;
	else
		fifo->tail = NULL;
	fifo->head = element->next;

	// Releasing the memory
	free(element->data->nickname);
	free(element->data->text);
	free(element->data);
	free(element);

	fifo->total -= 1;
}