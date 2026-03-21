/**
 * @file tal_bluetooth.c
 * @brief This is tal_bluetooth file
 * @version 1.0
 * @date 2024-03-13
 *
 * @copyright Copyright 2024-2024 Tuya Inc. All Rights Reserved.
 *
 */

#include "tal_bluetooth.h"
#include <string.h>                     // [20220215] Use Standard C Library.
#include "tal_bluetooth_def.h"
#include "tal_bluetooth_mesh_def.h"
#include "tkl_bluetooth_def.h"
#include "tkl_bluetooth.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define TAL_COMMON_SERVICE_INDEX        (0)

#define TAL_COMMON_SERVICE_MAX_NUM      (1)
#define TAL_COMMON_CHAR_MAX_NUM         (3)

#ifndef TAL_BLE_SERVICE_VERSION
#define TAL_BLE_SERVICE_VERSION         (2)     // Default Service is version 2: Follow the latest service.
#endif

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
/**< Use TKL Definitions */
STATIC TKL_BLE_GATTS_PARAMS_T           tkl_ble_gatt_service = {0};
STATIC TKL_BLE_SERVICE_PARAMS_T         tkl_ble_common_service[4] = {0};    // [20220531] Remove TKL Macro Use
STATIC TKL_BLE_CHAR_PARAMS_T            tkl_ble_common_char[6] = {0};       // [20220531] Remove TKL Macro Use
STATIC USHORT_T                         tkl_ble_common_connect_handle = TKL_BLE_GATT_INVALID_HANDLE;

/**< Use TAL Definitions, TAL Only Support One GATT Link at one time, follow Bluetooth Spec. */
STATIC UCHAR_T                          tal_ble_disc_svc_busy = 0;
STATIC TAL_BLE_PEER_INFO_T              tal_ble_peer = {0};
STATIC TAL_BLE_EVT_FUNC_CB              tal_ble_event_callback;

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/
STATIC USHORT_T tal_ble_uuid16_convert(TKL_BLE_UUID_T *p_uuid)
{
    USHORT_T uuid16 = 0xFFFF;
    UCHAR_T uuid128[16] = TAL_BLE_SPECIFIC_SVC_UUID128;
    UCHAR_T *p_uuid128 = NULL;

    if (p_uuid->uuid_type == TKL_BLE_UUID_TYPE_16) {
        return p_uuid->uuid.uuid16;
    } else if (p_uuid->uuid_type == TKL_BLE_UUID_TYPE_32) {
        /**< Not Support */
        return 0xFFFF;
    } else if (p_uuid->uuid_type == TKL_BLE_UUID_TYPE_128) {
        p_uuid128 = p_uuid->uuid.uuid128;
        /**< For special tuya mesh device, we need to find the specific uuid */
        if (memcmp(&uuid128[2], &p_uuid128[2], 14) == 0) {
            uuid16 = p_uuid128[1] << 8 | p_uuid128[0];
        } else {
            uuid16 = p_uuid128[13] << 8 | p_uuid128[12];
        }
        return uuid16;
    }

    return 0xFFFF;
}

STATIC OPERATE_RET tal_ble_svc_handle_assign(TAL_BLE_PEER_INFO_T *peer_info, USHORT_T char_uuid16, USHORT_T start_handle, USHORT_T end_handle)
{
    OPERATE_RET ret = OPRT_NOT_FOUND;
    UCHAR_T svc_index = peer_info->service_num;

    switch (char_uuid16) {
    case TAL_BLE_CMD_SERVICE_UUID_V1:
        peer_info->service[svc_index].uuid16 = char_uuid16;
        peer_info->service[svc_index].start_handle = start_handle;
        peer_info->service[svc_index].end_handle = end_handle;
        peer_info->flag |= TAL_BLE_SVC_UUID_V1;
        peer_info->service_num ++;
        ret = OPRT_OK;
        break;
    case TAL_BLE_CMD_SERVICE_UUID_V2:
        peer_info->service[svc_index].uuid16 = char_uuid16;
        peer_info->service[svc_index].start_handle = start_handle;
        peer_info->service[svc_index].end_handle = end_handle;
        peer_info->flag |= TAL_BLE_SVC_UUID_V2;
        peer_info->service_num ++;
        ret = OPRT_OK;
        break;
    case TAL_MESH_DEVICE_OTA_SVC_UUID:
        peer_info->service[svc_index].uuid16 = char_uuid16;
        peer_info->service[svc_index].start_handle = start_handle;
        peer_info->service[svc_index].end_handle = end_handle;
        peer_info->flag |= TAL_BLE_SVC_UUID_MESH_OTA;
        peer_info->service_num ++;
        ret = OPRT_OK;
        break;
    case TAL_MESH_DEVICE_INFO_SVC_UUID:
        peer_info->service[svc_index].uuid16 = char_uuid16;
        peer_info->service[svc_index].start_handle = start_handle;
        peer_info->service[svc_index].end_handle = end_handle;
        peer_info->flag |= TAL_BLE_SVC_UUID_MESH_VER;
        peer_info->service_num ++;
        ret = OPRT_OK;
        break;
    case TAL_MESH_PROXY_DEVICE_SVC_UUID:
        peer_info->service[svc_index].uuid16 = char_uuid16;
        peer_info->service[svc_index].start_handle = start_handle;
        peer_info->service[svc_index].end_handle = end_handle;
        peer_info->flag |= TAL_BLE_SVC_UUID_MESH_PROXY;
        peer_info->service_num ++;
        ret = OPRT_OK;
        break;
    }

    return ret;
}

STATIC OPERATE_RET tal_ble_char_handle_discovery(TAL_BLE_PEER_INFO_T *peer_info, USHORT_T svc_uuid16)
{
    UCHAR_T i = 0;
    OPERATE_RET ret = OPRT_NOT_FOUND;

    for (i = 0; i < peer_info->service_num; i ++) {
        if (peer_info->service[i].uuid16 == svc_uuid16) {
            ret = tkl_ble_gattc_all_char_discovery(peer_info->conn_handle, peer_info->service[i].start_handle, peer_info->service[i].end_handle);
            return ret;
        }
    }

    return ret;
}

STATIC OPERATE_RET tal_ble_char_handle_assign(TAL_BLE_PEER_INFO_T *peer_info, USHORT_T char_uuid16, USHORT_T char_handle)
{
    OPERATE_RET ret = OPRT_NOT_FOUND;

    switch (char_uuid16) {
    case TAL_BLE_CMD_WRITE_CHAR_UUID_V1:
    case TAL_BLE_CMD_WRITE_CHAR_UUID_V2:
        peer_info->char_handle[TAL_COMMON_WRITE_CHAR_INDEX]   = char_handle;
        ret = OPRT_OK;
        break;
    case TAL_BLE_CMD_NOTIFY_CHAR_UUID_V1:
    case TAL_BLE_CMD_NOTIFY_CHAR_UUID_V2:
        peer_info->char_handle[TAL_COMMON_NOTIFY_CHAR_INDEX]  = char_handle;
        ret = OPRT_OK;
        break;
    case TAL_BLE_CMD_READ_CHAR_UUID_V2:
        peer_info->char_handle[TAL_COMMON_READ_CHAR_INDEX]    = char_handle;
        ret = OPRT_OK;
        break;
    case TAL_MESH_OTA_WRITE_CHAR_UUID:
        peer_info->char_handle[TAL_MESH_OTA_WRITE_CHAR_INDEX] = char_handle;
        ret = OPRT_OK;
        break;
    case TAL_MESH_FW_VERSION_READ_CHAR_UUID:
        peer_info->char_handle[TAL_MESH_FW_READ_CHAR_INDEX]   = char_handle;
        ret = OPRT_OK;
        break;
    case TAL_MESH_PROXY_WRITE_CHAR_UUID:
        peer_info->char_handle[TAL_MESH_PROXY_WRITE_CHAR_INDEX] = char_handle;
        ret = OPRT_OK;
        break;
    }

    return ret;
}

