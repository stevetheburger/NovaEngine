/*
Author: Stephen Hyberger
Date: 4.17.2021
File: novacmp.c
Purpose: Does the parsing for the Nova language.
*/

//Necessary imports.
#include"novacmp.h"

//Private functions.
void parse_through(struct parse_t *, uint64_t, uint64_t, int *, int *, int *, uint64_t *, uint64_t *);
void interpret_in(struct interpret_t *, uint64_t, int *, uint64_t *);

//Constants that define language.
#define NUM_NOVA_LANG_ALPHA 13
#define NUM_NOVA_LANG_STR_LEN 13
#define NUM_NOVA_LANG_PRODS 93
#define NUM_NOVA_LANG_RULES 5

#define NOVA_CHAR_LEN 1
#define NOVA_RULE_LEN 3
#define NOVA_ARROW_LEN 4
#define NOVA_DELIMIT_LEN 2

#define NOVA_LANG_TERM 254
#define NOVA_LANG_EMPTY 255

const char *NOVA_LANG_ALPHA = "+-0123456789;";

//The static human readable repersentation of the language grammar. Each item here is a string that represents a production in the grammar in the format: 
//starting rule (numerical char) -> (produces) character (ASCII char), (delimiter) next rule (numerical char).
const char NOVA_LANG_PRODS[NUM_NOVA_LANG_PRODS][NUM_NOVA_LANG_STR_LEN] = {
	"000 -> 0, 001", "000 -> 1, 001", "000 -> 2, 001", "000 -> 3, 001", "000 -> 4, 001", "000 -> 5, 001", "000 -> 6, 001", "000 -> 7, 001", "000 -> 8, 001", "000 -> 9, 001", "000 -> -, 002", 
	"000 -> +, 002", "000 -> 1, 003", "000 -> 2, 003", "000 -> 3, 003", "000 -> 4, 003", "000 -> 5, 003", "000 -> 6, 003", "000 -> 7, 003", "000 -> 8, 003", "000 -> 9, 003", "000 -> 0, 004",
	"000 -> 1, 004", "000 -> 2, 004", "000 -> 3, 004", "000 -> 4, 004", "000 -> 5, 004", "000 -> 6, 004", "000 -> 7, 004", "000 -> 8, 004", "000 -> 9, 004",
	
	"001 -> ;, 254",

	"002 -> 0, 001", "002 -> 1, 001", "002 -> 2, 001", "002 -> 3, 001", "002 -> 4, 001", "002 -> 5, 001", "002 -> 6, 001", "002 -> 7, 001", "002 -> 8, 001", "002 -> 9, 001", "002 -> 1, 003", 
	"002 -> 2, 003", "002 -> 3, 003", "002 -> 4, 003", "002 -> 5, 003", "002 -> 6, 003", "002 -> 7, 003", "002 -> 8, 003", "002 -> 9, 003", "002 -> 0, 004", "002 -> 1, 004", "002 -> 2, 004", 
	"002 -> 3, 004", "002 -> 4, 004", "002 -> 5, 004", "002 -> 6, 004", "002 -> 7, 004", "002 -> 8, 004", "002 -> 9, 004",

	"003 -> 0, 001", "003 -> 1, 001", "003 -> 2, 001", "003 -> 3, 001", "003 -> 4, 001", "003 -> 5, 001", "003 -> 6, 001", "003 -> 7, 001", "003 -> 8, 001", "003 -> 9, 001", "003 -> 0, 003",
	"003 -> 1, 003", "003 -> 2, 003", "003 -> 3, 003", "003 -> 4, 003", "003 -> 5, 003", "003 -> 6, 003", "003 -> 7, 003", "003 -> 8, 003", "003 -> 9, 003", "003 -> 0, 004", "003 -> 1, 004",
	"003 -> 2, 004", "003 -> 3, 004", "003 -> 4, 004", "003 -> 5, 004", "003 -> 6, 004", "003 -> 7, 004", "003 -> 8, 004", "003 -> 9, 004",

	"004 -> -, 000", "004 -> +, 000"
};

const unsigned char NOVA_LANG_CONTEXT[NUM_NOVA_LANG_RULES] = {0, 2, 0, 0, 1};

struct dictionary_t *create_dictionary();
char dictionary(struct parse_t *, int *, int *);
unsigned char alpha_index(int);

