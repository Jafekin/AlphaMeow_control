/*
 * @Author        陈佳辉 1946847867@qq.com
 * @Date          2024-07-09 17:52:37
 * @LastEditTime  2024-07-09 17:55:08
 * @Description
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <math.h>
#include <stdlib.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_errno.h"
#include "iot_gpio.h"
#include "iot_adc.h"
#include "iot_gpio_ex.h"
#include "iot_uart.h"
#include "iot_i2c.h"
#include "iot_i2c_ex.h"
#include "hi_i2c.h"
#include "hi_adc.h"

#include "wifi_connect.h"
#include "wifi_device.h"
#include "lwip/netifapi.h"
#include "lwip/api_shell.h"
#include "lwip/sockets.h"

#define NATIVE_IP_ADDRESS "192.168.145.5"
#define DEVICE_IP_ADDRESS "192.168.145.245"
#define HOST_PORT (666)
#define DEVICE_PORT (777)

extern int flag;

static void UDPServerTask2(void)
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
        flag = -1;

        recvfrom(sServer, recvBuf, sizeof(recvBuf), 0, (struct sockaddr *)&remoteAddr, &remote_addr_length);
        printf("%s:%d=>%s\n", inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port), recvBuf);
        ChangeData(recvBuf);

        if (flag == 4)
        {
            sprintf(stream, "Ack!\n");
            sendto(sServer, stream, strlen(stream), 0, (struct sockaddr *)&remoteAddr, remote_addr_length);
        }

        usleep(10);
        osThreadYield();
    }
    closesocket(sServer);
}

static void TaskCreateAndInit2(void)
{
    osThreadAttr_t attr;

    attr.name = "UDPServerTask2";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024 * 8;
    attr.priority = 24;
    if (osThreadNew((osThreadFunc_t)UDPServerTask2, NULL, &attr) == NULL)
    {
        printf("[TaskCreateAndInit2] Failed to create UDPServerTask2!\n");
    }
}

SYS_RUN(TaskCreateAndInit2);