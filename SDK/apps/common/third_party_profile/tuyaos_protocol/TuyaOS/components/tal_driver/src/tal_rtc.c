/**
 * @file tal_rtc.c
 * @brief This is tal_rtc file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tkl_rtc.h"
#include "tal_rtc.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/


/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_rtc_init(VOID_T)
{
    return tkl_rtc_init();
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_rtc_deinit(VOID_T)
{
    return tkl_rtc_deinit();
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_rtc_time_set(TIME_T time_sec)
{
    return tkl_rtc_time_set(time_sec);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_rtc_time_get(TIME_T *time_sec)
{
    return tkl_rtc_time_get(time_sec);
}

