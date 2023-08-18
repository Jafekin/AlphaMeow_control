#ifndef __TEST_H__
#define __TEST_H__

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>

extern float Vertical_Kp, Vertical_Kd, Velocity_Kp, Velocity_Ki, Med;

float GetData(uint8_t *DataBuff);

#endif /* __TEST_H__ */