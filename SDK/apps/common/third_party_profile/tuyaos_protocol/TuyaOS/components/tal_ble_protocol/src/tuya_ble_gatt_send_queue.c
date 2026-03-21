/**
 * @file tuya_ble_gatt_send_queue.c
 * @brief This is tuya_ble_gatt_send_queue file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "string.h"

#include "tuya_ble_gatt_send_queue.h"
#include "tuya_ble_type.h"
#include "tuya_ble_config.h"
#include "tuya_ble_port.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_main.h"
#include "tuya_ble_log.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/


/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
STATIC tuya_ble_queue_t gatt_send_queue;
STATIC tuya_ble_gatt_send_data_t send_buf[TUYA_BLE_GATT_SEND_DATA_QUEUE_SIZE];
STATIC volatile UINT8_T gatt_queue_flag = 0;

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




VOID_T tuya_ble_gatt_send_queue_init(VOID_T)
{
    gatt_queue_flag = 0;
    tuya_ble_queue_init(&gatt_send_queue, (VOID_T *) send_buf, TUYA_BLE_GATT_SEND_DATA_QUEUE_SIZE, SIZEOF(tuya_ble_gatt_send_data_t));
}

STATIC VOID_T tuya_ble_gatt_send_queue_free(VOID_T)
{
    tuya_ble_gatt_send_data_t data   = {0};
    memset(&data, 0, SIZEOF(tuya_ble_gatt_send_data_t));
    while (tuya_ble_dequeue(&gatt_send_queue, &data) == TUYA_BLE_SUCCESS) {
        if (data.buf) {
            tuya_ble_free(data.buf);
        }
        memset(&data, 0, SIZEOF(tuya_ble_gatt_send_data_t));
    }
    TUYA_BLE_LOG_DEBUG("tuya_ble_gatt_send_queue_free execute.");
}

VOID_T tuya_ble_gatt_send_data_handle(VOID_T *evt)
{
    tuya_ble_gatt_send_data_t data   = {0};
    tuya_ble_evt_param_t event;
    tuya_ble_connect_status_t currnet_connect_status;

    while (tuya_ble_queue_get(&gatt_send_queue, &data) == TUYA_BLE_SUCCESS) {
        currnet_connect_status = tuya_ble_connect_status_get();
        if ((currnet_connect_status == BONDING_UNCONN) || (currnet_connect_status == UNBONDING_UNCONN)) {
            tuya_ble_gatt_send_queue_free();
            break;
        }

        if (tuya_ble_gatt_send_data(data.buf, data.size) == TUYA_BLE_SUCCESS) {
            tuya_ble_free(data.buf);
            tuya_ble_queue_decrease(&gatt_send_queue);
        } else {
            event.hdr.event = TUYA_BLE_EVT_GATT_SEND_DATA;
            event.hdr.event_handler = tuya_ble_gatt_send_data_handle;
            if (tuya_ble_event_send(&event) != 0) {
                tuya_ble_gatt_send_queue_free();
                TUYA_BLE_LOG_ERROR("TUYA_BLE_EVT_GATT_SEND_DATA  error.");
            }

            break;

        }
    }
    if (tuya_ble_get_queue_used(&gatt_send_queue) == 0) {
        tuya_ble_queue_flush(&gatt_send_queue);
        gatt_queue_flag = 0;
    }

}

tuya_ble_status_t tuya_ble_gatt_send_data_enqueue(UINT8_T *p_data, UINT8_T data_len)
{
    tuya_ble_gatt_send_data_t data   = {0};

    data.buf = tuya_ble_malloc(data_len);

    if (data.buf) {
        memcpy(data.buf, p_data, data_len);
        data.size = data_len;
        if (tuya_ble_enqueue(&gatt_send_queue, &data) == TUYA_BLE_SUCCESS) {
            if (gatt_queue_flag == 0) {
                gatt_queue_flag = 1;
                tuya_ble_gatt_send_data_handle(NULL);
            }
            return TUYA_BLE_SUCCESS;
        } else {
            tuya_ble_free(data.buf);
            return TUYA_BLE_ERR_NO_MEM;
        }
    } else {
        return TUYA_BLE_ERR_NO_MEM;
    }

}

UINT32_T tuya_ble_get_gatt_send_queue_used(VOID_T)
{
    return tuya_ble_get_queue_used(&gatt_send_queue);
}

