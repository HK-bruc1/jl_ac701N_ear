/**
 * @file tuya_ble_api.h
 * @brief This is tuya_ble_api file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_API_H__
#define __TUYA_BLE_API_H__

#include "tuya_cloud_types.h"
#include "tuya_ble_type.h"
#include "tuya_ble_internal_config.h"
#include "tuya_ble_port.h"

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

#if (TUYA_BLE_USE_OS==0)

/**
 * @brief Function for executing all enqueued tasks.
 *
 * @note This function must be called from within the main loop. It will
 * execute all events scheduled since the last time it was called.
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_main_tasks_exec(VOID_T);

#endif

/**
 * @brief Function for transmit ble data from peer devices to tuya sdk.
 *
 * @note This function must be called from where the ble data is received.
 *
 * @param[in] *p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_gatt_receive_data(UINT8_T *p_data, UINT16_T len);

/**
 * @brief Function for transmit uart data to tuya sdk.
 *
 * @note This function must be called from where the uart data is received.
 *
 * @param[in] *p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_common_uart_receive_data(UINT8_T *p_data, UINT16_T len);

/**
 * @brief Function for send the full instruction received from uart to the sdk.
 *
 * @note If the application uses a custom uart parsing algorithm to obtain the full uart instruction,then call this function to send the full instruction.
 *
 * @param[in] *p_data: pointer to full instruction data(Complete instruction,include0x55 or 0x66 0xaa and checksum.)
 * @param[in] len: Number of bytes of pdata.
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_common_uart_send_full_instruction_received(UINT8_T *p_data, UINT16_T len);

/**
 * @brief Function for update the device id to tuya sdk.
 *
 * @note the following id of the device must be update immediately when changed.
 *
 * @param[in] type: type
 * @param[in] len: len
 * @param[in] p_buf: p_buf
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_device_update_product_id(tuya_ble_product_id_type_t type, UINT8_T len, UINT8_T *p_buf);

/**
 * @brief tuya_ble_device_update_login_key
 *
 * @param[in] p_buf: p_buf
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_device_update_login_key(UINT8_T *p_buf, UINT8_T len);

#if ((TUYA_BLE_PROTOCOL_VERSION_HIGN>=4) && (TUYA_BLE_BEACON_KEY_ENABLE))

/**
 * @brief tuya_ble_device_update_beacon_key
 *
 * @param[in] p_buf: p_buf
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_device_update_beacon_key(UINT8_T *p_buf, UINT8_T len);

#endif

/**
 * @brief tuya_ble_device_update_bound_state
 *
 * @param[in] state: state
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_device_update_bound_state(UINT8_T state);

/**
 * @brief tuya_ble_device_update_mcu_version
 *
 * @param[in] mcu_firmware_version: mcu_firmware_version
 * @param[in] mcu_hardware_version: mcu_hardware_version
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_device_update_mcu_version(UINT32_T mcu_firmware_version, UINT32_T mcu_hardware_version);

#if defined(TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE) && (TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE == 1)

/**
 * @brief tuya_ble_device_update_attach_version
 *
 * @param[in] buf: buf
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_device_update_attach_version(VOID_T *buf, UINT32_T size);

#endif

/**
 * @brief Function for initialize the tuya sdk.
 *
 * @note appliction should call this after all platform init complete.
 *
 * @param[in] param_data: param_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_sdk_init(tuya_ble_device_param_t *param_data);

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN==5)

#if (TUYA_BLE_MUTI_DATA_SOURCE_SUPPORTED)

/**
 * @brief Function for send the dp point data with data source type.
 *
 * @note    new api, The length of each dp data sent through this function must be 2 bytes. DTLD->'L' must be 2 bytes.
 *
 * @param[in] sn: The sending sequence number of the application definition management.
 * @param[in] src_type: See the description in the 'tuya_ble_type.h' file for details.
 * @param[in] type: DP_SEND_TYPE_ACTIVE- The device actively sends dp data;DP_SEND_TYPE_PASSIVE- The device passively sends dp data. For example, in order to answer the dp query command of the mobile app. Currently only applicable to WIFI+BLE combo devices.
 * @param[in] mode: See the description in the 'tuya_ble_type.h' file for details.
 * @param[in] ack: See the description in the 'tuya_ble_type.h' file for details.
 * @param[in] add_info_len: The length of additional Information.
 * @param[in] p_add_info: The pointer of additional Information data.
 * @param[in] p_dp_data: The pointer of dp data .
 * @param[in] dp_data_len: The length of dp data .
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_dp_data_with_src_type_send(uint32_t sn, tuya_ble_data_source_type_t src_type, tuya_ble_dp_data_send_type_t type, tuya_ble_dp_data_send_mode_t mode, tuya_ble_dp_data_send_ack_t ack, uint8_t add_info_len, uint8_t *p_add_info, uint8_t *p_dp_data, uint32_t dp_data_len);

/**
 * @brief Function for send the dp data with data source type and time.
 *
 * @note    new api, The length of each dp data sent through this function must be 2 bytes. DTLD->'L' must be 2 bytes.
 *
 * @param[in] sn: The sending sequence number of the application definition management.
 * @param[in] src_type: See the description in the 'tuya_ble_type.h' file for details.
 * @param[in] mode: See the description in the 'tuya_ble_type.h' file for details.
 * @param[in] time_type: DP_TIME_TYPE_MS_STRING - Indicates that the following 'p_time_data' is a string of milliseconds that must be 13 bytes in length.
 *                       E.g, 'p_time_data' points to the string "1600777955000";
 *                       DP_TIME_TYPE_UNIX_TIMESTAMP - Indicates that the following 'p_time_data' points to the four-byte unix timestamp data.
 *                       E.g, unix timestamp is 1600777955 = 0x5F69EEE3, then 'p_time_data' is {0x5F,0x69,0xEE,0xE3} ;
 * @param[in] p_time_data: time data pointer.
 * @param[in] add_info_len: The length of additional Information.
 * @param[in] p_add_info: The pointer of additional Information data.
 * @param[in] p_dp_data: The pointer of dp data .
 * @param[in] dp_data_len: The length of dp data .
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_dp_data_with_src_type_and_time_send(uint32_t sn, tuya_ble_data_source_type_t src_type, tuya_ble_dp_data_send_mode_t mode, tuya_ble_dp_data_send_time_type_t time_type, uint8_t *p_time_data, uint8_t add_info_len, uint8_t *p_add_info, uint8_t *p_dp_data, uint32_t dp_data_len);

#endif

/**
 * @brief Function for send the dp point data.
 *
 * @note    new api, The length of each dp data sent through this function must be 2 bytes. DTLD->'L' must be 2 bytes.
 *
 * @param[in] sn: The sending sequence number of the application definition management.
 * @param[in] type: DP_SEND_TYPE_ACTIVE- The device actively sends dp data;DP_SEND_TYPE_PASSIVE- The device passively sends dp data. For example, in order to answer the dp query command of the mobile app. Currently only applicable to WIFI+BLE combo devices.
 * @param[in] mode: See the description in the 'tuya_ble_type.h' file for details.
 * @param[in] ack: See the description in the 'tuya_ble_type.h' file for details.
 * @param[in] p_dp_data: The pointer of dp data .
 * @param[in] dp_data_len: The length of dp data .
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_dp_data_send(UINT32_T sn, tuya_ble_dp_data_send_type_t type, tuya_ble_dp_data_send_mode_t mode, tuya_ble_dp_data_send_ack_t ack, UINT8_T *p_dp_data, UINT32_T dp_data_len);

/**
 * @brief Function for send the dp data with time.
 *
 * @note    new api, The length of each dp data sent through this function must be 2 bytes. DTLD->'L' must be 2 bytes.
 *.         The default type of this function is 'DP_SEND_TYPE_ACTIVE', and the default ack is 'DP_SEND_WITH_RESPONSE', which cannot be changed.
 *
 * @param[in] sn: The sending sequence number of the application definition management.
 * @param[in] mode: See the description in the 'tuya_ble_type.h' file for details.
 * @param[in] time_type: DP_TIME_TYPE_MS_STRING - Indicates that the following 'p_time_data' is a string of milliseconds that must be 13 bytes in length.
 *                       E.g, 'p_time_data' points to the string "1600777955000";
 *                       DP_TIME_TYPE_UNIX_TIMESTAMP - Indicates that the following 'p_time_data' points to the four-byte unix timestamp data.
 *                       E.g, unix timestamp is 1600777955 = 0x5F69EEE3, then 'p_time_data' is {0x5F,0x69,0xEE,0xE3} ;
 * @param[in] p_time_data: time data pointer.
 * @param[in] p_dp_data: The pointer of dp data .
 * @param[in] dp_data_len: The length of dp data .
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_dp_data_with_time_send(UINT32_T sn, tuya_ble_dp_data_send_mode_t mode, tuya_ble_dp_data_send_time_type_t time_type, UINT8_T *p_time_data, UINT8_T *p_dp_data, UINT32_T dp_data_len);

#elif (TUYA_BLE_PROTOCOL_VERSION_HIGN==4)

#if (TUYA_BLE_MUTI_DATA_SOURCE_SUPPORTED&&TUYA_BLE_PROTOCOL_VERSION_LOW>=5)

/**
 * @brief Function for send the dp point data with data source type.
 *
 * @note    new api, The length of each dp data sent through this function must be 2 bytes. DTLD->'L' must be 2 bytes.
 *
 * @param[in] sn: The sending sequence number of the application definition management.
 * @param[in] src_type: See the description in the 'tuya_ble_type.h' file for details.
 * @param[in] type: DP_SEND_TYPE_ACTIVE- The device actively sends dp data;DP_SEND_TYPE_PASSIVE- The device passively sends dp data. For example, in order to answer the dp query command of the mobile app. Currently only applicable to WIFI+BLE combo devices.
 * @param[in] mode: See the description in the 'tuya_ble_type.h' file for details.
 * @param[in] ack: See the description in the 'tuya_ble_type.h' file for details.
 * @param[in] add_info_len: The length of additional Information.
 * @param[in] p_add_info: The pointer of additional Information data.
 * @param[in] p_dp_data: The pointer of dp data .
 * @param[in] dp_data_len: The length of dp data .
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_dp_data_with_src_type_send(uint32_t sn, tuya_ble_data_source_type_t src_type, tuya_ble_dp_data_send_type_t type, tuya_ble_dp_data_send_mode_t mode, tuya_ble_dp_data_send_ack_t ack, uint8_t add_info_len, uint8_t *p_add_info, uint8_t *p_dp_data, uint32_t dp_data_len);

/**
 * @brief Function for send the dp data with data source type and time.
 *
 * @param[in] sn: The sending sequence number of the application definition management.
 * @param[in] src_type: See the description in the 'tuya_ble_type.h' file for details.
 * @param[in] mode: See the description in the 'tuya_ble_type.h' file for details.
 * @param[in] time_type: DP_TIME_TYPE_MS_STRING - Indicates that the following 'p_time_data' is a string of milliseconds that must be 13 bytes in length.
 *                       E.g, 'p_time_data' points to the string "1600777955000";
 *                       DP_TIME_TYPE_UNIX_TIMESTAMP - Indicates that the following 'p_time_data' points to the four-byte unix timestamp data.
 *                       E.g, unix timestamp is 1600777955 = 0x5F69EEE3, then 'p_time_data' is {0x5F,0x69,0xEE,0xE3} ;
 * @param[in] p_time_data: time data pointer.
 * @param[in] add_info_len: The length of additional Information.
 * @param[in] p_add_info: The pointer of additional Information data.
 * @param[in] p_dp_data: The pointer of dp data .
 * @param[in] dp_data_len: The length of dp data .
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_dp_data_with_src_type_and_time_send(uint32_t sn, tuya_ble_data_source_type_t src_type, tuya_ble_dp_data_send_mode_t mode, tuya_ble_dp_data_send_time_type_t time_type, uint8_t *p_time_data, uint8_t add_info_len, uint8_t *p_add_info, uint8_t *p_dp_data, uint32_t dp_data_len);

#endif

/**
 * @brief Function for send the dp point data.
 *
 * @note    new api, The length of each dp data sent through this function must be 2 bytes. DTLD->'L' must be 2 bytes.
 *
 * @param[in] sn: The sending sequence number of the application definition management.
 * @param[in] type: DP_SEND_TYPE_ACTIVE- The device actively sends dp data;DP_SEND_TYPE_PASSIVE- The device passively sends dp data. For example, in order to answer the dp query command of the mobile app. Currently only applicable to WIFI+BLE combo devices.
 * @param[in] mode: See the description in the 'tuya_ble_type.h' file for details.
 * @param[in] ack: See the description in the 'tuya_ble_type.h' file for details.
 * @param[in] *p_dp_data: The pointer of dp data .
 * @param[in] dp_data_len: The length of dp data .
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_dp_data_send(UINT32_T sn, tuya_ble_dp_data_send_type_t type, tuya_ble_dp_data_send_mode_t mode, tuya_ble_dp_data_send_ack_t ack, UINT8_T *p_dp_data, UINT32_T dp_data_len);

/**
 * @brief Function for send the dp data with time.
 *
 * @note    new api, The length of each dp data sent through this function must be 2 bytes. DTLD->'L' must be 2 bytes.
 *
 * @param[in] sn: The sending sequence number of the application definition management.
 * @param[in] mode: See the description in the 'tuya_ble_type.h' file for details.
 * @param[in] time_type: DP_TIME_TYPE_MS_STRING - Indicates that the following 'p_time_data' is a string of milliseconds that must be 13 bytes in length.
 *                       E.g, 'p_time_data' points to the string "1600777955000";
 *                       DP_TIME_TYPE_UNIX_TIMESTAMP - Indicates that the following 'p_time_data' points to the four-byte unix timestamp data.
 *                       E.g, unix timestamp is 1600777955 = 0x5F69EEE3, then 'p_time_data' is {0x5F,0x69,0xEE,0xE3} ;
 * @param[in] p_time_data: time data pointer.
 * @param[in] p_dp_data: The pointer of dp data .
 * @param[in] dp_data_len: The length of dp data .
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_dp_data_with_time_send(UINT32_T sn, tuya_ble_dp_data_send_mode_t mode, tuya_ble_dp_data_send_time_type_t time_type, UINT8_T *p_time_data, UINT8_T *p_dp_data, UINT32_T dp_data_len);

#else

/**
 * @brief Function for report the dp point data.
 *
 * @param[in] *p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_dp_data_report(UINT8_T *p_data, UINT32_T len);

/**
 * @brief Function for report the dp point data with time.
 *
 * @param[in] timestamp: timestamp
 * @param[in] *p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_dp_data_with_time_report(UINT32_T timestamp, UINT8_T *p_data, UINT32_T len);

/**
 * @brief Function for report the dp point data with time.
 *
 * @param[in] time_string: 13-byte millisecond string ,for example ,"0000000123456";
 * @param[in] p_data: p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_dp_data_with_time_ms_string_report(UINT8_T *time_string, UINT8_T *p_data, UINT32_T len);

/**
 * @brief Function for report the dp point data with flag.
 *
 * @param[in] sn: sn
 * @param[in] mode: mode
 * @param[in] *p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_dp_data_with_flag_report(UINT16_T sn, tuya_ble_dp_data_send_mode_t mode, UINT8_T *p_data, UINT32_T len);

/**
 * @brief Function for report the dp point data with flag and time.
 *
 * @param[in] sn: sn
 * @param[in] mode: mode
 * @param[in] timestamp: timestamp
 * @param[in] *p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_dp_data_with_flag_and_time_report(UINT16_T sn, tuya_ble_dp_data_send_mode_t mode, UINT32_T timestamp, UINT8_T *p_data, UINT32_T len);

/**
 * @brief Function for report the dp point data with flag and time.
 *
 * @param[in] sn: sn
 * @param[in] mode: mode
 * @param[in] time_string: 13-byte millisecond string ,for example ,"0000000123456";
 * @param[in] p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_dp_data_with_flag_and_time_ms_string_report(UINT16_T sn, tuya_ble_dp_data_send_mode_t mode, UINT8_T *time_string, UINT8_T *p_data, UINT32_T len);

#endif

/**
 * @brief Function for process the internal state of tuya sdk, application should  call this in connect handler.
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_connected_handler(VOID_T);

/**
 * @brief Function for process the internal state of tuya sdk, application should  call this in disconnect handler.
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_disconnected_handler(VOID_T);

#if TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE

/**
 * @brief Function for update the link encrypted status to sdk, application should  call this function when the link layer encryption is successful.
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_link_encrypted_handler(VOID_T);

#endif

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN >= 3)

/**
 * @brief Function for process the internal state of tuya sdk, application should  call this in disconnect handler.
 *
 * @param[in] on_off: 0-off ,1 - on.
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_adv_data_connecting_request_set(UINT8_T on_off);

#endif

/**
 * @brief Function for data passthrough.
 *
 * @note    The tuya sdk will forwards the data to the app.
 *
 * @param[in] *p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_data_passthrough(UINT8_T *p_data, UINT32_T len);

#if defined(TUYA_BLE_FEATURE_SPEECH_ENABLE) && (TUYA_BLE_FEATURE_SPEECH_ENABLE == 1)

/**
 * @brief tuya_ble_speech_control
 *
 * @note    The tuya sdk will forwards the data to the app.
 *
 * @param[in] *p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_speech_control(UINT8_T *p_data, UINT32_T len);

/**
 * @brief tuya_ble_speech_raw_data_report
 *
 * @note    The tuya sdk will forwards the data to the app.
 *
 * @param[in] *p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_speech_raw_data_report(UINT8_T *p_data, UINT32_T len);

/**
 * @brief tuya_ble_speech_result_write_rsp
 *
 * @note    The tuya sdk will forwards the data to the app.
 *
 * @param[in] *p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_speech_result_write_rsp(UINT8_T *p_data, UINT32_T len);

/**
 * @brief tuya_ble_speech_clock_setting_rsp
 *
 * @note    The tuya sdk will forwards the data to the app.
 *
 * @param[in] *p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_speech_clock_setting_rsp(UINT8_T *p_data, UINT32_T len);

/**
 * @brief tuya_ble_speech_token_read_rsp
 *
 * @note    The tuya sdk will forwards the data to the app.
 *
 * @param[in] *p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_speech_token_read_rsp(UINT8_T *p_data, UINT32_T len);

/**
 * @brief tuya_ble_speech_token_report
 *
 * @note    The tuya sdk will forwards the data to the app.
 *
 * @param[in] *p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_speech_token_report(UINT8_T *p_data, UINT32_T len);

/**
 * @brief tuya_ble_speech_token_write_rsp
 *
 * @note    The tuya sdk will forwards the data to the app.
 *
 * @param[in] *p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_speech_token_write_rsp(UINT8_T *p_data, UINT32_T len);

/**
 * @brief tuya_ble_speech_common_setting_rsp
 *
 * @note    The tuya sdk will forwards the data to the app.
 *
 * @param[in] *p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_speech_common_setting_rsp(UINT8_T *p_data, UINT32_T len);

#endif

#if defined(TUYA_BLE_FEATURE_GPT_ENABLE) && (TUYA_BLE_FEATURE_GPT_ENABLE == 1)

/**
 * @brief tuya_ble_gpt_control
 *
 * @note    The tuya sdk will forwards the data to the app.
 *
 * @param[in] *p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_gpt_control(UINT8_T *p_data, UINT32_T len);

/**
 * @brief tuya_ble_gpt_raw_data_report
 *
 * @note    The tuya sdk will forwards the data to the app.
 *
 * @param[in] *p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_gpt_raw_data_report(UINT8_T *p_data, UINT32_T len);

/**
 * @brief tuya_ble_gpt_result_write_rsp
 *
 * @note    The tuya sdk will forwards the data to the app.
 *
 * @param[in] *p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_gpt_result_write_rsp(UINT8_T *p_data, UINT32_T len);

/**
 * @brief tuya_ble_gpt_raw_data_write_rsp
 *
 * @note    The tuya sdk will forwards the data to the app.
 *
 * @param[in] *p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_gpt_raw_data_write_rsp(UINT8_T *p_data, UINT32_T len);

#endif

/**
 * @brief Function for response the production test instruction asynchronous.
 *
 * @note    The tuya sdk will forwards the data to the app.
 *
 * @param[in] channel: 0-uart ,1 - ble.
 * @param[in] p_data: pointer to production test cmd data(Complete instruction,include0x66 0xaa and checksum.)
 * @param[in] len: Number of bytes of pdata.
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_production_test_asynchronous_response(UINT8_T channel, UINT8_T *p_data, UINT32_T len);

/**
 * @brief Function for response for the net config req.
 *
 * @param[in] result_code: result_code
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_net_config_response(INT16_T result_code);

#if (!TUYA_BLE_DEVICE_REGISTER_FROM_BLE)

/**
 * @brief Function for response for ubound req.
 *
 * @param[in] result_code: result_code
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_ubound_response(UINT8_T result_code);

/**
 * @brief Function for response for anomaly ubound req.
 *
 * @param[in] result_code: result_code
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_anomaly_ubound_response(UINT8_T result_code);

/**
 * @brief Function for response for device reset req.
 *
 * @param[in] result_code: result_code
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_device_reset_response(UINT8_T result_code);

#endif

/**
 * @brief Function for get the ble connet status.
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_connect_status_t tuya_ble_connect_status_get(VOID_T);

/**
 * @brief Function for get the ble connet source type.
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_connect_source_type_t tuya_ble_connect_source_type_get(void);

/**
 * @brief Function for notify the sdk the device has unbind.
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_device_unbind(VOID_T);

/**
 * @brief Function for notify the sdk the device has resumes factory Settings.
 *
 * @note    When the device resumes factory Settings,shoule notify the sdk.
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_device_factory_reset(VOID_T);

/**
 * @brief Function for Request update time.
 *
 * @param[in] time_type: 0-13-byte millisecond string [from cloud],1 - normal time format [from cloud] , 2 - normal time format[from app local]
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_time_req(UINT8_T time_type);

/**
 * @brief Function for Authentication Remoter.
 *
 * @param[in] data: Remoter info, p_data refer to struct tuya_ble_remoter_proxy_auth_data_unit_t.
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_remoter_proxy_auth(tuya_ble_remoter_proxy_auth_data_t data);

/**
 * @brief Function for setting Remoter Group.
 *
 * @param[in] status: 0 success, 1 false.
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_remoter_group_set(UINT8_T status);

/**
 * @brief Function for delete Remoter Group.
 *
 * @param[in] status: 0 success, 1 false.
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_remoter_group_delete(UINT8_T status);

/**
 * @brief Function for APP Get Remoter Group.
 *
 * @param[in] data: device info which in the group.
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_remoter_group_get(tuya_ble_remoter_group_get_data_rsp_t data);

/**
 * @brief Function for response the ota req.
 *
 * @note    response the ota data req from call back event.
 *
 * @param[in] *p_data: *p_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_ota_response(tuya_ble_ota_response_t *p_data);

/**
 * @brief Function for send custom event to main process of ble sdk.
 *
 * @param[in] evt: evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tuya_ble_custom_event_send(tuya_ble_custom_evt_t evt);

#if TUYA_BLE_BR_EDR_SUPPORTED

/**
 * @brief Function for update br/edr status in Dual-Mode Bluetooth  device.
 *
 * @param[in] p_data: pointer to br/edr information data.
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_br_edr_data_info_update(tuya_ble_br_edr_data_info_t *p_data);

#endif

#if TUYA_BLE_USE_OS

/**
 * @brief Function for registe queue to receive call back evt when use os
 *
 * @param[in] *cb_queue: *cb_queue
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_callback_queue_register(VOID_T *cb_queue);

/**
 * @brief Function for response the event.
 *
 * @note    if use os,must be sure to call this function after process one event in queue.
 *
 * @param[in] *param: *param
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_event_response(tuya_ble_cb_evt_param_t *param);

#else

/**
 * @brief Function for registe call back functions.
 *
 * @note    appliction should receive the message from the call back registed by this function.
 *
 * @param[in] cb: cb
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_callback_queue_register(tuya_ble_callback_t cb);

/**
 * @brief Function for get scheduler queue size.
 *
 * @note    If it returns 0, it means that the queue has not been initialized.
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT16_T tuya_ble_scheduler_queue_size_get(VOID_T);

/**
 * @brief Function for get queue free space.
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT16_T tuya_ble_scheduler_queue_space_get(VOID_T);

/**
 * @brief Function for get the number of current events in the queue.
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT16_T tuya_ble_scheduler_queue_events_get(VOID_T);

#endif

/**
 * @brief Function for check if sleep is allowed.
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
BOOL_T tuya_ble_sleep_allowed_check(VOID_T);

/**
 * @brief tuya_ble_storage_device_save_mac
 *
 * @param[in] *mac: *mac
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_storage_device_save_mac(UINT8_T *mac);

/**
 * @brief tuya_ble_storage_device_save_ota_status
 *
 * @param[in] ota_status: ota_status
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_storage_device_save_ota_status(UINT8_T ota_status);

#ifdef __cplusplus
}
#endif

#endif /* __TUYA_BLE_API_H__ */

