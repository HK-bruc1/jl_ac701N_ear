/**
 * @file tuya_ble_main.c
 * @brief This is tuya_ble_main file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "string.h"

#include "tuya_ble_type.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_api.h"
#include "tuya_ble_port.h"
#include "tuya_ble_main.h"
#include "tuya_ble_secure.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_sdk_version.h"
#include "tuya_ble_event.h"
#if defined(TUYA_BLE_FEATURE_UART_COMMON_ENABLE) && (TUYA_BLE_FEATURE_UART_COMMON_ENABLE == 1)
#include "tuya_ble_uart_common.h"
#endif
#if defined(TUYA_BLE_FEATURE_PRODUCT_TEST_ENABLE) && (TUYA_BLE_FEATURE_PRODUCT_TEST_ENABLE == 1)
#include "tuya_ble_product_test.h"
#endif
#include "tuya_ble_log.h"
#include "tuya_ble_internal_config.h"
#include "tal_util.h"

extern UINT8_T tuya_ble_encrypt_old_with_key(tuya_ble_parameters_settings_t *current_para, UINT8_T *adv_buf, UINT8_T version);

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define TUYA_BLE_ADV_DATA_LEN_MAX       31
#define TUYA_BLE_SCAN_RSP_DATA_LEN_MAX  31

#define tuya_ble_connect_monitor_timeout_ms 30000

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
tal_common_info_t tal_common_info = {0};
tuya_ble_parameters_settings_t tuya_ble_current_para;

STATIC volatile tuya_ble_connect_status_t tuya_ble_connect_status = UNKNOW_STATUS;

static volatile tuya_ble_connect_source_type_t tuya_ble_connect_source_type = UNKNOW_TYPES;

#if TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE
STATIC volatile tuya_ble_link_status_t tuya_ble_link_status = TY_LINK_UNKNOW_STATUS;
#endif

#if TUYA_BLE_USE_OS

UINT32_T m_cb_queue_table[TUYA_BLE_MAX_CALLBACKS];

#else
tuya_ble_callback_t m_cb_table[TUYA_BLE_MAX_CALLBACKS];
#endif

tuya_ble_timer_t tuya_ble_xtimer_connect_monitor;

#if TUYA_BLE_USE_OS

#if TUYA_BLE_SELF_BUILT_TASK
VOID_T *tuya_ble_task_handle;   //!< APP Task handle
VOID_T *tuya_ble_queue_handle;  //!< Event queue handle
#endif

#endif

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN==5)

STATIC CONST UINT8_T adv_data_const[TUYA_BLE_ADV_DATA_LEN_MAX] = {
    0x02, 0x01, 0x06,
    0x17, 0x16, 0x50, 0xFD,
    0x51, 0x00, 0x00, //Frame Control
};

STATIC CONST UINT8_T scan_rsp_data_const[TUYA_BLE_SCAN_RSP_DATA_LEN_MAX] = {
    0x17, 0xFF,
    0xD0, 0x07,
    0x00, //Encry Mode(8)
    0x00, 0x00, //communication way bit0-mesh bit1-wifi bit2-zigbee bit3-NB
    0x00, //FLAG
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x03, 0x09, 0x54, 0x59, //Name: "TY"
};

STATIC UINT8_T adv_data[TUYA_BLE_ADV_DATA_LEN_MAX];
STATIC UINT8_T scan_rsp_data[TUYA_BLE_SCAN_RSP_DATA_LEN_MAX];

#endif

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN==4)

STATIC CONST UINT8_T adv_data_const[TUYA_BLE_ADV_DATA_LEN_MAX] = {
    0x02, 0x01, 0x06,
    0x03, 0x02, 0x50, 0xFD,
    0x17, 0x16, 0x50, 0xFD, 0x41, 0x00,       //Frame Control
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

STATIC CONST UINT8_T scan_rsp_data_const[TUYA_BLE_SCAN_RSP_DATA_LEN_MAX] = {
    0x17, 0xFF,
    0xD0, 0x07,
    0x00, //Encry Mode(8)
    0x00, 0x00, //communication way bit0-mesh bit1-wifi bit2-zigbee bit3-NB
    0x00, //FLAG
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x03, 0x09, 0x54, 0x59, //Name: "TY"
};

STATIC UINT8_T adv_data[TUYA_BLE_ADV_DATA_LEN_MAX];
STATIC UINT8_T scan_rsp_data[TUYA_BLE_SCAN_RSP_DATA_LEN_MAX];

#endif

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN==3)

/** @brief  GAP - Advertisement data (max size = 31 bytes, best kept short to conserve power) */

#define TUYA_BLE_ADV_DATA_LEN  (12+TUYA_BLE_PRODUCT_ID_MAX_LEN)

