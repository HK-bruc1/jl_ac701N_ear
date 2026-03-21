/**
 * @file tal_ble_app_passthrough.c
 * @brief This is tal_ble_app_passthrough file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

/* #include "string.h" */
/* #include "stdio.h" */

#include "system/includes.h"
#include "tal_util.h"
#include "tal_ble_app_passthrough.h"

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

#if ( (TUYA_BLE_PROTOCOL_VERSION_HIGN>=4) && (TUYA_BLE_FEATURE_APP_PASSTHROUGH_ENABLE != 0) )

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define TAL_APP_PASSTHROUGH_DATA_MAX_SIZE (TUYA_BLE_AIR_FRAME_MAX-64) // may longer a little

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
typedef struct {
    uint32_t recv_len;
    uint32_t recv_len_max;
    uint8_t *recv_data;
} tal_app_passthrough_recv_packet;

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
static tal_app_passthrough_recv_packet sg_recv_packet = {0};
static frm_trsmitr_proc_s sg_trsmitr_proc = {0};

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/
static void recv_packet_free(tal_app_passthrough_recv_packet *p_recv_packet);
static uint32_t app_passthrough_data_unpack(frm_trsmitr_proc_s *p_trsmitr_proc, tal_app_passthrough_recv_packet *p_recv_packet, uint8_t *buf, uint32_t size);




void tal_ble_app_passthrough_handler(uint8_t *buf, uint32_t size)
{
    tuya_ble_cb_evt_param_t event;
    uint32_t err_code = 0;
    uint8_t buffer[15];
    uint16_t flag, length;
    uint8_t *p_data_buffer = NULL;

    flag = (buf[0] << 8) | buf[1];

    if (flag & 0x0002) { //bit1:1
        err_code = app_passthrough_data_unpack(&sg_trsmitr_proc, &sg_recv_packet, buf + 2, size - 2);

        buffer[0] = buf[0];
        buffer[1] = buf[1];
        buffer[2] = err_code;
        buffer[3] = sg_trsmitr_proc.subpkg_num >> 8;
        buffer[4] = sg_trsmitr_proc.subpkg_num;
        buffer[5] = sg_trsmitr_proc.subpkg_len >> 8;
        buffer[6] = sg_trsmitr_proc.subpkg_len;
        buffer[7] = sg_recv_packet.recv_len >> 24;
        buffer[8] = sg_recv_packet.recv_len >> 16;
        buffer[9] = sg_recv_packet.recv_len >> 8;
        buffer[10] = sg_recv_packet.recv_len;
        buffer[11] = sg_recv_packet.recv_len_max >> 24;
        buffer[12] = sg_recv_packet.recv_len_max >> 16;
        buffer[13] = sg_recv_packet.recv_len_max >> 8;
        buffer[14] = sg_recv_packet.recv_len_max;

        if (flag & 0x0001) { //bit0
            tuya_ble_comm_data_send(FRM_APP_DOWNSTREAM_PASSTHROUGH_RESP, 0, buffer, 15, ENCRYPTION_MODE_SESSION_KEY);
        }

        if (err_code == 0) { //ְδֹͣδʲôʱͷڴ棿Ͽʱ
            event.evt = TUYA_BLE_CB_EVT_APP_PASSTHROUGH_DATA;
            event.app_passthrough_data.type = (sg_recv_packet.recv_data[0] << 8) | sg_recv_packet.recv_data[1];
            event.app_passthrough_data.data_len = sg_recv_packet.recv_len_max - 2;

            p_data_buffer = (uint8_t *)tuya_ble_malloc(event.app_passthrough_data.data_len);
            if (p_data_buffer == NULL) {
                TAL_PR_ERR("app frame callback data malloc failed.");
            } else {
                memcpy(p_data_buffer, &sg_recv_packet.recv_data[2], event.app_passthrough_data.data_len);
                event.app_passthrough_data.p_data = p_data_buffer;

                if (tuya_ble_cb_event_send(&event) != 0) {
                    tuya_ble_free(p_data_buffer);
                    TAL_PR_ERR("tuya ble send app frame data callback event failed.");
                }
            }

            recv_packet_free(&sg_recv_packet);
        }
    } else { //bit1:0
        length = size - 2;
        buffer[0] = buf[0];
        buffer[1] = buf[1];
        buffer[2] = 0;
        buffer[3] = 0;
        buffer[4] = 0;
        buffer[5] = length >> 8;
        buffer[6] = length;
        buffer[7] = length >> 24;
        buffer[8] = length >> 16;
        buffer[9] = length >> 8;
        buffer[10] = length;
        buffer[11] = length >> 24;
        buffer[12] = length >> 16;
        buffer[13] = length >> 8;
        buffer[14] = length;

        if (flag & 0x0001) { //bit0
            tuya_ble_comm_data_send(FRM_APP_DOWNSTREAM_PASSTHROUGH_RESP, 0, buffer, 15, ENCRYPTION_MODE_SESSION_KEY);
        }

        event.evt = TUYA_BLE_CB_EVT_APP_PASSTHROUGH_DATA;
        event.app_passthrough_data.type = (buf[2] << 8) | buf[3];
        event.app_passthrough_data.data_len = length - 2;

        p_data_buffer = (uint8_t *)tuya_ble_malloc(event.app_passthrough_data.data_len);
        if (p_data_buffer == NULL) {
            TAL_PR_ERR("app frame callback data malloc failed.");
        } else {
            memcpy(p_data_buffer, &buf[4], event.app_passthrough_data.data_len);
            event.app_passthrough_data.p_data = p_data_buffer;

            if (tuya_ble_cb_event_send(&event) != 0) {
                tuya_ble_free(p_data_buffer);
                TAL_PR_ERR("tuya ble send app frame data callback event failed.");
            }
        }
    }
}

