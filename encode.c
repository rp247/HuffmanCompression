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

/* temp file used to store data if the input is from stdin (to seek again later) */
#define TEMP_FILE "temp_infile.txt"

/* to keep track of unique elements (used for tree size) */
static uint16_t unique_sym = 2; // 2 since elem 0 and 255 are always incremented

/* helper function to print usage (credits: idea from error c file in lab5) */
static void usage(char *argv) {
    fprintf(stderr,
        "SYNOPSIS\n"
        "  A Huffman encoder.\n"
        "  Compresses a file using the Huffman coding algorithm.\n"
        "\n"
        "USAGE\n"
        "  ./%s [-h] [-i infile] [-o outfile]\n"
        "\n"
        "OPTIONS\n"
        "  -h             Program usage and help.\n"
        "  -v             Print compression statistics.\n"
        "  -i infile      Input file to compress.\n"
        "  -o outfile     Output of compressed data.\n",
        argv);

    return;
}

/* helper function to close files open in main */
static void main_err(int in, int out, int temp_fd) {
    close(in);
    close(out);
    close(temp_fd);
    return;
}

/* credits: modified version of tally function in entropy.c */
/* helper function to compute histogram of a file */
static void compute_hist(int infile, uint64_t *hist, int temp_fd) {
    int tot_read;

    uint8_t *buffer = (uint8_t *) calloc(BLOCK, sizeof(uint8_t)); // ~4KB of mem

    /* read BLOCK till EOF */
    while ((tot_read = read_bytes(infile, buffer, BLOCK)) > 0) {

        /* save stdin input to the temp file if infile == stdin (to read the file again later) */
        if (infile == STDIN_FILENO)
            write_bytes(temp_fd, buffer, tot_read);

        /* increment the character encounter in histogram */
        for (uint16_t i = 0; i < tot_read; i++) {
            /* was zero, now it's not. therefore unique */
            if (hist[buffer[i]] == 0)
                unique_sym++;
            hist[buffer[i]]++;
        }
    }

    free(buffer);
    buffer = NULL;

    return;
}

/* helper function to determine if node's leaf or not */
static bool is_leaf(Node *n) {
    if (n)
        return (!n->left && !n->right); // no child then true, else false
    return false;
}

