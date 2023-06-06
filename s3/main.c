/** 
 **************************************************************
 * @file s3/main.c
 * @author Mike Smith - 46408789
 * @date 30032022
 * @brief main file for stage 3 (board: nucleo-f401)
 * REFERENCE: csse3010_stage3.pdf (task sheet)
 ***************************************************************
 * COMMENTED_OUT_CODE: FOR THE F401, PC6 (INPUT CAPTURE) AND PB4 (PANTILT TILT) BOTH USE TIM3 CH1 (stm32f401re.pdf pg 46 & 47)
 * SETTING TIM3->ARR TO PWM_PULSE_WIDTH_TICKS (200) INTERFERES WITH THE COUNTER FOR THE TIMER INPUT CAPTURE.
 * ENABLING THE OUTPUT PRELOAD BIT FOR CH1 ALSO INTERFERES WITH THE INPUT CAPTURE.
 * AS OF NOW, THE IRREMOTE LIBRARY DOES NOT WORK WITH THE PANTILT LIBRARY.
 * THIS IS NOT A PROBLEM FOR STAGE 3. HOWEVER, IT MAY BE A PROBLEM FOR FUTURE STAGES.
 */

#include <s4640878_joystick.h>
#include <s4640878_lta1000g.h>
#include <s4640878_pantilt.h>
#include <s4640878_hamming.h>
#include <s4640878_irremote.h>
#include "board.h"
#include "processor_hal.h"
#include "debug_log.h"

// state machine definitions
#define S0 0    // idle
#define S1 1    // encode
#define S2 2    // decode

// global variables
static unsigned short encoded = 0, decoded = 0, displayMSB = 1;

// function declarations
void hardware_init(void);
int process_fsm(int current, uint8_t* value, int* count);
int get_input(int current, uint8_t* value, int* count, int* prevChar);
int change_state(int next);
void convert_dec(char recv, uint8_t* value, int* count);
void convert_hex(char recv, uint8_t* value, int* count);
void convert_ir(char recv, uint8_t* value, int* count);

// the main function for stage 3
int main(void) {

    hardware_init();
    uint32_t prevTick = 0;
    int count = 0;
    uint8_t value;
    
    // default state: idle
    int currentState = S0;

    while (1) {
        currentState = process_fsm(currentState, &value, &count);
        
        /*** FOR INTEGRATION WITH THE PANTILT LIBRARY ***/
        // read from CCR1, check for accidental interrupts
        /* uint64_t current = TIM3->CCR1;
        uint64_t previous = s4640878_reg_irremote_get_previous();
        int diff = current - previous;
        if (((diff > 50 * MILLISECOND) || (diff < 0)) && (TIM3->CCR1 != PWM_PULSE_WIDTH_TICKS)) { 
            TIM3->CNT = 0;   // reset timer counter
            TIM3->ARR = PWM_PULSE_WIDTH_TICKS;
            TIM3->CCMR1 |= TIM_CCMR1_OC1PE;
            // debug_log("reset timer\r\n"); // DEBUG
        } */
    }
    return 0;
}

// initialises hardware
void hardware_init(void) {
    HAL_Init();
    BRD_debuguart_init();               // console
    s4640878_reg_lta1000g_init();       // led array
    s4640878_reg_joystick_pb_init();    // joystick pushbutton
    s4640878_reg_joystick_init();       // joystick x and y
    s4640878_reg_irremote_init();       // IR remote
    // s4640878_reg_pantilt_init();         // pan and tilt *** DOES NOT WORK WITH IR INPUT CAPTURE ***
}

