/*
 * @Author        陈佳辉 1946847867@qq.com
 * @Date          2023-08-14 22:00:49
 * @LastEditTime  2023-08-18 13:21:06
 * @Description
 *
 */
#include "test.h"

float GetData(uint8_t *DataBuff)
{
    uint8_t data_Start_Num = 0;       // 记录数据位开始的地方
    uint8_t data_End_Num = 0;         // 记录数据位结束的地方
    uint8_t data_Num = 0;             // 记录数据位数
    uint8_t minus_Flag = 0;           // 判断是不是负数
    float data_return = 0;            // 解析得到的数据
    for (uint8_t i = 0; i < 200; i++) // 查找等号和感叹号的位置
    {
        if (DataBuff[i] == '=')
            data_Start_Num = i + 1; // +1是直接定位到数据起始位
        if (DataBuff[i] == '#')
        {
            data_End_Num = i - 1;
            break;
        }
    }
    if (DataBuff[data_Start_Num] == '-') // 如果是负数
    {
        data_Start_Num += 1; // 后移一位到数据位
        minus_Flag = 1;      // 负数flag
    }
    data_Num = data_End_Num - data_Start_Num + 1;
    // printf("%d %d %d %d\r\n", data_Start_Num, data_End_Num, data_Num, minus_Flag);
    if (data_Num == 4) // 数据共4位
    {
        data_return = (DataBuff[data_Start_Num] - 48) + (DataBuff[data_Start_Num + 2] - 48) * 0.1f +
                      (DataBuff[data_Start_Num + 3] - 48) * 0.01f;
    }
    else if (data_Num == 5) // 数据共5位
    {
        data_return = (DataBuff[data_Start_Num] - 48) * 10 + (DataBuff[data_Start_Num + 1] - 48) + (DataBuff[data_Start_Num + 3] - 48) * 0.1f +
                      (DataBuff[data_Start_Num + 4] - 48) * 0.01f;
    }
    else if (data_Num == 6) // 数据共6位
    {
        data_return = (DataBuff[data_Start_Num] - 48) * 100 + (DataBuff[data_Start_Num + 1] - 48) * 10 + (DataBuff[data_Start_Num + 2] - 48) +
                      (DataBuff[data_Start_Num + 4] - 48) * 0.1f + (DataBuff[data_Start_Num + 5] - 48) * 0.01f;
    }
    if (minus_Flag == 1)
        data_return = -data_return;

    if (DataBuff[0] == 'P' && DataBuff[1] == '1') // 位置环P
        Vertical_Kp = data_return;
    else if (DataBuff[0] == 'D' && DataBuff[1] == '1') // 位置环D
        Vertical_Kd = data_return;
    else if (DataBuff[0] == 'P' && DataBuff[1] == '2') // 速度环P
        Velocity_Kp = data_return;
    else if (DataBuff[0] == 'I' && DataBuff[1] == '2') // 速度环I
        Velocity_Ki = data_return;
    // else if ((DataBuff[0] == 'S' && DataBuff[1] == 'p') && DataBuff[2] == 'e') // 目标速度
    //     L_Target_Speed = data_return;
    else if ((DataBuff[0] == 'M' && DataBuff[1] == 'e') && DataBuff[2] == 'd') // 目标位置
        Med = data_return;
    printf("data=%.2f\r\n", data_return);
    return data_return;
}
