/** 
 **************************************************************
 * @file mylib/s4640878_pantilt.c
 * @author Mike Smith - 46408789
 * @date 15032022
 * @brief mylib pantilt library
 *        pan: board pin D4
 *        tilt: board pin D5
 *        (board: nucleo-f401)
 * REFERENCE: csse3010_mylib_reg_pantilt.pdf (spec sheet)
 *            nucleo-f401re.pdf (pinout diagram for nucleo)
 *            stm32f429zi_reference.pdf (pg 281 - 286, register map for nucleo)
 *            stm32f401re.pdf (pg 45, alternative functions table)
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * S4640878_REG_PANTILT_PAN_WRITE() - writes angle for pan servo
 * S4640878_REG_PANTILT_PAN_READ() - reads angle of pan servo
 * S4640878_REG_PANTILT_TILT_WRITE() - writes angle for tilt servo
 * S4640878_REG_PANTILT_TILT_READ() - reads angle of tilt servo
 *************************************************************** 
 */

#include "s4640878_pantilt.h"
#include "board.h"
#include "processor_hal.h"

#define PWM_PERCENT2TICKS_DUTYCYCLE(value) (float)(value * PWM_PULSE_WIDTH_TICKS / 100) // duty cycle in timer ticks
#define PWM_TICKS2PERCENT_DUTYCYCLE(value) (float)(value * 100 / PWM_PULSE_WIDTH_TICKS) // duty cycle in percentage 
#define PWM_DUTYCYCLE_CHANGE_PAN(value) TIM4->CCR2 = (uint16_t)value    // change pwm pan dutycyle
#define PWM_DUTYCYCLE_CHANGE_TILT(value) TIM4->CCR1 = (uint16_t)value    // change pwm tilt dutycyle

// initialise servo
void s4640878_reg_pantilt_init(void) {

    // enable gpio clk for port b
    __GPIOB_CLK_ENABLE();

    // set up port a pins 4 and 5
    GPIOB->MODER &= ~((0x03 << (4 * 2)) | (0x03 << (5 * 2)));
    GPIOB->MODER |= (0x01 << (4 * 2)) | (0x01 << (5 * 2));      // push pull

    GPIOB->OSPEEDR &= ~((0x03 << (4 * 2)) | (0x03 << (5 * 2)));
    GPIOB->OSPEEDR |= (0x02 << (4 * 2)) | (0x02 << (5 * 2));    // fast speed

    GPIOB->OTYPER &= ~((0x03 << (4 * 2)) | (0x03 << (5 * 2)));

    GPIOB->PUPDR &= ~((0x03 << (4 * 2)) | (0x03 << (5 * 2)));  
    GPIOB->PUPDR |= (0x01 << (4 * 2)) | (0x01 << (5 * 2));   // pull down

    // enable timer 4 clock
    __TIM4_CLK_ENABLE();

    // compute the prescaler value
    TIM4->PSC = ((SystemCoreClock / 2) / TIMER_RUNNING_FREQ) - 1;

    // counting direction
    TIM4->CR1 &= ~TIM_CR1_DIR; 

    TIM4->ARR = PWM_PULSE_WIDTH_TICKS; 
    
    TIM4->CCR1 = PWM_PERCENT2TICKS_DUTYCYCLE(0);
    TIM4->CCR2 = PWM_PERCENT2TICKS_DUTYCYCLE(0);

    TIM4->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC2M);      // clear channel 1 and 2 
    // TIM4->CCMR1 |= (0x6 << 4 | 0x6 << 12);   	            // enable pwm mode 1, upcounting on Channel 2
    

    TIM4->CR1  |= (TIM_CR1_ARPE); 	                        // Set Auto-Reload Preload Enable 

    TIM4->DIER |= (TIM_DIER_CC1IE | TIM_DIER_CC2IE | TIM_DIER_UIE);  // enables interrupts

    HAL_NVIC_SetPriority(TIM4_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);

    TIM4->CR1 |= TIM_CR1_CEN;   // Enable the counter

    // set default position for pantilt module to be in the centre
    S4640878_REG_PANTILT_PAN_WRITE(S4640878_REG_PANTILT_PAN_90_CAL_OFFSET);
    S4640878_REG_PANTILT_TILT_WRITE(S4640878_REG_PANTILT_TILT_90_CAL_OFFSET);
}

// generic function for writing angle
// type: 0 for pan and 1 for tilt
// angle: from -90 to +90
void s4640878_pantilt_angle_write(int type, int angle) {
    float value = ((float)(angle + 90) / 20) + 2.75;
    if (type) {
        PWM_DUTYCYCLE_CHANGE_TILT(PWM_PERCENT2TICKS_DUTYCYCLE(value));
    } else {
        PWM_DUTYCYCLE_CHANGE_PAN(PWM_PERCENT2TICKS_DUTYCYCLE(value));
    }
}

// generic function for reading angle
// type: 0 for pan and 1 for tilt
// angle: from -90 to 90 degrees
int s4640878_pantilt_angle_read(int type) {
    int value = 0;
    if (type) {
        value = TIM4->CCR1;
    } else {
        value = TIM4->CCR2;
    }
    float angle = 20.0 * ((float)PWM_TICKS2PERCENT_DUTYCYCLE(value) - 2.75);
    return (int)angle - 90;
}

// interrupt service routine for pantilt pwm
void TIM4_IRQHandler(void) {
    // sets pan and tilt pin on timer 4 overflow (every 20 ms)
    if ((TIM4->SR & (1 << 0)) == (1 << 0)) {
        TIM4->SR &= ~(1 << 0);
        GPIOB->ODR |= (1 << 4) | (1 << 5);
    }
    // clears tilt pin on timer 4 channel 1 compare match
    if ((TIM4->SR & (1 << 1)) == (1 << 1)) {	 
        TIM4->SR &= ~(1 << 1);
        GPIOB->ODR &= ~(1 << 4);
    } 
    // clears pan pin on timer 4 channel 2 compare match
    if ((TIM4->SR & (1 << 2)) == (1 << 2)) {	 
        TIM4->SR &= ~(1 << 2);
        GPIOB->ODR &= ~(1 << 5);
    }
}