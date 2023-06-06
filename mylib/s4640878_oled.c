/** 
 **************************************************************
 * @file mylib/s4640878_oled.c
 * @author Mike Smith - 46408789
 * @date 12042022
 * @brief oled library (c file)
 *        sda: board pin D14 (PB9)
 *        scl: board pin D15 (PB8)
 *        (board: nucleo-f401)
 * REFERENCE: csse3010_mylib_reg_oled.pdf (spec sheet)
 *            nucleo-f401re.pdf (pinout diagram for nucleo-f401re)
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4640878_reg_oled_init() - initialise the oled
 * s4640878_tsk_oled_init() - created controlling task for the oled
 *************************************************************** 
 */

#include "s4640878_oled.h"
#include "board.h"
#include "processor_hal.h"

// i2c definitions
#define I2C_SDA 9
#define I2C_SCL 8
#define I2C_GPIO GPIOB
#define I2C_GPIO_AF GPIO_AF4_I2C1
#define I2C_GPIO_CLK() __GPIOB_CLK_ENABLE()
#define I2C_CLK_SPEED 100000

// internal function declarations
void s4640878TaskOled(void);
void oled_draw_boundary_box(void);

// initialise oled with i2c interface
void s4640878_reg_oled_init(void) {
    uint32_t pclk1;
    uint32_t freqrange;
    
    // enable clock for i2c gpio (port b)
    I2C_GPIO_CLK();

    // set alternative function: i2c
    MODIFY_REG(I2C_GPIO->AFR[1], ((0x0F) << ((I2C_SCL - 8) * 4)) | ((0x0F) << ((I2C_SDA - 8) * 4)), ((I2C_GPIO_AF << ((I2C_SCL - 8) * 4)) | (I2C_GPIO_AF << ((I2C_SDA - 8)) * 4)));

    // set alternative function push pull
    MODIFY_REG(I2C_GPIO->MODER, ((0x03 << (I2C_SCL * 2)) | (0x03 << (I2C_SDA * 2))), ((GPIO_MODE_AF_OD << (I2C_SCL * 2)) | (GPIO_MODE_AF_OD << (I2C_SDA * 2))));

    // set low speed
    SET_BIT(I2C_GPIO->OSPEEDR, (GPIO_SPEED_LOW << I2C_SCL) | (GPIO_SPEED_LOW << I2C_SDA));

    // set for push pull output
    SET_BIT(I2C_GPIO->OTYPER, ((0x01 << I2C_SCL) | (0x01 << I2C_SDA)));

    // set for no push pull, pullup
    MODIFY_REG(I2C_GPIO->PUPDR, (0x03 << (I2C_SCL * 2)) | (0x03 << (I2C_SDA * 2)), (GPIO_PULLUP << (I2C_SCL * 2)) | (GPIO_PULLUP << (I2C_SDA * 2)));

    // enable i2c clock
    __I2C1_CLK_ENABLE();

    // disable selected i2c peripheral
	CLEAR_BIT(I2C1->CR1, I2C_CR1_PE);

    pclk1 = HAL_RCC_GetPCLK1Freq();			// pclk1 freq
    freqrange = I2C_FREQRANGE(pclk1);		// freq range 

    // configure freq range
    MODIFY_REG(I2C1->CR2, I2C_CR2_FREQ, freqrange);

    // configure rise time
    MODIFY_REG(I2C1->TRISE, I2C_TRISE_TRISE, I2C_RISE_TIME(freqrange, I2C_CLK_SPEED));
    
    // configure speed
    MODIFY_REG(I2C1->CCR, (I2C_CCR_FS | I2C_CCR_DUTY | I2C_CCR_CCR), I2C_SPEED(pclk1, I2C_CLK_SPEED, I2C_DUTYCYCLE_2));
    
    // configure general call and no stretch mode
    MODIFY_REG(I2C1->CR1, (I2C_CR1_ENGC | I2C_CR1_NOSTRETCH), (I2C_GENERALCALL_DISABLE| I2C_NOSTRETCH_DISABLE));
    
    // configure own address 1 and addressing mode
    MODIFY_REG(I2C1->OAR1, (I2C_OAR1_ADDMODE | I2C_OAR1_ADD8_9 | I2C_OAR1_ADD1_7 | I2C_OAR1_ADD0), I2C_ADDRESSINGMODE_7BIT);
    
    // configure dual mode and own address 2
    MODIFY_REG(I2C1->OAR2, (I2C_OAR2_ENDUAL | I2C_OAR2_ADD2), I2C_DUALADDRESS_DISABLE);

    // enable selected i2c peripheral
    SET_BIT(I2C1->CR1, I2C_CR1_PE);
    
    // ssd1306
    ssd1306_Init();                 
}

// controlling task for oled
void s4640878TaskOLED(void) {
    // initialise the oled
    portDISABLE_INTERRUPTS();
    s4640878_reg_oled_init();
    portENABLE_INTERRUPTS();

    s4640878QueueOledMsg = xQueueCreate(1, sizeof(struct oledTextMsg));
    struct oledTextMsg msg;
    
    for (;;) {
        //Clear Screen
        ssd1306_Fill(Black);

        // draws boundary box, helps with text positioning
        // oled_draw_boundary_box();
        
        // receives time from queue
        if (s4640878QueueOledMsg != NULL) {
            if (xQueueReceive(s4640878QueueOledMsg, &msg, 10)) {
                ssd1306_SetCursor(msg.startX, msg.startY);
                ssd1306_WriteString(msg.displayText, Font_7x10, SSD1306_WHITE);
                ssd1306_UpdateScreen();
            }
        }

        // delay
        vTaskDelay(10);
    }
}

// task init function for oled
void s4640878_tsk_oled_init(void) {
    xTaskCreate((void*)&s4640878TaskOLED, "OLED", OLED_TASK_STACKSIZE, NULL, OLED_TASK_PRIORITY, NULL);
}

// draws boundary box on the oled dislpay
void oled_draw_boundary_box(void) {
    // horizontal lines
    for (int i = 0; i < SSD1306_WIDTH; i++) {
        ssd1306_DrawPixel(i, 0, SSD1306_WHITE);					// top line
        ssd1306_DrawPixel(i, SSD1306_HEIGHT-1, SSD1306_WHITE);	// bottom line
    }
    // vertical lines
    for (int i = 0; i < SSD1306_HEIGHT; i++) {
        ssd1306_DrawPixel(0, i, SSD1306_WHITE);					// left line
        ssd1306_DrawPixel(SSD1306_WIDTH-1, i, SSD1306_WHITE);	// right line
    }
}