STATIC CONST UINT8_T adv_data_const[TUYA_BLE_ADV_DATA_LEN] = {
    0x02,
    0x01,
    0x06,
    0x03,
    0x02,
    0x01, 0xA2,
    0x14,
    0x16,
    0x01, 0xA2,
    0x00,         //id type 00-pid 01-product key
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define TUYA_BLE_SCAN_RSP_DATA_LEN  30
STATIC CONST UINT8_T scan_rsp_data_const[TUYA_BLE_SCAN_RSP_DATA_LEN] = {
    0x03,
    0x09,
    0x54, 0x59,
    0x19,             // length
    0xFF,
    0xD0,
    0x07,
    0x00, //bond flag bit7
    0x03, //protocol version
    0x01, //Encry Mode
    0x00, 0x00, //communication way bit0-mesh bit1-wifi bit2-zigbee bit3-NB
    0x00, //data type
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

STATIC UINT8_T adv_data[TUYA_BLE_ADV_DATA_LEN];
STATIC UINT8_T scan_rsp_data[TUYA_BLE_SCAN_RSP_DATA_LEN];

#endif

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN==2)
#warning "Latest Tuya Ble SDK NOT SUPPORT Version 2"
#endif

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




VOID_T tuya_ble_connect_status_set(tuya_ble_connect_status_t status)
{
    tuya_ble_device_enter_critical();
    tuya_ble_connect_status = status;
    tuya_ble_device_exit_critical();

}

tuya_ble_connect_status_t tuya_ble_connect_status_get(VOID_T)
{
    return  tuya_ble_connect_status;
}

void tuya_ble_connect_source_type_set(tuya_ble_connect_source_type_t source_type)
{
    tuya_ble_device_enter_critical();
    tuya_ble_connect_source_type = source_type;
    tuya_ble_device_exit_critical();
}

tuya_ble_connect_source_type_t tuya_ble_connect_source_type_get(void)
{
    return  tuya_ble_connect_source_type;
}

#if TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE

VOID_T tuya_ble_link_status_set(tuya_ble_link_status_t status)
{
    tuya_ble_device_enter_critical();
    tuya_ble_link_status = status;
    tuya_ble_device_exit_critical();
}

tuya_ble_link_status_t tuya_ble_link_status_get(VOID_T)
{
    return  tuya_ble_link_status;
}

#endif

STATIC VOID_T tuya_ble_vtimer_conncet_monitor_callback(tuya_ble_timer_t timer)
{
    tuya_ble_connect_status_t connect_state = tuya_ble_connect_status_get();

    if ((connect_state == UNBONDING_UNAUTH_CONN) || (connect_state == BONDING_UNAUTH_CONN)) {
        TUYA_BLE_LOG_DEBUG("ble disconncet because monitor timer timeout.");
        tuya_ble_gap_disconnect();
    }

}

VOID_T tuya_ble_connect_monitor_timer_init(VOID_T)
{
    if (tuya_ble_timer_create(&tuya_ble_xtimer_connect_monitor, tuya_ble_connect_monitor_timeout_ms, TUYA_BLE_TIMER_SINGLE_SHOT, tuya_ble_vtimer_conncet_monitor_callback) != TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_ERROR("tuya_ble_xtimer_connect_monitor creat failed");
    }

}

VOID_T tuya_ble_connect_monitor_timer_start(VOID_T)
{
    if (tuya_ble_timer_start(tuya_ble_xtimer_connect_monitor) != TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_ERROR("tuya_ble_xtimer_connect_monitor start failed");
    }

}

VOID_T tuya_ble_connect_monitor_timer_stop(VOID_T)
{

    if (tuya_ble_timer_stop(tuya_ble_xtimer_connect_monitor) != TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_ERROR("tuya_ble_xtimer_connect_monitor start failed");
    }

}

#if TUYA_BLE_USE_OS

#if TUYA_BLE_SELF_BUILT_TASK

TUYA_WEAK_ATTRIBUTE VOID_T tuya_ble_main_task(VOID_T *p_param)
{
    tuya_ble_evt_param_t tuya_ble_evt = {0};
    printf("tuya_ble_main_task\n");
    while (TRUE) {
        printf("tuya_ble_os_msg_queue_recv\n");
        if (tuya_ble_os_msg_queue_recv(tuya_ble_queue_handle, &tuya_ble_evt, 0xFFFFFFFF) == TRUE) {
            printf("ble_main event %d\n", tuya_ble_evt.hdr.event);
            tuya_ble_event_process(&tuya_ble_evt);
        }
    }
}

#endif

#else

VOID_T tuya_ble_main_tasks_exec(VOID_T)
{
    tuya_sched_execute();
}

#endif

VOID_T tuya_ble_event_init(VOID_T)
{
#if TUYA_BLE_USE_OS

#if TUYA_BLE_SELF_BUILT_TASK
    tuya_ble_os_msg_queue_create(&tuya_ble_queue_handle, MAX_NUMBER_OF_TUYA_MESSAGE, SIZEOF(tuya_ble_evt_param_t));
    tuya_ble_os_task_create(&tuya_ble_task_handle, "tuya", tuya_ble_main_task, 0, TUYA_BLE_TASK_STACK_SIZE, TUYA_BLE_TASK_PRIORITY);
#endif

#else
    tuya_ble_event_queue_init();
#endif

#if TUYA_BLE_USE_OS
    for (UINT8_T i = 0; i < TUYA_BLE_MAX_CALLBACKS; i++) {
        m_cb_queue_table[i] = 0;
    }

#else
    for (UINT8_T i = 0; i < TUYA_BLE_MAX_CALLBACKS; i++) {
        m_cb_table[i] = NULL;
    }
#endif

}

UINT8_T tuya_ble_event_send(tuya_ble_evt_param_t *evt)
{
#if TUYA_BLE_USE_OS
#if TUYA_BLE_SELF_BUILT_TASK
    if (tuya_ble_os_msg_queue_send(tuya_ble_queue_handle, evt, 0)) {
        return 0;
    } else {
        return 1;
    }
#else
    if (tuya_ble_event_queue_send_port(evt, 0)) {
        return 0;
    } else {
        return 1;
    }
#endif

#else
    if (tuya_ble_message_send(evt) == TUYA_BLE_SUCCESS) {
        UINT16_T queue_size = tuya_ble_scheduler_queue_size_get();
        UINT16_T queue_space = tuya_ble_scheduler_queue_space_get();

#if defined(TUYA_SDK_TEST) && (TUYA_SDK_TEST == 1)
        UINT8_T rsp[4];
        (void)rsp;
        rsp[0] = queue_size >> 8;
        rsp[1] = queue_size & 0xFF;
        rsp[2] = queue_space >> 8;
        rsp[3] = queue_space & 0xFF;

//        test_cmd_send(TEST_ID_GET(TEST_GID_SYSTEM, TEST_CID_GET_QUEUE_SIZE), rsp, 4);
#endif

        return 0;
    } else {
        return 1;
    }
#endif
}

UINT8_T tuya_ble_custom_event_send(tuya_ble_custom_evt_t evt)
{
    tuya_ble_evt_param_t event;

#if TUYA_BLE_USE_OS
    event.hdr.event = TUYA_BLE_EVT_CUSTOM;
    event.custom_evt = evt;

#if TUYA_BLE_SELF_BUILT_TASK
    if (tuya_ble_os_msg_queue_send(tuya_ble_queue_handle, &event, 0)) {
        return 0;
    } else {
        return 1;
    }
#else
    if (tuya_ble_event_queue_send_port(&event, 0)) {
        return 0;
    } else {
        return 1;
    }
#endif

#else

    UINT8_T ret = 0;
    event.hdr.event = TUYA_BLE_EVT_CUSTOM;
    event.custom_evt = evt;

    if (tuya_ble_message_send(&event) == TUYA_BLE_SUCCESS) {
        ret = 0;
    } else {
        ret = 1;
    }

    return ret;
#endif
}

tuya_ble_status_t tuya_ble_inter_event_response(tuya_ble_cb_evt_param_t *param)
{

    switch (param->evt) {
    case TUYA_BLE_CB_EVT_CONNECT_STATUS:
        break;
    case TUYA_BLE_CB_EVT_DP_WRITE:
        if (param->dp_write_data.p_data) {
            tuya_ble_free(param->dp_write_data.p_data);
        }
        break;
    case TUYA_BLE_CB_EVT_DP_DATA_RECEIVED:
        if (param->dp_received_data.p_data) {
            tuya_ble_free(param->dp_received_data.p_data);
        }
        break;
    case TUYA_BLE_CB_EVT_DP_QUERY:
        if (param->dp_query_data.p_data) {
            tuya_ble_free(param->dp_query_data.p_data);
        }
        break;
    case TUYA_BLE_CB_EVT_OTA_DATA:
        if (param->ota_data.p_data) {
            tuya_ble_free(param->ota_data.p_data);
        }
        break;
    case TUYA_BLE_CB_EVT_FILE_DATA:
        if (param->file_data.p_data) {
            tuya_ble_free(param->file_data.p_data);
        }
        break;
    case TUYA_BLE_CB_EVT_NETWORK_INFO:
        if (param->network_data.p_data) {
            tuya_ble_free(param->network_data.p_data);
        }
        break;
    case TUYA_BLE_CB_EVT_WIFI_SSID:
        if (param->wifi_info_data.p_data) {
            tuya_ble_free(param->wifi_info_data.p_data);
        }
        break;
    case TUYA_BLE_CB_EVT_TIME_STAMP:
        break;
    case TUYA_BLE_CB_EVT_TIME_NORMAL:
        break;
    case TUYA_BLE_CB_EVT_APP_LOCAL_TIME_NORMAL:
        break;
    case TUYA_BLE_CB_EVT_TIME_STAMP_WITH_DST:
        if (param->timestamp_with_dst_data.p_data) {
            tuya_ble_free(param->timestamp_with_dst_data.p_data);
        }
        break;
    case TUYA_BLE_CB_EVT_DATA_PASSTHROUGH:

        if (param->ble_passthrough_data.p_data) {
            tuya_ble_free(param->ble_passthrough_data.p_data);
        }
        break;
#if defined(TUYA_BLE_FEATURE_SPEECH_ENABLE) && (TUYA_BLE_FEATURE_SPEECH_ENABLE == 1)
    case TUYA_BLE_CB_EVT_SPEECH_CONTROL:
    case TUYA_BLE_CB_EVT_SPEECH_RAW_DATA_REPORT_RSP:
    case TUYA_BLE_CB_EVT_SPEECH_RESULT_WRITE:
    case TUYA_BLE_CB_EVT_SPEECH_CLOCK_SETTING:
    case TUYA_BLE_CB_EVT_SPEECH_TOKEN_READ:
    case TUYA_BLE_CB_EVT_SPEECH_TOKEN_REPORT_RSP:
    case TUYA_BLE_CB_EVT_SPEECH_TOKEN_WRITE:
    case TUYA_BLE_CB_EVT_SPEECH_COMMON_SETTING:
        if (param->ble_speech_data.p_data) {
            tuya_ble_free(param->ble_speech_data.p_data);
        }
        break;
#endif

#if defined(TUYA_BLE_FEATURE_GPT_ENABLE) && (TUYA_BLE_FEATURE_GPT_ENABLE == 1)
    case TUYA_BLE_CB_EVT_GPT_CONTROL:
    case TUYA_BLE_CB_EVT_GPT_RAW_DATA_REPORT_RSP:
    case TUYA_BLE_CB_EVT_GPT_RESULT_WRITE:
    case TUYA_BLE_CB_EVT_GPT_RAW_DATA_WRITE:
        if (param->ble_speech_data.p_data) {
            tuya_ble_free(param->ble_speech_data.p_data);
        }
        break;
#endif
    case TUYA_BLE_CB_EVT_REMOTER_PROXY_AUTH_RESP:
        if (param->remoter_proxy_auth_data_rsp.p_data) {
            tuya_ble_free(param->remoter_proxy_auth_data_rsp.p_data);
        }
        break;

#if (TUYA_BLE_FEATURE_WEATHER_ENABLE != 0)
    case TUYA_BLE_CB_EVT_WEATHER_DATA_RECEIVED:
        if (param->weather_received_data.p_data) {
            tuya_ble_free(param->weather_received_data.p_data);
        }
        break;
#endif

#if (TUYA_BLE_FEATURE_IOT_CHANNEL_ENABLE != 0)
#if (TUYA_BLE_FEATURE_SCENE_ENABLE != 0)
    case TUYA_BLE_CB_EVT_SCENE_DATA_RECEIVED:
        if (param->scene_data_received_data.p_data) {
            tuya_ble_free(param->scene_data_received_data.p_data);
        }
        break;
    case TUYA_BLE_CB_EVT_SCENE_CTRL_RESULT_RECEIVED:
        if (param->scene_ctrl_received_data.p_scene_id) {
            tuya_ble_free(param->scene_ctrl_received_data.p_scene_id);
        }
        break;
#endif // (TUYA_BLE_FEATURE_SCENE_ENABLE != 0)
#endif

#if TUYA_BLE_FEATURE_APP_PASSTHROUGH_ENABLE
    case TUYA_BLE_CB_EVT_APP_PASSTHROUGH_DATA: {
        tuya_ble_free((uint8_t *)param->app_passthrough_data.p_data);
    }
    break;
#endif

#if (TUYA_BLE_FEATURE_EXT_MODULE_ENABLE)
    case TUYA_BLE_CB_EVT_QUERY_EXT_MODULE_DEV_INFO: {
    } break;
    case TUYA_BLE_CB_EVT_EXT_MODULE_ACTIVE_INFO_RECEIVED: {
        if (param->ext_module_active_data.p_data) {
            tuya_ble_free(param->ext_module_active_data.p_data);
        }
    }
    break;
#endif
#if (TUYA_BLE_MUTI_DATA_SOURCE_SUPPORTED)
    case TUYA_BLE_CB_EVT_WITH_SRC_TYPE_DP_QUERY:
        tuya_ble_free((uint8_t *)param->dp_query_data_with_src_type.p_add_info);
        tuya_ble_free((uint8_t *)param->dp_query_data_with_src_type.p_data);
        break;
    case TUYA_BLE_CB_EVT_WITH_SRC_TYPE_DP_DATA_RECEIVED:
        tuya_ble_free((uint8_t *)param->dp_data_with_src_type_received_data.p_add_info);
        tuya_ble_free((uint8_t *)param->dp_data_with_src_type_received_data.p_data);
        break;
    case TUYA_BLE_CB_EVT_DP_DATA_WITH_SRC_TYPE_SEND_RESPONSE:
        tuya_ble_free((uint8_t *)param->dp_with_src_type_send_response_data.p_add_info);
        break;
    case TUYA_BLE_CB_EVT_DP_DATA_WITH_SRC_TYPE_AND_TIME_SEND_RESPONSE:
        tuya_ble_free((uint8_t *)param->dp_with_src_type_and_time_send_response_data.p_add_info);
        break;
#endif
#if (TUYA_BLE_ACCESSORY_MOUNT_SUPPORTED)
    case TUYA_BLE_CB_EVT_ACCESSORY_INFO_READING:
        tuya_ble_free((uint8_t *)param->accessory_info_reading_data.attach_data);
        break;
    case TUYA_BLE_CB_EVT_ACCESSORY_ACTIVE_INFO_RECEIVED:
        tuya_ble_free((uint8_t *)param->accessory_active_info_data.active_info_data);
        break;
    case TUYA_BLE_CB_EVT_ACCESSORY_OTA_DATA:
        if (param->accessory_ota_data.p_data) {
            tuya_ble_free(param->accessory_ota_data.p_data);
        }
        break;
#endif
    case TUYA_BLE_CB_EVT_GROUP_DP_DATA_RECEIVED:
        if (param->group_dp_data_received.p_data) {
            tuya_ble_free(param->group_dp_data_received.p_data);
        }
        break;
    default:
        break;
    }

    return TUYA_BLE_SUCCESS;
}

UINT8_T tuya_ble_cb_event_send(tuya_ble_cb_evt_param_t *evt)
{
#if TUYA_BLE_USE_OS
    if (m_cb_queue_table[0]) {
        if (tuya_ble_os_msg_queue_send((VOID_T *)m_cb_queue_table[0], evt, 0)) {
            return 0;
        } else {
            return 1;
        }
    }
#else
    tuya_ble_callback_t fun;
    if (m_cb_table[0]) {
        fun = m_cb_table[0];
        fun(evt);
        tuya_ble_inter_event_response(evt);
    }
#endif
    return 0;

}

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN==5)

