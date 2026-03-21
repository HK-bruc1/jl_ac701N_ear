/**
 * @file tuya_ble_main.h
 * @brief This is tuya_ble_main file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_MAIN_H__
#define __TUYA_BLE_MAIN_H__

#include "tuya_cloud_types.h"
#include "tuya_ble_type.h"
#include "tuya_ble_internal_config.h"

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
extern tal_common_info_t tal_common_info;
extern tuya_ble_parameters_settings_t tuya_ble_current_para;

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/

/**
 * @brief tuya_ble_connect_status_set
 *
 * @param[in] status: status
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_connect_status_set(tuya_ble_connect_status_t status);

/**
 * @brief tuya_ble_connect_status_get
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_connect_status_t tuya_ble_connect_status_get(VOID_T);

/**
 * @brief tuya_ble_connect_source_type_set
 *
 * @param[in] source_type: source_type
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
void tuya_ble_connect_source_type_set(tuya_ble_connect_source_type_t source_type);

/**
 * @brief tuya_ble_connect_source_type_get
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_connect_source_type_t tuya_ble_connect_source_type_get(void);

#if TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE

/**
 * @brief tuya_ble_link_status_set
 *
 * @param[in] status: status
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_link_status_set(tuya_ble_link_status_t status);

/**
 * @brief tuya_ble_link_status_get
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_link_status_t tuya_ble_link_status_get(VOID_T);

#endif

/**
 * @brief tuya_ble_event_init
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_event_init(VOID_T);

/**
 * @brief tuya_ble_event_send
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tuya_ble_event_send(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_cb_event_send
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tuya_ble_cb_event_send(tuya_ble_cb_evt_param_t *evt);

/**
 * @brief tuya_ble_get_adv_connect_request_bit_status
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tuya_ble_get_adv_connect_request_bit_status(VOID_T);

/**
 * @brief tuya_ble_secure_connection_type
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tuya_ble_secure_connection_type(VOID_T);

/**
 * @brief tuya_ble_adv_change
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_adv_change(VOID_T);

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN >= 3)

/**
 * @brief tuya_ble_adv_change_with_connecting_request
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_adv_change_with_connecting_request(VOID_T);

#endif

/**
 * @brief tuya_ble_inter_event_response
 *
 * @param[in] *param: *param
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_inter_event_response(tuya_ble_cb_evt_param_t *param);

/**
 * @brief tuya_ble_connect_monitor_timer_init
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_connect_monitor_timer_init(VOID_T);

/**
 * @brief tuya_ble_connect_monitor_timer_start
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_connect_monitor_timer_start(VOID_T);

/**
 * @brief tuya_ble_connect_monitor_timer_stop
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_connect_monitor_timer_stop(VOID_T);

/**
 * @brief tal_common_info_init
 *
 * @param[in] p_tal_common_info: p_tal_common_info
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_common_info_init(tal_common_info_t *p_tal_common_info);


#ifdef __cplusplus
}
#endif

#endif /* __TUYA_BLE_MAIN_H__ */

