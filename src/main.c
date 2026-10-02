#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define DEBUG
#define CANARY

#ifdef DEBUG
#define ON_DBG(...) __VA_ARGS__
#else
#define ON_DBG(...)
#endif

#ifdef CANARY
#define ON_CANARY(...) __VA_ARGS__
#else
#define ON_CANARY(...)
#endif

typedef double stack_elem_t;

typedef enum {
    STACK_OK,
    STACK_EMPTY,
    STACK_BAD_ALLOC,
    STACK_NULL_POINTER,
    STACK_ALREADY_EXIST,
    STACK_BAD_SIZE,
    STACK_BAD_DATA,
    STACK_BAD_CAPACITY,
    STACK_NEGATIVE_SIZE,
    STACK_NEGATIVE_CAPACITY,
    STACK_CANARY_LEFT_ERR,
    STACK_CANARY_RIGHT_ERR,
    BUFFER_CANARY_LEFT_ERR,
    BUFFER_CANARY_RIGHT_ERR   
} stack_error_t;

typedef struct {
    ON_CANARY(long canary_left;)
    ON_CANARY(stack_elem_t* buffer_canary_left;)
    stack_elem_t* data;
    ON_CANARY(stack_elem_t* buffer_canary_right;)
    ssize_t size;
    ssize_t capacity;
    ON_DBG(
        stack_error_t error;
        const char* file_name;
        const char* func_name;
        size_t line;
        const char* name;
    )
    ON_CANARY(long canary_right;)
} stack_t;

const ssize_t STACK_INCR = 2;
const ssize_t STACK_DECR = STACK_INCR * STACK_INCR;

const stack_elem_t STACK_POISON = NAN;
const long STACK_CANARY_LEFT = 0xD01B0EB;
const long STACK_CANARY_RIGHT = 0xEBA1A4;
const stack_elem_t BUFFER_CANARY_LEFT = 0xBADEDA;
const stack_elem_t BUFFER_CANARY_RIGHT = 0xBADE1DA;

void stack_dump(stack_t* stk);
void stack_print_error(stack_error_t err);

stack_error_t stack_init(stack_t* stk, size_t initial_capacity 
    ON_DBG(, const char* file_name, const char* func_name, size_t line, const char* name));
stack_error_t stack_push(stack_t* stk, stack_elem_t elem);
stack_error_t stack_pop(stack_t* stk, stack_elem_t* elem);
stack_error_t stack_destroy(stack_t* stk);
stack_error_t stack_verifyer(stack_t* stk);
stack_error_t stack_isclear(stack_t* stk);
stack_error_t stack_canary(stack_t* stk);

int main()
{
    stack_t stk1 = {};
    stack_init(&stk1, 5 
        ON_DBG(, __FILE__, __func__, __LINE__, "stk1"));
    stack_elem_t elem = 0;
    for (size_t i = 0; i < 1000000; i++){
        stack_push(&stk1, 1);
    }
    for (size_t i = 0; i < 1000000; i++){
        stack_pop(&stk1, &elem);
    }
    stack_pop(&stk1, &elem);
    stack_pop(&stk1, &elem);
    stack_destroy(&stk1);
    return 0;
}

stack_error_t stack_init(stack_t* stk, size_t initial_capacity 
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

    ON_CANARY(
    stk->canary_left = STACK_CANARY_LEFT;
    stk->canary_right = STACK_CANARY_RIGHT;
    )
    
    stk->capacity = initial_capacity;
    stk->data = (stack_elem_t*) calloc(initial_capacity, sizeof(stack_elem_t));

    if (stk->data == NULL) {
        ON_DBG(
            stk->error = STACK_BAD_ALLOC;
            stack_dump(stk);
        )
        return STACK_BAD_ALLOC;
    }

    for (ssize_t i = 0; i < stk->capacity; i++) {
        stk->data[i] = STACK_POISON;
    }

    ON_DBG(
    if (stack_verifyer(stk) != STACK_OK) {
        return stk->error;
    }
    )

    return STACK_OK;
}

stack_error_t stack_push(stack_t* stk, stack_elem_t elem)
{
    stack_error_t err = STACK_OK;

    ON_DBG(
    if (stack_verifyer(stk) != STACK_OK) {
        return stk->error;
    }
    )

    if(stk->size == stk->capacity) {
        stk->capacity = stk->capacity * STACK_INCR;
        stack_elem_t* tmp = (stack_elem_t*) realloc(stk->data, stk->capacity * sizeof(stack_elem_t));

        if (tmp == NULL) {
            ON_DBG(
            stk->error = STACK_BAD_ALLOC;
            stack_dump(stk);
            )

            return STACK_BAD_ALLOC;
        }

        stk->data = tmp;
    }

    stk->data[stk->size] = elem;
    stk->size++;

    ON_DBG(
    if (stack_verifyer(stk) != STACK_OK) {
        return stk->error;
    }
    )

    return STACK_OK;
}

