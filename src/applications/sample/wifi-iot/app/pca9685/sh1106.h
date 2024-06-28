

#ifndef __SH1106_H__
#define __SH1106_H__

#include "hi_types_base.h"



// 初始化
hi_void SH1106_Init(hi_void);

// 清屏   
hi_void SH1106_Clear(hi_void);

// 清缓存
hi_void Buffer_Clear(hi_void);

// 缓存 显示
hi_void Buffer_Show(hi_void);

// ---------------------

// 清画布
hi_void Canvas_Clear(hi_void);

// 画线
hi_void Draw_Line(hi_s32 x1, hi_s32 y1, hi_s32 x2, hi_s32 y2);

// 画圆
hi_void Draw_Circle(hi_s32 cx, hi_s32 cy, hi_s32 cr);

// 画布 转入 缓存
hi_void Canv_Import_Buff(hi_void);

// ---------------------

// 输入字符
hi_void Input_Char_6x12(hi_u8 cx, hi_u8 cy, hi_u8 ch);

// 输入字符串
hi_void Input_String_6x12(hi_u8 sx, hi_u8 sy, hi_u8 *str, hi_u8 len);

// -------------------------------

// 输入汉字
hi_void Input_CHINESE_12x12(hi_u8 cx, hi_u8 cy, hi_u8 *cn);

// -------------------------

// 显示 1个 字符
hi_u8 Show_Char_4x8(hi_u8 x, hi_u8 y, hi_u8 c);

// 显示字符串
hi_u8 Show_String_4x8(hi_u8 sx, hi_u8 sy, hi_u8 *str, hi_u8 sl);












#endif