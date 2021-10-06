#include "code.h"
#include "header.h"
#include "huffman.h"
#include "io.h"
#include "node.h"
#include "pq.h"
#include "stack.h"

#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define BYTE 8

/* helper function to print usage (credits: idea from error c file in lab5) */
static void usage(char *argv) {
    fprintf(stderr,
        "SYNOPSIS\n"
        "  A Huffman decoder.\n"
        "  Decompresses a file using the Huffman coding algorithm.\n"
        "\n"
        "USAGE\n"
        "  ./%s [-h] [-i infile] [-o outfile]\n"
        "\n"
        "OPTIONS\n"
        "  -h             Program usage and help.\n"
        "  -v             Print compression statistics.\n"
        "  -i infile      Input file to decompress.\n"
        "  -o outfile     Output of decompressed data.\n",
        argv);

    return;
}

/* helper function to close files open in main */
static void main_err(int in, int out) {
    close(in);
    close(out);
    return;
}

/* helper function to determine if a node is leaf or not */
static bool is_leaf(Node *n) {
    if (n)
        return (!n->left && !n->right); // no child nodes then true, else false
    return false;
}

int main(int argc, char **argv) {
    int c;
    char *optlist = "hvi:o:";
    uint8_t verbose = 0; // no set since only one arg checked/added

    /* default file values */
    int infile = STDIN_FILENO;
    int outfile = STDOUT_FILENO;

    /* parse args and open file */
    while ((c = getopt(argc, argv, optlist)) != -1) {
        switch (c) {
        case 'h': usage(argv[0]); return 0;

        case 'i':
            infile = open(optarg, O_RDONLY); // open read only and handle err
            if (infile == -1) {
                fprintf(stderr, "Error: Cannot open input file.\n");
                return -1;
            }
            break;

        case 'o':
            outfile = open(optarg,
                O_CREAT | O_WRONLY); // open write only (create if not present) and handle err
            if (outfile == -1) {
                fprintf(stderr, "Error: Cannot open output file.\n");
                main_err(infile, -1);
                return -1;
            }
            break;

        case 'v': verbose = 1; break;

        default: usage(argv[0]); return -1;
        }
    }

    /* CREDITS: Modified version (for err handling) of the code snippet in the lab documentation */
    /* file permission setting */
    struct stat statbuf;
    uint64_t comp_fz
        = 0; // tracks compressed file size (to avoid copying file in case infile=stdin)

    /* get infile stats */
    if (fstat(infile, &statbuf) != 0) {
        fprintf(stderr, "Failed to get input file stat.\n");
        main_err(infile, outfile);
        return -1;
    }

    /* read in the header */
    Header h;
    comp_fz += (uint64_t) read_bytes(infile, (uint8_t *) &h,
        sizeof(Header)); // read in header and update compressed file size by it

    /* different magic number */
    if (h.magic != MAGIC) {
        fprintf(stderr, "Magic number does not match.\n");
        main_err(infile, outfile);
        return -1;
    }

    /* invalid ( > MAX_TREE_SIZE) tree size */
    if (h.tree_size > MAX_TREE_SIZE) {
        fprintf(stderr, "Invalid ( > MAX_TREE_SIZE) tree size.");
        main_err(infile, outfile);
        return -1;
    }

    /* change output file mode */
    if (fchmod(outfile, h.permissions) != 0) {
        fprintf(stderr, "Could not change mode for output file.\n");
        main_err(infile, outfile);
        return -1;
    }

    /* rebuild the huffman tree */
    uint16_t tree_size = h.tree_size;
    uint8_t *tree_dump = (uint8_t *) calloc(
        tree_size, sizeof(uint8_t)); // buffer to store tree dump (to use write bytes later)

    /* read in the dumped tree and increase compressed file size*/
    comp_fz += (uint64_t) read_bytes(infile, tree_dump, tree_size);

    Node *root = rebuild_tree(tree_size, tree_dump);
    free(tree_dump);
    tree_dump = NULL; // done with tree

    /* decompress */
    uint8_t *buffer = (uint8_t *) calloc(BLOCK, sizeof(uint8_t)); // buffer to hold read in bytes
    uint16_t at = 0; // index in buffer
    uint8_t bit_val; // read in bit val
    uint64_t tot_decoded = 0; // decompressed file size
    Node *temp = root; // for traversing the tree
    uint64_t temp_comp_fz = 0; // tracks totals bits read (to get total bytes read later)

    /* parse the tree, buffer symbols, and write them out */
    while (tot_decoded < h.file_size) {

        if (read_bit(infile, &bit_val))
            temp_comp_fz++; // total bits read++ if true

        /* reached leaf */
        if (is_leaf(temp)) {
            buffer[at] = temp->symbol; // store the symbol
            temp = root; // reinitialize
            tot_decoded++; // decompressed file size++
            at++; // index incremented
        }

        /* buffer full. write out codes */
        if (at == BLOCK) {
            write_bytes(outfile, buffer, at);
            at = 0; // reset the index
        }

        /* keep traversing */
        if (bit_val)
            temp = temp->right; // bit = 1 go down right, else left
        else
            temp = temp->left;
    }

    /* flush the remaining ones */
    if (at != 0)
        write_bytes(outfile, buffer, at);

    free(buffer);
    buffer = NULL; // done with the buffer

    /* print statistics */
    if (verbose) {

        /* compressed file size = minimum bytes for total bits read in + total bytes read in */
        temp_comp_fz = temp_comp_fz / BYTE == 0 ? 1 : temp_comp_fz / BYTE + 1;
        comp_fz += temp_comp_fz;

        fprintf(stderr, "Compressed file size: %" PRIu64 " bytes\n", comp_fz);
        fprintf(stderr, "Deompressed file size: %" PRIu64 " bytes\n", tot_decoded);

        double space_save
            = (100
                * (1
                    - (((double) comp_fz)
                        / tot_decoded))); // formula credit: provided in the lab documentation
        fprintf(stderr, "Space saving: %0.2lf%%\n", space_save);
    }

    /* free mem, close files */
    main_err(infile, outfile);
    delete_tree(&root);
    return 0;
}
