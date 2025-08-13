/*
Author: Stephen Hyberger
File: novaexe.c
Date: 3.25.2021
Purpose: To define nova execution line functions that do Nova's work at the C-level.
*/

//Necessary imports.
#include"novaexe.h"
#include<stdlib.h>

typedef int (*binary_op)(int, int);

//Creates execution node.
struct execute_t *execute_create() {

	//Allocate.
	struct execute_t *ret_val = malloc(sizeof(struct execute_t));
	struct execute_stack_t *back_stack = execute_stack_create();
	//Initialize.
	if(ret_val != NULL && back_stack != NULL) {
		
		ret_val->stack = NULL;
		ret_val->back_stack = back_stack;
		ret_val->cont_flag = 1;

	//Ensure that node creation is atomic.
	} else {
		if(ret_val != NULL) {
			free(ret_val);
			ret_val = NULL;
		}
		if(back_stack != NULL) execute_stack_destroy(&back_stack);
	}
	return ret_val;
}

//Destroys a given execution node.
void execute_destroy(struct execute_t **execute) {
	if(execute != NULL) {
		if(*execute != NULL) {

			//Free back stack.
			if((*execute)->back_stack != NULL) execute_stack_destroy(&((*execute)->back_stack));
			
			//Free node.
			free(*execute);
		}
	}
}

//Takes an execution stack and an execution node, sets the execution stack field of the node to the stack, and returns whatever the old value of the field was.
struct execute_stack_t *execute_set_stack(struct execute_t *execute, struct execute_stack_t *stack) {
	struct execute_stack_t *ret_val = NULL;
	if(execute != NULL) {

		//Get return value.
		ret_val = execute->stack;

		//Set new value.
		execute->stack = stack;
	}
	return ret_val;
}

void *do_execute(void *execute_ptr) {
	if(execute_ptr != NULL) {
		struct execute_t *execute = (struct execute_t *)execute_ptr;
		struct execute_stack_t *stack = execute->stack;
		struct execute_stack_t *back = execute->back_stack;
		struct execute_element_t *current_element = NULL;
		while(execute->cont_flag == 1) {
		
			pthread_mutex_lock(&stack->lock);
			pthread_cond_wait(&stack->exe_wait, &stack->lock);
			pthread_mutex_unlock(&stack->lock);

			do {
				current_element = execute_stack_pop(stack);
				nova_run(stack, back, current_element);

			} while(current_element->type_id != 2);
			execute_element_destroy(&current_element);	
		}
	}
	pthread_exit(NULL);
}

void nova_run(struct execute_stack_t *stack, struct execute_stack_t *back, struct execute_element_t *element) {
	if(stack != NULL && back != NULL && element != NULL) {
		int op_count = 0;
		struct execute_element_t *back_top = NULL;
		switch(element->type_id) {
			case 0:
				back_top = execute_stack_peek(back);
				if(back_top != NULL) {
					execute_stack_push(back, element);
					if(back_top->type_id == 0) {
						while(execute_stack_peek(back)->type_id == 0) {	
							execute_stack_push(stack, execute_stack_pop(back));
							++op_count;
						}
					}
					switch(execute_stack_peek(back)->type_id) {
						case 0:
							break;
						case 1:
							if(op_count == 2) {
								struct execute_element_t *op_one = execute_stack_pop(stack);
								struct execute_element_t *op_two = execute_stack_pop(stack);
								struct execute_element_t *op = execute_stack_pop(back);
								binary_op op_func = op->value;
								*(int *)op_two->value = op_func(*(int *)op_one->value, *(int *)op_two->value);
								execute_stack_push(stack, op_two);
								execute_element_destroy(&op_one);
								execute_element_destroy(&op);
							} else {
								while(op_count > 0) {
									execute_stack_push(back, execute_stack_pop(stack));
									--op_count;
								}
							}
							break;		
						case 2:
							break;
					}
				} else printf("%d\n", *(int *)element->value);
				break;
			case 1:
				execute_stack_push(back, element);
			case 2:
				break;
		}
	}
}

struct execute_element_t *execute_element_create(size_t size, char type_id) {
	struct execute_element_t *ret_val = malloc(sizeof(struct execute_element_t));
	if(ret_val != NULL) {
		ret_val->next = NULL;
		if(size > 0) ret_val->value = calloc(1, size);
		else ret_val->value = NULL;
		ret_val->type_id = type_id;
	}
	return ret_val;
}
void execute_element_destroy(struct execute_element_t **element) {
	if(element != NULL) {
		if(*element != NULL) {
			if((*element)->value != NULL && (*element)->type_id != 1) free((*element)->value);
			free(*element);
			*element = NULL;
		}
	}
}

struct execute_element_t *execute_stack_pop(struct execute_stack_t *stack) {
	struct execute_element_t *ret_val = NULL;
	if(stack != NULL) {	
		pthread_mutex_lock(&stack->lock);	
		ret_val = stack->top;
		if(stack->top != NULL) {
			stack->top = stack->top->next;
			ret_val->next = NULL;
		}
		pthread_mutex_unlock(&stack->lock);
	}
	return ret_val;
}
struct execute_element_t *execute_stack_peek(struct execute_stack_t *stack) {
	struct execute_element_t *ret_val = NULL;
	if(stack != NULL) {
		pthread_mutex_lock(&stack->lock);
		ret_val = stack->top;
		pthread_mutex_unlock(&stack->lock);
	}
	return ret_val;
}
void execute_stack_push(struct execute_stack_t *stack, struct execute_element_t *element) {
	if(stack != NULL && element != NULL) {
		pthread_mutex_lock(&stack->lock);
		element->next = stack->top;
		stack->top = element;
		pthread_mutex_unlock(&stack->lock);
	}
}

struct execute_stack_t *execute_stack_create() {
	struct execute_stack_t *ret_val = malloc(sizeof(struct execute_stack_t));
	if(ret_val != NULL) {
		ret_val->top = NULL;
		pthread_mutex_init(&ret_val->lock, NULL);
		pthread_cond_init(&ret_val->exe_wait, NULL);
	}
	return ret_val;
}
void execute_stack_destroy(struct execute_stack_t **stack) {
	if(stack != NULL) if(*stack != NULL) {
		struct execute_element_t *temp = NULL;
		while((*stack)->top != NULL) {
			temp = execute_stack_pop(*stack);
			execute_element_destroy(&temp);
		}
		pthread_mutex_destroy(&(*stack)->lock);
		pthread_cond_destroy(&(*stack)->exe_wait);
		free(*stack);
	}
}
