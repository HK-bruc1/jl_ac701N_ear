/**
 * @file tal_local_timer.c
 * @brief This is tal_local_timer file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "system/includes.h"
/* #include "string.h" */
/* #include "stdio.h" */

#include "tal_util.h"
#include "tal_local_timer.h"
#include "tal_ble_app_passthrough.h"
#include "tal_memory.h"
#include "tal_sw_timer.h"
#include "tal_rtc.h"
#include "tal_utc.h"
#include "tal_sdk_test.h"

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
#include "tuya_ble_protocol_callback.h"

#if ( (TUYA_BLE_PROTOCOL_VERSION_HIGN>=4) && (TUYA_BLE_FEATURE_LOCAL_TIMER_ENABLE != 0) )

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
typedef enum {
    TAL_LOCAL_TIMER_CMD_NONE = 0,
    TAL_LOCAL_TIMER_CMD_SET,
    TAL_LOCAL_TIMER_CMD_DELETE,
    TAL_LOCAL_TIMER_CMD_GET,
    TAL_LOCAL_TIMER_CMD_GET_INFO,
    TAL_LOCAL_TIMER_CMD_UNKONWN,
} TAL_LOCAL_TIMER_CMD_E;

typedef enum {
    TAL_LOCAL_TIMER_TYPE_NONE = 0,
    TAL_LOCAL_TIMER_TYPE_GENERAL,
    TAL_LOCAL_TIMER_TYPE_RANDOM,
    TAL_LOCAL_TIMER_TYPE_CYCLE,
    TAL_LOCAL_TIMER_TYPE_COUNTDOWN,
    TAL_LOCAL_TIMER_TYPE_UNKONWN,
} TAL_LOCAL_TIMER_TYPE_E;

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
#pragma pack(1)
typedef struct {
    uint8_t  id;
    uint8_t  type;
    uint8_t  len;
    uint8_t *p_value;
} tal_local_timer_dp_data_t;

// set
typedef struct {
    uint8_t  cmd;
    uint8_t  type;
    uint16_t len;
    uint8_t  year; // low 2 numbers
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  loop;
    uint32_t timer_id;
    uint8_t  dp_data[];
} tal_local_timer_set_t;

typedef struct {
    uint8_t status;
} tal_local_timer_set_rsp_t;

typedef struct {
    uint8_t status;
    uint8_t reason;
    uint8_t current_number;
} tal_local_timer_set_rsp2_t;

// delete
typedef struct {
    uint8_t cmd;
    uint8_t type;
    uint32_t timer_id;
} tal_local_timer_delete_t;

typedef struct {
    uint8_t status;
} tal_local_timer_delete_rsp_t;

// get info
typedef struct {
    uint8_t cmd;
    uint8_t type;
} tal_local_timer_get_info_t;

typedef struct {
    uint8_t status;
    uint8_t current_number;
    uint8_t max_number;
} tal_local_timer_get_info_rsp_t;
#pragma pack()

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
static TIMER_ID sg_local_timer_id = NULL;

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/
static void tal_local_timer_parser_handler(TIMER_ID timer_id, void *arg);
static uint32_t tal_local_timer_rsp(uint8_t *buf, uint32_t size);




void tal_local_timer_parser_event(void)
{
    TIME_T timestamp = {0};
    tal_utc_date_t date = {0};

    tal_rtc_time_get(&timestamp);
    tal_utc_timestamp2date(timestamp, &date, false);

    TAL_PR_INFO("tal_local_timer_parser_handler");

    for (uint32_t idx = 0; idx < TAL_LOCAL_TIMER_MAX_NUM; idx++) {
        if (g_local_timer_info[idx].timer_id != 0) {
            TAL_PR_INFO("tal_local_timer_parser_handler: %d = %d", g_local_timer_info[idx].year, date.year);
            TAL_PR_INFO("tal_local_timer_parser_handler: %d = %d", g_local_timer_info[idx].month, date.month);
            TAL_PR_INFO("tal_local_timer_parser_handler: %d = %d", g_local_timer_info[idx].day, date.day);
            TAL_PR_INFO("tal_local_timer_parser_handler: %d = %d", g_local_timer_info[idx].hour, date.hour);
            TAL_PR_INFO("tal_local_timer_parser_handler: %d = %d", g_local_timer_info[idx].minute, date.min);
            if ((g_local_timer_info[idx].year == date.year - 2000)
                && (g_local_timer_info[idx].month == date.month)
                && (g_local_timer_info[idx].day == date.day)
                && (g_local_timer_info[idx].hour == date.hour)
                && (g_local_timer_info[idx].minute == date.min)) {

                uint32_t dp_len = g_local_timer_info[idx].len - 14;
                uint8_t *dp_buf = tal_malloc(dp_len);
                if (dp_buf != NULL) {
                    tal_local_timer_dp_item_read(TAL_LOCAL_TIMER_DP_START_ID + idx, dp_buf, dp_len);

#if defined(TUYA_SDK_TEST) && (TUYA_SDK_TEST == 1)
                    test_cmd_send(TEST_ID_GET(TEST_GID_ELSE, TEST_CID_GET_LOCAL_TIMER_PARSER), dp_buf, dp_len);
#endif
                    tal_free(dp_buf);
                }

                tal_local_timer_info_delete(g_local_timer_info[idx].timer_id);
            }
        }
    }
}

