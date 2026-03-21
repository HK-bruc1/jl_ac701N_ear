/**
 * @file tuya_ble_uart_common.h
 * @brief This is tuya_ble_uart_common file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_UART_COMMON_H__
#define __TUYA_BLE_UART_COMMON_H__

#include "tuya_ble_internal_config.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define tuya_ble_uart_common_mcu_ota_data_from_ble_handler(cmd, recv_data, recv_len) tuya_ble_uart_common_mcu_or_ext_module_ota_data_from_ble_handler(1, cmd, recv_data, recv_len)

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/

#if defined(TUYA_BLE_FEATURE_UART_COMMON_ENABLE) && (TUYA_BLE_FEATURE_UART_COMMON_ENABLE == 1)

/**
 * @brief   Function for transmit ble data from peer devices to tuya sdk.
 *
 * @note    This function must be called from where the ble data is received.
 *.
 * */

/**
 * @brief tuya_ble_uart_common_process
 *
 * @param[in] *p_in_data: *p_in_data
 * @param[in] in_len: in_len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_uart_common_process(UINT8_T *p_in_data, UINT16_T in_len);

/**
 * @brief tuya_ble_uart_common_mcu_ota_data_from_ble_handler
 *
 * @param[in] cmd: cmd
 * @param[in] UINT8_T*recv_data: UINT8_T*recv_data
 * @param[in] recv_len: recv_len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_uart_common_mcu_or_ext_module_ota_data_from_ble_handler(UINT8_T channel, UINT16_T cmd, UINT8_T *recv_data, UINT32_T recv_len);

/**
 * @brief tuya_ble_uart_common_mcu_ota_disconnect_handler
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_uart_common_mcu_ota_disconnect_handler(VOID_T);

/**
 * @brief tuya_ble_send_file_data_from_ble_handler
 *
 * @param[in] cmd: cmd
 * @param[in] p_data: p_data
 * @param[in] data_len: data_len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_send_file_data_from_ble_handler(UINT8_T cmd, UINT8_T *p_data, UINT16_T data_len);

#endif   // TUYA_BLE_FEATURE_UART_COMMON_ENABLE


#ifdef __cplusplus
}
#endif

#endif /* __TUYA_BLE_UART_COMMON_H__ */
