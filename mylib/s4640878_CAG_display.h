/** 
 **************************************************************
 * @file mylib/s4640878_CAG_display.h
 * @author Mike Smith - 46408789
 * @date 06052022
 * @brief CAGDisplay (header file)
 *        (board: nucleo-f401)
 * REFERENCE: csse3010_project.pdf (spec sheet)
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4640878_tsk_CAG_display_init() - initialises CAG display
 *************************************************************** 
 */

#ifndef S4640878_CAG_DISPLAY_H_
#define S4640878_CAG_DISPLAY_H_

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

// CAGDisplay task definitions
#define CAG_DISPLAY_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define CAG_DISPLAY_TASK_STACKSIZE (configMINIMAL_STACK_SIZE * 2)

// external function declarations
void s4640878_tsk_CAG_display_init(void);

#endif