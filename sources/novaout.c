/*
Author: Stephen Hyberger
Date: 4.15.2021
File: novaout.c
Purpose: To implement all things output related. Uses locks to gatekeep access to the output to ensure that it is concurrent.
*/

//Necessary imports.
#include"novaout.h"

//Private functions.
void read_in(struct output_t *, uint64_t, int *, uint64_t *);

//Creates an output node.
struct output_t *output_create() {
	struct output_t *ret_val = malloc(sizeof(struct output_t));
	if(ret_val != NULL) {
		
		ret_val->out_file = NULL;
		ret_val->in_buf = NULL;
		pthread_mutex_init(&ret_val->lock, NULL);
		ret_val->cont_flag = 1;
	}
	return ret_val;
}

//Destroys a given output node.
void output_destroy(struct output_t **output) {
	if(output != NULL) {
		if(*output != NULL) {	

			//Free mutex
			pthread_mutex_destroy(&(*output)->lock);
			
			//Free node.
			free(*output);
		}
	}
}

//Takes in a buffer and an output node, sets the in buffer field of the node to the buffer, and returns whatever the old contents of the buffer field was.
struct buffer_t *output_set_in(struct output_t *output, struct buffer_t *buf) {
	struct buffer_t *ret_val = NULL;
	if(output != NULL) {

		//Get return value.
		ret_val = output->in_buf;

		//Set new value.
		output->in_buf = buf;
	}
	return ret_val;
}

//Takes in a file and an output node, sets the out file field of the node to the file, and return whatever the old contents of the file field was.
FILE *output_set_out(struct output_t *output, FILE *file) {
	FILE *ret_val = NULL;
	if(output != NULL) {

		//Get return value.
		ret_val = output->out_file;

		//Remove any native buffers. Buffering is done by nova.
		if(file != NULL) setbuf(file, NULL);

		//Set new value.
		output->out_file = file;
	}
	return ret_val;
}

//Do writing from buffer to output. A thread will run this until program termination or until it is put to sleep when it has no work and is to be awoken later to either work or terminate.
void *do_out(void *output_ptr) {

	//Check for null.
	if(output_ptr != NULL) {

		//Get handles for the structures to be used.
		struct output_t *output = (struct output_t *)output_ptr;
		struct buffer_t *buf = output->in_buf;
		//Check for null.
		if(buf != NULL) {

			//Variables on the stack to be used by this thread during output runtime.
			int current = 0;
			uint64_t diff = 0;

			//Loop until program termination begins.
			while(output->cont_flag) {

				//Lock the buffer. Do control logic.
				buffer_read_lock(buf);	

				//Check if write is ahead of read, if so read until write is reached.
				if(buf->cursor_write > buf->cursor_read) read_in(output, buf->cursor_write - buf->cursor_read, &current, &diff);	
				//Otherwise, the read is ahead of write, if so read until end of buffer.
				else read_in(output, buf->end - buf->cursor_read, &current, &diff);
				//Update the cursor.
				buf->cursor_read += diff;
				fflush(output->out_file);

				//Unlock the buffer. Do control logic.
				buffer_read_unlock(buf, diff);

				//Reset cursor counter.
				diff = 0;
			}
		}
	}
	pthread_exit(NULL);
}

//Do read into the output stream from the buffer upto max_size characters, from buffer buf to the provided output using the provided character and counter variables.
void read_in(struct output_t *output, uint64_t max_size, int *current, uint64_t *diff) {
	
	//Obtain a lock on the output file, and loop until the write cursor has been reached.
	pthread_mutex_lock(&output->lock);
	while(*diff < max_size) {
		//Get the current character from the buffer.
		*current = (int)*((char *)(output->in_buf->cursor_read + (*diff)++));
		//Unlock the buffer, to enable input to interact with it in the case of I/O delays.
		pthread_mutex_unlock(&output->in_buf->lock);
		//Put the current character onto the file.
		fputc(*current, output->out_file);
		//Relock the buffer.
		pthread_mutex_lock(&output->in_buf->lock);
	}

	//Unlock the file.
	pthread_mutex_unlock(&output->lock);
}
