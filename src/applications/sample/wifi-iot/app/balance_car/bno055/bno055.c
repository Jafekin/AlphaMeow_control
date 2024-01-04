/*
 * @Author        陈佳辉 1946847867@qq.com
 * @Date          2023-08-05 18:38:54
 * @LastEditTime  2023-08-28 18:35:39
 * @Description
 *
 */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "cmsis_os2.h"
#include "iot_errno.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"
#include "iot_uart.h"

#include "bno055.h"

#define BNO055_UART 1
#define UART_BUFF_SIZE 20
uint8_t uart_buff[UART_BUFF_SIZE] = {0};
uint8_t *uart_buff_ptr = uart_buff;

uint8_t Read_Cmd[4] = {0xAA, 0x01, 0, 1};
uint8_t Write_Cmd[5] = {0xAA, 0x00, 0x00, 0x01, 0x00};
#define Writereg Write_Cmd[2]
#define Writedat Write_Cmd[4]
#define Readreg Read_Cmd[2]
#define Readlen Read_Cmd[3]

/**
 * @brief 初始化连接BNO055的串口
 * @return 0，成功；1，失败
 */
uint8_t BNO055UartIoInit(void)
{
    IotUartAttribute uart_attr =
        {
            .baudRate = 115200,                    // 波特率
            .dataBits = IOT_UART_DATA_BIT_8,       // 数据位
            .stopBits = IOT_UART_STOP_BIT_1,       // 停止位
            .parity = IOT_UART_PARITY_NONE,        // 奇偶校验
            .rxBlock = IOT_UART_BLOCK_STATE_BLOCK, // 接收是否阻塞
            .txBlock = IOT_UART_BLOCK_STATE_BLOCK, // 发送是否阻塞
            .pad = 0,
        };
    // 设置 UART1 的引脚复用并初始化
    IoTGpioInit(5);
    IoTGpioSetFunc(5, IOT_GPIO_FUNC_GPIO_5_UART1_RXD);
    IoTGpioInit(6);

    IoTGpioSetFunc(6, IOT_GPIO_FUNC_GPIO_6_UART1_TXD);
    // 初始化 UART1
    uint32_t ret = IoTUartInit(BNO055_UART, &uart_attr);
    if (ret != IOT_SUCCESS)
    {
        printf("Failed to init UART! Error: %d\n", ret);
        return 1;
    }
    return 0;
}

/**
 * @brief 写数据到BNO055寄存器
 * @param {uint8_t} reg_add 写入地址
 * @param {uint8_t} reg_dat 写入数据
 * @return 0，成功；1，失败
 */
uint8_t BNO055WriteReg(uint8_t reg_add, uint8_t reg_dat)
{
    uint8_t ret;
    Writereg = reg_add;
    Writedat = reg_dat;
    ret = IoTUartWrite(BNO055_UART, Write_Cmd, 5);
    if (ret != 5)
    {
        printf("Error: Uart write ret = 0x%x!\r\n", ret);
        return 1;
    }
    return 0;
}

/**
 * @brief 向寄存器发出读取的命令
 * @param {uint8_t} reg_add 读取寄存器
 * @param {uint8_t} len 读取字节数
 * @return 0，成功；1，失败
 */
uint8_t BNO055ReadReg(uint8_t reg_add, uint8_t len)
{
    uint8_t ret;
    Readreg = reg_add;
    Readlen = len;
    ret = IoTUartWrite(BNO055_UART, Read_Cmd, 4);
    if (ret != 4)
    {
        printf("Error: Uart read ret = 0x%x!\r\n", ret);
        return 1;
    }
    return 0;
}

/**
 * @brief 初始化bno055
 * @return {*}
 */
