/**
 * @file tuya_ble_bulkdata_demo.c
 * @brief This is tuya_ble_bulkdata_demo file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "string.h"

#include "vendor/jl_ble/tuyaos/include/board.h"

#include "tal_log.h"
#include "tal_rtc.h"
#include "tal_utc.h"
#include "tal_util.h"
#include "tal_bluetooth.h"
#include "tuya_sdk_callback.h"
#include "tuya_ble_protocol_callback.h"

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
#include "tuya_ble_log.h"
#include "tuya_ble_internal_config.h"
#include "tuya_ble_bulkdata.h"

#include "tuya_ble_bulkdata_demo.h"

#if defined(TUYA_BLE_FEATURE_BULKDATA_ENABLE) && (TUYA_BLE_FEATURE_BULKDATA_ENABLE == 1)

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define TEST_BULK_DATA_TYPE1_START_ADDR             BOARD_FLASH_SDK_TEST_START_ADDR
#define TEST_BULK_DATA_TYPE1_END_ADDR               (TEST_BULK_DATA_TYPE1_START_ADDR + (TUYA_NV_ERASE_MIN_SIZE*4))

#if ((TEST_BULK_DATA_TYPE1_START_ADDR>=TUYA_NV_START_ADDR) || (TEST_BULK_DATA_TYPE1_END_ADDR>TUYA_NV_START_ADDR))
#error "Storage Memory overflow!"
#endif

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
#pragma pack(1)
typedef struct {
    UINT32_T total_blocks;
    UINT8_T *block_buf;
} TUYA_BLE_BULKDATA_PARAM_T;
#pragma pack()

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
UINT8_T TLD_Example_Data[64] = {
    0x01,                                                                            /* TYPE:1 */
    0x00, 0x3D,                                                                      /* LENGTH: 0x003D = 61*/
    0x5F, 0xC0, 0xAF, 0xDD,                                                          /* UNIX TIME: 0x5FC0AFDD = 1606463453 */
    0x14, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x0A,                                  /* dp id: 20; dp type : value; dp len : 4; dp data : 10 */
    0x15, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x64,                                  /* dp id: 21; dp type : value; dp len : 4; dp data : 100 */
    0x16, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x28,                                  /* dp id: 22; dp type : value; dp len : 4; dp data : 40 */
    0x18, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x32,                                  /* dp id: 24; dp type : value; dp len : 4; dp data : 50 ����*/
    0x19, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x46,                                  /* dp id: 25; dp type : value; dp len : 4; dp data : 70 */
    0x69, 0x00, 0x00, 0x0D, 0x14, 0x09, 0x04, 0x01, 0x0e, 0x00, 0x02, 0x0f, 0x00, 0x00, 0x10, 0x00, /* dp id: 105; dp type : raw; dp len : 18; dp data : raw data */
    0x01,
};

STATIC TUYA_BLE_BULKDATA_PARAM_T sg_bulk_param = {0};
STATIC TUYA_BLE_BULKDATA_INFO_T sg_bulk_info = {0};
STATIC TUYA_BLE_BULKDATA_EXTERNAL_PARAM_T *sg_bulk_external_param[TUYA_BLE_BULKDATA_TYPE_NUM] = {0};
STATIC TUYA_BLE_BULKDATA_CB_T *tuya_ble_bulkdata_cb[TUYA_BLE_BULKDATA_TYPE_NUM] = {0};

STATIC UINT16_T current_block_number = 0;
STATIC UINT16_T current_block_size = 0;

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




UINT32_T tuya_ble_bulk_data_init(TUYA_BLE_BULKDATA_EXTERNAL_PARAM_T *param, TUYA_BLE_BULKDATA_CB_T *cb)
{
    UINT8_T idx = param->type - 1;

    sg_bulk_external_param[idx] = param;
    tuya_ble_bulkdata_cb[idx] = cb;

    return OPRT_OK;
}

