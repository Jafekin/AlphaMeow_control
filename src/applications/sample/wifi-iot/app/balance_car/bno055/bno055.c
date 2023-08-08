/*
 * @Author        陈佳辉 1946847867@qq.com
 * @Date          2023-08-05 18:38:54
 * @LastEditTime  2023-08-08 17:46:29
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
        ret = BNO055WriteReg(BNO_REG_ADDR_OPR_MODE, BNO_REG_VAL_OPR_MODE_NDOF);
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
        gyro->z = (int16_t)uart_buff_ptr[5] << 8 | (int16_t)uart_buff_ptr[6];

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
