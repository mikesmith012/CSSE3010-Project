/** 
 **************************************************************
 * @file mylib/s4640878_hamming.h
 * @author Mike Smith - 46408789
 * @date 30032022
 * @brief mylib hamming library (header file)
 *        (board: nucleo-f401)
 * REFERENCE: csse3010_mylib_lib_hamming.pdf (spec sheet)
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4640878_lib_hamming_byte_encode();
 * s4640878_lib_hamming_byte_decode();
 * s4640878_lib_hamming_parity_error();
 *************************************************************** 
 */

#ifndef S4640878_HAMMING_H_
#define S4640878_HAMMING_H_

unsigned short s4640878_lib_hamming_byte_encode(unsigned char value);
unsigned char s4640878_lib_hamming_byte_decode(unsigned char value);
int s4640878_lib_hamming_parity_error(unsigned char value);
int s4640878_lib_hamming_get_bit_error(void);

#endif