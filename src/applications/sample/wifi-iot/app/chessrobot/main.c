/*
 * @Author        陈佳辉 1946847867@qq.com
 * @Date          2024-06-26 13:12:34
 * @LastEditTime  2024-06-30 00:22:21
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
#include "iot_adc.h"
#include "iot_gpio_ex.h"
#include "iot_uart.h"
#include "iot_i2c.h"
#include "iot_i2c_ex.h"
#include "hi_i2c.h"
#include "hi_adc.h"
#include "pca9685.h"
#include "oled_ssd1306.h"

#define BUTTON_GPIO (5)

#define IOT_I2C0_SDA_GPIO (13)
#define IOT_I2C0_SCL_GPIO (14)

#define SOLENOID_VALVE_CHANNEL (0)
#define PUMP_CHANNEL (1)

#define ACTUATOR_CHANNEL_1 (2)
#define ACTUATOR_CHANNEL_2 (3)
#define ACTUATOR_CHANNEL_3 (4)
#define ACTUATOR_CHANNEL_4 (5)

osMutexId_t adjustMutex;
osTimerId_t keyTimerId;

int keySet = 0;
int actuatorNo = 2;
int pwm[4] = {153, 23, 113, 92};
int action[90][4] = {{168, 3, 78, 70}};

static float GetVoltage(void)
{
    unsigned int ret;
    unsigned short data;

    if ((ret = IoTAdcRead(HI_ADC_CHANNEL_2, &data, IOT_ADC_EQU_MODEL_8, IOT_ADC_CUR_BAIS_DEFAULT, 0xA)) != 0)
    {
        printf("[GetVoltage] ADC Read Fail: 0x%x\n", ret);
    }

    return (float)data * 1.8 * 4 / 4096.0;
}

static void KeyTimerCallback(char *arg)
{
    (void)arg;
    static int pre_keySet = 2;
    float key = GetVoltage();
    if (key > 2.0f)
        keySet = 0;
    else if (key > 0.85f)
    {
        keySet = 2;
        if (keySet != pre_keySet)
        {
            pwm[actuatorNo - 2]++;
            if (pwm[actuatorNo - 2] > 180)
                pwm[actuatorNo - 2] -= 181;
            else if (pwm[actuatorNo - 2] < 0)
                pwm[actuatorNo - 2] += 181;
        }
    }
    else if (key > 0.5f)
    {
        keySet = 1;
        if (keySet != pre_keySet)
        {
            pwm[actuatorNo - 2]--;
            if (pwm[actuatorNo - 2] > 180)
                pwm[actuatorNo - 2] -= 181;
            else if (pwm[actuatorNo - 2] < 0)
                pwm[actuatorNo - 2] += 181;
        }
    }
    else if (key > 0.0f)
    {
        keySet = 3;
        if (keySet != pre_keySet)
        {
            actuatorNo++;
            if (actuatorNo > 5)
                actuatorNo -= 4;
            else if (actuatorNo < 2)
                actuatorNo += 4;
        }
    }
    // printf("[KeyTimerCallback] %d -> %d (%f)\n", pre_keySet, keySet, key);
    pre_keySet = keySet;
}

static void SuckUp(void)
{
    PCA9685_Angle(SOLENOID_VALVE_CHANNEL, 180);
    PCA9685_Angle(PUMP_CHANNEL, 0);
    msleep(3000);
    PCA9685_Angle(PUMP_CHANNEL, 180);
}

static void PutDown(void)
{
    PCA9685_Angle(PUMP_CHANNEL, 180);
    PCA9685_Angle(SOLENOID_VALVE_CHANNEL, 0);
    msleep(3000);
    PCA9685_Angle(SOLENOID_VALVE_CHANNEL, 180);
}

static void ControlTask(void)
{
    unsigned int ret = 0;

    IoTGpioInit(IOT_I2C0_SDA_GPIO);
    IoTGpioSetFunc(IOT_I2C0_SDA_GPIO, IOT_GPIO_FUNC_GPIO_13_I2C0_SDA);

    IoTGpioInit(IOT_I2C0_SCL_GPIO);
    IoTGpioSetFunc(IOT_I2C0_SCL_GPIO, IOT_GPIO_FUNC_GPIO_14_I2C0_SCL);

    if ((ret = IoTI2cInit(HI_I2C_IDX_0, 400000)) != 0)
    {
        printf(" [IoTI2c0Init] Failed!: 0x%x \n", ret);
    }

    PCA9685_Init();
    PCA9685_Set_PWM_Freq(50);

    PCA9685_Angle(ACTUATOR_CHANNEL_1, pwm[ACTUATOR_CHANNEL_1 - 2]);
    PCA9685_Angle(ACTUATOR_CHANNEL_2, pwm[ACTUATOR_CHANNEL_2 - 2]);
    PCA9685_Angle(ACTUATOR_CHANNEL_3, pwm[ACTUATOR_CHANNEL_3 - 2]);
    PCA9685_Angle(ACTUATOR_CHANNEL_4, pwm[ACTUATOR_CHANNEL_4 - 2]);

    while (1)
    {
        // osMutexAcquire(adjustMutex, 1);
        PCA9685_Angle(actuatorNo, pwm[actuatorNo - 2]);
        // osMutexRelease(adjustMutex);
        msleep(100);

        // SuckUp();
        // PutDown();
    }
}

static void AdjustTask(void)
{
    IoTGpioInit(BUTTON_GPIO);
    IoTGpioSetFunc(BUTTON_GPIO, IOT_GPIO_FUNC_GPIO_5_GPIO);
    IoTGpioSetDir(BUTTON_GPIO, IOT_GPIO_DIR_IN);
    // IoTGpioSetPull(BUTTON_GPIO, IOT_GPIO_PULL_UP);
    // IoTGpioRegisterIsrFunc(BUTTON_GPIO, IOT_INT_TYPE_EDGE, IOT_GPIO_EDGE_FALL_LEVEL_LOW, F1Pressed, NULL);

    OledInit();
    OledFillScreen(0);

    char show[100];

    while (1)
    {
        // osMutexAcquire(adjustMutex, 1);

        sprintf(show, "Actuator No: %d", actuatorNo);
        OledShowString(0, 0, show, FONT6_X8);
        sprintf(show, "Pwm: %d %d %d %d", pwm[0], pwm[1], pwm[2], pwm[3]);
        OledShowString(0, 1, show, FONT6_X8);

        // printf("Actuator No: %d\n", actuatorNo);
        // printf("Pwm: %d\n", pwm);

        // osMutexRelease(adjustMutex);

        msleep(100);
    }
}

static void TaskCreateAndInit(void)
{
    if ((keyTimerId = osTimerNew(KeyTimerCallback, osTimerPeriodic, NULL, NULL)) == NULL)
    {
        printf("[TaskCreateAndInit] Failed to create keyTimerId!\n");
    }
    osTimerStart(keyTimerId, 1U);

    if ((adjustMutex = osMutexNew(NULL)) == NULL)
    {
        printf("[TaskCreateAndInit] Failed to create adjustMutex!\n");
    }

    osThreadAttr_t attr;

    attr.name = "ControlTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024 * 8;
    attr.priority = 25;
    if (osThreadNew((osThreadFunc_t)ControlTask, NULL, &attr) == NULL)
    {
        printf("[TaskCreateAndInit] Failed to create ControlTask!\n");
    }

    attr.name = "AdjustTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024 * 8;
    attr.priority = 24;
    if (osThreadNew((osThreadFunc_t)AdjustTask, NULL, &attr) == NULL)
    {
        printf("[TaskCreateAndInit] Failed to create AdjustTask!\n");
    }
}

SYS_RUN(TaskCreateAndInit);
