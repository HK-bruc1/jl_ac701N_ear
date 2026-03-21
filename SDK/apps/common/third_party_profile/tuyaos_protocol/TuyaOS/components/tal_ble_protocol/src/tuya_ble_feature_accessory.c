/**
 * @file tuya_ble_feature_accessory.c
 * @brief This is tuya_ble_feature_accessory file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tuya_ble_type.h"
#include "tal_memory.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_api.h"
#include "tuya_ble_port.h"
#include "tuya_ble_main.h"
#include "tuya_ble_internal_config.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_mutli_tsf_protocol.h"
#include "tal_util.h"
#include "tuya_ble_secure.h"
#include "tuya_ble_main.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_log.h"
#include "tuya_ble_gatt_send_queue.h"
#include "tuya_ble_feature_accessory.h"

#if ((TUYA_BLE_PROTOCOL_VERSION_HIGN >= 4 && TUYA_BLE_PROTOCOL_VERSION_LOW >= 5) || (TUYA_BLE_PROTOCOL_VERSION_HIGN >= 5))

#if (TUYA_BLE_ACCESSORY_MOUNT_SUPPORTED != 0)

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




VOID_T tuya_ble_handle_accessory_info_reading(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T *p_attach_data = NULL;
    tuya_ble_cb_evt_param_t event;
    UINT8_T attach_len = 0;
    UINT16_T data_len;

    data_len = (recv_data[11] << 8) | recv_data[12];

    event.evt = TUYA_BLE_CB_EVT_ACCESSORY_INFO_READING;

    attach_len = recv_data[14];

    if (data_len < 2 + attach_len) {
        TUYA_BLE_LOG_ERROR("frame error,could not be processed.");
        return;
    }

    if (attach_len > 0) {
        p_attach_data = (UINT8_T *)tuya_ble_malloc(attach_len);
        if (p_attach_data == NULL) {
            TUYA_BLE_LOG_ERROR("p_attach_data malloc failed.");
            return;
        } else {
            memcpy(p_attach_data, recv_data + 15, attach_len);
        }
    }

    event.accessory_info_reading_data.attach_len = attach_len;
    event.accessory_info_reading_data.attach_data = p_attach_data;

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya ble send cb event [accessory info reading] failed.");
        tuya_ble_free(p_attach_data);
    }
}

tuya_ble_status_t tuya_ble_accessory_info_report(UINT8_T *p_data, UINT32_T len)
{
    UINT8_T *ble_evt_buffer = NULL;
    tuya_ble_evt_param_t evt;

    if (len > TUYA_BLE_SEND_MAX_DATA_LEN) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    ble_evt_buffer = tuya_ble_malloc(len);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        memcpy(ble_evt_buffer, p_data, len);
    }
    evt.hdr.event = TUYA_BLE_EVT_ACCESSORY_INFO_REPORT;
    evt.accessory_info_report_data.data_len = len;
    evt.accessory_info_report_data.p_data = ble_evt_buffer;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

VOID_T tuya_ble_handle_accessory_info_report_evt(tuya_ble_evt_param_t *evt)
{
    UINT8_T encry_mode = 0;

    if (tuya_ble_pair_rand_valid_get() == 1) {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    } else {
        encry_mode = ENCRYPTION_MODE_KEY_4;
    }

    tuya_ble_comm_data_send(FRM_ACCESSORY_INFO_REPORT_REQ, 0, evt->accessory_info_report_data.p_data, evt->accessory_info_report_data.data_len, encry_mode);

    if (evt->accessory_info_report_data.p_data) {
        tuya_ble_free(evt->accessory_info_report_data.p_data);
    }
}

VOID_T tuya_ble_handle_accessory_info_report_response(UINT8_T *recv_data, UINT16_T recv_len)
{
    tuya_ble_cb_evt_param_t event;
    UINT16_T data_len = recv_data[11] << 8 | recv_data[12];

    if (data_len < 1) {
        TUYA_BLE_LOG_ERROR("frame error,could not be processed.");
        return;
    }

    event.evt = TUYA_BLE_CB_EVT_ACCESSORY_INFO_REPORT_RESPONSE;

    event.accessory_info_report_response_data.status = recv_data[13];

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_accessory_info_report_response-tuya ble send cb event failed.");
    } else {

    }
}

VOID_T tuya_ble_handle_accessory_active_info_received(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T *p_active_info_data = NULL;
    UINT8_T *p_accessory_info = NULL;
    UINT32_T ack_sn = 0;
    UINT16_T data_len = 0;
    UINT16_T cmp_len = 0;
    UINT8_T accessory_num = 0;
    UINT8_T i = 0;
    UINT8_T ret = 0;
    tuya_ble_cb_evt_param_t event;

    data_len = (recv_data[11] << 8) | recv_data[12];

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    accessory_num = recv_data[14];

    p_accessory_info = &recv_data[15];

    for (i = 0; i < accessory_num; i++) {
        cmp_len = cmp_len + 3; /*3 = SHORT_ID, UUID_LEN*/

        if (cmp_len + 2 >= data_len) { /*2 = VERSION, ACCESSORY_NUM*/
            /*UUID empty.*/
            ret = 1;
            break;
        }
        cmp_len = cmp_len + p_accessory_info[cmp_len - 1]; /*add uuid_len.*/

        cmp_len = cmp_len + 1; /*add ACTIVE_STATE.*/
    }
    cmp_len = cmp_len + 2; /*2 = VERSION, ACCESSORY_NUM*/

    if (ret || data_len != cmp_len) { //Only checks the correctness of the data format, with up-throw application layer processing.
        TUYA_BLE_LOG_ERROR("frame error,could not be processed.");
        ret = 1;
        tuya_ble_comm_data_send(FRM_ACCESSORY_ACTIVE_INFO_RECEIVED_RESP, ack_sn, &ret, 1, ENCRYPTION_MODE_SESSION_KEY);
        return;
    }

    event.evt = TUYA_BLE_CB_EVT_ACCESSORY_ACTIVE_INFO_RECEIVED;

    if (data_len > 1) {
        p_active_info_data = (UINT8_T *)tuya_ble_malloc(data_len - 1);
        if (p_active_info_data == NULL) {
            TUYA_BLE_LOG_ERROR("p_active_info_data malloc failed.");
            ret = 1;
            tuya_ble_comm_data_send(FRM_ACCESSORY_ACTIVE_INFO_RECEIVED_RESP, ack_sn, &ret, 1, ENCRYPTION_MODE_SESSION_KEY);
            return;
        } else {
            memcpy(p_active_info_data, recv_data + 14, data_len - 1);
        }
    }

    event.accessory_active_info_data.active_info_len = data_len - 1;
    event.accessory_active_info_data.active_info_data = p_active_info_data;

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya ble send cb event [accessory info reading] failed.");
        tuya_ble_free(p_active_info_data);
        ret = 1;
        tuya_ble_comm_data_send(FRM_ACCESSORY_ACTIVE_INFO_RECEIVED_RESP, ack_sn, &ret, 1, ENCRYPTION_MODE_SESSION_KEY);
    } else {
        ret = 0;
        tuya_ble_comm_data_send(FRM_ACCESSORY_ACTIVE_INFO_RECEIVED_RESP, ack_sn, &ret, 1, ENCRYPTION_MODE_SESSION_KEY);
    }
}

