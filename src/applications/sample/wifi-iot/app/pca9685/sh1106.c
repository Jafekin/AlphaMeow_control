
/**
 *  sh1106 v2
 * 
 *  23.12.10
 * 
 * 
 * 
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "ohos_init.h"
#include "cmsis_os2.h"

#include "hi_types_base.h" 
#include "hi_errno.h"

#include "hi_io.h"
#include "hi_gpio.h"

#include "hi_i2c.h"

#include "hi_time.h"
#include "hi_watchdog.h"


#include "sh1106.h"
#include "ascii_char.h"

#define SH1106_I2C_IDX  HI_I2C_IDX_0

#define SH1106_ADDR 0x78            // 0x78 设备地址

#define W_CMD 0x00                  // 写入指令
#define W_DAT 0x40                  // 写入数据

static hi_u8 buffer[8][128] = {0};  // 显示缓存
static hi_u8 canvas[64][128] = {0}; // 画布



// 写入 1条 指令, addr 地址, data 数据
static hi_void SH1106_I2C_Write(hi_u8 addr, hi_u8 data)
{
    hi_u8 data_send_buf[2] = {addr, data};
    hi_i2c_data sh1106_i2c_data = {0};
    sh1106_i2c_data.send_buf = data_send_buf;
    sh1106_i2c_data.send_len = 2;
    if(hi_i2c_write(SH1106_I2C_IDX, SH1106_ADDR, &sh1106_i2c_data) != 0)
    {
        printf(" [SH1106 I2C Write] Failed! \n");
    }
}


// 初始化
hi_void SH1106_Init(hi_void)
{
    SH1106_I2C_Write(W_CMD, 0xAE);
    SH1106_I2C_Write(W_CMD, 0x02);
    SH1106_I2C_Write(W_CMD, 0x10);
    SH1106_I2C_Write(W_CMD, 0x40);
    SH1106_I2C_Write(W_CMD, 0xB0);
    SH1106_I2C_Write(W_CMD, 0x81);
    SH1106_I2C_Write(W_CMD, 0xCF);
    SH1106_I2C_Write(W_CMD, 0xA1);
    SH1106_I2C_Write(W_CMD, 0xC8);
    SH1106_I2C_Write(W_CMD, 0xA6);
    SH1106_I2C_Write(W_CMD, 0xA8);
    SH1106_I2C_Write(W_CMD, 0x3F);
    SH1106_I2C_Write(W_CMD, 0xAD);
    SH1106_I2C_Write(W_CMD, 0x8B);
    SH1106_I2C_Write(W_CMD, 0x33);
    SH1106_I2C_Write(W_CMD, 0xC8);
    SH1106_I2C_Write(W_CMD, 0xD3);
    SH1106_I2C_Write(W_CMD, 0x00);
    SH1106_I2C_Write(W_CMD, 0xD5);
    SH1106_I2C_Write(W_CMD, 0x80);
    SH1106_I2C_Write(W_CMD, 0xD9);
    SH1106_I2C_Write(W_CMD, 0xF1);
    SH1106_I2C_Write(W_CMD, 0xDA);
    SH1106_I2C_Write(W_CMD, 0x12);
    SH1106_I2C_Write(W_CMD, 0xDB);
    SH1106_I2C_Write(W_CMD, 0x40);
    SH1106_I2C_Write(W_CMD, 0xA4);
    SH1106_I2C_Write(W_CMD, 0xAF);
    hi_udelay(50*1000);  
}


/************************************/


// 清屏
hi_void SH1106_Clear(hi_void)
{
    for(hi_u8 i = 0; i < 8; i++)  
    {
        SH1106_I2C_Write (W_CMD, 0xB0 + i);
        SH1106_I2C_Write (W_CMD, 0x02);
        SH1106_I2C_Write (W_CMD, 0x10);
        for(hi_u8 j = 0; j < 128; j++)
        {
            SH1106_I2C_Write(W_DAT, 0x00);
        }
    }
}


