#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef DEBUG
#define ON_DBG(...) __VA_ARGS__
#else
#define ON_DBG(...)
#endif

typedef double stack_elem_t;

typedef enum {
    STACK_OK,
    STACK_NULL_POINTER,
    STACK_ALREADY_EXIST,
    STACK_BAD_SIZE,
    STACK_BAD_ALLOC,
    STACK_BAD_DATA,
    STACK_BAD_CAPACITY,
    STACK_NEGATIVE_SIZE,
    STACK_NEGATIVE_CAPACITY,
    STACK_EMPTY
} stack_error_t;

typedef struct {
    stack_elem_t* data;
    ssize_t size;
    ssize_t capacity;
    stack_error_t error;
    ON_DBG(
        const char* file_name;
        const char* func_name;
        size_t line;
        const char* name;
    )
} stack_t;

void stack_dump(stack_t* stk, stack_error_t);
void print_stack_error(stack_error_t err);

stack_error_t stack_init(stack_t* stk, ssize_t initial_capacity 
    ON_DBG(, const char* file_name, const char* func_name, size_t line, const char* name));
stack_error_t stack_push(stack_t* stk, stack_elem_t elem);
stack_error_t stack_pop(stack_t* stk, stack_elem_t* elem);
stack_error_t stack_destroy(stack_t* stk);
stack_error_t stack_verifyer(stack_t* stk);
stack_error_t stack_isclear(stack_t* stk);

#endif