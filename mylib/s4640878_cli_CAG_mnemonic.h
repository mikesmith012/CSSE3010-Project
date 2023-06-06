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

#ifndef S4640878_CLI_CAG_MNEMONIC_H_
#define S4640878_CLI_CAG_MNEMONIC_H_

#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include "s4640878_CAG_simulator.h"

// init function to register commands
void s4640878_cli_CAG_mnemonic_init(void);

// callback function declarations
static BaseType_t prvEchoCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCellCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvStillCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvOscCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvGliderCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvStartCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvStopCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvClearCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvDelCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCreCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvSystemCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvUsageCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

// caMessage typedef struct
caMessage_t caMsg;

#endif