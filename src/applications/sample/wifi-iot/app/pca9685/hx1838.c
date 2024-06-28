
/**
 *  hx1838 红外接收头
 *
 *  编码格式：NEC
 * 
 *  引导码: 9ms高 - 4.5ms低 
 *  重复码: 9ms高 - 2.25ms低 
 * 
 *  数据码0: 0.56ms高 - 0.56ms低
 *  数据码1: 0.56ms高 - 1.96ms低
 * 
 *  结束码: 0.63ms高 - 低
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

#include "hx1838.h"


static hi_gpio_value hx_vol = HX_VOL_LOW;   // 输入电压值
static hi_u64 time_sta = 0;                 // 开始时间
static hi_u64 time_end = 0;                 // 结束时间
static hi_u8 datas[4] = {0};                // 数据4组 
static hi_u8 ret_val = 0;                   // 返回值



// HX 端口初始化
hi_void Hx1838_Init(hi_void)
{
    hi_io_set_func(HX_IO_NAME, HX_IO_FUNC);
    hi_gpio_set_dir(HX_GPIO_ID, HI_GPIO_DIR_IN);
    hi_gpio_set_ouput_val(HX_GPIO_ID, HX_VOL_HIGH);
}


// 读取 信号
hi_u8 Get_Hx_Data(hi_void)
{
    ret_val = 0;

    time_sta = hi_get_us();

    hi_gpio_get_input_val(HX_GPIO_ID, &hx_vol);

    while(hx_vol == HX_VOL_LOW)
    {
        hi_gpio_get_input_val(HX_GPIO_ID, &hx_vol);
    }

    time_end = hi_get_us();

    if( (time_end - time_sta) / 1000 == 9 )
    {
        time_sta = hi_get_us();

        while(hx_vol == HX_VOL_HIGH)
        {
            hi_gpio_get_input_val(HX_GPIO_ID, &hx_vol);
        }

        time_end = hi_get_us();

        if( (time_end - time_sta) / 1000 == 4 )
        {
            // 接收数据

            memset(datas, 0, 4);

            for(hi_u8 i=0; i<4; i++)
            {
                for(hi_u8 j=0; j<8; j++)
                {
                    while(hx_vol == HX_VOL_LOW)
                    {
                        hi_gpio_get_input_val(HX_GPIO_ID, &hx_vol);
                    }

                    time_sta = hi_get_us();

                    while(hx_vol == HX_VOL_HIGH)
                    {
                        hi_gpio_get_input_val(HX_GPIO_ID, &hx_vol);
                    }

                    time_end = hi_get_us();

                    datas[i] = datas[i] << 1;

                    if( (time_end - time_sta) / 1000 == 1)
                    {
                        datas[i] = datas[i] + 1;                        
                    }
                }
            }

            // 检验数据

            if(datas[0] == 0x00 && datas[1] == 0xff)
            {
                datas[3] = ~datas[3];
                
                if(datas[2] == datas[3])
                {                    
                    ret_val = datas[2];
                }
            }
            
        }
    }

    // printf(" Ret Val = 0x%02x \n", ret_val);

    // 退出

    static hi_u8 exit = 0;

    while(1)
    {
        exit = 0;

        while(hx_vol == HX_VOL_LOW)
        {
            hi_gpio_get_input_val(HX_GPIO_ID, &hx_vol);
        }

        while(hx_vol == HX_VOL_HIGH)
        {
            if(exit > 10)
            {
                return ret_val;
            }

            exit++;
            
            hi_udelay(500);
            
            hi_gpio_get_input_val(HX_GPIO_ID, &hx_vol);
        }

    }

}