/* helper function to write the tree dump to an array */
static void tree_dump(Node *n, uint8_t *tree) {
    static uint16_t at = 0; // keeps track of index in the tree array

    /* at leaf. write L[symbol] */
    if (is_leaf(n)) {
        tree[at] = 'L';
        tree[at + 1] = n->symbol; // write leaf and it's symbol
        at += 2; // skip over the symbol
        return;
    }

    /* recurse from left and right nodes */
    tree_dump(n->left, tree);
    tree_dump(n->right, tree);

    tree[at] = 'I';
    at++; // print the parent node. increment array index

    return;
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
            infile = open(optarg, O_RDONLY); // read only input file
            if (infile == -1) {
                fprintf(stderr, "Error: Cannot open input file.\n");
                return -1;
            }
            break;

        case 'o':
            outfile
                = open(optarg, O_CREAT | O_WRONLY); // write only output file. create if not present
            if (outfile == -1) {
                fprintf(stderr, "Error: Cannot open output file.\n");
                main_err(infile, 0, 0);
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
    uint64_t comp_fz = 0;

    /* credits: idea from replies on piazza post 749 */
    /* handle case where input file is stdin */
    const char *temp_infile = NULL;
    int temp_fd = infile;
    if (infile == STDIN_FILENO) {
        temp_infile = TEMP_FILE; // the temporary file

        /* create a file to read & write with only user allowed to read & write */
        temp_fd = open(temp_infile, O_CREAT | O_RDWR, 0600);

        /* cannot create temp file */
        if (temp_fd == -1) {
            fprintf(stderr, "Error: Cannot open temporary file to direct stdin input.\n");
            main_err(infile, outfile, 0);
            return -1;
        }
    }

    /* histogram of a file */
    uint64_t hist[ALPHABET] = { 0 };
    hist[0]++;
    hist[255]++;
    compute_hist(infile, hist, temp_fd);

    /* get infile stats (temp file if infile == stdin, else infile) */
    if (fstat(infile == STDIN_FILENO ? temp_fd : infile, &statbuf) != 0) {
        fprintf(stderr, "Failed to get input file stat.\n");
        main_err(infile, outfile, 0);
        if (temp_infile)
            remove(temp_infile);
        return -1;
    }

    /* change output file mode */
    if (fchmod(outfile, statbuf.st_mode) != 0) {
        fprintf(stderr, "Could not change mode for output file.\n");
        main_err(infile, outfile, 0);
        if (temp_infile)
            remove(temp_infile); // delete the temp file
        return -1;
    }

    /* construct a huffman tree */
    Node *root = build_tree(hist);

    /* construct a code table */
    Code temp_code = code_init(); // to avoid non-zero elem errors in Code table
    Code table[ALPHABET] = { temp_code };
    build_codes(root, table);

    /* construct and write the header structure (tree size formula credit: from the lab document) */
    uint16_t tree_size = (3 * unique_sym) - 1; // tree size (number of nodes in the tree)
    Header h = { .magic = MAGIC,
        .permissions = (uint16_t) statbuf.st_mode,
        .tree_size = tree_size,
        .file_size = (uint64_t) statbuf.st_size };

    comp_fz += write_bytes(outfile, (uint8_t *) &h,
        sizeof(Header)); // write the header and update compressed file size

    /* tree dump */
    uint8_t *tree
        = (uint8_t *) calloc(tree_size, sizeof(uint8_t)); // made array to make use of write_bytes
    tree_dump(root, tree);
    comp_fz += write_bytes(outfile, tree, tree_size); // increment the compressed file size
    free(tree);
    tree = NULL; // done with tree

    /* writes code for each byte in infile to outfile */

    /* seek to beginning of this file (tempfile if infile == stdin, else infile) */
    int seek_from_here = (infile == STDIN_FILENO) ? temp_fd : infile;

    /* seek to the beginning of the file and handle errors */
    if (lseek(seek_from_here, 0, SEEK_SET) == -1) {
        fprintf(stderr, "Failed to seek the beginning of input file.\n");
        main_err(infile, outfile, temp_fd);
        if (temp_infile)
            remove(temp_infile); //delete the temp file
        delete_tree(&root);
        return -1;
    }

    /* write each corresponding codes to outfile */
    uint8_t *buffer = (uint8_t *) calloc(BLOCK, sizeof(uint8_t)); // ~4KB of mem
    int tot_read;
    uint64_t temp_comp_fz = 0; // tracks number of bits written (for compressed file size tracking)

    /* read BLOCK till EOF */
    while ((tot_read = read_bytes(seek_from_here, buffer, BLOCK)) > 0) {
        /* increment the character encounter in histogram */
        for (uint16_t i = 0; i < tot_read; i++) {
            /* write code for the correesponding byte (code already in code table) */
            write_code(outfile, &table[buffer[i]]);
            temp_comp_fz += table[buffer[i]].top; // increment total bits written
        }
    }

    /* flush any remaining codes */
    flush_codes(outfile);

    free(buffer);
    buffer = NULL; // done with buffer

    /* print statistics */
    if (verbose) {

        /* compressed file size = minimum bytes for total bits read in + total bytes read in */
        temp_comp_fz = temp_comp_fz / BYTE == 0 ? 1 : temp_comp_fz / BYTE + 1;
        comp_fz += temp_comp_fz;

        fprintf(stderr, "Uncompressed file size: %" PRIu64 " bytes\n", h.file_size);
        fprintf(stderr, "Compressed file size: %" PRIu64 " bytes\n", comp_fz);

        double space_save
            = (100
                * (1
                    - (((double) comp_fz)
                        / h.file_size))); // formula credit: provided in the lab documentation
        fprintf(stderr, "Space saving: %0.2lf%%\n", space_save);
    }

    /* free mem, close files */
    main_err(infile, outfile, temp_fd);
    if (temp_infile)
        remove(temp_infile); // delete the temp file
    delete_tree(&root);
    return 0;
}