struct dictionary_t *create_dictionary() {
	struct dictionary_t *ret_val = malloc(sizeof(struct dictionary_t));
	if(ret_val != NULL) {
		ret_val->dict = malloc(sizeof(char *) * NUM_NOVA_LANG_RULES * NUM_NOVA_LANG_ALPHA);
		if(ret_val->dict != NULL) {

			uint64_t count = 0;
			for(; count < NUM_NOVA_LANG_RULES * NUM_NOVA_LANG_ALPHA; ++count) {
				ret_val->dict[count] = malloc(sizeof(char));
				if(ret_val->dict[count] != NULL) {
					*ret_val->dict[count] = NOVA_LANG_EMPTY;
				} else {
					while(count > 0) {
						free(ret_val->dict[--count]);
					}
					free(ret_val->dict);
					free(ret_val);
					ret_val = NULL;
					break;
				}
			}

			if(ret_val != NULL) {
				ret_val->num_rules = NUM_NOVA_LANG_RULES;
				ret_val->num_alpha = NUM_NOVA_LANG_ALPHA;

				char rule_left_str[NOVA_RULE_LEN + 1], rule_right_str[NOVA_RULE_LEN + 1], alpha_char;
				rule_left_str[NOVA_RULE_LEN] = '\0';
				rule_right_str[NOVA_RULE_LEN] = '\0';
				const char *strptr = NULL;
				unsigned char rule_left, rule_right, alpha, char_count;
				uint64_t dict_index = char_count = 0;
				unsigned char *cursor, *temp;
				cursor = temp = NULL;

				for(count = 0; count < NUM_NOVA_LANG_PRODS; ++count) {
					
					strptr = NOVA_LANG_PRODS[count];
					strncpy(rule_left_str, strptr, NOVA_RULE_LEN);
					alpha_char = *(strptr += (NOVA_RULE_LEN + NOVA_ARROW_LEN));
					strncpy(rule_right_str, strptr + NOVA_CHAR_LEN + NOVA_DELIMIT_LEN, NOVA_RULE_LEN);

					rule_left = (unsigned char)atoi(rule_left_str);
					rule_right = (unsigned char)atoi(rule_right_str);
					
					alpha = alpha_index(alpha_char);

					dict_index = rule_left * ret_val->num_alpha + alpha;
					temp = cursor = ret_val->dict[dict_index];
					char_count = 0;
					while(*cursor != NOVA_LANG_EMPTY) {
						++char_count;
						++cursor;
					}
					
					ret_val->dict[dict_index] = realloc(ret_val->dict[dict_index], char_count + 1);
					if(ret_val->dict[dict_index] != NULL) {
						ret_val->dict[dict_index][char_count] = rule_right;
						ret_val->dict[dict_index][char_count + 1] = NOVA_LANG_EMPTY;
					}
				}
			}
		}
	}

	return ret_val;
}
unsigned char alpha_index(int alpha) {
	unsigned char cur_index = NUM_NOVA_LANG_ALPHA / 2;
	int cur = 0;
	unsigned char min = 0;
	unsigned char max = NUM_NOVA_LANG_ALPHA;
	while(min != max) {
		cur = (int)NOVA_LANG_ALPHA[cur_index];
		if(alpha > cur) {
			min = cur_index + 1;
			cur_index += (max - cur_index) / 2;
		} else if(alpha < cur) {
			max = cur_index - 1;
			cur_index -= (cur_index - min) / 2;
		} else {
			max = cur_index;
			break;
		}
	}

	return max;
}

char dictionary(struct parse_t *parse, int *c, int *ahead) {
	if(parse != NULL) {
		if(parse->dict != NULL) {
			unsigned char *search_list = NULL;
			unsigned char check_result;
			unsigned char alpha = alpha_index(*c);
			if(alpha != NOVA_LANG_EMPTY) search_list = parse->dict->dict[parse->current_prod * parse->dict->num_alpha + alpha];
			if(search_list != NULL) {
				while(*search_list != NOVA_LANG_EMPTY) {
					alpha = alpha_index(*ahead);
					if(alpha != NOVA_LANG_EMPTY) check_result = parse->dict->dict[*search_list * parse->dict->num_alpha + alpha][0];
					else check_result = NOVA_LANG_EMPTY;
					
					if(check_result != NOVA_LANG_EMPTY) {
						parse->last_context = parse->current_context;
						parse->current_context = NOVA_LANG_CONTEXT[*search_list];
						parse->current_prod = *search_list;
						return 1;
					}
					++search_list;
				}
			}
		}
	}
	return 0;
}

//Creates a parse node.
struct parse_t *parse_create() {

