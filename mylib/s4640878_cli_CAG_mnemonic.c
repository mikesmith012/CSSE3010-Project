/** 
 **************************************************************
 * @file mylib/s4640878_cli_CAG_mnemonic.c
 * @author Mike Smith - 46408789
 * @date 06052022
 * @brief CLI CAGMnemonic (c file)
*        (board: nucleo-f401)
* REFERENCE: csse3010_project.pdf (spec sheet)
*            csse3010_mylib_cli_peripheral_guide.pdf (mylib guide)
***************************************************************
* EXTERNAL FUNCTIONS 
***************************************************************
* 
*************************************************************** 
*/

#include "s4640878_cli_CAG_mnemonic.h"
#include "s4640878_CAG_joystick.h"
#include "board.h"
#include "processor_hal.h"
#include "task.h"
#include <stdlib.h>

#define BUF_LEN 50

// echo command
CLI_Command_Definition_t xEcho = {
    "echo",
    "echo <str>: Echo the input.\r\n\r\n",
    prvEchoCommand,
    1
};

// cell command
CLI_Command_Definition_t xCell = {
    "cell", 
    "cell <type><x><y>: Draw a cell. dead(0), alive(1), \r\n\r\n",
    prvCellCommand,
    3
};

// still command
CLI_Command_Definition_t xStill = {
    "still", 
    "still <type><x><y>: Draw a still life. block(0), beehive(1), loaf(2).\r\n\r\n",
    prvStillCommand,
    3
};

// osc command
CLI_Command_Definition_t xOsc = {
    "osc", 
    "osc <type><x><y>: Draw an oscillator. blinker(0), toad(1), beacon(2).\r\n\r\n",
    prvOscCommand,
    3
};

// glider command
CLI_Command_Definition_t xGlider = {
    "glider", 
    "glider <x><y>: Draw a glider at cell (x, y).\r\n\r\n",
    prvGliderCommand,
    2
};

// start command
CLI_Command_Definition_t xStart = {
    "start", 
    "start: Start simulator.\r\n\r\n",
    prvStartCommand,
    0
};

// stop command
CLI_Command_Definition_t xStop = {
    "stop", 
    "stop: Stop simulator.\r\n\r\n",
    prvStopCommand,
    0
};

// clear command
CLI_Command_Definition_t xClear = {
    "clear", 
    "clear: Clears display.\r\n\r\n",
    prvClearCommand,
    0
};

// del command
CLI_Command_Definition_t xDel = {
    "del", 
    "del: Deletes task: CAGSimulator(0), CAGJoystick(1).\r\n\r\n",
    prvDelCommand,
    1
};

// cre command
CLI_Command_Definition_t xCre = {
    "cre", 
    "cre: Creates task: CAGSimulator(0), CAGJoystick(1).\r\n\r\n",
    prvCreCommand,
    1
};

// system command
CLI_Command_Definition_t xSystem = {
    "system", 
    "system: Current system time since boot-up in ms.\r\n\r\n",
    prvSystemCommand,
    0
};

// usage command
CLI_Command_Definition_t xUsage = {
    "usage", 
    "usage: Current number of running tasks, respective task state and stack high water-mark usage.\r\n\r\n",
    prvUsageCommand,
    0
};

// init function - registers commands
void s4640878_cli_CAG_mnemonic_init(void) {
    FreeRTOS_CLIRegisterCommand(&xEcho);
    FreeRTOS_CLIRegisterCommand(&xCell);
    FreeRTOS_CLIRegisterCommand(&xStill);
    FreeRTOS_CLIRegisterCommand(&xOsc);
    FreeRTOS_CLIRegisterCommand(&xGlider);
    FreeRTOS_CLIRegisterCommand(&xStart);
    FreeRTOS_CLIRegisterCommand(&xStop);
    FreeRTOS_CLIRegisterCommand(&xClear);
    FreeRTOS_CLIRegisterCommand(&xDel);
    FreeRTOS_CLIRegisterCommand(&xCre);
    FreeRTOS_CLIRegisterCommand(&xSystem);
    FreeRTOS_CLIRegisterCommand(&xUsage);
}