// processes fsm
// returns the next state
int process_fsm(int current, uint8_t* value, int* count) {
    int next = current;
    char recvChar = '\0';
    static int prevChar = 0;
    next = get_input(current, value, count, &prevChar);
    switch (current) {
        // idle state (do nothing)
        case S0:
            break;
        // encode state
        case S1:
            // encodes value
            if (*count >= 2) {
                encoded = s4640878_lib_hamming_byte_encode((unsigned char)(*value));
                debug_log("<%X><%X><%X><%X>\n\r", (encoded & 0xF000) >> 12, (encoded & 0xF00) >> 8, (encoded & 0xF0) >> 4, encoded & 0xF);
                displayMSB = 1;
            }
            // check joystick pushbutton press
            if (s4640878_reg_joystick_press_get()) {
                displayMSB = 1 - displayMSB;
                s4640878_reg_joystick_press_reset();
            }
            // displays msb or lsb of encoded value
            if (displayMSB) {
                s4640878_reg_lta1000g_write((unsigned short)(encoded & 0xFF00) >> 8);
            } else {
                s4640878_reg_lta1000g_write((unsigned short)(encoded & 0x00FF));
            }
            break;
        // decode state
        case S2:
            // decodes value
            if (*count >= 2) {
                decoded = s4640878_lib_hamming_byte_decode((unsigned char)(*value));
                debug_log("<%X><%X>", decoded, *value & 0x0F);
            }
            // checks for bit errors
            uint8_t bitErrors = s4640878_lib_hamming_get_bit_error();
            if (*count >= 2) {
                debug_log("<%X>\n\r", bitErrors);
            }
            // displays decoded value and bit error status to led array
            if (bitErrors == 1) {
                s4640878_reg_lta1000g_write((unsigned short)(decoded | 1 << 8));
            } else if (bitErrors == 2) {
                s4640878_reg_lta1000g_write((unsigned short)(decoded | 1 << 9));
            } else {
                s4640878_reg_lta1000g_write((unsigned short)decoded);
            }
            break;
    }
    return next;
}

// gets input from console or ir remote
// returns the next state
int get_input(int current, uint8_t* value, int* count, int* prevChar) {
    int next = current;
    char recvChar = '\0';
    // checks for input from console
    if ((recvChar = BRD_debuguart_getc()) != '\0') { 
        debug_log("Input: %c\n\r", recvChar);
        if ((int)recvChar == *prevChar && (int)recvChar == 68) { 
            // checks for <D> <D>
            *prevChar = 0;
            *count = 0;
            next = change_state(S2);    // change to decode state
        } else if ((int)recvChar == *prevChar && (int)recvChar == 69) { 
            // checks for <E> <E>
            *prevChar = 0;
            *count = 0;
            next = change_state(S1);    // change to encode state
        } else if ((int)recvChar == *prevChar && (int)recvChar == 70) { 
            // checks for <F> <F>
            *prevChar = 0;
            next = change_state(S0);    // change to idle state
        } else if ((int)recvChar < 58 && (int)recvChar > 47) { 
            // checks for decimal numbers
            convert_dec(recvChar, value, count);
        } else if ((int)recvChar < 71 && (int)recvChar > 64) { 
            // checks for hexadecimal numbers
            convert_hex(recvChar, value, count);
        } else {
            *count = 0;
        }
        *prevChar = (int)recvChar;
    } else if (s4640878_reg_irremote_readkey(&recvChar)) { 
        // checks for ir remote button presses
        debug_log("Input: %d\n\r", recvChar);
        convert_ir(recvChar, value, count);
        *prevChar = (int)recvChar;
    } else if (*count >= 2) {
        *count = 0;
    }
    return next;
}

// function to change states
// returns the next state
int change_state(int next) {
    switch (next) {
        case S0:
            debug_log("MODE: IDLE\n\r");
            break;
        case S1:
            debug_log("MODE: ENCODE\n\r");
            break;
        case S2:
            debug_log("MODE: DECODE\n\r");
            break;
    }
    return next;
}

// converts to dec from ascii
void convert_dec(char recv, uint8_t* value, int* count) {
    if (*count == 0) {
        *value = ((int)recv - 48) << 4;
    } else if (*count == 1) {
        *value |= ((int)recv - 48);
    }
    (*count)++;
}

// converts to hex from ascii
void convert_hex(char recv, uint8_t* value, int* count) {
    if (*count == 0) {
        *value = ((int)recv - 55) << 4;
    } else if (*count == 1) {
        *value |= ((int)recv - 55);
    }
    (*count)++;
}

// converts ir remote inputs
void convert_ir(char recv, uint8_t* value, int* count) {
    if (*count == 0) {
        *value = (int)recv << 4;
    } else if (*count == 1) {
        *value |= (int)recv;
    }
    (*count)++;
}