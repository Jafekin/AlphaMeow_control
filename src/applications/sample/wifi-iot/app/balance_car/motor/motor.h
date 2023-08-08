/*
 * @Author        陈佳辉 1946847867@qq.com
 * @Date          2023-08-08 10:57:48
 * @LastEditTime  2023-08-08 21:27:42
 * @Description
 *
 */
#ifndef __MOTOR_H__
#define __MOTOR_H__

void MotorPwmInit(void);
void MotorPwmStop(void);
void MotorForward(uint32_t Duty);
void MotorBackward(uint32_t Duty);

#endif /* __MOTOR_H__ */