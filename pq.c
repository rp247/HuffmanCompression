#include "pq.h"

#include "node.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* a circular PriorityQueue (PQ) w/ array */
struct PriorityQueue {
    uint32_t head; // head index
    uint32_t tail; // tail index
    uint32_t size; // size of queue
    uint32_t capacity; // capacity of queue
    Node **n_arr; // pointer to array that node pointers
};

/* to calculate next postion of head or tail (wrap around pos) */
/* [create a function] idea from Eugene's section */
static inline uint32_t next_pos(uint32_t elem, uint32_t capacity) {
    return (elem + 1) % capacity;
}

/* to calculate previous postion of head or tail (wrap around pos) */
/* credits: based on prev player function provided in lab1 and from Eugene's section */
static inline uint32_t prev_pos(uint32_t elem, uint32_t capacity) {
    return (elem + capacity - 1) % capacity;
}

/* constructor for a PQ */
PriorityQueue *pq_create(uint32_t capacity) {
    PriorityQueue *q = (PriorityQueue *) malloc(sizeof(PriorityQueue));

    if (q) {
        q->head = 0;
        q->tail = 0;
        q->size = 0;
        q->capacity = capacity;
        q->n_arr = (Node **) calloc(capacity, sizeof(Node *)); // allocate space for array

        /* if no mem allocated for the array */
        if (!q->n_arr) {
            free(q);
            q = NULL;
        }
    }

    return q;
}

/* destructor for a PQ */
void pq_delete(PriorityQueue **q) {
    if (*q && (*q)->n_arr) {
        free((*q)->n_arr); // free the array (doesn't free the actual node mem, just the array)
        free(*q);
        *q = NULL;
    }
    return;
}

/* returns true if PQ empty, else false */
bool pq_empty(PriorityQueue *q) {
    if (!q)
        return true; // no pq
    return q->size == 0; // empty if size == 0
}

/* returns true if PQ full, else false */
bool pq_full(PriorityQueue *q) {
    if (!q)
        return true; // no pq
    return q->size == q->capacity; // at full capacity
}

/* returns the size of the PQ */
uint32_t pq_size(PriorityQueue *q) {
    if (!q)
        return 0; // no pq
    return q->size;
}

/* credits: based on the shell sort pseudocode provided in lab3 */
/* insertion sort to sort a PQ in ascending order */
static void ins_sort(PriorityQueue *q) {

    Node *temp = q->n_arr[q->tail]; // store the node at current index
    uint32_t i = q->tail, prev = prev_pos(i, q->capacity);

    /* backtrack till current is the minimum node */
    while (i != q->head && temp->frequency < q->n_arr[prev]->frequency) {
        q->n_arr[i] = q->n_arr[prev]; // move the max one over
        i = prev_pos(i, q->capacity);
        prev = prev_pos(i, q->capacity);
    }

    q->n_arr[i] = temp;

    return;
}

/* enqueue a node in PQ (sorted in ascending order) */
bool enqueue(PriorityQueue *q, Node *n) {
    if (!q || pq_full(q))
        return false; // no pq or pq full

    q->n_arr[q->tail] = n; // add the node

    ins_sort(q); // sort the queue

    q->tail = next_pos(q->tail, q->capacity); // move tail one step or wrap around
    q->size++; // increment the size

    return true;
}

/* dequeues the minimum (the first) element from the PQ */
bool dequeue(PriorityQueue *q, Node **n) {
    if (!q || pq_empty(q))
        return false; // queue false

    *n = q->n_arr[q->head]; // store to the node
    q->head = next_pos(q->head, q->capacity); // get the next had position
    q->size--; // decrement size

    return true;
}

/* prints a priority queue */
void pq_print(PriorityQueue *q) {

    if (!q || pq_empty(q))
        return;

    printf("Queue (Head<-Tail): ");

    uint32_t i = q->head;

    /* couldn't print head w/o do-while */
    do {
        fprintf(stdout, " <- "); // print the node
        node_print(q->n_arr[i]);
        i = next_pos(i, q->capacity);
    } while (i != q->tail);

    printf("\n");
}
