#include <stdio.h>
#include <stdlib.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "hi_io.h"
#include "iot_gpio_ex.h"
#include "iot_watchdog.h"
#include "hi_time.h"
#include "pwm.h"

#define SOLENOID_VALVE_GPIO (6)
#define PUMP_GPIO (7)
#define ACTUATOR_GPIO_1 (9)
#define ACTUATOR_GPIO_2 (10)
#define ACTUATOR_GPIO_3 (2)
#define ACTUATOR_GPIO_4 (8)
#define ACTUATOR_GPIO_5 (11)
#define ACTUATOR_GPIO_6 (12)

#define COUNT 10
#define FREQ_TIME 20000

void PwmInit(void)
{
    IoTGpioInit(SOLENOID_VALVE_GPIO);
    IoTGpioSetFunc(SOLENOID_VALVE_GPIO, IOT_GPIO_FUNC_GPIO_6_GPIO);
    IoTGpioSetDir(SOLENOID_VALVE_GPIO, IOT_GPIO_DIR_OUT);

    IoTGpioInit(PUMP_GPIO);
    IoTGpioSetFunc(PUMP_GPIO, IOT_GPIO_FUNC_GPIO_7_GPIO);
    IoTGpioSetDir(PUMP_GPIO, IOT_GPIO_DIR_OUT);

    IoTGpioInit(ACTUATOR_GPIO_1);
    IoTGpioSetFunc(ACTUATOR_GPIO_1, IOT_GPIO_FUNC_GPIO_9_GPIO);
    IoTGpioSetDir(ACTUATOR_GPIO_1, IOT_GPIO_DIR_OUT);
}

void SetAngle(unsigned int duty, int gpio)
{
    unsigned int time = FREQ_TIME;

    IoTGpioSetOutputVal(gpio, IOT_GPIO_VALUE1);
    hi_udelay(duty);
    IoTGpioSetOutputVal(gpio, IOT_GPIO_VALUE0);
    hi_udelay(time - duty);
}

/* The steering gear is centered
 * 1、依据角度与脉冲的关系，设置高电平时间为1500微秒
 * 2、不断地发送信号，控制舵机居中
 */
void RegressMiddle(int gpio)
{
    unsigned int angle = 1500;
    for (int i = 0; i < COUNT; i++)
    {
        SetAngle(angle, gpio);
    }
}

/* Turn 90 degrees to the right of the steering gear
 * 1、依据角度与脉冲的关系，设置高电平时间为500微秒
 * 2、不断地发送信号，控制舵机向右旋转90度
 */
/*  Steering gear turn right */
void EngineTurnRight(int gpio)
{
    unsigned int angle = 500;
    for (int i = 0; i < COUNT; i++)
    {
        SetAngle(angle, gpio);
    }
}

/* Turn 90 degrees to the left of the steering gear
 * 1、依据角度与脉冲的关系，设置高电平时间为2500微秒
 * 2、不断地发送信号，控制舵机向左旋转90度
 */
/* Steering gear turn left */
void EngineTurnLeft(int gpio)
{
    unsigned int angle = 2500;
    for (int i = 0; i < COUNT; i++)
    {
        SetAngle(angle, gpio);
    }
}

