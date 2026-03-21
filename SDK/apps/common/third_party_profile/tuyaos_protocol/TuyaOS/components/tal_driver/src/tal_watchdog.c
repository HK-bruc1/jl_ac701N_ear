/**
 * @file tal_watchdog.c
 * @brief This is tal_watchdog file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tkl_watchdog.h"
#include "tal_watchdog.h"

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




TUYA_WEAK_ATTRIBUTE UINT32_T tal_watchdog_start(TUYA_WDOG_BASE_CFG_T *cfg)
{
    return tkl_watchdog_init(cfg);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_watchdog_stop(VOID_T)
{
    return tkl_watchdog_deinit();
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_watchdog_refresh(VOID_T)
{
    return tkl_watchdog_refresh();
}

