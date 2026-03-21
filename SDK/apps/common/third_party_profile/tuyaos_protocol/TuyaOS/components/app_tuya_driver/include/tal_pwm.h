/**
 * @file tal_pwm.h
 * @brief This is tal_pwm file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TAL_PWM_H__
#define __TAL_PWM_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

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

/**
 * @brief pwm init
 *
 * @param[in] ch_id: pwm channal id,id index starts from 0
 * @param[in] cfg: pwm config
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_pwm_init(TUYA_PWM_NUM_E ch_id, TUYA_PWM_BASE_CFG_T *cfg);

/**
 * @brief pwm deinit
 *
 * @param[in] ch_id: pwm channal id,id index starts from 0
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_pwm_deinit(TUYA_PWM_NUM_E ch_id);

/**
 * @brief pwm start
 *
 * @param[in] ch_id: pwm channal id,id index starts from 0
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_pwm_start(TUYA_PWM_NUM_E ch_id);

/**
 * @brief pwm stop
 *
 * @param[in] ch_id: pwm channal id,id index starts from 0
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_pwm_stop(TUYA_PWM_NUM_E ch_id);

/**
 * @brief pwm duty set
 *
 * @param[in] ch_id: pwm channal id, id index starts from 0
 * @param[in] duty:  pwm duty cycle
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_pwm_duty_set(TUYA_PWM_NUM_E ch_id, UINT32_T duty);

/**
 * @brief pwm frequency set
 *
 * @param[in] ch_id: pwm channal id, id index starts from 0
 * @param[in] frequency: pwm frequency
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_pwm_frequency_set(TUYA_PWM_NUM_E ch_id, UINT32_T frequency);

/**
 * @brief pwm polarity set
 *
 * @param[in] ch_id: pwm channal id, id index starts from 0
 * @param[in] polarity: pwm polarity
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_pwm_polarity_set(TUYA_PWM_NUM_E ch_id, TUYA_PWM_POLARITY_E polarity);

/**
 * @brief set pwm info
 *
 * @param[in] ch_id: pwm channal id,id index starts from 0
 * @param[in] info: pwm info
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_pwm_info_set(TUYA_PWM_NUM_E ch_id, CONST TUYA_PWM_BASE_CFG_T *info);

/**
 * @brief get pwm info
 *
 * @param[in] ch_id: pwm channal id,id index starts from 0
 * @param[out] info: pwm info
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_pwm_info_get(TUYA_PWM_NUM_E ch_id, TUYA_PWM_BASE_CFG_T *info);


#ifdef __cplusplus
}
#endif

#endif /* __TAL_PWM_H__ */
