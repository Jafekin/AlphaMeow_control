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

osTimerId_t EncoderTimerId;
osEventFlagsId_t MainFuncEventId;

extern float Vertical_Kp, Vertical_Kd, Velocity_Kp, Velocity_Ki, Med;
int tes = 0;

static void PIDTask(void);
static void UDPClientTask(void);

void EncoderTimerCallback(void)
{
    osEventFlagsSet(MainFuncEventId, 2023);
}

static void TaskCreate(void)
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

    osThreadAttr_t attr;

    // PIDTASK
    attr.name = "PIDTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = 25;
    if (osThreadNew((osThreadFunc_t)PIDTask, NULL, &attr) == NULL)
    {
        printf("Failed to create PIDTask!\n");
    }

    // UDPCLIENTTASK
    attr.name = "UDPClientTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024 * 8;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)UDPClientTask, NULL, &attr) == NULL)
    {
        printf("Failed to create UDPClientTask!\n");
    }
}

SYS_RUN(TaskCreate);

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

        pwm = Vertical_out - Velocity_Kp * Velocity_out;
        Pwm_Left = limit(pwm);
        Pwm_Right = limit(pwm);

        printf("speed: %d,%d, %d\n", Pwm_Left, Pwm_Right, pwm);

        MotorControl(Pwm_Left, Pwm_Right);

        if (tes == 1)
            MotorForward(50);
        else if (tes == -1)
            MotorBackward(50);

        osThreadYield();
    }
}

#define CONFIG_WIFI_SSID "_OurEDA_OurFi"
#define CONFIG_WIFI_PWD "OurEDA2021"
#define DEVICE_PORT (777)
#define DEVICE_IP_ADDRESS "172.6.3.110"
#define HOST_PORT (888)
#define NATIVE_IP_ADDRESS "172.6.5.191"

static void UDPClientTask(void)
{
    // 在sock_fd 进行监听，在 new_fd 接收新的链接
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1)
    {
        printf("create server socket failed\r\n");
        close(sock_fd);
    }

    // 服务器的地址信息
    struct sockaddr_in serAddr = {0};
    struct sockaddr_in remoteAddr = {0};
    socklen_t ser_addr_length = sizeof(serAddr);
    socklen_t remote_addr_length = sizeof(remoteAddr);

    // 连接Wifi
    WifiConnect(CONFIG_WIFI_SSID, CONFIG_WIFI_PWD);

    // 本地主机ip和端口号
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(HOST_PORT);
    serAddr.sin_addr.s_addr = inet_addr(NATIVE_IP_ADDRESS);
    if (bind(sock_fd, (struct sockaddr *)&serAddr, sizeof(serAddr)) == -1)
    {
        printf("bind socket failed\r\n");
        close(sock_fd);
    }

    // 对方ip和端口号
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons(DEVICE_PORT);
    serAddr.sin_addr.s_addr = htons(DEVICE_IP_ADDRESS);

    char stream[100] = {0};
    char recvBuf[512] = {0};
    while (1)
    {
        sprintf(stream, "\n%.2f %.2f %.2f %.2f %.2f %d\n", Vertical_Kp, Vertical_Kd, Velocity_Kp, Velocity_Ki, Med, tes);
        sendto(sock_fd, stream, strlen(stream), 0, (struct sockaddr *)&remoteAddr, remote_addr_length);
        usleep(10);
        recvfrom(sock_fd, recvBuf, sizeof(recvBuf), 0, (struct sockaddr *)&remoteAddr, &remote_addr_length);

        if (recvBuf[0] == '1')
            tes = 1;
        else if (recvBuf[0] == '-' && recvBuf[1] == '1')
            tes = -1;
        else if (recvBuf[0] == '0')
            tes = 0;

        printf("%s:%d=>%s\n", inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port), recvBuf);
        GetData(recvBuf);

        osThreadYield();
    }

    closesocket(sock_fd);
}
