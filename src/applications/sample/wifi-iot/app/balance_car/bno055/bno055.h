/*
 * @Author        陈佳辉 1946847867@qq.com
 * @Date          2023-08-05 18:39:02
 * @LastEditTime  2023-08-28 18:33:46
 * @Description
 *
 */

#ifndef __BNO055_H__
#define __BNO055_H__

#define BNO_CMD_WR (0x00)
#define BNO_CMD_RD (0x01)
#define BNO_SYNC_CMD (0xAA)
#define BNO_SYNC_CMD_RES (0xEE)
#define BNO_SYNC_DATA_RES (0xBB)

#define BNO_RES_SUCC (0x01)
#define BNO_RES_FAIL (0x03)
#define BNO_RES_INVALID_REG_ADDR (0x04)
#define BNO_RES_REG_WR_DISABLED (0x05)
#define BNO_RES_WRONG_START_BYTE (0x06)
#define BNO_RES_BUS_OVER_RUN (0x07)
#define BNO_RES_MAX_LEN (0x08)
#define BNO_RES_MIN_LEN (0x09)
#define BNO_RES_RECV_CHAR_TIMEOUT (0x0A)

// Page0
#define BNO_REG_ADDR_CHIPID 0x00  /* value: 0xA0 */
#define BNO_REG_ADDR_ACC_ID 0x01  /* value: 0xFB */
#define BNO_REG_ADDR_MAG_ID 0x02  /* value: 0x32 */
#define BNO_REG_ADDR_GYRO_ID 0x03 /* value: 0x0F */

#define BNO_REG_ADDR_OPR_MODE 0x3D // operation mode
#define BNO_REG_ADDR_PAGE_ID 0x07
#define BNO_REG_ADDR_FW_VER_H 0x05
#define BNO_REG_ADDR_FW_VER_L 0x04

#define BNO_REG_ADDR_TEMP 0x34 // temperature

// Euler data
#define BNO_REG_ADDR_EUL_PITCH_H 0x1F
#define BNO_REG_ADDR_EUL_PITCH_L 0x1E
#define BNO_REG_ADDR_EUL_ROLL_H 0x1D
#define BNO_REG_ADDR_EUL_ROLL_L 0x1C
#define BNO_REG_ADDR_EUL_YAW_H 0x1B
#define BNO_REG_ADDR_EUL_YAW_L 0x1A

// Gyroscope data
#define BNO_REG_ADDR_GYR_Z_H 0x19
#define BNO_REG_ADDR_GYR_Z_L 0x18
#define BNO_REG_ADDR_GYR_Y_H 0x17
#define BNO_REG_ADDR_GYR_Y_L 0x16
#define BNO_REG_ADDR_GYR_X_H 0x15
#define BNO_REG_ADDR_GYR_X_L 0x14

// Magnetic data
#define BNO_REG_ADDR_MAG_Z_H 0x13
#define BNO_REG_ADDR_MAG_Z_L 0x12
#define BNO_REG_ADDR_MAG_Y_H 0x11
#define BNO_REG_ADDR_MAG_Y_L 0x10
#define BNO_REG_ADDR_MAG_X_H 0x0F
#define BNO_REG_ADDR_MAG_X_L 0x0E

// Accelerator data
#define BNO_REG_ADDR_ACC_Z_H 0x0D
#define BNO_REG_ADDR_ACC_Z_L 0x0C
#define BNO_REG_ADDR_ACC_Y_H 0x0B
#define BNO_REG_ADDR_ACC_Y_L 0x0A
#define BNO_REG_ADDR_ACC_X_H 0x09
#define BNO_REG_ADDR_ACC_X_L 0x08

/*Quaternion data registers*/
#define BNO055_QUATERNION_DATA_W_LSB_ADDR (0X20)
#define BNO055_QUATERNION_DATA_W_MSB_ADDR (0X21)
#define BNO055_QUATERNION_DATA_X_LSB_ADDR (0X22)
#define BNO055_QUATERNION_DATA_X_MSB_ADDR (0X23)
#define BNO055_QUATERNION_DATA_Y_LSB_ADDR (0X24)
#define BNO055_QUATERNION_DATA_Y_MSB_ADDR (0X25)
#define BNO055_QUATERNION_DATA_Z_LSB_ADDR (0X26)
#define BNO055_QUATERNION_DATA_Z_MSB_ADDR (0X27)

/* Linear acceleration data registers*/
#define BNO055_LINEAR_ACCEL_DATA_X_LSB_ADDR (0X28)
#define BNO055_LINEAR_ACCEL_DATA_X_MSB_ADDR (0X29)
#define BNO055_LINEAR_ACCEL_DATA_Y_LSB_ADDR (0X2A)
#define BNO055_LINEAR_ACCEL_DATA_Y_MSB_ADDR (0X2B)
#define BNO055_LINEAR_ACCEL_DATA_Z_LSB_ADDR (0X2C)
#define BNO055_LINEAR_ACCEL_DATA_Z_MSB_ADDR (0X2D)

/*Gravity data registers*/
#define BNO055_GRAVITY_DATA_X_LSB_ADDR (0X2E)
#define BNO055_GRAVITY_DATA_X_MSB_ADDR (0X2F)
#define BNO055_GRAVITY_DATA_Y_LSB_ADDR (0X30)
#define BNO055_GRAVITY_DATA_Y_MSB_ADDR (0X31)
#define BNO055_GRAVITY_DATA_Z_LSB_ADDR (0X32)
#define BNO055_GRAVITY_DATA_Z_MSB_ADDR (0X33)

