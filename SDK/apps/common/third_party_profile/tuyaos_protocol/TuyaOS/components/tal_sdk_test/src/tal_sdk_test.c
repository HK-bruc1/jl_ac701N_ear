/**
 * @file tal_sdk_test.c
 * @brief This is tal_sdk_test file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tal_sdk_test.h"

#if defined(TUYA_SDK_TEST) && (TUYA_SDK_TEST == 1)

#if (1 == TUYA_SDK_TEST_TYPE)

#include "string.h"
//tal system
#include "tal_memory.h"
#include "tal_log.h"
#include "tal_sleep.h"
#include "tal_ota.h"
#include "tal_system.h"
//tal driver
#include "tal_rtc.h"
#include "tal_utc.h"
#include "tal_uart.h"
#include "tal_flash.h"
#include "tal_adc.h"
#include "tal_gpio.h"
#include "tal_pwm.h"
#include "tal_spi.h"
#include "tal_i2c.h"
#include "tal_watchdog.h"
//tal bluetooth
#include "tal_bluetooth.h"
#include "tal_ble_beacon.h"
#include "tal_repeater.h"

#include "tal_util.h"
#include "tal_oled.h"
#include "tuya_ble_api.h"
#include "tuya_ble_main.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_weather.h"
#include "tuya_ble_bulkdata_demo.h"
#include "tuya_ble_protocol_callback.h"

#include "tuya_sdk_callback.h"
#include "tal_sdk_test.h"
#include "app_dp_parser.h"
#if ( (TUYA_BLE_FEATURE_IOT_CHANNEL_ENABLE != 0) && (TUYA_BLE_FEATURE_SCENE_ENABLE != 0) )
#include "tuya_ble_scene.h"
#endif
#if defined(TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE) && (TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE == 1)
#include "tuya_ble_attach_ota_port.h"
#endif
#if defined(TUYA_BLE_FEATURE_ENABLE_GROUP) && (TUYA_BLE_FEATURE_ENABLE_GROUP == 1)
#include "tal_ble_group.h"
#endif

#include "tuya_ble_weather.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define     TUYA_TEST_ADC_CHANNEL       7

#define     TUYA_TEST_UART_CACHE_SIZE   256

#define     TEST_GROUP_VARIABLE \
                OPERATE_RET ret  = 0; \
                UINT32_T    idx  = 0; \
                UINT8_T     *rsp = p_rsp_data;

#define     TEST_RSP \
                rsp[idx++] = (ret >> 24) & 0xFF; \
                rsp[idx++] = (ret >> 16) & 0xFF; \
                rsp[idx++] = (ret >> 8) & 0xFF; \
                rsp[idx++] = (ret) & 0xFF;

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
#pragma pack(1)
typedef struct {
    UINT16_T sof;
    UINT8_T  version;
    UINT8_T  id;
    UINT16_T len;
    UINT8_T  type;
    UINT8_T  group_id;
    UINT8_T  cmd_id;
    UINT8_T  value[];
} test_cmd_t;

typedef struct {
    UINT32_T port_id;
    TIMER_ID timer_id;
    UINT32_T rx_count;
    UINT8_T  rx_buf[TUYA_TEST_UART_CACHE_SIZE];
} TAL_UART_CACHE_T;

typedef struct {
    BOOL_T    flag;
    UINT8_T mode;
} test_enter_sleep_t;

typedef struct {
    test_enter_sleep_t enter_sleep;
} test_param_t;

typedef struct {
    UINT32_T count;
    UINT32_T rsp_count;
} adv_report_t;
#pragma pack()

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
STATIC UINT8_T tx_buffer[256] = {0x66, 0xAA, 0x00, 0xF2};

STATIC TAL_UART_CACHE_T uart_cache = {0};

STATIC test_param_t test_param = {0};

STATIC TIMER_ID sg_test_enter_sleep_timer_id = NULL;
STATIC TIMER_ID sg_test_adv_report_timer_id = NULL;

STATIC adv_report_t sg_adv_report = {0};

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/
STATIC VOID_T tal_sdk_test_unbind_mode_rsp(UINT8_T mode);
STATIC VOID_T tal_sdk_test_adv_report_handler(TIMER_ID timer_id, VOID_T *arg);
STATIC VOID_T tal_sdk_test_uart_irq_rx_cb(TUYA_UART_NUM_E port_id, VOID_T *buff, UINT16_T len);
STATIC VOID_T tal_sdk_test_uart_timeout_handler(TIMER_ID timer_id, VOID_T *arg);




TUYA_WEAK_ATTRIBUTE VOID_T tal_sdk_test_init(VOID_T)
{
    tal_sw_timer_create(tal_sdk_test_enter_sleep_handler, NULL, &sg_test_enter_sleep_timer_id);
    tal_sw_timer_create(tal_sdk_test_adv_report_handler, NULL, &sg_test_adv_report_timer_id);

    test_cmd_send(TEST_ID_GET(TEST_GID_SYSTEM, TEST_CID_POWER_ON), NULL, 0);
}

VOID_T tal_sdk_test_ble_evt_callback(TAL_BLE_EVT_PARAMS_T *p_event)
{
    switch (p_event->type) {
    case TAL_BLE_EVT_PERIPHERAL_CONNECT: {
    } break;

    case TAL_BLE_EVT_DISCONNECT: {
    } break;

    case TAL_BLE_EVT_ADV_REPORT: {
        TAL_BLE_ADV_REPORT_T *p_adv_report = &p_event->ble_event.adv_report;

        sg_adv_report.count++;
        if (p_adv_report->adv_type == TAL_BLE_RSP_DATA) {
            sg_adv_report.rsp_count++;
        }
    }
    break;

    case TAL_BLE_EVT_CONN_PARAM_UPDATE: {
        TAL_BLE_CONN_PARAMS_T conn_param = p_event->ble_event.conn_param.conn;
        tal_util_reverse_byte(&conn_param.min_conn_interval, sizeof(UINT16_T));
        tal_util_reverse_byte(&conn_param.max_conn_interval, sizeof(UINT16_T));
        tal_util_reverse_byte(&conn_param.latency, sizeof(UINT16_T));
        tal_util_reverse_byte(&conn_param.conn_sup_timeout, sizeof(UINT16_T));
        test_cmd_send(TEST_ID_GET(TEST_GID_CONN, TEST_CID_CONN_PARAM_UPDATE), (VOID_T *)&conn_param, SIZEOF(TAL_BLE_CONN_PARAMS_T));
    }
    break;

    case TAL_BLE_EVT_MTU_REQUEST: {
        UINT16_T mtu = p_event->ble_event.exchange_mtu.mtu;
        tal_util_reverse_byte(&mtu, sizeof(UINT16_T));
        test_cmd_send(TEST_ID_GET(TEST_GID_CONN, TEST_CID_MTU_UPDATE), (VOID_T *)&mtu, SIZEOF(UINT16_T));
    }
    break;

    default: {
    } break;
    }
}

VOID_T tal_sdk_test_ble_protocol_callback(tuya_ble_cb_evt_param_t *event)
{
#if (TUYA_BLE_FEATURE_IOT_CHANNEL_ENABLE != 0) && (TUYA_BLE_FEATURE_SCENE_ENABLE != 0)
    UINT8_T *rsp_buf = NULL;
    UINT16_T rsp_buf_len = 0;
#endif

    switch (event->evt) {
    case TUYA_BLE_CB_EVT_CONNECT_STATUS: {
        test_cmd_send(TEST_ID_GET(TEST_GID_CONN, TEST_CID_NET_STATE), (VOID_T *)&event->connect_status, SIZEOF(UINT8_T));
    }
    break;

    case TUYA_BLE_CB_EVT_DP_DATA_RECEIVED: {
        if (event->dp_received_data.data_len < 200) {
            test_cmd_send(TEST_ID_GET(TEST_GID_DATA, TEST_CID_DP_WRITE), event->dp_received_data.p_data, event->dp_received_data.data_len);
        } else {
            UINT16_T dp_received_data_len = event->dp_received_data.data_len;
            tal_util_reverse_byte((VOID_T *)&dp_received_data_len, SIZEOF(UINT16_T));
            test_cmd_send(TEST_ID_GET(TEST_GID_DATA, TEST_CID_LONG_DP_WRITE), (VOID_T *)&dp_received_data_len, SIZEOF(UINT16_T));
        }
    }
    break;

    case TUYA_BLE_CB_EVT_DP_DATA_SEND_RESPONSE: {
        test_cmd_send(TEST_ID_GET(TEST_GID_DATA, TEST_CID_DP_REPORT_RSP), &event->dp_send_response_data.status, SIZEOF(UINT8_T));
    }
    break;

    case TUYA_BLE_CB_EVT_DP_DATA_WITH_TIME_SEND_RESPONSE: {
        test_cmd_send(TEST_ID_GET(TEST_GID_DATA, TEST_CID_DP_REPORT_TIME_RSP), &event->dp_with_time_send_response_data.status, SIZEOF(UINT8_T));
    }
    break;

    case TUYA_BLE_CB_EVT_TIME_STAMP: {
        UINT32_T timestamp_s = 0;
        tal_util_str_intstr2int((VOID_T *)event->timestamp_data.timestamp_string, 10, &timestamp_s);

        tal_utc_date_t date = {0};
        tal_utc_timestamp2date(timestamp_s, &date, false);
        date.year -= 2000;
        tal_sdk_test_get_time_rsp(&date);
    }
    break;

    case TUYA_BLE_CB_EVT_TIME_NORMAL: {
        tal_utc_date_t date = {0};
        date.year = event->time_normal_data.nYear;
        date.month = event->time_normal_data.nMonth;
        date.day = event->time_normal_data.nDay;
        date.hour = event->time_normal_data.nHour;
        date.min = event->time_normal_data.nMin;
        date.sec = event->time_normal_data.nSec;
        date.dayIndex = event->time_normal_data.DayIndex;
        tal_sdk_test_get_time_rsp(&date);
    }
    break;

    case TUYA_BLE_CB_EVT_APP_LOCAL_TIME_NORMAL: {
        tal_utc_date_t date = {0};
        date.year = event->time_normal_data.nYear;
        date.month = event->time_normal_data.nMonth;
        date.day = event->time_normal_data.nDay;
        date.hour = event->time_normal_data.nHour;
        date.min = event->time_normal_data.nMin;
        date.sec = event->time_normal_data.nSec;
        date.dayIndex = event->time_normal_data.DayIndex;
        tal_sdk_test_get_time_rsp(&date);
    }
    break;

    case TUYA_BLE_CB_EVT_UNBOUND: {
        tal_sdk_test_unbind_mode_rsp(0);
    }
    break;

    case TUYA_BLE_CB_EVT_ANOMALY_UNBOUND: {
        tal_sdk_test_unbind_mode_rsp(1);
    }
    break;

    case TUYA_BLE_CB_EVT_DEVICE_RESET: {
        tal_sdk_test_unbind_mode_rsp(2);
    }
    break;

    case TUYA_BLE_CB_EVT_UNBIND_RESET_RESPONSE: {
        if (event->reset_response_data.type == RESET_TYPE_UNBIND) {
            if (event->reset_response_data.status == 0) {
                tal_sdk_test_unbind_mode_rsp(3);
            }
        } else if (event->reset_response_data.type == RESET_TYPE_FACTORY_RESET) {
            if (event->reset_response_data.status == 0) {
                tal_sdk_test_unbind_mode_rsp(4);
            }
        }
    }
    break;

    case TUYA_BLE_CB_EVT_DATA_PASSTHROUGH: {
        test_cmd_send(TEST_ID_GET(TEST_GID_DATA, TEST_CID_DP_PASSTHROUGH), event->ble_passthrough_data.p_data, event->ble_passthrough_data.data_len);
    }
    break;

    case TUYA_BLE_CB_EVT_WEATHER_DATA_REQ_RESPONSE: {
        test_cmd_send(TEST_ID_GET(TEST_GID_ELSE, TEST_CID_GET_WEATHER_RSP), &event->weather_req_response_data.status, SIZEOF(UINT8_T));
        TAL_PR_INFO("received weather data request response result code = %d", event->weather_req_response_data.status);
    }
    break;

    case TUYA_BLE_CB_EVT_WEATHER_DATA_RECEIVED: {
        tuya_ble_wd_object_t *object;
        UINT16_T object_len = 0;
        for (;;) {
            object = (tuya_ble_wd_object_t *)(event->weather_received_data.p_data + object_len);

            TAL_PR_DEBUG("recvived weather data, n_days = [%d] key = [0x%08x] val_type = [%d] val_len = [%d]", \
                         object->n_day, object->key_type, object->val_type, object->value_len);
            TAL_PR_HEXDUMP_INFO("vaule :", (UINT8_T *)object->vaule, object->value_len);

            // TODO .. YOUR JOBS

            object_len += (SIZEOF(tuya_ble_wd_object_t) + object->value_len);
            if (object_len >= event->weather_received_data.data_len) {
                break;
            }
        }
        if (event->weather_received_data.data_len < 200) {
            test_cmd_send(TEST_ID_GET(TEST_GID_ELSE, TEST_CID_GET_WEATHER_DATA), event->weather_received_data.p_data, event->weather_received_data.data_len);
        } else {
            test_cmd_send(TEST_ID_GET(TEST_GID_ELSE, TEST_CID_GET_WEATHER_DATA), NULL, 0);
        }
    }
    break;


#if (TUYA_BLE_FEATURE_IOT_CHANNEL_ENABLE != 0) && (TUYA_BLE_FEATURE_SCENE_ENABLE != 0)
    case TUYA_BLE_CB_EVT_SCENE_REQ_RESPONSE: {
        rsp_buf = tuya_ble_malloc(1 + sizeof(tuya_ble_scene_req_response_t));
        if (rsp_buf != NULL) {
            // type + cmd + err_code
            rsp_buf[rsp_buf_len++] = 0x01;
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_req_response_data.scene_cmd >> 8);
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_req_response_data.scene_cmd);
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_req_response_data.err_code >> 24);
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_req_response_data.err_code >> 16);
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_req_response_data.err_code >> 8);
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_req_response_data.err_code);

            test_cmd_send(TEST_ID_GET(TEST_GID_ELSE, TEST_CID_GET_SCENE_RSP), rsp_buf, rsp_buf_len);
            tuya_ble_free(rsp_buf);
        }
    }
    break;

    case TUYA_BLE_CB_EVT_SCENE_DATA_RECEIVED: {
        rsp_buf = tuya_ble_malloc(1 + 4 + 1 + 4 + 4);
        if (rsp_buf != NULL) {
            // type + err_code + updata_flag + crc32
            rsp_buf[rsp_buf_len++] = 0x02;
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_data_received_data.err_code >> 24);
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_data_received_data.err_code >> 16);
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_data_received_data.err_code >> 8);
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_data_received_data.err_code);
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_data_received_data.need_update);
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_data_received_data.check_code >> 24);
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_data_received_data.check_code >> 16);
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_data_received_data.check_code >> 8);
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_data_received_data.check_code);
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_data_received_data.data_len >> 24);
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_data_received_data.data_len >> 16);
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_data_received_data.data_len >> 8);
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_data_received_data.data_len);

            test_cmd_send(TEST_ID_GET(TEST_GID_ELSE, TEST_CID_GET_SCENE_RSP), rsp_buf, rsp_buf_len);
            tuya_ble_free(rsp_buf);
        }

        // scene_data
        UINT8_T *tmp_buf = event->scene_data_received_data.p_data;
        for (UINT32_T idx = 0; idx < event->scene_data_received_data.data_len;) {
            UINT32_T id_len = tmp_buf[idx];
            UINT32_T name_len = (tmp_buf[idx + 1 + id_len] << 8) + tmp_buf[idx + 2 + id_len];
            UINT32_T item_len = 1 + id_len + 2 + name_len;

            rsp_buf_len = 0;

            rsp_buf = tuya_ble_malloc(1 + item_len);
            if (rsp_buf != NULL) {
                rsp_buf[rsp_buf_len++] = 0x04;

                if (event->scene_data_received_data.status == 0 && \
                    event->scene_data_received_data.need_update && \
                    event->scene_data_received_data.data_len != 0) {
                    memcpy(&rsp_buf[rsp_buf_len], &tmp_buf[idx], item_len);
                    rsp_buf_len += item_len;
                }

                test_cmd_send(TEST_ID_GET(TEST_GID_ELSE, TEST_CID_GET_SCENE_RSP), rsp_buf, rsp_buf_len);
                tal_system_delay(100);
                tal_watchdog_refresh();
                tuya_ble_free(rsp_buf);
            }

            idx += item_len;
        }
    }
    break;

    case TUYA_BLE_CB_EVT_SCENE_CTRL_RESULT_RECEIVED: {
        rsp_buf = tuya_ble_malloc(1 + sizeof(tuya_ble_scene_ctrl_received_data_t) + event->scene_ctrl_received_data.scene_id_len);
        if (rsp_buf != NULL) {
            // type + err_code + id_len + sceneid
            rsp_buf[rsp_buf_len++] = 0x03;
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_ctrl_received_data.err_code >> 24);
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_ctrl_received_data.err_code >> 16);
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_ctrl_received_data.err_code >> 8);
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_ctrl_received_data.err_code);
            rsp_buf[rsp_buf_len++] = (UINT8_T)(event->scene_ctrl_received_data.scene_id_len);
            if (event->scene_ctrl_received_data.scene_id_len != 0) {
                memcpy(&rsp_buf[rsp_buf_len], event->scene_ctrl_received_data.p_scene_id, event->scene_ctrl_received_data.scene_id_len);
                rsp_buf_len += event->scene_ctrl_received_data.scene_id_len;
            }

            test_cmd_send(TEST_ID_GET(TEST_GID_ELSE, TEST_CID_GET_SCENE_RSP), rsp_buf, rsp_buf_len);
            tuya_ble_free(rsp_buf);
        }
    }
    break;
#endif // (TUYA_BLE_FEATURE_IOT_CHANNEL_ENABLE != 0) && (TUYA_BLE_FEATURE_SCENE_ENABLE != 0)

    default: {
    } break;
    }
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET test_group_system(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data)
{
    TEST_GROUP_VARIABLE

    switch (cmd) {
    case TEST_CID_SHAKE_HAND: {
    } break;

    case TEST_CID_RESET: {
        tuya_ble_disconnect_and_reset_timer_start();
        ret = 0;
        TEST_RSP
    }
    break;

    case TEST_CID_FACTORY_RESET: {
        if (cmd_data[0] == 0) {
            ret = tuya_ble_device_unbind();
        } else if (cmd_data[0] == 1) {
            ret = tuya_ble_device_factory_reset();
        } else {
            ret = OPRT_NOT_SUPPORTED;
        }
        TEST_RSP
    }
    break;

    case TEST_CID_GET_TIME: {
        TIME_T time_sec = 0;
        tal_rtc_time_get(&time_sec);

        tal_utc_date_t date = {0};
        tal_utc_timestamp2date(time_sec, &date, false);

        INT16_T time_zone = tal_utc_get_time_zone();

        rsp[0] = date.year - 2000;
        rsp[1] = date.month;
        rsp[2] = date.day;
        rsp[3] = date.hour;
        rsp[4] = date.min;
        rsp[5] = date.sec;
        rsp[6] = date.dayIndex;
        rsp[7] = time_zone >> 8;
        rsp[8] = time_zone;
        idx += 9;
    }
    break;

    case TEST_CID_REQ_TIME: {
        if (tuya_ble_connect_status_get() == BONDING_CONN) {
            ret = tuya_ble_time_req(cmd_data[0]);
        } else {
            ret = OPRT_NETWORK_ERROR;
        }

        TEST_RSP
    }
    break;

    case TEST_CID_GET_RAND: {
        UINT8_T rand_buf[32] = {0};
        UINT8_T len = cmd_data[0];
        tuya_ble_rand_generator(rand_buf, len);

        memcpy(rsp, rand_buf, len);
        idx += len;
    }
    break;

    case TEST_CID_GET_QUEUE_SIZE: {
#if !TUYA_BLE_USE_OS
        UINT16_T queue_size = tuya_ble_scheduler_queue_size_get();
        UINT16_T queue_space = tuya_ble_scheduler_queue_space_get();
#else
        UINT16_T queue_size = 0;
        UINT16_T queue_space = 0;
#endif
        rsp[0] = queue_size >> 8;
        rsp[1] = queue_size & 0xFF;
        rsp[2] = queue_space >> 8;
        rsp[3] = queue_space & 0xFF;
        idx += 4;
    }
    break;

    default: {
    } break;
    }

    return idx;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET test_group_device_info(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data)
{
    TEST_GROUP_VARIABLE

    switch (cmd) {
    case TEST_CID_SET_MAC: {
    } break;

    case TEST_CID_GET_MAC: {
        TAL_BLE_ADDR_T addr = {0};
        tal_ble_address_get(&addr);

        tal_util_reverse_byte(addr.addr, 6);
        memcpy(&rsp[idx], addr.addr, 6);
        idx += 6;
    }
    break;

    case TEST_CID_SET_PID: {
        if (cmd_data_len == TUYA_BLE_PRODUCT_ID_DEFAULT_LEN) {
            ret = tuya_ble_device_update_product_id(TUYA_BLE_PRODUCT_ID_TYPE_PID, TUYA_BLE_PRODUCT_ID_DEFAULT_LEN, cmd_data);
        } else {
            ret = OPRT_INVALID_PARM;
        }

        TEST_RSP
    }
    break;

    case TEST_CID_GET_PID: {
        rsp[idx] = TUYA_BLE_PRODUCT_ID_TYPE_PID;
        idx += 1;

        rsp[idx] = 8;
        idx += 1;

        memcpy(&rsp[idx], TY_DEVICE_PID, 8);
        idx += 8;
    }
    break;

    case TEST_CID_SET_VERSION: {
        UINT32_T firmware_version = (cmd_data[0] << 24) + (cmd_data[1] << 16) + (cmd_data[2] << 8) + cmd_data[3];
        UINT32_T hardware_version = (cmd_data[4] << 24) + (cmd_data[5] << 16) + (cmd_data[6] << 8) + cmd_data[7];
        tuya_ble_set_device_version(firmware_version, hardware_version);

        ret = 0;
        TEST_RSP
    }
    break;

    case TEST_CID_GET_VERSION: {
        rsp[0] = tal_common_info.firmware_version >> 24;
        rsp[1] = tal_common_info.firmware_version >> 16;
        rsp[2] = tal_common_info.firmware_version >> 8;
        rsp[3] = tal_common_info.firmware_version & 0xFF;
        rsp[4] = tal_common_info.hardware_version >> 24;
        rsp[5] = tal_common_info.hardware_version >> 16;
        rsp[6] = tal_common_info.hardware_version >> 8;
        rsp[7] = tal_common_info.hardware_version & 0xFF;
        idx += 8;
    }
    break;

    default: {
    } break;
    }

    return idx;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET test_group_adv(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data)
{
    TEST_GROUP_VARIABLE

    switch (cmd) {
    case TEST_CID_ADV_ENABLE: {
    } break;

    case TEST_CID_SET_ADV_INTERVAL: {
        UINT16_T adv_interval_min = (cmd_data[0] << 8) + cmd_data[1];
        UINT16_T adv_interval_max = (cmd_data[2] << 8) + cmd_data[3];

        tal_adv_param.adv_type = TAL_BLE_ADV_TYPE_CS_UNDIR;
        tal_adv_param.direct_addr.type = 0;
        tal_adv_param.adv_interval_min = adv_interval_min * 8 / 5;
        tal_adv_param.adv_interval_max = adv_interval_max * 8 / 5;

        ret = tal_ble_advertising_start(&tal_adv_param);
        TEST_RSP
    }
    break;

    case TEST_CID_SET_ADV_PARAM: {
        UINT16_T adv_interval_min = (cmd_data[0] << 8) + cmd_data[1];
        UINT16_T adv_interval_max = (cmd_data[2] << 8) + cmd_data[3];

        tal_adv_param.adv_type = 1;
        tal_adv_param.direct_addr.type = 0;
        tal_adv_param.adv_interval_min = adv_interval_min * 8 / 5;
        tal_adv_param.adv_interval_max = adv_interval_max * 8 / 5;

        ret = tal_ble_advertising_start(&tal_adv_param);
        TEST_RSP
    }
    break;

    case TEST_CID_SET_ADV_DATA: {
        TAL_BLE_DATA_T adv_data = {0};
        TAL_BLE_DATA_T scan_rsp = {0};

        adv_data.len = cmd_data[0];
        adv_data.p_data = &cmd_data[1];
        scan_rsp.len = cmd_data[adv_data.len + 1];
        scan_rsp.p_data = &cmd_data[adv_data.len + 2];

        ret = tal_ble_advertising_data_update(&adv_data, &scan_rsp);
        TEST_RSP
    }
    break;

    case TEST_CID_CONN_REQUEST: {
        ret = tuya_ble_adv_data_connecting_request_set(cmd_data[0]);
        TEST_RSP
    }
    break;

    case TEST_CID_MESH_FAST_PROV: {
    } break;

#if defined(TUYA_BLE_FEATURE_LONG_RANGE) && (TUYA_BLE_FEATURE_LONG_RANGE == 1)
    case TEST_CID_SET_EXT_ADV_PARAM: {
        uint16_t adv_interval = (cmd_data[0] << 8) + cmd_data[1];
        // uint16_t adv_type = (cmd_data[2]<<8) + cmd_data[3];
        // uint8_t  primary_phy = cmd_data[4];
        // uint8_t  secondary_phy = cmd_data[5];

        tal_ext_adv_param.adv_interval_min = adv_interval * 8 / 5;
        tal_ext_adv_param.adv_interval_max = adv_interval * 8 / 5;
        // tal_ext_adv_param.properties = adv_type;
        // tal_ext_adv_param.primary_phy = primary_phy;
        // tal_ext_adv_param.secondary_phy = secondary_phy;

        tal_ble_ext_advertising_config(tal_ext_adv_extended_adv, &tal_ext_adv_param, NULL, NULL);
        ret = tal_ble_ext_advertising_start(tal_ext_adv_extended_adv);
        TEST_RSP
    }
    break;
#endif

    default: {
    } break;
    }

    return idx;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET test_group_scan(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data)
{
    TEST_GROUP_VARIABLE

    switch (cmd) {
    case TEST_CID_SCAN_START: {
        TAL_BLE_SCAN_PARAMS_T tal_scan_param = {
            .type = TAL_BLE_SCAN_TYPE_ACTIVE,
            .scan_interval = 100 * 8 / 5,
            .scan_window = 80 * 8 / 5,
            .timeout = 0,
            .filter_dup = 0,
        };

        UINT8_T  type = cmd_data[0];
        UINT16_T scan_interval = (cmd_data[1] << 8) + cmd_data[2];
        UINT16_T scan_window = (cmd_data[3] << 8) + cmd_data[4];

        sg_adv_report.count = 0;
        sg_adv_report.rsp_count = 0;

        tal_scan_param.type = type;
        tal_scan_param.scan_interval = scan_interval;
        tal_scan_param.scan_window = scan_window;

        ret = tal_ble_scan_start(&tal_scan_param);
        if (ret == OPRT_OK) {
            tal_sw_timer_start(sg_test_adv_report_timer_id, 1000, TAL_TIMER_CYCLE);
        }

        TEST_RSP
    }
    break;

    case TEST_CID_SCAN_STOP: {
        ret = tal_ble_scan_stop();
        if (ret == OPRT_OK) {
            tal_sw_timer_stop(sg_test_adv_report_timer_id);
        }

        TEST_RSP
    }
    break;

    default: {
    } break;
    }

    return idx;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET test_group_conn(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data)
{
    TEST_GROUP_VARIABLE

    switch (cmd) {
    case TEST_CID_CONN: {
    } break;

    case TEST_CID_SET_CONN_INTERVAL: {
        UINT16_T conn_interval_min = (cmd_data[0] << 8) + cmd_data[1];
        UINT16_T conn_interval_max = (cmd_data[2] << 8) + cmd_data[3];

        TAL_BLE_PEER_INFO_T peer_info = {0};
        peer_info.conn_handle = tuya_app_get_conn_handle();
        TAL_BLE_CONN_PARAMS_T conn_param = {0};
        conn_param.min_conn_interval = conn_interval_min * 4 / 5;
        conn_param.max_conn_interval = conn_interval_max * 4 / 5;
        conn_param.latency = 0;
        conn_param.conn_sup_timeout = 6000 / 10;
        conn_param.connection_timeout = 0;
        ret = tal_ble_conn_param_update(peer_info, &conn_param);
        TEST_RSP
    }
    break;

    case TEST_CID_SET_CONN_PARAM: {
        UINT16_T conn_interval_min = (cmd_data[0] << 8) + cmd_data[1];
        UINT16_T conn_interval_max = (cmd_data[2] << 8) + cmd_data[3];

        TAL_BLE_PEER_INFO_T peer_info = {0};
        peer_info.conn_handle = tuya_app_get_conn_handle();
        TAL_BLE_CONN_PARAMS_T conn_param = {0};
        conn_param.min_conn_interval = conn_interval_min * 4 / 5;
        conn_param.max_conn_interval = conn_interval_max * 4 / 5;
        conn_param.latency = 0;
        conn_param.conn_sup_timeout = 6000 / 10;
        conn_param.connection_timeout = 0;
        ret = tal_ble_conn_param_update(peer_info, &conn_param);
        TEST_RSP
    }
    break;

    case TEST_CID_DISCONN: {
        TAL_BLE_PEER_INFO_T info = {0};
        info.conn_handle = tuya_app_get_conn_handle();
        ret = tal_ble_disconnect(info);
        TEST_RSP
    }
    break;

    case TEST_CID_NET_STATE: {
        if (cmd_data[0] == 0) {
            rsp[idx] = tuya_ble_connect_status_get();
        } else {
            rsp[idx] = 0xFF;
        }
        idx++;
    }
    break;

    case TEST_CID_GET_MESH_ADDR: {
    } break;

    default: {
    } break;
    }

    return idx;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET test_group_data(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data)
{
    TEST_GROUP_VARIABLE

    switch (cmd) {
    case TEST_CID_MASTER_SEND: {
    } break;

    case TEST_CID_SLAVE_SEND: {
        TAL_BLE_DATA_T data = {0};
        data.len = cmd_data_len;
        data.p_data = cmd_data;
        ret = tal_ble_server_common_send(&data);
        TEST_RSP
    }
    break;

    case TEST_CID_DP_REPORT: {
        ret = tuya_ble_dp_data_send(g_sn++, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, cmd_data[0], &cmd_data[1], cmd_data_len - 1);
        TEST_RSP
    }
    break;

    case TEST_CID_LONG_DP_REPORT: {
        UINT8_T *tmp_buf = tal_malloc(TUYA_BLE_SEND_MAX_DATA_LEN);
        if (tmp_buf) {
            UINT16_T dp_data_len = (cmd_data[5] << 8) + cmd_data[6];

            tmp_buf[0] = cmd_data[1];
            tmp_buf[1] = cmd_data[2];
            tmp_buf[2] = cmd_data[5];
            tmp_buf[3] = cmd_data[6];

            for (UINT32_T idx = 0; idx < dp_data_len; idx++) {
                tmp_buf[4 + idx] = idx;
            }

            ret = tuya_ble_dp_data_send(g_sn++, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, cmd_data[0], tmp_buf, dp_data_len + 4);

            tal_free(tmp_buf);
        } else {
            ret = OPRT_MALLOC_FAILED;
        }

        TEST_RSP
    }
    break;

    case TEST_CID_DP_REPORT_TIME: {
        if (cmd_data[0] == DP_TIME_TYPE_MS_STRING) {
            ret = tuya_ble_dp_data_with_time_send(g_sn++, DP_SEND_FOR_CLOUD_PANEL, cmd_data[0], &cmd_data[1], cmd_data + 1 + 13, cmd_data_len - 1 - 13);
        } else if (cmd_data[0] == DP_TIME_TYPE_UNIX_TIMESTAMP) {
            ret = tuya_ble_dp_data_with_time_send(g_sn++, DP_SEND_FOR_CLOUD_PANEL, cmd_data[0], &cmd_data[1], cmd_data + 1 + 4, cmd_data_len - 1 - 4);
        }

        TEST_RSP
    }
    break;

    case TEST_CID_LONG_DP_REPORT_TIME: {
        UINT8_T *tmp_buf = tal_malloc(TUYA_BLE_SEND_MAX_DATA_LEN);
        if (tmp_buf) {
            UINT16_T dp_data_len = (cmd_data[9] << 8) + cmd_data[10];

            tmp_buf[0] = cmd_data[5];
            tmp_buf[1] = cmd_data[6];
            tmp_buf[2] = cmd_data[9];
            tmp_buf[3] = cmd_data[10];

            for (UINT32_T idx = 0; idx < dp_data_len; idx++) {
                tmp_buf[4 + idx] = idx;
            }

            if (cmd_data[0] == DP_TIME_TYPE_MS_STRING) {
                ret = tuya_ble_dp_data_with_time_send(g_sn++, DP_SEND_FOR_CLOUD_PANEL, cmd_data[0], &cmd_data[1], tmp_buf, dp_data_len + 4);
            } else if (cmd_data[0] == DP_TIME_TYPE_UNIX_TIMESTAMP) {
                ret = tuya_ble_dp_data_with_time_send(g_sn++, DP_SEND_FOR_CLOUD_PANEL, cmd_data[0], &cmd_data[1], tmp_buf, dp_data_len + 4);
            }

            tal_free(tmp_buf);
        } else {
            ret = OPRT_MALLOC_FAILED;
        }

        TEST_RSP
    }
    break;

    case TEST_CID_DP_PASSTHROUGH: {
    } break;

    case TEST_CID_MESH_DP_REPORT: {
    } break;

#if (TAL_BLE_BEACON_ROAMING_FLAG)
    case TEST_CID_ROAMING_DP_REPORT: {
        g_roaming_param.adv_duration = (cmd_data[1] << 24) + (cmd_data[2] << 16) + (cmd_data[3] << 8) + cmd_data[4];

        g_dp_report_count = 6;
        ret = tal_ble_beacon_dp_data_send(adv_frame_counter++, cmd_data[0], 20, g_roaming_param.adv_duration, &cmd_data[5], cmd_data_len - 5);
        if (adv_frame_counter % 256 == 0) {
            tuya_ble_custom_evt_send(APP_EVT_1);
        }
        TEST_RSP
    }
    break;
#endif

    default: {
    } break;
    }

    return idx;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET test_group_else(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data)
{
    TEST_GROUP_VARIABLE

    switch (cmd) {
#if (TUYA_BLE_FEATURE_WEATHER_ENABLE)
    case TEST_CID_GET_WEATHER: {
        UINT32_T combine_type = (cmd_data[1] << 24) + (cmd_data[2] << 16) + (cmd_data[3] << 8) + cmd_data[4];

        if (tuya_ble_connect_status_get() == BONDING_CONN) {
            ret = tuya_ble_feature_weather_data_request_with_location(cmd_data[0], combine_type, cmd_data[5]);
        } else {
            ret = OPRT_NETWORK_ERROR;
        }

        TEST_RSP
    }
    break;
#endif
    case TEST_CID_SET_BULK_DATA: {
//            UINT32_T cycle = (cmd_data[0]<<24) + (cmd_data[1]<<16) + (cmd_data[2]<<8) + cmd_data[3];
        UINT32_T timestep = (cmd_data[4] << 24) + (cmd_data[5] << 16) + (cmd_data[6] << 8) + cmd_data[7];

        if (tuya_ble_connect_status_get() == BONDING_CONN) {
            ret = tuya_ble_bulk_data_generation(timestep, &cmd_data[8], cmd_data_len - 8);
        } else {
            ret = OPRT_NETWORK_ERROR;
        }

        TEST_RSP
    }
    break;

#if (TUYA_BLE_FEATURE_IOT_CHANNEL_ENABLE != 0) && (TUYA_BLE_FEATURE_SCENE_ENABLE != 0)
    case TEST_CID_GET_SCENE_LIST: {
        tuya_ble_request_scene_t req_scene_data;

        if (tuya_ble_connect_status_get() != BONDING_CONN) {
            ret = OPRT_NETWORK_ERROR;
        } else {
            /* request scene list data */
            UINT16_T name_unicode_len = (cmd_data[1] << 8) + cmd_data[2];
            UINT32_T check_code = ((cmd_data[3] << 24) + (cmd_data[4] << 16) + (cmd_data[5] << 8) + cmd_data[6]);

            req_scene_data.scene_cmd = REQUEST_SCENE_DATA;
            req_scene_data.scene_data.nums = cmd_data[0];
            req_scene_data.scene_data.name_unicode_length = name_unicode_len;
            req_scene_data.scene_data.check_code = check_code;

            ret = tuya_ble_feature_scene_request(&req_scene_data);
        }

        TEST_RSP
    }
    break;

    case TEST_CID_REQ_SCENE_CTRL: {
        tuya_ble_request_scene_t req_scene_data;

        if (tuya_ble_connect_status_get() != BONDING_CONN) {
            ret = OPRT_NETWORK_ERROR;
        } else {
            /* request scene control */
            req_scene_data.scene_cmd = REQUEST_SCENE_CONTROL;
            req_scene_data.scene_control.scene_id_length = cmd_data[0];
            memcpy(req_scene_data.scene_control.scene_id, &(cmd_data[1]), req_scene_data.scene_control.scene_id_length);

            ret = tuya_ble_feature_scene_request(&req_scene_data);
        }

        TEST_RSP
    }
    break;