	//Allocate.
	struct parse_t *ret_val = malloc(sizeof(struct parse_t));
	//Initialize.
	if(ret_val != NULL) {
		ret_val->dict = create_dictionary();
		if(ret_val->dict != NULL) {
			
			ret_val->in_buf = NULL;
			ret_val->out_buf = NULL;
			ret_val->context_queue = NULL;	
			ret_val->current_context = 0;
			ret_val->last_context = 0;
			ret_val->current_prod = 0;
			ret_val->cont_flag = 1;
		} else {
			free(ret_val);
			ret_val = NULL;
		}
	}
	return ret_val;
}

//Destroys given parse node.
void parse_destroy(struct parse_t **parse_ptr) {
	if(parse_ptr != NULL) {
		if(*parse_ptr != NULL) {	

			//Free node.
			free(*parse_ptr);
		}
	}
}

//Takes input buffer and parse node and sets the node's input buffer to that buffer. Returns whatever the old contents of the input buffer field was.
struct buffer_t *parse_set_in(struct parse_t *parse, struct buffer_t *buf) {
	struct buffer_t *ret_val = NULL;
	if(parse != NULL) {
	
		//Get return value.
		ret_val = parse->in_buf;

		//Set new value.
		parse->in_buf = buf;
	}
	return ret_val;
}

//Takes output buffer and a parse node and sets the node's output buffer to that buffer. Returns whatever the old contents of the input buffer field was.
struct buffer_t *parse_set_out(struct parse_t *parse, struct buffer_t *buf) {
	struct buffer_t *ret_val = NULL;
	if(parse != NULL) {

		//Get return value.
		ret_val = parse->out_buf;

		//Set new value.
		parse->out_buf = buf;
	}
	return ret_val;
}

//Takes context queue and a parse node and sets the node's context queue to that queue. Returns whatever the old contents of the context queue field was.
struct context_queue_t *parse_set_queue(struct parse_t *parse, struct context_queue_t *queue) {
	struct context_queue_t *ret_val = NULL;
	if(parse != NULL) {

		//Get return value.
		ret_val = parse->context_queue;

		//Set new value.
		parse->context_queue = queue;
	}
	return ret_val;
}	

//Does parsing operation until termination.
void *do_parse(void *parse_ptr) {

	//Check for null.
	if(parse_ptr != NULL) {

		//Get handles for the structs to be used for parsing.
		struct parse_t *parse = (struct parse_t *)parse_ptr;
		struct buffer_t *in_buf = parse->in_buf;
		struct buffer_t *out_buf = parse->out_buf;
		//Check for null.
		if(in_buf != NULL && out_buf != NULL) {

			//Variables on the stack to be used by this thread during parse runtime.
			int current = 0;
			int last = 0;
			int look_ahead = 0;
			uint64_t in_diff = 0;
			uint64_t out_diff = 0;

			//Loop until program termination begins.
			while(parse->cont_flag) {

				//Lock buffer. Do control logic.
				buffer_read_lock(in_buf);	
				buffer_write_lock(out_buf);	

				//Check if write is ahead of read. If so read until write.
				if(out_buf->cursor_read > out_buf->cursor_write) {
					if(in_buf->cursor_write > in_buf->cursor_read) parse_through(parse, in_buf->cursor_write - in_buf->cursor_read, out_buf->cursor_read - out_buf->cursor_write, &current, &last, &look_ahead, &in_diff, &out_diff);
					else parse_through(parse, in_buf->end - in_buf->cursor_read, out_buf->cursor_read - out_buf->cursor_write, &current, &last, &look_ahead, &in_diff, &out_diff);
				//Otherwise, the read is ahead of write. If so read until end.
				} else {
					if(in_buf->cursor_write > in_buf->cursor_read) parse_through(parse, in_buf->cursor_write - in_buf->cursor_read, out_buf->end - out_buf->cursor_write, &current, &last, &look_ahead, &in_diff, &out_diff);
					else parse_through(parse, in_buf->end - in_buf->cursor_read, out_buf->end - out_buf->cursor_write, &current, &last, &look_ahead, &in_diff, &out_diff);
				}

				//Update the cursor.
				in_buf->cursor_read += in_diff;
				//Unlock buffer. Do control logic.
				buffer_read_unlock(in_buf, in_diff);
				
				//Update the cursor.
				out_buf->cursor_write += out_diff;
				//Unlock buffer. Do control logic.
				buffer_write_unlock(out_buf, out_diff);

				//Reset cursor counter.
				in_diff = 0;
				out_diff = 0;
			}
		}
	}
	pthread_exit(NULL);
}

