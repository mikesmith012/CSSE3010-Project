/** 
 **************************************************************
 * @file mylib/s4640878_hamming.c
 * @author Mike Smith - 46408789
 * @date 30032022
 * @brief mylib hamming library (c file)
 *        (board: nucleo-f401)
 * REFERENCE: csse3010_mylib_lib_hamming.pdf (spec sheet)
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4640878_lib_hamming_byte_encode();
 * s4640878_lib_hamming_byte_decode();
 * s4640878_lib_hamming_parity_error();
 * s4640878_lib_hamming_get_bit_error();
 *************************************************************** 
 */

#include "s4640878_hamming.h"
#include "s4640878_lta1000g.h"
#include "board.h"
#include "processor_hal.h"

// function declariations (internal)
unsigned char hamming_hbyte_encode(unsigned char value);

// global variables
static uint8_t bitErrors = 0;

// returns a 16-bit encoded value of a byte
// calls hamming_hbyte_encode()
// INPUT: 1-byte (8-bit) value, OUTPUT: 2-byte (16-bit) value
unsigned short s4640878_lib_hamming_byte_encode(unsigned char value) {
    unsigned short out;

    // Encode D0 to D3, then D4 to D7
    out = hamming_hbyte_encode(value & 0xF) | (hamming_hbyte_encode(value >> 4) << 8);
    return out;
}

// returns the decoded half byte from a byte
// detects up to 2-bit errors, corrects 1-bit errors
// INPUT: 1-byte (8-bit) value, OUTPUT: half-byte (4-bit) value
unsigned char s4640878_lib_hamming_byte_decode(unsigned char value) {
    
    // check for parity error
    uint8_t parityError = s4640878_lib_hamming_parity_error(value);

    // extract hamming and data bits
    uint8_t p0 = !!(value & (1 << 0));
    uint8_t h0 = !!(value & (1 << 1));
    uint8_t h1 = !!(value & (1 << 2));
    uint8_t h2 = !!(value & (1 << 3));
    uint8_t d0 = !!(value & (1 << 4));
    uint8_t d1 = !!(value & (1 << 5));
    uint8_t d2 = !!(value & (1 << 6));
    uint8_t d3 = !!(value & (1 << 7));

    // check for hamming parity errors
    uint8_t h0err = h0 ^ d1 ^ d2 ^ d3;
    uint8_t h1err = h1 ^ d0 ^ d2 ^ d3;
    uint8_t h2err = h2 ^ d0 ^ d1 ^ d3;
    uint8_t hammingError = h0err | h1err | h2err;

    // checks for bit errors
    if ((parityError && hammingError) || parityError) {
        // corrects hamming parity errors
        d0 ^= (!h0err & h1err & h2err);
        d1 ^= (h0err & !h1err & h2err);
        d2 ^= (h0err & h1err & !h2err);
        d3 ^= (h0err & h1err & h2err);
        bitErrors = 1;
    } else if (hammingError) {
        bitErrors = 2;
    } else {
        bitErrors = 0;
    }

    // generate output
    unsigned char out = (d0 << 0) | (d1 << 1) | (d2 << 2) | (d3 << 3);
    return out;
}

// internal function to encode a half byte
// called by s4640878_lib_hamming_byte_encode()
// INPUT: half-byte (4-bit) value, OUTPUT: 1-byte (8-bit) value
unsigned char hamming_hbyte_encode(unsigned char value) {
    
    // extract data bits
    uint8_t d0 = !!(value & 0x1);
    uint8_t d1 = !!(value & 0x2);
    uint8_t d2 = !!(value & 0x4);
    uint8_t d3 = !!(value & 0x8);

    // calculate hamming parity bits
    uint8_t h0 = d1 ^ d2 ^ d3;
    uint8_t h1 = d0 ^ d2 ^ d3;
    uint8_t h2 = d0 ^ d1 ^ d3;

    // generate out byte without parity bit P0
    unsigned char out = (h0 << 1) | (h1 << 2) | (h2 << 3) | (d0 << 4) | (d1 << 5) | (d2 << 6) | (d3 << 7);

    // calculate even parity bit
    uint8_t p0 = 0;
    for (int i = 1; i < 8; i++) {
        p0 = p0 ^ !!(out & (1 << i));
    }
    out |= p0;
    return out;
}

// returns 1 if parity error occurs, returns 0 otherwise
int s4640878_lib_hamming_parity_error(unsigned char value) {
    
    // extract parity bit
    uint8_t p0 = !!(value & (1 << 0));

    // check for parity error
    uint8_t p0err = 0;
    for (int i = 1; i < 8; i++) {
        p0err = p0err ^ !!(value & (1 << i));
    }
    return p0err ^ p0;
}

// returns number of bit errors
int s4640878_lib_hamming_get_bit_error(void) {
    return bitErrors;
}