// 清缓存
hi_void Buffer_Clear(hi_void)
{
    for(hi_u8 i = 0; i < 8; i++)  
    {
        for(hi_u8 j = 0; j < 128; j++)
        {
            buffer[i][j] = 0x00;
        }
    }
}


// 缓存 显示
hi_void Buffer_Show(hi_void)
{
    for(hi_u8 i = 0; i < 8; i++)  
    {
        SH1106_I2C_Write (W_CMD, 0xB0 + i);
        SH1106_I2C_Write (W_CMD, 0x02);
        SH1106_I2C_Write (W_CMD, 0x10);
        for(hi_u8 j = 0; j < 128; j++)
        {
            SH1106_I2C_Write(W_DAT, buffer[i][j]);
        }
    }
}


/******************** 画线 画圆 *********************/


// 清画布
hi_void Canvas_Clear(hi_void)
{
    for(hi_u8 i = 0; i < 64; i++)  
    {
        for(hi_u8 j = 0; j < 128; j++)
        {
            canvas[i][j] = 0x00;
        }
    }
}

// 交换
static hi_void Swap(hi_s32 *a, hi_s32 *b)
{
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

// 画线  线两端 A点 B点 坐标, 可以任意A点B点 x: 0-127 y: 0-63
hi_void Draw_Line(hi_s32 x1, hi_s32 y1, hi_s32 x2, hi_s32 y2)
{
    hi_s32 w = abs(x2 - x1);
    hi_s32 h = abs(y2 - y1);

    hi_bool b = 0;

    if(h > w)
    {
        b = 1;
        Swap(&x1, &y1);
        Swap(&x2, &y2);
        Swap(&w, &h);
    }

    hi_s32 ix = 1;

    if (x1 > x2)
    {
        ix = -1;
    }

    hi_s32 iy = 1;

    if (y1 > y2)
    {
        iy = -1;
    }

    hi_s32 x = x1, y = y1;
    hi_s32 d = h * 2 - w;
    hi_s32 m = h * 2;
    hi_s32 n = (h - w) * 2;

    while (x != x2)
    {
        if (d < 0)
        {
            d += m;
        } else {
            d += n;
            y += iy;
        }

        if (b)
        {
            canvas[x][y] = 1;
        } else {
            canvas[y][x] = 1;
        }

        x += ix;
    }
}


// 画圆 圆心坐标 圆半径 注意圆不要越界
hi_void Draw_Circle(hi_s32 cx, hi_s32 cy, hi_s32 cr)
{
    hi_s32 x = 0;
    hi_s32 y = cr;
    hi_s32 d = 3 - 2 * cr;

    while (x <= y)
    {
        canvas[cy - y][cx + x] = 1;
        canvas[cy - x][cx + y] = 1;
        canvas[cy + x][cx + y] = 1;
        canvas[cy + y][cx + x] = 1;
        canvas[cy + y][cx - x] = 1;
        canvas[cy + x][cx - y] = 1;
        canvas[cy - x][cx - y] = 1;
        canvas[cy - y][cx - x] = 1;

        if (d < 0)
        {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }

        x++;
    }
}


// 画布 转入 缓存
hi_void Canv_Import_Buff(hi_void)
{
    hi_u8 p;

    for (hi_u8 h = 0; h < 8; h++)
    {
        for (hi_u8 w = 0; w < 128; w++)
        {
            p = 0;

            p = ( p | canvas[h * 8 + 7][w] ) << 1;
            p = ( p | canvas[h * 8 + 6][w] ) << 1;
            p = ( p | canvas[h * 8 + 5][w] ) << 1;
            p = ( p | canvas[h * 8 + 4][w] ) << 1;
            p = ( p | canvas[h * 8 + 3][w] ) << 1;
            p = ( p | canvas[h * 8 + 2][w] ) << 1;
            p = ( p | canvas[h * 8 + 1][w] ) << 1;
            p = p | canvas[h * 8 + 0][w];

            buffer[h][w] |= p;
        }
    }
}


/************* 6x12 字符  ***************/


// 输入字符
hi_void Input_Char_6x12(hi_u8 cx, hi_u8 cy, hi_u8 ch)
{
    hi_u8  n = ch - 32;

    hi_u8 l1 = 0x00;
    hi_u8 l2 = 0x00;
    hi_u8 l3 = 0x00;

    hi_u8 ny = cy / 8;
    hi_u8 ly = cy % 8;

    for(hi_u8 i=0; i<6; i++)
    {
        l1 = 0x00;
        l2 = 0x00;
        l3 = 0x00;

        l1 |= char_6x12[n][i] << ly;
        l2 |= char_6x12[n][i] >> (8-ly);
        l2 |= char_6x12[n][i+6] << ly;
        l3 |= char_6x12[n][i+6] >> (8-ly);

        if(ny < 8)
        {
            buffer[ny][cx+i] |= l1;
        }

        if((ny+1) < 8)
        {
            buffer[ny+1][cx+i] |= l2;
        }

        if((ny+2) < 8)
        {
            buffer[ny+2][cx+i] |= l3;
        }
    }
}


// 输入字符串
hi_void Input_String_6x12(hi_u8 sx, hi_u8 sy, hi_u8 *str, hi_u8 len)
{
    for(hi_u8 i=0; i<len; i++)
    {
        if(sx > 122)
        {
            sx = 0;
            sy += 12;
        }

        if(sy < 52)
        {
            Input_Char_6x12(sx, sy, str[i]);
            sx += 6;
        }
    }
}


/********************* 12x12 **************/


// 输入汉字
hi_void Input_CHINESE_12x12(hi_u8 cx, hi_u8 cy, hi_u8 *cn)
{
    hi_u8 l1 = 0x00;
    hi_u8 l2 = 0x00;
    hi_u8 l3 = 0x00;

    hi_u8 ny = cy / 8;
    hi_u8 ly = cy % 8;

    for(hi_u8 i=0; i<12; i++)
    {
        l1 = 0x00;
        l2 = 0x00;
        l3 = 0x00;

        l1 |= cn[i] << ly;
        l2 |= cn[i] >> (8-ly);
        l2 |= cn[i+12] << ly;
        l3 |= cn[i+12] >> (8-ly);

        if(ny < 8)
        {
            buffer[ny][cx+i] |= l1;
        }

        if((ny+1) < 8)
        {
            buffer[ny+1][cx+i] |= l2;
        }

        if((ny+2) < 8)
        {
            buffer[ny+2][cx+i] |= l3;
        }
    }
}


/************* 4x8 *******************/


// 设置坐标 
static hi_void SH1106_Set_Pos(hi_u8 x, hi_u8 y) 
{
    x += 2;
    SH1106_I2C_Write(W_CMD, 0xB0 + y);
    SH1106_I2C_Write(W_CMD, (x & 0x0F));
    SH1106_I2C_Write(W_CMD, ((x & 0xF0) >> 4) | 0x10);
}


// 显示 1个 字符
hi_u8 Show_Char_4x8(hi_u8 x, hi_u8 y, hi_u8 c)
{
    hi_u8  n = c - 32;

    SH1106_Set_Pos(x, y);

    for(hi_u8 i = 0; i < 6; i++)
    {
        SH1106_I2C_Write(W_DAT, char_6x8[n][i]);
    }

}

// 显示字符串
hi_u8 Show_String_4x8(hi_u8 sx, hi_u8 sy, hi_u8 *str, hi_u8 sl)
{
    hi_u8 x = sx;
    hi_u8 y = sy;
    hi_u8 n = 0;

    SH1106_Set_Pos(x, y);

    for(hi_u8 i=0; i<sl; i++)
    {
        n = str[i] - 32;

        if(x > 122)
        {
            x = 0;
            y += 1;

            SH1106_Set_Pos(x, y);
        }

        for(hi_u8 j = 0; j < 6; j++)
        {
            SH1106_I2C_Write(W_DAT, char_6x8[n][j]);
        }
        
        x += 6;
    }
}









