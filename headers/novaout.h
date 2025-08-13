/*
Author: Stephen Hyberger
Date: 4.15.2021
File: novaout.h
Purpose: Header file for the output functions and elements.
*/

#ifndef NOVAOUT_H
#define NOVAOUT_H

//Necessary imports.
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include"novapipe.h"

//Struct to store output variables.
struct output_t {
	FILE *out_file;
	struct buffer_t *in_buf;
	pthread_mutex_t lock;
	char cont_flag;
};

//Prototypes for the output control.
struct output_t *output_create();
void output_destroy(struct output_t **);
struct buffer_t *output_set_in(struct output_t *, struct buffer_t *);
FILE *output_set_out(struct output_t *, FILE *);
void *do_out(void *);

#endif
