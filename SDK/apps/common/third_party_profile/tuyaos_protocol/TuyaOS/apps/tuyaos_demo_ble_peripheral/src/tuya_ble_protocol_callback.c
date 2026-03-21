/**
 * @file tuya_ble_protocol_callback.c
 * @brief This is tuya_ble_protocol_callback file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */


#include "string.h"

#include "tal_memory.h"
#include "tal_log.h"
#include "tal_rtc.h"
#include "tal_utc.h"
#include "tal_bluetooth.h"
#include "tal_util.h"
#include "tal_sdk_test.h"

#include "tuya_ble_api.h"
#include "tuya_ble_ota.h"
#include "tuya_ble_attach_ota.h"
#include "tuya_ble_bulkdata_demo.h"
#include "tuya_ble_protocol_callback.h"
#include "tuya_ble_main.h"
#include "tuya_sdk_callback.h"
#include "tuya_ble_storage.h"

#include "app_dp_parser.h"
#include "app_led.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/


/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
STATIC tuya_ble_device_param_t tuya_ble_protocol_param = {
#if (TUYA_SDK_DEBUG_MODE)
    .use_ext_license_key = 1, //1-info in tuya_ble_protocol_callback.h, 0-auth info
    .device_id_len       = DEVICE_ID_LEN,
#else
    .use_ext_license_key = 0,
    .device_id_len       = 0,
#endif
    .p_type              = TUYA_BLE_PRODUCT_ID_TYPE_PID,
#if (TUYA_BLE_PROD_SUPPORT_OEM_TYPE == TUYA_BLE_PROD_OEM_TYPE_NONE)
    .product_id_len      = 8,
#else
    .product_id_len      = 0,
#endif
    .adv_local_name_len  = 4,
};

STATIC tuya_ble_timer_t disconnect_timer;
STATIC tuya_ble_timer_t update_conn_param_timer;

VOID_T *tuya_custom_task_handle = NULL;
VOID_T *tuya_custom_queue_handle = NULL;

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/


