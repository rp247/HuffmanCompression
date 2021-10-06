#include "node.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

/* constructor for a node */
Node *node_create(uint8_t symbol, uint64_t frequency) {
    Node *n = (Node *) malloc(sizeof(Node));

    if (n) {
        n->left = NULL; // no child initially
        n->right = NULL;
        n->symbol = symbol;
        n->frequency = frequency;
    }

    return n;
}

/* destructor for a node */
void node_delete(Node **n) {
    if (n && *n) {
        free(*n);
        *n = NULL; // done with node
    }
    return;
}

/* joins two nodes into a new one */
Node *node_join(Node *left, Node *right) {
    if (!left || !right)
        return NULL; // no left and right. can't join

    Node *n = node_create('$', left->frequency + right->frequency); // create a new joint node
    n->left = left;
    n->right = right;

    return n;
}

/* prints a node */
void node_print(Node *n) {
    if (!n)
        return;

    fprintf(stdout, "{left: %p. right: %p. symbol: %c. freq: %" PRIu64 ".}\n", (void *) n->left,
        (void *) n->right, n->symbol, n->frequency);

    return;
}
