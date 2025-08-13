/*
Author: Stephen Hyberger
Date: 4.15.2021
File: novapipe.h
Purpose: A header to define Nova application pipeline elements and functions.
*/

#ifndef NOVAPIPE_H
#define NOVAPIPE_H

//Necessary imports.
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<pthread.h>

//Pipeline structs.
//Struct for the buffers between the pipeline stages. Is a circular buffer with a read and a write cursor.
struct buffer_t {
	void *memory;
	void *end;
	void *cursor_read;
	void *cursor_write;
	uint64_t size;
	pthread_mutex_t lock;
	pthread_cond_t cond_write;
	pthread_cond_t cond_read;
	unsigned char num_sleeping_writers;
	unsigned char num_sleeping_readers;
	char wait;
	char state;
};
//A struct to establish context for each element discovered during parsing. This is put on a stack shared between the parser and the interpreter.
struct context_element_t {
	struct context_element_t *next;
	struct context_element_t *back;
	uint64_t start;
	uint64_t size;
	uint64_t read;
	char complete;
	char type;
};
struct context_queue_t {
	pthread_mutex_t lock;
	struct context_element_t *start;
	struct context_element_t *end;
};

//Function prototypes.
//Protypes for manipulating the buffers.
struct buffer_t *buffer_create(uint64_t);
void buffer_destroy(struct buffer_t **);
void buffer_write_lock(struct buffer_t *);
void buffer_write_unlock(struct buffer_t *, uint64_t);
void buffer_read_lock(struct buffer_t *);
void buffer_read_unlock(struct buffer_t *, uint64_t);

//Queue functions for context.
struct context_queue_t *context_queue_create();
void context_queue_destroy(struct context_queue_t **);
struct context_element_t *context_element_create();
void context_element_destroy(struct context_element_t **);
void context_append(struct context_queue_t *, uint64_t);
char context_remove(struct context_queue_t *);
void context_increment(struct context_queue_t *, uint64_t);
void context_read_inc(struct context_queue_t *, uint64_t);
uint64_t context_type(struct context_queue_t *);
uint64_t context_read(struct context_queue_t *);
uint64_t context_size(struct context_queue_t *);

#endif
