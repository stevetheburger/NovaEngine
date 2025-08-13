/*
Author: Stephen Hyberger
File: novaops.c
Date: 3.24.2021
Purpose: Define operations for the Nova language.
*/

//Necessary imports.
#include"novaops.h"

//Function definitions for arithmetic operators.
int nova_add(int op_one, int op_two) {	
	return op_one + op_two;
}

int nova_sub(int op_one, int op_two) {
	return op_two - op_one;
}

int nova_mult(int op_one, int op_two) {
	return op_one * op_two;
}

int nova_div(int op_one, int op_two) {
	return op_two / op_one;
}

int nova_mod(int op_one, int op_two) {
	return op_two % op_one;
}
