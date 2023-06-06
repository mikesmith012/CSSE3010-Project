/** 
 **************************************************************
 * @file s4/main.c
 * @author Mike Smith - 46408789
 * @date 12042022
 * @brief main file for stage 4 (board: nucleo-f401)
 * REFERENCE: csse3010_stage4.pdf (task sheet)
 ***************************************************************
 */

#include <s4640878_joystick.h>
#include <s4640878_lta1000g.h>
#include <s4640878_pantilt.h>
#include <s4640878_hamming.h>
#include <s4640878_irremote.h>
#include <s4640878_oled.h>

#include "board.h"
#include "processor_hal.h"
#include "debug_log.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "string.h"

// function declarations
void hardware_init(void);
void timer_init(void);

// dual timer message struct
struct dualTimerMsg {
    char type;
    unsigned short timerValue;
};

// timer message queue
QueueHandle_t s4640878QueueTimerMsg;

// task timer left
#define LEFT_TIMER_PRIORITY (tskIDLE_PRIORITY + 4)
#define LEFT_TIMER_STACKSIZE (configMINIMAL_STACK_SIZE * 2)
void TaskTimerLeft(void);

// task timer right
#define RIGHT_TIMER_PRIORITY (tskIDLE_PRIORITY + 3)
#define RIGHT_TIMER_STACKSIZE (configMINIMAL_STACK_SIZE * 2)
void TaskTimerRight(void);

// task timer display
#define DISPLAY_TIMER_PRIORITY (tskIDLE_PRIORITY + 3)
#define DISPLAY_TIMER_STACKSIZE (configMINIMAL_STACK_SIZE * 2)
void TaskTimerDisplay(void);

// timer mode: 0 for stop, 1 for start
static volatile int mode = 1;

// the main function for stage 4
int main(void) {
    HAL_Init();
    timer_init();   // creates tasks

    vTaskStartScheduler();
    return 0;
}

// initialise hardware
void hardware_init(void) {
    BRD_debuguart_init();               // debug
    s4640878_tsk_oled_init();           // oled
    s4640878_tsk_joystick_pb_init();    // joystick pushbutton
    s4640878_tsk_joystick_init();       // joystick x and y
}

// initialise timers & hardware
void timer_init(void) {
    xTaskCreate((void*)&TaskTimerLeft, "LEFT_TIMER", LEFT_TIMER_STACKSIZE, NULL, LEFT_TIMER_PRIORITY, NULL);
    xTaskCreate((void*)&TaskTimerRight, "RIGHT_TIMER", RIGHT_TIMER_STACKSIZE, NULL, RIGHT_TIMER_PRIORITY, NULL);
    xTaskCreate((void*)&TaskTimerDisplay, "DISPLAY_TIMER", DISPLAY_TIMER_STACKSIZE, NULL, DISPLAY_TIMER_PRIORITY, NULL);
    s4640878QueueTimerMsg = xQueueCreate(1, sizeof(struct dualTimerMsg));
    hardware_init();
}

// left timer controlling task
void TaskTimerLeft(void) {
    int time = 0;
    struct dualTimerMsg msg;
    msg.type = 'l';
    for (;;) {
        msg.timerValue = time;
        if (s4640878QueueTimerMsg != NULL) {
            // sends time through queue
            xQueueSendToFront(s4640878QueueTimerMsg, (void*)&msg, (portTickType)10);
        }
        if (s4640878SemaphoreJoystickZ != NULL) {
            // checks for button press
            if (xSemaphoreTake(s4640878SemaphoreJoystickZ, 10) == pdTRUE) {
                mode = 1 - mode;
            }
        }
        vTaskDelay(1000);
        if (mode) {
            // increments time
            time++;
        }
    }
}

// right timer controlling task
void TaskTimerRight(void) {
    int time = 0;
    struct dualTimerMsg msg;
    msg.type = 'r';
    for (;;) {
        msg.timerValue = time;
        if (s4640878QueueTimerMsg != NULL) {
            // sends time through queue
            xQueueSendToFront(s4640878QueueTimerMsg, (void*)&msg, (portTickType)10);
        }
        if (s4640878SemaphoreJoystickZ != NULL) {
            // checks for button press
            if (xSemaphoreTake(s4640878SemaphoreJoystickZ, 10) == pdTRUE) {
                mode = 1 - mode;
            }
        }
        vTaskDelay(1);
        if (mode) {
            // increments time
            time++;
        }
    }
}

// display timer contolling task
void TaskTimerDisplay(void) {
    struct dualTimerMsg msg;
    struct oledTextMsg oledMsg;
    int left2 = 0, left1 = 0, left0 = 0;
    int right1 = 0, right0 = 0;
    for (;;) {
        if (s4640878QueueTimerMsg != NULL) {
            // receives time from queue
            if (xQueueReceive(s4640878QueueTimerMsg, &msg, 10)) {
                // checks for left or right times
                if (msg.type == 'r') {
                    right1 = (msg.timerValue / 10) % 10;
                    right0 = msg.timerValue % 10;
                } else if (msg.type == 'l') {
                    left2 = (msg.timerValue / 100) % 10;
                    left1 = (msg.timerValue / 10) % 10;
                    left0 = msg.timerValue % 10;
                }
                // formats string to be displayed
                sprintf(oledMsg.displayText, "%d%d%d:%d%d", left2, left1, left0, right1, right0);
            }
        }
        // joystick x
        if (joystickXY.x > 3000) {
            oledMsg.startX = (SSD1306_WIDTH / 2) + 0;
        } else if (joystickXY.x < 1000) {
            oledMsg.startX = (SSD1306_WIDTH / 2) - 40;
        } else {
            oledMsg.startX = (SSD1306_WIDTH / 2) - 20;
        }
        // joystick y
        if (joystickXY.y > 3000) {
            oledMsg.startY = (SSD1306_HEIGHT / 2) - 12;
        } else if (joystickXY.y < 1000) {
            oledMsg.startY = (SSD1306_HEIGHT / 2) + 4;
        } else {
            oledMsg.startY = (SSD1306_HEIGHT / 2) - 4;
        }
        
        // sends time through queue
        if (s4640878QueueOledMsg != NULL) {
            xQueueSendToFront(s4640878QueueOledMsg, (void*)&oledMsg, (portTickType)10);
        }
    }
}

