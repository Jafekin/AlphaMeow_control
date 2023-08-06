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

osMessageQueueId_t EulerQueueId;

static void UARTTask(void);
static void UDPClientTask(void);

static void TaskCreate(void)
{
    osThreadAttr_t attr;

    // UARTTASK
    attr.name = "UARTTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = 25;
    if (osThreadNew((osThreadFunc_t)UARTTask, NULL, &attr) == NULL)
    {
        printf("Failed to create UARTTask!\n");
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
        printf("[UDPClientDemo] Failed to create UDPClientTask!\n");
    }
}

SYS_RUN(TaskCreate);

static void UARTTask(void)
{
    BNO055Init();
    struct bno055_euler_t euler;
    EulerQueueId = osMessageQueueNew(1, sizeof(euler), NULL);
    IoTGpioInit(2);
    IoTGpioSetFunc(2, IOT_GPIO_FUNC_GPIO_2_GPIO);
    IoTGpioSetDir(2, IOT_GPIO_DIR_OUT);
    IoTGpioSetOutputVal(2, IOT_GPIO_VALUE1);
    while (1)
    {
        ReadBNO005Euler(&euler);
        int ret = osMessageQueuePut(EulerQueueId, &euler, 0U, 0U);
        printf("1:%d\n", ret);
        osThreadYield();
    }
}

#define CONFIG_WIFI_SSID "_OurEDA_OurFi" // 要连接的WiFi 热点账号
#define CONFIG_WIFI_PWD "OurEDA2021"     // 要连接的WiFi 热点密码
#define CONFIG_SERVER_IP "172.6.3.110"   // 要连接的服务器IP
#define CONFIG_SERVER_PORT 777           // 要连接的服务器端口

static void UDPClientTask(void)
{
    // 在sock_fd 进行监听，在 new_fd 接收新的链接
    int sock_fd;

    // 服务器的地址信息
    struct sockaddr_in send_addr;
    socklen_t addr_length = sizeof(send_addr);

    // 连接Wifi
    WifiConnect(CONFIG_WIFI_SSID, CONFIG_WIFI_PWD);

    // 创建socket
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
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
    char recvBuf[512] = {0};
    struct bno055_euler_t euler;
    while (1)
    {
        int ret = osMessageQueueGet(EulerQueueId, &euler, 0U, 10);
        printf("2:%d\n", ret);
        sprintf(stream, "channels: %d,%d,%d\n", euler.h, euler.r, euler.p);
        sendto(sock_fd, stream, strlen(stream), 0, (struct sockaddr *)&send_addr, addr_length);
        osThreadYield();
    }

    closesocket(sock_fd);
}