VOID_T tuya_ble_handle_accessory_ota_req(UINT16_T cmd, UINT8_T *recv_data, UINT32_T recv_len)
{
    UINT8_T *ble_cb_evt_buffer = NULL;
    UINT16_T data_len;
    tuya_ble_cb_evt_param_t event;
    tuya_ble_accesory_ota_data_type_t cmd_type;

    data_len = (recv_data[11] << 8) + recv_data[12];

    if (data_len < 5) {
        TUYA_BLE_LOG_ERROR("Invalied data len.");
        return;
    }

    event.evt = TUYA_BLE_CB_EVT_ACCESSORY_OTA_DATA;

    ble_cb_evt_buffer = (UINT8_T *)tuya_ble_malloc(data_len);
    if (ble_cb_evt_buffer == NULL) {
        TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
        return;
    } else {
        memcpy(ble_cb_evt_buffer, &recv_data[13], data_len);
    }

    switch (cmd) {
    case FRM_ACCESSORY_OTA_START_REQ:
        cmd_type = TUYA_BLE_ACCESSORY_OTA_REQ;
        break;
    case FRM_ACCESSORY_OTA_FILE_INFOR_REQ:
        cmd_type = TUYA_BLE_ACCESSORY_OTA_FILE_INFO;
        break;
    case FRM_ACCESSORY_OTA_FILE_OFFSET_REQ:
        cmd_type = TUYA_BLE_ACCESSORY_OTA_FILE_OFFSET_REQ;
        break;
    case FRM_ACCESSORY_OTA_DATA_REQ:
        cmd_type = TUYA_BLE_ACCESSORY_OTA_DATA;
        break;
    case FRM_ACCESSORY_OTA_END_REQ:
        cmd_type = TUYA_BLE_ACCESSORY_OTA_END;
        break;
    default:
        cmd_type = TUYA_BLE_ACCESSORY_OTA_UNKONWN;
        break;
    }

    event.accessory_ota_data.type = cmd_type;
    event.accessory_ota_data.data_len = data_len;
    event.accessory_ota_data.p_data = ble_cb_evt_buffer;

    if (tuya_ble_cb_event_send(&event) != 0) {
        tuya_ble_free(ble_cb_evt_buffer);
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_accessory_ota_req-tuya ble send cb event failed.");
    } else {

    }
}

#else

VOID_T tuya_ble_handle_accessory_info_report_evt(tuya_ble_evt_param_t *evt)
{

}

#endif // (TUYA_BLE_ACCESSORY_MOUNT_SUPPORTED != 0)

#endif // ((TUYA_BLE_PROTOCOL_VERSION_HIGN >= 4 && TUYA_BLE_PROTOCOL_VERSION_LOW >= 5) || (TUYA_BLE_PROTOCOL_VERSION_HIGN >= 5))

