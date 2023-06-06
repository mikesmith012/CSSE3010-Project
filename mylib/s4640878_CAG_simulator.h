/** 
 **************************************************************
 * @file mylib/s4640878_CAG_simulator.h
 * @author Mike Smith - 46408789
 * @date 06052022
 * @brief CAGSimulator (header file)
 *        (board: nucleo-f401)
 * REFERENCE: csse3010_project.pdf (spec sheet)
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4640878_tsk_CAG_simulator_init() - initialises CAG simulator
 * s4640878_tsk_CAG_simulator_del() - deletes CAG simulator
 * s4640878_lib_CAG_simulator_get_pause() - gets current pause state
 * s4640878_lib_CAG_simulator_get_current_cell() - gets current cell position
 * s4640878_lib_CAG_simulator_get_grid() - gets current grid mode
 * s4640878_lib_CAG_simulator_toggle_grid() - toggles current grid mode
 *************************************************************** 
 */

#ifndef S4640878_CAG_SIMULATOR_H_
#define S4640878_CAG_SIMULATOR_H_

#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"
#include "queue.h"
#include <string.h>

// CAGSimulator task definitions
#define CAG_SIMULATOR_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define CAG_SIMULATOR_TASK_STACKSIZE (configMINIMAL_STACK_SIZE * 2)

// display definitions
#define WIDTH 64
#define HEIGHT 16

// CAG grid event-group bits
#define MOVE_UP (1 << 0)
#define MOVE_DOWN (1 << 1)
#define MOVE_LEFT (1 << 2)
#define MOVE_RIGHT (1 << 3)
#define SELECT_CELL (1 << 4)
#define UNSELECT_CELL (1 << 5)
#define START_GAME (1 << 6)
#define STOP_GAME (1 << 7)
#define MOVE_TO_ORIGIN (1 << 8)
#define CLEAR_DISPLAY (1 << 9)
#define GRID_BITS (0x3FF)

// CAG simulator event-group bits
#define CLEAR_GRID (1 << 0)
#define START_SIMULATION (1 << 1)
#define STOP_SIMULATION (1 << 2)
#define UPDATE_1000MS (1 << 3)
#define UPDATE_1500MS (1 << 4)
#define UPDATE_2000MS (1 << 5)
#define UPDATE_5000MS (1 << 6)
#define UPDATE_10000MS (1 << 7)
#define SIMULATOR_BITS (0xFF)

// lifeform definitions
#define CELL 1
#define STILL 2
#define OSCILLATOR 3
#define SPACE_SHIP 4

// cell definitions
#define DEAD 0
#define ALIVE 1

// still life definitions
#define BLOCK 0
#define BEEHIVE 1
#define LOAF 2

// oscillator definitions
#define BLINKER 0
#define TOAD 1
#define BEACON 2

// space ship definitions
#define GLIDER 0

// axis definitions
#define X 0
#define Y 1

// task definitions
#define CAG_SIMULATOR 0
#define CAG_JOYSTICK 1

// simulation event-group
EventGroupHandle_t GroupEventCAGSimulator;

// grid event-group
EventGroupHandle_t GroupEventCAGGrid;

// cellular automation message
typedef struct caMessage {
    int type;       // cell or lifeform
    int cell_x;     // x position
    int cell_y;     // y position
} caMessage_t;

// CAGMnemonic queue
QueueHandle_t s4640878QueueCAGMnemonic;

// semaphores
SemaphoreHandle_t s4640878SemaphoreCAGSimulatorInit;

// external function declarations
void s4640878_tsk_CAG_simulator_init(void);
void s4640878_tsk_CAG_simulator_del(void);
int s4640878_lib_CAG_simulator_get_pause(void);
int s4640878_lib_CAG_simulator_get_current_cell(int);
int s4640878_lib_CAG_simulator_get_grid(void);
void s4640878_lib_CAG_simulator_toggle_grid(void);

#endif