STATIC VOID_T tuya_ble_protocol_callback(tuya_ble_cb_evt_param_t *event)
{
    switch (event->evt) {
    case TUYA_BLE_CB_EVT_CONNECT_STATUS: {
        if (event->connect_status == BONDING_CONN) {
            TAL_PR_INFO("bonding and connecting");

            tuya_ble_update_conn_param_timer_start();
        }
    }
    break;

    case TUYA_BLE_CB_EVT_DP_DATA_RECEIVED: {
        //app_dp_parser(event->dp_received_data.p_data, event->dp_received_data.data_len);
        tuya_data_parse(event);
    }
    break;

#if (TUYA_BLE_AUTO_REQUEST_TIME_CONFIGURE == 1)
    case TUYA_BLE_CB_EVT_TIME_STAMP: {
        UINT32_T timestamp_s = 0;
        UINT32_T timestamp_ms = 0;
        tal_util_str_intstr2int((VOID_T *)event->timestamp_data.timestamp_string, 10, &timestamp_s);
        tal_util_str_intstr2int((VOID_T *)(event->timestamp_data.timestamp_string + 10), 3, &timestamp_ms);

        tal_rtc_time_set(timestamp_s);
        tal_utc_set_time_zone(event->timestamp_data.time_zone);

        TAL_PR_INFO("TUYA_BLE_CB_EVT_TIME_STAMP - time_zone: %d", event->timestamp_data.time_zone);
        TAL_PR_INFO("TUYA_BLE_CB_EVT_TIME_STAMP - timestamp: %d", timestamp_s);
    }
    break;
#endif

#if (TUYA_BLE_AUTO_REQUEST_TIME_CONFIGURE == 3)
    case TUYA_BLE_CB_EVT_TIME_STAMP_WITH_DST: {
        tal_rtc_time_set(event->timestamp_with_dst_data.timestamp);
        tal_utc_set_time_zone(event->timestamp_with_dst_data.time_zone);

        TAL_PR_INFO("TUYA_BLE_CB_EVT_TIME_STAMP_WITH_DST - time_zone: %d", event->timestamp_with_dst_data.time_zone);
        TAL_PR_INFO("TUYA_BLE_CB_EVT_TIME_STAMP_WITH_DST - timestamp: %d", event->timestamp_with_dst_data.timestamp);
        TAL_PR_INFO("TUYA_BLE_CB_EVT_TIME_STAMP_WITH_DST - n_years_dst: %d", event->timestamp_with_dst_data.n_years_dst);
        for (UINT32_T idx = 0 ; idx < event->timestamp_with_dst_data.n_years_dst; idx++) {
            UINT32_T timestamp_1 = 0;
            UINT32_T timestamp_2 = 0;
            tal_util_str_intstr2int(event->timestamp_with_dst_data.p_data + idx * 20, 10, &timestamp_1);
            tal_util_str_intstr2int(event->timestamp_with_dst_data.p_data + idx * 20 + 10, 10, &timestamp_2);
            TAL_PR_INFO("DST Year %d, From %d to %d", idx, timestamp_1, timestamp_2);
        }
    }
    break;
#endif

    case TUYA_BLE_CB_EVT_UNBOUND: {
        TAL_PR_INFO("TUYA_BLE_CB_EVT_UNBOUND");
    }
    break;

    case TUYA_BLE_CB_EVT_ANOMALY_UNBOUND: {
        TAL_PR_INFO("TUYA_BLE_CB_EVT_ANOMALY_UNBOUND");
    }
    break;

    case TUYA_BLE_CB_EVT_DEVICE_RESET: {
        TAL_PR_INFO("TUYA_BLE_CB_EVT_DEVICE_RESET");
    }
    break;

    case TUYA_BLE_CB_EVT_UNBIND_RESET_RESPONSE: {
        TAL_PR_INFO("TUYA_BLE_CB_EVT_UNBIND_RESET_RESPONSE");
    }
    break;

    case TUYA_BLE_CB_EVT_DP_QUERY: {
        TAL_PR_HEXDUMP_INFO("TUYA_BLE_CB_EVT_DP_QUERY", event->dp_query_data.p_data, event->dp_query_data.data_len);
        /* UINT8_T led_on = 1; */
        /* app_dp_report(WR_BASIC_LED, &led_on, SIZEOF(UINT8_T)); */
        tuya_info_indicate();
    }
    break;

    case TUYA_BLE_CB_EVT_OTA_DATA: {
        void tuya_ota_proc(uint16_t type, uint8_t *recv_data, uint32_t recv_len);
        tuya_ota_proc(event->ota_data.type, event->ota_data.p_data, event->ota_data.data_len);
        /*
                if (event->ota_data.p_data[0] == 0) {
        #if defined(TUYA_BLE_FEATURE_OTA_ENABLE) && (TUYA_BLE_FEATURE_OTA_ENABLE == 1)
                    tuya_ble_ota_handler(&event->ota_data);
        #endif
                } else {
        #if defined(TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE) && (TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE == 1)
                    tuya_ble_attach_ota_handler(&event->ota_data);
        #endif
                }
        */
    }
    break;

#if defined(TUYA_BLE_FEATURE_BULKDATA_ENABLE) && (TUYA_BLE_FEATURE_BULKDATA_ENABLE == 1)
    case TUYA_BLE_CB_EVT_BULK_DATA: {
        TAL_PR_INFO("TUYA_BLE_CB_EVT_BULK_DATA");
        tuya_ble_bulk_data_demo_handler(&event->bulk_req_data);
    }
    break;
#endif

    case TUYA_BLE_CB_EVT_UPDATE_LOGIN_KEY_VID: {
    } break;

    default: {
        TAL_PR_INFO("tuya_ble_protocol_callback Unprocessed event type 0x%04x", event->evt);
    }
    break;
    }

#if defined(TUYA_SDK_TEST) && (TUYA_SDK_TEST == 1)
    tal_sdk_test_ble_protocol_callback(event);
#endif

#if TUYA_BLE_USE_OS
    tuya_ble_event_response(event);
#endif
}

VOID_T tuya_ble_protocol_callback_task(VOID_T *p_param)
{
    tuya_ble_cb_evt_param_t event = {0};
    printf("tuya_ble_protocol_callback_task\n");
    while (1) {
        printf("tuya_ble_protocol recv\n");
        if (tuya_ble_os_msg_queue_recv(tuya_custom_queue_handle, &event, 0xFFFFFFFF) == true) {
            printf("ble os event %d\n", event.evt);
            tuya_ble_protocol_callback(&event);
        }
    }
}

extern const uint8_t *bt_get_mac_addr();
void tuya_one_click_connect_init(uint8_t is_paired, uint8_t connect_status)
{
#if TUYA_BLE_BR_EDR_SUPPORTED
    printf("%s\n", __func__);
    tuya_ble_br_edr_data_info_t bt_state_info;
    bt_state_info.is_paired = is_paired;
    bt_state_info.dev_ability = 0;
    bt_state_info.connect_status = connect_status ? 2 : 0;
    bt_state_info.name_len = strlen(bt_get_local_name());
    memcpy(bt_state_info.name, bt_get_local_name(), strlen(bt_get_local_name()));
    memcpy(bt_state_info.mac, bt_get_mac_addr(), 6);
    put_buf(&bt_state_info, sizeof(bt_state_info));
    tuya_ble_br_edr_data_info_update(&bt_state_info);
#endif
}

