/*
Author: Stephen Hyberger
Date: 4.15.2021
File: novapipe.c
Purpose: Provide functions to manipulate the modules and elements of the Nova application pipeline in a thread-safe manner.
*/

//Necessary imports.
#include"novapipe.h"

//Create pipeline buffer.
struct buffer_t *buffer_create(uint64_t size) {
	
	//Variable to return at the end. Attempt allocation.
	struct buffer_t *ret_val = malloc(sizeof(struct buffer_t));

	//If allocation is successful. allocate the buffer inside the wrapper.
	if(ret_val != NULL) {
		ret_val->memory = malloc(size);

		//If allocation is successful. Initialize
		if(ret_val->memory != NULL) {
			pthread_mutex_init(&(ret_val->lock), NULL);
			pthread_cond_init(&(ret_val->cond_write), NULL);
			pthread_cond_init(&(ret_val->cond_read), NULL);
			ret_val->size = size;
			ret_val->end = ret_val->memory + size;
			ret_val->cursor_read = ret_val->memory;
			ret_val->cursor_write = ret_val->memory;
			ret_val->num_sleeping_writers = 0;
			ret_val->num_sleeping_readers = 0;
			ret_val->wait = 0;
			ret_val->state = -1;

		//Otherwise free struct and null it.
		} else {
			free(ret_val);
			ret_val = NULL;
		}
	}
	
	//Return buffer.
	return ret_val;
}

//Deallocate buffer.
void buffer_destroy(struct buffer_t **buf) {

	//Check for null.
	if(buf != NULL) {
		if(*buf != NULL) {

			//If buffer inside wrapper isn't null, free it.
			if((*buf)->memory != NULL) {
				free((*buf)->memory);
			}

			//Free the pthread library structures.
			pthread_mutex_destroy(&((*buf)->lock));
			pthread_cond_destroy(&((*buf)->cond_write));
			pthread_cond_destroy(&((*buf)->cond_read));

			//Free the buffer.
			free(*buf);
		}
	}
}

//Locks lock for writer. Does control logic.
void buffer_write_lock(struct buffer_t *buf) {
	//Obtain buffer lock.
	pthread_mutex_lock(&(buf->lock));

	//Check if buffer state is set to full. If full, increment the sleeping writers counter and sleep until signaled by output.
	if(buf->state == 1) {
		++buf->num_sleeping_writers;
		while(buf->state == 1) {
			pthread_cond_wait(&buf->cond_write, &buf->lock);
		}
		--buf->num_sleeping_writers;
	}

	//Check if input has been instructed to wait for output to obtain the lock, and if so wait until signaled by output that the lock has been obtained.
	//This is used to ensure that each wake is successful.
	while(buf->wait == 1) pthread_cond_wait(&buf->cond_write, &buf->lock);
	//Check if output is waiting for a signal from input to resume after having woken input. If so reset the wait flag and signal output.
	if(buf->wait == -1) {
		buf->wait = 0;
		pthread_cond_signal(&buf->cond_read);
	}
}

//Unlocks lock for writer. Does control logic.
void buffer_write_unlock(struct buffer_t *buf, uint64_t diff) {

	//Reset the cursor to the start if the end has been reached.
	if(buf->cursor_write >= buf->end) buf->cursor_write = buf->memory;
	//Set the buffer state to full if the write cursor has met with the read cursor.
	if(buf->cursor_read == buf->cursor_write) buf->state = 1;
	//Otherwise, the buffer state is to be a 0, or normal.
	else buf->state = 0;	

	//Check if writing was done and if there are any readers that are sleeping due to an empty buffer.
	//If so set wait flag to 1, to force input to wait on output, and signal output to wake.
	if(diff > 0 && buf->num_sleeping_readers > 0) {
		buf->wait = 1;
		pthread_cond_signal(&(buf->cond_read));
	}
	
	//Unlock the buffer.
	pthread_mutex_unlock(&(buf->lock));
}

