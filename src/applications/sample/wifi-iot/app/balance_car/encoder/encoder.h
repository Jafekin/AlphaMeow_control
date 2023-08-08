/*
 * @Author        陈佳辉 1946847867@qq.com
 * @Date          2023-08-06 19:57:57
 * @LastEditTime  2023-08-07 18:07:59
 * @Description
 *
 */

#ifndef __ENCODER_H__
#define __ENCODER_H__

void EncoderInit(void);
void EncoderCallback(char *arg);
int16_t EncoderGetRightWheelCnt(void);
int16_t EncoderGetLeftWheelCnt(void);
void EncoderGetWheelCnt(int16_t *LeftCnt, int16_t *RightCnt);

#endif /* __ENCODER_H__ */