uint32_t tal_local_timer_init(void)
{
    uint32_t ret = OPRT_OK;

    ret = tal_local_timer_storage_init();
    ret = tal_sw_timer_create(tal_local_timer_parser_handler, NULL, &sg_local_timer_id);
    ret = tal_sw_timer_start(sg_local_timer_id, 15000, TAL_TIMER_CYCLE);

    return ret;
}

static uint32_t tal_local_timer_set_handler(uint8_t *buf, uint32_t size)
{
    tal_local_timer_set_t *set_info = (void *)buf;
    tal_local_timer_set_rsp_t set_rsp = {0};
    tal_local_timer_set_rsp2_t set_rsp2 = {0};

    tal_local_timer_info_t info = {0};

    tal_util_reverse_byte(&set_info->len, sizeof(uint16_t));
    tal_util_reverse_byte(&set_info->timer_id, sizeof(uint32_t));

    memcpy(&info, &set_info->type, sizeof(tal_local_timer_info_t) - 8);
    info.dp_data = tal_malloc(size - 18);
    if (info.dp_data == NULL) {
        return OPRT_MALLOC_FAILED;
    }
    memcpy(info.dp_data, set_info->dp_data, set_info->len - 4 - 14);
    info.crc32 = (buf[size - 4] << 24) + (buf[size - 3] << 16) + (buf[size - 2] << 8) + (buf[size - 1]);
    info.len = set_info->len - 4;

    uint32_t ret = tal_local_timer_info_write(&info);
    TAL_PR_INFO("local timer ret: %d", ret);
    if (ret == 2) {
        set_rsp.status = 0x00;
        tal_local_timer_rsp((void *)&set_rsp, sizeof(tal_local_timer_set_rsp_t));
    } else if (ret == OPRT_EXCEED_UPPER_LIMIT) {
        set_rsp2.status = 0x01;
        set_rsp2.reason = 0x01;
        set_rsp2.current_number = TAL_LOCAL_TIMER_MAX_NUM;
        tal_local_timer_rsp((void *)&set_rsp2, sizeof(tal_local_timer_set_rsp2_t));
    }

    if (info.dp_data != NULL) {
        tal_free(info.dp_data);
    }

    return OPRT_OK;
}

static uint32_t tal_local_timer_delete_handler(uint8_t *buf, uint32_t size)
{
    tal_local_timer_delete_t *delete_info = (void *)buf;
    tal_local_timer_delete_rsp_t delete_rsp = {0};

    tal_util_reverse_byte(&delete_info->timer_id, sizeof(uint32_t));

    tal_local_timer_info_delete(delete_info->timer_id);

    delete_rsp.status = 0x00;

    return tal_local_timer_rsp((void *)&delete_rsp, sizeof(tal_local_timer_delete_rsp_t));
}

