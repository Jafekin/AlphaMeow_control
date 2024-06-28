

#ifndef __HX1838_H__
#define __HX1838_H__


// 红外信号端 设置
#define HX_IO_NAME      HI_IO_NAME_GPIO_1
#define HX_IO_FUNC      HI_IO_FUNC_GPIO_1_GPIO
#define HX_GPIO_ID      HI_GPIO_IDX_1

// 端口电压 设置
#define HX_VOL_HIGH     HI_GPIO_VALUE1
#define HX_VOL_LOW      HI_GPIO_VALUE0

// 红外接收端口初始化
hi_void Hx1838_Init(hi_void);

// 读取 值
hi_u8 Get_Hx_Data(hi_void);


#endif



/*****************************************
 * 
 * 
        // 码值对照表  正 反
        static hi_u8 code_list[17][2] =
        {
            {0xa2, 0x5d},   // 1
            {0x62, 0x9d},   // 2
            {0xe2, 0x1d},   // 3
            {0x22, 0xdd},   // 4
            {0x02, 0xfd},   // 5
            {0xc2, 0x3d},   // 6
            {0xe0, 0x1f},   // 7
            {0xa8, 0x57},   // 8
            {0x90, 0x6f},   // 9
            {0x98, 0x67},   // 0
            {0x68, 0x97},   // *
            {0xb0, 0x4f},   // #
            {0x18, 0xe7},   // up
            {0x4a, 0xb5},   // down
            {0x10, 0xef},   // left
            {0x5a, 0xa5},   // right
            {0x38, 0xc7},   // OK
        };
 * 
 *
 *****************************************/





