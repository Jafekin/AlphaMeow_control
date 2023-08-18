/*
 * @Author        陈佳辉 1946847867@qq.com
 * @Date          2023-08-09 01:08:39
 * @LastEditTime  2023-08-18 20:44:31
 * @Description
 *
 */

#include "pid.h"

// -2.25 0.00 0.00 0.00 -2.00 0
float Vertical_Kp = -2.24, Vertical_Kd = 0, Velocity_Kp = 0, Velocity_Ki = 0, Med = -2.0;

// 直立环：Kp*e(k)+Kd*e(k)_D
int Vertical(float Angle, float Gyro_y)
{
    int pwm_out;
    static float Angle_last = 0;
    float Angle_speed = (Angle - Angle_last) / 10;
    Angle_last = Angle;

    // pwm_out = Vertical_Kp * (Angle - Med) + Vertical_Kd * (Gyro_y - 0);
    pwm_out = Vertical_Kp * (Angle - Med) + Vertical_Kd * (Angle_speed - 0);
    return pwm_out;
}

// 速度环：Kp*e(k)+Ki*e(k)_S
int Velocity(int Target, int Encoder_Left, int Encoder_Right)
{
    static int pwm_out, Encoder_Err, Encoder_S, Encoder_RC, Encoder_RC_last = 0;

    // 计算速度偏差
    Encoder_Err = Encoder_Left + Encoder_Right - Target;

    // 对速度进行低通滤波：low_out = (1-a) * e(k) + a * low_out_last
    Encoder_RC = 0.7 * Encoder_Err + 0.3 * Encoder_RC_last;
    Encoder_RC_last = Encoder_RC;

    // 积分位移
    Encoder_S += Encoder_RC_last;
    Encoder_S = Encoder_S > 15000 ? 15000 : (Encoder_S < -15000 ? -15000 : Encoder_S);

    pwm_out = Velocity_Kp * Encoder_RC_last + Velocity_Ki * Encoder_S;

    return pwm_out;
}

// 转向环：
int Turn(int gyro_z)
{
    return (-0.5) * gyro_z;
}
