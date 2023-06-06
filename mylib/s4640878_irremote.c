/** 
 **************************************************************
 * @file mylib/s4640878_irremote.c
 * @author Mike Smith - 46408789
 * @date 30032022
 * @brief mylib IR remote library (c file)
 *        IR receiver: board pin 4 (PC6)
 *        (board: nucleo-f401)
 * REFERENCE: csse3010_mylib_reg_irremote.pdf (spec sheet)
 *            nucleo-f401re.pdf (pinout diagram for nucleo-f401re)
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4640878_irremote_init();
 * s4640878_irremote_recv();
 * s4640878_irremote_readkey();
 *************************************************************** 
 */

#include "s4640878_irremote.h"
#include "board.h"
#include "processor_hal.h"
#include "debug_log.h"

// IR remote encodings
#define ZERO 0xFF6897
#define ONE 0xFF30CF
#define TWO 0xFF18E7
#define THREE 0xFF7A85
#define FOUR 0xFF10EF
#define FIVE 0xFF38C7
#define SIX 0xFF5AA5
#define SEVEN 0xFF42BD 
#define EIGHT 0xFF4AB5
#define NINE 0xFF52AD

// internal function declarations
void irremote_read_value(int recv);
int irremote_translate_data(int data);
unsigned char irremote_reverse_byte(unsigned char value);

// global variables (internal)
static int keyPressedFlag;          // set to 1 is a key is pressed, cleared by _readkey()
static char keyPressedValue;        // stored the latest key that has been pressed in ascii
static int receivedData;            // stores the bit pattern received from the ir remote
static uint64_t current = 0, previous = 0;

// initialises the IR receiver hardware
void s4640878_reg_irremote_init(void) {

    // initilise press value and clears press flag
    keyPressedFlag = 0;
    keyPressedValue = 0;

    // enable gpio clk for port c
    __GPIOC_CLK_ENABLE();

    // set up port c pin 6
    GPIOC->MODER &= ~(0x03 << (6 * 2));
    GPIOC->MODER |= 0x02 << (6 * 2);                // alternate function: push pull

    GPIOC->OSPEEDR &= ~(0x03 << (6 * 2));
    GPIOC->OSPEEDR |= 0x02 << (6 * 2);              // fast speed

    GPIOC->OTYPER &= ~(0x03 << (6 * 2));

    GPIOC->PUPDR &= ~(0x03 << (6 * 2));

    GPIOC->AFR[0] &= ~(0x0F << (6 * 4));
    GPIOC->AFR[0] |= GPIO_AF2_TIM3 << (6 * 4);	    // set alternative function

    // enable timer 3 clock
    __TIM3_CLK_ENABLE();

    // compute the prescaler value
    TIM3->PSC = ((SystemCoreClock / 2) / TIMER_RUNNING_FREQ) - 1;
    
    TIM3->CR1 &= ~TIM_CR1_DIR;                      // counting direction

    TIM3->CCMR1 &= ~(TIM_CCMR1_CC1S);               // clear channel 1
    TIM3->CCMR1 |= TIM_CCMR1_CC1S_0; 	            // active input
    
    TIM3->CCMR1 &= ~TIM_CCMR1_IC1F;                 // disable digital filtering
    TIM3->CCMR1 &= ~TIM_CCMR1_IC1PSC;               // clear filtering
	
    TIM3->CCER |= TIM_CCER_CC1P;                    // detect falling edge (ir receiver output is active low)
    TIM3->CCER |= TIM_CCER_CC1E;                    // enable capture for channel 1

    TIM3->DIER |= (TIM_DIER_CC1IE | TIM_DIER_UIE);  // enables interrupts

    HAL_NVIC_SetPriority(TIM3_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
    
    TIM3->CR1 |= TIM_CR1_CEN;                       // Enable the counter
}

// processes the received input
// called by timer input capture ISR
void s4640878_reg_irremote_recv(void) {
    // static uint64_t current = 0, previous = 0;
    static int8_t count = 0;

    // check for overflow
    if ((TIM3->SR & 0x01) == 0x01) {	 
        TIM3->SR &= ~0x01; 	
    }
    // check for input capture 
    if ((TIM3->SR & TIM_SR_CC1IF) == TIM_SR_CC1IF) { 
        
        // read from CCR1
        current = TIM3->CCR1;
        int diff = current - previous;
        
        // determine encoded bits
        if ((diff > 10 * MILLISECOND) || (diff < 0)) { 
            // start of message
            count = 0;
            receivedData = 0;
        } else if ((count > 0) && (count <= 32)) { 
            // generates received data
            if (diff >= 2 * MILLISECOND) {
                receivedData |= 1 << (32 - count);
            }
        }
        if (count >= 32) { 
            // end of message
            keyPressedFlag = 1;
            count = 0;
        }
        previous = current;
        count++;
	}
}

// check if key press has been detected
// returns 1 if key press has been detected, returns 0 otherwise
// stores the key ascii value into the keyPressedValue variable
// clears the keyPressedFlag variable
int s4640878_reg_irremote_readkey(char* value) {
    // checks that a key has been pressed
    if (keyPressedFlag) { 
        irremote_read_value(receivedData);
        irremote_read_value(irremote_translate_data(receivedData));
        
        *value = keyPressedValue;
        keyPressedFlag = 0;
        keyPressedValue = 0;
        return 1;
    } else {
        return 0;
    }
}

// timer input capture isr
void TIM3_IRQHandler(void) {
    s4640878_reg_irremote_recv();
}

// *** NOT TESTED WITH IR REMOTES USED IN INTERNAL OFFERINGS
// checks received bit pattern aaginst protocol encodings
// theoretically supports both encodings for internal and external ir remotes
void irremote_read_value(int recv) {
    switch (recv) {
        case ZERO:
        case 0xFD0CF3:
            keyPressedValue = 0;
            break;
        case ONE:
        case 0xFD10EF:
            keyPressedValue = 1;
            break;
        case TWO:
        case 0xFD11EE:
            keyPressedValue = 2;
            break;
        case THREE:
        case 0xFD12ED:
            keyPressedValue = 3;
            break;
        case FOUR:
        case 0xFD14EB:
            keyPressedValue = 4;
            break;
        case FIVE:
        case 0xFD15EA:
            keyPressedValue = 5;
            break;
        case SIX:
        case 0xFD16E9:
            keyPressedValue = 6;
            break;
        case SEVEN:
        case 0xFD18E7:
            keyPressedValue = 7;
            break;
        case EIGHT:
        case 0xFD19E6:
            keyPressedValue = 8;
            break;
        case NINE:
        case 0xFD1AE5:
            keyPressedValue = 9;
            break;
    } 
}

// translate data received from the ir remote
// calls irremote_reverse_byte()
int irremote_translate_data(int data) {
    unsigned char byte1 = irremote_reverse_byte((unsigned char)(data & 0xFF));
    unsigned char byte2 = irremote_reverse_byte((unsigned char)((data & 0xFF00) >> 8));
    data &= ~0xFFFF;
    data |= byte1 | byte2 << 8;
}

// reverses the bit order of a byte
// called by irremote_translate_data()
unsigned char irremote_reverse_byte(unsigned char value) {
    value = (value & (0b11110000)) >> 4 | (value & (0b00001111)) << 4;
    value = (value & (0b11001100)) >> 2 | (value & (0b00110011)) << 2;
    value = (value & (0b10101010)) >> 1 | (value & (0b01010101)) << 1;
    return value;
}