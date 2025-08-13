/*
Author: Stephen Hyberger
Date: 3.25.2021
File: novamain.c
Purpose: To serve as the entry point to the Nova program.
*/

//Necessary imports.
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include"novapipe.h"
#include"novain.h"
#include"novaout.h"
#include"novacmp.h"
#include"novaexe.h"

const int NUM_PIPE_THREADS = 4;
const int BUFFER_SIZE = 1024;

//A temporarily empty main method. Will serve as the entry point to the Nova program.
int main() {

	//Print console header.
	printf("\n\nNova 0.0.0\n\n");

	//Create thread pool.
	pthread_t *threads = malloc(sizeof(pthread_t) * NUM_PIPE_THREADS);

	//Create shared pipeline elements.
	struct buffer_t *buffer_in = buffer_create(BUFFER_SIZE);
	struct buffer_t *buffer_parse = buffer_create(BUFFER_SIZE);
	struct context_queue_t *context_queue = context_queue_create();
	struct execute_stack_t *execute_stack = execute_stack_create();

	//Create pipeline nodes.
	struct input_t *input = input_create();
	struct parse_t *parse = parse_create();
	struct interpret_t *interpret = interpret_create();
	struct execute_t *execute = execute_create();

	//Before proceeding, check that the thread pool, the pipeline elements, and the pipeline nodes are all non null.
	if(threads != NULL && input != NULL && parse != NULL && interpret != NULL && execute != NULL && buffer_in != NULL && buffer_parse != NULL && context_queue != NULL && execute_stack != NULL) {

		//Set up input node.
		input_set_in(input, stdin);
		input_set_out(input, buffer_in);

		//Set up parse node.
		parse_set_in(parse, buffer_in);
		parse_set_out(parse, buffer_parse);
		parse_set_queue(parse, context_queue);

		//Set up interpret node.
		interpret_set_in(interpret, buffer_parse);
		interpret_set_queue(interpret, context_queue);
		interpret_set_stack(interpret, execute_stack);
		
		//Set up execute node.
		execute_set_stack(execute, execute_stack);

		//Activate pipeline nodes by starting off thread pool.
		pthread_create(&threads[0], NULL, do_in, (void *)input);
		pthread_create(&threads[1], NULL, do_parse, (void *)parse);
		pthread_create(&threads[2], NULL, do_interpret, (void *)interpret);
		pthread_create(&threads[3], NULL, do_execute, (void *)execute);

		//Wait for pipeline nodes to stop.
		for(int i = 0; i < NUM_PIPE_THREADS; ++i) pthread_join(threads[i], NULL);
	}

	//Afterward, teardown, whether successful or not.
	//Ensure that all pointers are checked for null to prevent memory errors. The destroy functions have the check built in internally.

	//Free pipeline nodes.
	input_destroy(&input);
	parse_destroy(&parse);
	interpret_destroy(&interpret);
	execute_destroy(&execute);

	//Free shared pipeline elements.
	buffer_destroy(&buffer_in);
	buffer_destroy(&buffer_parse);
	context_queue_destroy(&context_queue);
	execute_stack_destroy(&execute_stack);

	//Free thread pool.
	if(threads != NULL) free(threads);

	//Print space.
	printf("\n\n");

	//End program.
	pthread_exit(NULL);
}
