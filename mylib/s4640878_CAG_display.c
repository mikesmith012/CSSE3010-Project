/** 
 **************************************************************
 * @file mylib/s4640878_CAG_display.c
 * @author Mike Smith - 46408789
 * @date 06052022
 * @brief CAGDisplay (c file)
 *        (board: nucleo-f401)
 * REFERENCE: csse3010_project.pdf (spec sheet)
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4640878_tsk_CAG_display_init() - initialises CAG display
 *************************************************************** 
 */

#include "s4640878_CAG_display.h"
#include "s4640878_CAG_simulator.h"
#include "s4640878_oled.h"
#include "board.h"
#include "processor_hal.h"

// global variables
extern int cells[WIDTH][HEIGHT];

// internal function declarations
void s4640878TaskCAGDisplay(void);
void CAG_display_init(void);
void CAG_display_draw(void);

// controlling task for CAGDisplay
void s4640878TaskCAGDisplay(void) {
    portDISABLE_INTERRUPTS();
    s4640878_reg_oled_init();   // initialise the oled
    portENABLE_INTERRUPTS();

    CAG_display_init();         // receives semaphore when CAGSimulator is ready
    for(;;) {
        ssd1306_Fill(Black);    // clear screen
        CAG_display_draw();     // draws simulation
        ssd1306_UpdateScreen();
        vTaskDelay(100);        // delay 0.1s
    }
}

// task init function for CAGDisplay
void s4640878_tsk_CAG_display_init(void) {
    xTaskCreate((void*)&s4640878TaskCAGDisplay, "CAG_DISPLAY", CAG_DISPLAY_TASK_STACKSIZE, NULL, CAG_DISPLAY_TASK_PRIORITY, NULL);
}

// waits for CAGSimulator to set up
void CAG_display_init(void) {
    if (s4640878SemaphoreCAGSimulatorInit != NULL) {
        if (xSemaphoreTake(s4640878SemaphoreCAGSimulatorInit, 10) == pdTRUE) {
            ; // waits for semaphore from CAGSimulator 
        }
    }
}

// draws pixels of corresponding cells on the oled
void CAG_display_draw(void) {
    // loops through the cells array
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            // draws cells on the display if it is alive
            if (cells[x][y]) {
                ssd1306_DrawPixel(2*x, 2*y, SSD1306_WHITE);
                ssd1306_DrawPixel(2*x + 1, 2*y, SSD1306_WHITE);
                ssd1306_DrawPixel(2*x, 2*y + 1, SSD1306_WHITE);
                ssd1306_DrawPixel(2*x + 1, 2*y + 1, SSD1306_WHITE);
            }
        }
    }
}