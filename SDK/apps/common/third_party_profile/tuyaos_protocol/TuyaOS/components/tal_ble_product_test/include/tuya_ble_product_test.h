/**
 * @file tuya_ble_product_test.h
 * @brief This is tuya_ble_product_test file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_PRODUCT_TEST_H__
#define __TUYA_BLE_PRODUCT_TEST_H__

#include <stdint.h>
#include "tuya_ble_internal_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#if defined(TUYA_BLE_FEATURE_PRODUCT_TEST_ENABLE) && (TUYA_BLE_FEATURE_PRODUCT_TEST_ENABLE == 1)

#define      TUYA_BLE_AUC_CMD_ENTER              0x00
#define      TUYA_BLE_AUC_CMD_QUERY_HID          0x01
#define      TUYA_BLE_AUC_CMD_GPIO_TEST          0x02
#define      TUYA_BLE_AUC_CMD_WRITE_AUTH_INFO    0x03
#define      TUYA_BLE_AUC_CMD_QUERY_INFO         0x04
#define      TUYA_BLE_AUC_CMD_RESET              0x05
#define      TUYA_BLE_AUC_CMD_QUERY_FINGERPRINT  0x06
#define      TUYA_BLE_AUC_CMD_WRITE_HID          0x07
#define      TUYA_BLE_AUC_CMD_RSSI_TEST          0x08
#define      TUYA_BLE_AUC_CMD_WRITE_OEM_INFO     0x09
#define      TUYA_BLE_AUC_CMD_WRITE_SUBPKG_START 0x0C
#define      TUYA_BLE_AUC_CMD_WRITE_SUBPKG_END   0x0D
#define      TUYA_BLE_AUC_CMD_WRITE_CONUTRY_CODE 0x0E
#define      TUYA_BLE_AUC_CMD_READ_CONUTRY_CODE  0x0F
#define      TUYA_BLE_AUC_CMD_WRITE_COMM_CFG     0x12 // ADD
#define      TUYA_BLE_AUC_CMD_READ_MAC           0x13
#define      TUYA_BLE_AUC_CMD_EXIT               0x14

#define      TUYA_BLE_AUC_CMD_EXTEND             0xF0
#define      TUYA_BLE_SDK_TEST_CMD_EXTEND        0xF2

#define      TUYA_BLE_AUC_FINGERPRINT_VER   1

#if ( TUYA_BLE_PROD_SUPPORT_OEM_TYPE == TUYA_BLE_PROD_OEM_TYPE_0_5 )
#define      TUYA_BLE_AUC_WRITE_PID         1
#else
#define      TUYA_BLE_AUC_WRITE_PID         0
#endif

#define      TUYA_BLE_AUC_WRITE_DEV_CERT    1

#define      TUYA_BLE_AUC_NEED_PULL_GPIO_TEST_MAP   1

enum {
    TUYA_BLE_AUC_FW_FINGERPRINT_POS = 0,
    TUYA_BLE_AUC_WRITE_PID_POS      = 1,
    TUYA_BLE_AUC_NEED_PULL_GPIO_TEST_MAP_POS = 4,
    TUYA_BLE_AUC_WRITE_DEV_CERT_POS = 5,
};

#if ENABLE_BLUETOOTH_BREDR
enum {
    TUYA_BLE_WRITE_BT_MAC_POS        = 0,
};
#endif

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
 * @brief tuya_ble_app_production_test_process
 *
 * @param[in] channel: 0-uart ,1 - ble.
 * @param[in] *p_in_data: *p_in_data
 * @param[in] in_len: in_len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_app_production_test_process(UINT8_T channel, UINT8_T *p_in_data, UINT16_T in_len);

#if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE && TUYA_BLE_DEVICE_AUTH_DATA_STORE)

/**
 * @brief tuya_ble_prod_beacon_scan_start
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_prod_beacon_scan_start(VOID_T);

/**
 * @brief tuya_ble_prod_beacon_scan_stop
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_prod_beacon_scan_stop(VOID_T);

/**
 * @brief tuya_ble_prod_beacon_get_rssi_avg
 *
 * @param[in] *rssi: *rssi
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_prod_beacon_get_rssi_avg(INT8_T *rssi);

/**
 * @brief tuya_ble_prod_gpio_test
 *
 * @param[in] error_sequence: error_sequence
 * @param[in] *para: *para
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_prod_gpio_test(CHAR_T *error_sequence, UINT8_T *para, UINT8_T len);

/**
 * @brief tuya_ble_internal_production_test_with_ble_flag_clear
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_internal_production_test_with_ble_flag_clear(VOID_T);

/**
 * @brief tuya_ble_internal_production_test_with_ble_flag_get
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tuya_ble_internal_production_test_with_ble_flag_get(VOID_T);

/**
 * @brief  storage product test common data, for example: shorturl
 *
 * @param  common_data_type      describe data type
 * @param  p_data                storage data point
 * @param  data_size             p_data size
 *
 * @return tuya_ble_status_t
 */
tuya_ble_status_t tuya_ble_product_test_storage_common_data(tuya_ble_common_data_type type, UINT8_T *p_data, UINT32_T data_size);

/**
 * @brief  read product test common data, for example: shorturl
 *
 * @param[in]  common_data_type      describe data type
 * @param[in]  p_data                storage data point
 * @param[in]  data_size             the point of p_data size
 *
 * @return tuya_ble_status_t
 */
tuya_ble_status_t tuya_ble_product_test_readback_common_data(tuya_ble_common_data_type type, UINT8_T *p_data, UINT32_T *data_size);

/**
 * @brief storage private data, for example: token/dev cert/keys
 *
 * @param[in] private_data_type: describe data type
 * @param[in] p_data: storage data point
 * @param[in] data_size: p_data size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_storage_private_data(tuya_ble_private_data_type private_data_type, uint8_t *p_data, uint32_t data_size);

/**
 * @brief Get device certificate length
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tuya_ble_get_dev_crt_len(VOID_T);

/**
 * @brief Get device certificate data
 *
 * @param[in] *p_der: *p_der
 * @param[in] der_len: der_len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_get_dev_crt_der(UINT8_T *p_der, UINT32_T der_len);

/**
 * @brief tuya_ble_ecc_keypair_gen_secp256r1
 *
 * @param[in] *public_key: *public_key
 * @param[in] *private_key: *private_key
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_ecc_keypair_gen_secp256r1(UINT8_T *public_key, UINT8_T *private_key);

/**
 * @brief tuya_ble_ecc_shared_secret_compute_secp256r1
 *
 * @param[in] *public_key: *public_key
 * @param[in] *private_key: *private_key
 * @param[in] *secret_key: *secret_key
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_ecc_shared_secret_compute_secp256r1(UINT8_T *public_key, UINT8_T *private_key, UINT8_T *secret_key);

#endif // (TUYA_BLE_DEVICE_REGISTER_FROM_BLE && TUYA_BLE_DEVICE_AUTH_DATA_STORE)

#endif // TUYA_BLE_FEATURE_PRODUCT_TEST_ENABLE


#ifdef __cplusplus
}
#endif

#endif /* __TUYA_BLE_PRODUCT_TEST_H__ */