void BNO055Init(void)
{
    BNO055UartIoInit();
    uint8_t ret;
    for (uint8_t i = 0; i < 50; i++)
    {
        // 查找ID-->0xA0
        BNO055ReadReg(0, 1);
        usleep(10);
        IoTUartRead(BNO055_UART, uart_buff_ptr, 3);
        if (uart_buff_ptr[2] == BNO_REG_VAL_CHIPID)
            break;
    }
    if (uart_buff_ptr[2] != BNO_REG_VAL_CHIPID)
    {
        printf("Error: BNO055Init ID !\r\n");
    }
    else
    {
        printf("BNO055_Init ID OK\r\n");
        ret = BNO055WriteReg(BNO_REG_ADDR_OPR_MODE, BNO_REG_VAL_OPR_MODE_CFG);
        if (ret != 0)
        {
            printf("Error: BNO055Init 1 !\r\n");
        }
        usleep(19);
        ret = BNO055WriteReg(BNO_REG_ADDR_OPR_MODE, BNO_REG_VAL_OPR_MODE_NDOF_FMC_OFF);
        if (ret != 0)
        {
            printf("Error: BNO055Init 2 !\r\n");
        }
        usleep(7);
    }
}

/**
 * @brief 读取各个方向加速度
 * @param {bno055_accel_t} *acc
 * @return {*}
 */
void ReadBNO005Acc(struct bno055_accel_t *acc)
{
    BNO055ReadReg(BNO_REG_ADDR_ACC_X_L, 6);
    usleep(5);
    IoTUartRead(BNO055_UART, uart_buff_ptr, 8);

    if (uart_buff_ptr[0] == 0xBB)
    {
        acc->x = (int16_t)uart_buff_ptr[3] << 8 | (int16_t)uart_buff_ptr[2];
        acc->y = (int16_t)uart_buff_ptr[5] << 8 | (int16_t)uart_buff_ptr[4];
        acc->z = (int16_t)uart_buff_ptr[7] << 8 | (int16_t)uart_buff_ptr[6];

        acc->x /= BNO_ACC_LSB;
        acc->y /= BNO_ACC_LSB;
        acc->z /= BNO_ACC_LSB;
        // printf("ACC:X = %d,Y = %d,Z = %d\r\n", acc->x, acc->y, acc->z);
    }
    // else
    // {
    //     printf("acc return error\t\n");
    // }
}

/**
 * @brief 读取各个方向的磁力
 * @param {bno055_mag_t} *mag
 * @return {*}
 */
void ReadBNO005Mag(struct bno055_mag_t *mag)
{
    BNO055ReadReg(BNO_REG_ADDR_MAG_X_L, 6);
    usleep(5);
    IoTUartRead(BNO055_UART, uart_buff_ptr, 8);

    if (uart_buff_ptr[0] == 0xBB)
    {
        mag->x = (int16_t)uart_buff_ptr[3] << 8 | (int16_t)uart_buff_ptr[2];
        mag->y = (int16_t)uart_buff_ptr[5] << 8 | (int16_t)uart_buff_ptr[4];
        mag->z = (int16_t)uart_buff_ptr[7] << 8 | (int16_t)uart_buff_ptr[6];

        mag->x >>= BNO_MAG_SHIFT_BITS;
        mag->y >>= BNO_MAG_SHIFT_BITS;
        mag->z >>= BNO_MAG_SHIFT_BITS;
        // printf("MAG:X = %d,Y = %d,Z = %d\r\n", mag->x, mag->y, mag->z);
    }
    // else
    // {
    //     printf("mag return error\t\n");
    // }
}

/**
 * @brief 读取各个方向角速度
 * @param {bno055_gyro_t} *gyro
 * @return {*}
 */
void ReadBNO005Gyro(struct bno055_gyro_t *gyro)
{
    BNO055ReadReg(BNO_REG_ADDR_GYR_X_L, 6);
    usleep(5);
    IoTUartRead(BNO055_UART, uart_buff_ptr, 8);

    if (uart_buff_ptr[0] == 0xBB)
    {
        gyro->x = (int16_t)uart_buff_ptr[3] << 8 | (int16_t)uart_buff_ptr[2];
        gyro->y = (int16_t)uart_buff_ptr[5] << 8 | (int16_t)uart_buff_ptr[4];
        gyro->z = (int16_t)uart_buff_ptr[7] << 8 | (int16_t)uart_buff_ptr[6];

        gyro->x >>= BNO_GYR_SHIFT_BITS;
        gyro->y >>= BNO_GYR_SHIFT_BITS;
        gyro->z >>= BNO_GYR_SHIFT_BITS;
        // printf("GYR:X = %d,Y = %d,Z = %d\r\n", gyro->x, gyro->y, gyro->z);
    }
    // else
    // {
    //     printf("gyro return error\t\n");
    // }
}

