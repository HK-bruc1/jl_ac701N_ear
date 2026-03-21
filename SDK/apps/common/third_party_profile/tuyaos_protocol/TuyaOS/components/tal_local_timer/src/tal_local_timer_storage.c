/**
 * @file tal_local_timer_storage.c
 * @brief This is tal_local_timer_storage file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "system/includes.h"
/* #include "string.h" */
/* #include "stdio.h" */

#include "vendor/jl_ble/tuyaos/include/board.h"

#include "tal_util.h"
#include "tal_local_timer.h"
#include "tal_ble_app_passthrough.h"
#include "tal_sdk_test.h"
#include "tal_memory.h"

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

#if ( (TUYA_BLE_PROTOCOL_VERSION_HIGN>=4) && (TUYA_BLE_FEATURE_LOCAL_TIMER_ENABLE != 0) )
#include "tbs_storage_ib.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define TAL_LOCAL_TIMER_INFO_BLOCK_IDX  0
#define TAL_LOCAL_TIMER_DP_BLOCK_IDX    1

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
static TBS_STORAGE_IB_BLOCK_T sg_storage_ib_block[2] = {
    {
        .offset = 0,
        .size = 1024,
        .item_size = sizeof(tal_local_timer_info_t),
    },
    {
        .offset = 1024,
        .size = 4096 - 1024,
        .item_size = 0,
    },
};

tal_local_timer_info_t g_local_timer_info[TAL_LOCAL_TIMER_MAX_NUM] = {0};

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/
static uint32_t tal_local_timer_info_load(void);




uint32_t tal_local_timer_storage_init(void)
{
    uint32_t ret = OPRT_OK;

    ret += tbs_storage_ib_init(BOARD_FLASH_SDK_TEST_START_ADDR, BOARD_FLASH_SDK_TEST_START_ADDR + 0x1000, sg_storage_ib_block, 2);
//    tbs_storage_ib_clear();
    ret += tal_local_timer_info_load();

    return ret;
}

static uint32_t tal_local_timer_info_item_write(uint8_t id, uint8_t *buf, uint32_t size)
{
    return tbs_storage_ib_insert_item(TAL_LOCAL_TIMER_INFO_BLOCK_IDX, id, size, buf);
}

static uint32_t tal_local_timer_info_item_read(uint8_t id, uint8_t *buf, uint32_t size)
{
    return tbs_storage_ib_query_item(TAL_LOCAL_TIMER_INFO_BLOCK_IDX, id, size, buf);
}

static uint32_t tal_local_timer_info_item_delete(uint8_t id)
{
    return tbs_storage_ib_delete_item(TAL_LOCAL_TIMER_INFO_BLOCK_IDX, id);
}

static uint32_t tal_local_timer_dp_item_write(uint8_t id, uint8_t *buf, uint32_t size)
{
    return tbs_storage_ib_insert_item(TAL_LOCAL_TIMER_DP_BLOCK_IDX, id, size, buf);
}

uint32_t tal_local_timer_dp_item_read(uint8_t id, uint8_t *buf, uint32_t size)
{
    return tbs_storage_ib_query_item(TAL_LOCAL_TIMER_DP_BLOCK_IDX, id, size, buf);
}

static uint32_t tal_local_timer_dp_item_delete(uint8_t id)
{
    return tbs_storage_ib_delete_item(TAL_LOCAL_TIMER_DP_BLOCK_IDX, id);
}

static uint8_t tal_local_timer_search_idx(uint32_t timer_id)
{
    for (uint32_t idx = 0; idx < TAL_LOCAL_TIMER_MAX_NUM; idx++) {
        if (g_local_timer_info[idx].timer_id == timer_id) {
            return idx;
        }
    }
    return TAL_LOCAL_TIMER_MAX_NUM;
}