UINT8_T tuya_ble_get_adv_connect_request_bit_status(VOID_T)
{
    return ((adv_data[7] & 0x02) >> 1);
}

__TUYA_BLE_WEAK UINT8_T tuya_ble_secure_connection_type(VOID_T)
{
    return TUYA_BLE_SECURE_CONNECTION_TYPE;
}

__TUYA_BLE_WEAK UINT8_T tuya_ble_connectivity(VOID_T)
{
    return TUYA_BLE_CONNECTIVITY;
}

__TUYA_BLE_WEAK UINT8_T tuya_ble_gateway_conn_mode(VOID_T)
{
    return TUYA_BLE_GATEWAY_CONN_MODE;
}

__TUYA_BLE_WEAK UINT8_T tuya_ble_gateway_online_mode(VOID_T)
{
    /* 0 - Using standard power device online strategy, 1 - using low power device online strategy, currently only applicable to Bluetooth gateway, mobile APP does not need to care about this bit. */
    return 0;
}

STATIC VOID_T tuya_ble_adv_change_setting(UINT8_T enable_connect_req)
{
#if (TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE || TUYA_BLE_FEATURE_LONG_RANGE)
    UINT8_T *p_buf = NULL;
#endif
    UINT8_T adv_data_length = 0;
    UINT8_T scan_rsp_data_length = 0;

    memcpy(adv_data, adv_data_const, TUYA_BLE_ADV_DATA_LEN_MAX);
    memcpy(&scan_rsp_data, scan_rsp_data_const, TUYA_BLE_SCAN_RSP_DATA_LEN_MAX);

    adv_data[3] = 7 + tuya_ble_current_para.pid_len;

    adv_data[10] = (tuya_ble_current_para.pid_type << 5) | (tuya_ble_current_para.pid_len & 0x1F);

    if (TUYA_BLE_DEVICE_SHARED) {
        adv_data[7] |= 0x04 ;
    } else {
        adv_data[7] &= (~0x04);
    }

    if (enable_connect_req) {
        adv_data[7] |= 0x02;  //set connect request bit
    } else {
        adv_data[7] &= (~0x02);  //clear connect request bit
    }

    if (tuya_ble_connectivity() == 0) {
        adv_data[8] &= (~0x40);
        adv_data[8] &= (~0x20);
    } else if (tuya_ble_connectivity() == 1) {
        adv_data[8] &= (~0x40);
        adv_data[8] |= 0x20;
    } else if (tuya_ble_connectivity() == 2) {
        adv_data[8] |= 0x40;
        adv_data[8] &= (~0x20);
    }

    if (tuya_ble_gateway_conn_mode()) {
        adv_data[8] |= 0x10;
    }

    adv_data[8] |= 0x0C; // v2

    scan_rsp_data[4] = tuya_ble_secure_connection_type();

    scan_rsp_data[5] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY >> 8;
    scan_rsp_data[6] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY & 0xFF;

    if (tuya_ble_current_para.pid_len == 20) {
        scan_rsp_data[7] |= 0x01 ;
    } else {
        scan_rsp_data[7] &= (~0x01);
    }

#if (TUYA_BLE_SECURE_CONNECTION_TYPE == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)
    if (TUYA_BLE_ADVANCED_ENCRYPTION_AUTH_ON_CONNECT) {
        scan_rsp_data[7] |= 0x02 ;
    } else {
        scan_rsp_data[7] &= (~0x02);
    }
#endif

    if (tuya_ble_gateway_online_mode()) {
        scan_rsp_data[7] |= 0x04;
    } else {
        scan_rsp_data[7] &= (~0x04);
    }

#if (TAL_BLE_BEACON_ROAMING_FLAG)
    adv_data[9] |= 0x40;
#endif

    if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
        adv_data[7] |= 0x08 ;

        tuya_ble_encrypt_old_with_key(&tuya_ble_current_para, adv_data, TUYA_BLE_PROTOCOL_VERSION_HIGN);
        tuya_ble_device_id_encrypt_v4(&tuya_ble_current_para, adv_data, scan_rsp_data, TUYA_BLE_PROTOCOL_VERSION_HIGN);
    } else {
        adv_data[7] &= (~0x08);

        memcpy(&adv_data[11], tuya_ble_current_para.pid, tuya_ble_current_para.pid_len);
        tuya_ble_device_id_encrypt_v4(&tuya_ble_current_para, adv_data, scan_rsp_data, TUYA_BLE_PROTOCOL_VERSION_HIGN);
    }

    if (tuya_ble_current_para.adv_local_name_len > TUYA_BLE_ADV_LOCAL_NAME_MAX_LEN) {
        tuya_ble_current_para.adv_local_name_len = TUYA_BLE_ADV_LOCAL_NAME_MAX_LEN;
    }

    if ((tuya_ble_secure_connection_type() == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_V2) || (tuya_ble_secure_connection_type() == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION_V2) || (tuya_ble_secure_connection_type() == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_FOR_QR_CODE_V2)) {
        UINT8_T mac_temp[MAC_LEN] = {0};
        scan_rsp_data[0] = 0x0E;
        scan_rsp_data[8] = 0;
        scan_rsp_data[8] &= (~0x03);   // flag2 bit1-bit0:00
        memcpy(mac_temp, tuya_ble_current_para.auth_settings.mac, MAC_LEN);
        tal_util_reverse_byte(mac_temp, MAC_LEN);
        memcpy(&scan_rsp_data[9], mac_temp, MAC_LEN);

        if (tuya_ble_current_para.adv_local_name_len > 0) {
            scan_rsp_data[15] = tuya_ble_current_para.adv_local_name_len + 1;
            scan_rsp_data[16] = 0x09;
            memset(&scan_rsp_data[17], 0x00, TUYA_BLE_ADV_LOCAL_NAME_MAX_LEN);
            memcpy(&scan_rsp_data[17], tuya_ble_current_para.adv_local_name, tuya_ble_current_para.adv_local_name_len);
        }
        adv_data_length = tuya_ble_current_para.pid_len + 15;
        scan_rsp_data_length = (tuya_ble_current_para.adv_local_name_len == 0 ? (-2) : tuya_ble_current_para.adv_local_name_len) + 17;
    } else {
        if (tuya_ble_current_para.adv_local_name_len > 0) {
            scan_rsp_data[24] = tuya_ble_current_para.adv_local_name_len + 1;
            scan_rsp_data[25] = 0x09;
            memcpy(&scan_rsp_data[26], tuya_ble_current_para.adv_local_name, tuya_ble_current_para.adv_local_name_len);
        }
        adv_data_length = tuya_ble_current_para.pid_len + 11;
        scan_rsp_data_length = (tuya_ble_current_para.adv_local_name_len == 0 ? (-2) : tuya_ble_current_para.adv_local_name_len) + 26;
    }

    TUYA_BLE_LOG_INFO("adv data changed ,current bound flag = %d", tuya_ble_current_para.sys_settings.bound_flag);
    tuya_ble_gap_advertising_adv_data_update(adv_data, adv_data_length);
    tuya_ble_gap_advertising_scan_rsp_data_update(scan_rsp_data, scan_rsp_data_length);

