/**
 * @file tuya_ble_bulkdata.c
 * @brief This is tuya_ble_bulkdata file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "string.h"

#include "tuya_ble_bulkdata.h"
#include "tuya_ble_type.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_api.h"
#include "tuya_ble_port.h"
#include "tuya_ble_main.h"
#include "tuya_ble_internal_config.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_mutli_tsf_protocol.h"
#include "tuya_ble_secure.h"
#include "tuya_ble_main.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_log.h"
#include "tuya_ble_gatt_send_queue.h"

#if defined(TUYA_BLE_FEATURE_BULKDATA_ENABLE) && (TUYA_BLE_FEATURE_BULKDATA_ENABLE == 1)

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




UINT32_T tuya_ble_bulk_data_read_block_size_get(VOID_T)
{
    UINT32_T mtu_length = 0;
    UINT32_T read_block_size = 0;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        read_block_size = 0;
    } else {
        mtu_length = tuya_ble_send_packet_data_length_get();
        if (mtu_length < 100) {
            read_block_size = 256;
        } else if ((mtu_length >= 100) && (mtu_length < 150)) {
            read_block_size = 512;
        } else {
            read_block_size = TUYA_BLE_BULK_DATA_MAX_READ_BLOCK_SIZE;
        }
    }

    return read_block_size;
}

VOID_T tuya_ble_handle_bulk_data_req(UINT16_T cmd, UINT8_T *p_recv_data, UINT32_T recv_data_len)
{
    UINT16_T data_len;
    tuya_ble_cb_evt_param_t event;
    UINT8_T err_code = 0;

    data_len = (p_recv_data[11] << 8) + p_recv_data[12];

    if (data_len == 0) {
        return;
    } else if (p_recv_data[13] != 0) { /*Currently only supports version 0*/
        return;
    } else {
    }

    event.evt = TUYA_BLE_CB_EVT_BULK_DATA;

    switch (cmd) {
    case FRM_BULK_DATA_READ_INFO_REQ:
        event.bulk_req_data.evt = TUYA_BLE_BULK_DATA_EVT_READ_INFO;
        event.bulk_req_data.bulk_type = p_recv_data[14];
        break;

    case FRM_BULK_DATA_READ_DATA_REQ:
        event.bulk_req_data.evt = TUYA_BLE_BULK_DATA_EVT_READ_BLOCK;
        event.bulk_req_data.bulk_type = p_recv_data[14];
        event.bulk_req_data.params.block_data_req_data.block_number = (p_recv_data[15] << 8) + p_recv_data[16];
        break;

    case FRM_BULK_DATA_ERASE_DATA_REQ:
        event.bulk_req_data.evt = TUYA_BLE_BULK_DATA_EVT_ERASE;
        event.bulk_req_data.bulk_type = p_recv_data[14];
        break;

    default:
        err_code = 1;
        break;
    }

    if (err_code == 0) {
        if (tuya_ble_cb_event_send(&event) != 0) {
            TUYA_BLE_LOG_ERROR("tuya_ble_handle_bulk_data_req-tuya ble send cb event failed.");
        }
    }
}

