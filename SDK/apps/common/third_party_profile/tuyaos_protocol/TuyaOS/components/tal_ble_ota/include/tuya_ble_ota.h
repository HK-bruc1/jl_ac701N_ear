/**
 * @file tuya_ble_ota.h
 * @brief This is tuya_ble_ota file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_OTA_H__
#define __TUYA_BLE_OTA_H__

#include "tuya_cloud_types.h"
#include "tuya_ble_type.h"
#include "vendor/jl_ble/tuyaos/include/board.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(TUYA_BLE_FEATURE_OTA_ENABLE) && (TUYA_BLE_FEATURE_OTA_ENABLE == 1)

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#ifndef TUYA_BLE_OTA_PKG_LEN
#ifndef BOARD_BLE_OTA_PKG_LEN
#define TUYA_BLE_OTA_PKG_LEN         1024
#else
#define TUYA_BLE_OTA_PKG_LEN         BOARD_BLE_OTA_PKG_LEN
#endif
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
 * @brief tuya_ble_ota_init
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tuya_ble_ota_init(VOID_T);

/**
 * @brief tuya_ble_ota_handler
 *
 * @param[in] ota: ota
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tuya_ble_ota_handler(tuya_ble_ota_data_t *ota);

/**
 * @brief tuya_ble_ota_disconn_handler
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tuya_ble_ota_disconn_handler(VOID_T);

/**
 * @brief tuya_ble_ota_set_status
 *
 * @param[in] status: status
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_ota_set_status(INT32_T status);

/**
 * @brief tuya_ble_ota_get_status
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
INT32_T tuya_ble_ota_get_status(VOID_T);

#if defined(TUYA_BLE_OTA_SIGNATURE_ENABLE) && (TUYA_BLE_OTA_SIGNATURE_ENABLE == 1)

/**
 * @brief tuya_ble_ota_sha256_create_init
 *
 * @param[in] p_ctx: p_ctx
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_ble_ota_sha256_create_init(void **p_ctx);

/**
 * @brief tuya_ble_ota_sha256_free
 *
 * @param[in] ctx: ctx
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_ble_ota_sha256_free(void *ctx);

/**
 * @brief tuya_ble_ota_sha256_starts_ret
 *
 * @param[in] ctx: ctx
 * @param[in] is224: is224
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_ble_ota_sha256_starts_ret(void *ctx, int32_t is224);

/**
 * @brief tuya_ble_ota_sha256_update_ret
 *
 * @param[in] ctx: ctx
 * @param[in] *input: *input
 * @param[in] ilen: ilen
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_ble_ota_sha256_update_ret(void *ctx, const uint8_t *input, size_t ilen);

/**
 * @brief tuya_ble_ota_sha256_finish_ret
 *
 * @param[in] ctx: ctx
 * @param[out] output[32]: output[32]
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_ble_ota_sha256_finish_ret(void *ctx, uint8_t output[32]);

#endif // TUYA_BLE_OTA_SIGNATURE_ENABLE

#endif // TUYA_BLE_FEATURE_OTA_ENABLE


#ifdef __cplusplus
}
#endif

#endif /* __TUYA_BLE_OTA_H__ */

