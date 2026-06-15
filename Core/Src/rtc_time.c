/*
 * rtc_time.c
 *
 *  Created on: 2026年4月25日
 *      Author: Administrator
 */
#include "main.h"
#include "rtc_time.h"
extern RTC_HandleTypeDef hrtc;

void RTC_Get_CurrentTime(uint8_t *hour, uint8_t *min, uint8_t *sec)
{
    RTC_TimeTypeDef sTime;
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, (RTC_DateTypeDef*)0, RTC_FORMAT_BIN);

    *hour = sTime.Hours;
    *min  = sTime.Minutes;
    *sec  = sTime.Seconds;
}

void RTC_Get_DateTime(RTC_DateTime_t *dt)
{
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    dt->year  = sDate.Year;
    dt->month = sDate.Month;
    dt->date  = sDate.Date;
    dt->hour  = sTime.Hours;
    dt->min   = sTime.Minutes;
    dt->sec   = sTime.Seconds;
}

void RTC_Set_DateTime(RTC_DateTime_t *dt)
{
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;

    sTime.Hours          = dt->hour;
    sTime.Minutes        = dt->min;
    sTime.Seconds        = dt->sec;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    sDate.Year    = dt->year;
    sDate.Month   = dt->month;
    sDate.Date    = dt->date;
    sDate.WeekDay = RTC_WEEKDAY_MONDAY;

    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

