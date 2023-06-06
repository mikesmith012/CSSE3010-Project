/** 
 **************************************************************
 * @file mylib/s4640878_joystick.h
 * @author Mike Smith - 46408789
 * @date 22022022
 * @brief mylib joystick library (header file)
 * REFERENCE: csse3010_mylib_reg_joystick_pushbutton.pdf (task sheet)
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4640878_reg_joystick_pb_init() - initialises joystick
 * s4640878_reg_joystick_pb_isr() - joystick interrupt service routine 
 * s4640878_reg_joystick_press_get() - returns joystick press count
 * s4640878_reg_joystick_press_reset() - resets joystick press count 
 * S4640878_REG_JOYSTICK_X_READ() - reads the joystick x-value
 * S4640878_REG_JOYSTICK_Y_READ() - reads the joystick y-value
 * s4640878_tsk_joystick_pb_init() - controlling task for joystick pushbutton
 * s4640878_tsk_joystick_init() - controlling task for joystick x and y values
 *************************************************************** 
 */

#ifndef S4640878_JOYSTICK_H_
#define S4640878_JOYSTICK_H_

#include "processor_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

ADC_HandleTypeDef AdcHandleX, AdcHandleY;
ADC_ChannelConfTypeDef AdcChanConfigX, AdcChanConfigY;

// joystick pushbutton task definitions
#define JOYSTICKPB_TASK_PRIORITY (tskIDLE_PRIORITY + 0)
#define JOYSTICKPB_TASK_STACKSIZE (configMINIMAL_STACK_SIZE * 2)
SemaphoreHandle_t s4640878SemaphoreJoystickZ;

// joystick x and y values task definitions
#define JOYSTICKXY_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define JOYSTICKXY_TASK_STACKSIZE (configMINIMAL_STACK_SIZE * 2)

// reads joystick x any y values
#define S4640878_REG_JOYSTICK_X_READ() ({\
    HAL_ADC_ConfigChannel(&AdcHandleX, &AdcChanConfigX);\
    s4640878_joystick_readxy(AdcHandleX);\
})
#define S4640878_REG_JOYSTICK_Y_READ() ({\
    HAL_ADC_ConfigChannel(&AdcHandleY, &AdcChanConfigY);\
    s4640878_joystick_readxy(AdcHandleY);\
})

// joystick calibration values
#define S4640878_REG_JOYSTICK_X_ZERO_CAL_OFFSET 5
#define S4640878_REG_JOYSTICK_Y_ZERO_CAL_OFFSET 5

// scales adc reading to degrees
#define S4640878_LIB_ADC_SCALE (float)(4096 / 180)  

// function declarations for joystick pushbutton
void s4640878_reg_joystick_pb_init(void);
void s4640878_reg_joystick_pb_isr(void);
int s4640878_reg_joystick_press_get(void);
void s4640878_reg_joystick_press_reset(void);

// function declarations for joystick x and y
void s4640878_reg_joystick_init(void);
int s4640878_joystick_readxy(ADC_HandleTypeDef AdcHandler);

// function declaration for joystick tasks
void s4640878_tsk_joystick_pb_init(void);
void s4640878_tsk_joystick_init(void);

// joystick x and y values
struct joystickXY {
    int x;
    int y;
} joystickXY;

// joystick x and y queue
QueueHandle_t s4640878QueueJoystick;

#endif