#endif

#if defined(TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE) && (TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE == 1)
    case TEST_CID_SET_ATTACH_OTA_VERSION: {
        (*p_attach_ota_idx) = 0;
        for (UINT32_T idx = 0; idx < TUYA_BLE_ATTACH_OTA_PORT_NUM; idx++) {
            if (idx == 0) {
                tuya_ble_attach_ota_info[idx].type = 1;
                tuya_ble_attach_ota_info[idx].version = ((cmd_data[0] << 24) + (cmd_data[1] << 16) + (cmd_data[2] << 8) + cmd_data[3]);
                if (tuya_ble_attach_ota_info[idx].version != 0x00010000) {
                    (*p_attach_ota_idx) = 1;
                }
            } else {
                tuya_ble_attach_ota_info[idx].type = idx + 9;
                tuya_ble_attach_ota_info[idx].version = ((cmd_data[4] << 24) + (cmd_data[5] << 16) + (cmd_data[6] << 8) + cmd_data[7]);
            }
            memset(&tuya_ble_attach_ota_info[idx].firmware_info, 0, sizeof(TUYA_OTA_FIRMWARE_INFO_T));
        }

        ret += tuya_ble_device_update_mcu_version(tuya_ble_attach_ota_info[0].version, 0x00010000);
        ret += tuya_ble_device_update_attach_version(&tuya_ble_attach_ota_info[1], TUYA_BLE_ATTACH_OTA_PORT_NUM - 1);

        tuya_ble_attach_ota_port_info_save();

        tuya_ble_disconnect_and_reset_timer_start();

        TEST_RSP
    }
    break;
