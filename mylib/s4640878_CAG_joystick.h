/** 
 **************************************************************
 * @file mylib/s4640878_CAG_joystick.c
 * @author Mike Smith - 46408789
 * @date 06052022
 * @brief CAGJoystick (c file)
 *        (board: nucleo-f401)
 * REFERENCE: csse3010_project.pdf (spec sheet)
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4640878_tsk_CAG_joystick_init() - initialises CAG joystick
 * s4640878_tsk_CAG_joystick_del() - deletes CAG joystick
 *************************************************************** 
 */

#ifndef S4640878_CAG_JOYSTICK_H_
#define S4640878_CAG_JOYSTICK_H_

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "s4640878_joystick.h"

// CAGJoystick task definitions
#define CAG_JOYSTICK_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define CAG_JOYSTICK_TASK_STACKSIZE (configMINIMAL_STACK_SIZE * 2)

// external function declarations
void s4640878_tsk_CAG_joystick_init(void);
void s4640878_tsk_CAG_joystick_del(void);

#endif