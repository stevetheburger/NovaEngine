/*
Author: Stephen Hyberger
Date: 4.15.2021
File: novain.c
Purpose: To implement all things input related. Uses locks to gatekeep access to input in order to ensure concurrency.
*/

//Necessary imports.
#include"novain.h"

//Private functions.
void write_out(struct input_t *, uint64_t, int *, uint64_t *);

//Creates input node
struct input_t *input_create() {
	struct input_t *ret_val = malloc(sizeof(struct input_t));
	if(ret_val != NULL) {

		ret_val->in_file = NULL;
		ret_val->out_buf = NULL;
		pthread_mutex_init(&ret_val->lock, NULL);
		ret_val->cont_flag = 1;
	}
	return ret_val;
}

//Destroys a provided input node.
void input_destroy(struct input_t **input) {
	if(input != NULL) {
		if(*input != NULL) {

			//Free the input node mutex.
			pthread_mutex_destroy(&(*input)->lock);
			
			//Free input struct.
			free(*input);
		}
	}
}

//Takes in a file pointer and an input node, set the in file of the node to the file provided, and returns whatever the old contents of the in file field was.
FILE *input_set_in(struct input_t *input, FILE *file) {

	FILE *ret_val = NULL;
	if(input != NULL) {
		
		//Set the return variable to the current value.
		ret_val = input->in_file;

		//Remove any native buffers.
		if(file != NULL) setbuf(file, NULL);

		//Set the new value.
		input->in_file = file;
	}
	return ret_val;
}

//Takes in a buffer struct and an input node, sets the out buffer of the input to the buffer, and returns whatever the old contents of the out buffer field was.
struct buffer_t *input_set_out(struct input_t *input, struct buffer_t *buf) {
	
	struct buffer_t *ret_val = NULL;
	if(input != NULL) {
		
		//Set the return variable to the current value.
		ret_val = input->out_buf;
		
		//Set the new value.
		input->out_buf = buf;
	}
	return ret_val;
}

//Do reading from input to buffer. A thread will run this until program termination or until it is put to sleep when it has no work and is to be awoken later to either work or terminate.
void *do_in(void *input_ptr) {

	//Check for null.
	if(input_ptr != NULL) {

		//Get handles for structures to be used.
		struct input_t *input = (struct input_t *)input_ptr;
		struct buffer_t *buf = input->out_buf;
		//Check for null.
		if(buf != NULL) {

			//Variables on the stack to be used by this thread during runtime.
			uint64_t diff = 0;
			int current = 0;	

			//Loop until termination signal
			while(input->cont_flag) {
				
				//Lock the buffer. Do control logic.
				buffer_write_lock(buf);	

				//If read is ahead of write, write until read.
				if(buf->cursor_write < buf->cursor_read) write_out(input, buf->cursor_read - buf->cursor_write, &current, &diff);
				//If read is behind write, write until end.
				else write_out(input, buf->end - buf->cursor_write, &current, &diff);
				//Update the cursor.
				buf->cursor_write += diff;

				//Unlock buffer. Do control logic.
				buffer_write_unlock(buf, diff);

				//Reset the cursor counter.
				diff = 0;
			}
		}
	}
	pthread_exit(NULL);
}

//Write from input to buffer buf up to max_size characters, using the provided character holder and counter.
void write_out(struct input_t *input, uint64_t max_size, int *current, uint64_t *diff) {

	//Get lock on the input file, and loop until the read cursor has been reached. 
	pthread_mutex_lock(&input->lock);
	while(*diff < max_size) {

		//Unlocks the buffer lock, to enable output to do work while there is any I/O latency.
		pthread_mutex_unlock(&input->out_buf->lock);
		//Gets input of one character.
		*current = fgetc(input->in_file);	
		//Checks if character is a line end or a carriage return, if so replace with a null character to indicate end of input.
		if(*current == '\n' || *current == '\r') {
			*((char *)(input->out_buf->cursor_write + (*diff)++)) = '\0';
			break;
		}
		//Relock the buffer and write to the buffer.
		pthread_mutex_lock(&input->out_buf->lock);
		*((char *)(input->out_buf->cursor_write + (*diff)++)) = *((char *)current);
	}

	//Unlock the file.
	pthread_mutex_unlock(&input->lock);
}
