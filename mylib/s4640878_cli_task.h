/** 
 **************************************************************
 * @file mylib/s4640878_cli_task.h
 * @author Mike Smith - 46408789
 * @date 06052022
 * @brief CLI Task (header file)
 *        (board: nucleo-f401)
 * REFERENCE: csse3010_project.pdf (spec sheet)
 *            csse3010_mylib_cli_peripheral_guide.pdf (mylib guide)
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4640878_cli_init() - initialises CLI
 *************************************************************** 
 */

#ifndef S4640878_CLI_TASK_H_
#define S4640878_CLI_TASK_H_

#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include "task.h"
#include "event_groups.h"
#include "s4640878_cli_CAG_mnemonic.h"

// CLI task definitions
#define CLI_TASK_PRIORITY (tskIDLE_PRIORITY + 0)
#define CLI_TASK_STACKSIZE (configMINIMAL_STACK_SIZE * 8)

// external function declarations
void s4640878_cli_init(void);

#endif