// echo command
static BaseType_t prvEchoCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    long lParam_len;
    const char *cCmd_string;

    // get parameters from command string
    cCmd_string = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParam_len);

    // write command echo output string to write buffer
    xWriteBufferLen = sprintf((char*) pcWriteBuffer, "\n\r%s\n\r", cCmd_string);
    return pdFALSE;
}

// cell command
static BaseType_t prvCellCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    long lTypeLen, lXLen, lYLen;
    const char *cType, *cX, *cY;

    // get parameters from command string
    cType = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lTypeLen);
    cX = FreeRTOS_CLIGetParameter(pcCommandString, 2, &lXLen);
    cY = FreeRTOS_CLIGetParameter(pcCommandString, 3, &lYLen);

    // create cell
    int type = atoi(cType);
    caMsg.cell_x = atoi(cX);
    caMsg.cell_y = atoi(cY);

    // checks cell type: alive or dead
    switch (type) {
        case DEAD:
            caMsg.type = (CELL << 4) | DEAD;
            break;
        case ALIVE:
            caMsg.type = (CELL << 4) | ALIVE;
            break;
        default: 
            caMsg.type = 0;
    }

    // sends msg through queue
    xQueueSendToFront(s4640878QueueCAGMnemonic, (void*)&caMsg, (portTickType)10);
    xWriteBufferLen = sprintf((char*) pcWriteBuffer, "\n\r");   // clears the write buffer
    return pdFALSE;
}

// still command
static BaseType_t prvStillCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    long lTypeLen, lXLen, lYLen;
    const char *cType, *cX, *cY;

    // get parameters from command string
    cType = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lTypeLen);
    cX = FreeRTOS_CLIGetParameter(pcCommandString, 2, &lXLen);
    cY = FreeRTOS_CLIGetParameter(pcCommandString, 3, &lYLen);

    // create still lifeform
    int type = atoi(cType);
    caMsg.cell_x = atoi(cX);
    caMsg.cell_y = atoi(cY);

    // checks still lifeform type: block, beehive or loaf
    switch (type) {
        case BLOCK:
            caMsg.type = (STILL << 4) | BLOCK;
            break;
        case BEEHIVE:
            caMsg.type = (STILL << 4) | BEEHIVE;
            break;
        case LOAF:
            caMsg.type = (STILL << 4) | LOAF;
            break;
        default: 
            caMsg.type = 0;
    }

    // sends msg through queue
    xQueueSendToFront(s4640878QueueCAGMnemonic, (void*)&caMsg, (portTickType)10);
    xWriteBufferLen = sprintf((char*) pcWriteBuffer, "\n\r");   // clears the write buffer
    return pdFALSE;
}

// osc command
static BaseType_t prvOscCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    long lTypeLen, lXLen, lYLen;
    const char *cType, *cX, *cY;

    // get parameters from command string
    cType = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lTypeLen);
    cX = FreeRTOS_CLIGetParameter(pcCommandString, 2, &lXLen);
    cY = FreeRTOS_CLIGetParameter(pcCommandString, 3, &lYLen);

    // create osc lifeform
    int type = atoi(cType);
    caMsg.cell_x = atoi(cX);
    caMsg.cell_y = atoi(cY);

    // checks the oscillator lifeform type: blinker, toad or loaf
    switch (type) {
        case BLINKER:
            caMsg.type = (OSCILLATOR << 4) | BLINKER;
            break;
        case TOAD:
            caMsg.type = (OSCILLATOR << 4) | TOAD;
            break;
        case BEACON:
            caMsg.type = (OSCILLATOR << 4) | BEACON;
            break;
        default: 
            caMsg.type = 0;
    }

    // sends msg through queue
    xQueueSendToFront(s4640878QueueCAGMnemonic, (void*)&caMsg, (portTickType)10);
    xWriteBufferLen = sprintf((char*) pcWriteBuffer, "\n\r");   // clears the write buffer
    return pdFALSE;
}

