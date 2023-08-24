/*
 * @Author        陈佳辉 1946847867@qq.com
 * @Date          2023-08-19 14:11:19
 * @LastEditTime  2023-08-19 14:23:38
 * @Description
 *
 */
#ifndef __MPU6050_H__
#define __MPU6050_H__

/* 宏定义 --------------------------------------------------------------------*/
#define MPU6050_GYRO_OUT 0x43        // MPU6050陀螺仪数据寄存器地址
#define MPU6050_ACC_OUT 0x3B         // MPU6050加速度数据寄存器地址
#define MPU6050_ADDRESS 0x68         // MPU6050器件读地址
#define MPU6050_ADDRESS_AD0_LOW 0x68 // address pin low (GND), default for InvenSense evaluation board
#define MPU6050_RA_CONFIG 0x1A
#define MPU6050_RA_ACCEL_CONFIG 0x1C
#define MPU6050_RA_FF_THR 0x1D
#define MPU6050_RA_FF_DUR 0x1E
#define MPU6050_RA_MOT_THR 0x1F // 运动检测阀值设置寄存器
#define MPU6050_RA_MOT_DUR 0x20 // 运动检测时间阀值
#define MPU6050_RA_ZRMOT_THR 0x21
#define MPU6050_RA_ZRMOT_DUR 0x22
#define MPU6050_RA_FIFO_EN 0x23
#define MPU6050_RA_INT_PIN_CFG 0x37 // 中断/旁路设置寄存器
#define MPU6050_RA_INT_ENABLE 0x38  // 中断使能寄存器
#define MPU6050_RA_TEMP_OUT_H 0x41
#define MPU6050_RA_USER_CTRL 0x6A
#define MPU6050_RA_PWR_MGMT_1 0x6B
#define MPU6050_RA_WHO_AM_I 0x75
#define MPU6050_DATA_2_BYTE 2
#define MPU6050_DATA_256_BYTE 256
#define MPU6050_DATA_DELAY 100

#define SENSOR_DATA_WIDTH_8_BIT 8 // 8 bit
#define ACCEL_DATA_LEN 6
#define TEMP_DATA_LEN 2

#define MPU6050_CONSTANT_1 13200
#define MPU6050_CONSTANT_2 280
#define MPU6050_CONSTANT_3 13

#define WIFI_IOT_IO_FUNC_GPIO_0_I2C1_SDA 6
#define WIFI_IOT_IO_FUNC_GPIO_1_I2C1_SCL 6
#define WIFI_IOT_I2C_IDX_1 1
#define WIFI_IOT_I2C_BAUDRATE 400000

#define WIFI_IOT_IO_NAME_GPIO_0 0
#define WIFI_IOT_IO_NAME_GPIO_1 1
#define RESET_DELAY_US 20000
#define READ_DATA_DELAY_US 50000

enum AccelAxisNum
{
    ACCEL_X_AXIS = 0,
    ACCEL_Y_AXIS = 1,
    ACCEL_Z_AXIS = 2,
    ACCEL_AXIS_NUM = 3,
};
enum AccelAxisPart
{
    ACCEL_X_AXIS_LSB = 0,
    ACCEL_X_AXIS_MSB = 1,
    ACCEL_Y_AXIS_LSB = 2,
    ACCEL_Y_AXIS_MSB = 3,
    ACCEL_Z_AXIS_LSB = 4,
    ACCEL_Z_AXIS_MSB = 5,
    ACCEL_AXIS_BUTT,
};
enum TempPart
{
    TEMP_LSB = 0,
    TEMP_MSB = 1,
};

#endif