/** 
 **************************************************************
 * @file mylib/s4640878_CAG_simulator.c
 * @author Mike Smith - 46408789
 * @date 06052022
 * @brief CAGSimulator (c file)
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

#include "s4640878_CAG_simulator.h"
#include "board.h"
#include "processor_hal.h"

// delay definitions
#define DELAY_1000MS 10
#define DELAY_1500MS 15
#define DELAY_2000MS 20
#define DELAY_5000MS 50
#define DELAY_10000MS 100

// buffers for 2D array of cells
int cells[WIDTH][HEIGHT];
int cellsBuf[WIDTH][HEIGHT];

// internal variables
static int gridMode;               // mode -> 1: grid or 0: mnemonic
static int currentCell[2];         // selected cell position
static int pause;                  // pause-game variable
static int delay;                  // sets update time
static TaskHandle_t xHandleCAGSimulator = NULL;     // CAGSimulator task handler

// internal function declarations for CAGSimulator
void s4640878TaskCAGSimulator(void);
void CAG_simulator_init(void);
void CAG_simulator_process(void);
void CAG_simulator_process_grid_event(void);
void CAG_simulator_process_simulator_event(void);
void CAG_simulator_process_queue(void);
void CAG_simulator_clear(void);
void CAG_simulator_move_origin(void);

// internal function declarations for lifeforms 
void draw_block(int x, int y);
void draw_beehive(int x, int y);
void draw_loaf(int x, int y);
void draw_blinker(int x, int y);
void draw_toad(int x, int y);
void draw_beacon(int x, int y);
void draw_glider(int x, int y);

// controlling task for CAGSimulator
void s4640878TaskCAGSimulator(void) {

    // created simulator initilisation semaphore
    // used to signal CAGDisplay that simulator is ready
    s4640878SemaphoreCAGSimulatorInit = xSemaphoreCreateBinary();

    CAG_simulator_init();       // initilises the simulator
    int count = 0;              // initilises count (used for variable update time)
    for(;;) {
        CAG_simulator_process_grid_event();         // checks grid event bits
        CAG_simulator_process_simulator_event();    // checks joystick event bits
        CAG_simulator_process_queue();              // checks the queue

        // checks if game is currently paused
        // only run simulation and and update count if game is not paused
        if (!pause) {
            // compare count to delay
            // if delay time is reached, then run simulation
            if (count >= delay) {
                CAG_simulator_process();    // implements game logic
                count = 0;                  // reset count after running simulation
            } else {
                count++;                // increments count
            }
        }
        vTaskDelay(80);          // delay
    }
}

// task init function for CAGSimulator
void s4640878_tsk_CAG_simulator_init(void) {
    // creates the CAGSimulator task if one does not already exist
    if (xHandleCAGSimulator == NULL) {
        xTaskCreate((void*)&s4640878TaskCAGSimulator, "CAG_SIMULATOR", CAG_SIMULATOR_TASK_STACKSIZE, NULL, CAG_SIMULATOR_TASK_PRIORITY, &xHandleCAGSimulator);
    }
}

// task deletion function for CAGSimulator
void s4640878_tsk_CAG_simulator_del(void) {
    // deletes the CAGSimulator task if one exists
    if (xHandleCAGSimulator != NULL) {
        vTaskDelete(xHandleCAGSimulator);
    }
    // set the task handle to NULL
    // signifies that task no longer exists
    xHandleCAGSimulator = NULL;
}

// initialize CAG simulation
void CAG_simulator_init(void) {
    CAG_simulator_clear();          // resets the simulator
    CAG_simulator_move_origin();    // default position: origin
    gridMode = 1;                   // default: grid mode
    pause = 1;                      // default: pause
    delay = 2000;                   // default delay: 2s

    // signals to CAGDisplay that simulator is ready
    if (s4640878SemaphoreCAGSimulatorInit != NULL) {
        xSemaphoreGive(s4640878SemaphoreCAGSimulatorInit);  // signals to oled task that simulator init is complete
    }
    GroupEventCAGSimulator = xEventGroupCreate();   // init simulator event group for joystick and mnemonic
    GroupEventCAGGrid = xEventGroupCreate();   // init grid event group for keyboard inputs
}

// processes grid event group bits
void CAG_simulator_process_grid_event(void) {
    EventBits_t uxBits = xEventGroupWaitBits(GroupEventCAGGrid, GRID_BITS, pdTRUE, pdFALSE, 1);
    if ((uxBits & MOVE_UP) != 0 && currentCell[Y] > 0) {
        currentCell[Y]--;       // move up
        uxBits = xEventGroupClearBits(GroupEventCAGGrid, MOVE_UP);
    }
    if ((uxBits & MOVE_DOWN) != 0 && currentCell[Y] < HEIGHT - 1) {
        currentCell[Y]++;       // move down
        uxBits = xEventGroupClearBits(GroupEventCAGGrid, MOVE_DOWN);
    }
    if ((uxBits & MOVE_LEFT) != 0 && currentCell[X] > 0) {
        currentCell[X]--;       // move left
        uxBits = xEventGroupClearBits(GroupEventCAGGrid, MOVE_LEFT);
    }
    if ((uxBits & MOVE_RIGHT) != 0 && currentCell[X] < WIDTH - 1) {
        currentCell[X]++;       // move right
        uxBits = xEventGroupClearBits(GroupEventCAGGrid, MOVE_RIGHT);
    }
    if ((uxBits & SELECT_CELL) != 0) {
        cells[currentCell[X]][currentCell[Y]] = ALIVE;      // selects cell
        uxBits = xEventGroupClearBits(GroupEventCAGGrid, SELECT_CELL);
    }
    if ((uxBits & UNSELECT_CELL) != 0) {
        cells[currentCell[X]][currentCell[Y]] = DEAD;       // unselects cell
        uxBits = xEventGroupClearBits(GroupEventCAGGrid, UNSELECT_CELL);
    }
    if ((uxBits & START_GAME) != 0) {
        pause = 0;      // starts game
        uxBits = xEventGroupClearBits(GroupEventCAGGrid, START_GAME);
    }
    if ((uxBits & STOP_GAME) != 0) {
        pause = 1;      // stops game
        uxBits = xEventGroupClearBits(GroupEventCAGGrid, STOP_GAME);
    }
    if ((uxBits & MOVE_TO_ORIGIN) != 0) {
        CAG_simulator_move_origin();    // moves cursor to origin (0, 0)
        uxBits = xEventGroupClearBits(GroupEventCAGGrid, MOVE_TO_ORIGIN);
    }
    if ((uxBits & CLEAR_DISPLAY) != 0) {
        CAG_simulator_clear();          // clears the display
        uxBits = xEventGroupClearBits(GroupEventCAGGrid, CLEAR_DISPLAY);
    }
}

// processes simulator event group bits
void CAG_simulator_process_simulator_event(void) {
    EventBits_t uxBits = xEventGroupWaitBits(GroupEventCAGSimulator, SIMULATOR_BITS, pdTRUE, pdFALSE, 1);
    if ((uxBits & CLEAR_GRID) != 0) {
        CAG_simulator_clear();          // clears the display
        uxBits = xEventGroupClearBits(GroupEventCAGSimulator, CLEAR_GRID);
    }
    if ((uxBits & START_SIMULATION) != 0) {
        pause = 0;      // starts game
        uxBits = xEventGroupClearBits(GroupEventCAGSimulator, START_SIMULATION);
    }
    if ((uxBits & STOP_SIMULATION) != 0) {
        pause = 1;      // stops game
        uxBits = xEventGroupClearBits(GroupEventCAGSimulator, STOP_SIMULATION);
    }
    if ((uxBits & UPDATE_1000MS) != 0) {
        delay = DELAY_1000MS;
        uxBits = xEventGroupClearBits(GroupEventCAGSimulator, UPDATE_1000MS);
    }
    if ((uxBits & UPDATE_1500MS) != 0) {
        delay = DELAY_1500MS;
        uxBits = xEventGroupClearBits(GroupEventCAGSimulator, UPDATE_1500MS);
    }
    if ((uxBits & UPDATE_2000MS) != 0) {
        delay = DELAY_2000MS;
        uxBits = xEventGroupClearBits(GroupEventCAGSimulator, UPDATE_2000MS);
    }
    if ((uxBits & UPDATE_5000MS) != 0) {
        delay = DELAY_5000MS;
        uxBits = xEventGroupClearBits(GroupEventCAGSimulator, UPDATE_5000MS);
    }
    if ((uxBits & UPDATE_10000MS) != 0) {
        delay = DELAY_10000MS;
        uxBits = xEventGroupClearBits(GroupEventCAGSimulator, UPDATE_10000MS);
    }
}

// processes the simulator lifeform queue sent from CAGMnemonic
void CAG_simulator_process_queue(void) {
    caMessage_t caMsg;
    if (s4640878QueueCAGMnemonic != NULL) {
        // checks the queue
        if (xQueueReceive(s4640878QueueCAGMnemonic, &caMsg, 10)) {
            // checks the first 4 bits
            // types: cell, still, oscillator or space ship
            switch ((caMsg.type & 0xF0) >> 4) {
                case CELL:
                    cells[caMsg.cell_x][caMsg.cell_y] = caMsg.type & 0xF;
                    break;
                case STILL:
                    // checks the last 4 bits for still lifeforms
                    // options: block, beehive or loaf
                    switch (caMsg.type & 0xF) {
                        case BLOCK:
                            draw_block(caMsg.cell_x, caMsg.cell_y);
                            break;
                        case BEEHIVE:
                            draw_beehive(caMsg.cell_x, caMsg.cell_y);
                            break;
                        case LOAF:
                            draw_loaf(caMsg.cell_x, caMsg.cell_y);
                            break;
                    }
                    break;
                case OSCILLATOR:
                    // checks the last 4 bits for oscillator lifeforms
                    // options: blinker, toad or beacon
                    switch (caMsg.type & 0xF) {
                        case BLINKER:
                            draw_blinker(caMsg.cell_x, caMsg.cell_y);
                            break;
                        case TOAD:
                            draw_toad(caMsg.cell_x, caMsg.cell_y);
                            break;
                        case BEACON:
                            draw_beacon(caMsg.cell_x, caMsg.cell_y);
                            break;
                    }
                    break;
                case SPACE_SHIP:
                    // checks the last 4 bits for space ship lifeforms
                    // options: glider
                    switch (caMsg.type & 0xF) {
                        case GLIDER:
                            draw_glider(caMsg.cell_x, caMsg.cell_y);
                            break;
                    }
                    break;
            }
        }
    }
}

// clears CAG display
void CAG_simulator_clear(void) {
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            cells[x][y] = DEAD;
            cellsBuf[x][y] = DEAD;
        }
    }
}

// moves to origin
void CAG_simulator_move_origin(void) {
    currentCell[X] = 0;
    currentCell[Y] = 0;
}

// returns current pause state: 0 or 1
int s4640878_lib_CAG_simulator_get_pause(void) {
    return pause;
}

// returns current cell position: x or y value
int s4640878_lib_CAG_simulator_get_current_cell(int index) {
    int position = 0;
    switch (index) {
        case X:
            position = currentCell[X];  // gets current x position
            break;
        case Y:
            position = currentCell[Y];  // gets current y position
            break;
    }
    return position;
}

// returns current gridMode state: 0 or 1
int s4640878_lib_CAG_simulator_get_grid(void) {
    return gridMode;
}

// toggles current gridMode state
void s4640878_lib_CAG_simulator_toggle_grid(void) {
    gridMode = 1 - gridMode;
}

/* code adapted from: 
 * processing.org/examples/gameoflife.html
 * A Processing Implementation of Game of Life by Joan Soler-Adillon
 */