#if TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE

    p_buf = tuya_ble_malloc(adv_data_length + scan_rsp_data_length + 3);
    if (p_buf == NULL) {
        TUYA_BLE_LOG_ERROR("Malloc failed for read buf in adv change.");
        return;
    } else {
        memset(p_buf, 0, adv_data_length + scan_rsp_data_length + 3);
        p_buf[0] = 0;
        p_buf[1] = adv_data_length + scan_rsp_data_length;
        memcpy(&p_buf[2], adv_data, adv_data_length);
        memcpy(&p_buf[2 + adv_data_length], scan_rsp_data, scan_rsp_data_length);
    }

    tuya_ble_device_info_characteristic_value_update(p_buf, (adv_data_length + scan_rsp_data_length + 3));

    tuya_ble_free(p_buf);
#endif

#if TUYA_BLE_FEATURE_LONG_RANGE
    p_buf = tuya_ble_malloc(adv_data_length + scan_rsp_data_length);
    if (p_buf == NULL) {
        TUYA_BLE_LOG_ERROR("Malloc failed for read buf in ext adv change.");
        return;
    } else {
        memset(p_buf, 0, adv_data_length + scan_rsp_data_length);
        memcpy(p_buf, adv_data, adv_data_length);
        memcpy(&p_buf[adv_data_length], scan_rsp_data, scan_rsp_data_length);
    }

    tuya_ble_long_range_ext_adv_data_update(p_buf, (adv_data_length + scan_rsp_data_length));

    tuya_ble_free(p_buf);