// glider command
static BaseType_t prvGliderCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    long lXLen, lYLen;
    const char *cX, *cY;

    // get parameters from command string
    cX = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lXLen);
    cY = FreeRTOS_CLIGetParameter(pcCommandString, 2, &lYLen);

    // create glider lifeform
    caMsg.cell_x = atoi(cX);
    caMsg.cell_y = atoi(cY);
    caMsg.type = (SPACE_SHIP << 4) | GLIDER;

    // sends msg through queue
    xQueueSendToFront(s4640878QueueCAGMnemonic, (void*)&caMsg, (portTickType)10);
    xWriteBufferLen = sprintf((char*) pcWriteBuffer, "\n\r");   // clears the write buffer
    return pdFALSE;
}

// start command
static BaseType_t prvStartCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    // sets event group bit to start game
    EventBits_t uxBits = xEventGroupSetBits(GroupEventCAGSimulator, START_SIMULATION);
    xWriteBufferLen = sprintf((char*) pcWriteBuffer, "\n\r");   // clears the write buffer
    return pdFALSE;
}

// stop command
static BaseType_t prvStopCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    // sets event group bit to stop game
    EventBits_t uxBits = xEventGroupSetBits(GroupEventCAGSimulator, STOP_SIMULATION);
    xWriteBufferLen = sprintf((char*) pcWriteBuffer, "\n\r");   // clears the write buffer
    return pdFALSE;
}

// clear command
static BaseType_t prvClearCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    // sets the event bit to clear display
    EventBits_t uxBits = xEventGroupSetBits(GroupEventCAGSimulator, CLEAR_GRID);
    xWriteBufferLen = sprintf((char*) pcWriteBuffer, "\n\r");   // clears the write buffer
    return pdFALSE;
}

// del command
static BaseType_t prvDelCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    long lLen;
    const char* cTask;

    // get parameters from command string
    cTask = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lLen);

    // deletes specified task driver
    // checks the specified task: CAGSimulator or CAGJoystick
    int task = atoi(cTask);
    switch (task) {
        case CAG_SIMULATOR:
            s4640878_tsk_CAG_simulator_del();
            break;
        case CAG_JOYSTICK:
            s4640878_tsk_CAG_joystick_del();
            break;
    }
    xWriteBufferLen = sprintf((char*) pcWriteBuffer, "\n\r");   // clears the write buffer
    return pdFALSE;
}

// cre command
static BaseType_t prvCreCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    long lLen;
    const char* cTask;

    // get parameters from command string
    cTask = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lLen);

    // creates specified task driver
    // checks the specified task: CAGSimulator or CAGJoystick
    int task = atoi(cTask);
    switch (task) {
        case CAG_SIMULATOR:
            s4640878_tsk_CAG_simulator_init();
            break;
        case CAG_JOYSTICK:
            s4640878_tsk_CAG_joystick_init();
            break;
    }
    xWriteBufferLen = sprintf((char*) pcWriteBuffer, "\n\r"); // clears the write buffer
    return pdFALSE;
}

// system command
static BaseType_t prvSystemCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    // gets the tick count since system boot-up
    // ticks are incremented 1000 times / sec
    int time = xTaskGetTickCount();

    // displays the time elapsed since boot-up in ms
    xWriteBufferLen = sprintf((char*) pcWriteBuffer, "%d ms\n\r\n\r", time);
    return pdFALSE;
}

