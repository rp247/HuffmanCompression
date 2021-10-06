#include "code.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#define BYTE 8

/* helper function to set the bit at top index in code */
static void set_top(Code *c) {
    c->bits[c->top / BYTE] |= ((uint8_t) 1 << (c->top % BYTE));
    return;
}

/* helper function to clear the bit at top index in code */
static void clr_top(Code *c) {
    c->bits[c->top / BYTE] &= ~((uint8_t) 1 << (c->top % BYTE));
    return;
}

/* helper function to get the bit at top index in code */
static uint8_t get_top(Code *c) {
    return (c->bits[c->top / BYTE] & ((uint8_t) 1 << (c->top % BYTE))) ? 1 : 0;
}

/* constructor for Code ADT */
Code code_init(void) {
    Code c = { 0, { 0 } }; // top = 0, array = 0
    return c;
}

/* return size of Code */
uint32_t code_size(Code *c) {
    if (!c)
        return 0; // no code
    return c->top; // size == top
}

/* returns true if Code empty, else false */
bool code_empty(Code *c) {
    if (!c)
        return true; // no code
    return c->top == 0; // 0 then true, else false
}

/* returns true if Code full, else false */
bool code_full(Code *c) {
    if (!c)
        return true; // no code
    return c->top == ALPHABET; // can have max ALPHABET bits
}

/* pushes bit to Code */
bool code_push_bit(Code *c, uint8_t bit) {
    if (!c || code_full(c))
        return false;

    if (bit)
        set_top(c); // set if bit == 1 else clear top
    else
        clr_top(c);

    c->top++; // increment top
    return true;
}

/* pops a bit off of Code */
bool code_pop_bit(Code *c, uint8_t *bit) {
    if (!c || code_empty(c))
        return false;
    c->top--; // decrement top
    if (bit)
        *bit = get_top(c); // get the topmost bit
    return true;
}

/* prints a code */
void code_print(Code *c) {
    uint32_t i = ALPHABET, temp;
    while (i > 0) {
        i--;
        temp = c->bits[i / BYTE] & ((uint8_t) 1 << (i % BYTE));
        fprintf(stdout, "%c ", temp ? '1' : '0');
    }
    fprintf(stdout, "\n");
}
