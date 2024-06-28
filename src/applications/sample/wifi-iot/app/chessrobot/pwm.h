/*
 * @Author        陈佳辉 1946847867@qq.com
 * @Date          2024-06-26 17:00:44
 * @LastEditTime  2024-06-26 17:01:19
 * @Description
 *
 */
#ifndef __PWM_H__
#define __PWM_H__

void PwmInit(void);
void SetAngle(unsigned int duty, int gpio);
void RegressMiddle(int gpio);
void EngineTurnRight(int gpio);
void EngineTurnLeft(int gpio);

#endif /* __PWM_H__ */