UINT32_T tuya_ble_bulk_data_generation(UINT32_T timestep, UINT8_T *buf, UINT32_T size)
{
    UINT64_T data_len = SIZEOF(TLD_Example_Data);
    UINT32_T dp_time = 0;

    tal_rtc_time_get(&dp_time);

    if (buf != NULL) {
        memcpy(TLD_Example_Data, buf, size);
        data_len = size;
        dp_time = (buf[3] << 24) + (buf[4] << 16) + (buf[5] << 8) + buf[6];
    }

    if (dp_time == 0) {
        tal_rtc_time_get(&dp_time);
    }

    UINT32_T write_addr = TEST_BULK_DATA_TYPE1_START_ADDR;
    sg_bulk_info.total_length = 0;
    sg_bulk_info.total_crc32 = 0;

    while (write_addr < TEST_BULK_DATA_TYPE1_END_ADDR) {

        if (write_addr % TUYA_NV_ERASE_MIN_SIZE == 0) {
            if (tuya_ble_nv_erase(write_addr, TUYA_NV_ERASE_MIN_SIZE) != TUYA_BLE_SUCCESS) {
                TAL_PR_ERR("tuya_ble_bulk_data_generation ERROR!");
                sg_bulk_info.total_length = 0;
                sg_bulk_info.total_crc32 = 0;
                return 1;
            }
        }

        TLD_Example_Data[3] = dp_time >> 24;
        TLD_Example_Data[4] = dp_time >> 16;
        TLD_Example_Data[5] = dp_time >> 8;
        TLD_Example_Data[6] = dp_time;

        if (tuya_ble_nv_write(write_addr, TLD_Example_Data, data_len) != TUYA_BLE_SUCCESS) {
            TAL_PR_ERR("tuya_ble_bulk_data_generation ERROR!");
            sg_bulk_info.total_length = 0;
            sg_bulk_info.total_crc32 = 0;
            return 1;
        }

        write_addr += data_len;
        sg_bulk_info.total_length += data_len;
        sg_bulk_info.total_crc32 = tal_util_crc32(TLD_Example_Data, data_len, &sg_bulk_info.total_crc32);
        dp_time -= timestep;

//        tuya_ble_device_delay_ms(10);
    }

    sg_bulk_param.total_blocks = (sg_bulk_info.total_length + sg_bulk_info.block_length - 1) / sg_bulk_info.block_length;

    TAL_PR_DEBUG("tuya_ble_bulk_data_generation OK, TOTAL LENGTH = %d , TOTAL CRC32 = 0x%08x, BLOCK SIZE = %d , BLOCK NUMBERS = %d , TIME = %d",
                 sg_bulk_info.total_length, sg_bulk_info.total_crc32, sg_bulk_info.block_length, sg_bulk_param.total_blocks, dp_time);

    return 0;
}

