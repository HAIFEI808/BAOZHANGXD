/*
 * rtc_time.h
 *
 *  Created on: 2026年4月25日
 *      Author: Administrator
 */

#ifndef INC_RTC_TIME_H_
#define INC_RTC_TIME_H_

#include "main.h"

/**
 * @brief RTC 完整日期时间结构体
 * @note  year: 0-99 (2000-2099), month: 1-12, date: 1-31
 */
typedef struct {
    uint8_t year;   /* 0-99 */
    uint8_t month;  /* 1-12 */
    uint8_t date;   /* 1-31 */
    uint8_t hour;   /* 0-23 */
    uint8_t min;    /* 0-59 */
    uint8_t sec;    /* 0-59 */
} RTC_DateTime_t;

void RTC_Get_CurrentTime(uint8_t *hour, uint8_t *min, uint8_t *sec);
/** @brief 从 RTC 读取完整日期时间 */
void RTC_Get_DateTime(RTC_DateTime_t *dt);
/** @brief 写入日期时间到 RTC（用于时间校准） */
void RTC_Set_DateTime(RTC_DateTime_t *dt);

#endif /* INC_RTC_TIME_H_ */
