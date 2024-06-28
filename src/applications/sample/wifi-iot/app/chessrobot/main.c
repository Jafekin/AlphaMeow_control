/*
 * @Author        陈佳辉 1946847867@qq.com
 * @Date          2024-06-26 13:12:34
 * @LastEditTime  2024-06-28 16:31:46
 * @Description
 *
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_errno.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"
#include "iot_uart.h"
#include "iot_i2c.h"
#include "iot_i2c_ex.h"
#include "hi_i2c.h"
#include "pca9685.h"

#define IOT_I2C0_SDA_GPIO (13)
#define IOT_I2C0_SCL_GPIO (14)

#define SOLENOID_VALVE_CHANNEL (0)
#define PUMP_CHANNEL (1)

#define ACTUATOR_CHANNEL_1 (2)
#define ACTUATOR_CHANNEL_2 (3)
#define ACTUATOR_CHANNEL_3 (4)
#define ACTUATOR_CHANNEL_4 (5)

static void ControTask(void)
{
    IoTGpioInit(IOT_I2C0_SDA_GPIO);
    IoTGpioSetFunc(IOT_I2C0_SDA_GPIO, IOT_GPIO_FUNC_GPIO_13_I2C0_SDA);

    IoTGpioInit(IOT_I2C0_SCL_GPIO);
    IoTGpioSetFunc(IOT_I2C0_SCL_GPIO, IOT_GPIO_FUNC_GPIO_14_I2C0_SCL);

    int ret;
    if ((ret = IoTI2cInit(HI_I2C_IDX_0, 400000)) != 0)
    {
        printf(" [IoTI2c0Init] Failed!: 0x%x \n", ret);
    }

    PCA9685_Init();
    PCA9685_Set_PWM_Freq(50);

    while (1)
    {

        PCA9685_Angle(ACTUATOR_CHANNEL_4, 180);

        sleep(3000000);
        // PCA9685_Angle(SOLENOID_VALVE_CHANNEL, 180);
        // PCA9685_Angle(PUMP_CHANNEL, 0);
        // msleep(3000);
        // PCA9685_Angle(PUMP_CHANNEL, 180);

        // PCA9685_Angle(PUMP_CHANNEL, 180);
        // PCA9685_Angle(SOLENOID_VALVE_CHANNEL, 0);
        // msleep(3000);
        // PCA9685_Angle(SOLENOID_VALVE_CHANNEL, 180);
    }
}

static void TaskCreateAndInit(void)
{
    osThreadAttr_t attr;

    attr.name = "ControTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024 * 8;
    attr.priority = 25;
    if (osThreadNew((osThreadFunc_t)ControTask, NULL, &attr) == NULL)
    {
        printf("Failed to create PIDTask!\n");
    }
}

SYS_RUN(TaskCreateAndInit);
