/**
 * @file tkl_sleep.c
 * @brief This is tkl_sleep file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tkl_sleep.h"

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




OPERATE_RET tkl_cpu_sleep_callback_register(TUYA_SLEEP_CB_T *sleep_cb)
{
    return OPRT_OK;
    /* TY_PRINTF("%s", __FUNCTION__); */
    /* return OPRT_NOT_SUPPORTED; */
}

VOID_T tkl_cpu_allow_sleep(VOID_T)
{
}

VOID_T tkl_cpu_force_wakeup(VOID_T)
{
}

OPERATE_RET tkl_cpu_sleep_mode_set(BOOL_T enable, TUYA_CPU_SLEEP_MODE_E mode)
{
    return OPRT_OK;
    /* TY_PRINTF("%s", __FUNCTION__); */
    /* return OPRT_NOT_SUPPORTED; */
}