#endif
}

VOID_T tuya_ble_adv_change(VOID_T)
{
    tuya_ble_adv_change_setting(0);
}

VOID_T tuya_ble_adv_change_with_connecting_request(VOID_T)
{
    tuya_ble_adv_change_setting(1);
}

#endif

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN==4)

UINT8_T tuya_ble_get_adv_connect_request_bit_status(VOID_T)
{
    return ((adv_data[11] & 0x02) >> 1);
}

__TUYA_BLE_WEAK UINT8_T tuya_ble_secure_connection_type(VOID_T)
{
    return TUYA_BLE_SECURE_CONNECTION_TYPE;
}

__TUYA_BLE_WEAK UINT8_T tuya_ble_connectivity(VOID_T)
{
    return TUYA_BLE_CONNECTIVITY;
}

__TUYA_BLE_WEAK UINT8_T tuya_ble_gateway_conn_mode(VOID_T)
{
    return TUYA_BLE_GATEWAY_CONN_MODE;
}

__TUYA_BLE_WEAK UINT8_T tuya_ble_gateway_online_mode(VOID_T)
{
    /* 0 - Using standard power device online strategy, 1 - using low power device online strategy, currently only applicable to Bluetooth gateway, mobile APP does not need to care about this bit. */
    return 0;
}

