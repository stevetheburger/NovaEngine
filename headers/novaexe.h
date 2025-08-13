/*
Author: Stephen Hyberger
File: novaexe.h
Date: 3.25.2021
Purpose: Defines functions that are executed on the c-level as well as the mechanisms for executing them.
*/

#ifndef NOVAEXE_H
#define NOVAEXE_H

//Necessary imports.
#include"novapipe.h"
#include<stdlib.h>

//Struct for the execution header.
struct execute_t {
	struct execute_stack_t *stack;
	struct execute_stack_t *back_stack;
	char cont_flag;
};

//Structs for the execution queue.
struct execute_element_t {
	void *value;
	struct execute_element_t *next;
	char type_id;
};
struct execute_stack_t {	
	pthread_mutex_t lock;
	pthread_cond_t exe_wait;
	struct execute_element_t *top;
};

//Functions for execution control.
struct execute_t *execute_create();
void execute_destroy(struct execute_t **);
struct execute_stack_t *execute_set_stack(struct execute_t *, struct execute_stack_t *);
void *do_execute(void *);
void nova_run(struct execute_stack_t *, struct execute_stack_t *, struct execute_element_t *);

//Helper methods.
//String to integer converter.
//int nova_int_conv(void *, uint64_t *);

//Queue function for execution.
//struct execute_params_t *execute_params_create(uint64_t);
//void execute_params_destroy(struct execute_params_t **);
//struct execute_queue_t *execute_queue_create();
//void execute_queue_destroy(struct execute_queue_t **);
struct execute_element_t *execute_element_create(size_t, char);
void execute_element_destroy(struct execute_element_t **);
struct execute_element_t *execute_stack_pop(struct execute_stack_t *);
struct execute_element_t *execute_stack_peek(struct execute_stack_t *);
void execute_stack_push(struct execute_stack_t *, struct execute_element_t *);
struct execute_stack_t *execute_stack_create();
void execute_stack_destroy(struct execute_stack_t **);
//void execute_enqueue(struct execute_queue_t *, struct execute_element_t *);
//struct execute_element_t *execute_dequeue(struct execute_queue_t *);

//Write add/sub commands here.

#endif