//Locks lock for reader. Does control logic.
void buffer_read_lock(struct buffer_t *buf) {

	//Obtain the buffer lock.
	pthread_mutex_lock(&(buf->lock));
	
	//Check if buffer state is empty. If so wait until input signals that there is output to read.
	if(buf->state == -1) {
		++buf->num_sleeping_readers;
		while(buf->state == -1) pthread_cond_wait(&buf->cond_read, &buf->lock);
		--buf->num_sleeping_readers;
	}
	
	//Check if the wait flag indicates that output needs to wait for input to start, and wait if necessary.
	//This prevents output from locking input out after signaling input to wake back up.
	while(buf->wait == -1) pthread_cond_wait(&buf->cond_read, &buf->lock);
	//Check if the wait flag indicates that input was just woken, and needs to wake output up to resume normal execution.
	
	if(buf->wait == 1) {
		buf->wait = 0;
		pthread_cond_signal(&buf->cond_write);
	}
}

//Unlocks lock for reader. Does control logic.
void buffer_read_unlock(struct buffer_t *buf, uint64_t diff) {

	//Check if the read cursor has reached the end and if so reset it to the start.
	if(buf->cursor_read >= buf->end) buf->cursor_read = buf->memory;
	//Check if the read cursor has met the write cursor, if so set the buffer state to empty.
	if(buf->cursor_read == buf->cursor_write) buf->state = -1;
	//Otherwise, set buffer state to 0, or normal.
	else buf->state = 0;	

	//Check if anything was read and if there are sleeping writers waiting for free space to write to in the buffer.
	//If so, set the wait flag to -1 to force output to wait for input to wake, and wake input.
	if(diff > 0 && buf->num_sleeping_writers > 0) {
		buf->wait = -1;
		pthread_cond_signal(&(buf->cond_write));
	}

	//Unlock the buffer.
	pthread_mutex_unlock(&(buf->lock));
}

//Methods that create and destroy context stacks.
struct context_queue_t *context_queue_create() {
	struct context_queue_t *ret_val = malloc(sizeof(struct context_queue_t));
	if(ret_val != NULL) {
		pthread_mutex_init(&ret_val->lock, NULL);
		ret_val->start = NULL;
		ret_val->end = NULL;
	}
	return ret_val;
}
void context_queue_destroy(struct context_queue_t **queue) {
	if(queue != NULL) {
		if(*queue != NULL) {
			do {
				context_remove(*queue);
			} while((*queue)->end != NULL);
			pthread_mutex_destroy(&(*queue)->lock);
			free(*queue);
		}
	}
}

//Functions that create and destroy context elements.
struct context_element_t *context_element_create() {
	struct context_element_t *ret_val = malloc(sizeof(struct context_element_t));
	if(ret_val != NULL) {
		ret_val->next = NULL;
		ret_val->back = NULL;
		ret_val->start = 0;
		ret_val->size = 0;
		ret_val->read = 0;
		ret_val->type = 0;
		ret_val->complete = 0;
	}
	return ret_val;
}
void context_element_destroy(struct context_element_t **element) {
	if(element != NULL) {
		if(*element != NULL) {
			free(*element);
		}
	}
}

/*
void context_enqueue(struct context_queue_t *queue, struct context_element_t *element) {
	if(queue != NULL && element != NULL) {
		pthread_mutex_lock(&queue->lock);

		element->next = queue->start;
		if(queue->start != NULL) {
			element->start = queue->start->start + queue->start->size;
			queue->start->back = element;
			queue->start->complete = 1;
		}
		queue->start = 1;

		pthread_mutex_unlock(&queue->lock);
	}
}

struct context_element_t *context_dequeue(struct context_queue_t *queue) {

}*/