STATIC VOID_T tuya_ble_adv_change_setting(UINT8_T enable_connect_req)
{
#if (TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE || TUYA_BLE_FEATURE_LONG_RANGE)
    UINT8_T *p_buf = NULL;
#endif
    UINT8_T adv_data_length = 0;
    UINT8_T scan_rsp_data_length = 0;

    memcpy(adv_data, adv_data_const, TUYA_BLE_ADV_DATA_LEN_MAX);
    memcpy(&scan_rsp_data, scan_rsp_data_const, TUYA_BLE_SCAN_RSP_DATA_LEN_MAX);

    adv_data[7] = 7 + tuya_ble_current_para.pid_len;

    adv_data[13] = tuya_ble_current_para.pid_type;
    adv_data[14] = tuya_ble_current_para.pid_len;

    if (TUYA_BLE_DEVICE_SHARED) {
        adv_data[11] |= 0x04 ;
    } else {
        adv_data[11] &= (~0x04);
    }

    if (enable_connect_req) {
        adv_data[11] |= 0x02;  //set connect request bit
    } else {
        adv_data[11] &= (~0x02);  //clear connect request bit
    }

#if (TUYA_BLE_PROTOCOL_VERSION_LOW >= 6)

    if (tuya_ble_connectivity() == 0) {
        adv_data[12] &= (~0x40);
        adv_data[12] &= (~0x20);
    } else if (tuya_ble_connectivity() == 1) {
        adv_data[12] &= (~0x40);
        adv_data[12] |= 0x20;
    } else if (tuya_ble_connectivity() == 2) {
        adv_data[12] |= 0x40;
        adv_data[12] &= (~0x20);
    }

    if (tuya_ble_gateway_conn_mode()) {
        adv_data[12] |= 0x10;
    }

#endif

#if (TUYA_BLE_PROTOCOL_VERSION_LOW >= 7)
    adv_data[12] |= 0x04;
    if (tuya_ble_current_para.sys_settings.protocol_v2_enable) {
        adv_data[12] |= 0x08;
    } else {
        adv_data[12] &= (~0x08);
    }
#endif

    scan_rsp_data[4] = tuya_ble_secure_connection_type();

    scan_rsp_data[5] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY >> 8;
    scan_rsp_data[6] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY & 0xFF;

    if (tuya_ble_current_para.pid_len == 20) {
        scan_rsp_data[7] |= 0x01 ;
    } else {
        scan_rsp_data[7] &= (~0x01);
    }

#if (TUYA_BLE_SECURE_CONNECTION_TYPE == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)
    if (TUYA_BLE_ADVANCED_ENCRYPTION_AUTH_ON_CONNECT) {
        scan_rsp_data[7] |= 0x02;
    } else {
        scan_rsp_data[7] &= (~0x02);
    }
#endif

    if (tuya_ble_gateway_online_mode()) {
        scan_rsp_data[7] |= 0x04;
    } else {
        scan_rsp_data[7] &= (~0x04);
    }

    if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
        adv_data[11] |= 0x08 ;

        tuya_ble_encrypt_old_with_key(&tuya_ble_current_para, adv_data, TUYA_BLE_PROTOCOL_VERSION_HIGN);

        tuya_ble_device_id_encrypt_v4(&tuya_ble_current_para, adv_data, scan_rsp_data, TUYA_BLE_PROTOCOL_VERSION_HIGN);

    } else {
        adv_data[11] &= (~0x08);

        memcpy(&adv_data[15], tuya_ble_current_para.pid, tuya_ble_current_para.pid_len);

        tuya_ble_device_id_encrypt_v4(&tuya_ble_current_para, adv_data, scan_rsp_data, TUYA_BLE_PROTOCOL_VERSION_HIGN);
    }

    if (tuya_ble_current_para.adv_local_name_len > TUYA_BLE_ADV_LOCAL_NAME_MAX_LEN) {
        tuya_ble_current_para.adv_local_name_len = TUYA_BLE_ADV_LOCAL_NAME_MAX_LEN;
    }

    if ((tuya_ble_secure_connection_type() == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_V2) || (tuya_ble_secure_connection_type() == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION_V2) || (tuya_ble_secure_connection_type() == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_FOR_QR_CODE_V2)) {
        UINT8_T mac_temp[MAC_LEN] = {0};
        scan_rsp_data[0] = 0x0E;
        scan_rsp_data[8] = 0;
        scan_rsp_data[8] &= (~0x03);   // flag2 bit1-bit0:00
        memcpy(mac_temp, tuya_ble_current_para.auth_settings.mac, MAC_LEN);
        tal_util_reverse_byte(mac_temp, MAC_LEN);
        memcpy(&scan_rsp_data[9], mac_temp, MAC_LEN);

        if (tuya_ble_current_para.adv_local_name_len > 0) {
            scan_rsp_data[15] = tuya_ble_current_para.adv_local_name_len + 1;
            scan_rsp_data[16] = 0x09;
            memset(&scan_rsp_data[17], 0x00, TUYA_BLE_ADV_LOCAL_NAME_MAX_LEN);
            memcpy(&scan_rsp_data[17], tuya_ble_current_para.adv_local_name, tuya_ble_current_para.adv_local_name_len);
        }
        adv_data_length = tuya_ble_current_para.pid_len + 15;
        scan_rsp_data_length = (tuya_ble_current_para.adv_local_name_len == 0 ? (-2) : tuya_ble_current_para.adv_local_name_len) + 17;
    } else {
        if (tuya_ble_current_para.adv_local_name_len > 0) {
            scan_rsp_data[24] = tuya_ble_current_para.adv_local_name_len + 1;
            scan_rsp_data[25] = 0x09;
            memcpy(&scan_rsp_data[26], tuya_ble_current_para.adv_local_name, tuya_ble_current_para.adv_local_name_len);
        }
        adv_data_length = tuya_ble_current_para.pid_len + 15;
        scan_rsp_data_length = (tuya_ble_current_para.adv_local_name_len == 0 ? (-2) : tuya_ble_current_para.adv_local_name_len) + 26;
    }

    TUYA_BLE_LOG_INFO("adv data changed ,current bound flag = %d", tuya_ble_current_para.sys_settings.bound_flag);
    tuya_ble_gap_advertising_adv_data_update(adv_data, adv_data_length);
    tuya_ble_gap_advertising_scan_rsp_data_update(scan_rsp_data, scan_rsp_data_length);

