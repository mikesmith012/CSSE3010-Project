/** 
 **************************************************************
 * @file pf/main.c
 * @author Mike Smith - 46408789
 * @date 06052022
 * @brief main file for final project (board: nucleo-f401)
 * REFERENCE: csse3010_project.pdf (task sheet)
 ***************************************************************
 */

#include <s4640878_CAG_simulator.h>
#include <s4640878_CAG_display.h>
#include <s4640878_CAG_grid.h>
#include <s4640878_CAG_joystick.h>
#include <s4640878_cli_task.h>

#include "board.h"
#include "processor_hal.h"
#include "FreeRTOS.h"


// the main function for final project
int main(void) {
    HAL_Init();
    
    // initializes tasks
    s4640878_tsk_CAG_simulator_init();
    s4640878_tsk_CAG_display_init();
    s4640878_tsk_CAG_grid_init();
    s4640878_tsk_CAG_joystick_init();
    s4640878_cli_CAG_mnemonic_init();
    s4640878_cli_init();
    s4640878_tsk_joystick_init();
    s4640878_tsk_joystick_pb_init();

    vTaskStartScheduler();
    return 0;
}