/**
 * @file tkl_watchdog.c
 * @brief This is tkl_watchdog file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tkl_watchdog.h"
#include "vendor/jl_ble/tuyaos/include/board.h"

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




UINT32_T tkl_watchdog_init(TUYA_WDOG_BASE_CFG_T *cfg)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_watchdog_deinit(VOID_T)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_watchdog_refresh(VOID_T)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

