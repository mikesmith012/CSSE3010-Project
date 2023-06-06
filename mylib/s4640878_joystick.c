 /** 
 **************************************************************
 * @file mylib/s4640878_joystick.c
 * @author Mike Smith - 46408789
 * @date 22022022
 * @brief mylib joystick library
 *        joystick pushbutton: board A0
 *        joystick x-value: board A1
 *        joystick y-value: board A2
 *        (board: nucleo-f401)
 * REFERENCE: csse3010_mylib_reg_joystick_pushbutton.pdf (task sheet)
 *            nucleo-f401re.pdf (pinout diagram for nucleo)
 *            stm32f429zi_reference.pdf (pg 281 - 286, register map for nucleo)
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4640878_reg_joystick_pb_init() - initialises joystick
 * s4640878_reg_joystick_pb_isr() - joystick interrupt service routine
 * s4640878_reg_joystick_press_get() - returns joystick press count
 * s4640878_reg_joystick_press_reset() - resets joystick press count
 * S4640878_REG_JOYSTICK_X_READ() - reads the joystick x-value
 * S4640878_REG_JOYSTICK_Y_READ() - reads the joystick y-value
 * s4640878_tsk_joystick_pb_init() - controlling task for joystick pushbutton
 * s4640878_tsk_joystick_init() - controlling task for joystick x and y values
 *************************************************************** 
 */

#include "s4640878_joystick.h"
#include "board.h"
#include "processor_hal.h"

// global variables
static int joystickPressCounter = 0;
static unsigned long prevTick = 0;
static unsigned short joystickButtonState = 0;

// internal function declarations
void s4640878TaskJoystickPushbutton(void);
void s4640878TaskJoystickXY(void);