stack_error_t stack_pop(stack_t* stk, stack_elem_t* elem)
{
    stack_error_t err = STACK_OK;

    ON_DBG(
    if (stack_verifyer(stk) != STACK_OK) {
        return stk->error;
    }
    )

    if (stk->size == 0) {
        ON_DBG(
        stk->error = STACK_EMPTY;
        stack_dump(stk);
        )

        return STACK_EMPTY;
    }

    stk->size = stk->size - 1;
    *elem = stk->data[stk->size];
    stk->data[stk->size] = STACK_POISON;

    if (stk->capacity / STACK_DECR > stk->size) {
        stk->capacity = stk->capacity / STACK_DECR;
        stack_elem_t* tmp = (stack_elem_t*) realloc(stk->data, stk->capacity * sizeof(stack_elem_t));

        if (tmp == NULL) {
            ON_DBG(
            stk->error = STACK_BAD_ALLOC;
            stack_dump(stk);
            )

            return STACK_BAD_ALLOC;
        }

        stk->data = tmp;
    } 

    ON_DBG(
    if (stack_verifyer(stk) != STACK_OK) {
        return stk->error;
    }
    )

    return STACK_OK;
}

stack_error_t stack_destroy(stack_t* stk)
{
    ON_DBG(
    if (stack_verifyer(stk) != STACK_OK) {
        return stk->error;
    }
    )

    free(stk->data);
    *stk = (stack_t){};

    return STACK_OK;
}

void stack_dump(stack_t* stk)
{
    if (stk == NULL) {
        printf("Stack pointer is NULL!!\n");
        return;
    }
    
    ON_DBG(
    printf("Stack named <%s> created by <%s> in file <%s> on line %lu\n", 
        stk->name, stk->func_name, stk->file_name, stk->line);
    stack_print_error(stk->error);
    )
    
    printf("[%p]\n", stk);
    printf("Stack capacity: %lu\n", stk->capacity);
    printf("Stack size: %lu\n", stk->size);
    printf("Stack data addr: %p\n", stk->data);

    if (stk->data == NULL) {
        return;
    }

    for (ssize_t i = 0; i < stk->capacity; i++) {
        if (i < stk->size) {
            printf("[%lu] %lf\n", i, stk->data[i]);
        } else {
            printf("[%lu] %lf (poison)\n", i, stk->data[i]);
        }
    }
}

stack_error_t stack_verifyer(stack_t* stk)
{
    if (stk == NULL) {
        printf("Stack pointer is NULL!!!!!!!\n");
        return STACK_NULL_POINTER;
    }

    ON_DBG(
    if (stk->capacity < stk->size) {
        stk->error = STACK_BAD_CAPACITY;
    } else if (stk->capacity <= 0) {
        stk->error = STACK_NEGATIVE_CAPACITY;
    } else if (stk->size < 0) {
        stk->error = STACK_NEGATIVE_SIZE;
    } else if (stk->data == NULL) {
        stk->error = STACK_BAD_DATA;
    }

    if (stk->error != STACK_OK) {
        stack_dump(stk);
    }

    return stk->error;
    )
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
        stack_dump(stk);
    }

    return err;
}

void stack_print_error(stack_error_t err)
{
    printf("Error code %d: ", err);
    switch(err)
    {
        case STACK_OK:
            printf("STACK_OK\n");
            break;
        case STACK_EMPTY:
            printf("STACK_EMPTY\n");
            break;
        case STACK_BAD_ALLOC:
            printf("STACK_BAD_ALLOC\n");
            break;
        case STACK_NULL_POINTER:
            printf("STACK_NULL_POINTER\n");
            break;
        case STACK_ALREADY_EXIST:
            printf("STACK_ALREADY_EXIST\n");
            break;
        case STACK_BAD_SIZE:
            printf("STACK_BAD_SIZE\n");
            break;
        case STACK_BAD_DATA:
            printf("STACK_BAD_DATA\n");
            break;
        case STACK_BAD_CAPACITY:
            printf("STACK_BAD_CAPACITY\n");
            break;
        case STACK_NEGATIVE_SIZE:
            printf("STACK_NEGATIVE_SIZE\n");
            break;
        case STACK_NEGATIVE_CAPACITY:
            printf("STACK_NEGATIVE_CAPACITY\n");
            break;
    }
}

stack_error_t stack_canary(stack_t* stk)
{
    ON_CANARY(
        if (stk->canary_left != STACK_CANARY_LEFT) {
            stk->error = STACK_CANARY_LEFT_ERR;
            stack_dump(stk);
        } else if (stk->canary_left != STACK_CANARY_RIGHT) {
            stk->error = STACK_CANARY_RIGHT_ERR;
            stack_dump(stk);
        } else if (*stk->buffer_canary_left != BUFFER_CANARY_LEFT) {
            stk->error = BUFFER_CANARY_LEFT_ERR;
            stack_dump(stk);
        } else if (*stk->buffer_canary_right != BUFFER_CANARY_RIGHT) {
            stk->error = BUFFER_CANARY_RIGHT_ERR;
            stack_dump(stk);
        } 
    )

    return stk->error;
}