VOID_T tuya_ble_bulk_data_demo_handler(tuya_ble_bulk_data_request_t *p_data)
{
    UINT8_T rsp_flag = 1;
    tuya_ble_bulk_data_response_t res_data = {0};

    res_data.evt = p_data->evt;

    if (p_data->bulk_type - 1 >= TUYA_BLE_BULKDATA_TYPE_NUM) {
        TAL_PR_ERR("Invalid bulk type = %d ", p_data->bulk_type);
    }

    UINT8_T idx = p_data->bulk_type - 1;

//    TAL_PR_INFO("p_data->evt: %d", p_data->evt);
//    TAL_PR_HEXDUMP_INFO("p_data", (void*)p_data, sizeof(tuya_ble_bulk_data_request_t));

    switch (p_data->evt) {
    case TUYA_BLE_BULK_DATA_EVT_READ_INFO: {
        if (p_data->bulk_type != sg_bulk_external_param[idx]->type) {
            res_data.bulk_type = p_data->bulk_type;
            res_data.params.bulk_info_res_data.status = 1;  //Invalid type value.
            TAL_PR_ERR("Invalid bulk type = %d ", p_data->bulk_type);
        } else {
            if (tuya_ble_bulkdata_cb[idx]->info_cb != NULL) {
                tuya_ble_bulkdata_cb[idx]->info_cb(&sg_bulk_info);

                if (sg_bulk_info.block_length >= TUYA_BLE_SEND_MAX_DATA_LEN) {
                    sg_bulk_info.block_length = sg_bulk_info.block_length;
                }

                sg_bulk_param.total_blocks = (sg_bulk_info.total_length + sg_bulk_info.block_length - 1) / sg_bulk_info.block_length;
            }

            if (sg_bulk_info.total_length != 0) {
                TAL_BLE_PEER_INFO_T peer_info = {0};
                peer_info.conn_handle = tuya_app_get_conn_handle();
                TAL_BLE_CONN_PARAMS_T conn_param = {0};
                conn_param.min_conn_interval = 15 * 4 / 5;
                conn_param.max_conn_interval = 15 * 4 / 5;
                conn_param.latency = 0;
                conn_param.conn_sup_timeout = 6000 / 10;
                conn_param.connection_timeout = 0;
                tal_ble_conn_param_update(peer_info, &conn_param);
            }

            res_data.bulk_type = p_data->bulk_type;
            res_data.params.bulk_info_res_data.status = 0;
            res_data.params.bulk_info_res_data.flag = sg_bulk_external_param[idx]->flag;
            res_data.params.bulk_info_res_data.bulk_data_length = sg_bulk_info.total_length;
            res_data.params.bulk_info_res_data.bulk_data_crc = sg_bulk_info.total_crc32;
            res_data.params.bulk_info_res_data.block_data_length = sg_bulk_info.block_length;
        }
    }
    break;

    case TUYA_BLE_BULK_DATA_EVT_READ_BLOCK: {
        if (p_data->bulk_type != sg_bulk_external_param[idx]->type) {
            res_data.bulk_type = p_data->bulk_type;
            res_data.params.block_res_data.status = 1;
            TAL_PR_ERR("Invalid bulk type = %d ", p_data->bulk_type);
        } else {
            current_block_number = p_data->params.block_data_req_data.block_number;

            res_data.bulk_type = p_data->bulk_type;
            if (current_block_number > (sg_bulk_param.total_blocks - 1)) {
                res_data.params.block_res_data.status = 2;
                TAL_PR_ERR("Invalid bulk block number,received block number = %d total blocks = %d", current_block_number, sg_bulk_param.total_blocks);
            } else {
                res_data.params.block_res_data.status = 0;
                res_data.params.block_res_data.block_number = current_block_number;

                current_block_size = (current_block_number < (sg_bulk_param.total_blocks - 1)) ?
                                     sg_bulk_info.block_length : (sg_bulk_info.total_length - current_block_number * sg_bulk_info.block_length); //The data length of the last block may be less than TEST_BULK_DATA_TYPE0_BLOCK_SIZE.

                res_data.params.block_res_data.block_data_length = current_block_size;
                res_data.params.block_res_data.max_packet_data_length = sg_bulk_info.block_length;

                if (tuya_ble_bulkdata_cb[idx]->report_cb != NULL) {
                    UINT8_T *p_buf = (UINT8_T *)tuya_ble_malloc(current_block_size);
                    if (p_buf) {
                        tuya_ble_bulkdata_cb[idx]->report_cb(p_buf, current_block_size, current_block_number);

                        res_data.params.block_res_data.block_data_crc16 = tal_util_crc16(p_buf, current_block_size, NULL);

                        sg_bulk_param.block_buf = p_buf;
                    }
                }

                TAL_PR_DEBUG("Read block data : block number = %d, block size = %d ,block data crc16 = 0x%04x",
                             current_block_number, current_block_size, res_data.params.block_res_data.block_data_crc16);
            }
        }
    }
    break;

    case TUYA_BLE_BULK_DATA_EVT_SEND_DATA: {
        //The bulk data type to be read is not the type specified by the 'TUYA_BLE_BULK_DATA_EVT_READ_INFO' event.
        if (p_data->bulk_type != sg_bulk_external_param[idx]->type) {
            TAL_PR_ERR("Invalid bulk type = %d , not start send bulk data.", p_data->bulk_type);
        } else {
            if (current_block_size == 0) {
                TAL_PR_ERR("The current_block_total_packets is 0,not start send bulk data.");
                rsp_flag = 0;
            } else {
                res_data.bulk_type = p_data->bulk_type;

                if (current_block_size > sg_bulk_info.block_length) {
                    current_block_size = sg_bulk_info.block_length;
                }

                res_data.params.send_res_data.current_block_number = current_block_number;
                res_data.params.send_res_data.current_block_length = current_block_size;
                res_data.params.send_res_data.p_current_block_data = sg_bulk_param.block_buf;

                if (current_block_number == (sg_bulk_param.total_blocks - 1)) {
                    TAL_BLE_PEER_INFO_T peer_info = {0};
                    peer_info.conn_handle = tuya_app_get_conn_handle();
                    TAL_BLE_CONN_PARAMS_T conn_param = {0};
                    conn_param.min_conn_interval = TY_CONN_INTERVAL_MIN * 4 / 5;
                    conn_param.max_conn_interval = TY_CONN_INTERVAL_MAX * 4 / 5;
                    conn_param.latency = 0;
                    conn_param.conn_sup_timeout = 6000 / 10;
                    conn_param.connection_timeout = 0;
                    tal_ble_conn_param_update(peer_info, &conn_param);
                }
            }
        }
    }
    break;

    case TUYA_BLE_BULK_DATA_EVT_ERASE: {
        if (p_data->bulk_type != sg_bulk_external_param[idx]->type) {
            res_data.bulk_type = p_data->bulk_type;
            res_data.params.erase_res_data.status = 1;
        } else {
            res_data.bulk_type = p_data->bulk_type;

            if (tuya_ble_bulkdata_cb[idx]->erase_cb != NULL) {
                tuya_ble_bulkdata_cb[idx]->erase_cb(p_data->bulk_type, &res_data.params.erase_res_data.status);
            }

            sg_bulk_info.total_length = 0;
            sg_bulk_param.total_blocks = 0;
        }
    }
    break;

    default: {
        rsp_flag = 0;
    }
    break;
    };

    if (rsp_flag) {
        tuya_ble_bulk_data_response(&res_data);

        if (res_data.evt == TUYA_BLE_BULK_DATA_EVT_SEND_DATA) {
            tuya_ble_free(res_data.params.send_res_data.p_current_block_data);
            TAL_PR_DEBUG("Send block data: block number = %d, block size = %d",
                         res_data.params.send_res_data.current_block_number, res_data.params.send_res_data.current_block_length, res_data.params.send_res_data);
        }
    }
}

#endif // TUYA_BLE_FEATURE_BULKDATA_ENABLE

