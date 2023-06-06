/** 
 **************************************************************
 * @file mylib/s4640878_oled.h
 * @author Mike Smith - 46408789
 * @date 12042022
 * @brief oled library (header file)
 *        (board: nucleo-f401)
 * REFERENCE: csse3010_mylib_reg_oled.pdf (spec sheet)
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4640878_reg_oled_init() - initialise the oled
 * s4640878_tsk_oled_init() - created controlling task for the oled
 *************************************************************** 
 */

#ifndef S4640878_OLED_H_
#define S4640878_OLED_H_

#include "oled_pixel.h"
#include "oled_string.h"
#include "fonts.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// oled task definitions
#define OLED_TASK_PRIORITY (tskIDLE_PRIORITY + 3)
#define OLED_TASK_STACKSIZE (configMINIMAL_STACK_SIZE * 2)

// struct for oled text message
struct oledTextMsg {
    int startX;
    int startY;
    char displayText[20];
};

// oled message queue
QueueHandle_t s4640878QueueOledMsg;

// external function declarations
void s4640878_reg_oled_init(void);
void s4640878_tsk_oled_init(void);

#endif