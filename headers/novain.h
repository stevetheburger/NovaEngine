/*
Author: Stephen Hyberger
Date: 4.15.2021
File: novain.h
Purpose: Header file for the input functions and elements.
*/

#ifndef NOVAIN_H
#define NOVAIN_H

//Necessary imports.
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include"novapipe.h"

//struct to store input variables.
struct input_t {
	FILE *in_file;
	struct buffer_t *out_buf;
	pthread_mutex_t lock;
	char cont_flag;
};

//Prototypes for the input control.
struct input_t *input_create();
void input_destroy(struct input_t **);
FILE *input_set_in(struct input_t *, FILE *);
struct buffer_t *input_set_out(struct input_t *, struct buffer_t *);
void *do_in(void *);

#endif
