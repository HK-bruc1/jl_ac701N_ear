/**
 * @file tal_pwm.c
 * @brief This is tal_pwm file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tkl_pwm.h"
#include "tal_pwm.h"

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




TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_pwm_init(TUYA_PWM_NUM_E ch_id, TUYA_PWM_BASE_CFG_T *cfg)
{
    return tkl_pwm_init(ch_id, cfg);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_pwm_deinit(TUYA_PWM_NUM_E ch_id)
{
    return tkl_pwm_deinit(ch_id);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_pwm_start(TUYA_PWM_NUM_E ch_id)
{
    return tkl_pwm_start(ch_id);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_pwm_stop(TUYA_PWM_NUM_E ch_id)
{
    return tkl_pwm_stop(ch_id);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_pwm_duty_set(TUYA_PWM_NUM_E ch_id, UINT32_T duty)
{
    return tkl_pwm_duty_set(ch_id, duty);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_pwm_frequency_set(TUYA_PWM_NUM_E ch_id, UINT32_T frequency)
{
    return tkl_pwm_frequency_set(ch_id, frequency);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_pwm_polarity_set(TUYA_PWM_NUM_E ch_id, TUYA_PWM_POLARITY_E polarity)
{
    return tkl_pwm_polarity_set(ch_id, polarity);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_pwm_info_set(TUYA_PWM_NUM_E ch_id, CONST TUYA_PWM_BASE_CFG_T *info)
{
    return tkl_pwm_info_set(ch_id, info);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_pwm_info_get(TUYA_PWM_NUM_E ch_id, TUYA_PWM_BASE_CFG_T *info)
{
    return tkl_pwm_info_get(ch_id, info);
}

