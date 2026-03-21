/**
 * @file tal_utc.h
 * @brief This is tal_utc file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TAL_UTC_H__
#define __TAL_UTC_H__

#include "tuya_cloud_types.h"
#include "tuya_error_code.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/


/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
#pragma pack(1)
typedef struct {
    UINT16_T year;
    UINT8_T  month;
    UINT8_T  day;
    UINT8_T  hour;
    UINT8_T  min;
    UINT8_T  sec;
    UINT8_T  dayIndex; /* 0 = Sunday */
} tal_utc_date_t;
#pragma pack()

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/

/**
 * @brief tal_utc_is_leap_year
 *
 * @param[in] year: year
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tal_utc_is_leap_year(UINT16_T year);

/**
 * @brief tal_utc_get_days_of_month
 *
 * @param[in] month: month
 * @param[in] year: year
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tal_utc_get_days_of_month(UINT8_T month, UINT16_T year);

/**
 * @brief //Zeller formula
 *        after  1582.10.4    w = (y + y/4 + c/4 - 2*c + (26*(m+1))/10 + d - 1) % 7
 *        before 1582.10.4    w = (y + y/4 + c/4 -2*c + (13*(m+1))/5 + d + 2) % 7;
 *        //Kim larsen calculation formula
 *        W = (d+2m+3(m+1)/5+y+y/4-y/100+y/400+1)%7
 * @param[in] year: year
 * @param[in] month: month
 * @param[in] day: day
 *
 * @return 0-Sunday  6-Saturday
 */
UINT8_T tal_utc_date2dayindex(UINT16_T year, UINT8_T month, UINT8_T day);

/**
 * @brief tal_utc_timestamp2date
 *
 * @param[in] timestamp: timestamp
 * @param[in] date: date
 * @param[in] daylightSaving: daylightSaving
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_utc_timestamp2date(UINT32_T timestamp, tal_utc_date_t *date, BOOL_T daylightSaving);

/**
 * @brief tal_utc_timestamp2datestring
 *
 * @param[in] timestamp: timestamp
 * @param[in] dateStr: dateStr
 * @param[in] daylightSaving: daylightSaving
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_utc_timestamp2datestring(UINT32_T timestamp, CHAR_T *dateStr, BOOL_T daylightSaving);

/**
 * @brief tal_utc_date2timestamp
 *
 * @param[in] *date: *date
 * @param[in] daylightSaving: daylightSaving
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tal_utc_date2timestamp(tal_utc_date_t *date, BOOL_T daylightSaving);

/**
 * @brief tal_utc_set_time_zone
 *
 * @param[in] time_zone: time_zone
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_utc_set_time_zone(INT16_T time_zone);

/**
 * @brief tal_utc_get_time_zone
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
INT16_T tal_utc_get_time_zone(VOID_T);

/**
 * @brief tal_utc_get_local_time
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tal_utc_get_local_time(VOID_T);

/**
 * @brief tal_utc_test
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tal_utc_test(VOID_T);


#ifdef __cplusplus
}
#endif

#endif /* __TAL_UTC_H__ */

