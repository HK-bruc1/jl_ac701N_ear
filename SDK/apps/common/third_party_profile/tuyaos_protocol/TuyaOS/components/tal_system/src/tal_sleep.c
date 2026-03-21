/**
 * @file tal_sleep.c
 * @brief This is tal_sleep file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tkl_sleep.h"
#include "tal_sleep.h"

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




TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_cpu_sleep_callback_register(TUYA_SLEEP_CB_T *sleep_cb)
{
    return tkl_cpu_sleep_callback_register(sleep_cb);
}

TUYA_WEAK_ATTRIBUTE VOID_T tal_cpu_allow_sleep(VOID_T)
{
    tkl_cpu_allow_sleep();
}

TUYA_WEAK_ATTRIBUTE VOID_T tal_cpu_force_wakeup(VOID_T)
{
    tkl_cpu_force_wakeup();
}

TUYA_WEAK_ATTRIBUTE VOID_T tal_cpu_set_lp_mode(BOOL_T lp_enable)
{
    return;
}

TUYA_WEAK_ATTRIBUTE BOOL_T tal_cpu_get_lp_mode(VOID_T)
{
    return OPRT_NOT_SUPPORTED;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_cpu_lp_enable(VOID_T)
{
    return OPRT_NOT_SUPPORTED;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_cpu_lp_disable(VOID_T)
{
    return OPRT_NOT_SUPPORTED;
}

