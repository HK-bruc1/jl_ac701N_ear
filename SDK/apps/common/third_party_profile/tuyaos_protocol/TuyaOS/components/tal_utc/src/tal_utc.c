/**
 * @file tal_utc.c
 * @brief This is tal_utc file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tal_memory.h"
#include "tal_rtc.h"
#include "tal_utc.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define UTC_BASE_YEAR       1970
#define MONTH_PER_YEAR      12
#define DAY_PER_YEAR        365
#define SEC_PER_DAY         86400
#define SEC_PER_HOUR        3600
#define SEC_PER_MIN         60

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
/* Number of days per month */
STATIC CONST UINT8_T sg_day_per_mon[MONTH_PER_YEAR] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
STATIC INT16_T sg_time_zone = 0; //100 times of the actual time zone, for example, Beijing East District 8 is 8x100=800, west district 7.5 is -750

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




UINT8_T tal_utc_is_leap_year(UINT16_T year)
{
    if ((year % 400) == 0) {
        return 1;
    } else if ((year % 100) == 0) {
        return 0;
    } else if ((year % 4) == 0) {
        return 1;
    } else {
        return 0;
    }
}

UINT8_T tal_utc_get_days_of_month(UINT8_T month, UINT16_T year)
{
    if ((month == 0) || (month > 12)) {
        return sg_day_per_mon[1] + tal_utc_is_leap_year(year);
    }

    if (month != 2) {
        return sg_day_per_mon[month - 1];
    } else {
        return sg_day_per_mon[1] + tal_utc_is_leap_year(year);
    }
}

UINT8_T tal_utc_date2dayindex(UINT16_T year, UINT8_T month, UINT8_T day)
{
    //Zeller formula
    INT8_T century_code, year_code, month_code, day_code;
    INT32_T week = 0;

    century_code = year_code = month_code = day_code = 0;

    if (month == 1 || month == 2) {
        century_code = (year - 1) / 100;
        year_code = (year - 1) % 100;
        month_code = month + 12;
        day_code = day;
    } else {
        century_code = year / 100;
        year_code = year % 100;
        month_code = month;
        day_code = day;
    }

    week = year_code + year_code / 4 + century_code / 4 - 2 * century_code + 26 * (month_code + 1) / 10 + day_code - 1;
    week = week > 0 ? (week % 7) : ((week % 7) + 7);

//    //Kim larsen calculation formula
//    UINT8_T week = (day + 2*month + 3*(month+1)/5 + year + year/4 - year/100 + year/400 + 1)%7;

    return week;
}

OPERATE_RET tal_utc_timestamp2date(UINT32_T timestamp, tal_utc_date_t *date, BOOL_T daylightSaving)
{
    INT32_T sec, day;
    UINT16_T y;
    UINT8_T  m;
    UINT16_T d;

    if (daylightSaving) {
        timestamp += SEC_PER_HOUR;
    }

    /* hour, min, sec */
    /* hour */
    sec = timestamp % SEC_PER_DAY; //Less than the number of seconds in a day
    date->hour = sec / SEC_PER_HOUR;

    /* min */
    sec = sec % SEC_PER_HOUR; //Less than the number of seconds in an hour
    date->min = sec / SEC_PER_MIN;

    /* sec */
    sec = sec % SEC_PER_MIN; //Less than the number of seconds in a minute
    date->sec = sec;


    /* year, month, day */
    /* year */
    day = timestamp / SEC_PER_DAY;
    for (y = UTC_BASE_YEAR; day > 0; y++) {
        d = (DAY_PER_YEAR + tal_utc_is_leap_year(y));
        if (day >= d) {
            day -= d;
        } else {
            break;
        }
    }
    date->year = y;

    /* month */
    for (m = 1; m < MONTH_PER_YEAR; m++) {
        d = tal_utc_get_days_of_month(m, y);
        if (day >= d) {
            day -= d;
        } else {
            break;
        }
    }
    date->month = m;

    /* day */
    date->day = (UINT8_T)(day + 1);

    /* dayindix */
    date->dayIndex = tal_utc_date2dayindex(date->year, date->month, date->day);

    return OPRT_OK;
}

