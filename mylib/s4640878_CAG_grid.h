/** 
 **************************************************************
 * @file mylib/s4640878_CAG_grid.h
 * @author Mike Smith - 46408789
 * @date 06052022
 * @brief CAGGrid (header file)
 *        (board: nucleo-f401)
 * REFERENCE: csse3010_project.pdf (spec sheet)
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4640878_tsk_CAG_grid_init() - initialises CAG grid
 *************************************************************** 
 */

#ifndef S4640878_CAG_GRID_H_
#define S4640878_CAG_GRID_H_

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"

// CAGGrid task definitions
#define CAG_GRID_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define CAG_GRID_TASK_STACKSIZE (configMINIMAL_STACK_SIZE * 2)

// external function declarations
void s4640878_tsk_CAG_grid_init(void);

#endif