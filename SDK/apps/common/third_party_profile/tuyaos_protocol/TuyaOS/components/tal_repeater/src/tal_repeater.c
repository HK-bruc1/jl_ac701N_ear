/**
 * @file tal_repeater.c
 * @brief This is tal_repeater file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "vendor/jl_ble/tuyaos/include/board.h"

#include "tal_repeater.h"
#include "tal_bluetooth.h"
#include "tal_memory.h"
#include "tal_util.h"
#include "tuya_sdk_callback.h"
#include "tuya_ble_protocol_callback.h"
#include "tal_ble_app_passthrough.h"
#include "tal_sdk_test.h"
#include "tal_log.h"
#include "tal_memory.h"
#include "tuya_ble_api.h"
#include "tuya_ble_mutli_tsf_protocol.h"
#include "tuya_ble_mem.h"
#include "tal_system.h"
#include "tal_sw_timer.h"

#if (TUYA_BLE_FEATURE_REPEATER_ENABLE != 0)

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define  TAL_REPEATER_ADV_REPORT_TIME      2000
#define  TAL_REPEATER_CONN_TIMEOUT_TIME    10000
#define  TAL_REPEATER_DEFAULT_MTU          23

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
#pragma pack(1)
typedef struct {
    UINT8_T  len;
    UINT8_T  value[31];
} tal_repeater_ble_data_t;

typedef struct {
    TAL_BLE_PEER_INFO_T peer;
    uint16_t mtu;
} tal_repeater_slave_info_t;

typedef struct {
    uint8_t  cmd;
    uint8_t  state;
} tal_repeater_set_adv_t;

typedef struct {
    uint8_t  cmd;
    uint8_t  status;
} tal_repeater_set_adv_rsp_t;

typedef struct {
    uint8_t  cmd;
    TAL_BLE_ADDR_T peer_addr;
    uint16_t conn_interval_min;
    uint16_t conn_interval_max;
    uint16_t slave_latency;
    uint16_t conn_sup_timeout;
    uint32_t connection_timeout;
} tal_repeater_conn_t;

typedef struct {
    uint8_t  cmd;
    TAL_BLE_ADDR_T peer_addr;
    uint8_t  status;
} tal_repeater_conn_rsp_t;

typedef struct {
    uint8_t  cmd;
    TAL_BLE_ADDR_T peer_addr;
} tal_repeater_disconn_t;

typedef struct {
    uint8_t  cmd;
    TAL_BLE_ADDR_T peer_addr;
    uint8_t  status;
} tal_repeater_disconn_rsp_t;

typedef struct {
    uint8_t  cmd;
    TAL_BLE_ADDR_T peer_addr;
    uint16_t len;
    uint8_t  buf[];
} tal_repeater_rx_data_t;

typedef struct {
    uint8_t  cmd;
    TAL_BLE_ADDR_T peer_addr;
    uint8_t  status;
} tal_repeater_rx_data_rsp_t;

typedef struct {
    uint8_t  cmd;
    TAL_BLE_ADDR_T peer_addr;
} tal_repeater_get_rssi_t;

typedef struct {
    uint8_t  cmd;
    TAL_BLE_ADDR_T peer_addr;
    uint8_t  rssi;
} tal_repeater_get_rssi_rsp_t;

typedef struct {
    uint8_t  cmd;
} tal_repeater_get_info_t;

typedef struct {
    uint8_t  cmd;
    uint8_t  conn_num_max;
} tal_repeater_get_info_rsp_t;

typedef struct {
    int8_t rssi;
    TAL_BLE_ADDR_T peer_addr;
    tal_repeater_ble_data_t adv_data;
    tal_repeater_ble_data_t scan_rsp;
} tal_repeater_adv_data_t;

typedef struct {
    uint8_t cmd;
    tal_repeater_adv_data_t advs[TAL_REPEATER_ADV_MAX_NUM];
} tal_repeater_adv_report_t;

typedef struct {
    uint8_t  cmd;
    TAL_BLE_ADDR_T peer_addr;
    uint8_t  status;
    uint16_t conn_interval;
    uint16_t slave_latency;
    uint16_t conn_sup_timeout;
    uint16_t mtu;
} tal_repeater_conn_success_report_t;

typedef struct {
    uint8_t  cmd;
    TAL_BLE_ADDR_T peer_addr;
    uint8_t  status;
    uint8_t  reason;
} tal_repeater_disconn_report_t;

typedef struct {
    uint8_t  cmd;
    TAL_BLE_ADDR_T peer_addr;
    uint8_t  status;
} tal_repeater_conn_failed_report_t;

typedef struct {
    uint8_t  cmd;
    TAL_BLE_ADDR_T peer_addr;
    uint16_t mtu;
} tal_repeater_mtu_report_t;

typedef struct {
    uint8_t  cmd;
    TAL_BLE_ADDR_T peer_addr;
    uint16_t len;
    uint8_t  buf[];
} tal_repeater_data_report_t;

typedef struct {
    uint32_t recv_len;
    uint32_t recv_len_max;
    uint8_t  *recv_data;
} tal_repeater_air_recv_packet;
#pragma pack()

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
static tal_repeater_adv_report_t sg_adv_report = {0};
static tal_repeater_adv_report_t sg_adv_report2 = {0};
static uint8_t sg_adv_data_is_valid[TAL_REPEATER_ADV_MAX_NUM] = {0};
static uint32_t sg_adv_count = 0;

static tal_repeater_slave_info_t sg_slave_info[TAL_REPEATER_SLAVE_MAX_NUM] = {0};
static TAL_BLE_PEER_INFO_T sg_conn_peer_info = {0};
static uint16_t sg_tmp_mtu = 0;
static uint16_t sg_is_connecting = 0;
static TAL_BLE_ADDR_T sg_is_connecting_addr = {0};
static uint32_t sg_conn_timeout_time = 0;
static uint32_t sg_0x08_count = 0;
static uint32_t sg_0x08_count_max = 0;
static uint8_t  sg_is_gateway_disconn = 0;

static TIMER_ID sg_adv_report_timer_id = NULL;
static TIMER_ID sg_conn_timer_id = NULL;
static TIMER_ID sg_0x08_timer_id = NULL;

static TAL_BLE_CONN_PARAMS_T sg_conn_param = {0};

static tal_repeater_air_recv_packet air_recv_packet = {0};
static frm_trsmitr_proc_s ty_trsmitr_proc = {0};
static frm_trsmitr_proc_s ty_trsmitr_proc_send = {0};

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/
static OPERATE_RET tal_repeater_rsp(uint8_t *buf, uint32_t size);
static uint32_t adv_mac_is_exist(uint8_t *p_addr);
static bool adv_add(TAL_BLE_ADV_REPORT_T *adv_report);
static bool adv_replace(TAL_BLE_ADV_REPORT_T *adv_report, uint32_t idx);
static void adv_report_timeout_handler(TIMER_ID timer_id, void *arg);
static void conn_timeout_handler(TIMER_ID timer_id, void *arg);
static void _0x08_timeout_handler(TIMER_ID timer_id, void *arg);
static void tal_repeater_air_recv_packet_free(void);
static uint32_t tal_repeater_ble_data_unpack(uint8_t *buf, uint32_t len);
extern OPERATE_RET tal_sw_timer_start_with_context(TIMER_ID timer_id, TIME_MS time_ms, TIMER_TYPE timer_type, VOID_T *arg);





OPERATE_RET tal_repeater_init(void)
{
    // slave info init
    for (uint32_t idx = 0; idx < TAL_REPEATER_SLAVE_MAX_NUM; idx++) {
        memset(&sg_slave_info[idx], 0, sizeof(tal_repeater_slave_info_t));
        sg_slave_info[idx].peer.conn_handle = TKL_BLE_GATT_INVALID_HANDLE;
        sg_slave_info[idx].mtu = TAL_REPEATER_DEFAULT_MTU;
    }

    tal_sw_timer_create(adv_report_timeout_handler, NULL, &sg_adv_report_timer_id);
    tal_sw_timer_create(conn_timeout_handler, NULL, &sg_conn_timer_id);
    tal_sw_timer_create(_0x08_timeout_handler, NULL, &sg_0x08_timer_id);

    return OPRT_OK;
}

static OPERATE_RET tal_repeater_set_scan_handler(uint8_t *buf, uint32_t size)
{
    tal_repeater_set_adv_t *set_adv = (void *)buf;

    if (set_adv->state) {
        TAL_BLE_SCAN_PARAMS_T scan_param = {0};
        scan_param.type = TAL_BLE_SCAN_TYPE_ACTIVE;
        scan_param.scan_interval = 100 * 8 / 5;
        scan_param.scan_window = 50 * 8 / 5;
        scan_param.timeout = 0;
        scan_param.filter_dup = 1;
        tal_ble_scan_start(&scan_param);
        tal_sw_timer_start(sg_adv_report_timer_id, TAL_REPEATER_ADV_REPORT_TIME, TAL_TIMER_CYCLE);
    } else {
        tal_ble_scan_stop();
        tal_sw_timer_stop(sg_adv_report_timer_id);
    }

    // rsp
    tal_repeater_set_adv_rsp_t set_adv_rsp = {0};
    set_adv_rsp.cmd = set_adv->cmd;
    set_adv_rsp.status = 0x00;
    tal_repeater_rsp((void *)&set_adv_rsp, sizeof(tal_repeater_set_adv_rsp_t));

    return OPRT_OK;
}

static OPERATE_RET tal_repeater_conn_handler(uint8_t *buf, uint32_t size)
{
    tal_repeater_conn_t *conn = (void *)buf;
    tal_repeater_conn_rsp_t conn_rsp = {0};

    tal_util_reverse_byte(&conn->conn_interval_min, sizeof(uint16_t));
    tal_util_reverse_byte(&conn->conn_interval_max, sizeof(uint16_t));
    tal_util_reverse_byte(&conn->slave_latency, sizeof(uint16_t));
    tal_util_reverse_byte(&conn->conn_sup_timeout, sizeof(uint16_t));
    tal_util_reverse_byte(&conn->connection_timeout, sizeof(uint32_t));

    if (tal_repeater_slave_info_count() >= TAL_REPEATER_SLAVE_MAX_NUM) {
        conn_rsp.status = 0x01; // Exceed conn number
    } else if (sg_is_connecting == 1) {
        conn_rsp.status = 0x02; // Connecting
    } else {
        sg_conn_peer_info.peer_addr = conn->peer_addr;
        tal_util_reverse_byte(sg_conn_peer_info.peer_addr.addr, 6);
        sg_conn_param.min_conn_interval = (conn->conn_interval_min == 0) ? (BOARD_CENTRAL_CONN_INTERVAL) * 4 / 5 : conn->conn_interval_min;
        sg_conn_param.max_conn_interval = (conn->conn_interval_max == 0) ? (BOARD_CENTRAL_CONN_INTERVAL) * 4 / 5 : conn->conn_interval_max;
        sg_conn_param.latency = (conn->slave_latency == 0) ? 3 : conn->slave_latency;
        sg_conn_param.conn_sup_timeout = (conn->conn_sup_timeout == 0) ? 6000 / 10 : conn->conn_sup_timeout;
        OPERATE_RET ret = tal_ble_connect_and_discovery(sg_conn_peer_info, &sg_conn_param);
        if (ret == OPRT_OK) {
            conn_rsp.status = 0x00;
            sg_is_connecting = 1;
            sg_is_connecting_addr = sg_conn_peer_info.peer_addr;
            sg_tmp_mtu = TAL_REPEATER_DEFAULT_MTU;
            sg_conn_timeout_time = (conn->connection_timeout == 0) ? TAL_REPEATER_CONN_TIMEOUT_TIME : conn->connection_timeout;
            tal_sw_timer_start_with_context(sg_conn_timer_id, sg_conn_timeout_time, TAL_TIMER_ONCE, &sg_conn_peer_info.peer_addr);
        } else {
            TAL_PR_INFO("tal_ble_connect_and_discovery ret: %d", ret);
        }
    }

    conn_rsp.cmd = conn->cmd;
    conn_rsp.peer_addr = conn->peer_addr;
    tal_repeater_rsp((void *)&conn_rsp, sizeof(tal_repeater_conn_rsp_t));

    return OPRT_OK;
}

static OPERATE_RET tal_repeater_disconn_handler(uint8_t *buf, uint32_t size)
{
    tal_repeater_disconn_t *disconn = (void *)buf;
    tal_repeater_disconn_rsp_t disconn_rsp = {0};

    tal_util_reverse_byte(disconn->peer_addr.addr, 6);

    uint16_t conn_handle = tal_repeater_slave_info_get_connhandle_from_mac(disconn->peer_addr);
    if (conn_handle == TKL_BLE_GATT_INVALID_HANDLE) {
        disconn_rsp.status = 0x01; // Conn not exist
    } else {
        TAL_BLE_PEER_INFO_T peer_info = {0};
        peer_info.conn_handle = conn_handle;
        tal_ble_disconnect(peer_info);
        disconn_rsp.status = 0x00;
    }

    disconn_rsp.cmd = disconn->cmd;
    tal_util_reverse_byte(disconn->peer_addr.addr, 6);
    disconn_rsp.peer_addr = disconn->peer_addr;
    tal_repeater_rsp((void *)&disconn_rsp, sizeof(tal_repeater_disconn_rsp_t));

    return OPRT_OK;
}

static OPERATE_RET tal_repeater_rx_data_handler(uint8_t *buf, uint32_t size)
{
    tal_repeater_rx_data_t *rx_data = (void *)buf;
    tal_repeater_rx_data_rsp_t rx_data_rsp = {0};

    tal_util_reverse_byte(&rx_data->len, sizeof(uint16_t));
    tal_util_reverse_byte(rx_data->peer_addr.addr, 6);

    uint16_t conn_handle = tal_repeater_slave_info_get_connhandle_from_mac(rx_data->peer_addr);
    if (conn_handle == TKL_BLE_GATT_INVALID_HANDLE) {
        rx_data_rsp.status = 0x01;
    } else if (rx_data->len > TAL_REPEATER_DATA_LEN_MAX) {
        rx_data_rsp.status = 0x02;
    } else {
        mtp_ret ret = MTP_OK;
        uint16_t send_len = 0;
        uint8_t *p_buf = NULL;
        trsmitr_init(&ty_trsmitr_proc_send);
        do {
            uint16_t tmp_mtu = tal_repeater_slave_info_get_mtu(rx_data->peer_addr) - 3;
            ret = trsmitr_send_pkg_encode_with_packet_length(&ty_trsmitr_proc_send, tmp_mtu, TUYA_BLE_PROTOCOL_VERSION_HIGN, rx_data->buf, rx_data->len);
            if (MTP_OK != ret && MTP_TRSMITR_CONTINUE != ret) {
                return 1;
            }
            send_len = get_trsmitr_subpkg_len(&ty_trsmitr_proc_send);
            p_buf = get_trsmitr_subpkg(&ty_trsmitr_proc_send);
            // send data
            TAL_BLE_PEER_INFO_T peer_info = {0};
            peer_info.conn_handle = conn_handle;
            peer_info.char_handle[0] = tal_repeater_slave_info_get_charhandle_from_mac(rx_data->peer_addr);
            TAL_BLE_DATA_T data = {0};
            data.len = send_len;
            data.p_data = p_buf;
            tal_ble_client_common_send(peer_info, &data);
        } while (ret == MTP_TRSMITR_CONTINUE);

//        // send data
//        TAL_BLE_PEER_INFO_T peer_info = {0};
//        peer_info.conn_handle = conn_handle;
//        peer_info.char_handle[0] = tal_repeater_slave_info_get_charhandle_from_mac(rx_data->peer_addr);
//        TAL_BLE_DATA_T data = {0};
//        data.len = rx_data->len;
//        data.p_data = rx_data->buf;
//        tal_ble_client_common_send(peer_info, &data);
    }

    rx_data_rsp.cmd = rx_data->cmd;
    tal_util_reverse_byte(rx_data->peer_addr.addr, 6);
    rx_data_rsp.peer_addr = rx_data->peer_addr;
    tal_repeater_rsp((void *)&rx_data_rsp, sizeof(tal_repeater_rx_data_rsp_t));

    return OPRT_OK;
}

static OPERATE_RET tal_repeater_get_rssi_handler(uint8_t *buf, uint32_t size)
{
    tal_repeater_get_rssi_t *get_rssi = (void *)buf;
    tal_repeater_get_rssi_rsp_t get_rssi_rsp = {0};

    tal_util_reverse_byte(get_rssi->peer_addr.addr, 6);

    uint16_t conn_handle = tal_repeater_slave_info_get_connhandle_from_mac(get_rssi->peer_addr);
    if (conn_handle == TKL_BLE_GATT_INVALID_HANDLE) {
        get_rssi_rsp.cmd = TAL_REPEATER_CMD_GET_RSSI;
        get_rssi_rsp.peer_addr = get_rssi->peer_addr;
        tal_util_reverse_byte(get_rssi_rsp.peer_addr.addr, 6);
        get_rssi_rsp.rssi = 0xFF;
        tal_repeater_rsp((void *)&get_rssi_rsp, sizeof(tal_repeater_get_rssi_rsp_t));
    } else {
        TAL_BLE_PEER_INFO_T info = {0};
        info.conn_handle = conn_handle;
        tal_ble_rssi_get(info);
    }

    return OPRT_OK;
}

OPERATE_RET tal_repeater_get_rssi_report_handler(TAL_BLE_CONN_RSSI_EVT_T link_rssi)
{
    TAL_PR_INFO("TAL_BLE_EVT_CONN_RSSI: conn_handle: %d, rssi: %d", link_rssi.conn_handle, (int8_t)link_rssi.rssi);

    tal_repeater_get_rssi_rsp_t get_rssi_rsp = {0};
    get_rssi_rsp.cmd = TAL_REPEATER_CMD_GET_RSSI;
    get_rssi_rsp.peer_addr = tal_repeater_slave_info_get_mac_from_connhandle(link_rssi.conn_handle);
    tal_util_reverse_byte(get_rssi_rsp.peer_addr.addr, 6);
    get_rssi_rsp.rssi = link_rssi.rssi;
    tal_repeater_rsp((void *)&get_rssi_rsp, sizeof(tal_repeater_get_rssi_rsp_t));

    return OPRT_OK;
}

static OPERATE_RET tal_repeater_get_info_handler(uint8_t *buf, uint32_t size)
{
    tal_repeater_get_info_t *get_info = (void *)buf;
    tal_repeater_get_info_rsp_t get_info_rsp = {0};

    get_info_rsp.cmd = get_info->cmd;
    get_info_rsp.conn_num_max = TAL_REPEATER_SLAVE_MAX_NUM;
    tal_repeater_rsp((void *)&get_info_rsp, sizeof(tal_repeater_get_info_rsp_t));

    return OPRT_OK;
}

OPERATE_RET tal_repeater_adv_report_handler(void)
{
#if defined(TUYA_SDK_TEST) && (TUYA_SDK_TEST == 1)
    uint8_t tmp_buf[200] = {TAL_REPEATER_CMD_REPORT_ADV};
#endif
    TKL_ENTER_CRITICAL();
    uint32_t count = 0;

    for (uint32_t idx = 0; idx < TAL_REPEATER_ADV_MAX_NUM; idx++) {
        if (sg_adv_data_is_valid[idx] == 1) {
            sg_adv_report2.advs[count] = sg_adv_report.advs[idx];
            tal_util_reverse_byte(sg_adv_report2.advs[count].peer_addr.addr, 6);

#if defined(TUYA_SDK_TEST) && (TUYA_SDK_TEST == 1)
            memcpy(1 + tmp_buf + count * 8, &sg_adv_report.advs[idx], 8);
#endif

            count++;
            sg_adv_data_is_valid[idx] = 0;
        }
    }

    sg_adv_report2.cmd = TAL_REPEATER_CMD_REPORT_ADV;
    tal_repeater_rsp((void *)&sg_adv_report2, 1 + sizeof(tal_repeater_adv_data_t)*count);

    memset(&sg_adv_report, 0, sizeof(tal_repeater_adv_report_t));
    memset(&sg_adv_report2, 0, sizeof(tal_repeater_adv_report_t));
    sg_adv_count = 0;
    TKL_EXIT_CRITICAL();

#if defined(TUYA_SDK_TEST) && (TUYA_SDK_TEST == 1)
//    test_cmd_send(TEST_ID_GET(TEST_GID_CUSTOM, TEST_CID_REPEATER), (void*)&sg_adv_report2, 1 + sizeof(tal_repeater_adv_data_t)*count);
    test_cmd_send(TEST_ID_GET(TEST_GID_CUSTOM, TEST_CID_REPEATER), tmp_buf, 1 + count * 8);
#endif

    return OPRT_OK;
}

OPERATE_RET tal_repeater_conn_success_report_handler(TAL_BLE_CONNECT_EVT_T *connect)
{
    sg_is_connecting = 0;
    tal_sw_timer_stop(sg_conn_timer_id);

    tal_repeater_slave_info_add(connect);

    tal_repeater_slave_info_set_mtu(connect->peer.peer_addr, sg_tmp_mtu);

    tal_repeater_conn_success_report_t conn_success_report = {0};
    conn_success_report.cmd = TAL_REPEATER_CMD_REPORT_CONN_STATE;
    conn_success_report.peer_addr = connect->peer.peer_addr;
    tal_util_reverse_byte(conn_success_report.peer_addr.addr, 6);
    conn_success_report.status = 0x00;
    conn_success_report.conn_interval = connect->conn_param.max_conn_interval;
    conn_success_report.slave_latency = connect->conn_param.latency;
    conn_success_report.conn_sup_timeout = connect->conn_param.conn_sup_timeout;
    conn_success_report.mtu = tal_repeater_slave_info_get_mtu(connect->peer.peer_addr);
    tal_util_reverse_byte(&conn_success_report.mtu, sizeof(uint16_t));
    tal_repeater_rsp((void *)&conn_success_report, sizeof(tal_repeater_conn_success_report_t));

    return OPRT_OK;
}

OPERATE_RET tal_repeater_gateway_conn_success_handler(TAL_BLE_CONNECT_EVT_T *connect)
{
    sg_is_gateway_disconn = 0;
    return OPRT_OK;
}

OPERATE_RET tal_repeater_disconn_report_handler(TAL_BLE_DISCONNECT_EVT_T *disconnect)
{
    tal_repeater_disconn_report_t disconn_report = {0};
    disconn_report.cmd = TAL_REPEATER_CMD_REPORT_CONN_STATE;
    TAL_BLE_ADDR_T peer_addr = tal_repeater_slave_info_get_mac_from_connhandle(disconnect->peer.conn_handle);
    if (peer_addr.type == (TAL_BLE_ADDR_TYPE_E)0xFF) {
        sg_is_connecting = 0;
        disconn_report.peer_addr = sg_is_connecting_addr;
    } else {
        disconn_report.peer_addr = peer_addr;
    }
    tal_util_reverse_byte(disconn_report.peer_addr.addr, 6);
    disconn_report.status = 0x01;
    disconn_report.reason = disconnect->reason;
    tal_repeater_rsp((void *)&disconn_report, sizeof(tal_repeater_disconn_report_t));

    if (disconnect->reason == 0x08) {
        sg_0x08_count++;
        if (sg_0x08_count == 1) {
            sg_0x08_count_max = tal_repeater_slave_info_count();
            tal_sw_timer_start(sg_0x08_timer_id, 2000, TAL_TIMER_ONCE);
        } else if (sg_0x08_count == (sg_0x08_count_max)) {
            TAL_PR_INFO("All devices are disconnected due to 0x08. Use a reset to temporarily fix the problem");
            tuya_ble_disconnect_and_reset_timer_start();
        }
    }

    tal_repeater_slave_info_delete(disconnect);

    if (disconnect->reason == 0x16) {
        tal_repeater_slave_info_disconnect_all();
    }

//    if (disconnect->reason == 0x3E || disconnect->reason == 0x08) {
//        TAL_BLE_PEER_INFO_T sg_conn_peer_info = {0};
//        sg_conn_peer_info.peer_addr = disconn_report.peer_addr;
//        OPERATE_RET ret = tal_ble_connect_and_discovery(sg_conn_peer_info, &sg_conn_param);
//        if (ret == OPRT_OK) {
//            sg_is_connecting = 1;
//            tal_sw_timer_start_with_context(sg_conn_timer_id, sg_conn_timeout_time, TAL_TIMER_ONCE, &disconn_report.peer_addr);
//        } else {
//            TAL_PR_INFO("tal_ble_connect_and_discovery ret: %d", ret);
//        }
//    }

    return OPRT_OK;
}

OPERATE_RET tal_repeater_gateway_disconn_handler(TAL_BLE_DISCONNECT_EVT_T *disconnect)
{
    sg_is_gateway_disconn = 1;
    tal_repeater_slave_info_disconnect_all();

    return OPRT_OK;
}

OPERATE_RET tal_repeater_conn_failed_report_handler(TAL_BLE_ADDR_T *peer_addr, uint8_t status)
{
    sg_is_connecting = 0;
    tal_sw_timer_stop(sg_conn_timer_id);

    void gap_stop_conn(void);
    gap_stop_conn();
    TAL_BLE_PEER_INFO_T peer = {0};
    peer.conn_handle = tal_repeater_slave_info_get_connhandle_from_mac(*peer_addr);
    tal_ble_disconnect(peer);

    tal_repeater_conn_failed_report_t conn_failed_report = {0};
    conn_failed_report.cmd = TAL_REPEATER_CMD_REPORT_CONN_STATE;
    conn_failed_report.peer_addr = *peer_addr;
    tal_util_reverse_byte(conn_failed_report.peer_addr.addr, 6);
    conn_failed_report.status = status;
    tal_repeater_rsp((void *)&conn_failed_report, sizeof(tal_repeater_conn_failed_report_t));

    return OPRT_OK;
}

OPERATE_RET tal_repeater_mtu_report_handler(TAL_BLE_EXCHANGE_MTU_EVT_T *mtu_evt)
{
    TAL_BLE_ADDR_T peer_addr = tal_repeater_slave_info_get_mac_from_connhandle(mtu_evt->conn_handle);

    // Update mtu before TAL_BLE_EVT_CENTRAL_CONNECT_DISCOVERY
    if ((uint8_t)peer_addr.type == 0xFF) {
        sg_tmp_mtu = mtu_evt->mtu;
    } else {
        tal_repeater_mtu_report_t mtu_report = {0};
        mtu_report.cmd = TAL_REPEATER_CMD_REPORT_MTU;
        mtu_report.peer_addr = tal_repeater_slave_info_get_mac_from_connhandle(mtu_evt->conn_handle);
        tal_util_reverse_byte(mtu_report.peer_addr.addr, 6);
        mtu_report.mtu = mtu_evt->mtu;
        tal_repeater_rsp((void *)&mtu_report, sizeof(tal_repeater_mtu_report_t));
    }

    return OPRT_OK;
}

OPERATE_RET tal_repeater_data_report_handler(TAL_BLE_DATA_REPORT_T *data)
{
//    TAL_PR_INFO("TAL_BLE_EVT_NOTIFY_RX");

    if (tal_repeater_ble_data_unpack(data->report.p_data, data->report.len) != 0) {
        return OPRT_RESOURCE_NOT_READY;
    }

    if (air_recv_packet.recv_len > TUYA_BLE_AIR_FRAME_MAX) {
        tal_repeater_air_recv_packet_free();
        TAL_PR_INFO("air_recv_packet.recv_len bigger than TUYA_BLE_AIR_FRAME_MAX.");
        return OPRT_RESOURCE_NOT_READY;
    }

    uint32_t report_size = sizeof(tal_repeater_data_report_t) + air_recv_packet.recv_len;
    uint8_t *tmp_buf = tal_malloc(report_size);
    if (tmp_buf) {
        tal_repeater_data_report_t sg_data_report = {0};
        sg_data_report.cmd = TAL_REPEATER_CMD_TX_DATA;
        sg_data_report.peer_addr = tal_repeater_slave_info_get_mac_from_connhandle(data->peer.conn_handle);
        tal_util_reverse_byte(sg_data_report.peer_addr.addr, 6);
        sg_data_report.len = air_recv_packet.recv_len;
        tal_util_reverse_byte(&sg_data_report.len, sizeof(uint16_t));

        memcpy(tmp_buf, &sg_data_report, sizeof(tal_repeater_data_report_t));
        memcpy(tmp_buf + sizeof(tal_repeater_data_report_t), air_recv_packet.recv_data, air_recv_packet.recv_len);

        tal_repeater_rsp(tmp_buf, report_size);

        tal_free(tmp_buf);
        tal_repeater_air_recv_packet_free();
    }

    return OPRT_OK;
}

OPERATE_RET tal_repeater_handler(tuya_ble_app_passthrough_data_t *data)
{
    uint8_t cmd = data->p_data[0];

#if defined(ENABLE_LOG) && (ENABLE_LOG == 1)
    uint32_t log_max_len = (data->data_len > 16) ? 16 : data->data_len;
    TAL_PR_HEXDUMP_INFO("tal_repeater_cmd", data->p_data, log_max_len);
#endif

    switch (cmd) {
    case TAL_REPEATER_CMD_SET_SCAN: {
        tal_repeater_set_scan_handler(data->p_data, data->data_len);
    }
    break;

    case TAL_REPEATER_CMD_CONN: {
        tal_repeater_conn_handler(data->p_data, data->data_len);
    }
    break;

    case TAL_REPEATER_CMD_DISCONN: {
        tal_repeater_disconn_handler(data->p_data, data->data_len);
    }
    break;

    case TAL_REPEATER_CMD_RX_DATA: {
        tal_repeater_rx_data_handler(data->p_data, data->data_len);
    }
    break;

    case TAL_REPEATER_CMD_GET_RSSI: {
        tal_repeater_get_rssi_handler(data->p_data, data->data_len);
    }
    break;

    case TAL_REPEATER_CMD_GET_INFO: {
        tal_repeater_get_info_handler(data->p_data, data->data_len);
    }
    break;

    default: {
    } break;
    }

    return 0;
}

OPERATE_RET tal_repeater_adv_report_cb(TAL_BLE_ADV_REPORT_T *adv_report)
{
    TKL_ENTER_CRITICAL();
    uint8_t *data = adv_report->p_data;
//    if (adv_report->rssi < 80) {
    if (adv_report->adv_type == TAL_BLE_ADV_DATA) {
        if ((data[5] == 0x50 && data[6] == 0xFD) || (data[5] == 0x01 && data[6] == 0xA2)) {
            uint32_t idx = adv_mac_is_exist(adv_report->peer_addr.addr);
            if (idx < TAL_REPEATER_ADV_MAX_NUM) {
                adv_replace(adv_report, idx);
            } else {
                adv_add(adv_report);
            }
        }
    } else if (adv_report->adv_type == TAL_BLE_RSP_DATA) {
        if ((data[2] == 0xD0 && data[3] == 0x07) || (data[6] == 0xD0 && data[7] == 0x07)) {
            uint32_t idx = adv_mac_is_exist(adv_report->peer_addr.addr);
            if (idx < TAL_REPEATER_ADV_MAX_NUM) {
                tal_repeater_adv_data_t *p_adv = &sg_adv_report.advs[idx];

                p_adv->scan_rsp.len = adv_report->data_len;
                memcpy(&p_adv->scan_rsp.value, adv_report->p_data, p_adv->scan_rsp.len);

                p_adv->rssi = adv_report->rssi;
                sg_adv_data_is_valid[idx] = 1;
            }
        }
    }
//    }
    TKL_EXIT_CRITICAL();

    return OPRT_OK;
}

OPERATE_RET tal_repeater_slave_info_add(TAL_BLE_CONNECT_EVT_T *connect)
{
    if (connect == NULL) {
        return OPRT_INVALID_PARM;
    }

    for (uint32_t idx = 0; idx < TAL_REPEATER_SLAVE_MAX_NUM; idx++) {
        if (sg_slave_info[idx].peer.conn_handle == TKL_BLE_GATT_INVALID_HANDLE) {
            memcpy(&sg_slave_info[idx].peer, &connect->peer, sizeof(TAL_BLE_PEER_INFO_T));
            break;
        }
    }

    tal_repeater_slave_info_list();

    return OPRT_OK;
}

OPERATE_RET tal_repeater_slave_info_delete(TAL_BLE_DISCONNECT_EVT_T *disconnect)
{
    if (disconnect == NULL) {
        return OPRT_INVALID_PARM;
    }

    for (uint32_t idx = 0; idx < TAL_REPEATER_SLAVE_MAX_NUM; idx++) {
        if (sg_slave_info[idx].peer.conn_handle == disconnect->peer.conn_handle) {
            memset(&sg_slave_info[idx], 0, sizeof(tal_repeater_slave_info_t));
            sg_slave_info[idx].mtu = TAL_REPEATER_DEFAULT_MTU;
            sg_slave_info[idx].peer.conn_handle = TKL_BLE_GATT_INVALID_HANDLE;
            break;
        }
    }

    tal_repeater_slave_info_list();

    return OPRT_OK;
}

uint16_t tal_repeater_slave_info_get_connhandle_from_mac(TAL_BLE_ADDR_T peer_addr)
{
    for (uint32_t idx = 0; idx < TAL_REPEATER_SLAVE_MAX_NUM; idx++) {
        if (memcmp(&sg_slave_info[idx].peer.peer_addr, &peer_addr, sizeof(TAL_BLE_ADDR_T)) == 0) {
            return sg_slave_info[idx].peer.conn_handle;
        }
    }

    return TKL_BLE_GATT_INVALID_HANDLE;
}

uint16_t tal_repeater_slave_info_get_charhandle_from_mac(TAL_BLE_ADDR_T peer_addr)
{
    for (uint32_t idx = 0; idx < TAL_REPEATER_SLAVE_MAX_NUM; idx++) {
        if (memcmp(&sg_slave_info[idx].peer.peer_addr, &peer_addr, sizeof(TAL_BLE_ADDR_T)) == 0) {
            return sg_slave_info[idx].peer.char_handle[TAL_COMMON_WRITE_CHAR_INDEX];
        }
    }

    return TKL_BLE_GATT_INVALID_HANDLE;
}

TAL_BLE_ADDR_T tal_repeater_slave_info_get_mac_from_connhandle(uint16_t conn_handle)
{
    TAL_BLE_ADDR_T tmp_peer_addr = {0};
    tmp_peer_addr.type = 0xFF;

    for (uint32_t idx = 0; idx < TAL_REPEATER_SLAVE_MAX_NUM; idx++) {
        if (sg_slave_info[idx].peer.conn_handle == conn_handle) {
            return sg_slave_info[idx].peer.peer_addr;
        }
    }

    return tmp_peer_addr;
}

OPERATE_RET tal_repeater_slave_info_set_mtu(TAL_BLE_ADDR_T peer_addr, uint16_t mtu)
{
    for (uint32_t idx = 0; idx < TAL_REPEATER_SLAVE_MAX_NUM; idx++) {
        if (memcmp(&sg_slave_info[idx].peer.peer_addr, &peer_addr, sizeof(TAL_BLE_ADDR_T)) == 0) {
            sg_slave_info[idx].mtu = mtu;
        }
    }

    return OPRT_OK;
}

uint16_t tal_repeater_slave_info_get_mtu(TAL_BLE_ADDR_T peer_addr)
{
    for (uint32_t idx = 0; idx < TAL_REPEATER_SLAVE_MAX_NUM; idx++) {
        if (memcmp(&sg_slave_info[idx].peer.peer_addr, &peer_addr, sizeof(TAL_BLE_ADDR_T)) == 0) {
            return sg_slave_info[idx].mtu;
        }
    }

    return TAL_REPEATER_DEFAULT_MTU;
}

OPERATE_RET tal_repeater_slave_info_disconnect_all(void)
{
    if (sg_is_gateway_disconn) {
        uint32_t idx = tal_repeater_slave_info_get_valid();
        if (idx < TAL_REPEATER_SLAVE_MAX_NUM) {
            TAL_PR_INFO("tal_ble_disconnect conn_handle: %d", sg_slave_info[idx].peer.conn_handle);

            TAL_BLE_PEER_INFO_T info = {0};
            info.conn_handle = sg_slave_info[idx].peer.conn_handle;
            uint32_t ret = tal_ble_disconnect(info);
            if (ret != OPRT_OK) {
                TAL_PR_INFO("tal_ble_disconnect ret: %d", ret);
            }
        }
    }

    return OPRT_OK;
}

uint32_t tal_repeater_slave_info_get_valid(void)
{
    for (uint32_t idx = 0; idx < TAL_REPEATER_SLAVE_MAX_NUM; idx++) {
        if (sg_slave_info[idx].peer.conn_handle != TKL_BLE_GATT_INVALID_HANDLE) {
            return idx;
        }
    }

    return TAL_REPEATER_SLAVE_MAX_NUM;
}

uint32_t tal_repeater_slave_info_count(void)
{
    uint32_t count = 0;
    for (uint32_t idx = 0; idx < TAL_REPEATER_SLAVE_MAX_NUM; idx++) {
        if (sg_slave_info[idx].peer.conn_handle != TKL_BLE_GATT_INVALID_HANDLE) {
            count++;
        }
    }

    return count;
}

OPERATE_RET tal_repeater_slave_info_list(void)
{
#if defined(ENABLE_LOG) && (ENABLE_LOG == 1)
    for (uint32_t idx = 0; idx < TAL_REPEATER_SLAVE_MAX_NUM; idx++) {
        if (sg_slave_info[idx].peer.conn_handle != TKL_BLE_GATT_INVALID_HANDLE) {
            uint8_t *p_mac = sg_slave_info[idx].peer.peer_addr.addr;
            TAL_PR_INFO("tal_repeater_slave_info_list, idx: %d, handle: %d, mac: %02x %02x %02x %02x %02x %02x", idx, sg_slave_info[idx].peer.conn_handle, p_mac[5], p_mac[4], p_mac[3], p_mac[2], p_mac[1], p_mac[0]);
        }
    }
#endif

    return OPRT_OK;
}

static OPERATE_RET tal_repeater_rsp(uint8_t *buf, uint32_t size)
{
#if defined(TUYA_SDK_TEST) && (TUYA_SDK_TEST == 1)
    if (buf[0] != TAL_REPEATER_CMD_REPORT_ADV) {
        test_cmd_send(TEST_ID_GET(TEST_GID_CUSTOM, TEST_CID_REPEATER), buf, size);
    }
#endif

    tuya_ble_app_passthrough_data_t rsp = {0};
    rsp.type = TAL_BLE_APP_PASSTHROUGH_SUBCMD_REPEATER;
    rsp.data_len = size;
    rsp.p_data = buf;

#if defined(ENABLE_LOG) && (ENABLE_LOG == 1)
    if (rsp.p_data[0] != 0x80) {
        uint32_t log_max_len = (rsp.data_len > 16) ? 16 : rsp.data_len;
        TAL_PR_HEXDUMP_INFO("tal_repeater_rsp", rsp.p_data, log_max_len);
    }
#endif

    return tal_ble_app_passthrough_data_send(&rsp);
}

static uint32_t adv_mac_is_exist(uint8_t *p_addr)
{
    for (uint32_t idx = 0; idx < TAL_REPEATER_ADV_MAX_NUM; idx++) {
        if (memcmp(sg_adv_report.advs[idx].peer_addr.addr, p_addr, 6) == 0) {
            return idx;
        }
    }

    return TAL_REPEATER_ADV_MAX_NUM;
}

static bool adv_add(TAL_BLE_ADV_REPORT_T *adv_report)
{
    tal_repeater_adv_data_t *p_adv = &sg_adv_report.advs[sg_adv_count];

    memcpy(&p_adv->peer_addr, &adv_report->peer_addr, sizeof(TAL_BLE_ADDR_T));
    p_adv->adv_data.len = adv_report->data_len;
    memcpy(&p_adv->adv_data.value, adv_report->p_data, p_adv->adv_data.len);

    sg_adv_count++;
    if (sg_adv_count == TAL_REPEATER_ADV_MAX_NUM) {
        sg_adv_count = 0;
//        tuya_ble_custom_evt_send(APP_EVT_2);
    }

    return OPRT_OK;
}

static bool adv_replace(TAL_BLE_ADV_REPORT_T *adv_report, uint32_t idx)
{
    tal_repeater_adv_data_t *p_adv = &sg_adv_report.advs[idx];

    memcpy(&p_adv->peer_addr, &adv_report->peer_addr, sizeof(TAL_BLE_ADDR_T));
    p_adv->adv_data.len = adv_report->data_len;
    memcpy(&p_adv->adv_data.value, adv_report->p_data, p_adv->adv_data.len);

    return OPRT_OK;
}

static void adv_report_timeout_handler(TIMER_ID timer_id, void *arg)
{
    tuya_ble_custom_evt_send(APP_EVT_2);
}

static void conn_timeout_handler(TIMER_ID timer_id, void *arg)
{
    TAL_BLE_ADDR_T *peer_addr = arg;
    tal_repeater_conn_failed_report_handler(peer_addr, 0x02);
}

static void _0x08_timeout_handler(TIMER_ID timer_id, void *arg)
{
    sg_0x08_count = 0;
}

static void tal_repeater_air_recv_packet_free(void)
{
    if (air_recv_packet.recv_data) {
        tuya_ble_free(air_recv_packet.recv_data);
        air_recv_packet.recv_data = NULL;
        air_recv_packet.recv_len_max = 0;
        air_recv_packet.recv_len = 0;
    }
}

static uint32_t tal_repeater_ble_data_unpack(uint8_t *buf, uint32_t len)
{
    static uint32_t offset = 0;

    mtp_ret ret = trsmitr_recv_pkg_decode(&ty_trsmitr_proc, buf, len);
    if (MTP_OK != ret && MTP_TRSMITR_CONTINUE != ret) {
        tal_repeater_air_recv_packet_free();
        return 1;
    }

    if (FRM_PKG_FIRST == ty_trsmitr_proc.pkg_desc) {
        tal_repeater_air_recv_packet_free();

        air_recv_packet.recv_len_max = get_trsmitr_frame_total_len(&ty_trsmitr_proc);
        if ((air_recv_packet.recv_len_max > TUYA_BLE_AIR_FRAME_MAX) || (air_recv_packet.recv_len_max == 0)) {
            tal_repeater_air_recv_packet_free();
            TAL_PR_INFO("tal_repeater_ble_data_unpack total size [%d ]error.", air_recv_packet.recv_len_max);
            return 2;
        }

        air_recv_packet.recv_len = 0;
        air_recv_packet.recv_data = tuya_ble_malloc(air_recv_packet.recv_len_max);
        if (air_recv_packet.recv_data == NULL) {
            tal_repeater_air_recv_packet_free();
            TAL_PR_ERR("tal_repeater_ble_data_unpack malloc failed.");
            return 2;
        }
        memset(air_recv_packet.recv_data, 0, air_recv_packet.recv_len_max);
        offset = 0;
    }

    if ((offset + get_trsmitr_subpkg_len(&ty_trsmitr_proc)) <= air_recv_packet.recv_len_max) {
        if (air_recv_packet.recv_data) {
            memcpy(air_recv_packet.recv_data + offset, get_trsmitr_subpkg(&ty_trsmitr_proc), get_trsmitr_subpkg_len(&ty_trsmitr_proc));
            offset += get_trsmitr_subpkg_len(&ty_trsmitr_proc);
            air_recv_packet.recv_len = offset;
        } else {
            tal_repeater_air_recv_packet_free();
            TAL_PR_ERR("tal_repeater_ble_data_unpack error.");
            return 2;
        }
    } else {
        ret = MTP_INVALID_PARAM;
        TAL_PR_ERR("tal_repeater_ble_data_unpack[] error:MTP_INVALID_PARAM");
        tal_repeater_air_recv_packet_free();
    }

    if (ret == MTP_OK) {
        offset = 0;
//        TAL_PR_DEBUG("tal_repeater_ble_data_unpack[%d]", air_recv_packet.recv_len);
        return 0;
    } else {
        return 2;
    }
}

#endif // TUYA_BLE_FEATURE_REPEATER_ENABLE

