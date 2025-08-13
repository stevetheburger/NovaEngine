/*
Author: Stephen Hyberger
Date: 4.17.2021
File: novacmp.h
Purpose: Header file for the language specification.
*/

#ifndef NOVACMP_H
#define NOVACMP_H

//Necessary imports.
#include"novaops.h"
#include"novapipe.h"
#include"novaexe.h"
#include<stdlib.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

struct dictionary_t {
	unsigned char **dict;
	unsigned char num_alpha;
	unsigned char num_rules;
};
//A struct for parsing data.
struct parse_t {
	struct buffer_t *in_buf;
	struct buffer_t *out_buf;
	struct context_queue_t *context_queue;
	//uint64_t current_rule;
	//uint64_t rule_count;
	//uint64_t prod_count;
	//uint64_t next_rule;
	//uint64_t next_prod_count;
	unsigned char current_context;
	unsigned char last_context;
	unsigned char current_prod;
	//char next_prod;
	char cont_flag;
	struct dictionary_t *dict;
};

//A struct for generating nova instructions.
struct interpret_t {
	struct buffer_t *in_buf;
	struct context_queue_t *context_queue;
	struct execute_stack_t *execute_stack;
	struct execute_stack_t *back_stack;
	struct execute_element_t *current_element;
	uint64_t current_count;
	uint64_t current_type;
	uint64_t current_size;
	char neg_flag;
	char cont_flag;
};

//Function prototypes for parsing.
struct parse_t *parse_create();
void parse_destroy(struct parse_t **);
struct buffer_t *parse_set_in(struct parse_t *, struct buffer_t *);
struct buffer_t *parse_set_out(struct parse_t *, struct buffer_t *);
struct context_queue_t *parse_set_queue(struct parse_t *, struct context_queue_t *);
void *do_parse(void *);

//Function prototypes for interpreting.
struct interpret_t *interpret_create();
void interpret_destroy(struct interpret_t **);
struct buffer_t *interpret_set_in(struct interpret_t *, struct buffer_t *);
struct context_queue_t *interpret_set_queue(struct interpret_t *, struct context_queue_t *);
struct execute_stack_t *interpret_set_stack(struct interpret_t *, struct execute_stack_t *);
//void interpret_elements_resize(struct interpret_t *, uint64_t new_size);
void *do_interpret(void *);

//Function prototypes for conversion from console text to something usable by the execution environment.
void console_to_int(struct interpret_t *, int *);
#endif