// processes simulation
void CAG_simulator_process(void) {
    // make a copy of the cells array
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            cellsBuf[x][y] = cells[x][y];
        }
    }
    // loops through cells
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            // loops through adjacent cells
            int count = 0, value = DEAD;
            for (int dx = x - 1; dx <= x + 1; dx++) {
                for (int dy = y - 1; dy <= y + 1; dy++) {
                    // check boundary conditions
                    if ((dx >= 0) && (dx < WIDTH) && (dy >= 0) && (dy < HEIGHT)) {
                        // skip own cell
                        if (!((dx == x) && (dy == y))) {
                            // increment count if adjacent cell is alive
                            if (cellsBuf[dx][dy]) {
                                count++;
                                if (cellsBuf[dx][dy] > value) {
                                    value = cellsBuf[dx][dy];   // save the higher adjacent state value
                                }
                            }
                        }
                    }
                }
            }
            // check if cell is alive or dead, applies rules
            if (cellsBuf[x][y]) {
                if ((count < 2) || (count > 3)) {
                    cells[x][y] = DEAD;
                } else {
                    cells[x][y]++;      // increment state value
                }
            } else {
                if (count == 3) {
                    cells[x][y] = value;    // assigns the highest state value to new cell
                }
            }
        }
    }
}

// draws block lifeform
void draw_block(int x, int y) {
    if ((x + 1) < WIDTH && (y + 1) < HEIGHT) {
        // row 0
        cells[x + 0][y + 0] = ALIVE;
        cells[x + 1][y + 0] = ALIVE;
        //row 1
        cells[x + 0][y + 1] = ALIVE;
        cells[x + 1][y + 1] = ALIVE;
    }
}