extern int tws_api_get_role(void);
extern void tuya_ble_sdk_reset_mac(tuya_ble_device_param_t *param_data);
void tuya_reset_mac(unsigned char tws_en)
{
#if (TUYA_SDK_DEBUG_MODE)
    memcpy(tuya_ble_protocol_param.device_id,       TY_DEVICE_DID, DEVICE_ID_LEN);
    memcpy(tuya_ble_protocol_param.auth_key,        TY_DEVICE_AUTH_KEY,  AUTH_KEY_LEN);
    memcpy(tuya_ble_protocol_param.mac_addr_string, TY_DEVICE_MAC,  MAC_STRING_LEN);
#else
    tuya_get_lic_from_flash_vm(&tuya_ble_protocol_param);
#endif
    if (tws_en == 0 && tws_api_get_role() == 1) {
        unsigned char local_mac_str[MAC_STRING_LEN];
        unsigned char local_mac[6];
        bt_get_tws_local_addr(local_mac);
        sprintf((char *)local_mac_str, "%02x%02x%02x%02x%02x%02x", \
                (u8)local_mac[0], (u8)local_mac[1], (u8)local_mac[2], \
                (u8)local_mac[3], (u8)local_mac[4], (u8)local_mac[5]);
        memcpy(tuya_ble_protocol_param.mac_addr_string, local_mac_str,  MAC_STRING_LEN);
    }
    tuya_ble_sdk_reset_mac(&tuya_ble_protocol_param);
    tuya_ble_adv_change();
}

VOID_T tuya_ble_protocol_init(VOID_T)
{
    tuya_ble_protocol_param.firmware_version = tal_common_info.firmware_version;
    tuya_ble_protocol_param.hardware_version = tal_common_info.hardware_version;
    printf("TUYA  firmware ver:%s hardware ver:%s\n", FIRMWARE_VERSION, HARDWARE_VERSION);

#if (TUYA_SDK_DEBUG_MODE)
    memcpy(tuya_ble_protocol_param.device_id,       TY_DEVICE_DID, DEVICE_ID_LEN);
    memcpy(tuya_ble_protocol_param.auth_key,        TY_DEVICE_AUTH_KEY,  AUTH_KEY_LEN);
    memcpy(tuya_ble_protocol_param.mac_addr_string, TY_DEVICE_MAC,  MAC_STRING_LEN);
#else
    tuya_get_lic_from_flash_vm(&tuya_ble_protocol_param);
#endif

    memcpy(tuya_ble_protocol_param.product_id,      TY_DEVICE_PID,  tuya_ble_protocol_param.product_id_len);
    tuya_ble_protocol_param.adv_local_name_len = strlen(TY_DEVICE_NAME);
    memcpy(tuya_ble_protocol_param.adv_local_name,  TY_DEVICE_NAME, tuya_ble_protocol_param.adv_local_name_len);
    tuya_ble_sdk_init(&tuya_ble_protocol_param);

    tuya_ble_os_msg_queue_create(&tuya_custom_queue_handle, 16, SIZEOF(tuya_ble_cb_evt_param_t));
    tuya_ble_callback_queue_register(tuya_custom_queue_handle);
    tuya_ble_os_task_create(&tuya_custom_task_handle, "custom", tuya_ble_protocol_callback_task, 0, 512, TUYA_BLE_TASK_PRIORITY);


#if defined(TUYA_BLE_FEATURE_OTA_ENABLE) && (TUYA_BLE_FEATURE_OTA_ENABLE == 1)
    tuya_ble_ota_init();
#endif
#if defined(TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE) && (TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE == 1)
    tuya_ble_attach_ota_init();
#endif
    tuya_ble_disconnect_and_reset_timer_init();
    tuya_ble_update_conn_param_timer_init();

    extern tuya_ble_parameters_settings_t tuya_ble_current_para;
    TAL_PR_HEXDUMP_INFO("auth key", tuya_ble_current_para.auth_settings.auth_key, AUTH_KEY_LEN);
    TAL_PR_HEXDUMP_INFO("device id", tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN);
}

STATIC VOID_T tuya_ble_disconnect_and_reset_timer_cb(tuya_ble_timer_t timer)
{
    tuya_ble_gap_disconnect();
    tuya_ble_device_delay_ms(100);
    tuya_ble_device_reset();
}