OPERATE_RET tal_utc_timestamp2datestring(UINT32_T timestamp, CHAR_T *dateStr, BOOL_T daylightSaving)
{
    tal_utc_date_t date = {0};
    UINT32_T temp = 0;

    tal_utc_timestamp2date(timestamp, &date, daylightSaving);

    temp = date.year;
    dateStr[0] = temp / 1000 + 0x30;
    temp = date.year % 1000;
    dateStr[1] = temp / 100 + 0x30;
    temp = temp % 100;
    dateStr[2]  = temp / 10 + 0x30;
    dateStr[3]  = temp % 10 + 0x30;
    dateStr[4]  = '-';
    dateStr[5]  = date.month / 10 + 0x30;
    dateStr[6]  = date.month % 10 + 0x30;
    dateStr[7]  = '-';
    dateStr[8]  = date.day / 10 + 0x30;
    dateStr[9]  = date.day % 10 + 0x30;
    dateStr[10] = ' ';
    dateStr[11] = date.hour / 10 + 0x30;
    dateStr[12] = date.hour % 10 + 0x30;
    dateStr[13] = ':';
    dateStr[14] = date.min / 10 + 0x30;
    dateStr[15] = date.min % 10 + 0x30;
    dateStr[16] = ':';
    dateStr[17] = date.sec / 10 + 0x30;
    dateStr[18] = date.sec % 10 + 0x30;
    dateStr[19] = '\0';

    return OPRT_OK;
}

UINT32_T tal_utc_date2timestamp(tal_utc_date_t *date, BOOL_T daylightSaving)
{
    UINT16_T idx;
    UINT32_T day = 0;
    UINT32_T timestamp;

    if (date->year < UTC_BASE_YEAR) {
        return 0;
    }

    /* year */
    for (idx = UTC_BASE_YEAR; idx < date->year; idx++) {
        day += (DAY_PER_YEAR + tal_utc_is_leap_year(idx));
    }

    /* month */
    for (idx = 1; idx < date->month; idx++) {
        day += tal_utc_get_days_of_month((UINT8_T)idx, date->year);
    }

    /* day */
    day += (date->day - 1);

    /* sec */
    timestamp = (UINT32_T)(day * SEC_PER_DAY) + (UINT32_T)(date->hour * SEC_PER_HOUR + date->min * SEC_PER_MIN + date->sec);

    if (daylightSaving) {
        timestamp -= SEC_PER_HOUR;
    }

    return timestamp;
}

OPERATE_RET tal_utc_set_time_zone(INT16_T time_zone)
{
    sg_time_zone = time_zone;
    return OPRT_OK;
}

INT16_T tal_utc_get_time_zone(VOID_T)
{
    return sg_time_zone;
}

UINT32_T tal_utc_get_local_time(VOID_T)
{
    UINT32_T timestamp = 0;
    tal_rtc_time_get(&timestamp);

    UINT32_T local_time = timestamp + sg_time_zone * 36;
    return local_time;
}

VOID_T tal_utc_test(VOID_T)
{
    UINT8_T week = tal_utc_date2dayindex(2020, 10, 22); //week = 4


    tal_utc_date_t data = {0};
    tal_utc_timestamp2date(1603353201, &data, false); //data = 2020/10/22 7:53:21 //Note: online tool may be converted to Beijing time, so it will be 8h more
    UINT32_T timestamp = tal_utc_date2timestamp(&data, false); //timestamp = 1603353201


    CHAR_T dateStr[20];
    tal_utc_timestamp2datestring(1603353201, dateStr, false); //dateStr = "2020-10-22 07:53:21"
}

