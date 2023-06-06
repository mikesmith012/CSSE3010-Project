/** 
 **************************************************************
 * @file s2/main.c
 * @author Mike Smith - 46408789
 * @date 15032022
 * @brief main file for stage 2 (board: nucleo-f401)
 * REFERENCE: csse3010_stage2.pdf (task sheet)
 ***************************************************************
 */

#include <s4640878_joystick.h>
#include <s4640878_lta1000g.h>
#include <s4640878_pantilt.h>
#include "board.h"
#include "processor_hal.h"

// function declaration for hardware_init()
void hardware_init(void);

// the main function for stage 2
int main(void) {

    HAL_Init();
    hardware_init();

    while (1) {
        // get joystick readings (x and y values)
        int xValue = S4640878_REG_JOYSTICK_X_READ();
        int yValue = S4640878_REG_JOYSTICK_Y_READ();

        // add offset to joystick x and y values
        xValue += S4640878_REG_JOYSTICK_X_ZERO_CAL_OFFSET;
        yValue += S4640878_REG_JOYSTICK_Y_ZERO_CAL_OFFSET;

        // scale the adc readings to degrees
        float xDegrees = ((float)xValue / S4640878_LIB_ADC_SCALE) - 90;
        float yDegrees = ((float)yValue / S4640878_LIB_ADC_SCALE) - 90;

        // add offset to the angle for pan and tilt
        xDegrees += (float)S4640878_REG_PANTILT_PAN_90_CAL_OFFSET;
        yDegrees += (float)S4640878_REG_PANTILT_TILT_90_CAL_OFFSET;

        // write angle to pan and tilt
        S4640878_REG_PANTILT_PAN_WRITE(xDegrees);
        S4640878_REG_PANTILT_TILT_WRITE(yDegrees);

        // read pan angle
        int xAngle = S4640878_REG_PANTILT_PAN_READ() + 90;

        // writes to led array to display current pan angle
        if (xAngle < 20) s4640878_reg_lta1000g_write(1 << 0);
        else if (xAngle < 40) s4640878_reg_lta1000g_write(1 << 1);
        else if (xAngle < 60) s4640878_reg_lta1000g_write(1 << 2);
        else if (xAngle < 80) s4640878_reg_lta1000g_write(1 << 3);
        else if (xAngle < 100) s4640878_reg_lta1000g_write(1 << 4);
        else if (xAngle < 120) s4640878_reg_lta1000g_write(1 << 5);
        else if (xAngle < 140) s4640878_reg_lta1000g_write(1 << 6);
        else if (xAngle < 160) s4640878_reg_lta1000g_write(1 << 7);
        else if (xAngle < 180) s4640878_reg_lta1000g_write(1 << 8);
        else s4640878_reg_lta1000g_write(1 << 9);
        
        HAL_Delay(10);
    }
    return 0;
}

// initialises hardware
void hardware_init(void) {
    s4640878_reg_lta1000g_init();
    s4640878_reg_joystick_pb_init();
    s4640878_reg_pantilt_init();
    s4640878_reg_joystick_init();
}