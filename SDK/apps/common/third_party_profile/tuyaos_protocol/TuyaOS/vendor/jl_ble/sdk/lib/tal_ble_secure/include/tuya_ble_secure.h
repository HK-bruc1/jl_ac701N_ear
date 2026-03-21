/**
 * @file tuya_ble_secure.h
 * @brief This is tuya_ble_secure file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_SECURE_H__
#define __TUYA_BLE_SECURE_H__

#include "tuya_cloud_types.h"
#include "tuya_ble_type.h"
#include "tuya_ble_internal_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
enum {
    BLE_ENCRYPTION_MODE_NONE = 0,
    BLE_ENCRYPTION_MODE_KEY_1,
    BLE_ENCRYPTION_MODE_KEY_2,
    BLE_ENCRYPTION_MODE_KEY_3,
    BLE_ENCRYPTION_MODE_KEY_4,
    BLE_ENCRYPTION_MODE_SESSION_KEY,
    BLE_ENCRYPTION_MODE_ECDH_KEY,
    BLE_ENCRYPTION_MODE_FTM_KEY, // Only RunFeng Body fat scale is using by now

    BLE_ENCRYPTION_MODE_KEY_11 = 11,
    BLE_ENCRYPTION_MODE_KEY_12,
    BLE_ENCRYPTION_MODE_KEY_13,
    BLE_ENCRYPTION_MODE_KEY_14,
    BLE_ENCRYPTION_MODE_SESSION_KEY_ENC,
    BLE_ENCRYPTION_MODE_KEY_16,
    BLE_ENCRYPTION_MODE_MAX,
};

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN==5)

#define ENCRYPTION_MODE_NONE            BLE_ENCRYPTION_MODE_NONE
#define ENCRYPTION_MODE_KEY_1           BLE_ENCRYPTION_MODE_KEY_11
#define ENCRYPTION_MODE_KEY_2           BLE_ENCRYPTION_MODE_KEY_12
#define ENCRYPTION_MODE_KEY_3           BLE_ENCRYPTION_MODE_KEY_13
#define ENCRYPTION_MODE_KEY_4           BLE_ENCRYPTION_MODE_KEY_14
#define ENCRYPTION_MODE_SESSION_KEY     BLE_ENCRYPTION_MODE_SESSION_KEY_ENC
#define ENCRYPTION_MODE_ECDH_KEY        BLE_ENCRYPTION_MODE_ECDH_KEY
#define ENCRYPTION_MODE_FTM_KEY         BLE_ENCRYPTION_MODE_FTM_KEY
#define ENCRYPTION_MODE_AN_UNBOND_KEY   BLE_ENCRYPTION_MODE_KEY_16
#define ENCRYPTION_MODE_MAX             BLE_ENCRYPTION_MODE_MAX

#else

#define ENCRYPTION_MODE_NONE            BLE_ENCRYPTION_MODE_NONE
#define ENCRYPTION_MODE_KEY_1           (tuya_ble_cryption_v2_status_get(&tuya_ble_current_para)? BLE_ENCRYPTION_MODE_KEY_11:BLE_ENCRYPTION_MODE_KEY_1)
#define ENCRYPTION_MODE_KEY_2           (tuya_ble_cryption_v2_status_get(&tuya_ble_current_para)? BLE_ENCRYPTION_MODE_KEY_12:BLE_ENCRYPTION_MODE_KEY_2)
#define ENCRYPTION_MODE_KEY_3           BLE_ENCRYPTION_MODE_KEY_3
#define ENCRYPTION_MODE_KEY_4           (tuya_ble_cryption_v2_status_get(&tuya_ble_current_para)? BLE_ENCRYPTION_MODE_KEY_14:BLE_ENCRYPTION_MODE_KEY_4)
#define ENCRYPTION_MODE_SESSION_KEY     (tuya_ble_cryption_v2_status_get(&tuya_ble_current_para)? BLE_ENCRYPTION_MODE_SESSION_KEY_ENC:BLE_ENCRYPTION_MODE_SESSION_KEY)
#define ENCRYPTION_MODE_ECDH_KEY        BLE_ENCRYPTION_MODE_ECDH_KEY
#define ENCRYPTION_MODE_FTM_KEY         BLE_ENCRYPTION_MODE_FTM_KEY
#define ENCRYPTION_MODE_AN_UNBOND_KEY   (tuya_ble_cryption_v2_status_get(&tuya_ble_current_para)? BLE_ENCRYPTION_MODE_KEY_16:BLE_ENCRYPTION_MODE_KEY_1)
#define ENCRYPTION_MODE_MAX             BLE_ENCRYPTION_MODE_MAX

#endif

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
#if defined(TUYA_BLE_OTA_SIGNATURE_ENABLE) && (TUYA_BLE_OTA_SIGNATURE_ENABLE == 1)
typedef struct {
    uint8_t  key_version;
    uint8_t  key_len;
    uint8_t  key[64];
} tuya_ble_ota_signature_key_t;
#endif

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
#if defined(TUYA_BLE_OTA_SIGNATURE_ENABLE) && (TUYA_BLE_OTA_SIGNATURE_ENABLE == 1)
extern const tuya_ble_ota_signature_key_t sg_signature_key_info;
extern const uint8_t rand_num[];
#endif

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/

/**
 * @brief tuya_ble_register_key_generate
 *
 * @param[out] *output: *output
 * @param[in] *current_para: *current_para
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
BOOL_T tuya_ble_register_key_generate(UINT8_T *output, tuya_ble_parameters_settings_t *current_para);

/**
 * @brief tuya_ble_encryption
 *
 * @param[in] protocol_version: protocol_version
 * @param[in] encryption_mode: encryption_mode
 * @param[in] *iv: *iv
 * @param[in] *in_buf: *in_buf
 * @param[in] in_len: in_len
 * @param[out] *out_len: *out_len
 * @param[out] *out_buf: *out_buf
 * @param[in] *current_para_data: *current_para_data
 * @param[in] *dev_rand: *dev_rand
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tuya_ble_encryption(UINT16_T protocol_version, UINT8_T encryption_mode, UINT8_T *iv, UINT8_T *in_buf, UINT32_T in_len, UINT32_T *out_len, UINT8_T *out_buf, tuya_ble_parameters_settings_t *current_para_data, UINT8_T *dev_rand);

/**
 * @brief tuya_ble_encrypt_old_with_key
 *
 * @param[in] *current_para: *current_para
 * @param[in] *adv_buf: *adv_buf
 * @param[in] version: version
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tuya_ble_encrypt_old_with_key(tuya_ble_parameters_settings_t *current_para, UINT8_T *adv_buf, UINT8_T version);

/**
 * @brief tuya_ble_device_id_encrypt
 *
 * @param[in] *current_para: *current_para
 * @param[in] *adv_buf: *adv_buf
 * @param[in] *scan_rsp_buf: *scan_rsp_buf
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tuya_ble_device_id_encrypt(tuya_ble_parameters_settings_t *current_para, UINT8_T *adv_buf, UINT8_T *scan_rsp_buf);

/**
 * @brief tuya_ble_device_id_encrypt_v4
 *
 * @param[in] *current_para: *current_para
 * @param[in] *adv_buf: *adv_buf
 * @param[in] *scan_rsp_buf: *scan_rsp_buf
 * @param[in] version: version
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tuya_ble_device_id_encrypt_v4(tuya_ble_parameters_settings_t *current_para, UINT8_T *adv_buf, UINT8_T *scan_rsp_buf, UINT8_T version);

/**
 * @brief tuya_ble_decryption
 *
 * @param[in] protocol_version: protocol_version
 * @param[in] *in_buf: *in_buf
 * @param[in] in_len: in_len
 * @param[out] *out_len: *out_len
 * @param[out] *out_buf: *out_buf
 * @param[in] *current_para_data: *current_para_data
 * @param[in] *dev_rand: *dev_rand
 * @param[in] version: version
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tuya_ble_decryption(UINT16_T protocol_version, UINT8_T CONST *in_buf, UINT32_T in_len, UINT32_T *out_len, UINT8_T *out_buf, tuya_ble_parameters_settings_t *current_para_data, UINT8_T *dev_rand, UINT8_T version);

/**
 * @brief tuya_ble_server_cert_data_verify
 *
 * @param[in] *p_data: *p_data
 * @param[in] data_len: data_len
 * @param[in] *p_sig: *p_sig
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
BOOL_T tuya_ble_server_cert_data_verify(CONST UINT8_T *p_data, UINT16_T data_len, CONST UINT8_T *p_sig);

/**
 * @brief tuya_ble_sig_data_verify
 *
 * @param[in] *p_pk: *p_pk
 * @param[in] *p_data: *p_data
 * @param[in] data_len: data_len
 * @param[in] *p_sig: *p_sig
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
BOOL_T tuya_ble_sig_data_verify(CONST UINT8_T *p_pk, CONST UINT8_T *p_data, UINT16_T data_len, CONST UINT8_T *p_sig);

/**
 * @brief tuya_ble_accessory_encrypt
 *
 * @param[in] *key_in: *key_in
 * @param[in] key_len: key_len
 * @param[in] *input: *input
 * @param[in] input_len: input_len
 * @param[out] *output: *output
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
BOOL_T tuya_ble_accessory_encrypt(UINT8_T *key_in, UINT16_T key_len, UINT8_T *input, UINT16_T input_len, UINT8_T *output);

/**
 * @brief tuya_ble_event_process
 *
 * @param[in] *tuya_ble_evt: *tuya_ble_evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_event_process(tuya_ble_evt_param_t *tuya_ble_evt);

/**
 * @brief tuya_ble_cryption_v2_data_verify
 *
 * @param[in] *current_para: *current_para
 * @param[in] *verify_buf: *verify_buf
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
BOOL_T tuya_ble_cryption_v2_data_verify(tuya_ble_parameters_settings_t *current_para, UINT8_T *verify_buf);

/**
 * @brief tuya_ble_cryption_v2_status_get
 *
 * @param[in] *current_para: *current_para
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
BOOL_T tuya_ble_cryption_v2_status_get(tuya_ble_parameters_settings_t *current_para);

/**
 * @brief tuya_ble_cryption_v2_enable_set
 *
 * @param[in] *current_para: *current_para
 * @param[in] enable: enable
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_cryption_v2_enable_set(tuya_ble_parameters_settings_t *current_para, BOOL_T enable);

/**
 * @brief tuya_ble_cryption_v2_ongoing_set
 *
 * @param[in] enable: enable
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_cryption_v2_ongoing_set(BOOL_T enable);


#ifdef __cplusplus
}
#endif

#endif /* __TUYA_BLE_SECURE_H__ */