tuya_ble_status_t tal_ble_app_passthrough_data_send(tuya_ble_app_passthrough_data_t *p_data)
{
    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if (p_data == NULL) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    if (p_data->p_data == NULL) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    if (p_data->data_len > TAL_APP_PASSTHROUGH_DATA_MAX_SIZE) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    uint8_t *p_buffer = (void *)tuya_ble_malloc(sizeof(tuya_ble_app_passthrough_data_t) + p_data->data_len + 4);
    if (p_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    }

    p_buffer[0] = 0;
    p_buffer[1] = 0;
    p_buffer[2] = p_data->type >> 8;
    p_buffer[3] = p_data->type & 0xFF;
    memcpy(p_buffer + 4, p_data->p_data, p_data->data_len);

    uint8_t status = tuya_ble_comm_data_send(FRM_APP_UPSTREAM_PASSTHROUGH_REQ, 0, p_buffer, p_data->data_len + 4, ENCRYPTION_MODE_SESSION_KEY);
    if (status != 0) {
        TAL_PR_ERR("App frame data send failed, error code = %d.", status);
    }

    if (p_buffer) {
        tuya_ble_free(p_buffer);
    }

    return status;
}

void tal_ble_app_passthrough_disconnect_handler(void)
{
    recv_packet_free(&sg_recv_packet);
}

static void recv_packet_free(tal_app_passthrough_recv_packet *p_recv_packet)
{
    if (p_recv_packet->recv_data) {
        tuya_ble_free(p_recv_packet->recv_data);
        p_recv_packet->recv_data = NULL;
        p_recv_packet->recv_len_max = 0;
        p_recv_packet->recv_len = 0;
    }
}

static uint32_t app_passthrough_data_unpack(frm_trsmitr_proc_s *p_trsmitr_proc, tal_app_passthrough_recv_packet *p_recv_packet, uint8_t *buf, uint32_t size)
{
    static uint32_t offset = 0;
    mtp_ret ret;

    ret = trsmitr_recv_pkg_decode(p_trsmitr_proc, buf, size);
    if (MTP_OK != ret && MTP_TRSMITR_CONTINUE != ret) {
        p_recv_packet->recv_len_max = 0;
        p_recv_packet->recv_len = 0;
        if (p_recv_packet->recv_data) {
            tuya_ble_free(p_recv_packet->recv_data);
            p_recv_packet->recv_data = NULL;
        }

        return 3;  //error
    }

    if (FRM_PKG_FIRST == p_trsmitr_proc->pkg_desc) {
        if (p_recv_packet->recv_data) {
            tuya_ble_free(p_recv_packet->recv_data);
            p_recv_packet->recv_data = NULL;
        }

        p_recv_packet->recv_len_max = get_trsmitr_frame_total_len(p_trsmitr_proc);
        if ((p_recv_packet->recv_len_max > TAL_APP_PASSTHROUGH_DATA_MAX_SIZE) || (p_recv_packet->recv_len_max == 0)) {
            p_recv_packet->recv_len_max = 0;
            p_recv_packet->recv_len = 0;
            TAL_PR_ERR("app frame data total size [%d ]error.", p_recv_packet->recv_len_max);
            return 3;  //Invalid total length
        }

        p_recv_packet->recv_len = 0;
        p_recv_packet->recv_data = tuya_ble_malloc(p_recv_packet->recv_len_max + (4 - (p_recv_packet->recv_len_max & 0x0003))); //Ensure that the length is a multiple of 4.
        if (p_recv_packet->recv_data == NULL) {
            TAL_PR_ERR("app frame data unpack malloc failed.");
            return 3;   //malloc failed.
        }

        memset(p_recv_packet->recv_data, 0, p_recv_packet->recv_len_max);
        offset = 0;
    }

    if ((offset + get_trsmitr_subpkg_len(p_trsmitr_proc)) <= p_recv_packet->recv_len_max) {
        if (p_recv_packet->recv_data) {
            memcpy(p_recv_packet->recv_data + offset, get_trsmitr_subpkg(p_trsmitr_proc), get_trsmitr_subpkg_len(p_trsmitr_proc));
            offset += get_trsmitr_subpkg_len(p_trsmitr_proc);
            p_recv_packet->recv_len = offset;
        } else {
            TAL_PR_ERR("app frame data unpack error.");
            p_recv_packet->recv_len_max = 0;
            p_recv_packet->recv_len = 0;
            return 2;  //current packet error.
        }
    } else {
        ret = MTP_INVALID_PARAM;
        TAL_PR_ERR("app frame data unpack[] error:MTP_INVALID_PARAM");
        recv_packet_free(p_recv_packet);
        return 3;
    }

    if (ret == MTP_OK) {
        offset = 0;
        TAL_PR_INFO("app passthrough data unpack[%d]", p_recv_packet->recv_len);

        return 0;
    } else if (ret == MTP_TRSMITR_CONTINUE) {
        return 1;
    } else {
        return 3;
    }
}

#else

tuya_ble_status_t tal_ble_app_passthrough_data_send(tuya_ble_app_passthrough_data_t *p_data)
{
    return OPRT_NOT_SUPPORTED;
}

#endif // TUYA_BLE_FEATURE_APP_PASSTHROUGH_ENABLE

