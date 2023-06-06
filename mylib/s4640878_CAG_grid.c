/** 
 **************************************************************
 * @file mylib/s4640878_CAG_grid.c
 * @author Mike Smith - 46408789
 * @date 06052022
 * @brief CAGGrid (c file)
 *        (board: nucleo-f401)
 * REFERENCE: csse3010_project.pdf (spec sheet)
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4640878_tsk_CAG_grid_init() - initialises CAG grid
 *************************************************************** 
 */

#include "s4640878_CAG_grid.h"
#include "s4640878_CAG_simulator.h"
#include "s4640878_oled.h"
#include "s4640878_lta1000g.h"
#include "board.h"
#include "processor_hal.h"
#include "debug_log.h"

// internal variables
static unsigned long prevTick = 0;
static unsigned short buttonState = 0;

// internal function declarations
void s4640878TaskCAGGrid(void);
void CAG_grid_process_input(void);
void CAG_grid_disp_ledbar(void);
void CAG_grid_userbutton_init(void);
void CAG_grid_process_input(void);

// controlling task for CAGGrid
void s4640878TaskCAGGrid(void) {
    portDISABLE_INTERRUPTS();
    CAG_grid_userbutton_init();     // initilise the on-board user push-button
    BRD_debuguart_init();           // initilise the serial communication
    s4640878_reg_lta1000g_init();   // initilise the led array
    BRD_LEDInit();                  // initilise board led
    portENABLE_INTERRUPTS();

    for(;;) {
        // check current mode
        int gridMode = s4640878_lib_CAG_simulator_get_grid();
        if (gridMode) {
            BRD_LEDGreenOn();
            CAG_grid_process_input();   // process keyboard inputs from user
            CAG_grid_disp_ledbar();     // display current position on led bar
        } else {
            BRD_LEDGreenOff();
        }
        vTaskDelay(100);    // delay 0.1s
    }
}

// task init function for CAGGrid
void s4640878_tsk_CAG_grid_init(void) {
    xTaskCreate((void*)&s4640878TaskCAGGrid, "CAG_GRID", CAG_GRID_TASK_STACKSIZE, NULL, CAG_GRID_TASK_PRIORITY, NULL);
}

// processes inputs
void CAG_grid_process_input(void) {
    char CAGGridKey = '\0';
    EventBits_t uxBits;

    // checks for user inputs via uart
    // supports both upper-case and lower-case inputs
    if ((CAGGridKey = BRD_debuguart_getc()) != '\0') {
        switch(CAGGridKey) {
            case 'W':
            case 'w':
                uxBits = xEventGroupSetBits(GroupEventCAGGrid, MOVE_UP);
                break;
            case 'A':
            case 'a':
                uxBits = xEventGroupSetBits(GroupEventCAGGrid, MOVE_LEFT);
                break;
            case 'S':
            case 's':
                uxBits = xEventGroupSetBits(GroupEventCAGGrid, MOVE_DOWN);
                break;
            case 'D':
            case 'd':
                uxBits = xEventGroupSetBits(GroupEventCAGGrid, MOVE_RIGHT);
                break;
            case 'X':
            case 'x':
                uxBits = xEventGroupSetBits(GroupEventCAGGrid, SELECT_CELL);
                break;
            case 'Z':
            case 'z':
                uxBits = xEventGroupSetBits(GroupEventCAGGrid, UNSELECT_CELL);
                break;
            case 'P': ;
            case 'p': ;
                // checks if the game is currently paused
                int pause = s4640878_lib_CAG_simulator_get_pause();

                // if game is paused then resume, else pause
                if (pause) {
                    uxBits = xEventGroupSetBits(GroupEventCAGGrid, START_GAME);
                } else {
                    uxBits = xEventGroupSetBits(GroupEventCAGGrid, STOP_GAME);
                }
                break;
            case 'O':
            case 'o':
                uxBits = xEventGroupSetBits(GroupEventCAGGrid, MOVE_TO_ORIGIN);
                break;
            case 'C':
            case 'c':
                uxBits = xEventGroupSetBits(GroupEventCAGGrid, CLEAR_DISPLAY);
                break;
        }
    }
}

// displays current cell on ledbar
void CAG_grid_disp_ledbar(void) {
    int x = s4640878_lib_CAG_simulator_get_current_cell(X);     // current x position
    int y = s4640878_lib_CAG_simulator_get_current_cell(Y);     // current y position
    s4640878_reg_lta1000g_write(y | (x << 4));      // formats display (x->[9:4], y->[3:0])
}

// initialises user button
void CAG_grid_userbutton_init(void) {
    // Enable GPIO Clock
    __GPIOC_CLK_ENABLE();

    GPIOC->OSPEEDR |= (GPIO_SPEED_FAST << 13);      // fast speed.
    GPIOC->PUPDR &= ~(0x03 << (13 * 2));            // no push pull
    GPIOC->MODER &= ~(0x03 << (13 * 2));            // input mode

    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;           // Enable EXTI clock

    //select trigger source (port c, pin 13) on EXTICR4.
    SYSCFG->EXTICR[3] &= ~SYSCFG_EXTICR4_EXTI13;
    SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PC;

    EXTI->RTSR |= EXTI_RTSR_TR13;	// enable rising dedge
    EXTI->FTSR &= ~EXTI_FTSR_TR13;	// disable falling edge
    EXTI->IMR |= EXTI_IMR_IM13;		// enable external interrupt

    // enable priority of 10 and interrupt callback
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

// isr for on-board user push-button
void EXTI15_10_IRQHandler(void) {
    // checks for interrupt on board push-button
    NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
    if ((EXTI->PR & EXTI_PR_PR13) == EXTI_PR_PR13) {
        EXTI->PR |= EXTI_PR_PR13;
        portDISABLE_INTERRUPTS();

        // implements button debouncing
        if ((HAL_GetTick() - prevTick) > 10) {
            if (!buttonState) {
                s4640878_lib_CAG_simulator_toggle_grid();   // toggles grid mode
            } else {
                buttonState = 0;
            }
        }
        prevTick = HAL_GetTick();
        portENABLE_INTERRUPTS();
    }
}