tuya_ble_status_t tuya_ble_bulk_data_response(tuya_ble_bulk_data_response_t *p_data)
{
    tuya_ble_evt_param_t evt;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if ((p_data->evt == TUYA_BLE_BULK_DATA_EVT_SEND_DATA) && (p_data->params.send_res_data.current_block_length > (TUYA_BLE_SEND_MAX_DATA_LEN - 8))) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    evt.hdr.event = TUYA_BLE_EVT_BULK_DATA_RESPONSE;

    memcpy(&evt.bulk_res_data, p_data, SIZEOF(tuya_ble_bulk_data_response_t));

    if (p_data->evt == TUYA_BLE_BULK_DATA_EVT_SEND_DATA) {
        evt.bulk_res_data.params.send_res_data.p_current_block_data = NULL;
        evt.bulk_res_data.params.send_res_data.p_current_block_data = (UINT8_T *)tuya_ble_malloc(p_data->params.send_res_data.current_block_length);
        if (evt.bulk_res_data.params.send_res_data.p_current_block_data == NULL) {
            return TUYA_BLE_ERR_NO_MEM;
        }
        memcpy(evt.bulk_res_data.params.send_res_data.p_current_block_data, p_data->params.send_res_data.p_current_block_data, p_data->params.send_res_data.current_block_length);
    }

    if (tuya_ble_event_send(&evt) != 0) {
        if (p_data->evt == TUYA_BLE_BULK_DATA_EVT_SEND_DATA) {
            tuya_ble_free(evt.bulk_res_data.params.send_res_data.p_current_block_data);
        }

        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

VOID_T tuya_ble_handle_bulk_data_evt(tuya_ble_evt_param_t *p_evt)
{
    UINT16_T bulk_data_cmd = 0;
    UINT8_T buffer[32];
    UINT8_T *p_buf = NULL;
    UINT16_T data_length = 0;
    UINT8_T encry_mode = 0;
    tuya_ble_cb_evt_param_t event;
    UINT8_T err_code = 0;


    if (tuya_ble_pair_rand_valid_get() == 1) {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    } else {
        encry_mode = ENCRYPTION_MODE_KEY_4;
    }

    buffer[0] = 0; // version
    buffer[1] = p_evt->bulk_res_data.bulk_type;

    switch (p_evt->bulk_res_data.evt) {
    case TUYA_BLE_BULK_DATA_EVT_READ_INFO:

        bulk_data_cmd = FRM_BULK_DATA_READ_INFO_RESP;
        buffer[2] = p_evt->bulk_res_data.params.bulk_info_res_data.status;
        buffer[3] = p_evt->bulk_res_data.params.bulk_info_res_data.flag;
        buffer[4] = p_evt->bulk_res_data.params.bulk_info_res_data.bulk_data_length >> 24;
        buffer[5] = p_evt->bulk_res_data.params.bulk_info_res_data.bulk_data_length >> 16;
        buffer[6] = p_evt->bulk_res_data.params.bulk_info_res_data.bulk_data_length >> 8;
        buffer[7] = p_evt->bulk_res_data.params.bulk_info_res_data.bulk_data_length;
        buffer[8] = p_evt->bulk_res_data.params.bulk_info_res_data.bulk_data_crc >> 24;
        buffer[9] = p_evt->bulk_res_data.params.bulk_info_res_data.bulk_data_crc >> 16;
        buffer[10] = p_evt->bulk_res_data.params.bulk_info_res_data.bulk_data_crc >> 8;
        buffer[11] = p_evt->bulk_res_data.params.bulk_info_res_data.bulk_data_crc;
        buffer[12] = p_evt->bulk_res_data.params.bulk_info_res_data.block_data_length >> 8;
        buffer[13] = p_evt->bulk_res_data.params.bulk_info_res_data.block_data_length;
        data_length = 14;
        break;

    case TUYA_BLE_BULK_DATA_EVT_READ_BLOCK:

        bulk_data_cmd = FRM_BULK_DATA_READ_DATA_RESP;
        buffer[2] = p_evt->bulk_res_data.params.block_res_data.status;
        buffer[3] = p_evt->bulk_res_data.params.block_res_data.block_number >> 8;
        buffer[4] = p_evt->bulk_res_data.params.block_res_data.block_number;
        buffer[5] = p_evt->bulk_res_data.params.block_res_data.block_data_length >> 8;
        buffer[6] = p_evt->bulk_res_data.params.block_res_data.block_data_length;
        buffer[7] = p_evt->bulk_res_data.params.block_res_data.max_packet_data_length >> 8;
        buffer[8] = p_evt->bulk_res_data.params.block_res_data.max_packet_data_length;
        buffer[9] = p_evt->bulk_res_data.params.block_res_data.block_data_crc16 >> 8;
        buffer[10] = p_evt->bulk_res_data.params.block_res_data.block_data_crc16;
        data_length = 11;
        break;

    case TUYA_BLE_BULK_DATA_EVT_SEND_DATA :

        bulk_data_cmd = FRM_BULK_DATA_SEND_DATA;
        p_buf = (UINT8_T *)tuya_ble_malloc(p_evt->bulk_res_data.params.send_res_data.current_block_length + 8);
        if (p_buf) {
            p_buf[0] = 0;
            p_buf[1] = p_evt->bulk_res_data.bulk_type;
            p_buf[2] = p_evt->bulk_res_data.params.send_res_data.current_block_number >> 8;
            p_buf[3] = p_evt->bulk_res_data.params.send_res_data.current_block_number;
            p_buf[4] = 0;
            p_buf[5] = 0;
            p_buf[6] = p_evt->bulk_res_data.params.send_res_data.current_block_length >> 8;
            p_buf[7] = p_evt->bulk_res_data.params.send_res_data.current_block_length;
            memcpy(&p_buf[8], p_evt->bulk_res_data.params.send_res_data.p_current_block_data, p_evt->bulk_res_data.params.send_res_data.current_block_length);
            data_length = p_evt->bulk_res_data.params.send_res_data.current_block_length + 8;
        }
        tuya_ble_free(p_evt->bulk_res_data.params.send_res_data.p_current_block_data);
        break;

    case TUYA_BLE_BULK_DATA_EVT_ERASE :

        bulk_data_cmd = FRM_BULK_DATA_ERASE_DATA_RESP;
        buffer[2] = p_evt->bulk_res_data.params.erase_res_data.status;
        data_length = 3;

        break;

    case TUYA_BLE_BULK_DATA_EVT_SYNC_REQ :

        bulk_data_cmd = FRM_BULK_DATA_SYNC_REQUEST;
        buffer[1] = p_evt->bulk_res_data.params.sync_req.request_bulkdata_count;
        if ((buffer[1] >= 1) && (buffer[1] <= 4)) { // 4*7 = 28 + 2 = 30 bytes
            memcpy(&buffer[2], p_evt->bulk_res_data.params.sync_req.p_request_bulkdata_info, buffer[1] * 7);
        }
        data_length = 2 + buffer[1] * 7;

        break;

    default:
        break;
    }

    if (data_length > 0) {
        if (p_evt->bulk_res_data.evt == TUYA_BLE_BULK_DATA_EVT_READ_BLOCK) {
            err_code = tuya_ble_comm_data_send(bulk_data_cmd, 0, buffer, data_length, encry_mode);

            if (p_evt->bulk_res_data.params.block_res_data.status == 0) {
                event.evt = TUYA_BLE_CB_EVT_BULK_DATA;

                event.bulk_req_data.evt = TUYA_BLE_BULK_DATA_EVT_SEND_DATA;
                event.bulk_req_data.bulk_type = p_evt->bulk_res_data.bulk_type;

                event.bulk_req_data.params.send_data_req_data.block_number = p_evt->bulk_res_data.params.block_res_data.block_number;

                if (!err_code) {
                    if (tuya_ble_cb_event_send(&event) != 0) {
                        TUYA_BLE_LOG_ERROR("tuya_ble_handle_bulk_data_evt-tuya ble send cb event failed.");
                    }
                } else {
                    TUYA_BLE_LOG_ERROR("tuya_ble_handle_bulk_data_evt-response read bulk data error.");
                }
            }

        } else if (p_evt->bulk_res_data.evt == TUYA_BLE_BULK_DATA_EVT_SEND_DATA) {
            err_code = tuya_ble_comm_data_send(bulk_data_cmd, 0, p_buf, data_length, encry_mode);
            tuya_ble_free(p_buf);
        } else {
            tuya_ble_comm_data_send(bulk_data_cmd, 0, buffer, data_length, encry_mode);
        }
    }

}

VOID_T tuya_ble_bulk_data_sync_request(UINT8_T request_bulkdata_count, UINT8_T *p_request_bulkdata_info)
{
    tuya_ble_bulk_data_response_t res_data = {0};

    res_data.evt = TUYA_BLE_BULK_DATA_EVT_SYNC_REQ;
    res_data.params.sync_req.request_bulkdata_count = request_bulkdata_count;
    res_data.params.sync_req.p_request_bulkdata_info = p_request_bulkdata_info;

    tuya_ble_bulk_data_response(&res_data);
}

#endif // TUYA_BLE_FEATURE_BULKDATA_ENABLE