STATIC VOID_T tuya_ble_update_conn_param_timer_cb(tuya_ble_timer_t timer)
{
    if (tuya_ble_connect_status_get() == BONDING_CONN) {
        TAL_BLE_PEER_INFO_T peer_info = {0};
        peer_info.conn_handle = tuya_app_get_conn_handle();
        TAL_BLE_CONN_PARAMS_T conn_param = {0};
        conn_param.min_conn_interval = TY_CONN_INTERVAL_MIN * 4 / 5;
        conn_param.max_conn_interval = TY_CONN_INTERVAL_MAX * 4 / 5;
        conn_param.latency = 0;
        conn_param.conn_sup_timeout = 6000 / 10;
        conn_param.connection_timeout = 0;
#if (TUYA_BLE_FEATURE_OTA_ENABLE == 1) && (TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE == 1)
        if (tuya_ble_ota_get_status() == -1 && tuya_ble_attach_ota_get_status() == -1) {
            tal_ble_conn_param_update(peer_info, &conn_param);
        }
#elif (TUYA_BLE_FEATURE_OTA_ENABLE == 1) && (TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE==0)
        if (tuya_ble_ota_get_status() == -1) {
            tal_ble_conn_param_update(peer_info, &conn_param);
        }
#elif (TUYA_BLE_FEATURE_OTA_ENABLE == 0) && (TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE == 1)
        if (tuya_ble_attach_ota_get_status() == -1) {
            tal_ble_conn_param_update(peer_info, &conn_param);
        }
#else
        tal_ble_conn_param_update(peer_info, &conn_param);
#endif
    }
}

VOID_T tuya_ble_disconnect_and_reset_timer_init(VOID_T)
{
    tuya_ble_timer_create(&disconnect_timer, 1000, TUYA_BLE_TIMER_SINGLE_SHOT, tuya_ble_disconnect_and_reset_timer_cb);
}

VOID_T tuya_ble_update_conn_param_timer_init(VOID_T)
{
    tuya_ble_timer_create(&update_conn_param_timer, 1000, TUYA_BLE_TIMER_SINGLE_SHOT, tuya_ble_update_conn_param_timer_cb);
}

VOID_T tuya_ble_disconnect_and_reset_timer_start(VOID_T)
{
    tuya_ble_timer_start(disconnect_timer);
}

VOID_T tuya_ble_update_conn_param_timer_start(VOID_T)
{
    tuya_ble_timer_start(update_conn_param_timer);
}

STATIC VOID_T tuya_ble_app_data_process(INT32_T evt_id, VOID_T *data)
{
    custom_evt_data_t *custom_data = data;

    switch (evt_id) {
    case APP_EVT_0: {
    } break;

    case APP_EVT_1: {
    } break;

    case APP_EVT_2: {
    } break;

    case APP_EVT_3: {
    } break;

    case APP_EVT_4: {
    } break;

    case APP_EVT_AUTH_FLASH_SAVE:
        break;
    case APP_EVT_SYS_FLASH_SAVE:
        tuya_tws_bind_info_sync();
        break;
    case APP_EVT_TWS_SYNC_RECV:
        tuya_ble_storage_save_sys_settings();
        tuya_ble_adv_change();
        break;

    default: {
    } break;

    }

    if (custom_data != NULL) {
        tal_free((VOID_T *)custom_data);
    }
}

VOID_T tuya_ble_custom_evt_send(custom_evtid_t evtid)
{
    tuya_ble_custom_evt_t custom_evt;

    custom_evt.evt_id = evtid;
    custom_evt.data = NULL;
    custom_evt.custom_event_handler = tuya_ble_app_data_process;

    tuya_ble_custom_event_send(custom_evt);
}

VOID_T tuya_ble_custom_evt_send_with_data(custom_evtid_t evtid, VOID_T *buf, UINT32_T size)
{
    custom_evt_data_t *custom_data = tal_malloc(SIZEOF(custom_evt_data_t) + size);
    if (custom_data) {
        tuya_ble_custom_evt_t custom_evt;

        custom_data->len = size;
        memcpy(custom_data->value, buf, size);

        custom_evt.evt_id = evtid;
        custom_evt.data = custom_data;
        custom_evt.custom_event_handler = tuya_ble_app_data_process;

        tuya_ble_custom_event_send(custom_evt);
    } else {
        TAL_PR_ERR("tuya_ble_custom_evt_send_with_data: malloc failed");
    }
}

