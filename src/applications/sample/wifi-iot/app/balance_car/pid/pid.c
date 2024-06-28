/*
 * @Author        陈佳辉 1946847867@qq.com
 * @Date          2023-08-09 01:08:39
 * @LastEditTime  2024-01-22 23:19:43
 * @Description
 *
 */

#include "pid.h"
//-2.74
// float Vertical_Kp = -2.245 * 0.6, Vertical_Kd = -80.91 * 0.6, Velocity_Kp = -0.55, Velocity_Ki = -0.0015, Med = -2.0;
float Vertical_Kp = -2.765, Vertical_Kd = -5.74, Velocity_Kp = 0, Velocity_Ki = 0, Med = -2.0;
int integral_limit = 1000;

// 直立环：Kp*e(k)+Kd*e(k)_D
int Vertical(float Angle, float Gyro_y)
{
    int pwm_out;
    static float Angle_last = 0;
    float Angle_speed = Angle - Angle_last;
    Angle_last = Angle;

    // pwm_out = Vertical_Kp * (Angle - Med) + Vertical_Kd * (Gyro_y - 0);
    pwm_out = Vertical_Kp * (Angle - Med) + Vertical_Kd * (Angle_speed - 0);
    return pwm_out;
}

// 速度环：Kp*e(k)+Ki*e(k)_S
int Velocity(int Target, int Encoder_Left, int Encoder_Right)
{
    static int pwm_out, Encoder_Err, Encoder_S = 0, Encoder_Err_last = 0;

    // 计算速度偏差
    Encoder_Err = Encoder_Left + Encoder_Right - Target;

    // 对速度进行低通滤波：low_out = (1-a) * e(k) + a * low_out_last
    Encoder_Err = 0.7 * Encoder_Err + 0.3 * Encoder_Err_last;
    Encoder_Err_last = Encoder_Err;

    // 积分位移
    Encoder_S += Encoder_Err;
    Encoder_S = Encoder_S > integral_limit ? integral_limit : (Encoder_S < -integral_limit ? -integral_limit : Encoder_S);

    pwm_out = Velocity_Kp * Encoder_Err + Velocity_Ki * Encoder_S;
    printf("Velocity: %d %d\n", Encoder_Err, Encoder_S);

    return pwm_out;
}

// 转向环：
int Turn(int gyro_z)
{
    return (-0.5) * gyro_z;
}
