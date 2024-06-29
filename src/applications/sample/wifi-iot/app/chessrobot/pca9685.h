/*
 * @Author        陈佳辉 1946847867@qq.com
 * @Date          2024-06-27 14:04:30
 * @LastEditTime  2024-06-28 23:14:52
 * @Description
 *
 */
#ifndef __PCA9685_H__
#define __PCA9685_H__

#include "hi_types_base.h"
#include "hi_i2c.h"

#define PCA9685_I2C_IDX HI_I2C_IDX_0 // i2c设备名

#define PCA9685_READ_ADDR 0x81  // 读
#define PCA9685_WRITE_ADDR 0x80 // 写

#define PCA9685_MODE1 0x00     // 模式寄存器1
#define PCA9685_PRE_SCALE 0xFE // 输出预分频器频率

#define PCA9685_LED0_ON_L 0x06
#define PCA9685_LED0_ON_H 0x07
#define PCA9685_LED0_OFF_L 0x08
#define PCA9685_LED0_OFF_H 0x09

// 初始化
hi_void PCA9685_Init(hi_void);

// 重置
hi_u8 PCA9685_Reset(hi_void);

// 设置PWM频率
hi_u8 PCA9685_Set_PWM_Freq(hi_u8 freq);

// 设置PWM 占空比
hi_void PCA9685_Set_PWM(hi_u8 num, hi_u16 on, hi_u16 off);

// 设置 角度
hi_void PCA9685_Angle(hi_u8 num, hi_u8 ang);

#endif