/**
 * @file tkl_bluetooth.c
 * @brief This is tkl_bluetooth file
 * @version 1.0
 * @date 2020-08-15
 *
 * @copyright Copyright 2020-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "vendor/jl_ble/tuyaos/include/board.h"
#include "tkl_memory.h"
#include "tkl_bluetooth.h"
#include "app_ble_spp_api.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
static const uint8_t tuyaos_profile_data[];

#define ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE 0x0003
#define ATT_CHARACTERISTIC_00000001_0000_1001_8001_00805F9B07D0_01_VALUE_HANDLE 0x0006
#define ATT_CHARACTERISTIC_00000002_0000_1001_8001_00805F9B07D0_01_VALUE_HANDLE 0x0008
#define ATT_CHARACTERISTIC_00000002_0000_1001_8001_00805F9B07D0_01_CLIENT_CONFIGURATION_HANDLE 0x0009
#define ATT_CHARACTERISTIC_00000003_0000_1001_8001_00805F9B07D0_01_VALUE_HANDLE 0x000b


/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/

void *tuyaos_ble_hdl = NULL;
TKL_BLE_GAP_EVT_FUNC_CB tuyaos_gap_evt = NULL;
TKL_BLE_GATT_EVT_FUNC_CB tuyaos_gatt_evt = NULL;

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/
extern const u8 *bt_get_mac_addr();
extern const char *bt_get_local_name();
static void tuyaos_cbk_packet_handler(void *hdl, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static uint16_t tuyaos_att_read_callback(void *hdl, hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
static int tuyaos_att_write_callback(void *hdl, hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);


#define TUYAOS_BLE_HDL_UUID \
    (((u8)('T' + 'U' + 'Y') << (3 * 8)) | \
     ((u8)('A' + 'O' + 'S') << (2 * 8)) | \
     ((u8)('B' + 'L' + 'E') << (1 * 8)) | \
     ((u8)('H' + 'D' + 'L') << (0 * 8)))

OPERATE_RET tkl_ble_stack_init(UCHAR_T role)
{
    TY_PRINTF("%s", __FUNCTION__);
    u8 *ble_addr = (u8 *)bt_get_mac_addr();
    tuyaos_ble_hdl = app_ble_hdl_alloc();
    if (tuyaos_ble_hdl == NULL) {
        printf("tuyaos_ble_hdl alloc err !\n");
        return OPRT_COM_ERROR;
    }
    app_ble_hdl_uuid_set(tuyaos_ble_hdl, TUYAOS_BLE_HDL_UUID);
    app_ble_set_mac_addr(tuyaos_ble_hdl, (void *)ble_addr);
    app_ble_profile_set(tuyaos_ble_hdl, tuyaos_profile_data);
    app_ble_att_read_callback_register(tuyaos_ble_hdl, tuyaos_att_read_callback);
    app_ble_att_write_callback_register(tuyaos_ble_hdl, tuyaos_att_write_callback);
    app_ble_att_server_packet_handler_register(tuyaos_ble_hdl, tuyaos_cbk_packet_handler);
    app_ble_hci_event_callback_register(tuyaos_ble_hdl, tuyaos_cbk_packet_handler);
    app_ble_l2cap_packet_handler_register(tuyaos_ble_hdl, tuyaos_cbk_packet_handler);

    return OPRT_OK;
    /* return OPRT_NOT_SUPPORTED; */
}

OPERATE_RET tkl_ble_stack_deinit(UCHAR_T role)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_OK;
    /* return OPRT_NOT_SUPPORTED; */
}

OPERATE_RET tkl_ble_stack_gatt_link(USHORT_T *p_link)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gap_callback_register(TKL_BLE_GAP_EVT_FUNC_CB p_gap_evt)
{
    tuyaos_gap_evt = p_gap_evt;
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_OK;
    /* return OPRT_NOT_SUPPORTED; */
}

OPERATE_RET tkl_ble_gatt_callback_register(TKL_BLE_GATT_EVT_FUNC_CB p_gatt_evt)
{
    tuyaos_gatt_evt = p_gatt_evt;
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_OK;
    /* return OPRT_NOT_SUPPORTED; */
}

