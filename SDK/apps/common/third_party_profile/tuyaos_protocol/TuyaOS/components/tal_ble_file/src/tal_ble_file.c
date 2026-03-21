/**
 * @file tal_ble_file.c
 * @brief This is tal_ble_file file
 * @version 1.0
 * @date 2023-05-17
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

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
#include "tal_ble_file.h"

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
#if defined(TUYA_BLE_FILE_ENABLE) && (TUYA_BLE_FILE_ENABLE==1)

STATIC VOID_T tuya_ble_handle_file_response_evt(INT32_T evt_id, VOID_T *data)
{
    tuya_ble_file_response_t *p_res_data = (tuya_ble_file_response_t *)data;
    UINT16_T file_cmd_type = 0;
    UINT8_T encry_mode = 0;

    if (tuya_ble_pair_rand_valid_get() == 1) {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    } else {
        encry_mode = ENCRYPTION_MODE_KEY_4;
    }

    switch (p_res_data->type) {
    case TUYA_BLE_FILE_INFO:
        file_cmd_type = FRM_FILE_INFOR_REQ;
        break;
    case TUYA_BLE_FILE_OFFSET_REQ:
        file_cmd_type = FRM_FILE_OFFSET_REQ;
        break;
    case TUYA_BLE_FILE_DATA:
        file_cmd_type = FRM_FILE_DATA_REQ;
        break;
    case TUYA_BLE_FILE_END:
        file_cmd_type = FRM_FILE_END_REQ;
        break;
    default:
        break;
    }

    TUYA_BLE_LOG_DEBUG("file_cmd_type:0x%x", file_cmd_type);

    if (file_cmd_type != 0) {
        tuya_ble_comm_data_send(file_cmd_type, 0, p_res_data->p_data, p_res_data->data_len, encry_mode);
    }

    if (p_res_data) {
        tuya_ble_free((UINT8_T *)p_res_data);
    }
}

VOID_T tuya_ble_handle_file_req(UINT16_T cmd, UINT8_T *recv_data, UINT32_T recv_len)
{
    UINT16_T data_len;
    tuya_ble_file_data_type_t cmd_type;
    tuya_ble_cb_evt_param_t event;

    data_len = (recv_data[11] << 8) + recv_data[12];
    if (data_len == 0) {
        return;
    }

    UINT8_T *ble_cb_evt_buffer = (UINT8_T *)tuya_ble_malloc(data_len);
    if (ble_cb_evt_buffer == NULL) {
        TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
        return;
    } else {
        memcpy(ble_cb_evt_buffer, &recv_data[13], data_len);
    }

    switch (cmd) {
    case FRM_FILE_INFOR_REQ:
        cmd_type = TUYA_BLE_FILE_INFO;
        break;
    case FRM_FILE_OFFSET_REQ:
        cmd_type = TUYA_BLE_FILE_OFFSET_REQ;
        break;
    case FRM_FILE_DATA_REQ:
        cmd_type = TUYA_BLE_FILE_DATA;
        break;
    case FRM_FILE_END_REQ:
        cmd_type = TUYA_BLE_FILE_END;
        break;
    default:
        cmd_type = TUYA_BLE_FILE_UNKONWN;
        break;
    }

    event.evt = TUYA_BLE_CB_EVT_FILE_DATA;
    event.file_data.type = cmd_type;
    event.file_data.data_len = data_len;
    event.file_data.p_data = ble_cb_evt_buffer;

    if (tuya_ble_cb_event_send(&event) != 0) {
        tuya_ble_free(ble_cb_evt_buffer);
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_file_req-tuya ble send cb event failed.");
    } else {

    }
}

tuya_ble_status_t tuya_ble_file_response(tuya_ble_file_response_t *p_data)
{
    tuya_ble_custom_evt_t custom_evt;
    tuya_ble_file_response_t *p_res_data = NULL;
    UINT8_T *p_buffer = NULL;

    if (p_data->data_len > TUYA_BLE_SEND_MAX_DATA_LEN) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    p_buffer = tuya_ble_malloc(sizeof(tuya_ble_file_response_t) + p_data->data_len);

    if (p_buffer) {
        p_res_data = (tuya_ble_file_response_t *)p_buffer;
        p_res_data->p_data = p_buffer + sizeof(tuya_ble_file_response_t);
        p_res_data->data_len = p_data->data_len;
        p_res_data->type = p_data->type;
        memcpy(p_res_data->p_data, p_data->p_data, p_data->data_len);
    } else {
        return TUYA_BLE_ERR_NO_MEM;
    }

    custom_evt.evt_id = 0; //res
    custom_evt.data = p_res_data;
    custom_evt.custom_event_handler = tuya_ble_handle_file_response_evt;

    TUYA_BLE_LOG_HEXDUMP_DEBUG("custom_data", (UINT8_T *)p_res_data->p_data, p_res_data->data_len);

    if (tuya_ble_custom_event_send(custom_evt) != 0) {
        tuya_ble_free(p_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

#endif