// usage command
static BaseType_t prvUsageCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    // gets information regarding CAGSimulator
    char pcWriteCAGSimulator[BUF_LEN];
    TaskStatus_t xStatusCAGSimulator;
    TaskHandle_t xHandleCAGSimulator = xTaskGetHandle("CAG_SIMULATOR");
    vTaskGetInfo(xHandleCAGSimulator, &xStatusCAGSimulator, pdTRUE, eInvalid);
    int xLenCAGSimulator = sprintf(pcWriteCAGSimulator, "%s\t%d\t%d\r\n", 
            xStatusCAGSimulator.pcTaskName, xStatusCAGSimulator.eCurrentState, xStatusCAGSimulator.usStackHighWaterMark);
    
    // gets information regarding CAGDisplay
    char pcWriteCAGDisplay[BUF_LEN];
    TaskStatus_t xStatusCAGDisplay;
    TaskHandle_t xHandleCAGDisplay = xTaskGetHandle("CAG_DISPLAY");
    vTaskGetInfo(xHandleCAGDisplay, &xStatusCAGDisplay, pdTRUE, eInvalid);
    int xLenCAGDisplay = sprintf(pcWriteCAGDisplay, "%s\t%d\t%d\r\n", 
            xStatusCAGDisplay.pcTaskName, xStatusCAGDisplay.eCurrentState, xStatusCAGDisplay.usStackHighWaterMark);
    
    // gets information regarding CAGGrid
    char pcWriteCAGGrid[BUF_LEN];
    TaskStatus_t xStatusCAGGrid;
    TaskHandle_t xHandleCAGGrid = xTaskGetHandle("CAG_GRID");
    vTaskGetInfo(xHandleCAGGrid, &xStatusCAGGrid, pdTRUE, eInvalid);
    int xLenCAGGrid = sprintf(pcWriteCAGGrid, "%s\t%d\t%d\r\n", 
            xStatusCAGGrid.pcTaskName, xStatusCAGGrid.eCurrentState, xStatusCAGGrid.usStackHighWaterMark);
    
    // gets information regarding CAGJoystick
    char pcWriteCAGJoystick[BUF_LEN];
    TaskStatus_t xStatusCAGJoystick;
    TaskHandle_t xHandleCAGJoystick = xTaskGetHandle("CAG_JOYSTICK");
    vTaskGetInfo(xHandleCAGJoystick, &xStatusCAGJoystick, pdTRUE, eInvalid);
    int xLenCAGJoystick = sprintf(pcWriteCAGJoystick, "%s\t%d\t%d\r\n", 
            xStatusCAGJoystick.pcTaskName, xStatusCAGJoystick.eCurrentState, xStatusCAGJoystick.usStackHighWaterMark);
    
    // gets information regarding CAGJoystick
    char pcWriteCAGMnemonic[BUF_LEN];
    TaskStatus_t xStatusCAGMnemonic;
    TaskHandle_t xHandleCAGMnemonic = xTaskGetHandle("CLI");
    vTaskGetInfo(xHandleCAGMnemonic, &xStatusCAGMnemonic, pdTRUE, eInvalid);
    int xLenCAGMnemonic = sprintf(pcWriteCAGMnemonic, "%s\t%d\t%d\r\n", 
            xStatusCAGMnemonic.pcTaskName, xStatusCAGMnemonic.eCurrentState, xStatusCAGMnemonic.usStackHighWaterMark);
    
    // gets information regarding JoystickXY
    char pcWriteJoystickXY[BUF_LEN];
    TaskStatus_t xStatusJoystickXY;
    TaskHandle_t xHandleJoystickXY = xTaskGetHandle("JOYSTICK_XY");
    vTaskGetInfo(xHandleJoystickXY, &xStatusJoystickXY, pdTRUE, eInvalid);
    int xLenJoystickXY = sprintf(pcWriteJoystickXY, "%s\t%d\t%d\r\n", 
            xStatusJoystickXY.pcTaskName, xStatusJoystickXY.eCurrentState, xStatusJoystickXY.usStackHighWaterMark);
    
    // gets information regarding JoystickPB
    char pcWriteJoystickPB[BUF_LEN];
    TaskStatus_t xStatusJoystickPB;
    TaskHandle_t xHandleJoystickPB = xTaskGetHandle("JOYSTICK_PB");
    vTaskGetInfo(xHandleJoystickPB, &xStatusJoystickPB, pdTRUE, eInvalid);
    int xLenJoystickPB = sprintf(pcWriteJoystickPB, "%s\t%d\t%d\r\n", 
            xStatusJoystickPB.pcTaskName, xStatusJoystickPB.eCurrentState, xStatusJoystickPB.usStackHighWaterMark);

    // outputs information to cli write buffer
    xWriteBufferLen = sprintf((char*) pcWriteBuffer, "\r\nTask\t\tState\tStack High Water Mark\n\r%s%s%s%s%s%s%s\n\r",
            pcWriteCAGSimulator, pcWriteCAGDisplay, pcWriteCAGGrid, pcWriteCAGMnemonic, 
            pcWriteCAGJoystick, pcWriteJoystickXY, pcWriteJoystickPB);
    return pdFALSE;
}