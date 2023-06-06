/** 
 **************************************************************
 * @file mylib/s4640878_irremote.h
 * @author Mike Smith - 46408789
 * @date 30032022
 * @brief mylib IR remote library (header file)
 *        (board: nucleo-f401)
 * REFERENCE: csse3010_mylib_reg_irremote.pdf (spec sheet)
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4640878_irremote_init();
 * s4640878_irremote_recv();
 * s4640878_irremote_readkey();
 *************************************************************** 
 */

#ifndef S4640878_IRREMOTE_H_
#define S4640878_IRREMOTE_H_

#include "board.h"

// timer ticks definitions
#define TIMER_RUNNING_FREQ 5000
#define MILLISECOND 10

// external function declarations
void s4640878_reg_irremote_init(void);
void s4640878_reg_irremote_recv(void);
int s4640878_reg_irremote_readkey(char* value);

#endif