 /** 
 **************************************************************
 * @file mylib/s4640878_lta1000g.c
 * @author Mike Smith - 46408789
 * @date 22022022
 * @brief mylib lta1000g led array library
 * REFERENCE: csse3010_mylib_reg_lta1000g.pdf (task sheet)
 *            nucleo-f401re.pdf (pinout diagram for nucleo)
 *            lta1000g_lightbar.pdf (pinout diagram for led array)
 *            stm32f429zi_reference.pdf (pg 281 - 286, register map for nucleo)
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4640878_reg_lta1000g_init() - initialise led array
 * s4640878_reg_lta1000g_write() - writes low or high to each led in array 
 *************************************************************** 
 */

#include "s4640878_lta1000g.h"
#include "board.h"
#include "processor_hal.h"

// initialise led bar gpio pins as outputs
void s4640878_reg_lta1000g_init(void) {
    // enable gpio clk for port a, b & c
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();

    // set port a pin 11 & 12 as output
    GPIOA->MODER &= ~((0x03 << (11 * 2)) | (0x03 << (12 * 2)));     
    GPIOA->MODER |= (0x01 << (11 * 2)) | (0x01 << (12 * 2));        // push pull
    
    GPIOA->OSPEEDR &= ~((0x03 << (11 * 2)) | (0x03 << (12 * 2)));
    GPIOA->OSPEEDR |= (0x02 << (11 * 2)) | (0x02 << (12 * 2));      // fast speed

    GPIOA->OTYPER &= ~((0x03 << (11 * 2)) | (0x03 << (12 * 2)));

    GPIOA->PUPDR &= ~((0x03 << (11 * 2)) | (0x03 << (12 * 2)));
    GPIOA->PUPDR |= (0x01 << (11 * 2)) | (0x01 << (12 * 2));        // pull down

    // set port b pin 1, 2, 12, 13, 14 & 15 as output
    GPIOB->MODER &= ~((0x03 << (1 * 2)) | (0x03 << (2 * 2)) | (0x03 << (12 * 2)) | 
            (0x03 << (13 * 2)) | (0x03 << (14 * 2)) | (0x03 << (15 * 2)));
    GPIOB->MODER |= (0x01 << (1 * 2)) | (0x01 << (2 * 2)) | (0x01 << (12 * 2)) | 
            (0x01 << (13 * 2)) | (0x01 << (14 * 2)) | (0x01 << (15 * 2));      // push pull

    GPIOB->OSPEEDR &= ~((0x03 << (1 * 2)) | (0x03 << (2 * 2)) | (0x03 << (12 * 2)) | 
            (0x03 << (13 * 2)) | (0x03 << (14 * 2)) | (0x03 << (15 * 2)));
    GPIOB->OSPEEDR |= (0x02 << (1 * 2)) | (0x02 << (2 * 2)) | (0x02 << (12 * 2)) | 
            (0x02 << (13 * 2)) | (0x02 << (14 * 2)) | (0x02 << (15 * 2));    // fast speed

    GPIOB->OTYPER &= ~((0x03 << (1 * 2)) | (0x03 << (2 * 2)) | (0x03 << (12 * 2)) | 
            (0x03 << (13 * 2)) | (0x03 << (14 * 2)) | (0x03 << (15 * 2)));

    GPIOB->PUPDR &= ~((0x03 << (1 * 2)) | (0x03 << (2 * 2)) | (0x03 << (12 * 2)) | 
            (0x03 << (13 * 2)) | (0x03 << (14 * 2)) | (0x03 << (15 * 2)));
    GPIOB->PUPDR |= (0x01 << (1 * 2)) | (0x01 << (2 * 2)) | (0x01 << (12 * 2)) | 
            (0x01 << (13 * 2)) | (0x01 << (14 * 2)) | (0x01 << (15 * 2));      // pull down

    // set port c pin 4 & 5 as output
    GPIOC->MODER &= ~((0x03 << (4 * 2)) | (0x03 << (5 * 2)));     
    GPIOC->MODER |= (0x01 << (4 * 2)) | (0x01 << (5 * 2));        // push pull
    
    GPIOC->OSPEEDR &= ~((0x03 << (4 * 2)) | (0x03 << (5 * 2)));
    GPIOC->OSPEEDR |= (0x02 << (4 * 2)) | (0x02 << (5 * 2));      // fast speed

    GPIOC->OTYPER &= ~((0x03 << (4 * 2)) | (0x03 << (5 * 2)));

    GPIOC->PUPDR &= ~((0x03 << (4 * 2)) | (0x03 << (5 * 2)));
    GPIOC->PUPDR |= (0x01 << (4 * 2)) | (0x01 << (5 * 2));        // pull down

}

// internal funtion
// set value (0 or 1) for corresponding led bar seg gpio pin
void lta1000g_seg_set(int segment, unsigned char segmentValue) {
    switch (segment) {
        case 0:
            if (segmentValue) {
                GPIOC->ODR |= (1 << 4);
            } else {
                GPIOC->ODR &= ~(1 << 4);
            }
            break;
        case 1:
            if (segmentValue) {
                GPIOB->ODR |= (1 << 13);
            } else {
                GPIOB->ODR &= ~(1 << 13);
            }
            break;
        case 2:
            if (segmentValue) {
                GPIOB->ODR |= (1 << 14);
            } else {
                GPIOB->ODR &= ~(1 << 14);
            }
            break;
        case 3:
            if (segmentValue) {
                GPIOB->ODR |= (1 << 15);
            } else {
                GPIOB->ODR &= ~(1 << 15);
            }
            break;
        case 4:
            if (segmentValue) {
                GPIOB->ODR |= (1 << 1);
            } else {
                GPIOB->ODR &= ~(1 << 1);
            }
            break;
        case 5:
            if (segmentValue) {
                GPIOB->ODR |= (1 << 2);
            } else {
                GPIOB->ODR &= ~(1 << 2);
            }
            break;
        case 6:
            if (segmentValue) {
                GPIOB->ODR |= (1 << 12);
            } else {
                GPIOB->ODR &= ~(1 << 12);
            }
            break;
        case 7:
            if (segmentValue) {
                GPIOA->ODR |= (1 << 11);
            } else {
                GPIOA->ODR &= ~(1 << 11);
            }
            break;
        case 8:
            if (segmentValue) {
                GPIOA->ODR |= (1 << 12);
            } else {
                GPIOA->ODR &= ~(1 << 12);
            }
            break;
        case 9:
            if (segmentValue) {
                GPIOC->ODR |= (1 << 5);
            } else {
                GPIOC->ODR &= ~(1 << 5);
            }
            break;
    }
}

// calls lta1000g_seg_set()
// uses bit iteration
// writes led bar segment high or low
void s4640878_reg_lta1000g_write(unsigned short value) {
    for (int i = 0; i < 10; i++) {
        if ((value >> i) & 0x01) {
            lta1000g_seg_set(i, 1);
        } else {
            lta1000g_seg_set(i, 0);
        }
    }
}