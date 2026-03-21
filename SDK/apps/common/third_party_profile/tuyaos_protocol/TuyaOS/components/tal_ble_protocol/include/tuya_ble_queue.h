/**
 * @file tuya_ble_queue.h
 * @brief This is tuya_ble_queue file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_QUEUE_H__
#define __TUYA_BLE_QUEUE_H__

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
typedef struct {
    VOID_T *buf;
    volatile UINT8_T size;
    volatile UINT8_T offset;
    volatile UINT8_T rd_ptr;
    volatile UINT8_T wr_ptr;
    volatile UINT8_T used;
} tuya_ble_queue_t;

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/

/**
 * @brief tuya_ble_queue_init
 *
 * @param[in] *q: *q
 * @param[in] *buf: *buf
 * @param[in] size: size
 * @param[in] elem_size: elem_size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_queue_init(tuya_ble_queue_t *q, VOID_T *buf, UINT8_T queue_size, UINT8_T elem_size);

/**
 * @brief tuya_ble_enqueue
 *
 * @param[in] *q: *q
 * @param[in] *in: *in
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_enqueue(tuya_ble_queue_t *q, VOID_T *in);

/**
 * @brief tuya_ble_queue_get
 *
 * @param[in] *q: *q
 * @param[out] *out: *out
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_queue_get(tuya_ble_queue_t *q, VOID_T *out);

/**
 * @brief tuya_ble_dequeue
 *
 * @param[in] *q: *q
 * @param[out] *out: *out
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_dequeue(tuya_ble_queue_t *q, VOID_T *out);

/**
 * @brief tuya_ble_queue_decrease
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_queue_decrease(tuya_ble_queue_t *q);

/**
 * @brief tuya_ble_queue_flush
 *
 * @param[in] *q: *q
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_queue_flush(tuya_ble_queue_t *q);

/**
 * @brief tuya_ble_get_queue_used
 *
 * @param[in] *q: *q
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tuya_ble_get_queue_used(tuya_ble_queue_t *q);


#ifdef __cplusplus
}
#endif

#endif /* __TUYA_BLE_QUEUE_H__ */