// enables joystick pushbutton source
// enables gpio input and interrupt
void s4640878_reg_joystick_pb_init(void) {
    // enable gpio clk for port a
    __GPIOA_CLK_ENABLE();

    // set up port a pin 0
    GPIOA->MODER &= ~(0x03 << (0 * 2));

    GPIOA->OSPEEDR &= ~(0x03 << (0 * 2));
    GPIOA->OSPEEDR |= 0x02 << (0 * 2);      // fast speed

    GPIOA->OTYPER &= ~(0x03 << (0 * 2));

    GPIOA->PUPDR &= ~(0x03 << (0 * 2));     // no push pull

    // enable exti clock
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // trigger: port a pin 0 on exticr1
    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PC;
    SYSCFG->EXTICR[0] &= ~(0x000F);

    EXTI->RTSR |= EXTI_RTSR_TR0;            // enable rising edge
    EXTI->FTSR &= ~EXTI_FTSR_TR0;           // disable falling edge
    EXTI->IMR |= EXTI_IMR_IM0;              // enable external interrupt

    // priority: 10
    // enable interrupt callback
    HAL_NVIC_SetPriority(EXTI0_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

// joystick pushbutton interrupt service routine
// called by corresponding gpio exti isr
// includes pushbutton debouncing
void s4640878_reg_joystick_pb_isr(void) {
    if ((HAL_GetTick() - prevTick) > 10) {
        if (!joystickButtonState) {
            BaseType_t xHigherPriorityTaskWoken;
            xHigherPriorityTaskWoken = pdFALSE;
            if (s4640878SemaphoreJoystickZ != NULL) {
                xSemaphoreGiveFromISR(s4640878SemaphoreJoystickZ, &xHigherPriorityTaskWoken);
            }
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            joystickButtonState = 1;
            joystickPressCounter++;
        } else {
            joystickButtonState = 0;
        }
    }
    prevTick = HAL_GetTick();
}

// returns value of joystick pushbutton press counter
int s4640878_reg_joystick_press_get(void) {
    return joystickPressCounter;
}

// reset joystick event counter to 0
void s4640878_reg_joystick_press_reset(void) {
    joystickPressCounter = 0;
}

// GPIO EXTI interrupt service routine
// calls s4640878_reg_joystick_pb_isr()
void EXTI0_IRQHandler(void) {
    // clear flag
    NVIC_ClearPendingIRQ(EXTI0_IRQn);

    // pending register
    if ((EXTI->PR & EXTI_PR_PR0) == EXTI_PR_PR0) {
        EXTI->PR |= EXTI_PR_PR0;
        s4640878_reg_joystick_pb_isr();
    }
}

// initialises gpio pins for adc
void s4640878_reg_joystick_init(void) {
    // enable gpio clk for port a
    __GPIOA_CLK_ENABLE();

    // set up port a pins 1 and 4
    GPIOA->MODER |= (0x03 << (1 * 2)) | (0x03 << (4 * 2));          // analog input mode

    GPIOA->OSPEEDR &= ~((0x03 << (1 * 2)) | (0x03 << (4 * 2)));
    GPIOA->OSPEEDR |= (0x02 << (1 * 2)) | (0x02 << (4 * 2));        // fast speed

    GPIOA->OTYPER &= ~((0x03 << (1 * 2)) | (0x03 << (4 * 2)));

    GPIOA->PUPDR &= ~((0x03 << (1 * 2)) | (0x03 << (4 * 2)));       // no push pull

    // enables clock for adc 1 and 2
    __ADC1_CLK_ENABLE();

    AdcHandleX.Instance = (ADC_TypeDef *)(ADC1_BASE);               // adc1
    AdcHandleX.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV2;  // clock prescaler: div 2
    AdcHandleX.Init.Resolution = ADC_RESOLUTION12b;                 // data resolution: 12 bits
    AdcHandleX.Init.ScanConvMode = DISABLE;
    AdcHandleX.Init.ContinuousConvMode = DISABLE;
    AdcHandleX.Init.DiscontinuousConvMode = DISABLE;
    AdcHandleX.Init.NbrOfDiscConversion = 0;
    AdcHandleX.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    AdcHandleX.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1;
    AdcHandleX.Init.DataAlign = ADC_DATAALIGN_RIGHT;                // right align
    AdcHandleX.Init.NbrOfConversion = 1;
    AdcHandleX.Init.DMAContinuousRequests = DISABLE;
    AdcHandleX.Init.EOCSelection = DISABLE;

    AdcHandleY.Instance = (ADC_TypeDef *)(ADC1_BASE);               // adc1
    AdcHandleY.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV2;  // clock prescaler: div 2
    AdcHandleY.Init.Resolution = ADC_RESOLUTION12b;                 // data resolution: 12 bits
    AdcHandleY.Init.ScanConvMode = DISABLE;
    AdcHandleY.Init.ContinuousConvMode = DISABLE;
    AdcHandleY.Init.DiscontinuousConvMode = DISABLE;
    AdcHandleY.Init.NbrOfDiscConversion = 0;
    AdcHandleY.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    AdcHandleY.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1;
    AdcHandleY.Init.DataAlign = ADC_DATAALIGN_RIGHT;                // right align
    AdcHandleY.Init.NbrOfConversion = 1;
    AdcHandleY.Init.DMAContinuousRequests = DISABLE;
    AdcHandleY.Init.EOCSelection = DISABLE;

    // initialise adc
    HAL_ADC_Init(&AdcHandleX);
    HAL_ADC_Init(&AdcHandleY);

    AdcChanConfigX.Channel = ADC_CHANNEL_1;
    AdcChanConfigX.Rank = 1;
    AdcChanConfigX.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    AdcChanConfigX.Offset = 0;

    AdcChanConfigY.Channel = ADC_CHANNEL_4;
    AdcChanConfigY.Rank = 1;
    AdcChanConfigY.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    AdcChanConfigY.Offset = 0;
}

// generic function to read joystick x or y values
int s4640878_joystick_readxy(ADC_HandleTypeDef AdcHandler) {
    // start adc
    HAL_ADC_Start(&AdcHandler);

    // wait for conversion
    while (HAL_ADC_PollForConversion(&AdcHandler, 10) != HAL_OK);

    int adcValue = ADC1->DR;
    return adcValue;
}

// controlling task for joystick pushbutton
void s4640878TaskJoystickPushbutton(void) {
    // created binary semaphore for joystick pushbutton
    s4640878SemaphoreJoystickZ = xSemaphoreCreateBinary();
    
    // initilise the joystick bushbutton
    portDISABLE_INTERRUPTS();
    s4640878_reg_joystick_pb_init();
    portENABLE_INTERRUPTS();

    // always check for pushbutton interrupt
    for (;;) {
        vTaskDelay(1);
    }
}

// task init function for joystick pushbutton
void s4640878_tsk_joystick_pb_init(void) {
    xTaskCreate((void*)&s4640878TaskJoystickPushbutton, "JOYSTICK_PB", JOYSTICKPB_TASK_STACKSIZE, NULL, JOYSTICKPB_TASK_PRIORITY, NULL);
}

// controlling task for joystick x and y values
void s4640878TaskJoystickXY(void) {
    // initialises the joystick x and y adc
    portDISABLE_INTERRUPTS();
    s4640878_reg_joystick_init();
    portENABLE_INTERRUPTS();

    s4640878QueueJoystick = xQueueCreate(1, sizeof(joystickXY));
    for (;;) {
        // reads the joystick x and y values
        joystickXY.x = S4640878_REG_JOYSTICK_X_READ();
        joystickXY.y = S4640878_REG_JOYSTICK_Y_READ();

        // send joystick values through queue
        if (s4640878QueueJoystick != NULL) {
            xQueueSendToFront(s4640878QueueJoystick, (void*)&joystickXY, (portTickType)10);
        }
        vTaskDelay(10);
    }
}

// task init function for joystick x and y values
void s4640878_tsk_joystick_init(void) {
    xTaskCreate((void*)&s4640878TaskJoystickXY, "JOYSTICK_XY", JOYSTICKXY_TASK_STACKSIZE, NULL, JOYSTICKXY_TASK_PRIORITY, NULL);
}