//Does parsing for the input provided on buffer buf up to max_size using the provided parse struct and the provided character, rule, and production holders, and the rule, production, and character counters.
void parse_through(struct parse_t *parse, uint64_t max_size_in, uint64_t max_size_out, int *current, int *last, int *look_ahead, uint64_t *in_diff, uint64_t *out_diff) {
	
	//Loop until max size is reached.
	while(*in_diff < max_size_in && *out_diff < max_size_out) {
	
		//Get current character.
		if(*last == 0) {
			*current = (int)*((char *)parse->in_buf->cursor_read + (*in_diff)++);	
			*((char *)(parse->out_buf->cursor_write + (*out_diff)++)) = *((char *)current);
		} else {
			*current = *last;
			*last = 0;
		}
		
		if(*in_diff < max_size_in) {
			*look_ahead = (int)*((char *)parse->in_buf->cursor_read + *in_diff);
			context_increment(parse->context_queue, 1);
			if(*look_ahead != '\0') {
				if(dictionary(parse, current, look_ahead)) {
						if(parse->current_context != parse->last_context) context_append(parse->context_queue, parse->last_context);
				//Syntax error handling will be done here.
				} else printf("Error State\n");
			} else {
				++(*in_diff);
			
				context_append(parse->context_queue, parse->current_context);
				context_append(parse->context_queue, -1);
				parse->current_prod = 0;
				parse->current_context = 0;
				parse->last_context = 0;
			}
		} else *last = *current;
	}
}

//Creates interpret node.
struct interpret_t *interpret_create() {

	//Allocate.
	struct interpret_t *ret_val = malloc(sizeof(struct interpret_t));
	struct execute_stack_t *back_stack = execute_stack_create();
	//Initialize.
	if(ret_val != NULL && back_stack != NULL) {
		ret_val->in_buf = NULL;
		ret_val->context_queue = NULL;
		ret_val->execute_stack = NULL;
		ret_val->back_stack = back_stack;
		ret_val->current_element = NULL;
		ret_val->current_count = 0;
		ret_val->current_size = 0;
		ret_val->neg_flag = 0;
		ret_val->cont_flag = 1;

	//Make node creation atomic.
	} else {
		if(ret_val != NULL) {
			free(ret_val);
			ret_val = 0;
		}
		if(back_stack != NULL) execute_stack_destroy(&back_stack);
	}
	return ret_val;
}

//Destroys given interpret node.
void interpret_destroy(struct interpret_t **interpret) {
	if(interpret != NULL) {
		if(*interpret != NULL) {

			//Frees current execution element.
			if((*interpret)->current_element != NULL) execute_element_destroy(&(*interpret)->current_element);
			
			//Frees back stack.
			if((*interpret)->back_stack != NULL) execute_stack_destroy(&((*interpret)->back_stack));	

			//Frees node.
			free(*interpret);
		}
	}
}

//Takes a buffer and a interpret node, sets the in buffer field of the node to the buffer, and returns whatever the old value of the field was.
struct buffer_t *interpret_set_in(struct interpret_t *interpret, struct buffer_t *buf) {
	struct buffer_t *ret_val = NULL;
	if(interpret != NULL) {

		//Get return value.
		ret_val = interpret->in_buf;

		//Set new value.
		interpret->in_buf = buf;
	}
	return ret_val;
}
//Takes a context queue and a interpret node, set the context queue field of the node to the queue, and returns whatever the old value of the field was.
struct context_queue_t *interpret_set_queue(struct interpret_t *interpret, struct context_queue_t *queue) {
	struct context_queue_t *ret_val = NULL;
	if(interpret != NULL) {

		//Get return value.
		ret_val = interpret->context_queue;

		//Set new value.
		interpret->context_queue = queue;
	}
	return ret_val;
}
//Takes an execution stack and a interpret node, set the execution stack field of the node to the stack, and returns whatever the old value of the field was.
struct execute_stack_t *interpret_set_stack(struct interpret_t *interpret, struct execute_stack_t *stack) {
	struct execute_stack_t *ret_val = NULL;
	if(interpret != NULL) {

		//Get return value.
		ret_val = interpret->execute_stack;

		//Set new value.
		interpret->execute_stack = stack;
	}
	return ret_val;
}

void *do_interpret(void *interpret_ptr) {
	
	//Check for null.
	if(interpret_ptr != NULL) {
		
		//Get handles for the interpreter.
		struct interpret_t *interpret = (struct interpret_t *)interpret_ptr;
		struct buffer_t *buf = interpret->in_buf;
		//Check for null.
		if(buf != NULL) {

			int current = 0;
			uint64_t diff = 0;

			//Loop until program termination begins.
			while(interpret->cont_flag) {

				//Lock buffer. Do control logic.
				buffer_read_lock(buf);	

				if(buf->cursor_write > buf->cursor_read) interpret_in(interpret, buf->cursor_write - buf->cursor_read, &current, &diff);
				else interpret_in(interpret, buf->end - buf->cursor_read, &current, &diff);

				buf->cursor_read += diff;

				buffer_read_unlock(buf, diff);

				//Reset cursor counter.
				diff = 0;
			}
		}
	}
	pthread_exit(NULL);
}