uint32_t tal_local_timer_info_write(tal_local_timer_info_t *data)
{
    uint32_t ret = OPRT_OK;

    uint8_t idx = tal_local_timer_search_idx(data->timer_id);
    if (idx == TAL_LOCAL_TIMER_MAX_NUM) { // new
        uint32_t idy = 0;

        for (idy = 0; idy < TAL_LOCAL_TIMER_MAX_NUM; idy++) {
            if (g_local_timer_info[idy].timer_id == 0) {
                memcpy(&g_local_timer_info[idy].type, data, sizeof(tal_local_timer_info_t));

                ret += tal_local_timer_info_item_write(TAL_LOCAL_TIMER_INFO_START_ID + idy, (void *)data, sizeof(tal_local_timer_info_t));
                if (ret == 1) {
                    ret += tal_local_timer_dp_item_write(TAL_LOCAL_TIMER_DP_START_ID + idy, data->dp_data, data->len - 14);
                    break;
                }
            }
        }

        if (idy == TAL_LOCAL_TIMER_MAX_NUM) {
            // full
            return OPRT_EXCEED_UPPER_LIMIT;
        }
    } else { // replace
        memcpy(&g_local_timer_info[idx].type, data, sizeof(tal_local_timer_info_t));

        ret += tal_local_timer_info_item_write(TAL_LOCAL_TIMER_INFO_START_ID + idx, (void *)data, sizeof(tal_local_timer_info_t));
        if (ret == 1) {
            ret += tal_local_timer_dp_item_write(TAL_LOCAL_TIMER_DP_START_ID + idx, data->dp_data, data->len - 14);
        }
    }

    return ret;
}

uint32_t tal_local_timer_info_delete(uint32_t timer_id)
{
    uint32_t ret = OPRT_OK;

    for (uint32_t idx = 0; idx < TAL_LOCAL_TIMER_MAX_NUM; idx++) {
        if (g_local_timer_info[idx].timer_id == timer_id) {
            ret += tal_local_timer_info_item_delete(TAL_LOCAL_TIMER_INFO_START_ID + idx);
            if (ret == 1) {
                ret += tal_local_timer_dp_item_delete(TAL_LOCAL_TIMER_DP_START_ID + idx);
                memset(&g_local_timer_info[idx].type, 0x00, sizeof(tal_local_timer_info_t));
                break;
            }
        }
    }

    return ret;
}

uint32_t tal_local_timer_info_delete_all(void)
{
    uint32_t ret = OPRT_OK;

    for (uint32_t idx = 0; idx < TAL_LOCAL_TIMER_MAX_NUM; idx++) {
        ret += tal_local_timer_info_item_delete(TAL_LOCAL_TIMER_INFO_START_ID + idx);
        if (ret == 1) {
            ret += tal_local_timer_dp_item_delete(TAL_LOCAL_TIMER_DP_START_ID + idx);
        }
        memset(&g_local_timer_info[idx].type, 0x00, sizeof(tal_local_timer_info_t));
    }

    return ret;
}

uint32_t tal_local_timer_get_num(void)
{
    uint32_t num = 0;

    for (uint32_t idx = 0; idx < TAL_LOCAL_TIMER_MAX_NUM; idx++) {
        if (g_local_timer_info[idx].timer_id != 0) {
            num++;
        }
    }

    return num;
}

static uint32_t tal_local_timer_info_load(void)
{
    uint32_t ret = OPRT_OK;

    for (uint32_t idx = 0; idx < TAL_LOCAL_TIMER_MAX_NUM; idx++) {
        ret += tal_local_timer_info_item_read(TAL_LOCAL_TIMER_INFO_START_ID + idx, &g_local_timer_info[idx].type, sizeof(tal_local_timer_info_t));
        TAL_PR_HEXDUMP_INFO("g_local_timer_info", &g_local_timer_info[idx].type, sizeof(tal_local_timer_info_t));
    }

    return ret;
}

#endif // TUYA_BLE_FEATURE_LOCAL_TIMER_ENABLE