OPERATE_RET tkl_ble_gap_addr_set(TKL_BLE_GAP_ADDR_T CONST *p_addr)
{
    TY_PRINTF("%s", __FUNCTION__);
    if (p_addr->type == TKL_BLE_GAP_ADDR_TYPE_PUBLIC) {
        app_ble_adv_address_type_set(tuyaos_ble_hdl, 0);
    } else if (p_addr->type == TKL_BLE_GAP_ADDR_TYPE_RANDOM) {
        app_ble_adv_address_type_set(tuyaos_ble_hdl, 1);
    }
    app_ble_set_mac_addr(tuyaos_ble_hdl, (void *)p_addr->addr);
    return OPRT_OK;
    /* return OPRT_NOT_SUPPORTED; */
}

OPERATE_RET tkl_ble_gap_address_get(TKL_BLE_GAP_ADDR_T *p_addr)
{
    TY_PRINTF("%s", __FUNCTION__);
    u8 *ble_addr = app_ble_adv_addr_get(tuyaos_ble_hdl);
    if (ble_addr) {
        memcpy(p_addr->addr, ble_addr, 6);
    } else {
        memset(p_addr->addr, 0x00, 6);
    }
    return OPRT_OK;
    //return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gap_adv_start(TKL_BLE_GAP_ADV_PARAMS_T CONST *p_adv_params)
{
    TY_PRINTF("%s", __FUNCTION__);
    app_ble_set_adv_param(tuyaos_ble_hdl, p_adv_params->adv_interval_min, APP_ADV_IND, APP_ADV_CHANNEL_ALL);
    app_ble_adv_enable(tuyaos_ble_hdl, 1);
    return OPRT_OK;
    //return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gap_adv_stop(VOID)
{
    TY_PRINTF("%s", __FUNCTION__);
    app_ble_adv_enable(tuyaos_ble_hdl, 0);
    return OPRT_OK;
    //return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gap_adv_rsp_data_set(TKL_BLE_DATA_T CONST *p_adv, TKL_BLE_DATA_T CONST *p_scan_rsp)
{
    TY_PRINTF("%s", __FUNCTION__);
    if (p_adv->p_data && p_adv->length) {
        app_ble_adv_data_set(tuyaos_ble_hdl, p_adv->p_data, p_adv->length);
    }
    if (p_scan_rsp->p_data && p_scan_rsp->length) {
        app_ble_rsp_data_set(tuyaos_ble_hdl, p_scan_rsp->p_data, p_scan_rsp->length);
    }
    return OPRT_OK;
}

OPERATE_RET tkl_ble_gap_adv_rsp_data_update(TKL_BLE_DATA_T CONST *p_adv, TKL_BLE_DATA_T CONST *p_scan_rsp)
{
    TY_PRINTF("%s", __FUNCTION__);
    int adv_state = app_ble_adv_state_get(tuyaos_ble_hdl);
    if (adv_state) {
        app_ble_adv_enable(tuyaos_ble_hdl, 0);
    }
    tkl_ble_gap_adv_rsp_data_set(p_adv, p_scan_rsp);
    if (adv_state) {
        app_ble_adv_enable(tuyaos_ble_hdl, 1);
    }
    return OPRT_OK;
    //return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gap_ext_adv_create(TKL_BLE_GAP_EXT_ADV_T *p_ext_adv)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gap_ext_adv_config(TKL_BLE_GAP_EXT_ADV_T ext_adv, TKL_BLE_GAP_EXT_ADV_PARAMS_T CONST *p_adv_params, TKL_BLE_DATA_T CONST *p_adv_data, TKL_BLE_DATA_T CONST *p_scan_rsp)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gap_ext_adv_start(TKL_BLE_GAP_EXT_ADV_T ext_adv)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gap_ext_adv_stop(TKL_BLE_GAP_EXT_ADV_T ext_adv)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gap_ext_adv_delete(TKL_BLE_GAP_EXT_ADV_T ext_adv)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gap_ext_adv_clear(void)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

uint16_t tkl_ble_gap_ext_adv_get_max_data_length(void)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

uint8_t tkl_ble_gap_ext_adv_get_support_number(void)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gap_scan_start(TKL_BLE_GAP_SCAN_PARAMS_T CONST *p_scan_params)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gap_scan_stop(VOID)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gap_connect(TKL_BLE_GAP_ADDR_T CONST *p_peer_addr, TKL_BLE_GAP_SCAN_PARAMS_T CONST *p_scan_params, TKL_BLE_GAP_CONN_PARAMS_T CONST *p_conn_params)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gap_disconnect(USHORT_T conn_handle, UCHAR_T hci_reason)
{
    TY_PRINTF("%s", __FUNCTION__);
    app_ble_disconnect(tuyaos_ble_hdl);
    return OPRT_OK;
    //return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gap_conn_param_update(USHORT_T conn_handle, TKL_BLE_GAP_CONN_PARAMS_T CONST *p_conn_params)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gap_tx_power_set(UCHAR_T role, INT32_T tx_power)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gap_rssi_get(USHORT_T conn_handle)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gatts_service_add(TKL_BLE_GATTS_PARAMS_T *p_service)
{
    TY_PRINTF("%s", __FUNCTION__);

    int i = 0;
    printf("svc_num: %d\n", p_service->svc_num);
    for (i = 0; i < p_service->svc_num; i++) {
        printf("handle: %d\n", p_service->p_service[i].handle);
        printf("svc_type: %d\n", p_service->p_service[i].svc_uuid.uuid_type);
        printf("svc_uuid:%x\n", p_service->p_service[i].svc_uuid.uuid.uuid16);
        printf("char_num:%x\n", p_service->p_service[i].char_num);
    }
    return OPRT_OK;
    //return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gatts_value_set(USHORT_T conn_handle, USHORT_T char_handle, UCHAR_T *p_data, USHORT_T length)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gatts_value_get(USHORT_T conn_handle, USHORT_T char_handle, UCHAR_T *p_data, USHORT_T length)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gatts_value_notify(USHORT_T conn_handle, USHORT_T char_handle, UCHAR_T *p_data, USHORT_T length)
{
    TY_PRINTF("%s", __FUNCTION__);
    int ret = 0;
    ret = app_ble_att_send_data(tuyaos_ble_hdl, ATT_CHARACTERISTIC_00000002_0000_1001_8001_00805F9B07D0_01_VALUE_HANDLE, p_data, length, ATT_OP_AUTO_READ_CCC);
    if (ret) {
        printf("tuyaos ble send err:%d\n", ret);
    }
    return ret ? OPRT_INVALID_PARM : OPRT_OK;
    //return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gatts_value_indicate(USHORT_T conn_handle, USHORT_T char_handle, UCHAR_T *p_data, USHORT_T length)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gatts_exchange_mtu_reply(USHORT_T conn_handle, USHORT_T server_rx_mtu)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gattc_all_service_discovery(USHORT_T conn_handle)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gattc_all_char_discovery(USHORT_T conn_handle, USHORT_T start_handle, USHORT_T end_handle)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gattc_char_desc_discovery(USHORT_T conn_handle, USHORT_T start_handle, USHORT_T end_handle)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gattc_write_without_rsp(USHORT_T conn_handle, USHORT_T char_handle, UCHAR_T *p_data, USHORT_T length)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gattc_write(USHORT_T conn_handle, USHORT_T char_handle, UCHAR_T *p_data, USHORT_T length)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gattc_read(USHORT_T conn_handle, USHORT_T char_handle)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_gattc_exchange_mtu_request(USHORT_T conn_handle, USHORT_T client_rx_mtu)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_ble_vendor_command_control(USHORT_T opcode, VOID_T *user_data, USHORT_T data_len)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}


///////////////////// ble api /////////////////////////

static const uint8_t tuyaos_profile_data[] = {
    //////////////////////////////////////////////////////
    //
    // 0x0001 PRIMARY_SERVICE  1800
    //
    //////////////////////////////////////////////////////
    0x0a, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x28, 0x00, 0x18,

    /* CHARACTERISTIC,  2a00, READ | DYNAMIC, */
    // 0x0002 CHARACTERISTIC 2a00 READ | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x02, 0x00, 0x03, 0x28, 0x02, 0x03, 0x00, 0x00, 0x2a,
    // 0x0003 VALUE 2a00 READ | DYNAMIC
    0x08, 0x00, 0x02, 0x01, 0x03, 0x00, 0x00, 0x2a,

    //////////////////////////////////////////////////////
    //
    // 0x0004 PRIMARY_SERVICE  FD50
    //
    //////////////////////////////////////////////////////
    0x0a, 0x00, 0x02, 0x00, 0x04, 0x00, 0x00, 0x28, 0x50, 0xfd,

    /* CHARACTERISTIC,  00000001-0000-1001-8001-00805F9B07D0, WRITE_WITHOUT_RESPONSE | DYNAMIC, */
    // 0x0005 CHARACTERISTIC 00000001-0000-1001-8001-00805F9B07D0 WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x1b, 0x00, 0x02, 0x00, 0x05, 0x00, 0x03, 0x28, 0x04, 0x06, 0x00, 0xd0, 0x07, 0x9b, 0x5f, 0x80, 0x00, 0x01, 0x80, 0x01, 0x10, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    // 0x0006 VALUE 00000001-0000-1001-8001-00805F9B07D0 WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x16, 0x00, 0x04, 0x03, 0x06, 0x00, 0xd0, 0x07, 0x9b, 0x5f, 0x80, 0x00, 0x01, 0x80, 0x01, 0x10, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,

    /* CHARACTERISTIC,  00000002-0000-1001-8001-00805F9B07D0, NOTIFY | DYNAMIC, */
    // 0x0007 CHARACTERISTIC 00000002-0000-1001-8001-00805F9B07D0 NOTIFY | DYNAMIC
    0x1b, 0x00, 0x02, 0x00, 0x07, 0x00, 0x03, 0x28, 0x10, 0x08, 0x00, 0xd0, 0x07, 0x9b, 0x5f, 0x80, 0x00, 0x01, 0x80, 0x01, 0x10, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
    // 0x0008 VALUE 00000002-0000-1001-8001-00805F9B07D0 NOTIFY | DYNAMIC
    0x16, 0x00, 0x10, 0x03, 0x08, 0x00, 0xd0, 0x07, 0x9b, 0x5f, 0x80, 0x00, 0x01, 0x80, 0x01, 0x10, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
    // 0x0009 CLIENT_CHARACTERISTIC_CONFIGURATION
    0x0a, 0x00, 0x0a, 0x01, 0x09, 0x00, 0x02, 0x29, 0x00, 0x00,

    /* CHARACTERISTIC,  00000003-0000-1001-8001-00805F9B07D0, READ | DYNAMIC, */
    // 0x000a CHARACTERISTIC 00000003-0000-1001-8001-00805F9B07D0 READ | DYNAMIC
    0x1b, 0x00, 0x02, 0x00, 0x0a, 0x00, 0x03, 0x28, 0x02, 0x0b, 0x00, 0xd0, 0x07, 0x9b, 0x5f, 0x80, 0x00, 0x01, 0x80, 0x01, 0x10, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    // 0x000b VALUE 00000003-0000-1001-8001-00805F9B07D0 READ | DYNAMIC
    0x16, 0x00, 0x02, 0x03, 0x0b, 0x00, 0xd0, 0x07, 0x9b, 0x5f, 0x80, 0x00, 0x01, 0x80, 0x01, 0x10, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,

    // END
    0x00, 0x00,
};

static void tuyaos_cbk_packet_handler(void *hdl, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    TKL_BLE_GAP_PARAMS_EVT_T tkl_gap_event = {0};
    u16 con_handle;
    // printf("cbk packet_type:0x%x, packet[0]:0x%x, packet[2]:0x%x", packet_type, packet[0], packet[2]);
    switch (packet_type) {
    case HCI_EVENT_PACKET:
        switch (hci_event_packet_get_type(packet)) {
        case ATT_EVENT_CAN_SEND_NOW:
            printf("ATT_EVENT_CAN_SEND_NOW");
            break;
        case HCI_EVENT_LE_META:
            switch (hci_event_le_meta_get_subevent_code(packet)) {
            case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
                con_handle = little_endian_read_16(packet, 4);
                printf("HCI_SUBEVENT_LE_CONNECTION_COMPLETE: %0x", con_handle);
                put_buf(&packet[8], 6);
                if (tuyaos_gap_evt) {
                    tkl_gap_event.type = TKL_BLE_GAP_EVT_CONNECT;
                    tkl_gap_event.result = OPRT_OK;
                    tkl_gap_event.conn_handle = con_handle;
                    tkl_gap_event.gap_event.connect.role = TKL_BLE_ROLE_SERVER;
                    tuyaos_gap_evt(&tkl_gap_event);
                }
                break;
            default:
                break;
            }
            break;
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            printf("HCI_EVENT_DISCONNECTION_COMPLETE: %0x", packet[5]);
            con_handle = little_endian_read_16(packet, 3);
            if (tuyaos_gap_evt) {
                tkl_gap_event.type = TKL_BLE_GAP_EVT_DISCONNECT;
                tkl_gap_event.result = OPRT_OK;
                tkl_gap_event.conn_handle = con_handle;
                tkl_gap_event.gap_event.disconnect.reason = packet[5];
                tuyaos_gap_evt(&tkl_gap_event);
            }
            break;
        default:
            break;
        }
        break;
    }
    return;
}


static uint16_t tuyaos_att_read_callback(void *hdl, hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    uint16_t  att_value_len = 0;
    uint16_t handle = att_handle;
    printf("<-------------read_callback, handle= 0x%04x,buffer= %08x", handle, (u32)buffer);
    switch (handle) {
    case ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE:
        const char *gap_name = bt_get_local_name();
        att_value_len = strlen(gap_name);
        if ((offset >= att_value_len) || (offset + buffer_size) > att_value_len) {
            break;
        }
        if (buffer) {
            memcpy(buffer, &gap_name[offset], buffer_size);
            att_value_len = buffer_size;
            printf("\n------read gap_name: %s", gap_name);
        }
        break;
    case ATT_CHARACTERISTIC_00000002_0000_1001_8001_00805F9B07D0_01_CLIENT_CONFIGURATION_HANDLE:
        if (buffer) {
            buffer[0] = multi_att_get_ccc_config(connection_handle, handle);
            buffer[1] = 0;
        }
        att_value_len = 2;
        break;
    default:
        break;
    }
    printf("att_value_len= %d", att_value_len);
    return att_value_len;
    return 0;
}

static int tuyaos_att_write_callback(void *hdl, hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    TKL_BLE_GATT_PARAMS_EVT_T tlk_gatt_event = {0};
    int result = 0;
    u16 tmp16;
    u16 handle = att_handle;
    printf("<-------------write_callback, handle= 0x%04x,size = %d", handle, buffer_size);
    switch (handle) {
    case ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE:
        break;
    case ATT_CHARACTERISTIC_00000001_0000_1001_8001_00805F9B07D0_01_VALUE_HANDLE:
        printf("rx(%d):\n", buffer_size);
        put_buf(buffer, buffer_size);
        if (tuyaos_gatt_evt) {
            tlk_gatt_event.type = TKL_BLE_GATT_EVT_WRITE_REQ;
            tlk_gatt_event.conn_handle = connection_handle;
            tlk_gatt_event.result = OPRT_OK;
            tlk_gatt_event.gatt_event.write_report.char_handle = att_handle;
            tlk_gatt_event.gatt_event.write_report.report.length = buffer_size;
            tlk_gatt_event.gatt_event.write_report.report.p_data = buffer;
            tuyaos_gatt_evt(&tlk_gatt_event);
        }
        break;
    case ATT_CHARACTERISTIC_00000002_0000_1001_8001_00805F9B07D0_01_CLIENT_CONFIGURATION_HANDLE:
        printf("\nwrite ccc:%04x, %02x\n", handle, buffer[0]);
        multi_att_set_ccc_config(connection_handle, handle, buffer[0]);
        break;
    default:
        break;
    }
    return 0;
}