static uint32_t tal_local_timer_get_handler(uint8_t *buf, uint32_t size)
{
    uint32_t local_timer_num = tal_local_timer_get_num();
    uint32_t crc32 = 0;

    uint32_t rsp_len = 3;
    uint8_t *rsp_buf = tal_malloc(rsp_len + local_timer_num * 8);
    if (rsp_buf != NULL) {
        for (uint32_t idx = 0; idx < TAL_LOCAL_TIMER_MAX_NUM; idx++) {
            if (g_local_timer_info[idx].timer_id != 0) {

                crc32 = 0;

                tal_util_reverse_byte(&g_local_timer_info[idx].timer_id, sizeof(uint32_t));
                crc32 = tal_util_crc32(&g_local_timer_info[idx].hour, 7, &crc32);
                tal_util_reverse_byte(&g_local_timer_info[idx].timer_id, sizeof(uint32_t));

                uint32_t dp_len = g_local_timer_info[idx].len - 14;
                uint8_t *dp_buf = tal_malloc(dp_len);
                if (dp_buf != NULL) {
                    tal_local_timer_dp_item_read(TAL_LOCAL_TIMER_DP_START_ID + idx, dp_buf, dp_len);

                    crc32 = tal_util_crc32(dp_buf, dp_len, &crc32);

                    tal_free(dp_buf);
                }

                rsp_buf[rsp_len++] = (g_local_timer_info[idx].timer_id >> 24) & 0xFF;
                rsp_buf[rsp_len++] = (g_local_timer_info[idx].timer_id >> 16) & 0xFF;
                rsp_buf[rsp_len++] = (g_local_timer_info[idx].timer_id >> 8) & 0xFF;
                rsp_buf[rsp_len++] = (g_local_timer_info[idx].timer_id >> 0) & 0xFF;

                rsp_buf[rsp_len++] = (crc32 >> 24) & 0xFF;
                rsp_buf[rsp_len++] = (crc32 >> 16) & 0xFF;
                rsp_buf[rsp_len++] = (crc32 >> 8) & 0xFF;
                rsp_buf[rsp_len++] = (crc32 >> 0) & 0xFF;
            }
        }

        rsp_buf[0] = 0x00; // status
        rsp_buf[1] = local_timer_num; // num
        rsp_buf[2] = local_timer_num * 8; // len
    }

    uint32_t ret = tal_local_timer_rsp(rsp_buf, rsp_len);

    if (rsp_buf) {
        tal_free(rsp_buf);
    }

    return ret;
}

static uint32_t tal_local_timer_get_info_handler(uint8_t *buf, uint32_t size)
{
//    tal_local_timer_get_info_t* get_info = (void*)buf;
    tal_local_timer_get_info_rsp_t get_info_rsp = {0};

    get_info_rsp.status = 0x00;

    return tal_local_timer_rsp((void *)&get_info_rsp, sizeof(tal_local_timer_get_info_rsp_t));
}

uint32_t tal_local_timer_handler(tuya_ble_app_passthrough_data_t *data)
{
    uint8_t cmd = data->p_data[0];

#if defined(TUYA_SDK_TEST) && (TUYA_SDK_TEST == 1)
    test_cmd_send(TEST_ID_GET(TEST_GID_ELSE, TEST_CID_GET_LOCAL_TIMER), data->p_data, data->data_len);
#endif

    TAL_PR_INFO("local timer cmd: %d", cmd);
    TAL_PR_HEXDUMP_INFO("local timer data", data->p_data + 1, data->data_len - 1);

    switch (cmd) {
    case TAL_LOCAL_TIMER_CMD_SET: {
        tal_local_timer_set_handler(data->p_data, data->data_len);
    }
    break;

    case TAL_LOCAL_TIMER_CMD_DELETE: {
        tal_local_timer_delete_handler(data->p_data, data->data_len);
    }
    break;

    case TAL_LOCAL_TIMER_CMD_GET: {
        tal_local_timer_get_handler(data->p_data, data->data_len);
    }
    break;

    case TAL_LOCAL_TIMER_CMD_GET_INFO: {
        tal_local_timer_get_info_handler(data->p_data, data->data_len);
    }
    break;

    default: {
    } break;
    }

    return 0;
}

static uint32_t tal_local_timer_rsp(uint8_t *buf, uint32_t size)
{
#if defined(TUYA_SDK_TEST) && (TUYA_SDK_TEST == 1)
    test_cmd_send(TEST_ID_GET(TEST_GID_ELSE, TEST_CID_GET_LOCAL_TIMER_RSP), buf, size);
#endif

    TAL_PR_HEXDUMP_INFO("local timer rsp data", buf, size);

    tuya_ble_app_passthrough_data_t rsp = {0};
    rsp.type = TAL_BLE_APP_PASSTHROUGH_SUBCMD_LOCAL_TIMER;
    rsp.data_len = size;
    rsp.p_data = buf;
    return tal_ble_app_passthrough_data_send(&rsp);
}

static void tal_local_timer_parser_handler(TIMER_ID timer_id, void *arg)
{
    tuya_ble_custom_evt_send(APP_EVT_0);
}

#else

uint32_t tal_local_timer_handler(tuya_ble_app_passthrough_data_t *data)
{
    return 0;
}

#endif // TUYA_BLE_FEATURE_LOCAL_TIMER_ENABLE