/**
 * @brief 读取欧拉角
 * @param {bno055_euler_t} *euler
 * @return {*}
 */
void ReadBNO005Euler(struct bno055_euler_t *euler)
{
    BNO055ReadReg(BNO_REG_ADDR_EUL_YAW_L, 6);
    usleep(5);
    IoTUartRead(BNO055_UART, uart_buff_ptr, 8);

    if (uart_buff_ptr[0] == 0xBB)
    {

        euler->h = (int16_t)uart_buff_ptr[3] << 8 | (int16_t)uart_buff_ptr[2];
        euler->r = (int16_t)uart_buff_ptr[5] << 8 | (int16_t)uart_buff_ptr[4];
        euler->p = (int16_t)uart_buff_ptr[7] << 8 | (int16_t)uart_buff_ptr[6];

        euler->h >>= BNO_EUL_DEG_SHIFT_BITS;
        euler->r >>= BNO_EUL_DEG_SHIFT_BITS;
        euler->p >>= BNO_EUL_DEG_SHIFT_BITS;
        // printf("euler:yaw = %d,roll = %d,pitch = %d\r\n", euler->h, euler->r, euler->p);
    }
    // else
    // {
    //     printf("euler return error\t\n");
    // }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// /**
//  * 6DOF 互补滤波姿态估计(via Mahony)
//  * @param[in] halfT:状态估计周期的一半
//  */
// const float Kp = 3.5, Ki = 0.05, halfT = 0.005;
// float exInt = 0, eyInt = 0, ezInt = 0;
// float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f; // roll,pitch,yaw 都为 0 时对应的四元数值。
// struct bno055_euler_float_t IMUupdate(float gx, float gy, float gz, float ax, float ay, float az)
// {
//     float norm;
//     float vx, vy, vz;
//     float ex, ey, ez;

//     float q0q0 = q0 * q0;
//     float q0q1 = q0 * q1;
//     float q0q2 = q0 * q2;
//     float q1q1 = q1 * q1;
//     float q1q3 = q1 * q3;
//     float q2q2 = q2 * q2;
//     float q2q3 = q2 * q3;
//     float q3q3 = q3 * q3;

//     struct bno055_euler_float_t euler;

//     if (ax * ay * az == 0)
//         return;

//     // 第一步：对加速度数据进行归一化
//     norm = sqrt(ax * ax + ay * ay + az * az);
//     ax = ax / norm;
//     ay = ay / norm;
//     az = az / norm;

//     // 第二步：DCM矩阵旋转
//     vx = 2 * (q1q3 - q0q2);
//     vy = 2 * (q0q1 + q2q3);
//     vz = q0q0 - q1q1 - q2q2 + q3q3;

//     // 第三步：在机体坐标系下做向量叉积得到补偿数据
//     ex = ay * vz - az * vy;
//     ey = az * vx - ax * vz;
//     ez = ax * vy - ay * vx;

//     // 第四步：对误差进行PI计算，补偿角速度
//     exInt = exInt + ex * Ki;
//     eyInt = eyInt + ey * Ki;
//     ezInt = ezInt + ez * Ki;

//     gx = gx + Kp * ex + exInt;
//     gy = gy + Kp * ey + eyInt;
//     gz = gz + Kp * ez + ezInt;

//     // 第五步：按照四元数微分公式进行四元数更新
//     q0 = q0 + (-q1 * gx - q2 * gy - q3 * gz) * halfT;
//     q1 = q1 + (q0 * gx + q2 * gz - q3 * gy) * halfT;
//     q2 = q2 + (q0 * gy - q1 * gz + q3 * gx) * halfT;
//     q3 = q3 + (q0 * gz + q1 * gy - q2 * gx) * halfT;

//     norm = sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
//     q0 = q0 / norm;
//     q1 = q1 / norm;
//     q2 = q2 / norm;
//     q3 = q3 / norm;

//     euler.r = atan2f(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) * 57.3;
//     euler.p = asinf(2 * q1 * q3 - 2 * q0 * q2) * 57.3;
//     euler.h = -atan2f(2 * q1 * q2 + 2 * q0 * q3, -2 * q2 * q2 - 2 * q3 * q3 + 1) * 57.3;

//     return euler;
// }
