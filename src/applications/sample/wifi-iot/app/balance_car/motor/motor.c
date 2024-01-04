/*
 * @Author        陈佳辉 1946847867@qq.com
 * @Date          2023-08-08 10:57:35
 * @LastEditTime  2023-09-13 23:01:03
 * @Description
 *
 */
#include "iot_gpio.h"
#include "iot_gpio_ex.h"
#include "iot_pwm.h"
#include "motor.h"

#define IOT_FREQ 100000
#define MOTOR_LEFT_ENA_PWM_PORT (0)
#define MOTOR_LEFT_ENA_PWM_PIN (9)
#define MOTOR_LEFT_ENA_PWM_PIN_FUNC (IOT_GPIO_FUNC_GPIO_9_PWM0_OUT)

#define MOTOR_RIGHT_ENB_PWM_PORT (2)
#define MOTOR_RIGHT_ENB_PWM_PIN (11)
#define MOTOR_RIGHT_ENB_PWM_PIN_FUNC (IOT_GPIO_FUNC_GPIO_11_PWM2_OUT)

#define MOTOR_LEFT_IN1_PIN (8)
#define MOTOR_LEFT_IN1_PIN_FUNC (IOT_GPIO_FUNC_GPIO_8_GPIO)
#define MOTOR_LEFT_IN2_PIN (14)
#define MOTOR_LEFT_IN2_PIN_FUNC (IOT_GPIO_FUNC_GPIO_14_GPIO)
#define MOTOR_RIGHT_IN3_PIN (2)
#define MOTOR_RIGHT_IN3_PIN_FUNC (IOT_GPIO_FUNC_GPIO_2_GPIO)
#define MOTOR_RIGHT_IN4_PIN (10)
#define MOTOR_RIGHT_IN4_PIN_FUNC (IOT_GPIO_FUNC_GPIO_10_GPIO)

void MotorPwmInit(void)
{
    IoTGpioSetFunc(MOTOR_LEFT_ENA_PWM_PIN, MOTOR_LEFT_ENA_PWM_PIN_FUNC);
    IoTGpioSetDir(MOTOR_LEFT_ENA_PWM_PIN, IOT_GPIO_DIR_OUT);
    IoTPwmInit(MOTOR_LEFT_ENA_PWM_PORT);

    IoTGpioSetFunc(MOTOR_RIGHT_ENB_PWM_PIN, MOTOR_RIGHT_ENB_PWM_PIN_FUNC);
    IoTGpioSetDir(MOTOR_RIGHT_ENB_PWM_PIN, IOT_GPIO_DIR_OUT);
    IoTPwmInit(MOTOR_RIGHT_ENB_PWM_PORT);

    IoTGpioSetFunc(MOTOR_LEFT_IN1_PIN, MOTOR_LEFT_IN1_PIN_FUNC);
    IoTGpioSetDir(MOTOR_LEFT_IN1_PIN, IOT_GPIO_DIR_OUT);
    IoTGpioSetOutputVal(MOTOR_LEFT_IN1_PIN, IOT_GPIO_VALUE0);

    IoTGpioSetFunc(MOTOR_LEFT_IN2_PIN, MOTOR_LEFT_IN2_PIN_FUNC);
    IoTGpioSetDir(MOTOR_LEFT_IN2_PIN, IOT_GPIO_DIR_OUT);
    IoTGpioSetOutputVal(MOTOR_LEFT_IN2_PIN, IOT_GPIO_VALUE0);

    IoTGpioSetFunc(MOTOR_RIGHT_IN3_PIN, MOTOR_RIGHT_IN3_PIN_FUNC);
    IoTGpioSetDir(MOTOR_RIGHT_IN3_PIN, IOT_GPIO_DIR_OUT);
    IoTGpioSetOutputVal(MOTOR_RIGHT_IN3_PIN, IOT_GPIO_VALUE0);

    IoTGpioSetFunc(MOTOR_RIGHT_IN4_PIN, MOTOR_RIGHT_IN4_PIN_FUNC);
    IoTGpioSetDir(MOTOR_RIGHT_IN4_PIN, IOT_GPIO_DIR_OUT);
    IoTGpioSetOutputVal(MOTOR_RIGHT_IN4_PIN, IOT_GPIO_VALUE0);
}

