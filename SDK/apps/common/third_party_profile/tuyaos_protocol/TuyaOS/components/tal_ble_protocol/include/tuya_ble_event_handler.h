/**
 * @file tuya_ble_event_handler.h
 * @brief This is tuya_ble_event_handler file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_EVENT_HANDLER_H__
#define __TUYA_BLE_EVENT_HANDLER_H__

#include "tuya_cloud_types.h"
#include "tuya_ble_type.h"

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
 * @brief tuya_ble_handle_device_info_update_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_device_info_update_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_dp_data_reported_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_dp_data_reported_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_dp_data_with_time_reported_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_dp_data_with_time_reported_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_dp_data_with_time_string_reported_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_dp_data_with_time_string_reported_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_dp_data_with_flag_reported_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_dp_data_with_flag_reported_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_dp_data_with_flag_and_time_reported_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_dp_data_with_flag_and_time_reported_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_dp_data_with_flag_and_time_string_reported_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_dp_data_with_flag_and_time_string_reported_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_dp_data_send_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_dp_data_send_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_dp_data_with_time_send_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_dp_data_with_time_send_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_dp_data_with_src_type_send_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_dp_data_with_src_type_send_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_dp_data_with_src_type_and_time_send_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_dp_data_with_src_type_and_time_send_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_accessory_info_report_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_accessory_info_report_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_device_unbind_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_device_unbind_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_factory_reset_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_factory_reset_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_ota_response_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_ota_response_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_data_passthrough_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_data_passthrough_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_speech_evts
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_speech_evts(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_gpt_control_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_gpt_evts(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_data_prod_test_response_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_data_prod_test_response_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_uart_cmd_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_uart_cmd_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_ble_cmd_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_ble_cmd_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_net_config_response_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_net_config_response_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_time_request_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_time_request_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_remoter_proxy_auth_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_remoter_proxy_auth_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_remoter_group_set_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_remoter_group_set_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_remoter_group_delete_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_remoter_group_delete_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_remoter_group_get_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_remoter_group_get_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_extend_time_request_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_extend_time_request_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_unbound_response_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_unbound_response_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_anomaly_unbound_response_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_anomaly_unbound_response_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_device_reset_response_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_device_reset_response_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_connect_change_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_connect_change_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_ble_data_evt
 *
 * @param[in] *buf: *buf
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_ble_data_evt(UINT8_T *buf, UINT16_T len);

/**
 * @brief tuya_ble_handle_connecting_request_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_connecting_request_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_handle_link_update_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_link_update_evt(tuya_ble_evt_param_t *evt);


#ifdef __cplusplus
}
#endif

#endif /* __TUYA_BLE_EVENT_HANDLER_H__ */

