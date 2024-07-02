/*
 * @Author        陈佳辉 1946847867@qq.com
 * @Date          2024-06-26 13:12:34
 * @LastEditTime  2024-07-03 00:23:53
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

#include "pca9685.h"
#include "oled_ssd1306.h"

#define BUTTON_GPIO (5)

#define IOT_I2C0_SDA_GPIO (13)
#define IOT_I2C0_SCL_GPIO (14)

#define SOLENOID_VALVE_CHANNEL (0)
#define PUMP_CHANNEL (1)

#define ACTUATOR_CHANNEL_1 (2)
#define ACTUATOR_CHANNEL_2 (3)
#define ACTUATOR_CHANNEL_3 (4)
#define ACTUATOR_CHANNEL_4 (5)
#define ACTUATOR_CHANNEL_5 (6)

osSemaphoreId_t adjustSemaphore;
osTimerId_t keyTimerId;

int keySet = 0;
int actuatorNo = 2;
int pwm[5] = {171, 30, 5, 86, 160};

int action[91][5] = {
    {171, 30, 5, 86, 160},   // 1*0
    {169, 0, 84, 70, 147},   // 1*1
    {178, 7, 80, 71, 141},   // 1*2
    {177, 5, 65, 70, 144},   // 1*3
    {178, 10, 62, 71, 149},  // 1*4
    {180, 16, 58, 70, 144},  // 1*5
    {170, 12, 47, 71, 157},  // 1*6
    {164, 13, 44, 71, 160},  // 1*7
    {157, 12, 40, 71, 159},  // 1*8
    {162, 28, 49, 72, 158},  // 1*9
    {171, 30, 5, 86, 160},   // 2*0
    {164, 4, 90, 72, 146},   // 2*1
    {173, 12, 89, 72, 145},  // 2*2
    {170, 10, 77, 74, 152},  // 2*3
    {173, 16, 72, 74, 155},  // 2*4
    {173, 18, 67, 74, 155},  // 2*5
    {172, 22, 64, 74, 155},  // 2*6
    {173, 31, 65, 74, 155},  // 2*7
    {167, 32, 61, 74, 162},  // 2*8
    {168, 44, 67, 75, 162},  // 2*9
    {171, 30, 5, 86, 160},   // 3*0
    {171, 27, 113, 78, 149}, // 3*1
    {174, 31, 107, 77, 152}, // 3*2
    {177, 34, 100, 77, 152}, // 3*3
    {180, 40, 99, 78, 152},  // 3*4
    {180, 44, 95, 77, 156},  // 3*5
    {167, 30, 77, 77, 156},  // 3*6
    {170, 38, 77, 78, 156},  // 3*7
    {167, 44, 77, 78, 158},  // 3*8
    {163, 48, 77, 78, 158},  // 3*9
    {171, 30, 5, 86, 160},   // 4*0
    {161, 27, 115, 78, 149}, // 4*1
    {165, 29, 109, 79, 149}, // 4*2
    {164, 28, 100, 78, 149}, // 4*3
    {168, 33, 96, 78, 154},  // 4*4
    {175, 48, 100, 78, 156}, // 4*5
    {175, 53, 98, 78, 156},  // 4*6
    {171, 55, 95, 78, 156},  // 4*7
    {171, 63, 96, 78, 156},  // 4*8
    {165, 64, 93, 78, 156},  // 4*9
    {171, 30, 5, 86, 160},   // 5*0
    {166, 46, 130, 80, 154}, // 5*1
    {164, 43, 121, 80, 154}, // 5*2
    {164, 42, 114, 80, 154}, // 5*3
    {157, 33, 101, 80, 154}, // 5*4
    {161, 42, 101, 80, 154}, // 5*5
    {156, 39, 95, 81, 154},  // 5*6
    {166, 61, 103, 81, 154}, // 5*7
    {168, 72, 107, 81, 154}, // 5*8
    {154, 56, 94, 81, 156},  // 5*9
};
// todo:调参
int adjust_x = 5;
int adjust_y = 8;
int mode = 1;
int line = 4;

static float GetVoltage(void)
{
    unsigned int ret;
    unsigned short data;

    if ((ret = IoTAdcRead(HI_ADC_CHANNEL_2, &data, IOT_ADC_EQU_MODEL_8, IOT_ADC_CUR_BAIS_DEFAULT, 0xA)) != 0)
    {
        printf("[GetVoltage] ADC Read Fail: 0x%x\n", ret);
    }

    return (float)data * 1.8 * 4 / 4096.0;
}

static void KeyTimerCallback(char *arg)
{
    (void)arg;
    static int pre_keySet = 2;
    float key = GetVoltage();
    if (key > 2.0f)
        keySet = 0;
    else if (key > 0.85f)
    {
        keySet = 2;
        if (keySet != pre_keySet)
        {
            pwm[actuatorNo - 2]++;
            if (pwm[actuatorNo - 2] > 180)
                pwm[actuatorNo - 2] = 180;
            else if (pwm[actuatorNo - 2] < 0)
                pwm[actuatorNo - 2] = 0;
        }
    }
    else if (key > 0.5f)
    {
        keySet = 1;
        if (keySet != pre_keySet)
        {
            pwm[actuatorNo - 2]--;
            if (pwm[actuatorNo - 2] > 180)
                pwm[actuatorNo - 2] = 180;
            else if (pwm[actuatorNo - 2] < 0)
                pwm[actuatorNo - 2] = 0;
        }
    }
    else if (key > 0.0f)
    {
        keySet = 3;
        if (keySet != pre_keySet)
        {
            actuatorNo++;
            if (actuatorNo > 6)
                actuatorNo -= 5;
            else if (actuatorNo < 2)
                actuatorNo += 5;
        }
    }
    // printf("[KeyTimerCallback] %d -> %d (%f)\n", pre_keySet, keySet, key);
    pre_keySet = keySet;
}

static void PumpSuckUp(void)
{
    printf("[PumpSuckUp]: Start!\n");
    PCA9685_Angle(SOLENOID_VALVE_CHANNEL, 0);
    PCA9685_Angle(PUMP_CHANNEL, 180);
    msleep(2000);
    PCA9685_Angle(PUMP_CHANNEL, 0);
    printf("[PumpSuckUp]: End!\n");
}

static void PumpPutDown(void)
{
    printf("[PumpPutDown]: Start!\n");
    PCA9685_Angle(PUMP_CHANNEL, 0);
    PCA9685_Angle(SOLENOID_VALVE_CHANNEL, 180);
    msleep(1000);
    PCA9685_Angle(SOLENOID_VALVE_CHANNEL, 0);
    printf("[PumpPutDown]: End!\n");
}

static void ResetPwm(void)
{
    PCA9685_Angle(SOLENOID_VALVE_CHANNEL, 0);
    PCA9685_Angle(PUMP_CHANNEL, 0);

    PCA9685_Angle(ACTUATOR_CHANNEL_1, action[0][ACTUATOR_CHANNEL_1 - 2]);
    PCA9685_Angle(ACTUATOR_CHANNEL_2, action[0][ACTUATOR_CHANNEL_2 - 2]);
    PCA9685_Angle(ACTUATOR_CHANNEL_3, action[0][ACTUATOR_CHANNEL_3 - 2]);
    PCA9685_Angle(ACTUATOR_CHANNEL_4, action[0][ACTUATOR_CHANNEL_4 - 2]);
    PCA9685_Angle(ACTUATOR_CHANNEL_5, action[0][ACTUATOR_CHANNEL_5 - 2]);

    for (int i = 0; i < 5; i++)
    {
        pwm[i] = action[0][i];
    }
    sleep(2);
}

static void MechanicalArmDown(int x, int y)
{
    int ord = (x - 1) * 10 + y;
    int pre_pwm[4] = {0};
    for (int i = 0; i < 3; i++)
    {
        pre_pwm[i] = pwm[i];
        int difference = action[ord][i] - pre_pwm[i];
        int step = difference / abs(difference);

        for (int j = 0; j < abs(difference); j++)
        {
            pwm[i] += step;
            PCA9685_Angle(i + 2, pwm[i]);
            msleep(30);
        }

        pwm[i] = action[ord][i];
        PCA9685_Angle(i + 2, pwm[i]);
    }
    msleep(1000);
    for (int i = 4; i > 2; i--)
    {
        pre_pwm[i] = pwm[i];
        int difference = action[ord][i] - pre_pwm[i];
        int step = difference / abs(difference);

        for (int j = 0; j < abs(difference); j++)
        {
            pwm[i] += step;
            PCA9685_Angle(i + 2, pwm[i]);
            msleep(200);
        }

        pwm[i] = action[ord][i];
        PCA9685_Angle(i + 2, pwm[i]);
    }
}

static void MechanicalArmUp(void)
{
    int pre_pwm[4] = {0};
    for (int i = 3; i < 5; i++)
    {
        pre_pwm[i] = pwm[i];
        int difference = action[0][i] - pre_pwm[i];
        int step = difference / abs(difference);

        for (int j = 0; j < abs(difference); j++)
        {
            pwm[i] += step;
            PCA9685_Angle(i + 2, pwm[i]);
            msleep(200);
        }

        pwm[i] = action[0][i];
        PCA9685_Angle(i + 2, pwm[i]);
    }
    msleep(500);
}

static void MechanicalArmGetBack(void)
{
    int ord = 0;
    int pre_pwm[4] = {0};
    for (int i = 0; i < 3; i++)
    {
        pre_pwm[i] = pwm[i];
        int difference = action[ord][i] - pre_pwm[i];
        int step = difference / abs(difference);

        for (int j = 0; j < abs(difference); j++)
        {
            pwm[i] += step;
            PCA9685_Angle(i + 2, pwm[i]);
            msleep(25);
        }

        pwm[i] = action[ord][i];
        PCA9685_Angle(i + 2, pwm[i]);
    }
    msleep(500);
}

void ChangeData(char *stream)
{
    if (stream[0] == '@')
    {
        int data[5] = {0};
        stream++;
        char *token = strtok(stream, ", ");
        int i = 0;

        while (token != NULL && i < 5)
        {
            data[i] = atoi(token);
            i++;
            token = strtok(NULL, ", ");
        }
        int ord = (adjust_x - 1) * 10 + adjust_y;
        printf("@: ");
        for (int i = 0; i < 5; i++)
        {
            action[ord][i] = data[i];
            printf("%d ", data[i]);
        }
        printf("\n");
    }
    else if (stream[0] == '#')
    {
        int data[2] = {0};
        stream++;
        char *token = strtok(stream, ", ");
        int i = 0;

        while (token != NULL && i < 2)
        {
            data[i] = atoi(token);
            i++;
            token = strtok(NULL, ", ");
        }
        adjust_x = data[0];
        adjust_y = data[1];
        printf("#: %d %d\n", data[0], data[1]);
    }
    else if (stream[0] == 'm')
    {
        int data[2] = {0};
        stream++;
        char *token = strtok(stream, ", ");
        int i = 0;

        while (token != NULL && i < 2)
        {
            data[i] = atoi(token);
            i++;
            token = strtok(NULL, ", ");
        }
        mode = data[0];
        line = data[1];
    }
    osSemaphoreRelease(adjustSemaphore);
}

static void ControlTask(void)
{
    unsigned int ret = 0;

    IoTGpioInit(IOT_I2C0_SDA_GPIO);
    IoTGpioSetFunc(IOT_I2C0_SDA_GPIO, IOT_GPIO_FUNC_GPIO_13_I2C0_SDA);

    IoTGpioInit(IOT_I2C0_SCL_GPIO);
    IoTGpioSetFunc(IOT_I2C0_SCL_GPIO, IOT_GPIO_FUNC_GPIO_14_I2C0_SCL);

    if ((ret = IoTI2cInit(HI_I2C_IDX_0, 400000)) != 0)
    {
        printf(" [IoTI2c0Init] Failed!: 0x%x \n", ret);
    }

    PCA9685_Init();
    PCA9685_Set_PWM_Freq(50);
    ResetPwm();

    while (1)
    {
        printf("[ControlTask]: begin!\n");

        // tag
        if (mode == 1)
        {
            for (int i = 1; i < 10; i++)
            {
                MechanicalArmDown(line, i);
                PumpSuckUp();
                MechanicalArmUp();
                MechanicalArmGetBack();
                PumpPutDown();
                sleep(2);
            }
        }
        else if (mode == 2)
        {
            MechanicalArmDown(adjust_x, adjust_y);
            PumpSuckUp();
            MechanicalArmUp();
            MechanicalArmGetBack();
            PumpPutDown();
            MechanicalArmDown(adjust_x, adjust_y);
        }

        osSemaphoreAcquire(adjustSemaphore, HI_SYS_WAIT_FOREVER);
        ResetPwm();
    }
}

static void AdjustTask(void)
{
    IoTGpioInit(BUTTON_GPIO);
    IoTGpioSetFunc(BUTTON_GPIO, IOT_GPIO_FUNC_GPIO_5_GPIO);
    IoTGpioSetDir(BUTTON_GPIO, IOT_GPIO_DIR_IN);
    // IoTGpioSetPull(BUTTON_GPIO, IOT_GPIO_PULL_UP);
    // IoTGpioRegisterIsrFunc(BUTTON_GPIO, IOT_INT_TYPE_EDGE, IOT_GPIO_EDGE_FALL_LEVEL_LOW, F1Pressed, NULL);

    OledInit();
    OledFillScreen(0);

    char show[100] = {0};
    char recvBuf[512] = {0};

    while (1)
    {
        PCA9685_Angle(actuatorNo, pwm[actuatorNo - 2]);

        sprintf(show, "Actuator No: %d", actuatorNo);
        OledShowString(0, 0, show, FONT6_X8);
        sprintf(show, "Pwm: %d %d", pwm[0], pwm[1]);
        OledShowString(0, 1, show, FONT6_X8);
        sprintf(show, "%d %d %d", pwm[2], pwm[3], pwm[4]);
        OledShowString(0, 3, show, FONT6_X8);

        // printf("Actuator No: %d\n", actuatorNo);
        // printf("Pwm: %d\n", pwm);
        msleep(10);
    }
}

#define CONFIG_WIFI_SSID "_OurEDA_OurFi"
#define CONFIG_WIFI_PWD "OurEDA2021"
#define NATIVE_IP_ADDRESS "172.6.1.118"
#define DEVICE_IP_ADDRESS "172.6.1.148"
#define HOST_PORT (888)
#define DEVICE_PORT (777)

static void UDPServerTask(void)
{
    WifiConnect(CONFIG_WIFI_SSID, CONFIG_WIFI_PWD);

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
        // sprintf(stream, "linked!\n");
        // sendto(sServer, stream, strlen(stream), 0, (struct sockaddr *)&remoteAddr, remote_addr_length);

        recvfrom(sServer, recvBuf, sizeof(recvBuf), 0, (struct sockaddr *)&remoteAddr, &remote_addr_length);
        printf("%s:%d=>%s\n", inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port), recvBuf);
        ChangeData(recvBuf);

        usleep(10);
        osThreadYield();
    }

    closesocket(sServer);
}

static void TaskCreateAndInit(void)
{
    if ((keyTimerId = osTimerNew(KeyTimerCallback, osTimerPeriodic, NULL, NULL)) == NULL)
    {
        printf("[TaskCreateAndInit] Failed to create keyTimerId!\n");
    }
    osTimerStart(keyTimerId, 1U);

    if ((adjustSemaphore = osSemaphoreNew(1, 0, NULL)) == NULL)
    {
        printf("[TaskCreateAndInit] Failed to create adjustMutex!\n");
    }

    osThreadAttr_t attr;

    attr.name = "ControlTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024 * 8;
    attr.priority = 25;
    if (osThreadNew((osThreadFunc_t)ControlTask, NULL, &attr) == NULL)
    {
        printf("[TaskCreateAndInit] Failed to create ControlTask!\n");
    }

    attr.name = "AdjustTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024 * 8;
    attr.priority = 24;
    if (osThreadNew((osThreadFunc_t)AdjustTask, NULL, &attr) == NULL)
    {
        printf("[TaskCreateAndInit] Failed to create AdjustTask!\n");
    }

    attr.name = "UDPServerTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024 * 8;
    attr.priority = 24;
    if (osThreadNew((osThreadFunc_t)UDPServerTask, NULL, &attr) == NULL)
    {
        printf("[TaskCreateAndInit] Failed to create UDPServerTask!\n");
    }
}

SYS_RUN(TaskCreateAndInit);
