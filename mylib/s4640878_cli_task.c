/** 
 **************************************************************
 * @file mylib/s4640878_cli_task.c
 * @author Mike Smith - 46408789
 * @date 06052022
 * @brief CLI Task (c file)
 *        (board: nucleo-f401)
 * REFERENCE: csse3010_project.pdf (spec sheet)
 *            csse3010_mylib_cli_peripheral_guide.pdf (mylib guide)
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4640878_cli_init() - initialises CLI
 *************************************************************** 
 */

#include "s4640878_cli_task.h"
#include "s4640878_CAG_simulator.h"
#include "board.h"
#include "processor_hal.h"
#include "debug_log.h"
#include <string.h>

// internal function declarations
void s4640878TaskCLI(void);

// controlling task for the CLI task
void s4640878TaskCLI(void) {
    int i;
    char cRxedChar;
    char cInputString[100];
    int InputIndex = 0;
    char *pcOutputString;
    BaseType_t xReturned;

    // initialise CAGMnemonic Queue
    s4640878QueueCAGMnemonic = xQueueCreate(1, sizeof(caMsg));

    // initialise pointer to CLI output buffer
    memset(cInputString, 0, sizeof(cInputString));
    pcOutputString = FreeRTOS_CLIGetOutputBuffer();
    for(;;) {
        int gridMode = s4640878_lib_CAG_simulator_get_grid();
        if (!gridMode) {
            if ((cRxedChar = debug_getc()) != '\0') {
                debug_putc(cRxedChar);
                if (cRxedChar == '\r') {
                    debug_putc('\n');
                    debug_flush();
                    cInputString[InputIndex] = '\0';
                    xReturned = pdTRUE;
                    // rocess command input string
                    while (xReturned != pdFALSE) {
                        // returns pdFALSE when all strings have been returned
                        xReturned = FreeRTOS_CLIProcessCommand( cInputString, pcOutputString, configCOMMAND_INT_MAX_OUTPUT_SIZE );
                        portENTER_CRITICAL();
                        for (i = 0; i < (int) strlen(pcOutputString); i++) {
                            debug_putc(*(pcOutputString + i));
                        }
                        portEXIT_CRITICAL();
                        vTaskDelay(1);      // delay
                    }
                    memset(cInputString, 0, sizeof(cInputString));
                    InputIndex = 0;
                } else {
                    debug_flush();
                    if( cRxedChar == '\r' ) {
                        // ignore the character
                    } else if( cRxedChar == '\b' ) {
                        // backspace
                        if( InputIndex > 0 ) {
                            InputIndex--;
                            cInputString[InputIndex] = '\0';
                        }
                    } else {
                        if( InputIndex < 20 ) {
                            cInputString[InputIndex] = cRxedChar;
                            InputIndex++;
                        }
                    }
                }
            }
            vTaskDelay(1);
        } else {
            vTaskDelay(1000);
        }
    }
}

// task init function for the CLI task
void s4640878_cli_init(void) {
    xTaskCreate((void*)&s4640878TaskCLI, "CAG_MNEMONIC", CLI_TASK_STACKSIZE, NULL, CLI_TASK_PRIORITY, NULL);
}