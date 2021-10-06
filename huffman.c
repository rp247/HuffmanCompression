#include "huffman.h"

#include "code.h"
#include "defines.h"
#include "node.h"
#include "pq.h"
#include "stack.h"

#include <stdint.h>
#include <stdlib.h>

/* algo credits: based upon the lab document description */
/* builds the huffman tree from a given histogram */
Node *build_tree(uint64_t hist[static ALPHABET]) {

    PriorityQueue *q = pq_create(ALPHABET); // max 256 elements. dynamic size made messy code
    Node *n = NULL, *left, *right; // temp Node pointer holders

    /* iterate over each elem in pq */
    for (uint16_t i = 0; i < ALPHABET; i++) {

        /* create and ins node if freq > 0 */
        if (hist[i] > 0) {
            n = node_create(i, hist[i]);
            enqueue(q, n);
        }
    }

    /* build a tree. dequeue two nodes, join & enqueue them */
    while (pq_size(q) > 1) {
        dequeue(q, &left);
        dequeue(q, &right);
        n = node_join(left, right);
        enqueue(q, n);
    }

    /* free pq mem */
    pq_delete(&q);

    return n; // the root node
}

/* returns true if a node is leaf, else false */
static bool is_leaf(Node *n) {
    if (n)
        return (!n->left && !n->right); // no child node then true, else false
    return false;
}

/* algo credits: based upon the lab document description */
/* recursive helper function to build codes for huffman tree */
static void code_traverse(Node *n, Code table[static ALPHABET], Code *c) {

    /* found a leaf. add its code */
    if (is_leaf(n)) {
        table[n->symbol] = *c;
        return;
    }

    /* recurse down left */
    code_push_bit(c, 0); // 0 for left
    code_traverse(n->left, table, c);
    code_pop_bit(c, NULL); // done with left

    /* recurse down right */
    code_push_bit(c, 1); // 1 for right
    code_traverse(n->right, table, c);
    code_pop_bit(c, NULL); // done with right

    return;
}

/* builds and stores codes for huffman tree */
void build_codes(Node *root, Code table[static ALPHABET]) {
    Code c = code_init();
    code_traverse(root, table, &c); // call the helper function
    return;
}

/* algo credits: based upon the lab document description */
/* builds a huffman tree from a tree dump */
Node *rebuild_tree(uint16_t nbytes, uint8_t tree[static nbytes]) {
    Node *n = NULL, *left = NULL, *right = NULL;
    Stack *s = stack_create(ALPHABET); // max stack size can grow to 256

    for (uint16_t i = 0; i < nbytes; i++) {
        /* add a leaf node */
        if (tree[i] == 'L') {
            n = node_create(tree[i + 1], 0); // add a node with symbol = char followed by L
            stack_push(s, n); // push to stack (node freq doesnt matter)
            i += 1; // skip next iteration
        }
        /* at parent node. pop two nodes, join & push them */
        else {
            stack_pop(s, &right);
            stack_pop(s, &left);
            n = node_join(left, right);
            stack_push(s, n);
        }
    }

    /* free stack mem */
    stack_delete(&s);

    return n; // the root node
}

/* deletes and frees memory for a huffman tree */
void delete_tree(Node **root) {

    /* free leafs */
    if (root && is_leaf(*root)) {
        node_delete(root);
        return;
    }

    /* recursively free all the left and right nodes */
    if (root) {
        delete_tree(&((*root)->left));
        delete_tree(&((*root)->right));
        node_delete(root); // free the parent nodes
    }

    return;
}
