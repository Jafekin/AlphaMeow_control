/*
 * @Author        陈佳辉 1946847867@qq.com
 * @Date          2024-06-26 13:12:34
 * @LastEditTime  2024-07-10 07:52:11
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
int pwm[5] = {};

int action[101][5] = {
    {180, 80, 20, 50, 164},   // 1*0
    {171, 29, 70, 17, 142},   // 1*1!
    {175, 29, 55, 15, 142},   // 1*2!
    {176, 32, 47, 13, 143},   // 1*3!
    {177, 35, 39, 13, 144},   // 1*4!
    {176, 42, 36, 17, 149},   // 1*5!
    {175, 49, 37, 16, 149},   // 1*6!
    {175, 59, 41, 13, 155},   // 1*7!
    {175, 70, 46, 17, 155},   // 1*8!
    {172, 78, 49, 18, 155},   // 1*9!
    {180, 80, 20, 50, 164},   // 2*0
    {161, 29, 73, 21, 142},   // 2*1!
    {165, 29, 60, 19, 142},   // 2*2!
    {169, 35, 56, 19, 140},   // 2*3!
    {175, 46, 58, 19, 142},   // 2*4!
    {175, 51, 54, 23, 145},   // 2*5!
    {178, 63, 58, 23, 145},   // 2*6!
    {178, 71, 58, 23, 145},   // 2*7!
    {178, 82, 61, 23, 145},   // 2*8!
    {155, 60, 43, 23, 151},   // 2*9!
    {180, 80, 20, 50, 164},   // 3*0
    {157, 36, 83, 27, 145},   // 3*1!
    {173, 59, 93, 26, 145},   // 3*2!
    {179, 70, 94, 28, 145},   // 3*3!
    {170, 54, 72, 27, 146},   // 3*4!
    {175, 65, 73, 28, 145},   // 3*5!
    {178, 76, 75, 28, 145},   // 3*6!
    {180, 88, 78, 28, 145},   // 3*7!
    {179, 93, 77, 28, 145},   // 3*8!
    {167, 88, 70, 30, 154},   // 3*9!
    {180, 80, 20, 50, 164},   // 4*0
    {164, 65, 110, 30, 151},  // 4*1!
    {167, 63, 99, 31, 145},   // 4*2!
    {174, 72, 99, 31, 145},   // 4*3!
    {180, 85, 100, 31, 145},  // 4*4!
    {182, 91, 98, 31, 145},   // 4*5!
    {175, 84, 87, 32, 145},   // 4*6!
    {180, 100, 92, 32, 145},  // 4*7!
    {177, 103, 90, 32, 150},  // 4*8!
    {173, 109, 90, 33, 151},  // 4*9!
    {180, 80, 20, 50, 164},   // 5*0
    {161, 71, 117, 34, 147},  // 5*1!
    {162, 71, 109, 34, 147},  // 5*2!
    {169, 79, 107, 33, 147},  // 5*3!
    {174, 90, 108, 33, 147},  // 5*4!
    {178, 100, 108, 33, 147}, // 5*5!
    {176, 99, 101, 33, 153},  // 5*6!
    {176, 108, 102, 33, 153}, // 5*7!
    {176, 116, 103, 33, 153}, // 5*8!
    {179, 136, 111, 33, 156}, // 5*9!
    {180, 80, 20, 50, 164},   // 6*0
    {150, 75, 123, 37, 147},  // 6*1!
    {147, 67, 112, 36, 147},  // 6*2!
    {156, 78, 112, 37, 147},  // 6*3!
    {153, 76, 105, 37, 156},  // 6*4!
    {157, 86, 106, 37, 156},  // 6*5!
    {157, 89, 102, 37, 156},  // 6*6!
    {159, 98, 104, 37, 156},  // 6*7
    {158, 104, 104, 37, 156}, // 6*8
    {158, 115, 108, 37, 156}, // 6*9
    {180, 80, 20, 50, 164},   // 7*0
    {148, 89, 134, 39, 152},  // 7*1
    {152, 90, 129, 39, 152},  // 7*2!
    {155, 93, 125, 39, 152},  // 7*3!
    {161, 103, 125, 38, 152}, // 7*4
    {163, 110, 124, 39, 152}, // 7*5!#
    {165, 119, 125, 38, 151}, // 7*6
    {165, 128, 126, 37, 155}, // 7*7!
    {165, 136, 127, 39, 155}, // 7*8!
    {155, 132, 123, 38, 165}, // 7*9!#
    {180, 80, 20, 50, 164},   // 8*0
    {140, 98, 143, 37, 152},  // 8*1
    {144, 95, 135, 37, 152},  // 8*2
    {154, 111, 139, 37, 160}, // 8*3
    {158, 121, 139, 37, 158}, // 8*4!
    {161, 128, 139, 37, 152}, // 8*5!
    {163, 137, 140, 37, 152}, // 8*6!
    {164, 146, 141, 37, 152}, // 8*7!
    {158, 144, 138, 37, 152}, // 8*8!
    {154, 150, 140, 39, 157}, // 8*9
    {180, 80, 20, 50, 164},   // 9*0
    {131, 101, 148, 41, 153}, // 9*1!
    {143, 119, 151, 41, 163}, // 9*2
    {149, 123, 149, 43, 155}, // 9*3
    {147, 125, 146, 42, 160}, // 9*4!
    {155, 144, 153, 41, 155}, // 9*5!
    {152, 145, 150, 41, 162}, // 9*6!
    {146, 139, 143, 41, 164}, // 9*7!
    {147, 150, 148, 42, 158}, // 9*8
    {132, 137, 140, 41, 158}, // 9*9!#
    {180, 80, 20, 50, 164},   // 10*0
    {125, 118, 161, 41, 162}, // 10*1
    {128, 121, 158, 41, 162}, // 10*2!
    {134, 127, 157, 41, 164}, // 10*3
    {140, 141, 160, 41, 164}, // 10*4!
    {139, 145, 160, 41, 161}, // 10*5
    {137, 150, 160, 41, 162}, // 10*6
    {131, 142, 152, 40, 162}, // 10*7!
    {127, 146, 153, 40, 162}, // 10*8!
    {115, 150, 157, 40, 162}, // 10*9!
};

// todo:调参
int adjust_x = 4;
int adjust_y = 1;
int mode = 2;
int line = 3;
int flag = -1;
bool drop = false;

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
    msleep(2000);
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
    printf("[MechanicalArmDown]: %d %d %d %d %d %d %d %d\n", x, y, ord, action[ord][0], action[ord][1],
           action[ord][2], action[ord][3], action[ord][4]);

    int pre_pwm[5] = {0};
    int max_steps = 0;

    for (int i = 0; i < 3; i++)
    {
        pre_pwm[i] = pwm[i];
        int difference = action[ord][i] - pre_pwm[i];
        int steps = abs(difference);
        if (steps > max_steps)
        {
            max_steps = steps;
        }
    }
    for (int step = 0; step < max_steps; step++)
    {
        for (int i = 0; i < 3; i++)
        {
            int difference = action[ord][i] - pre_pwm[i];
            int total_steps = abs(difference);
            if (step < total_steps)
            {
                int direction = difference / total_steps;
                pwm[i] += direction;
                PCA9685_Angle(i + 2, pwm[i]);
            }
        }
        msleep(25);
    }

    for (int i = 0; i < 3; i++)
    {
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
            msleep(50);
        }

        pwm[i] = action[ord][i];
        PCA9685_Angle(i + 2, pwm[i]);
    }
}

static void MechanicalArmUp(void)
{
    int pre_pwm[5] = {0};
    for (int i = 3; i < 5; i++)
    {
        pre_pwm[i] = pwm[i];
        int difference = action[0][i] - pre_pwm[i];
        int step = difference / abs(difference);

        for (int j = 0; j < abs(difference); j++)
        {
            pwm[i] += step;
            PCA9685_Angle(i + 2, pwm[i]);
            msleep(50);
        }

        pwm[i] = action[0][i];
        PCA9685_Angle(i + 2, pwm[i]);
    }
    msleep(500);
}

static void MechanicalArmGetBack(void)
{
    int ord = 0;
    int pre_pwm[5] = {0};
    int max_steps = 0;

    for (int i = 0; i < 3; i++)
    {
        pre_pwm[i] = pwm[i];
        int difference = action[ord][i] - pre_pwm[i];
        int steps = abs(difference);
        if (steps > max_steps)
        {
            max_steps = steps;
        }
    }
    for (int step = 0; step < max_steps; step++)
    {
        for (int i = 0; i < 3; i++)
        {
            int difference = action[ord][i] - pre_pwm[i];
            int total_steps = abs(difference);
            if (step < total_steps)
            {
                int direction = difference / total_steps;
                pwm[i] += direction;
                PCA9685_Angle(i + 2, pwm[i]);
            }
        }
        msleep(25);
    }

    for (int i = 0; i < 3; i++)
    {
        pwm[i] = action[ord][i];
        PCA9685_Angle(i + 2, pwm[i]);
    }
    msleep(500);
}

void ChangeData(char *stream)
{
    if (stream[0] == '@')
    {
        flag = 1;
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
        MechanicalArmDown(adjust_x, adjust_y);
        PumpSuckUp();
        MechanicalArmUp();
        MechanicalArmGetBack();
        PumpPutDown();
        MechanicalArmDown(adjust_x, adjust_y);
    }
    else if (stream[0] == '#')
    {
        flag = 2;
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
        MechanicalArmDown(adjust_x, adjust_y);
        PumpSuckUp();
        MechanicalArmUp();
        MechanicalArmGetBack();
        PumpPutDown();
        MechanicalArmDown(adjust_x, adjust_y);
    }
    else if (stream[0] == 'm')
    {
        flag = 3;
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
        line = data[0];
        mode = data[1];
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
    else if (stream[0] == '%')
    {
        drop = false;
        int data[4] = {0};
        stream++;
        char *token = strtok(stream, ", ");
        int i = 0;

        while (token != NULL && i < 4)
        {
            data[i] = atoi(token);
            i++;
            token = strtok(NULL, ", ");
        }
        if (data[2] == 1 && data[3] == 0)
            drop = true;
        MechanicalArmDown(data[0], data[1]);
        PumpSuckUp();
        MechanicalArmUp();
        MechanicalArmDown(data[2], data[3]);
        PCA9685_Angle(PUMP_CHANNEL, 0);
        PCA9685_Angle(SOLENOID_VALVE_CHANNEL, 180);
        MechanicalArmUp();
        PumpPutDown();
        ResetPwm();
        printf("%%: %d %d %d %d\n", data[0], data[1], data[2], data[3]);
    }
    // if (flag != 4 && flag != -1)
    //     osSemaphoreRelease(adjustSemaphore);
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

        // // tag
        // if (mode == 1)
        // {
        //     for (int i = 1; i < 10; i++)
        //     {
        //         MechanicalArmDown(line, i);
        //         PumpSuckUp();
        //         MechanicalArmUp();
        //         MechanicalArmGetBack();
        //         PumpPutDown();
        //         sleep(2);
        //     }
        // }
        // else if (mode == 2)
        // {
        //     MechanicalArmDown(adjust_x, adjust_y);
        //     PumpSuckUp();
        //     MechanicalArmUp();
        //     MechanicalArmGetBack();
        //     PumpPutDown();
        //     MechanicalArmDown(adjust_x, adjust_y);
        // }

        // osSemaphoreAcquire(adjustSemaphore, HI_SYS_WAIT_FOREVER);
        // ResetPwm();

        sleep(1);
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

// #define CONFIG_WIFI_SSID "_OurEDA_OurFi"
// #define CONFIG_WIFI_PWD "OurEDA2021"
// #define NATIVE_IP_ADDRESS "172.6.1.118"
// #define DEVICE_IP_ADDRESS "172.6.1.148"
// #define HOST_PORT (888)
// #define DEVICE_PORT (777)

#define CONFIG_WIFI_SSID "Wintoki"
#define CONFIG_WIFI_PWD "1234567890"
#define NATIVE_IP_ADDRESS "192.168.164.4"
#define DEVICE_IP_ADDRESS "192.168.164.239"
#define HOST_PORT (888)
#define DEVICE_PORT (45841)

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
        flag = -1;

        recvfrom(sServer, recvBuf, sizeof(recvBuf), 0, (struct sockaddr *)&remoteAddr, &remote_addr_length);
        printf("%s:%d=>%s\n", inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port), recvBuf);
        ChangeData(recvBuf);
        memset_s(recvBuf, 512 * sizeof(char), 0, 512 * sizeof(char));

        // if (!drop)
        // {
        sprintf(stream, "Ack!\n");
        sendto(sServer, stream, strlen(stream), 0, (struct sockaddr *)&remoteAddr, remote_addr_length);
        printf("%s:%d<=%s\n", inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port), stream);
        // }

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