// draws beehive lifeform
void draw_beehive(int x, int y) {
    if ((x + 3) < WIDTH && (y + 2) < HEIGHT) {
        // row 0
        cells[x + 1][y + 0] = ALIVE;
        cells[x + 2][y + 0] = ALIVE;
        // row 1
        cells[x + 0][y + 1] = ALIVE;
        cells[x + 3][y + 1] = ALIVE;
        // row 2
        cells[x + 1][y + 2] = ALIVE;
        cells[x + 2][y + 2] = ALIVE;
    }
}

// draws loaf lifeform
void draw_loaf(int x, int y) {
    if ((x + 3) < WIDTH && (y + 3) < HEIGHT) {
        // row 0
        cells[x + 1][y + 0] = ALIVE;
        cells[x + 2][y + 0] = ALIVE;
        // row 1
        cells[x + 0][y + 1] = ALIVE;
        cells[x + 3][y + 1] = ALIVE;
        // row 2
        cells[x + 1][y + 2] = ALIVE;
        cells[x + 3][y + 2] = ALIVE;
        // row 3
        cells[x + 2][y + 3] = ALIVE;
    }
}

// draws blinker lifeform
void draw_blinker(int x, int y) {
    if ((x + 2) < WIDTH && (y + 2) < HEIGHT) {
        // row 1
        cells[x + 0][y + 1] = ALIVE;
        cells[x + 1][y + 1] = ALIVE;
        cells[x + 2][y + 1] = ALIVE;
    }
}

