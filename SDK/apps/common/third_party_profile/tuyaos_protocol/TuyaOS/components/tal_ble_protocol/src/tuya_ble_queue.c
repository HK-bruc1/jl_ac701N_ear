/**
 * @file tuya_ble_queue.c
 * @brief This is tuya_ble_queue file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "string.h"

#include "tuya_ble_queue.h"
#include "tuya_ble_type.h"

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




tuya_ble_status_t tuya_ble_queue_init(tuya_ble_queue_t *q, VOID_T *buf, UINT8_T queue_size, UINT8_T elem_size)
{
    if (buf == NULL || q == NULL) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    q->buf = buf;
    q->size = queue_size;
    q->offset = elem_size;
    q->rd_ptr = 0;
    q->wr_ptr = 0;
    q->used = 0;

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_enqueue(tuya_ble_queue_t *q, VOID_T *in)
{
    if (q->used >= q->size) {
        return TUYA_BLE_ERR_NO_MEM;
    }

    memcpy((UINT8_T *)q->buf + q->wr_ptr * q->offset, in, q->offset);
    q->wr_ptr = (q->wr_ptr + 1) % q->size;
    q->used++;
    /* printf("tuya_ble_enqueue %x %d", (u32)q, q->used); */

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_queue_get(tuya_ble_queue_t *q, VOID_T *out)
{
    /* printf("tuya_ble_queue_get %x %d", (u32)q, q->used); */
    if (q->used > 0) {
        memcpy(out, (UINT8_T *)q->buf + q->rd_ptr * q->offset, q->offset);
        return TUYA_BLE_SUCCESS;
    } else {
        return TUYA_BLE_ERR_NOT_FOUND;
    }
}

tuya_ble_status_t tuya_ble_dequeue(tuya_ble_queue_t *q, VOID_T *out)
{
    if (q->used > 0) {
        memcpy(out, (UINT8_T *)q->buf + q->rd_ptr * q->offset, q->offset);
        q->rd_ptr = (q->rd_ptr + 1) % q->size;
        q->used--;
        /* printf("tuya_ble_dequeue %x %d", (u32)q, q->used); */
        return TUYA_BLE_SUCCESS;
    } else {
        return TUYA_BLE_ERR_NOT_FOUND;
    }
}

VOID_T tuya_ble_queue_decrease(tuya_ble_queue_t *q)
{
    /* printf("tuya_ble_decrease %x %d", (u32)q, q->used); */
    if (q->used > 0) {
        q->rd_ptr = (q->rd_ptr + 1) % q->size;
        q->used--;
    }
}

VOID_T tuya_ble_queue_flush(tuya_ble_queue_t *q)
{
    q->rd_ptr = 0;
    q->wr_ptr = 0;
    q->used = 0;
}

UINT8_T tuya_ble_get_queue_used(tuya_ble_queue_t *q)
{
    return q->used;
}

