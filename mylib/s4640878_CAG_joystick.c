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

#include "s4640878_CAG_joystick.h"
#include "s4640878_CAG_simulator.h"
#include "board.h"
#include "processor_hal.h"

// internal variables
static TaskHandle_t xHandleCAGJoystick = NULL;     // CAGJoystick task handler

// internal function declarations
void s4640878TaskCAGJoystick(void);

// controlling task for CAGJoystick
void s4640878TaskCAGJoystick(void) {
    struct joystickXY joystickMsg;
    EventBits_t uxBits;
    for(;;) {
        // joystick x and y queue
        if (s4640878QueueJoystick != NULL) {
            if (xQueueReceive(s4640878QueueJoystick, &joystickMsg, 10)) {
                // joystick x
                if (joystickMsg.x < 1000) {
                    uxBits = xEventGroupSetBits(GroupEventCAGSimulator, STOP_SIMULATION); // pause
                } else if (joystickMsg.x > 3000) {
                    uxBits = xEventGroupSetBits(GroupEventCAGSimulator, START_SIMULATION); // play
                }
                // joystick y
                if (joystickMsg.y < 500) {
                    uxBits = xEventGroupSetBits(GroupEventCAGSimulator, UPDATE_1000MS); // update 1000ms
                } else if (joystickMsg.y < 1500) {
                    uxBits = xEventGroupSetBits(GroupEventCAGSimulator, UPDATE_1500MS); // update 1500ms
                } else if (joystickMsg.y < 2500) {
                    uxBits = xEventGroupSetBits(GroupEventCAGSimulator, UPDATE_2000MS); // update 2000ms
                } else if (joystickMsg.y < 3500) {
                    uxBits = xEventGroupSetBits(GroupEventCAGSimulator, UPDATE_5000MS); // update 5000ms
                } else {
                    uxBits = xEventGroupSetBits(GroupEventCAGSimulator, UPDATE_10000MS); // update 10000ms
                }
            }
        }
        // joystick z semaphore
        if (s4640878SemaphoreJoystickZ != NULL) {
            // checks for joystick z semaphore
            if (xSemaphoreTake(s4640878SemaphoreJoystickZ, 10) == pdTRUE) {
                uxBits = xEventGroupSetBits(GroupEventCAGSimulator, CLEAR_GRID); // clear grid
            }
        }
        vTaskDelay(100);    // delay 0.1s
    }
}

// task init function for CAGJoystick
void s4640878_tsk_CAG_joystick_init(void) {
    // creates the CAGJoystick task if one does not already exist
    if (xHandleCAGJoystick == NULL) {
        xTaskCreate((void*)&s4640878TaskCAGJoystick, "CAG_JOYSTICK", CAG_JOYSTICK_TASK_STACKSIZE, NULL, CAG_JOYSTICK_TASK_PRIORITY, &xHandleCAGJoystick);
    }
}

// task delete function for CAGJoystick
void s4640878_tsk_CAG_joystick_del(void) {
    // deletes the CAGJoystick task if one exists
    if (xHandleCAGJoystick != NULL) {
        vTaskDelete(xHandleCAGJoystick);
    }
    // set the task handle to NULL
    // signifies that task no longer exists
    xHandleCAGJoystick = NULL;   
}