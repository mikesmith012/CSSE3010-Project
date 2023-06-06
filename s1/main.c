/** 
 **************************************************************
 * @file s1/main.c
 * @author Mike Smith - 46408789
 * @date 22022022
 * @brief main file for stage 1
 * REFERENCE: csse3010_stage1.pdf (task sheet)
 ***************************************************************
 */

#include <s4640878_joystick.h>
#include <s4640878_lta1000g.h>
#include "board.h"
#include "processor_hal.h"

// function declaration for hardware_init()
void hardware_init(void);

// the main function for stage 1
int main(void) {

    HAL_Init();
    hardware_init();

    uint32_t prevTick = 0;
    while (1) {
        if ((HAL_GetTick() - prevTick) > 10) {
            unsigned short count = s4640878_reg_joystick_press_get();
            s4640878_reg_lta1000g_write(count);
            prevTick = HAL_GetTick();
        }
    }

    return 0;
}

// initialises hardware
void hardware_init(void) {
    s4640878_reg_lta1000g_init();
    s4640878_reg_joystick_pb_init();
}