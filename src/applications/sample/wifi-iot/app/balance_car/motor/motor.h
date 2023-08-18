/*
 * @Author        陈佳辉 1946847867@qq.com
 * @Date          2023-08-08 10:57:48
 * @LastEditTime  2023-08-18 14:42:18
 * @Description
 *
 */
#ifndef __MOTOR_H__
#define __MOTOR_H__
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>

void MotorPwmInit(void);
void MotorPwmStop(void);
void MotorForward(uint16_t Pwm);
void MotorBackward(uint16_t Pwm);
void MotorControl(int LeftPwm, int RightPwm);

#endif /* __MOTOR_H__ */