#if TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE

    p_buf = tuya_ble_malloc(adv_data_length + scan_rsp_data_length + 3);
    if (p_buf == NULL) {
        TUYA_BLE_LOG_ERROR("Malloc failed for read buf in adv change.");
        return;
    } else {
        memset(p_buf, 0, adv_data_length + scan_rsp_data_length + 3);
        p_buf[0] = 0;
        p_buf[1] = adv_data_length + scan_rsp_data_length;
        memcpy(&p_buf[2], adv_data, adv_data_length);
        memcpy(&p_buf[2 + adv_data_length], scan_rsp_data, scan_rsp_data_length);
    }

    tuya_ble_device_info_characteristic_value_update(p_buf, (adv_data_length + scan_rsp_data_length + 3));

    tuya_ble_free(p_buf);
#endif

#if TUYA_BLE_FEATURE_LONG_RANGE
    p_buf = tuya_ble_malloc(adv_data_length + scan_rsp_data_length);
    if (p_buf == NULL) {
        TUYA_BLE_LOG_ERROR("Malloc failed for read buf in ext adv change.");
        return;
    } else {
        memset(p_buf, 0, adv_data_length + scan_rsp_data_length);
        memcpy(p_buf, adv_data, adv_data_length);
        memcpy(&p_buf[adv_data_length], scan_rsp_data, scan_rsp_data_length);
    }

    tuya_ble_long_range_ext_adv_data_update(p_buf, (adv_data_length + scan_rsp_data_length));

    tuya_ble_free(p_buf);
