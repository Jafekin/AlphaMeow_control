/*
 * @Author        陈佳辉 1946847867@qq.com
 * @Date          2024-06-26 13:03:09
 * @LastEditTime  2024-06-26 20:19:59
 * @Description   
 * 
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_errno.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"
#include "iot_uart.h"

#include "wifi_connect.h"
#include "wifi_device.h"
#include "lwip/netifapi.h"
#include "lwip/api_shell.h"
#include "lwip/sockets.h"

#include "bno055.h"
#include "encoder.h"
#include "motor.h"
#include "pid.h"
#include "test.h"

#define CONFIG_WIFI_SSID "_OurEDA_OurFi"
#define CONFIG_WIFI_PWD "OurEDA2021"

osMessageQueueId_t DataToSeeQueueId;
osTimerId_t EncoderTimerId;
osEventFlagsId_t MainFuncEventId;

extern float Vertical_Kp, Vertical_Kd, Velocity_Kp, Velocity_Ki, Med;
extern int integral_limit;

typedef struct
{
    struct bno055_euler_t euler_ToSee;
    int16_t Speed_Left_ToSee, Speed_Right_ToSee;
    int Vertical_out_ToSee, Velocity_out_ToSee, Turn_out_ToSee;
    int pwm_ToSee;
} DataToSee;

static void PIDTask(void);
static void UDPServerTask(void);
static void UDPClientTask(void);

void EncoderTimerCallback(void)
{
    osEventFlagsSet(MainFuncEventId, 2023);
}

static void TaskCreateAndInit(void)
{
    BNO055Init();
    EncoderInit();
    MotorPwmInit();

    IoTGpioInit(2);
    IoTGpioSetFunc(2, IOT_GPIO_FUNC_GPIO_2_GPIO);
    IoTGpioSetDir(2, IOT_GPIO_DIR_OUT);
    IoTGpioSetOutputVal(2, IOT_GPIO_VALUE1);

    if ((MainFuncEventId = osEventFlagsNew(NULL)) == NULL)
    {
        printf("Failed to create MainFuncEventFlags!\n");
    }

    if ((EncoderTimerId = osTimerNew(EncoderTimerCallback, osTimerPeriodic, NULL, NULL)) == NULL)
    {
        printf("Failed to create EncoderTimer!\n");
    }
    osTimerStart(EncoderTimerId, 1U);

    WifiConnect(CONFIG_WIFI_SSID, CONFIG_WIFI_PWD);

    osThreadAttr_t attr;

    // PIDTASK
    attr.name = "PIDTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024 * 8;
    attr.priority = 25;
    if (osThreadNew((osThreadFunc_t)PIDTask, NULL, &attr) == NULL)
    {
        printf("Failed to create PIDTask!\n");
    }

    // UDPCLIENTTASK
    attr.name = "UDPServerTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024 * 8;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)UDPServerTask, NULL, &attr) == NULL)
    {
        printf("Failed to create UDPServerTask!\n");
    }

    // UDPCLIENTTASK
    attr.name = "UDPClientTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024 * 8;
    attr.priority = 23;

    if (osThreadNew((osThreadFunc_t)UDPClientTask, NULL, &attr) == NULL)
    {
        printf("Failed to create UDPClientTask!\n");
    }
}

SYS_RUN(TaskCreateAndInit);

static void PIDTask(void)
{
    int pwm;
    struct bno055_euler_t euler;
    struct bno055_accel_t acc;
    struct bno055_gyro_t gyro;
    int16_t Encoder_Left, Encoder_Right;
    static int16_t Encoder_Left_Last = 0, Encoder_Right_Last = 0;
    int16_t Speed_Left, Speed_Right;
    int Vertical_out, Velocity_out, Turn_out;
    int Pwm_Left, Pwm_Right;

    DataToSee see;
    if ((DataToSeeQueueId = osMessageQueueNew(1, sizeof(see), NULL)) == NULL)
    {
        printf("Failed to create DataToSeeQueueId!\n");
    }

    while (1)
    {
        osEventFlagsWait(MainFuncEventId, 2023, osFlagsWaitAny, osWaitForever);

        Encoder_Left = EncoderGetLeftWheelCnt();
        Encoder_Right = EncoderGetRightWheelCnt();
        Speed_Left = Encoder_Left - Encoder_Left_Last;
        Speed_Right = Encoder_Right - Encoder_Right_Last;
        Encoder_Left_Last = Encoder_Left;
        Encoder_Right_Last = Encoder_Right;

        ReadBNO005Euler(&euler);
        ReadBNO005Acc(&acc);
        ReadBNO005Gyro(&gyro);

        Vertical_out = Vertical(euler.p, gyro.y);
        Velocity_out = Velocity(0, Speed_Left, Speed_Right);
        Turn_out = Turn(gyro.z);

        pwm = Vertical_out - Velocity_out;
        Pwm_Left = limit(pwm);
        Pwm_Right = limit(pwm);

        MotorControl(Pwm_Left, Pwm_Right);

        see.euler_ToSee = euler;
        see.Speed_Left_ToSee = Speed_Left;
        see.Speed_Right_ToSee = Speed_Right;
        see.Vertical_out_ToSee = Vertical_out;
        see.Velocity_out_ToSee = Velocity_out;
        see.Turn_out_ToSee = Turn_out;
        see.pwm_ToSee = pwm;
        if (osMessageQueuePut(DataToSeeQueueId, &see, 0U, 0U) != osOK)
        {
            printf("DataToSeeQueueId put error!\n");
        }

        osThreadYield();
    }
}

#define NATIVE_IP_ADDRESS "172.6.5.191"
#define DEVICE_IP_ADDRESS "172.6.3.110"
#define HOST_PORT (888)
#define DEVICE_PORT (777)

static void UDPServerTask(void)
{
    // sServer 进行监听，在 new_fd 接收新的链接
    int sServer = socket(AF_INET, SOCK_DGRAM, 0);
    if (sServer == -1)
    {
        printf("create server socket failed\r\n");
        close(sServer);
    }

    // 服务器的地址信息
    struct sockaddr_in serAddr = {0};
    struct sockaddr_in remoteAddr = {0};
    socklen_t ser_addr_length = sizeof(serAddr);
    socklen_t remote_addr_length = sizeof(remoteAddr);

    // 本地主机ip和端口号
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(HOST_PORT);
    serAddr.sin_addr.s_addr = inet_addr(NATIVE_IP_ADDRESS);
    if (bind(sServer, (struct sockaddr *)&serAddr, sizeof(serAddr)) == -1)
    {
        printf("bind socket failed\r\n");
        close(sServer);
    }

    // 对方ip和端口号
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons(DEVICE_PORT);
    serAddr.sin_addr.s_addr = htons(DEVICE_IP_ADDRESS);

    char stream[100] = {0};
    char recvBuf[512] = {0};

    while (1)
    {
        sprintf(stream, "\n%.2f %.2f %.2f %.2f %.2f %d\n", Vertical_Kp, Vertical_Kd, Velocity_Kp, Velocity_Ki, Med, integral_limit);
        sendto(sServer, stream, strlen(stream), 0, (struct sockaddr *)&remoteAddr, remote_addr_length);
        usleep(10);
        recvfrom(sServer, recvBuf, sizeof(recvBuf), 0, (struct sockaddr *)&remoteAddr, &remote_addr_length);

        printf("%s:%d=>%s\n", inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port), recvBuf);
        GetData(recvBuf);

        osThreadYield();
    }

    // closesocket(sServer);
}

#define CONFIG_SERVER_IP "172.6.3.110" // 要连接的服务器IP
#define CONFIG_SERVER_PORT 666         // 要连接的服务器端口

static void UDPClientTask(void)
{
    int sClient;

    // 服务器的地址信息
    struct sockaddr_in send_addr;
    socklen_t addr_length = sizeof(send_addr);

    // 创建socket
    if ((sClient = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("create socket failed!\r\n");
        return;
    }

    // 初始化预连接的服务端地址
    send_addr.sin_family = AF_INET;
    send_addr.sin_port = htons(CONFIG_SERVER_PORT);
    send_addr.sin_addr.s_addr = inet_addr(CONFIG_SERVER_IP);
    addr_length = sizeof(send_addr);

    char stream[100] = {0};
    DataToSee see;

    while (1)
    {
        if (osMessageQueueGet(DataToSeeQueueId, &see, 0U, 10) != osOK)
        {
            printf("DataToSeeQueueId get error!\n");
        }
        sprintf(stream, "channels: \t%d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d\n",
                see.euler_ToSee.h,
                see.euler_ToSee.r,
                see.euler_ToSee.p,
                see.Speed_Left_ToSee,
                see.Speed_Right_ToSee,
                see.Vertical_out_ToSee,
                see.Velocity_out_ToSee,
                see.Turn_out_ToSee,
                see.pwm_ToSee);
        sendto(sClient, stream, strlen(stream), 0, (struct sockaddr *)&send_addr, addr_length);
        osThreadYield();
    }

    closesocket(sClient);
}