STATIC VOID_T tkl_ble_kernel_gap_event_callback(TKL_BLE_GAP_PARAMS_EVT_T *p_event)
{
    TAL_BLE_EVT_PARAMS_T tal_event;

    if (p_event == NULL) {
        return;
    }

    memset(&tal_event, 0, sizeof(TAL_BLE_EVT_PARAMS_T));

    switch (p_event->type) {
    case TKL_BLE_EVT_STACK_INIT: {
        tal_event.type              = TAL_BLE_STACK_INIT;
        tal_event.ble_event.init    = p_event->result;
    }
    break;

    /**< Deinit Event Callback From Ble Adapter */
    case TKL_BLE_EVT_STACK_DEINIT: {
        tal_event.type              = TAL_BLE_STACK_DEINIT;
        tal_event.ble_event.init    = p_event->result;
    }
    break;

    /**< Reset Event Callback From Ble Adapter */
    case TKL_BLE_EVT_STACK_RESET: {
        tal_event.type              = TAL_BLE_STACK_RESET;
        tal_event.ble_event.init    = p_event->result;
    }
    break;

    case TKL_BLE_GAP_EVT_CONNECT: {
        tal_event.ble_event.connect.result  = p_event->result;
        if (p_event->gap_event.connect.role  == TKL_BLE_ROLE_SERVER) {
            tal_event.type = TAL_BLE_EVT_PERIPHERAL_CONNECT;

            tkl_ble_common_connect_handle                   = p_event->conn_handle;
            tal_event.ble_event.connect.peer.conn_handle    = p_event->conn_handle;

            tal_event.ble_event.connect.peer.char_handle[TAL_COMMON_WRITE_CHAR_INDEX]   = tkl_ble_common_char[TAL_COMMON_WRITE_CHAR_INDEX].handle;
            tal_event.ble_event.connect.peer.char_handle[TAL_COMMON_NOTIFY_CHAR_INDEX]  = tkl_ble_common_char[TAL_COMMON_NOTIFY_CHAR_INDEX].handle;
            tal_event.ble_event.connect.peer.char_handle[TAL_COMMON_READ_CHAR_INDEX]    = tkl_ble_common_char[TAL_COMMON_READ_CHAR_INDEX].handle;
        } else {
            if (p_event->result != OPRT_OK) {
                tal_event.type                              = TAL_BLE_EVT_CONN_GATT;
                tal_ble_peer.conn_handle                    = TKL_BLE_GATT_INVALID_HANDLE;
                tal_event.ble_event.connect.result          = OPRT_OS_ADAPTER_BLE_GATT_CONN_FAILED;
                memcpy(&tal_event.ble_event.connect.peer, &tal_ble_peer, sizeof(TAL_BLE_PEER_INFO_T));
            } else {
                tal_ble_peer.conn_handle                    = p_event->conn_handle;
                tal_event.type                              = TAL_BLE_EVT_CONN_GATT;

                // [20230602] 如果当前出现成功的连接事件与发起连接的设备不匹配，我们将主动断开并返回错误码
                if (memcmp(tal_ble_peer.peer_addr.addr, p_event->gap_event.connect.peer_addr.addr, 6)) {
                    tkl_ble_gap_disconnect(p_event->conn_handle, TKL_BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
                    tal_ble_peer.conn_handle                    = TKL_BLE_GATT_INVALID_HANDLE;
                    tal_event.ble_event.connect.result          = OPRT_OS_ADAPTER_BLE_GATT_CONN_FAILED;
                } else {
                    tal_event.ble_event.connect.result          = OPRT_OK;
                    memcpy(&tal_event.ble_event.connect.peer,  &tal_ble_peer, sizeof(TAL_BLE_PEER_INFO_T));
                    if (!tal_ble_peer.service_filter) {
                        if (tkl_ble_gattc_all_service_discovery(p_event->conn_handle) != 0) {  // Try To Discovery All Service, And Find the correct service
                            // [20230217] 由于我们是通过连接事件上来后立即下发发现，当在这个阶段发现服务失败后，我们通过TAL_BLE_EVT_CONN_GATT上报OPRT_OS_ADAPTER_BLE_SVC_DISC_FAILED
                            // tal_event.type = TAL_BLE_EVT_CENTRAL_CONNECT_DISCOVERY;
                            tal_event.ble_event.connect.result = OPRT_OS_ADAPTER_BLE_SVC_DISC_FAILED;
                        } else {
                            /* We will still be in service finding procedure. Should not report any states into TAL Application */
                            tal_ble_disc_svc_busy = 1;
                            /* [20221212][TimCheng] We can return gatt-connected event into gateway, also report connect-handle*/
                            // return;
                        }
                    }
                }
            }
        }
    }
    break;

    case TKL_BLE_GAP_EVT_DISCONNECT: {
        if (p_event->gap_event.disconnect.role == TKL_BLE_ROLE_SERVER && p_event->conn_handle == tkl_ble_common_connect_handle) {
            tkl_ble_common_connect_handle = TKL_BLE_GATT_INVALID_HANDLE;
        } else if (p_event->gap_event.disconnect.role == TKL_BLE_ROLE_CLIENT && p_event->conn_handle == tal_ble_peer.conn_handle
                   && tal_ble_disc_svc_busy) {
            /* [20220526][TimCheng]
                当已存在GATT连接，发现服务过程中突然断开时，我们可能无法收到失败的服务发现，但是会收到断开事件，此时我们需要上报该情况到业务端。
                触发该条件仅存在于服务发现过程中且先收到断开事件，当已上报服务发现状态到业务端时，我们无需重复上报服务发现失败。
            */
            tal_ble_disc_svc_busy = 0;
            tal_event.type = TAL_BLE_EVT_CENTRAL_CONNECT_DISCOVERY;
            tal_event.ble_event.connect.result = OPRT_OS_ADAPTER_BLE_SVC_DISC_FAILED;
            memcpy(&tal_event.ble_event.connect.peer, &tal_ble_peer, sizeof(TAL_BLE_PEER_INFO_T));

            if (tal_ble_event_callback) {
                tal_ble_event_callback(&tal_event);
            }
            return;
        }

        tal_event.type                                      = TAL_BLE_EVT_DISCONNECT;
        tal_event.ble_event.disconnect.peer.conn_handle     = p_event->conn_handle;
        tal_event.ble_event.disconnect.reason               = p_event->gap_event.disconnect.reason;
    }
    break;

    case TKL_BLE_GAP_EVT_ADV_REPORT: {
        tal_event.type                                      = TAL_BLE_EVT_ADV_REPORT;
        tal_event.ble_event.adv_report.rssi                 = p_event->gap_event.adv_report.rssi;
        tal_event.ble_event.adv_report.data_len             = (UCHAR_T)p_event->gap_event.adv_report.data.length;
        tal_event.ble_event.adv_report.p_data               = p_event->gap_event.adv_report.data.p_data;
        memcpy(tal_event.ble_event.adv_report.peer_addr.addr, p_event->gap_event.adv_report.peer_addr.addr, 6);

        if (p_event->gap_event.adv_report.adv_type == TKL_BLE_ADV_DATA) {
            tal_event.ble_event.adv_report.adv_type = TAL_BLE_ADV_DATA;
        } else if (p_event->gap_event.adv_report.adv_type == TKL_BLE_RSP_DATA) {
            tal_event.ble_event.adv_report.adv_type = TAL_BLE_RSP_DATA;
        } else if (p_event->gap_event.adv_report.adv_type == TKL_BLE_ADV_RSP_DATA) {
            tal_event.ble_event.adv_report.adv_type = TAL_BLE_ADV_RSP_DATA;
        } else if (p_event->gap_event.adv_report.adv_type == TKL_BLE_NONCONN_ADV_DATA) {
            tal_event.ble_event.adv_report.adv_type = TAL_BLE_NONCONN_ADV_DATA;
        } else if (p_event->gap_event.adv_report.adv_type == TKL_BLE_EXTENDED_ADV_DATA) {
            tal_event.ble_event.adv_report.adv_type = TAL_BLE_EXTENDED_ADV_DATA;
        }

        if (p_event->gap_event.adv_report.peer_addr.type == TKL_BLE_GAP_ADDR_TYPE_PUBLIC) {
            tal_event.ble_event.adv_report.peer_addr.type   = TAL_BLE_ADDR_TYPE_PUBLIC;
        } else if (p_event->gap_event.adv_report.peer_addr.type == TKL_BLE_GAP_ADDR_TYPE_RANDOM) {
            tal_event.ble_event.adv_report.peer_addr.type   = TAL_BLE_ADDR_TYPE_RANDOM;
        } else {
            /**< Currently, we not support other types. And Tuya Bluetooth Device will not use other types. */
            return;
        }
    }
    break;

    case TKL_BLE_GAP_EVT_CONN_PARAM_REQ: {
        tal_event.type                                          = TAL_BLE_EVT_CONN_PARAM_REQ;
        tal_event.ble_event.conn_param.conn.conn_sup_timeout    = p_event->gap_event.conn_param.conn_sup_timeout;
        tal_event.ble_event.conn_param.conn.max_conn_interval   = p_event->gap_event.conn_param.conn_interval_max;
        tal_event.ble_event.conn_param.conn.min_conn_interval   = p_event->gap_event.conn_param.conn_interval_min;
        tal_event.ble_event.conn_param.conn.latency             = p_event->gap_event.conn_param.conn_latency;
        tal_event.ble_event.conn_param.conn_handle              = p_event->conn_handle;
    }
    break;

    case TKL_BLE_GAP_EVT_CONN_PARAM_UPDATE: {
        tal_event.type                                      = TAL_BLE_EVT_CONN_PARAM_UPDATE;
        tal_event.ble_event.conn_param.conn_handle          = p_event->conn_handle;

        tal_event.ble_event.conn_param.conn.conn_sup_timeout    = p_event->gap_event.conn_param.conn_sup_timeout;
        tal_event.ble_event.conn_param.conn.max_conn_interval   = p_event->gap_event.conn_param.conn_interval_max;
        tal_event.ble_event.conn_param.conn.min_conn_interval   = p_event->gap_event.conn_param.conn_interval_min;
        tal_event.ble_event.conn_param.conn.latency             = p_event->gap_event.conn_param.conn_latency;
    }
    break;

    case TKL_BLE_GAP_EVT_CONN_RSSI: {
        tal_event.type                                      = TAL_BLE_EVT_CONN_RSSI;
        tal_event.ble_event.link_rssi.conn_handle           = p_event->conn_handle;

        tal_event.ble_event.link_rssi.rssi                  = p_event->gap_event.link_rssi;
    }
    break;

    default: {

    } return;
    }

    if (tal_ble_event_callback) {
        tal_ble_event_callback(&tal_event);
    }
}

STATIC VOID_T tkl_ble_kernel_gatt_event_callback(TKL_BLE_GATT_PARAMS_EVT_T *p_event)
{
    TAL_BLE_EVT_PARAMS_T tal_event;

    if (p_event == NULL) {
        return;
    }

    memset(&tal_event, 0, sizeof(TAL_BLE_EVT_PARAMS_T));

    switch (p_event->type) {
    case TKL_BLE_GATT_EVT_MTU_REQUEST: {
        tal_event.type = TAL_BLE_EVT_MTU_REQUEST;
        tal_event.ble_event.exchange_mtu.conn_handle        = p_event->conn_handle;
        tal_event.ble_event.exchange_mtu.mtu                = p_event->gatt_event.exchange_mtu;
    }
    break;

    case TKL_BLE_GATT_EVT_MTU_RSP: {
        tal_event.type = TAL_BLE_EVT_MTU_RSP;
        tal_event.ble_event.exchange_mtu.conn_handle        = p_event->conn_handle;
        tal_event.ble_event.exchange_mtu.mtu                = p_event->gatt_event.exchange_mtu;
    }
    break;

    case TKL_BLE_GATT_EVT_PRIM_SEV_DISCOVERY: {
        TKL_BLE_GATT_SVC_DISC_TYPE_T *p_svc_disc = &p_event->gatt_event.svc_disc;
        USHORT_T svc_uuid16 = 0, i = 0;

        tal_event.type = TAL_BLE_EVT_CENTRAL_CONNECT_DISCOVERY;
        if (p_event->result == OPRT_OK) {
            /* 连接规则如下：
                1，优先发现涂鸦自定义服务，当存在涂鸦自定义服务时，我们将以该服务为主，且无需其它服务。
                2，当不存在涂鸦自定义服务，我们以按需发现服务为主，且由于存在多个服务和特征值，我们需独立发现，且Proxy服务最后发现（存在CCCD）。
                注：不存在涂鸦自定义服务时，我们按0x1912 >> 0x180A >> 0x1828, 任意失败将上报失败。
            */
            for (i = 0; i < p_svc_disc->svc_num; i ++) {
                svc_uuid16 = tal_ble_uuid16_convert(&p_svc_disc->services[i].uuid);
                tal_ble_svc_handle_assign(&tal_ble_peer, svc_uuid16, p_svc_disc->services[i].start_handle, p_svc_disc->services[i].end_handle);
            }

            if (tal_ble_peer.flag & TAL_BLE_SVC_UUID_V1) {
                if (tal_ble_char_handle_discovery(&tal_ble_peer, TAL_BLE_CMD_SERVICE_UUID_V1) == OPRT_OK) {
                    return;
                }
            } else if (tal_ble_peer.flag & TAL_BLE_SVC_UUID_V2) {
                if (tal_ble_char_handle_discovery(&tal_ble_peer, TAL_BLE_CMD_SERVICE_UUID_V2) == OPRT_OK) {
                    return;
                }
            } else if ((tal_ble_peer.flag & TAL_BLE_SVC_UUID_MESH_OTA) && (tal_ble_peer.flag & TAL_BLE_SVC_UUID_MESH_VER) && (tal_ble_peer.flag & TAL_BLE_SVC_UUID_MESH_PROXY)) {
                if (tal_ble_char_handle_discovery(&tal_ble_peer, TAL_MESH_DEVICE_OTA_SVC_UUID) == OPRT_OK) {
                    return;
                }
            }

            memcpy(&tal_event.ble_event.connect.peer, &tal_ble_peer, sizeof(TAL_BLE_PEER_INFO_T));
            tal_event.ble_event.connect.result = OPRT_OS_ADAPTER_BLE_SVC_DISC_FAILED;
        } else {
            /**< Return Error, Cannot Discovery Service */
            memcpy(&tal_event.ble_event.connect.peer, &tal_ble_peer, sizeof(TAL_BLE_PEER_INFO_T));
            tal_event.ble_event.connect.result = p_event->result;
        }
    }
    break;

    case TKL_BLE_GATT_EVT_CHAR_DISCOVERY: {
        TKL_BLE_GATT_CHAR_DISC_TYPE_T *p_char_disc = &p_event->gatt_event.char_disc;
        OPERATE_RET char_result = OPRT_INVALID_PARM;
        USHORT_T i = 0, char_uuid = 0, tuya_svc_flag = 0;

        tal_event.type = TAL_BLE_EVT_CENTRAL_CONNECT_DISCOVERY;
        if (p_event->result == OPRT_OK) {
            for (i = 0; i < p_char_disc->char_num; i ++) {
                char_uuid = tal_ble_uuid16_convert(&p_char_disc->characteristics[i].uuid);
                tal_ble_char_handle_assign(&tal_ble_peer, char_uuid, p_char_disc->characteristics[i].handle);
                if (char_uuid == TAL_MESH_OTA_WRITE_CHAR_UUID && (tal_ble_peer.flag & TAL_BLE_SVC_UUID_MESH_VER)) {
                    // 1, 发现了OTA服务写入特征值，填充结束后开始发现OTA Version 特征值
                    if (tal_ble_char_handle_discovery(&tal_ble_peer, TAL_MESH_DEVICE_INFO_SVC_UUID) == OPRT_OK) {
                        return;
                    }
                } else if (char_uuid == TAL_MESH_FW_VERSION_READ_CHAR_UUID && (tal_ble_peer.flag & TAL_BLE_SVC_UUID_MESH_PROXY)
                           && !(tal_ble_peer.flag & TAL_BLE_SVC_UUID_V2)) {
                    // 2, 发现了OTA版本读入特征值，填充结束后开始发现OTA Enter（Mesh Proxy）特征值
                    if (tal_ble_char_handle_discovery(&tal_ble_peer, TAL_MESH_PROXY_DEVICE_SVC_UUID) == OPRT_OK) {
                        return;
                    }
                } else if (char_uuid == TAL_BLE_CMD_WRITE_CHAR_UUID_V2 || char_uuid == TAL_BLE_CMD_NOTIFY_CHAR_UUID_V2) {
                    tuya_svc_flag = 1;
                }
            }

            // 3, 针对Mesh子设备包含FD50服务外，还需读取能力值与版本的特征值，当包含0xFD50&0x180A&0x1828时,我们将发现0x180A服务特征值
            if (tuya_svc_flag && (tal_ble_peer.flag & TAL_BLE_SVC_UUID_V2)
                && (tal_ble_peer.flag & TAL_BLE_SVC_UUID_MESH_VER)
                && (tal_ble_peer.flag & TAL_BLE_SVC_UUID_MESH_PROXY)) {
                if (tal_ble_char_handle_discovery(&tal_ble_peer, TAL_MESH_DEVICE_INFO_SVC_UUID) == OPRT_OK) {
                    return;
                }
            }

            /**< Find Characteristic Descriptor */
            if (tal_ble_peer.char_handle[TAL_COMMON_NOTIFY_CHAR_INDEX] != TKL_BLE_GATT_INVALID_HANDLE) {
                char_result = tkl_ble_gattc_char_desc_discovery(p_event->conn_handle, tal_ble_peer.char_handle[TAL_COMMON_NOTIFY_CHAR_INDEX],
                              tal_ble_peer.char_handle[TAL_COMMON_NOTIFY_CHAR_INDEX] + 4);
                if (char_result == OPRT_OK) {
                    return;
                }
            } else if (tal_ble_peer.char_handle[TAL_MESH_OTA_WRITE_CHAR_INDEX] != TKL_BLE_GATT_INVALID_HANDLE
                       && tal_ble_peer.char_handle[TAL_MESH_FW_READ_CHAR_INDEX] != TKL_BLE_GATT_INVALID_HANDLE
                       && tal_ble_peer.char_handle[TAL_MESH_PROXY_WRITE_CHAR_INDEX] != TKL_BLE_GATT_INVALID_HANDLE) {

                // 4, 当所有Mesh所需OTA服务被复现后，且其中包含GATT Proxy的接口时，我们开始发现CCCD并使能。
                char_result = tkl_ble_gattc_char_desc_discovery(p_event->conn_handle, tal_ble_peer.char_handle[TAL_MESH_PROXY_WRITE_CHAR_INDEX],
                              tal_ble_peer.char_handle[TAL_MESH_PROXY_WRITE_CHAR_INDEX] + 4);
                if (char_result == OPRT_OK) {
                    return;
                }
            }

            memcpy(&tal_event.ble_event.connect.peer, &tal_ble_peer, sizeof(TAL_BLE_PEER_INFO_T));
            tal_event.ble_event.connect.result = OPRT_OS_ADAPTER_BLE_DESC_DISC_FAILED;
        } else {
            /**< Return Error, Cannot Discovery Service */
            memcpy(&tal_event.ble_event.connect.peer, &tal_ble_peer, sizeof(TAL_BLE_PEER_INFO_T));
            tal_event.ble_event.connect.result = p_event->result;
        }
    }
    break;

    case TKL_BLE_GATT_EVT_CHAR_DESC_DISCOVERY: {
        TKL_BLE_GATT_DESC_DISC_TYPE_T *p_desc_disc = &p_event->gatt_event.desc_disc;
        UCHAR_T cccd_enable[2] = {0x01, 0x00};
        INT_T desc_result;

        tal_event.type = TAL_BLE_EVT_CENTRAL_CONNECT_DISCOVERY;
        if (p_event->result == OPRT_OK) {
            desc_result = tkl_ble_gattc_write_without_rsp(p_event->conn_handle, p_desc_disc->cccd_handle, cccd_enable, sizeof(cccd_enable));
            if (desc_result != 0) {
                tal_event.ble_event.connect.result = OPRT_OS_ADAPTER_BLE_DESC_DISC_FAILED;
            } else if (p_event->conn_handle != tal_ble_peer.conn_handle) {
                tal_event.ble_event.connect.result = OPRT_OS_ADAPTER_BLE_HANDLE_ERROR;
                memcpy(&tal_event.ble_event.connect.peer, &tal_ble_peer, sizeof(TAL_BLE_PEER_INFO_T));
            } else {
                tal_event.ble_event.connect.result = OPRT_OK;
                tal_ble_peer.char_handle[TAL_COMMON_CCCD_CHAR_INDEX] = p_desc_disc->cccd_handle;
                memcpy(&tal_event.ble_event.connect.peer, &tal_ble_peer, sizeof(TAL_BLE_PEER_INFO_T));
            }
        } else {
            /**< Return Error, Cannot Discovery characteristic descriptor */
            tal_event.ble_event.connect.result = p_event->result;
            memcpy(&tal_event.ble_event.connect.peer, &tal_ble_peer, sizeof(TAL_BLE_PEER_INFO_T));
        }
    }
    break;

    case TKL_BLE_GATT_EVT_NOTIFY_TX: {
        tal_event.type = TAL_BLE_EVT_NOTIFY_TX;
        tal_event.ble_event.notify_result.conn_handle           = p_event->conn_handle;

        tal_event.ble_event.notify_result.char_handle           = p_event->gatt_event.notify_result.char_handle;
        tal_event.ble_event.notify_result.result                = p_event->gatt_event.notify_result.result;
    }
    break;

    case TKL_BLE_GATT_EVT_WRITE_REQ: {
        tal_event.type = TAL_BLE_EVT_WRITE_REQ;
        tal_event.ble_event.write_report.peer.conn_handle       = p_event->conn_handle;
        tal_event.ble_event.write_report.peer.char_handle[0]    = p_event->gatt_event.write_report.char_handle;
        tal_event.ble_event.write_report.report.len             = p_event->gatt_event.write_report.report.length;
        tal_event.ble_event.write_report.report.p_data          = p_event->gatt_event.write_report.report.p_data;
    }
    break;

    case TKL_BLE_GATT_EVT_NOTIFY_INDICATE_RX: {
        tal_event.type = TAL_BLE_EVT_NOTIFY_RX;
        tal_event.ble_event.data_report.peer.conn_handle        = p_event->conn_handle;
        tal_event.ble_event.data_report.peer.char_handle[0]     = p_event->gatt_event.data_report.char_handle;
        tal_event.ble_event.data_report.report.len              = p_event->gatt_event.data_report.report.length;
        tal_event.ble_event.data_report.report.p_data           = p_event->gatt_event.data_report.report.p_data;
    }
    break;

    case TKL_BLE_GATT_EVT_READ_RX: {
        tal_event.type = TAL_BLE_EVT_READ_RX;
        tal_event.ble_event.data_read.peer.conn_handle          = p_event->conn_handle;
        tal_event.ble_event.data_read.peer.char_handle[0]       = p_event->gatt_event.data_read.char_handle;
        tal_event.ble_event.data_read.report.len                = p_event->gatt_event.data_read.report.length;
        tal_event.ble_event.data_read.report.p_data             = p_event->gatt_event.data_read.report.p_data;
    }
    break;

    case TKL_BLE_GATT_EVT_SUBSCRIBE: {
        if (p_event->result == OPRT_OK) {
            tal_event.type = TAL_BLE_EVT_SUBSCRIBE;
            tal_event.ble_event.subscribe.conn_handle = p_event->conn_handle;
            tal_event.ble_event.subscribe.char_handle = p_event->gatt_event.subscribe.char_handle;
            tal_event.ble_event.subscribe.reason = p_event->gatt_event.subscribe.reason;
            tal_event.ble_event.subscribe.prev_notify = p_event->gatt_event.subscribe.prev_notify;
            tal_event.ble_event.subscribe.cur_notify = p_event->gatt_event.subscribe.cur_notify;
            tal_event.ble_event.subscribe.prev_indicate = p_event->gatt_event.subscribe.prev_indicate;
            tal_event.ble_event.subscribe.cur_indicate = p_event->gatt_event.subscribe.cur_indicate;
        }
    }
    break;

    case TKL_BLE_GATT_EVT_READ_CHAR_VALUE: {
        tal_event.type = TAL_BLE_EVT_READ_CHAR;
        tal_event.ble_event.char_read.conn_handle = p_event->conn_handle;
        tal_event.ble_event.char_read.char_handle = p_event->gatt_event.char_read.char_handle;
        tal_event.ble_event.char_read.offset = p_event->gatt_event.char_read.offset;
    }
    break;

    default: {
        // Not Support
    } return;
    }

    // 【20220526】【TimCheng】：仅对Ble Central有效。
    // 新增标志说明当前是否处于服务发现状态繁忙，用于表明GATT已连接但是未正确发现服务但是设备已断开时，我们将主动上报该状态至业务端。
    if (tal_event.type == TAL_BLE_EVT_CENTRAL_CONNECT_DISCOVERY) {
        tal_ble_disc_svc_busy = 0;
    }

    if (tal_ble_event_callback) {
        tal_ble_event_callback(&tal_event);
    }
}

OPERATE_RET tal_ble_bt_init(TAL_BLE_ROLE_E role, CONST TAL_BLE_EVT_FUNC_CB ble_event)
{
    UCHAR_T ble_stack_role = 0x00; //TKL_BLE_ROLE_SERVER;    // Default Value Should be zero if using in "Single" mode

    if (ble_event != NULL) {
        /**<  Get the TAL Event Callback. */
        tal_ble_event_callback = ble_event;

        /**< Register GAP And GATT Callback */
        tkl_ble_gap_callback_register(tkl_ble_kernel_gap_event_callback);
        tkl_ble_gatt_callback_register(tkl_ble_kernel_gatt_event_callback);
    }

    /**< Init Bluetooth Stack Role For Ble. */
    if ((role & TAL_BLE_ROLE_PERIPERAL) == TAL_BLE_ROLE_PERIPERAL || (role & TAL_BLE_ROLE_BEACON) == TAL_BLE_ROLE_BEACON) {
        ble_stack_role |= TKL_BLE_ROLE_SERVER;
    }
    if ((role & TAL_BLE_ROLE_CENTRAL) == TAL_BLE_ROLE_CENTRAL) {
        ble_stack_role |= TKL_BLE_ROLE_CLIENT;
    }

    if (tkl_ble_stack_init(ble_stack_role) != OPRT_OK) {
        return OPRT_OS_ADAPTER_BLE_INIT_FAILED;
    }

    if ((role & TAL_BLE_ROLE_PERIPERAL) == TAL_BLE_ROLE_PERIPERAL) {
        TKL_BLE_GATTS_PARAMS_T *p_ble_services = &tkl_ble_gatt_service;
        *p_ble_services = (TKL_BLE_GATTS_PARAMS_T) {
            .svc_num    = TAL_COMMON_SERVICE_MAX_NUM,
             .p_service  = tkl_ble_common_service,
        };

#if (defined(TAL_BLE_SERVICE_VERSION)  && (TAL_BLE_SERVICE_VERSION == 2))
        /**< Add Tuya Common Service */
        TKL_BLE_SERVICE_PARAMS_T *p_ble_common_service = tkl_ble_common_service;
        *(p_ble_common_service + TAL_COMMON_SERVICE_INDEX) = (TKL_BLE_SERVICE_PARAMS_T) {
            .handle     = TKL_BLE_GATT_INVALID_HANDLE,
            .svc_uuid   = {
                .uuid_type   = TKL_BLE_UUID_TYPE_16,
                .uuid.uuid16 = TAL_BLE_CMD_SERVICE_UUID_V2,
            },
            .type       = TKL_BLE_UUID_SERVICE_PRIMARY,
             .char_num   = TAL_COMMON_CHAR_MAX_NUM,
              .p_char     = tkl_ble_common_char,
        };

        /**< Add Write Characteristic */
        TKL_BLE_CHAR_PARAMS_T *p_ble_common_char = tkl_ble_common_char;

        *(p_ble_common_char + TAL_COMMON_WRITE_CHAR_INDEX) = (TKL_BLE_CHAR_PARAMS_T) {
            .handle = TKL_BLE_GATT_INVALID_HANDLE,
            .char_uuid  = {
                .uuid_type   = TKL_BLE_UUID_TYPE_128,
                .uuid.uuid128 = TAL_BLE_CMD_WRITE_CHAR_UUID128_V2,
            },
            .property   = TKL_BLE_GATT_CHAR_PROP_WRITE | TKL_BLE_GATT_CHAR_PROP_WRITE_NO_RSP,
             .permission = TKL_BLE_GATT_PERM_READ | TKL_BLE_GATT_PERM_WRITE,
              .value_len  = 244,
        };

        /**< Add Notify Characteristic */
        *(p_ble_common_char + TAL_COMMON_NOTIFY_CHAR_INDEX) = (TKL_BLE_CHAR_PARAMS_T) {
            .handle = TKL_BLE_GATT_INVALID_HANDLE,
            .char_uuid  = {
                .uuid_type   = TKL_BLE_UUID_TYPE_128,
                .uuid.uuid128 = TAL_BLE_CMD_NOTIFY_CHAR_UUID128_V2,
            },
            .property   = TKL_BLE_GATT_CHAR_PROP_NOTIFY,
             .permission = TKL_BLE_GATT_PERM_READ | TKL_BLE_GATT_PERM_WRITE,
              .value_len  = 244,
        };

        /**< Add Read Characteristic */
        *(p_ble_common_char + TAL_COMMON_READ_CHAR_INDEX) = (TKL_BLE_CHAR_PARAMS_T) {
            .handle = TKL_BLE_GATT_INVALID_HANDLE,
            .char_uuid  = {
                .uuid_type   = TKL_BLE_UUID_TYPE_128,
                .uuid.uuid128 = TAL_BLE_CMD_READ_CHAR_UUID128_V2,
            },
            .property   = TKL_BLE_GATT_CHAR_PROP_READ,
             .permission = TKL_BLE_GATT_PERM_READ,
              .value_len  = 244,
        };
#else
        /**< Add Tuya Common Service */
        TKL_BLE_SERVICE_PARAMS_T *p_ble_common_service = tkl_ble_common_service;
        *(p_ble_common_service + TAL_COMMON_SERVICE_INDEX) = (TKL_BLE_SERVICE_PARAMS_T) {
            .handle     = TKL_BLE_GATT_INVALID_HANDLE,
            .svc_uuid   = {
                .uuid_type   = TKL_BLE_UUID_TYPE_16,
                .uuid.uuid16 = TAL_BLE_CMD_SERVICE_UUID_V1,
            },
            .type       = TKL_BLE_UUID_SERVICE_PRIMARY,
             .char_num   = 2,
              .p_char     = tkl_ble_common_char,
        };

        /**< Add Write Characteristic */
        TKL_BLE_CHAR_PARAMS_T *p_ble_common_char = tkl_ble_common_char;

        *(p_ble_common_char + TAL_COMMON_WRITE_CHAR_INDEX) = (TKL_BLE_CHAR_PARAMS_T) {
            .handle = TKL_BLE_GATT_INVALID_HANDLE,
            .char_uuid  = {
                .uuid_type   = TKL_BLE_UUID_TYPE_16,
                .uuid.uuid16 = TAL_BLE_CMD_WRITE_CHAR_UUID_V1,
            },
            .property   = TKL_BLE_GATT_CHAR_PROP_WRITE | TKL_BLE_GATT_CHAR_PROP_WRITE_NO_RSP,
             .permission = TKL_BLE_GATT_PERM_READ | TKL_BLE_GATT_PERM_WRITE,
              .value_len  = 244,
        };

        /**< Add Notify Characteristic */
        *(p_ble_common_char + TAL_COMMON_NOTIFY_CHAR_INDEX) = (TKL_BLE_CHAR_PARAMS_T) {
            .handle = TKL_BLE_GATT_INVALID_HANDLE,
            .char_uuid  = {
                .uuid_type   = TKL_BLE_UUID_TYPE_16,
                .uuid.uuid16 = TAL_BLE_CMD_NOTIFY_CHAR_UUID_V1,
            },
            .property   = TKL_BLE_GATT_CHAR_PROP_NOTIFY,
             .permission = TKL_BLE_GATT_PERM_READ | TKL_BLE_GATT_PERM_WRITE,
              .value_len  = 244,
        };

#endif
        if (tkl_ble_gatts_service_add(p_ble_services) != 0) {
            return OPRT_OS_ADAPTER_BLE_INIT_FAILED;
        }
    }

    return OPRT_OK;
}

OPERATE_RET tal_ble_bt_deinit(TAL_BLE_ROLE_E role)
{
    return tkl_ble_stack_deinit(role);
}

OPERATE_RET tal_ble_address_set(TAL_BLE_ADDR_T CONST *p_addr)
{
    TKL_BLE_GAP_ADDR_T ble_addr = {0};

    if (p_addr == NULL) {
        return OPRT_INVALID_PARM;
    }

    ble_addr.type = p_addr->type;
    memcpy(ble_addr.addr, p_addr->addr, 6);
    return tkl_ble_gap_addr_set(&ble_addr);
}

OPERATE_RET tal_ble_address_get(TAL_BLE_ADDR_T *p_addr)
{
    TKL_BLE_GAP_ADDR_T tal_addr = {0};

    if (p_addr == NULL) {
        return OPRT_INVALID_PARM;
    }

    tkl_ble_gap_address_get(&tal_addr);

    p_addr->type = tal_addr.type;
    memcpy(p_addr->addr, tal_addr.addr, 6);

    return OPRT_OK;
}

OPERATE_RET tal_ble_bt_link_max(USHORT_T *p_maxlink)
{
    if (p_maxlink == NULL) {
        return OPRT_INVALID_PARM;
    }

    tkl_ble_stack_gatt_link(p_maxlink);

    return OPRT_OK;
}

OPERATE_RET tal_ble_advertising_start(TAL_BLE_ADV_PARAMS_T CONST *p_adv_param)
{
    TKL_BLE_GAP_ADV_PARAMS_T tal_adv_params = {0};

    if (p_adv_param == NULL) {
        return OPRT_INVALID_PARM;
    }

    tal_adv_params.adv_type             = p_adv_param->adv_type;
    tal_adv_params.direct_addr.type     = p_adv_param->direct_addr.type;
    tal_adv_params.adv_interval_min     = p_adv_param->adv_interval_min;
    tal_adv_params.adv_interval_max     = p_adv_param->adv_interval_max;
    tal_adv_params.adv_channel_map      = 0x01 | 0x02 | 0x04;

    memcpy(tal_adv_params.direct_addr.addr, p_adv_param->direct_addr.addr, 6);

    return tkl_ble_gap_adv_start(&tal_adv_params);
}

OPERATE_RET tal_ble_advertising_data_set(TAL_BLE_DATA_T *p_adv, TAL_BLE_DATA_T *p_scan_rsp)
{
    TKL_BLE_DATA_T adv_data = {0};
    TKL_BLE_DATA_T scan_rsp_data = {0};

    if (p_adv != NULL) {
        adv_data.length = p_adv->len;
        adv_data.p_data = p_adv->p_data;
    } else {
        adv_data.length = 0;
        adv_data.p_data = NULL;
    }

    if (p_scan_rsp != NULL) {
        scan_rsp_data.length = p_scan_rsp->len;
        scan_rsp_data.p_data = p_scan_rsp->p_data;
    } else {
        scan_rsp_data.length = 0;
        scan_rsp_data.p_data = NULL;
    }

    return tkl_ble_gap_adv_rsp_data_set(&adv_data, &scan_rsp_data);
}

OPERATE_RET tal_ble_advertising_stop(VOID_T)
{
    return tkl_ble_gap_adv_stop();
}

OPERATE_RET tal_ble_advertising_data_update(TAL_BLE_DATA_T *p_adv, TAL_BLE_DATA_T *p_scan_rsp)
{
    TKL_BLE_DATA_T adv_data = {0};
    TKL_BLE_DATA_T scan_rsp_data = {0};

    if (p_adv != NULL) {
        adv_data.length = p_adv->len;
        adv_data.p_data = p_adv->p_data;
    } else {
        adv_data.length = 0;
        adv_data.p_data = NULL;
    }

    if (p_scan_rsp != NULL) {
        scan_rsp_data.length = p_scan_rsp->len;
        scan_rsp_data.p_data = p_scan_rsp->p_data;
    } else {
        scan_rsp_data.length = 0;
        scan_rsp_data.p_data = NULL;
    }

    return tkl_ble_gap_adv_rsp_data_update(&adv_data, &scan_rsp_data);
}

OPERATE_RET tal_ble_ext_advertising_create(TAL_BLE_EXT_ADV_T *p_ext_adv)
{
    if (p_ext_adv == NULL) {
        return OPRT_INVALID_PARM;
    }

    return tkl_ble_gap_ext_adv_create((TKL_BLE_GAP_EXT_ADV_T *)p_ext_adv);
}

OPERATE_RET tal_ble_ext_advertising_config(TAL_BLE_EXT_ADV_T ext_adv, TAL_BLE_EXT_ADV_PARAMS_T CONST *p_adv_params, TAL_BLE_DATA_T CONST *p_adv_data, TAL_BLE_DATA_T CONST *p_scan_rsp)
{
    TKL_BLE_GAP_EXT_ADV_T tkl_ext_adv = {0};
    TKL_BLE_GAP_EXT_ADV_PARAMS_T tkl_ext_adv_param = {0};
    TKL_BLE_DATA_T tkl_ext_adv_data = {0};
    TKL_BLE_DATA_T tkl_ext_scan_rsp = {0};

    memcpy(&tkl_ext_adv, &ext_adv, sizeof(TKL_BLE_GAP_EXT_ADV_T));
    if (p_adv_params != NULL) {
        memcpy(&tkl_ext_adv_param, p_adv_params, sizeof(TKL_BLE_GAP_EXT_ADV_PARAMS_T));
    }
    if (p_adv_data != NULL) {
        memcpy(&tkl_ext_adv_data, p_adv_data, sizeof(TKL_BLE_DATA_T));
    }
    if (p_scan_rsp != NULL) {
        memcpy(&tkl_ext_scan_rsp, p_scan_rsp, sizeof(TKL_BLE_DATA_T));
    }

    return tkl_ble_gap_ext_adv_config(tkl_ext_adv, &tkl_ext_adv_param, &tkl_ext_adv_data, &tkl_ext_scan_rsp);
}

OPERATE_RET tal_ble_ext_advertising_start(TAL_BLE_EXT_ADV_T ext_adv)
{
    TKL_BLE_GAP_EXT_ADV_T tkl_ext_adv = {0};

    tkl_ext_adv.handle = ext_adv.handle;

    return tkl_ble_gap_ext_adv_start(tkl_ext_adv);
}

OPERATE_RET tal_ble_ext_advertising_stop(TAL_BLE_EXT_ADV_T ext_adv)
{
    TKL_BLE_GAP_EXT_ADV_T tkl_ext_adv = {0};

    tkl_ext_adv.handle = ext_adv.handle;

    return tkl_ble_gap_ext_adv_stop(tkl_ext_adv);
}

OPERATE_RET tal_ble_ext_advertising_delete(TAL_BLE_EXT_ADV_T ext_adv)
{
    TKL_BLE_GAP_EXT_ADV_T tkl_ext_adv = {0};

    tkl_ext_adv.handle = ext_adv.handle;

    return tkl_ble_gap_ext_adv_delete(tkl_ext_adv);
}

OPERATE_RET tal_ble_ext_advertising_clear(void)
{
    return tkl_ble_gap_ext_adv_clear();
}

uint16_t tal_ble_ext_advertising_get_max_data_length(void)
{
    return tkl_ble_gap_ext_adv_get_max_data_length();
}

uint8_t tal_ble_ext_advertising_get_support_number(void)
{
    return tkl_ble_gap_ext_adv_get_support_number();
}

OPERATE_RET tal_ble_scan_start(TAL_BLE_SCAN_PARAMS_T CONST *p_scan_param)
{
    TKL_BLE_GAP_SCAN_PARAMS_T tal_scan_params = {0};

    tal_scan_params.extended            = 0;
    tal_scan_params.active              = ((p_scan_param->type == TAL_BLE_SCAN_TYPE_ACTIVE) ? 1 : 0);
    tal_scan_params.scan_phys           = TKL_BLE_GAP_PHY_1MBPS;
    tal_scan_params.interval            = p_scan_param->scan_interval;
    tal_scan_params.window              = p_scan_param->scan_window;
    tal_scan_params.timeout             = p_scan_param->timeout;
    tal_scan_params.scan_channel_map    = 0x01 | 0x02 | 0x04;

    return tkl_ble_gap_scan_start(&tal_scan_params);
}

OPERATE_RET tal_ble_scan_stop(VOID_T)
{
    return tkl_ble_gap_scan_stop();
}

OPERATE_RET tal_ble_ext_scan_start(TAL_BLE_EXT_SCAN_PARAMS_T CONST *p_scan_param)
{
    TKL_BLE_GAP_SCAN_PARAMS_T tal_scan_params = {0};

    tal_scan_params.extended            = p_scan_param->extended;
    tal_scan_params.active              = ((p_scan_param->type == TAL_BLE_SCAN_TYPE_ACTIVE) ? 1 : 0);
    tal_scan_params.scan_phys           = p_scan_param->scan_phys;
    tal_scan_params.interval            = p_scan_param->scan_interval;
    tal_scan_params.window              = p_scan_param->scan_window;
    tal_scan_params.timeout             = p_scan_param->timeout;
    tal_scan_params.scan_channel_map    = p_scan_param->scan_channel_map;

    return tkl_ble_gap_scan_start(&tal_scan_params);
}

OPERATE_RET tal_ble_ext_scan_stop(VOID_T)
{
    return tkl_ble_gap_scan_stop();
}

OPERATE_RET tal_ble_rssi_get(CONST TAL_BLE_PEER_INFO_T peer)
{
    USHORT_T conn_handle = peer.conn_handle;

    return tkl_ble_gap_rssi_get(conn_handle);
}

OPERATE_RET tal_ble_tx_power_set(UCHAR_T role, INT_T tx_power)
{
    return tkl_ble_gap_tx_power_set(role, tx_power);
}

OPERATE_RET tal_ble_conn_param_update(CONST TAL_BLE_PEER_INFO_T peer, TAL_BLE_CONN_PARAMS_T CONST *p_conn_params)
{
    USHORT_T conn_handle = peer.conn_handle;
    TKL_BLE_GAP_CONN_PARAMS_T param = {0};

    if (p_conn_params == NULL) {
        return OPRT_INVALID_PARM;
    }

    param.conn_interval_min     = p_conn_params->min_conn_interval;
    param.conn_interval_max     = p_conn_params->max_conn_interval;
    param.conn_latency          = p_conn_params->latency;
    param.conn_sup_timeout      = p_conn_params->conn_sup_timeout;
    param.connection_timeout    = p_conn_params->connection_timeout;

    return tkl_ble_gap_conn_param_update(conn_handle, &param);
}

OPERATE_RET tal_ble_connect_and_discovery(CONST TAL_BLE_PEER_INFO_T peer, TAL_BLE_CONN_PARAMS_T CONST *p_conn_params)
{
    TKL_BLE_GAP_ADDR_T tal_conn_addr = {0};
    TKL_BLE_GAP_SCAN_PARAMS_T tal_scan_params = {0};
    TKL_BLE_GAP_CONN_PARAMS_T tal_conn_params = {0};

    if (p_conn_params == NULL) {
        tal_conn_params.conn_interval_max   = 0x12;
        tal_conn_params.conn_interval_min   = 0x12;
        tal_conn_params.conn_latency        = 0;
        tal_conn_params.conn_sup_timeout    = 0x100;
        tal_conn_params.connection_timeout  = 500;
    } else {
        tal_conn_params.conn_interval_max   = p_conn_params->max_conn_interval;
        tal_conn_params.conn_interval_min   = p_conn_params->min_conn_interval;
        tal_conn_params.conn_latency        = p_conn_params->latency;
        tal_conn_params.conn_sup_timeout    = p_conn_params->conn_sup_timeout;
        tal_conn_params.connection_timeout  = p_conn_params->connection_timeout;
    }

    tal_scan_params.interval            = 0x0010;
    tal_scan_params.window              = 0x0010;

    if (peer.peer_addr.type == TAL_BLE_ADDR_TYPE_RANDOM) {
        tal_conn_addr.type = TKL_BLE_GAP_ADDR_TYPE_RANDOM;
    } else {
        tal_conn_addr.type = TKL_BLE_GAP_ADDR_TYPE_PUBLIC;
    }
    memcpy(tal_conn_addr.addr, peer.peer_addr.addr, 6);

    memset(&tal_ble_peer, 0, sizeof(TAL_BLE_PEER_INFO_T));

    /**Assign Peer Info Into TAL Peer Info*/
    memcpy(&tal_ble_peer, &peer, sizeof(TAL_BLE_PEER_INFO_T));

    tal_ble_peer.conn_handle                                = TKL_BLE_GATT_INVALID_HANDLE;
    tal_ble_peer.char_handle[TAL_COMMON_WRITE_CHAR_INDEX]   = TKL_BLE_GATT_INVALID_HANDLE;
    tal_ble_peer.char_handle[TAL_COMMON_NOTIFY_CHAR_INDEX]  = TKL_BLE_GATT_INVALID_HANDLE;
    tal_ble_peer.char_handle[TAL_COMMON_READ_CHAR_INDEX]    = TKL_BLE_GATT_INVALID_HANDLE;
    tal_ble_peer.char_handle[TAL_COMMON_CCCD_CHAR_INDEX]     = TKL_BLE_GATT_INVALID_HANDLE;
    tal_ble_peer.char_handle[TAL_MESH_OTA_WRITE_CHAR_INDEX] = TKL_BLE_GATT_INVALID_HANDLE;
    tal_ble_peer.char_handle[TAL_MESH_PROXY_WRITE_CHAR_INDEX]   = TKL_BLE_GATT_INVALID_HANDLE;
    tal_ble_peer.char_handle[TAL_MESH_FW_READ_CHAR_INDEX]   = TKL_BLE_GATT_INVALID_HANDLE;

    tal_ble_peer.service_filter = peer.service_filter;
    tal_ble_peer.service_num = 0;
    tal_ble_peer.flag = 0;

    /**< After do connecting one device, we will discovery all service and char. */
    return tkl_ble_gap_connect(&tal_conn_addr, &tal_scan_params, &tal_conn_params);
}

OPERATE_RET tal_ble_disconnect(CONST TAL_BLE_PEER_INFO_T peer)
{
    USHORT_T conn_handle = peer.conn_handle;

    return tkl_ble_gap_disconnect(conn_handle, TKL_BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
}

OPERATE_RET tal_ble_server_common_send(TAL_BLE_DATA_T *p_data)
{
    if (p_data == NULL) {
        return OPRT_INVALID_PARM;
    }

    /* if (tkl_ble_common_connect_handle != TKL_BLE_GATT_INVALID_HANDLE */
    /* && tkl_ble_common_char[TAL_COMMON_NOTIFY_CHAR_INDEX].handle != TKL_BLE_GATT_INVALID_HANDLE) { */

    return tkl_ble_gatts_value_notify(tkl_ble_common_connect_handle, tkl_ble_common_char[TAL_COMMON_NOTIFY_CHAR_INDEX].handle,
                                      p_data->p_data, p_data->len);
    /* } */

    return OPRT_OS_ADAPTER_BLE_NOTIFY_FAILED;
}

OPERATE_RET tal_ble_server_common_read_update(TAL_BLE_DATA_T *p_data)
{
    if (p_data == NULL) {
        return OPRT_INVALID_PARM;
    }

    /* if (tkl_ble_common_connect_handle != TKL_BLE_GATT_INVALID_HANDLE */
    /* && tkl_ble_common_char[TAL_COMMON_READ_CHAR_INDEX].handle != TKL_BLE_GATT_INVALID_HANDLE) { */

    return tkl_ble_gatts_value_set(tkl_ble_common_connect_handle, tkl_ble_common_char[TAL_COMMON_READ_CHAR_INDEX].handle,
                                   p_data->p_data, p_data->len);
    /* } */

    return OPRT_INVALID_PARM;
}

OPERATE_RET tal_ble_client_common_send(CONST TAL_BLE_PEER_INFO_T peer, TAL_BLE_DATA_T *p_data)
{
    USHORT_T char_handle = 0;

    if (p_data == NULL || peer.conn_handle == TKL_BLE_GATT_INVALID_HANDLE) {
        return OPRT_INVALID_PARM;
    }

    if (peer.char_handle[TAL_COMMON_WRITE_CHAR_INDEX] != 0 && peer.char_handle[TAL_COMMON_WRITE_CHAR_INDEX] != TKL_BLE_GATT_INVALID_HANDLE) {
        char_handle = peer.char_handle[TAL_COMMON_WRITE_CHAR_INDEX];
    } else if (peer.char_handle[TAL_MESH_OTA_WRITE_CHAR_INDEX] != 0 && peer.char_handle[TAL_MESH_OTA_WRITE_CHAR_INDEX] != TKL_BLE_GATT_INVALID_HANDLE) {
        char_handle = peer.char_handle[TAL_MESH_OTA_WRITE_CHAR_INDEX];
    } else if (peer.char_handle[TAL_MESH_PROXY_WRITE_CHAR_INDEX] != 0 && peer.char_handle[TAL_MESH_PROXY_WRITE_CHAR_INDEX] != TKL_BLE_GATT_INVALID_HANDLE) {
        char_handle = peer.char_handle[TAL_MESH_PROXY_WRITE_CHAR_INDEX];
    } else if (peer.char_handle[TAL_COMMON_CCCD_CHAR_INDEX] != 0 && peer.char_handle[TAL_COMMON_CCCD_CHAR_INDEX] != TKL_BLE_GATT_INVALID_HANDLE) {
        char_handle = peer.char_handle[TAL_COMMON_CCCD_CHAR_INDEX];
    } else {
        return OPRT_OS_ADAPTER_BLE_WRITE_FAILED;
    }

    return tkl_ble_gattc_write_without_rsp(peer.conn_handle, char_handle, p_data->p_data, p_data->len);
}

OPERATE_RET tal_ble_client_common_read(CONST TAL_BLE_PEER_INFO_T peer)
{
    USHORT_T char_handle = 0;
    if (peer.conn_handle == TKL_BLE_GATT_INVALID_HANDLE) {
        return OPRT_INVALID_PARM;
    }

    if (peer.char_handle[TAL_COMMON_READ_CHAR_INDEX] != 0 && peer.char_handle[TAL_COMMON_READ_CHAR_INDEX] != TKL_BLE_GATT_INVALID_HANDLE) {
        char_handle = peer.char_handle[TAL_COMMON_READ_CHAR_INDEX];
    } else if (peer.char_handle[TAL_MESH_FW_READ_CHAR_INDEX] != 0 && peer.char_handle[TAL_MESH_FW_READ_CHAR_INDEX] != TKL_BLE_GATT_INVALID_HANDLE) {
        char_handle = peer.char_handle[TAL_MESH_FW_READ_CHAR_INDEX];
    } else {
        return OPRT_OS_ADAPTER_BLE_READ_FAILED;
    }

    return tkl_ble_gattc_read(peer.conn_handle, char_handle);
}

OPERATE_RET tal_ble_server_exchange_mtu_reply(CONST TAL_BLE_PEER_INFO_T peer, USHORT_T server_mtu)
{
    USHORT_T conn_handle = peer.conn_handle;

    if (server_mtu < 23 || server_mtu > 247) {
        return OPRT_INVALID_PARM;
    }

    return tkl_ble_gatts_exchange_mtu_reply(conn_handle, server_mtu);
}

OPERATE_RET tal_ble_client_exchange_mtu_request(CONST TAL_BLE_PEER_INFO_T peer, USHORT_T client_mtu)
{
    USHORT_T conn_handle = peer.conn_handle;

    if (client_mtu < 23 || client_mtu > 247) {
        return OPRT_INVALID_PARM;
    }

    return tkl_ble_gattc_exchange_mtu_request(conn_handle, client_mtu);
}

OPERATE_RET tal_ble_vendor_command_control(USHORT_T opcode, VOID_T *user_data, USHORT_T data_len)
{
    return tkl_ble_vendor_command_control(opcode, user_data, data_len);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_ble_cloud_func_register(TAL_BLE_LOG_SEQ_FUNC log_seq_func, TAL_BLE_PUBLISH_CB *publish_func)
{
    return OPRT_NOT_SUPPORTED;
}

