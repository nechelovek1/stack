#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "stack.h"

const stack_elem_t STACK_POISON = NAN;

stack_error_t stack_init(stack_t* stk, ssize_t initial_capacity 
    ON_DBG(, const char* file_name, const char* func_name, size_t line, const char* name))
{
    ON_DBG(
    if (stk != NULL) {
        stk->file_name = file_name;
        stk->func_name = func_name;
        stk->line = line;
        stk->name = name;
    }
    )

    ON_DBG(assert(stack_isclear(stk) == STACK_OK);)

    stk->data = calloc(initial_capacity, sizeof(stack_elem_t));

    if (stk->data == NULL) {
        stk->error = STACK_BAD_ALLOC;
        return STACK_BAD_ALLOC;
    }

    stk->capacity = initial_capacity;
    for (ssize_t i = 0; i < stk->capacity; i++) {
        stk->data[i] = STACK_POISON;
    }

    ON_DBG(assert(stack_verifyer(stk) == STACK_OK);)

    return STACK_OK;
}

stack_error_t stack_push(stack_t* stk, stack_elem_t elem)
{
    ON_DBG(assert(stack_verifyer(stk) == STACK_OK);)

    if(stk->size == stk->capacity) {
        stk->capacity = stk->capacity * 2;
        void* tmp = realloc(stk->data, stk->capacity * sizeof(stack_elem_t));
        if (tmp == NULL) {
            stk->error = STACK_BAD_ALLOC;
            return STACK_BAD_ALLOC;
        }
        stk->data = tmp;
    }

    stk->data[stk->size] = elem;

    ON_DBG(assert(stack_verifyer(stk) == STACK_OK);)

    return STACK_OK;
}

stack_error_t stack_pop(stack_t* stk, stack_elem_t* elem)
{
    ON_DBG(assert(stack_verifyer(stk) == STACK_OK);)

    *elem = stk->data[stk->size];
    stk->data[stk->size] = STACK_POISON;
    stk->size = stk->size - 1;

    if (stk->capacity / 4 > stk->size) {
        stk->capacity = stk->capacity / 4;
        void* tmp = realloc(stk->data, stk->capacity * sizeof(stack_elem_t));
        if (tmp == NULL) {
            stk->error = STACK_BAD_ALLOC;
            return STACK_BAD_ALLOC;
        }
        stk->data = tmp;
    } 

    ON_DBG(assert(stack_verifyer(stk) == STACK_OK);)
}

stack_error_t stack_destroy(stack_t* stk)
{
    ON_DBG(assert(stack_verifyer(stk) == STACK_OK);)

    free(stk->data);
    stk->data = NULL;
    stk->capacity = 0;
    stk->size = 0;

    return STACK_OK;
}

void stack_dump(stack_t* stk, stack_error_t err)
{
    if (stk == NULL) {
        printf("Stack pointer is NULL!!\n");
        return;
    }
    
    ON_DBG(printf("Stack named <%s> created by <%s> in file <%s> on line %lu\n", 
        stk->name, stk->func_name, stk->file_name, stk->line);)
    
    printf("Stack error code: %d\n", err);
    
    printf("[%p]\n", stk);
    printf("Stack capacity: %lu\n", stk->capacity);
    printf("Stack size: %lu\n", stk->size);
    printf("Stack data addr: %p\n", stk->data);

    if (stk->data == NULL) {
        return;
    }

    for (size_t i = 0; i < stk->capacity; i++) {
        if (i < stk->size) {
            printf("[%lu] %lf\n", i, stk->data[i]);
        } else {
            printf("[%lu] %lf (poison)\n", i, stk->data[i]);
        }
    }
}

stack_error_t stack_verifyer(stack_t* stk)
{
    stack_error_t err = STACK_OK;

    if (stk == NULL) {
        err = STACK_NULL_POINTER;
    } else if (stk->capacity < stk->size) {
        err = STACK_BAD_CAPACITY;
    } else if (stk->capacity < 0) {
        err = STACK_NEGATIVE_CAPACITY;
    } else if (stk->size < 0) {
        err = STACK_NEGATIVE_SIZE;
    } else if (stk->data == NULL) {
        err = STACK_BAD_DATA;
    }

    if (stk->error != STACK_OK) {
        stack_dump(stk, err);
    }

    return err;
}

stack_error_t stack_isclear(stack_t* stk)
{
    stack_error_t err = STACK_OK;

    if (stk == NULL) {
        err = STACK_NULL_POINTER;
    } else if (stk->data == NULL && stk->capacity == 0 && stk->size == 0) {
        err = STACK_OK;
    } else {
        err = STACK_ALREADY_EXIST;
    }

    if (err != STACK_OK) {
        stack_dump(stk, err);
    }

    return err;
}