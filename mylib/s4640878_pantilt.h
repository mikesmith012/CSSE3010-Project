/** 
 **************************************************************
 * @file mylib/s4640878_pantilt.h
 * @author Mike Smith - 46408789
 * @date 15032022
 * @brief mylib pantilt library (header file)
 * REFERENCE: csse3010_mylib_reg_pantilt.pdf (spec sheet)
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * S4640878_REG_PANTILT_PAN_WRITE() - writes angle for pan servo
 * S4640878_REG_PANTILT_PAN_READ() - reads angle of pan servo
 * S4640878_REG_PANTILT_TILT_WRITE() - writes angle for tilt servo
 * S4640878_REG_PANTILT_TILT_READ() - reads angle of tilt servo
 *************************************************************** 
 */

#ifndef S4640878_PANTILT_H_
#define S4640878_PANTILT_H_

#define S4640878_REG_PANTILT_PAN_WRITE(angle) s4640878_pantilt_angle_write(0, angle)
#define S4640878_REG_PANTILT_PAN_READ() s4640878_pantilt_angle_read(0)
#define S4640878_REG_PANTILT_TILT_WRITE(angle) s4640878_pantilt_angle_write(1, angle)
#define S4640878_REG_PANTILT_TILT_READ() s4640878_pantilt_angle_read(1)

#define S4640878_REG_PANTILT_PAN_90_CAL_OFFSET 0
#define S4640878_REG_PANTILT_TILT_90_CAL_OFFSET -20

#define TIMER_RUNNING_FREQ 5000         // frequency
#define PWM_PULSE_WIDTH_SECONDS 0.04
#define PWM_PULSE_WIDTH_TICKS 200     // period in timer ticks -> PWM_PULSE_WIDTH_SECONDS / (1 / TIMER_RUNNING_FREQ)

void s4640878_reg_pantilt_init(void);
void s4640878_pantilt_angle_write(int type, int angle);
int s4640878_pantilt_angle_read(int type);

#endif