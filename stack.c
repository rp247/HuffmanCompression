#include "stack.h"

#include "node.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* CREDITS: From lab documentation */
/* stack structure */
struct Stack {
    uint32_t top; // index of next empty slot
    uint32_t capacity; // max pushes
    Node **items; // array of nodes
};

/* constructor for stack adt */
Stack *stack_create(uint32_t capacity) {

    Stack *s = (Stack *) malloc(sizeof(Stack));

    if (s) {
        s->top = 0;
        s->capacity = capacity;
        s->items = (Node **) calloc(capacity, sizeof(Node *)); // array of node pointers

        /* cant allocate memory. free stack */
        if (!s->items) {
            free(s);
            s = NULL;
        }
    }
    return s;
}

/* destructor for stack */
void stack_delete(Stack **s) {
    if (*s && (*s)->items) {
        free((*s)->items); // set node pointer to null ?? rem comment
        free(*s); // free stack
        *s = NULL;
    }
    return;
}

/* returns true if stack empty, else false */
bool stack_empty(Stack *s) {
    if (!s)
        return true; // no stack
    return s->top == 0; // if top is 0 then no elements
}

/* returns true if stack full, else false */
bool stack_full(Stack *s) {
    if (!s)
        return true;
    return s->top == s->capacity; // all indexes filled then true, else false
}

/* returns size of the stack */
uint32_t stack_size(Stack *s) {
    if (!s)
        return 0; // no stack
    return s->top; // top == size of stack
}

/* pushes x on top of stack. returns true if push successful, else false */
bool stack_push(Stack *s, Node *n) {
    if (!s || stack_full(s))
        return false;
    s->items[s->top] = n; // make top element = n
    s->top++;
    return true;
}

/* pops x off of stack. returns true if pop successful, else false. pops value in var pointed by x */
bool stack_pop(Stack *s, Node **n) {
    if (!s || stack_empty(s))
        return false;
    s->top--;
    if (n)
        *n = s->items[s->top];
    return true;
}

/* prints a stack */
void stack_print(Stack *s) {
    if (!s || stack_empty(s))
        return;

    printf("Stack (Tail->Head): ");
    for (uint32_t i = 0; i < s->top; i++) {
        node_print(s->items[i]);
        fprintf(stdout, "->");
    }
    printf("\n");

    return;
}