void interpret_in(struct interpret_t *interpret, uint64_t max_size, int *current, uint64_t *diff) {

	while(*diff < max_size) {

		pthread_mutex_lock(&interpret->context_queue->lock);
		interpret->current_size = context_size(interpret->context_queue);
		interpret->current_type = context_type(interpret->context_queue);
		pthread_mutex_unlock(&interpret->context_queue->lock);

		//printf("Type: %ld\n", interpret->current_type);

		if(interpret->current_size > max_size) interpret->current_size = max_size;
		else if(interpret->current_size == 0) {
			if(interpret->current_type == -1) {
				context_remove(interpret->context_queue);
			}
			return;
		}

		if(interpret->current_element == NULL) {
			interpret->neg_flag = 0;
			switch(interpret->current_type) {
				case 0:
					interpret->current_element = execute_element_create(sizeof(int), 0);
					break;
				case 1:
					interpret->current_element = execute_element_create(0, 1);
					while(execute_stack_peek(interpret->back_stack) != NULL) {
						execute_stack_push(interpret->execute_stack, execute_stack_pop(interpret->back_stack));
					}
					break;
				case 2:
					interpret->current_element = execute_element_create(0, 2);
					//Do full backup
					while(execute_stack_peek(interpret->execute_stack) != NULL) {
						execute_stack_push(interpret->back_stack, execute_stack_pop(interpret->execute_stack));
					}
					break;
				default:
					//error.	
					break;
			}
		}	

		while(interpret->current_count < interpret->current_size) {
			*current = *((char *)interpret->in_buf->cursor_read + (*diff)++);

			switch(interpret->current_type) {	
				
				//Number conversion.	
				case 0:
					console_to_int(interpret, current);
					break;
				//Operator insertion.
				case 1:
					
					if(context_size(interpret->context_queue) == 1) {
						if(*current == '+') {
							interpret->current_element->value = nova_add;
						} else if(*current == '-') {
							interpret->current_element->value = nova_sub;
						}
					}
					break;
			}
			++interpret->current_count;
		}

		if(context_remove(interpret->context_queue)) {
			switch(interpret->current_type) {
				case 0:
					execute_stack_push(interpret->execute_stack, interpret->current_element);
					break;
				case 1:
					execute_stack_push(interpret->back_stack, interpret->current_element);
					break;	
				case 2:
					execute_stack_push(interpret->execute_stack, interpret->current_element);
					while(execute_stack_peek(interpret->back_stack) != NULL) {
						struct execute_element_t *element = execute_stack_pop(interpret->back_stack);
						execute_stack_push(interpret->execute_stack, element);
						
					}
					
					pthread_cond_signal(&interpret->execute_stack->exe_wait);
					break;
			}
			interpret->current_element = NULL;		
			interpret->current_count = 0;
		}
	}
}

/*void interpret_elements_resize(struct interpret_t *interpret, uint64_t new_size) {
	if(interpret != NULL) {
		if(interpret->elements != NULL) {
			struct execute_variable_t **temp = interpret->elements;
			interpret->elements = realloc(interpret->elements, sizeof(void *) * new_size);
			if(interpret->elements != NULL) interpret->elements_alloc = new_size;
			else interpret->elements = temp;
		} else {
			interpret->elements = malloc(sizeof(void *) * new_size);
			if(interpret->elements != NULL) interpret->elements_alloc = new_size;
		}
	}
}*/


void console_to_int(struct interpret_t *interpret, int *current) {
	if(interpret != NULL && current != NULL) {
		if(interpret->current_element->type_id == 0 && interpret->current_element->value != NULL) {
			if(context_read(interpret->context_queue) <= context_size(interpret->context_queue) && *current > 0b00101111 && *current < 0b00111010) {

				uint64_t ten_pow = 1;
				for(uint64_t i = 0; i < interpret->current_size - interpret->current_count - 1; ++i) {
					ten_pow *= 10;
				}

				if(interpret->neg_flag) *((int *)interpret->current_element->value) -= (*current - 0b00110000) * ten_pow;
				else *((int *)interpret->current_element->value) += (*current - 0b00110000) * ten_pow;
				
				context_read_inc(interpret->context_queue, 1);
			} else if(*current == 0b00101101) interpret->neg_flag = 1;
		} 
	}
}
