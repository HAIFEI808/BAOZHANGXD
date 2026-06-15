#ifndef __I2S_ALARM_H
#define __I2S_ALARM_H

#include "stm32f4xx_hal.h"

/**
 * @brief 闹钟状态结构体
 */
typedef struct {
    uint8_t hour;    /* 0-23 */
    uint8_t min;     /* 0-59 */
    uint8_t enabled; /* 0=关, 1=开 */
} AlarmTime_t;

void I2S_Alarm_Init(void);
void I2S_Alarm_Start(void);
void I2S_Alarm_Stop(void);
uint8_t I2S_Alarm_IsPlaying(void);

#endif
