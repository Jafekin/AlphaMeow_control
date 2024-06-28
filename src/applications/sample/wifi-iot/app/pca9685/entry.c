
/**
 * 
 * 
 * 
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


#include "pca9685.h"
#include "hx1838.h"
#include "sh1106.h"


#define SDA_IO_NAME     HI_IO_NAME_GPIO_13
#define SDA_IO_FUNC     HI_IO_FUNC_GPIO_13_I2C0_SDA

#define SCL_IO_NAME     HI_IO_NAME_GPIO_14
#define SCL_IO_FUNC     HI_IO_FUNC_GPIO_14_I2C0_SCL

#define I2C_IDX         HI_I2C_IDX_0
#define I2C_BAUDRATE    400000


#define PCA9685_PIN0    0
#define PCA9685_PIN1    1
#define PCA9685_PIN2    2
#define PCA9685_PIN3    3



hi_bool hx_state = HI_FALSE;    // 端口状态
hi_gpio_value hx_vol = 0;       // 端口电压值
hi_u8 hx_data = 0;              // 接收数据

static hi_void Sh1106_Show(hi_u8 d);
static hi_u8 PCA9685_Control(hi_u8 con);





// // 码值对照表
// static hi_u8 code_list[17][2] = {
//     {0xa2, '1'}, {0x62, '2'}, {0xe2, '3'},  // 0 1 2
//     {0x22, '4'}, {0x02, '5'}, {0xc2, '6'},  // 3 4 5
//     {0xe0, '7'}, {0xa8, '8'}, {0x90, '9'},  // 6 7 8
//     {0x68, '*'}, {0x98, '0'}, {0xb0, '#'},  // 9 10 11
//                  {0x18, 'U'},               // 12
//     {0x10, 'L'}, {0x38, 'K'}, {0x5a, 'R'},  // 13 14 15 
//                  {0x4a, 'D'},               // 16
// };



// 红外接收端 中断
static hi_void Hx_Isr_Callback(hi_void)
{
    hx_state = HI_TRUE;
}


// I2C初始
hi_void I2C_Init(hi_void)
{
    hi_io_set_func(SDA_IO_NAME, SDA_IO_FUNC);
    hi_io_set_func(SCL_IO_NAME, SCL_IO_FUNC);
    if(hi_i2c_init(I2C_IDX, I2C_BAUDRATE) != 0)
    {
        printf(" [ I2C Init ] Failed! \n");
    }
}


static hi_u8 PCA9685_Control(hi_u8 con)
{
    hi_u8 ret = 0;

    if(con == 0x22)     // 4
    {
        PCA9685_Angle(PCA9685_PIN0, 0);
        PCA9685_Set_PWM(PCA9685_PIN0, 0, 0);
        printf(" ang = 0 \n");
        Sh1106_Show(0);
    }

    if(con == 0x02)     // 5
    {
        PCA9685_Angle(PCA9685_PIN0, 45);
        PCA9685_Set_PWM(PCA9685_PIN0, 0, 0);
        printf(" ang = 45 \n");
        Sh1106_Show(45);
    }

    if(con == 0xc2)     // 6
    {
        PCA9685_Angle(PCA9685_PIN0, 90);
        PCA9685_Set_PWM(PCA9685_PIN0, 0, 0);
        printf(" ang = 90 \n");
        Sh1106_Show(90);
    }

    if(con == 0xe0)     // 7
    {
        PCA9685_Angle(PCA9685_PIN0, 135);
        PCA9685_Set_PWM(PCA9685_PIN0, 0, 0);
        printf(" ang = 135 \n");
        Sh1106_Show(135);
    }

    if(con == 0xa8)     // 8
    {
        PCA9685_Angle(PCA9685_PIN0, 180);
        PCA9685_Set_PWM(PCA9685_PIN0, 0, 0);
        printf(" ang = 180 \n");
        Sh1106_Show(180);
    }

    if(con == 0xb0)     // #
    {
        ret = 1;
    }

    return ret;

}


static hi_void Sh1106_Show(hi_u8 d)
{
    hi_u8 s[8] = {0};

    sprintf(s, "%d", d);

    hi_u8 l = (hi_u8)strlen(s);

    printf(" d = %d, s = %s, l = %d \n", d, s, l);

    Show_String_4x8(0, 0, s, l);
}




// 主任务
static void *Task(const char *arg)
{
    (void)arg;

    printf(" [ pca9685 t5 ] \n");

    hi_watchdog_disable();

    hi_gpio_init();

    Hx1838_Init();

    // 设置中断
    hi_gpio_register_isr_function(
        HX_GPIO_ID, 
        HI_INT_TYPE_EDGE,   
        HI_GPIO_EDGE_FALL_LEVEL_LOW,
        Hx_Isr_Callback,
        HI_NULL);

    I2C_Init();

    SH1106_Clear();
    Buffer_Clear();
    Canvas_Clear();
    
    PCA9685_Init();
    PCA9685_Set_PWM_Freq(50);
    PCA9685_Set_PWM(PCA9685_PIN0, 0, 0);
    PCA9685_Set_PWM(PCA9685_PIN1, 0, 0);
    PCA9685_Set_PWM(PCA9685_PIN2, 0, 0);
    PCA9685_Set_PWM(PCA9685_PIN3, 0, 0);
    
    hi_u8 exit = 0;

    while(1)
    {
        hi_gpio_get_input_val(HX_GPIO_ID, &hx_vol);

        if(hx_state)
        {
            hi_gpio_set_isr_mask(HX_GPIO_ID, HI_TRUE);

            hx_data = Get_Hx_Data();

            if(hx_data > 0 )
            {
                exit = PCA9685_Control(hx_data);
            }else{
                printf(" GET HX DATA Falied! \n");
            }

            hx_data = 0;

            // hi_udelay(50 * 1000);

            hx_state = HI_FALSE;

            if(exit == 1)
            {
                return;
            }
            
            hi_gpio_set_isr_mask(HX_GPIO_ID, HI_FALSE);
        }

    }


    hx_state = HI_FALSE;

    PCA9685_Set_PWM(PCA9685_PIN0, 0, 0);

    printf(" [ pca9685 t5 ] End \n");


    return NULL;
}


static void Entry(void)
{
    osThreadAttr_t attr;

    attr.name = "Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 2 * 1024;
    attr.priority = 25;

    if(osThreadNew((osThreadFunc_t)Task, NULL, &attr) == NULL)
    {
        printf("Falied to create Task!\n");
    }
}


SYS_RUN(Entry);










