/*
 * @Author        陈佳辉 1946847867@qq.com
 * @Date          2023-08-09 01:08:31
 * @LastEditTime  2023-08-09 10:50:47
 * @Description
 *
 */
#ifndef __PID_H__
#define __PID_H__

int Vertical(float Angle, float Gyro_y);
int Velocity(int Target, int Encoder_Left, int Encoder_Right);
int Turn(int gyro_z);

#endif /* __PID_H__ */