void context_append(struct context_queue_t *queue, uint64_t type) {
	if(queue != NULL) {
		struct context_element_t *new_context_element = context_element_create();
		if(new_context_element != NULL) {
			new_context_element->size = 0;
			pthread_mutex_lock(&queue->lock);
			new_context_element->next = queue->start;
			if(queue->start != NULL) {
				new_context_element->start = queue->start->start + queue->start->size;
				queue->start->back = new_context_element;
				queue->start->type = type;
				queue->start->complete = 1;
			}
			queue->start = new_context_element;
			if(queue->end == NULL) queue->end = new_context_element;	
			pthread_mutex_unlock(&queue->lock);
		}
	}
}
void context_increment(struct context_queue_t *queue, uint64_t sum) {
	if(queue != NULL) {
		pthread_mutex_lock(&queue->lock);
		if(queue->start == NULL) {
			queue->start = context_element_create();
			if(queue->start != NULL) {
				queue->end = queue->start;
			} else {
				pthread_mutex_unlock(&queue->lock);
				return;
			}
		}
		queue->start->size += sum;
		pthread_mutex_unlock(&queue->lock);
	}
}
void context_read_inc(struct context_queue_t *queue, uint64_t sum) {
	if(queue != NULL) if(queue->end != NULL) queue->end->read += sum;
}
uint64_t context_size(struct context_queue_t *queue) {
	uint64_t ret_val = 0;
	if(queue != NULL) if(queue->end != NULL) ret_val = queue->end->size;
	return ret_val;
}
uint64_t context_read(struct context_queue_t *queue) {
	uint64_t ret_val = 0;
	if(queue != NULL) if(queue->end != NULL) ret_val = queue->end->size;
	return ret_val;
}
uint64_t context_type(struct context_queue_t *queue) {
	uint64_t ret_val = 0;
	if(queue != NULL) if(queue->end != NULL) ret_val = queue->end->type;
	return ret_val;
}

/*void context_enqueue(struct context_queue_t *queue, struct context_element_t *element) {
	if(queue != NULL && element != NULL) {
		pthread_mutex_lock(&queue->lock);
		element->next = queue->start;
		if(queue->start != NULL) queue->start->back = element;
		queue->start = element;
		if(queue->end == NULL) queue->end = element;
		pthread_mutex_unlock(&queue->lock);
	}
}*/
char context_remove(struct context_queue_t *queue) {
	char ret_val = 0;
	if(queue != NULL) {
		pthread_mutex_lock(&queue->lock);
		if(queue->end != NULL && queue->end->complete == 1) {
			if(queue->end == queue->start) {
				context_element_destroy(&queue->end);
				queue->end = NULL;
				queue->start = NULL;
			} else {
				queue->end = queue->end->back;
				context_element_destroy(&queue->end->next);
				queue->end->next = NULL;
			}
			ret_val = 1;
		}
		pthread_mutex_unlock(&queue->lock);
	}
	return ret_val;
}
/*struct context_element_t *context_tail_peek(struct context_queue_t *queue) {
	struct context_element_t *ret_val = NULL;
	if(queue != NULL) {
		pthread_mutex_lock(&queue->lock);
		ret_val = queue->start;
		pthread_mutex_unlock(&queue->lock);
	}
	return ret_val;
}

struct context_element_t *context_front_peek(struct context_queue_t *queue) {
	struct context_element_t *ret_val = NULL;
	if(queue != NULL) {
		pthread_mutex_lock(&queue->lock);
		ret_val = queue->end;
		pthread_mutex_unlock(&queue->lock);
	}
	return ret_val;
}

char context_is_complete(struct context_element_t *element) {
	char ret_val = 0;
	if(element != NULL) {
		pthread_mutex_lock(&element->lock);
		ret_val = element->complete;
		pthread_mutex_unlock(&element->lock);
	}
	return ret_val;
}
void context_mark_complete(struct context_element_t *element) {
	if(element != NULL) {
		pthread_mutex_lock(&element->lock);
		element->complete = 1;
		pthread_mutex_unlock(&element->lock);
	}
}
void context_increment(struct context_element_t *element, uint64_t sum) {
	if(element != NULL) {
		pthread_mutex_lock(&element->lock);
		element->size += sum;
		pthread_mutex_unlock(&element->lock);
	}
}*/
