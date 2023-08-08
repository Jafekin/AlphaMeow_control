/*
 * @Author        陈佳辉 1946847867@qq.com
 * @Date          2023-08-06 21:56:38
 * @LastEditTime  2023-08-07 20:12:37
 * @DescripCurrentEncoderion
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_errno.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"
#include "hi_time.h"
#include "encoder.h"

// iot_gpio.h的极性设置与示波器实测是反的, 重新定义一个类型 （from 润和）
typedef enum
{
    /* InterrupCurrentEncoder at a high level or rising edge */
    IOT_GPIO_INT_EDGE_RISE = 0,
    IOT_GPIO_INT_EDGE_FALL
} GpioIntEdgeTriggerEdgeity;

// 串口定义
#define LEFT_ENCODER_A_GPIO (0)
#define LEFT_ENCODER_A_GPIO_FUNC (IOT_GPIO_FUNC_GPIO_0_GPIO)
#define LEFT_ENCODER_B_GPIO (12)
#define LEFT_ENCODER_B_GPIO_FUNC (IOT_GPIO_FUNC_GPIO_12_GPIO)
#define RIGHT_ENCODER_A_GPIO (7)
#define RIGHT_ENCODER_A_GPIO_FUNC (IOT_GPIO_FUNC_GPIO_7_GPIO)
#define RIGHT_ENCODER_B_GPIO (1)
#define RIGHT_ENCODER_B_GPIO_FUNC (IOT_GPIO_FUNC_GPIO_1_GPIO)
#define ENCODER_DIRECTION_REVERT (0) // 0: 使编码器增加方向与车前进方向相同
                                     // 1: 使编码器增加方向与车前进方向相反

#if ENCODER_DIRECTION_REVERT
#define COUNT_POSITVE (-1)
#define COUNT_NEGTITVE (1)
#else
#define COUNT_POSITVE (1)
#define COUNT_NEGTITVE (-1)
#endif

typedef struct
{
    /* data */
    unsigned int Phase_A;
    unsigned int Phase_B;
    int16_t Counter;
    GpioIntEdgeTriggerEdgeity TriggerEdge;
} Encoder;

Encoder LeftWheel;
Encoder RightWheel;

void EncoderInit(void)
{
    LeftWheel.Phase_A = LEFT_ENCODER_A_GPIO;
    LeftWheel.Phase_B = LEFT_ENCODER_B_GPIO;
    RightWheel.Phase_A = RIGHT_ENCODER_A_GPIO;
    RightWheel.Phase_A = RIGHT_ENCODER_B_GPIO;

    IoTGpioInit(LEFT_ENCODER_B_GPIO);                              // 左侧电机编码器B相的GPIO初始化
    IoTGpioSetFunc(LEFT_ENCODER_B_GPIO, LEFT_ENCODER_B_GPIO_FUNC); // 设置GPIO0的管脚复用关系为GPIO
    IoTGpioSetDir(LEFT_ENCODER_B_GPIO, IOT_GPIO_DIR_IN);           // GPIO0方向设置为输入
    IoTGpioSetPull(LEFT_ENCODER_B_GPIO, IOT_GPIO_PULL_UP);         // 设置GPIO0为上拉功能

    IoTGpioInit(LEFT_ENCODER_A_GPIO);                              // 左侧电机编码器A相的GPIO初始化
    IoTGpioSetFunc(LEFT_ENCODER_A_GPIO, LEFT_ENCODER_A_GPIO_FUNC); // 设置GPIO12的管脚复用关系为GPIO
    IoTGpioSetDir(LEFT_ENCODER_A_GPIO, IOT_GPIO_DIR_IN);           // GPIO12方向设置为输入
    IoTGpioSetPull(LEFT_ENCODER_A_GPIO, IOT_GPIO_PULL_UP);         // 设置GPIO12为上拉功能

    IoTGpioInit(RIGHT_ENCODER_B_GPIO);                               // 右侧电机编码器B相的GPIO初始化
    IoTGpioSetFunc(RIGHT_ENCODER_B_GPIO, RIGHT_ENCODER_B_GPIO_FUNC); // 设置GPIO07的管脚复用关系为GPIO
    IoTGpioSetDir(RIGHT_ENCODER_B_GPIO, IOT_GPIO_DIR_IN);            // GPIO07方向设置为输入
    IoTGpioSetPull(RIGHT_ENCODER_B_GPIO, IOT_GPIO_PULL_UP);          // 设置GPIO07为上拉功能

    IoTGpioInit(RIGHT_ENCODER_A_GPIO);                               // 右侧电机编码器A相的GPIO初始化
    IoTGpioSetFunc(RIGHT_ENCODER_A_GPIO, RIGHT_ENCODER_A_GPIO_FUNC); // 设置GPIO01的管脚复用关系为GPIO
    IoTGpioSetDir(RIGHT_ENCODER_A_GPIO, IOT_GPIO_DIR_IN);            // GPIO01方向设置为输入
    IoTGpioSetPull(RIGHT_ENCODER_A_GPIO, IOT_GPIO_PULL_UP);          // 设置GPIO01为上拉功能

    IoTGpioRegisterIsrFunc(LEFT_ENCODER_A_GPIO, IOT_INT_TYPE_EDGE,
                           IOT_GPIO_INT_EDGE_RISE, EncoderCallback, (char *)(&LeftWheel));
    IoTGpioRegisterIsrFunc(RIGHT_ENCODER_A_GPIO, IOT_INT_TYPE_EDGE,
                           IOT_GPIO_INT_EDGE_RISE, EncoderCallback, (char *)(&RightWheel));

    printf("init_wheel_codec\n");
}

void EncoderCallback(char *arg)
{
    IotGpioValue LevelGet;
    uint32_t *const reg = (uint32_t *)(0x5000603C); // reg GPIO_INT_TriggerEdgeITY
    Encoder *pCurrentEncoder = (Encoder *)(arg);

    if (IoTGpioGetInputVal(pCurrentEncoder->Phase_B, &LevelGet) != IOT_SUCCESS)
    {
        printf("read wheel right SIGNAL B read fail\n");
    }

    if (LevelGet == 0)
    {
        pCurrentEncoder->Counter += (pCurrentEncoder->TriggerEdge == IOT_GPIO_INT_EDGE_RISE) ? COUNT_POSITVE : COUNT_NEGTITVE;
    }
    else
    {
        pCurrentEncoder->Counter += (pCurrentEncoder->TriggerEdge == IOT_GPIO_INT_EDGE_RISE) ? COUNT_NEGTITVE : COUNT_POSITVE;
    }
    /* 加速中断极性转换时间 */
    pCurrentEncoder->TriggerEdge ^= 1;
    if (pCurrentEncoder->TriggerEdge)
    {
        *reg |= (1 << pCurrentEncoder->Phase_A);
    }
    else
    {
        *reg &= ~(1 << pCurrentEncoder->Phase_A);
    }
}

int16_t EncoderGetRightWheelCnt(void)
{
    return RightWheel.Counter;
}

int16_t EncoderGetLeftWheelCnt(void)
{
    return LeftWheel.Counter;
}

void EncoderGetWheelCnt(int16_t *LeftCnt, int16_t *RightCnt)
{
    *LeftCnt = LeftWheel.Counter;
    *RightCnt = RightWheel.Counter;
    return;
}