#endif

#if defined(TUYA_BLE_FEATURE_ENABLE_GROUP) && (TUYA_BLE_FEATURE_ENABLE_GROUP == 1)
    case TEST_CID_SEND_BLE_GROUP_DATA: {
        uint16_t group_idx = (cmd_data[0] << 8) | cmd_data[1];
        tal_ble_group_remoter_dp_send(group_idx, cmd_data + 2, cmd_data_len - 2);
        app_group_dp_parser(cmd_data + 3, cmd_data_len - 3);

        TEST_RSP
    }
    break;
#endif

    default: {
    } break;
    }

    return idx;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET test_group_gpio(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data)
{
    TEST_GROUP_VARIABLE

    switch (cmd) {
    case TEST_CID_PIN_DEINIT: {
        ret = tal_gpio_deinit(cmd_data[0]);
        TEST_RSP
    }
    break;

    case TEST_CID_OUTPUT_HIGH: {
        TUYA_GPIO_BASE_CFG_T gpio_cfg = {
            .mode = TUYA_GPIO_PUSH_PULL,
            .direct = TUYA_GPIO_OUTPUT,
            .level = TUYA_GPIO_LEVEL_LOW,
        };
        tal_gpio_init(cmd_data[0], &gpio_cfg);

        ret = tal_gpio_write(cmd_data[0], TUYA_GPIO_LEVEL_HIGH);
        TEST_RSP
    }
    break;

    case TEST_CID_OUTPUT_LOW: {
        TUYA_GPIO_BASE_CFG_T gpio_cfg = {
            .mode = TUYA_GPIO_PUSH_PULL,
            .direct = TUYA_GPIO_OUTPUT,
            .level = TUYA_GPIO_LEVEL_LOW,
        };
        tal_gpio_init(cmd_data[0], &gpio_cfg);

        ret = tal_gpio_write(cmd_data[0], TUYA_GPIO_LEVEL_LOW);
        TEST_RSP
    }
    break;

    case TEST_CID_PIN_READ: {
        TUYA_GPIO_BASE_CFG_T gpio_cfg = {
            .mode = TUYA_GPIO_PULLUP,
            .direct = TUYA_GPIO_INPUT,
            .level = TUYA_GPIO_LEVEL_LOW,
        };
        tal_gpio_init(cmd_data[0], &gpio_cfg);

        UINT32_T gpio_level = TUYA_GPIO_LEVEL_LOW;
        ret = tal_gpio_read(cmd_data[0], (TUYA_GPIO_LEVEL_E *)&gpio_level);
        rsp[idx++] = (gpio_level >> 24) & 0xFF;
        rsp[idx++] = (gpio_level >> 16) & 0xFF;
        rsp[idx++] = (gpio_level >> 8) & 0xFF;
        rsp[idx++] = (gpio_level) & 0xFF;
    }
    break;

    default: {
    } break;
    }

    return idx;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET test_group_uart(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data)
{
    TEST_GROUP_VARIABLE

    switch (cmd) {
    case TEST_CID_SET_BAUDRATE: {
        UINT32_T port_num = (cmd_data[0] << 24) + (cmd_data[1] << 16) + (cmd_data[2] << 8) + cmd_data[3];
        tal_uart_cfg.base_cfg.baudrate = (cmd_data[4] << 24) + (cmd_data[5] << 16) + (cmd_data[6] << 8) + cmd_data[7];

        ret = tal_uart_init(port_num, &tal_uart_cfg);
        if (ret == OPRT_OK) {
            tal_uart_rx_reg_irq_cb(port_num, tal_sdk_test_uart_irq_rx_cb);
            tal_sw_timer_create(tal_sdk_test_uart_timeout_handler, NULL, &uart_cache.timer_id);
            uart_cache.port_id = port_num;
        }

        TEST_RSP
    }
    break;

    case TEST_CID_TX_UART_DATA: {
        UINT32_T port_num = (cmd_data[0] << 24) + (cmd_data[1] << 16) + (cmd_data[2] << 8) + cmd_data[3];

        ret = tal_uart_write(port_num, cmd_data + 4, cmd_data_len - 4);
        TEST_RSP
    }
    break;

    default: {
    } break;
    }

    return idx;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET test_group_pwm(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data)
{
    TEST_GROUP_VARIABLE

    switch (cmd) {
    case TEST_CID_PWM_DEINIT: {
        UINT32_T channel = (cmd_data[0] << 24) | (cmd_data[1] << 16) | (cmd_data[2] << 8) | cmd_data[3];
        ret = tal_pwm_deinit(channel);
        TEST_RSP
    }
    break;

    case TEST_CID_SET_FREQ_DUTY: {
        UINT32_T channel = (cmd_data[0] << 24) | (cmd_data[1] << 16) | (cmd_data[2] << 8) | cmd_data[3];
        UINT32_T frequency = (cmd_data[4] << 24) | (cmd_data[5] << 16) | (cmd_data[6] << 8) | cmd_data[7];
        UINT32_T duty = (cmd_data[8] << 24) | (cmd_data[9] << 16) | (cmd_data[10] << 8) | cmd_data[11];
        UINT32_T cycle = (cmd_data[12] << 24) | (cmd_data[13] << 16) | (cmd_data[14] << 8) | cmd_data[15];

        TUYA_PWM_BASE_CFG_T pwm_cfg = {0};
        pwm_cfg.polarity = TUYA_PWM_POSITIVE;
        pwm_cfg.duty = duty;
        pwm_cfg.cycle = cycle;
        pwm_cfg.frequency = frequency;
        ret = tal_pwm_info_set(channel, &pwm_cfg);
        TEST_RSP
    }
    break;

    default: {
    } break;
    }

    return idx;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET test_group_adc(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data)
{
    TEST_GROUP_VARIABLE

    switch (cmd) {
    case TEST_CID_ADC_DEINIT: {
        ret = tal_adc_deinit(TUYA_ADC_NUM_0);
        TEST_RSP
    }
    break;

    case TEST_CID_READ_ADC_DATA: {
        UINT8_T  channel = cmd_data[3];
        UINT8_T  width = cmd_data[4];
        INT32_T  adc_value = 0;

        tal_adc_deinit(TUYA_ADC_NUM_0);

        TUYA_ADC_BASE_CFG_T adc_cfg = {
            .ch_nums = 1,
            .ch_list.data = (1 << channel),
            .width = width,
            .type = TUYA_ADC_EXTERNAL_SAMPLE_VOL,
        };
        ret = tal_adc_init(TUYA_ADC_NUM_0, &adc_cfg);
        if (ret == OPRT_OK) {
            ret = tal_adc_read_single_channel(TUYA_ADC_NUM_0, channel, &adc_value);
            if (ret == OPRT_OK) {
                test_cmd_send(TEST_ID_GET(TEST_GID_ADC, TEST_CID_READ_ADC_DATA_RSP), (VOID_T *)&adc_value, SIZEOF(UINT32_T));
            } else if (ret == OPRT_NOT_SUPPORTED) {
                ret = tal_adc_read_voltage(TUYA_ADC_NUM_0, &adc_value, 1);
                if (ret == OPRT_OK) {
                    test_cmd_send(TEST_ID_GET(TEST_GID_ADC, TEST_CID_READ_VOLTAGE_RSP), (VOID_T *)&adc_value, SIZEOF(UINT32_T));
                }
            }
        }

        TEST_RSP
    }
    break;

    default: {
    } break;
    }

    return idx;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET test_group_spi(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data)
{
    TEST_GROUP_VARIABLE

    switch (cmd) {
    case TEST_CID_TX_SPI_DATA: {
        UINT32_T channel        = (cmd_data[0] << 24) | (cmd_data[1] << 16) | (cmd_data[2] << 8) | cmd_data[3];
        UINT32_T frequency      = (cmd_data[4] << 24) | (cmd_data[5] << 16) | (cmd_data[6] << 8) | cmd_data[7];
        UINT8_T  *spi_data      = cmd_data + 8;
        UINT32_T spi_data_len   = cmd_data_len - 8;

        tal_spi_deinit(channel);

        TUYA_SPI_BASE_CFG_T spi_cfg = {
            .role = TUYA_SPI_ROLE_MASTER,
            .mode = TUYA_SPI_MODE0,
            .type = TUYA_SPI_SOFT_TYPE,
            .databits = TUYA_SPI_DATA_BIT8,
            .freq_hz = frequency
        };
        ret = tal_spi_init(channel, &spi_cfg);
        if (ret == OPRT_OK) {
            UINT8_T *buf = tal_malloc(spi_data_len);
            if (buf) {
                ret = tal_spi_xfer(channel, spi_data, buf, spi_data_len);
                if (ret == OPRT_OK) {
                    if (memcmp(spi_data, buf, spi_data_len) == 0) {
                        test_cmd_send(TEST_ID_GET(TEST_GID_SPI, TEST_CID_RX_SPI_DATA), buf, spi_data_len);
                    } else {
                        ret = OPRT_RECV_DA_NOT_ENOUGH;
                    }
                }

                tal_free(buf);
            } else {
                ret = OPRT_MALLOC_FAILED;
            }
        }

        TEST_RSP
    }
    break;

    case TEST_CID_RX_SPI_DATA: {
    } break;

    default: {
    } break;
    }

    return idx;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET test_group_iic(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data)
{
    TEST_GROUP_VARIABLE

    switch (cmd) {
    case TEST_CID_TX_IIC_DATA: {
        UINT32_T channel        = (cmd_data[0] << 24) | (cmd_data[1] << 16) | (cmd_data[2] << 8) | cmd_data[3];
        UINT8_T  *iic_data      = cmd_data + 4;
        UINT32_T iic_data_len   = cmd_data_len - 4;

        if (iic_data_len < 14) {
            if (tal_oled_check_i2c_port_num() != channel) {
                ret = OPRT_NOT_SUPPORTED;
            } else {
                UINT8_T buf[14] = {0};
                memcpy(buf, iic_data, iic_data_len);
                buf[iic_data_len] = '\0';

                tal_oled_clear();
                ret = tal_oled_show_string(12, 1, (VOID_T *)buf, 16);
            }
        } else {
            ret = OPRT_INVALID_PARM;
        }
        TEST_RSP
    }
    break;

    case TEST_CID_RX_IIC_DATA: {
    } break;

    default: {
    } break;
    }

    return idx;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET test_group_rtc(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data)
{
    TEST_GROUP_VARIABLE

    switch (cmd) {
    case TEST_CID_SET_RTC_TIME: {
        UINT32_T time_sec = (cmd_data[0] << 24) | (cmd_data[1] << 16) | (cmd_data[2] << 8) | cmd_data[3];
        ret = tal_rtc_time_set(time_sec);
        TEST_RSP
    }
    break;

    case TEST_CID_GET_RTC_TIME: {
        UINT32_T time_sec = 0;
        tal_rtc_time_get(&time_sec);

        rsp[0] = time_sec >> 24;
        rsp[1] = time_sec >> 16;
        rsp[2] = time_sec >> 8;
        rsp[3] = time_sec & 0xFF;
        idx += 4;
    }
    break;

    case TEST_CID_START_RTC: {
        ret = tal_rtc_init();
        TEST_RSP
    }
    break;

    case TEST_CID_STOP_RTC: {
//            ret = tal_rtc_deinit(); // RTC affects the soft timer, which affects uart rx
        ret = OPRT_NOT_SUPPORTED;
        TEST_RSP
    }
    break;

    default: {
    } break;
    }

    return idx;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET test_group_flash(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data)
{
    TEST_GROUP_VARIABLE

    switch (cmd) {
    case TEST_CID_READ_FLASH_DATA: {
        UINT32_T addr = (cmd_data[0] << 24) | (cmd_data[1] << 16) | (cmd_data[2] << 8) | cmd_data[3];
        UINT32_T len = (cmd_data[4] << 24) | (cmd_data[5] << 16) | (cmd_data[6] << 8) | cmd_data[7];

        tal_flash_read(addr, &rsp[idx], len);
        idx += len;
    }
    break;

    case TEST_CID_ERASE_FLASH_DATA: {
        UINT32_T addr = (cmd_data[0] << 24) | (cmd_data[1] << 16) | (cmd_data[2] << 8) | cmd_data[3];
        UINT32_T len = (cmd_data[4] << 24) | (cmd_data[5] << 16) | (cmd_data[6] << 8) | cmd_data[7];

        ret = tal_flash_erase(addr, len);
        TEST_RSP
    }
    break;

    case TEST_CID_WRITE_FLASH_DATA: {
        UINT32_T addr = (cmd_data[0] << 24) | (cmd_data[1] << 16) | (cmd_data[2] << 8) | cmd_data[3];
        UINT8_T  *flash_data = cmd_data + 4;
        UINT32_T flash_data_len = cmd_data_len - 4;

        ret = tal_flash_write(addr, flash_data, flash_data_len);
        TEST_RSP
    }
    break;

    default: {
    } break;
    }

    return idx;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET test_group_watchdog(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data)
{
    TEST_GROUP_VARIABLE

    switch (cmd) {
    case TEST_CID_START_WDG: {
        UINT32_T interval_ms = (cmd_data[0] << 24) | (cmd_data[1] << 16) | (cmd_data[2] << 8) | cmd_data[3];

        TUYA_WDOG_BASE_CFG_T wdog_cfg = {
            .interval_ms = interval_ms,
        };
        ret = tal_watchdog_start(&wdog_cfg);
        TEST_RSP
    }
    break;

    case TEST_CID_FEED_WDG: {
        ret = tal_watchdog_refresh();
        TEST_RSP
    }
    break;

    case TEST_CID_STOP_WDG: {
        ret = tal_watchdog_stop();
        TEST_RSP
    }
    break;

    default: {
    } break;
    }

    return idx;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET test_group_powermanger(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data)
{
    TEST_GROUP_VARIABLE

    switch (cmd) {
    case TEST_CID_ENTER_SLEEP: {
        test_param.enter_sleep.mode = cmd_data[0];

        ret = tal_sw_timer_start(sg_test_enter_sleep_timer_id, 200, TAL_TIMER_ONCE);

        TEST_RSP
    }
    break;

    default: {
    } break;
    }

    return idx;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET test_group_custom(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data)
{
    switch (cmd) {
#if (TUYA_BLE_FEATURE_REPEATER_ENABLE != 0)
    case TEST_CID_REPEATER: {
        tuya_ble_app_passthrough_data_t data = {0};
        data.type = 0x0010;
        data.data_len = cmd_data_len;
        data.p_data = cmd_data;
        tal_repeater_handler(&data);
    }
    break;
#endif

    default: {
    } break;
    }

    return 0;
}

VOID_T tuya_ble_app_sdk_test_process(UINT8_T channel, UINT8_T *p_in_data, UINT16_T in_len)
{
    if (!((p_in_data[7] == 0x01) && (p_in_data[8] == 0x05))) {
        TAL_PR_HEXDUMP_INFO("test_cmd", p_in_data + 7, in_len - 8);
    }

    test_cmd_t *cmd = (VOID_T *)p_in_data;
    tal_util_reverse_byte((VOID_T *)&cmd->len, SIZEOF(UINT16_T));

    if ((cmd->type != 3) || (cmd->len < 3)) {
        return;
    }

    UINT16_T cmd_data_len = cmd->len - 3;

    BOOL_T   rsp_flag = TRUE;
    UINT8_T  rsp_data[256] = {0};
    UINT32_T rsp_len = 0;

    switch (cmd->group_id) {
    case TEST_GID_SYSTEM: {
        rsp_len = test_group_system(cmd->cmd_id, cmd->value, cmd_data_len, rsp_data);
    }
    break;

    case TEST_GID_DEVICE_INFO: {
        rsp_len = test_group_device_info(cmd->cmd_id, cmd->value, cmd_data_len, rsp_data);
    }
    break;

    case TEST_GID_ADV: {
        rsp_len = test_group_adv(cmd->cmd_id, cmd->value, cmd_data_len, rsp_data);
    }
    break;

    case TEST_GID_SCAN: {
        rsp_len = test_group_scan(cmd->cmd_id, cmd->value, cmd_data_len, rsp_data);
    }
    break;

    case TEST_GID_CONN: {
        rsp_len = test_group_conn(cmd->cmd_id, cmd->value, cmd_data_len, rsp_data);
    }
    break;

    case TEST_GID_DATA: {
        rsp_len = test_group_data(cmd->cmd_id, cmd->value, cmd_data_len, rsp_data);
    }
    break;

    case TEST_GID_ELSE: {
        rsp_len = test_group_else(cmd->cmd_id, cmd->value, cmd_data_len, rsp_data);
    }
    break;

    case TEST_GID_GPIO: {
        rsp_len = test_group_gpio(cmd->cmd_id, cmd->value, cmd_data_len, rsp_data);
    }
    break;

    case TEST_GID_UART: {
        rsp_len = test_group_uart(cmd->cmd_id, cmd->value, cmd_data_len, rsp_data);
    }
    break;

    case TEST_GID_PWM: {
        rsp_len = test_group_pwm(cmd->cmd_id, cmd->value, cmd_data_len, rsp_data);
    }
    break;

    case TEST_GID_ADC: {
        rsp_len = test_group_adc(cmd->cmd_id, cmd->value, cmd_data_len, rsp_data);
    }
    break;

    case TEST_GID_SPI: {
        rsp_len = test_group_spi(cmd->cmd_id, cmd->value, cmd_data_len, rsp_data);
    }
    break;

    case TEST_GID_IIC: {
        rsp_len = test_group_iic(cmd->cmd_id, cmd->value, cmd_data_len, rsp_data);
    }
    break;

    case TEST_GID_RTC: {
        rsp_len = test_group_rtc(cmd->cmd_id, cmd->value, cmd_data_len, rsp_data);
    }
    break;

    case TEST_GID_FLASH: {
        rsp_len = test_group_flash(cmd->cmd_id, cmd->value, cmd_data_len, rsp_data);
    }
    break;

    case TEST_GID_WATCHDOG: {
        rsp_len = test_group_watchdog(cmd->cmd_id, cmd->value, cmd_data_len, rsp_data);
    }
    break;

    case TEST_GID_POWERMANGER: {
        rsp_len = test_group_powermanger(cmd->cmd_id, cmd->value, cmd_data_len, rsp_data);
    }
    break;

    case TEST_GID_CUSTOM: {
        rsp_len = test_group_custom(cmd->cmd_id, cmd->value, cmd_data_len, rsp_data);
        rsp_flag = false;
    }
    break;

    default: {
    } break;
    }

    if (rsp_flag) {
        UINT16_T id = (cmd->group_id << 8) + cmd->cmd_id;
        test_cmd_send(id, rsp_data, rsp_len);
    }
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET test_cmd_send(UINT16_T cmdId, UINT8_T *buf, UINT16_T size)
{
    UINT32_T len = 4;

    tx_buffer[len++] = (size + 3) >> 8;
    tx_buffer[len++] = (size + 3) & 0xFF;

    tx_buffer[len++] = 0x03;

    tx_buffer[len++] = cmdId >> 8;
    tx_buffer[len++] = cmdId & 0xFF;

    if (size > 0) {
        memcpy(&tx_buffer[len], buf, size);
        len += size;
    }

    tx_buffer[len] = tal_util_check_sum8(tx_buffer, len);
    len += 1;

//    tuya_ble_production_test_asynchronous_response(0, tx_buffer, len);
    tuya_ble_common_uart_send_data(tx_buffer, len);

    if (cmdId != 0x0105 && !(cmdId == 0x3000 && buf[0] == 0x80)) {
        TAL_PR_HEXDUMP_INFO("test_rsp", tx_buffer + 7, len - 8);
    }

    return OPRT_OK;
}

VOID_T tal_sdk_test_get_time_rsp(tal_utc_date_t *date)
{
    UINT8_T  rsp_data[10] = {0};
    UINT32_T rsp_len = 9;

    rsp_data[0] = date->year;
    rsp_data[1] = date->month;
    rsp_data[2] = date->day;
    rsp_data[3] = date->hour;
    rsp_data[4] = date->min;
    rsp_data[5] = date->sec;
    rsp_data[6] = date->dayIndex;
    rsp_data[7] = tal_utc_get_time_zone() >> 8;
    rsp_data[8] = tal_utc_get_time_zone();

    test_cmd_send(TEST_ID_GET(TEST_GID_SYSTEM, TEST_CID_GET_TIME), rsp_data, rsp_len);
}

STATIC VOID_T tal_sdk_test_unbind_mode_rsp(UINT8_T mode)
{
    UINT8_T tmp_mode = mode;
    test_cmd_send(TEST_ID_GET(TEST_GID_CONN, TEST_CID_UNBIND_MODE), &tmp_mode, SIZEOF(UINT8_T));
}

VOID_T tal_sdk_test_enter_sleep_handler(TIMER_ID timer_id, VOID_T *arg)
{
#if defined(TUYA_BLE_FEATURE_LONG_RANGE) && (TUYA_BLE_FEATURE_LONG_RANGE == 1)
    if (test_param.enter_sleep.mode == 0) {
        tal_ble_advertising_stop();
        tal_ble_ext_advertising_stop(tal_ext_adv_extended_adv);
    } else if (test_param.enter_sleep.mode == 1) {
        tal_ble_ext_advertising_stop(tal_ext_adv_extended_adv);
    } else if (test_param.enter_sleep.mode == 2) {
        tal_ble_advertising_stop();
    }
#else
    if (test_param.enter_sleep.mode == 0) {
        tal_ble_advertising_stop();
    }
#endif

    tal_uart_deinit(TUYA_UART_NUM_0);

//    tal_oled_clear(); //It will prolong the time it takes to enter sleep
    tal_i2c_deinit(TUYA_I2C_NUM_0);

    tal_pwm_deinit(TUYA_PWM_NUM_0);
    tal_adc_deinit(TUYA_ADC_NUM_0);

    tal_watchdog_stop();

    tal_cpu_allow_sleep();

    test_param.enter_sleep.flag = TRUE;
}

STATIC VOID_T tal_sdk_test_adv_report_handler(TIMER_ID timer_id, VOID_T *arg)
{
    UINT8_T count[8] = {0};
    count[0] = sg_adv_report.count >> 24;
    count[1] = sg_adv_report.count >> 16;
    count[2] = sg_adv_report.count >> 8;
    count[3] = sg_adv_report.count;
    count[4] = sg_adv_report.rsp_count >> 24;
    count[5] = sg_adv_report.rsp_count >> 16;
    count[6] = sg_adv_report.rsp_count >> 8;
    count[7] = sg_adv_report.rsp_count;
    test_cmd_send(TEST_ID_GET(TEST_GID_SCAN, TEST_CID_ADV_REPORT), count, 8);
}

VOID_T tal_ble_sdk_test_wake_up_handler(VOID_T)
{
    if (test_param.enter_sleep.flag) {
        test_param.enter_sleep.flag = false;

        tal_cpu_force_wakeup();

        tal_uart_init(TUYA_UART_NUM_0, &tal_uart_cfg);

        TUYA_IIC_BASE_CFG_T iic_cfg = {
            .role = TUYA_IIC_MODE_MASTER,
            .speed = TUYA_IIC_BUS_SPEED_400K,
            .addr_width = TUYA_IIC_ADDRESS_7BIT,
        };
        tal_i2c_init(TUYA_I2C_NUM_0, &iic_cfg);
    }

    tal_ble_advertising_start(&tal_adv_param);
#if defined(TUYA_BLE_FEATURE_LONG_RANGE) && (TUYA_BLE_FEATURE_LONG_RANGE == 1)
    tal_ble_ext_advertising_start(tal_ext_adv_extended_adv);
#endif
}

VOID_T tal_sdk_test_mesh_data_write(UINT8_T dp_id, UINT8_T dp_type, UINT8_T *dp_data, UINT16_T dp_len)
{
}

STATIC VOID_T tal_sdk_test_uart_irq_rx_cb(TUYA_UART_NUM_E port_id, VOID_T *buff, UINT16_T len)
{
    if (uart_cache.rx_count + len <= TUYA_TEST_UART_CACHE_SIZE) {
        memcpy(&uart_cache.rx_buf[uart_cache.rx_count], buff, len);
        uart_cache.rx_count += len;

        if (uart_cache.rx_count < TUYA_TEST_UART_CACHE_SIZE) {
            tal_sw_timer_start(uart_cache.timer_id, 20, TAL_TIMER_ONCE);
        } else {
            tal_sw_timer_start(uart_cache.timer_id, 0, TAL_TIMER_ONCE);
        }
    } else {
        //If the buffer is exceeded, data will still be lost.
        //Double-buffering may solve the problem, but for practical purposes, larger buffer is a better solution.
    }
}

STATIC VOID_T tal_sdk_test_uart_timeout_handler(TIMER_ID timer_id, VOID_T *arg)
{
    if (uart_cache.rx_count > 0) {
        test_cmd_send(TEST_ID_GET(TEST_GID_UART, TEST_CID_RX_UART_PORT), (VOID_T *)&uart_cache.port_id, SIZEOF(UINT32_T));
        test_cmd_send(TEST_ID_GET(TEST_GID_UART, TEST_CID_RX_UART_DATA), uart_cache.rx_buf, uart_cache.rx_count);

        uart_cache.rx_count = 0;
    }
}

#endif /* 1 == TUYA_SDK_TEST_TYPE */

#endif /* TUYA_SDK_TEST */

