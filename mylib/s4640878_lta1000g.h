/** 
 **************************************************************
 * @file mylib/s4640878_lta1000g.h
 * @author Mike Smith - 46408789
 * @date 22022022
 * @brief mylib lta1000g led array library (header file)
 * REFERENCE: csse3010_mylib_reg_lta1000g.pdf (task sheet)
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4640878_reg_lta1000g_init() - initialise led array
 * s4640878_reg_lta1000g_write() - writes low or high to each led in array
 *************************************************************** 
 */

#ifndef S4640878_LTA1000G_H_
#define S4640878_LTA1000G_H_

void s4640878_reg_lta1000g_init(void);
void lta1000g_seg_set(int segment, unsigned char segment_value);
void s4640878_reg_lta1000g_write(unsigned short value);

#endif