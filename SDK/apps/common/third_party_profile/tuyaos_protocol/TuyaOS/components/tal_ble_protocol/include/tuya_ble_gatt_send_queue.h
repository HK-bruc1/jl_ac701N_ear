/**
 * @file tuya_ble_gatt_send_queue.h
 * @brief This is tuya_ble_gatt_send_queue file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_GATT_SEND_QUEUE_H__
#define __TUYA_BLE_GATT_SEND_QUEUE_H__

#include "tuya_cloud_types.h"
#include "tuya_ble_type.h"
#include "tuya_ble_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/


/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
typedef struct {
    UINT8_T *buf;
    UINT8_T size;
} tuya_ble_gatt_send_data_t;

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/

/**
 * @brief tuya_ble_gatt_send_queue_init
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_gatt_send_queue_init(VOID_T);

/**
 * @brief tuya_ble_gatt_send_data_handle
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_gatt_send_data_handle(VOID_T *evt);

/**
 * @brief tuya_ble_gatt_send_data_enqueue
 *
 * @param[in] *p_data: *p_data
 * @param[in] data_len: data_len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_gatt_send_data_enqueue(UINT8_T *p_data, UINT8_T data_len);


#ifdef __cplusplus
}
#endif

#endif /* __TUYA_BLE_GATT_SEND_QUEUE_H__ */