void MotorPwmStop(void)
{
    IoTPwmStop(MOTOR_LEFT_ENA_PWM_PORT);
    IoTPwmStop(MOTOR_RIGHT_ENB_PWM_PORT);
}

void MotorForward(uint16_t Pwm)
{
    MotorPwmStop();
    // 左轮正转
    IoTGpioSetOutputVal(MOTOR_LEFT_IN1_PIN, IOT_GPIO_VALUE0);
    IoTGpioSetOutputVal(MOTOR_LEFT_IN2_PIN, IOT_GPIO_VALUE1);
    IoTPwmStart(MOTOR_LEFT_ENA_PWM_PORT, Pwm, IOT_FREQ);
    // 右轮正转
    IoTGpioSetOutputVal(MOTOR_RIGHT_IN3_PIN, IOT_GPIO_VALUE0);
    IoTGpioSetOutputVal(MOTOR_RIGHT_IN4_PIN, IOT_GPIO_VALUE1);
    IoTPwmStart(MOTOR_RIGHT_ENB_PWM_PORT, Pwm, IOT_FREQ);
}

void MotorBackward(uint16_t Pwm)
{
    MotorPwmStop();
    // 左轮反转
    IoTGpioSetOutputVal(MOTOR_LEFT_IN1_PIN, IOT_GPIO_VALUE1);
    IoTGpioSetOutputVal(MOTOR_LEFT_IN2_PIN, IOT_GPIO_VALUE0);
    IoTPwmStart(MOTOR_LEFT_ENA_PWM_PORT, Pwm, IOT_FREQ);
    // 右轮反转
    IoTGpioSetOutputVal(MOTOR_RIGHT_IN3_PIN, IOT_GPIO_VALUE1);
    IoTGpioSetOutputVal(MOTOR_RIGHT_IN4_PIN, IOT_GPIO_VALUE0);
    IoTPwmStart(MOTOR_RIGHT_ENB_PWM_PORT, Pwm, IOT_FREQ);
}

void MotorControl(int LeftPwm, int RightPwm)
{
    MotorPwmStop();
    // printf("%d %d\n", LeftPwm, RightPwm);
    if (LeftPwm < 0)
    {
        IoTGpioSetOutputVal(MOTOR_LEFT_IN1_PIN, IOT_GPIO_VALUE0);
        IoTGpioSetOutputVal(MOTOR_LEFT_IN2_PIN, IOT_GPIO_VALUE1);
        LeftPwm = -LeftPwm;
    }
    else
    {
        IoTGpioSetOutputVal(MOTOR_LEFT_IN1_PIN, IOT_GPIO_VALUE1);
        IoTGpioSetOutputVal(MOTOR_LEFT_IN2_PIN, IOT_GPIO_VALUE0);
    }
    IoTPwmStart(MOTOR_LEFT_ENA_PWM_PORT, LeftPwm, IOT_FREQ);

    if (RightPwm < 0)
    {
        IoTGpioSetOutputVal(MOTOR_RIGHT_IN3_PIN, IOT_GPIO_VALUE0);
        IoTGpioSetOutputVal(MOTOR_RIGHT_IN4_PIN, IOT_GPIO_VALUE1);
        RightPwm = -RightPwm;
    }
    else
    {
        IoTGpioSetOutputVal(MOTOR_RIGHT_IN3_PIN, IOT_GPIO_VALUE1);
        IoTGpioSetOutputVal(MOTOR_RIGHT_IN4_PIN, IOT_GPIO_VALUE0);
    }
    IoTPwmStart(MOTOR_RIGHT_ENB_PWM_PORT, RightPwm, IOT_FREQ);

    // printf("%d %d\n", LeftPwm, RightPwm);
}