#endif

}

VOID_T tuya_ble_adv_change(VOID_T)
{
    tuya_ble_adv_change_setting(0);
}

VOID_T tuya_ble_adv_change_with_connecting_request(VOID_T)
{
    tuya_ble_adv_change_setting(1);
}

#endif

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN==3)

UINT8_T tuya_ble_get_adv_connect_request_bit_status(VOID_T)
{
    return (scan_rsp_data[8] & 0x01);
}

STATIC VOID_T tuya_ble_adv_change_setting(UINT8_T enable_connect_req)
{
    UINT8_T *aes_buf = NULL;
    UINT8_T encry_device_id[DEVICE_ID_LEN];

    memcpy(adv_data, adv_data_const, TUYA_BLE_ADV_DATA_LEN);
    memcpy(&scan_rsp_data, scan_rsp_data_const, TUYA_BLE_SCAN_RSP_DATA_LEN);

    adv_data[7] = 4 + tuya_ble_current_para.pid_len;
    adv_data[11] = tuya_ble_current_para.pid_type;

    if (TUYA_BLE_DEVICE_SHARED) {
        scan_rsp_data[8] |= 0x02 ;
    } else {
        scan_rsp_data[8] &= (~0x02);
    }

    if (enable_connect_req) {
        scan_rsp_data[8] |= 0x01 ;
    } else {
        scan_rsp_data[8] &= (~0x01);
    }

#if (TUYA_BLE_PROTOCOL_VERSION_LOW >= 8)
    scan_rsp_data[8] |= 0x20;
    if (tuya_ble_current_para.sys_settings.protocol_v2_enable) {
        scan_rsp_data[8] |= 0x40;
    } else {
        scan_rsp_data[8] &= (~0x40);
    }
#endif

    scan_rsp_data[9] = TUYA_BLE_PROTOCOL_VERSION_HIGN;

    scan_rsp_data[10] = TUYA_BLE_SECURE_CONNECTION_TYPE;

    scan_rsp_data[11] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY >> 8;
    scan_rsp_data[12] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY & 0xFF;

    if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
        scan_rsp_data[8] |= 0x80 ;
        tuya_ble_encrypt_old_with_key(&tuya_ble_current_para, adv_data, TUYA_BLE_PROTOCOL_VERSION_HIGN);
        tuya_ble_device_id_encrypt(&tuya_ble_current_para, adv_data, scan_rsp_data);
    } else {
        scan_rsp_data[8] &= (~0x80);
        memcpy(&adv_data[12], tuya_ble_current_para.pid, tuya_ble_current_para.pid_len);
        tuya_ble_device_id_encrypt(&tuya_ble_current_para, adv_data, scan_rsp_data);
    }

    TUYA_BLE_LOG_INFO("adv data changed ,current bound flag = %d", tuya_ble_current_para.sys_settings.bound_flag);
    tuya_ble_gap_advertising_adv_data_update(adv_data, tuya_ble_current_para.pid_len + 12);
    tuya_ble_gap_advertising_scan_rsp_data_update(scan_rsp_data, sizeof(scan_rsp_data));
}

VOID_T tuya_ble_adv_change(VOID_T)
{
    tuya_ble_adv_change_setting(0);
}

VOID_T tuya_ble_adv_change_with_connecting_request(VOID_T)
{
    tuya_ble_adv_change_setting(1);
}

#endif

OPERATE_RET tal_common_info_init(tal_common_info_t *p_tal_common_info)
{
    memcpy(&tal_common_info, p_tal_common_info, SIZEOF(tal_common_info_t));
    return OPRT_OK;
}

tuya_ble_status_t tuya_ble_storage_device_save_mac(UINT8_T *mac)
{
    if (memcmp(tuya_ble_current_para.sys_settings.mac, mac, MAC_LEN) != 0) {
        memcpy(tuya_ble_current_para.sys_settings.mac, mac, MAC_LEN);
        return tuya_ble_storage_save_sys_settings();
    }
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_storage_device_save_ota_status(UINT8_T ota_status)
{
    if (tuya_ble_current_para.sys_settings.ota_status != ota_status) {
        tuya_ble_current_para.sys_settings.ota_status = ota_status;
        return tuya_ble_storage_save_sys_settings();
    }
    return TUYA_BLE_SUCCESS;
}

