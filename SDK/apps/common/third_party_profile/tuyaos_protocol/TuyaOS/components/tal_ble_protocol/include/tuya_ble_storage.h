/**
 * @file tuya_ble_storage.h
 * @brief This is tuya_ble_storage file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_STORAGE_H__
#define __TUYA_BLE_STORAGE_H__

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


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/

/**
 * @brief tuya_ble_storage_save_sys_settings
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tuya_ble_storage_save_sys_settings(VOID_T);

/**
 * @brief tuya_ble_storage_save_auth_settings
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tuya_ble_storage_save_auth_settings(VOID_T);

/**
 * @brief tuya_ble_storage_init
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tuya_ble_storage_init(VOID_T);

#if (TUYA_BLE_DEVICE_AUTH_DATA_STORE)

/**
 * @brief tuya_ble_storage_write_pid
 *
 * @param[in] pid_type: pid_type
 * @param[in] pid_len: pid_len
 * @param[in] *pid: *pid
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_storage_write_pid(tuya_ble_product_id_type_t pid_type, UINT8_T pid_len, UINT8_T *pid);

/**
 * @brief tuya_ble_storage_write_hid
 *
 * @param[in] *hid: *hid
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_storage_write_hid(UINT8_T *hid, UINT8_T len);

/**
 * @brief tuya_ble_storage_read_id_info
 *
 * @param[in] *id: *id
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_storage_read_id_info(tuya_ble_factory_id_data_t *id);

/**
 * @brief tuya_ble_storage_write_auth_key_device_id_mac
 *
 * @param[in] *auth_key: *auth_key
 * @param[in] auth_key_len: auth_key_len
 * @param[in] *device_id: *device_id
 * @param[in] device_id_len: device_id_len
 * @param[in] *mac: *mac
 * @param[in] mac_len: mac_len
 * @param[in] *mac_string: *mac_string
 * @param[in] mac_string_len: mac_string_len
 * @param[in] *pid: *pid
 * @param[in] pid_len: pid_len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_storage_write_auth_key_device_id_mac(UINT8_T *auth_key, UINT8_T auth_key_len, UINT8_T *device_id, UINT8_T device_id_len, UINT8_T *mac, UINT8_T mac_len, UINT8_T *mac_string, UINT8_T mac_string_len, UINT8_T *pid, UINT8_T pid_len);

/**
 * @brief tuya_ble_storage_write_auth_info
 *
 * @param[in] data: data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_storage_write_auth_info(tuya_ble_factory_id_data_t *data);

/**
 * @brief tuya_ble_storage_write_rf_param
 *
 * @param[in] country_code: country_code
 * @param[in] tx_dBm: tx_dBm
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_storage_write_rf_param(uint8_t country_code, uint32_t tx_dBm);

#endif // (TUYA_BLE_DEVICE_AUTH_DATA_STORE)


#ifdef __cplusplus
}
#endif

#endif /* __TUYA_BLE_STORAGE_H__ */