// draws toad lifeform
void draw_toad(int x, int y) {
    if ((x + 3) < WIDTH && (y + 3) < HEIGHT) {
        // row 1
        cells[x + 1][y + 1] = ALIVE;
        cells[x + 2][y + 1] = ALIVE;
        cells[x + 3][y + 1] = ALIVE;
        // row 2
        cells[x + 0][y + 2] = ALIVE;
        cells[x + 1][y + 2] = ALIVE;
        cells[x + 2][y + 2] = ALIVE;
    }
}

// draws beacon lifeform
void draw_beacon(int x, int y) {
    if ((x + 3) < WIDTH && (y + 3) < HEIGHT) {
        // row 0
        cells[x + 0][y + 0] = ALIVE;
        cells[x + 1][y + 0] = ALIVE;
        // row 1
        cells[x + 0][y + 1] = ALIVE;
        // row 2
        cells[x + 3][y + 2] = ALIVE;
        // row 3
        cells[x + 2][y + 3] = ALIVE;
        cells[x + 3][y + 3] = ALIVE;
    }
}

// draws glider lifeform
void draw_glider(int x, int y) {
    if ((x + 2) < WIDTH && (y + 2) < HEIGHT) {
        // row 0
        cells[x + 0][y + 0] = ALIVE;
        // row 1
        cells[x + 1][y + 1] = ALIVE;
        cells[x + 2][y + 1] = ALIVE;
        // row 2
        cells[x + 0][y + 2] = ALIVE;
        cells[x + 1][y + 2] = ALIVE;
    }
}