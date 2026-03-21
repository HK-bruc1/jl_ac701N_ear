/**
 * @file tkl_pwm.c
 * @brief This is tkl_pwm file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "vendor/jl_ble/tuyaos/include/board.h"
#include "tkl_pwm.h"

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




OPERATE_RET tkl_pwm_init(TUYA_PWM_NUM_E ch_id, CONST TUYA_PWM_BASE_CFG_T *cfg)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_pwm_deinit(TUYA_PWM_NUM_E ch_id)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_pwm_start(TUYA_PWM_NUM_E ch_id)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_pwm_stop(TUYA_PWM_NUM_E ch_id)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_pwm_duty_set(TUYA_PWM_NUM_E ch_id, UINT32_T duty)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_pwm_frequency_set(TUYA_PWM_NUM_E ch_id, UINT32_T frequency)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_pwm_polarity_set(TUYA_PWM_NUM_E ch_id, TUYA_PWM_POLARITY_E polarity)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_pwm_info_set(TUYA_PWM_NUM_E ch_id, CONST TUYA_PWM_BASE_CFG_T *info)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_pwm_info_get(TUYA_PWM_NUM_E ch_id, TUYA_PWM_BASE_CFG_T *info)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_pwm_ioctl(TUYA_PWM_NUM_E ch_id, UINT32_T cmd, VOID *arg)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