#define BNO_REG_VAL_OPR_MODE_CFG 0x00
#define BNO_REG_VAL_OPR_MODE_NDOF 0x0C
#define BNO_REG_VAL_OPR_MODE_NDOF_FMC_OFF 0x0B
#define BNO_REG_VAL_OPR_MODE_IMU 0x08
#define BNO_REG_VAL_OPR_MODE_COMPASS 0x09
#define BNO_REG_VAL_OPR_MODE_M4G 0x0A
#define BNO_REG_VAL_OPR_MODE_M4G 0x0A
#define BNO_REG_VAL_CHIPID 0xA0

#define BNO_EUL_DEG_LSB (16) // 1 degree = 16 LSB
#define BNO_ACC_LSB (100)
#define BNO_GYR_LSB (16)
#define BNO_EMAG_LSB (16)

#define BNO_EUL_DEG_SHIFT_BITS (4) // 2 ^ 4 == 16
#define BNO_GYR_SHIFT_BITS (4)
#define BNO_MAG_SHIFT_BITS (4)
#define BNO_ACC_SCALER (100) // acc * 100

struct bno055_accel_t
{
    short x; /**< accel x data */
    short y; /**< accel y data */
    short z; /**< accel z data */
};

/*!
 * @brief struct for Mag data read from registers
 */
struct bno055_mag_t
{
    short x; /**< mag x data */
    short y; /**< mag y data */
    short z; /**< mag z data */
};

/*!
 * @brief struct for Gyro data read from registers
 */
struct bno055_gyro_t
{
    short x; /**< gyro x data */
    short y; /**< gyro y data */
    short z; /**< gyro z data */
};

/*!
 * @brief struct for Euler data read from registers
 */
struct bno055_euler_t
{
    short h; /**< Euler h data */
    short r; /**< Euler r data */
    short p; /**< Euler p data */
};

/*!
 * @brief struct for Quaternion data read from registers
 */
struct bno055_quaternion_t
{
    short w; /**< Quaternion w data */
    short x; /**< Quaternion x data */
    short y; /**< Quaternion y data */
    short z; /**< Quaternion z data */
};

/*!
 * @brief struct for Linear Accel data read from registers
 */
struct bno055_linear_accel_t
{
    short x; /**< Linear Accel x data */
    short y; /**< Linear Accel y data */
    short z; /**< Linear Accel z data */
};

/*!
 * @brief struct for Gravity data read from registers
 */
struct bno055_gravity_t
{
    short x; /**< Gravity x data */
    short y; /**< Gravity y data */
    short z; /**< Gravity z data */
};
/*!
 * @brief struct for Accel-output data of precision double
 */
struct bno055_accel_double_t
{
    double x; /**< Accel x double data */
    double y; /**< Accel y double data */
    double z; /**< Accel z double data */
};

/*!
 * @brief struct for Mag-output data of precision double
 */
struct bno055_mag_double_t
{
    double x; /**< Mag x double data */
    double y; /**< Mag y double data */
    double z; /**< Mag z double data */
};

/*!
 * @brief struct for Gyro-output data of precision double
 */
struct bno055_gyro_double_t
{
    double x; /**< Gyro x double data */
    double y; /**< Gyro y double data */
    double z; /**< Gyro z double data */
};

/*!
 * @brief struct for Euler-output data of precision double
 */
struct bno055_euler_double_t
{
    double h; /**< Euler h double data */
    double r; /**< Euler r double data */
    double p; /**< Euler p double data */
};

/*!
 * @brief struct for Linear Accel-output data of precision double
 */
struct bno055_linear_accel_double_t
{
    double x; /**< linear accel x double data */
    double y; /**< linear accel y double data */
    double z; /**< linear accel z double data */
};

/*!
 * @brief struct for Gravity-output data of precision double
 */
struct bno055_gravity_double_t
{
    double x; /**< Gravity x double data */
    double y; /**< Gravity y double data */
    double z; /**< Gravity z double data */
};

/*!
 * @brief struct for Accel-output data of precision float
 */
struct bno055_accel_float_t
{
    float x; /**< accel x float data */
    float y; /**< accel y float data */
    float z; /**< accel z float data */
};

/*!
 * @brief struct for Mag-output data of precision float
 */
struct bno055_mag_float_t
{
    float x; /**< Mag x float data */
    float y; /**< Mag y float data */
    float z; /**< Mag z float data */
};

/*!
 * @brief struct for Gyro-output data of precision float
 */
struct bno055_gyro_float_t
{
    float x; /**< Gyro x float data */
    float y; /**< Gyro y float data */
    float z; /**< Gyro z float data */
};

/*!
 * @brief struct for Euler-output data of precision float
 */
struct bno055_euler_float_t
{
    float h; /**< Euler h float data */
    float r; /**< Euler r float data */
    float p; /**< Euler p float data */
};

/*!
 * @brief struct for Linear accel-output data of precision float
 */
struct bno055_linear_accel_float_t
{
    float x; /**< Linear accel x float data */
    float y; /**< Linear accel y float data */
    float z; /**< Linear accel z float data */
};

/*!
 * @brief struct for Gravity-output data of precision float
 */
struct bno055_gravity_float_t
{
    float x; /**< Gravity x float data */
    float y; /**< Gravity y float data */
    float z; /**< Gravity z float data */
};

uint8_t BNO055UartIoInit(void);
void BNO055Init(void);
void ReadBNO005Acc(struct bno055_accel_t *acc);
void ReadBNO005Mag(struct bno055_mag_t *mag);
void ReadBNO005Gyro(struct bno055_gyro_t *gyro);
void ReadBNO005Euler(struct bno055_euler_t *euler);

#endif /* __BNO055_H__ */