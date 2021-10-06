#include "io.h"

#include "code.h"
#include "defines.h"

#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define BYTE 8

/* extern vars */
uint64_t bytes_read = 0;
uint64_t bytes_written = 0;

/* array for buffer input or output */
static uint8_t buffer[BLOCK] = { 0 }; // bufer that can hold 4096 bytes
static uint16_t bufind = 0; // keeps track of index into buffer

/* helper function to set the bit at ind in buf (based on bv in lab5) */
static void set_bit(uint8_t *buf, uint16_t ind) {
    buf[ind / BYTE] |= ((uint8_t) 1 << (ind % BYTE));
    return;
}

/* helper function to clear the bit at ind in buf (based on bv in lab5) */
static void clr_bit(uint8_t *buf, uint16_t ind) {
    buf[ind / BYTE] &= ~((uint8_t) 1 << (ind % BYTE));
    return;
}

/* helper function to get the bit at ind in buf (based on bv in lab5) */
static uint8_t get_bit(uint8_t *buf, uint16_t ind) {
    return (buf[ind / BYTE] & ((uint8_t) 1 << (ind % BYTE))) ? 1 : 0;
}

/* reads nbytes from infile into buffer buf */
int read_bytes(int infile, uint8_t *buf, int nbytes) {
    int remaining = nbytes; // all remaining
    int read_ret = 1; // holds return value of read syscall

    /* still remaining and return val != EOF or error (>0) */
    while (
        remaining != 0 && (read_ret = read(infile, buf, remaining)) > 0) { // try to read remaining
        remaining -= read_ret; // reduce remaining by how many read
        bytes_read += read_ret; // update total bytes read
        buf += read_ret; // update the pointer (buf for next read)
    }

    uint64_t total_read = bytes_read;
    bytes_read = 0; // reset for next call

    return total_read;
}

/* writes nbytes from buf to outfile */
int write_bytes(int outfile, uint8_t *buf, int nbytes) {
    int remaining = nbytes; // all remaining
    int write_ret = 1; // holds return value write syscall

    /* still remaining and return val != EOF or error (>0) */
    while (remaining != 0
           && (write_ret = write(outfile, buf, remaining)) > 0) { // try to write remaining
        remaining -= write_ret; // reduce remaining by how many write
        bytes_written += write_ret; // update total bytes write
        buf += write_ret; // update the pointer (buf for next write)
    }

    uint64_t total_written = bytes_written;
    bytes_written = 0; // reset for next call

    return total_written;
}

/* reads one bit out of a buffer */
bool read_bit(int infile, uint8_t *bit) {

    /* read in if empty */
    if (bufind == 0) {
        bytes_read = read_bytes(infile, buffer, BLOCK);
    }

    /* get and store the bit */
    if (bit) {
        *bit = get_bit(buffer, bufind);
    }

    bufind++; // increment for next bit

    /* got all bits */
    if (bufind == BLOCK * 8) {
        bufind = 0; // reset for next call
    }

    return bufind < bytes_read * 8; // more to read if bits read are < bytesread*8 (total bits read)
}

/* writes a code to outfile */
void write_code(int outfile, Code *c) {
    for (uint32_t i = 0; i < c->top; i++) {
        /* set or clear (based on the value in code) bit in buffer */
        if (get_bit(c->bits, (uint16_t) i))
            set_bit(buffer, bufind);
        else
            clr_bit(buffer, bufind);

        bufind++; // increment bufind

        /* buffer full. write out */
        if (bufind == BLOCK * 8) {
            write_bytes(outfile, buffer, BLOCK);
            bufind = 0; // reset index
        }
    }

    return;
}

/* flushes any remaining code in the buffer */
void flush_codes(int outfile) {

    /* still bytes left */
    if (bufind != 0) {
        uint16_t min_bytes
            = (bufind / BYTE) == 0 ? 1 : (bufind / BYTE) + 1; // minimum bytes to be written

        /* zero out remaining bits before writing */
        for (uint16_t i = bufind; i < min_bytes * 8; i++) {
            clr_bit(buffer, i); // clear bit
        }

        /* write out the bytes */
        write_bytes(outfile, buffer, min_bytes);

        bufind = 0; // reset index
    }

    return;
}
