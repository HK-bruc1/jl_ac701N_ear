/**
 * @file tuya_ble_uart_common.c
 * @brief This is tuya_ble_uart_common file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "string.h"
#include "tal_util.h"
#include "tal_bluetooth.h"
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
#include "tuya_ble_uart_common.h"
#include "tuya_ble_log.h"
#include "tal_ble_file.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#if defined(TUYA_BLE_FEATURE_UART_COMMON_ENABLE) && (TUYA_BLE_FEATURE_UART_COMMON_ENABLE==1)

#define TUYA_BLE_UART_COMMON_MCU_OTA_DATA_LENGTH_MAX  1024

#define TUYA_BLE_OTA_MCU_TYPE 1

#define TUYA_BLE_UART_COMMON_MCU_OTA_REQUEST                    0xEA
#define TUYA_BLE_UART_COMMON_MCU_OTA_FILE_INFO                  0xEB
#define TUYA_BLE_UART_COMMON_MCU_OTA_FILE_OFFSET                0xEC
#define TUYA_BLE_UART_COMMON_MCU_OTA_DATA                       0xED
#define TUYA_BLE_UART_COMMON_MCU_OTA_END                        0xEE

#define TUYA_BLE_EXT_MODULE_OTA_REQUEST                         0xFA
#define TUYA_BLE_EXT_MODULE_OTA_FILE_INFO                       0xFB
#define TUYA_BLE_EXT_MODULE_OTA_FILE_OFFSET                     0xFC
#define TUYA_BLE_EXT_MODULE_OTA_DATA                            0xFD
#define TUYA_BLE_EXT_MODULE_OTA_END                             0xFE

#define TUYA_BLE_SEND_FILE_INFO                                 0xF5
#define TUYA_BLE_SEND_FILE_OFFSET                               0xF6
#define TUYA_BLE_SEND_FILE_DATA                                 0xF7
#define TUYA_BLE_SEND_FILE_END                                  0xF8

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
STATIC UINT16_T moudle_mcu_ota_max_dfu_length = TUYA_BLE_UART_COMMON_MCU_OTA_DATA_LENGTH_MAX;
STATIC UINT8_T *uart_data_buffer = NULL;

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_uart_common_mcu_ota_state_updata_handle(UINT8_T is_enter_mcu_ota)
{
    if (is_enter_mcu_ota) {
        // into product test mode
    } else {
        // exit product test mode
    }

    return TUYA_BLE_SUCCESS;
}

__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_common_mcu_ota_uart_send_data(UINT8_T *data, UINT16_T len)
{
    return tuya_ble_common_uart_send_data(data, len);
}

__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_common_uart_receive_ack_notify(UINT8_T cmd)
{
    return TUYA_BLE_SUCCESS;
}

STATIC tuya_ble_status_t tuya_ble_uart_common_mcu_ota_buffer_malloc(UINT32_T size)
{
    if (uart_data_buffer) {
        tuya_ble_free(uart_data_buffer);
    }
    uart_data_buffer = (UINT8_T *)tuya_ble_malloc(size);
    if (uart_data_buffer == NULL) {
        TUYA_BLE_LOG_ERROR("uart_data_buffer malloc failed.");
        return TUYA_BLE_ERR_NO_MEM;
    }
    return TUYA_BLE_SUCCESS;
}

STATIC tuya_ble_status_t tuya_ble_uart_common_mcu_ota_buffer_free(VOID_T)
{
    tuya_ble_free(uart_data_buffer);
    uart_data_buffer = NULL;
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_uart_common_mcu_ota_disconnect_handler(VOID_T)
{
    tuya_ble_uart_common_mcu_ota_buffer_free();
    return TUYA_BLE_SUCCESS;
}

VOID_T tuya_ble_uart_common_mcu_or_ext_module_ota_data_from_ble_handler(UINT8_T channel, UINT16_T cmd, UINT8_T *recv_data, UINT32_T recv_len)
{
    UINT16_T uart_data_len = 0;
    UINT8_T offset = 0;

    TUYA_BLE_LOG_DEBUG("cmd:%d,channel:%d", cmd, channel);

    if (cmd == FRM_OTA_START_REQ) {
        /*Allocate memory at the beginning of the OTA and release it at the end of the OTA*/
        if (tuya_ble_uart_common_mcu_ota_buffer_malloc(TUYA_BLE_UART_COMMON_MCU_OTA_DATA_LENGTH_MAX + 16) != TUYA_BLE_SUCCESS) {
            return;
        }
    }

    if (uart_data_buffer == NULL || (TUYA_BLE_UART_COMMON_MCU_OTA_DATA_LENGTH_MAX + 16) < (recv_len + 7)) {
        return;
    }

    uart_data_buffer[0] = 0x55;
    uart_data_buffer[1] = 0xAA;
    uart_data_buffer[2] = 0x00;

    if (channel >= 10 && channel <= 19) {
        uart_data_buffer[3] = TUYA_BLE_EXT_MODULE_OTA_REQUEST - TUYA_BLE_UART_COMMON_MCU_OTA_REQUEST;
        offset = 1;
        uart_data_buffer[6] = channel;
    } else {
        uart_data_buffer[3] = 0x00;
        offset = 0;
    }

    switch (cmd) {
    case FRM_OTA_START_REQ:
        uart_data_buffer[3] += TUYA_BLE_UART_COMMON_MCU_OTA_REQUEST;
        uart_data_buffer[4] = 0;
        uart_data_buffer[5] = 2 + offset;
        uart_data_buffer[6 + offset] = moudle_mcu_ota_max_dfu_length >> 8;
        uart_data_buffer[7 + offset] = (uint8_t)moudle_mcu_ota_max_dfu_length;
        uart_data_len = 8 + offset;
        tuya_ble_uart_common_mcu_ota_state_updata_handle(1);
        break;
    case FRM_OTA_FILE_INFOR_REQ:
        uart_data_buffer[3] += TUYA_BLE_UART_COMMON_MCU_OTA_FILE_INFO;
        uart_data_buffer[4] = 0;
        uart_data_buffer[5] = 35 + offset;
        memcpy(uart_data_buffer + 6 + offset, recv_data, 8);
        uart_data_buffer[14 + offset] = recv_data[9];
        uart_data_buffer[15 + offset] = recv_data[10];
        uart_data_buffer[16 + offset] = recv_data[11];
        memcpy(&uart_data_buffer[17 + offset], recv_data + 12, 24);
        uart_data_len = 41 + offset;
        break;
    case FRM_OTA_FILE_OFFSET_REQ:
        uart_data_buffer[3] += TUYA_BLE_UART_COMMON_MCU_OTA_FILE_OFFSET;
        uart_data_buffer[4] = 0;
        uart_data_buffer[5] = 4 + offset;
        memcpy(uart_data_buffer + 6 + offset, recv_data, 4);
        uart_data_len = 10 + offset;
        break;
    case FRM_OTA_DATA_REQ:
        uart_data_buffer[3] += TUYA_BLE_UART_COMMON_MCU_OTA_DATA;
        uart_data_buffer[4] = ((recv_len + offset) >> 8) & 0xff;
        uart_data_buffer[5] = (recv_len + offset);
        memcpy(uart_data_buffer + 6 + offset, recv_data, recv_len);
        uart_data_len = 6 + recv_len + offset;
        break;
    case FRM_OTA_END_REQ:
        uart_data_buffer[3] += TUYA_BLE_UART_COMMON_MCU_OTA_END;
        uart_data_buffer[4] = 0;
        uart_data_buffer[5] = 0 + offset;
        uart_data_len = 6 + offset;
        break;
    default:
        break;
    }

    uart_data_buffer[uart_data_len] = tal_util_check_sum8(uart_data_buffer, uart_data_len);
    uart_data_len++;

    tuya_ble_common_mcu_ota_uart_send_data(uart_data_buffer, uart_data_len);

    TUYA_BLE_LOG_DEBUG("CMD send:0x%02x", uart_data_buffer[3]);
}

STATIC VOID_T tuya_ble_uart_common_mcu_ota_data_from_uart_handler(UINT8_T cmd, UINT8_T *data_buffer, UINT16_T data_len)
{
    STATIC UINT8_T ble_data_buffer[30];
    STATIC UINT8_T ble_data_len = 0;
    UINT16_T mcu_max_dfu_len = 0;
    UINT16_T ble_cmd = 0;
    UINT8_T ota_channel_type = TUYA_BLE_OTA_MCU_TYPE;
    tuya_ble_connect_status_t currnet_connect_status;

    memset(ble_data_buffer, 0, SIZEOF(ble_data_buffer));
    ble_data_len = 0;

    if (cmd >= TUYA_BLE_EXT_MODULE_OTA_REQUEST && cmd <= TUYA_BLE_EXT_MODULE_OTA_END) {
        /*The extended firmware upgrade protocol has a 1-byte more channel number in the header than the MCU OTA protocol*/
        ota_channel_type = data_buffer[0];
        data_buffer = data_buffer + 1;
    }

    if (cmd == TUYA_BLE_UART_COMMON_MCU_OTA_END || cmd == TUYA_BLE_EXT_MODULE_OTA_END) {
        /*After receiving the MCU reply to the OTA end command, the upgrade can be considered completed to release the requested memory*/
        tuya_ble_uart_common_mcu_ota_buffer_free();
    }

    switch (cmd) {
    case TUYA_BLE_EXT_MODULE_OTA_REQUEST:
    case TUYA_BLE_UART_COMMON_MCU_OTA_REQUEST:
        ble_data_buffer[0] = data_buffer[0];
        ble_data_buffer[1] = 3;
        ble_data_buffer[2] = ota_channel_type;
        ble_data_buffer[3] = 0;
        mcu_max_dfu_len = (data_buffer[4] << 8) + data_buffer[5];
        if (moudle_mcu_ota_max_dfu_length < mcu_max_dfu_len) {
            data_buffer[4] = moudle_mcu_ota_max_dfu_length >> 8;
            data_buffer[5] = (uint8_t)moudle_mcu_ota_max_dfu_length;
        }
        memcpy(&ble_data_buffer[4], data_buffer + 1, 5);
        ble_data_len = 9;
        ble_cmd = FRM_OTA_START_RESP;
        break;
    case TUYA_BLE_EXT_MODULE_OTA_FILE_INFO:
    case TUYA_BLE_UART_COMMON_MCU_OTA_FILE_INFO:
        ble_data_buffer[0] = ota_channel_type;
        memcpy(&ble_data_buffer[1], data_buffer, 25);
        ble_data_len = 26;
        ble_cmd = FRM_OTA_FILE_INFOR_RESP;
        break;
    case TUYA_BLE_EXT_MODULE_OTA_FILE_OFFSET:
    case TUYA_BLE_UART_COMMON_MCU_OTA_FILE_OFFSET:
        ble_data_buffer[0] = ota_channel_type;
        memcpy(&ble_data_buffer[1], data_buffer, 4);
        ble_data_len = 5;
        ble_cmd = FRM_OTA_FILE_OFFSET_RESP;
        break;
    case TUYA_BLE_EXT_MODULE_OTA_DATA:
    case TUYA_BLE_UART_COMMON_MCU_OTA_DATA:
        ble_data_buffer[0] = ota_channel_type;
        ble_data_buffer[1] = data_buffer[0];
        ble_data_len = 2;
        ble_cmd = FRM_OTA_DATA_RESP;
        break;
    case TUYA_BLE_EXT_MODULE_OTA_END:
    case TUYA_BLE_UART_COMMON_MCU_OTA_END:
        ble_data_buffer[0] = ota_channel_type;
        ble_data_buffer[1] = data_buffer[0];
        ble_data_len = 2;
        ble_cmd = FRM_OTA_END_RESP;
        tuya_ble_uart_common_mcu_ota_state_updata_handle(0);
        break;
    default:
        break;
    };

    currnet_connect_status = tuya_ble_connect_status_get();

    if (currnet_connect_status != BONDING_CONN) {
        TUYA_BLE_LOG_ERROR("tuya_ble_uart_common_mcu_ota_process FAILED.");
        return;
    }

    if (ble_data_len > 0) {
        tuya_ble_comm_data_send(ble_cmd, 0, ble_data_buffer, ble_data_len, ENCRYPTION_MODE_SESSION_KEY);
    }
}

VOID_T tuya_ble_send_file_data_from_ble_handler(UINT8_T cmd, UINT8_T *p_data, UINT16_T data_len)
{
    UINT16_T uart_data_len = 0;
    UINT8_T offset = 0;

    TUYA_BLE_LOG_DEBUG("cmd:%d", cmd);

    if (cmd == TUYA_BLE_FILE_INFO) {
        /*Allocate memory at the beginning of the OTA and release it at the end of the OTA*/
        if (tuya_ble_uart_common_mcu_ota_buffer_malloc(TUYA_BLE_UART_COMMON_MCU_OTA_DATA_LENGTH_MAX + 16) != TUYA_BLE_SUCCESS) {
            return;
        }
    }

    if (uart_data_buffer == NULL || (TUYA_BLE_UART_COMMON_MCU_OTA_DATA_LENGTH_MAX + 16) < (data_len + 7)) {
        return;
    }

    uart_data_buffer[0] = 0x55;
    uart_data_buffer[1] = 0xAA;
    uart_data_buffer[2] = 0x00;

    switch (cmd) {
    case TUYA_BLE_FILE_INFO:
        uart_data_buffer[3] = TUYA_BLE_SEND_FILE_INFO;
        tuya_ble_uart_common_mcu_ota_state_updata_handle(1);
        break;
    case TUYA_BLE_FILE_OFFSET_REQ:
        uart_data_buffer[3] = TUYA_BLE_SEND_FILE_OFFSET;
        break;
    case TUYA_BLE_FILE_DATA:
        uart_data_buffer[3] = TUYA_BLE_SEND_FILE_DATA;
        break;
    case TUYA_BLE_FILE_END:
        uart_data_buffer[3] = TUYA_BLE_SEND_FILE_END;
        break;
    default:
        break;
    }

    uart_data_buffer[4] = ((data_len >> 8) & 0xff);
    uart_data_buffer[5] = (data_len & 0xff);

    uart_data_len = 6;

    memcpy(uart_data_buffer + 6, p_data, data_len);

    uart_data_len = uart_data_len + data_len;

    uart_data_buffer[uart_data_len] = tal_util_check_sum8(uart_data_buffer, uart_data_len);
    uart_data_len++;

    tuya_ble_common_mcu_ota_uart_send_data(uart_data_buffer, uart_data_len);

    TUYA_BLE_LOG_DEBUG("CMD send:0x%02x", uart_data_buffer[3]);
}

STATIC VOID_T tuya_ble_send_file_data_from_uart_handler(UINT8_T cmd, UINT8_T *p_data, UINT16_T data_len)
{
    tuya_ble_file_response_t file_response = {0};
    STATIC UINT8_T ble_data_buffer[30];
    STATIC UINT8_T ble_data_len = 0;
    UINT16_T mcu_max_dfu_len = 0;

    memset(ble_data_buffer, 0, SIZEOF(ble_data_buffer));
    ble_data_len = 0;

    if (cmd == TUYA_BLE_SEND_FILE_END) {
        /*After receiving the MCU reply to the OTA end command, the upgrade can be considered completed to release the requested memory*/
        tuya_ble_uart_common_mcu_ota_buffer_free();
    }

    memcpy(ble_data_buffer, p_data, data_len);
    ble_data_len = data_len;

    switch (cmd) {
    case TUYA_BLE_SEND_FILE_INFO:
        file_response.type = TUYA_BLE_FILE_INFO;
        mcu_max_dfu_len = (((UINT16_T)p_data[4] << 8) & 0xff00) | (p_data[5] & 0x00ff);
        if (mcu_max_dfu_len > 1024) {
            ble_data_buffer[4] = 0x04;
            ble_data_buffer[5] = 0x00;
        }
        break;
    case TUYA_BLE_SEND_FILE_OFFSET:
        file_response.type = TUYA_BLE_FILE_OFFSET_REQ;
        break;
    case TUYA_BLE_SEND_FILE_DATA:
        file_response.type = TUYA_BLE_FILE_DATA;
        break;
    case TUYA_BLE_SEND_FILE_END:
        file_response.type = TUYA_BLE_FILE_END;
        tuya_ble_uart_common_mcu_ota_state_updata_handle(0);
        break;
    default:
        file_response.type = TUYA_BLE_FILE_UNKONWN;
        break;
    }


    file_response.p_data = ble_data_buffer;
    file_response.data_len = ble_data_len;

    tuya_ble_file_response(&file_response);
}

__TUYA_BLE_WEAK VOID_T tuya_ble_custom_app_uart_common_process(UINT8_T *p_in_data, UINT16_T in_len)
{
    UINT8_T cmd = p_in_data[3];
    UINT16_T data_len = (p_in_data[4] << 8) + p_in_data[5];
    UINT8_T *data_buffer = p_in_data + 6;

    switch (cmd) {

    default:
        break;
    };
}

VOID_T tuya_ble_uart_common_process(UINT8_T *p_in_data, UINT16_T in_len)
{
    UINT8_T cmd = p_in_data[3];
    UINT16_T data_len = (p_in_data[4] << 8) + p_in_data[5];
    UINT8_T *data_buffer = p_in_data + 6;

    switch (cmd) {
    /*mcu extended module ota*/
    case TUYA_BLE_EXT_MODULE_OTA_REQUEST:
    case TUYA_BLE_EXT_MODULE_OTA_FILE_INFO:
    case TUYA_BLE_EXT_MODULE_OTA_FILE_OFFSET:
    case TUYA_BLE_EXT_MODULE_OTA_DATA:
    case TUYA_BLE_EXT_MODULE_OTA_END:
    /*mcu ota*/
    case TUYA_BLE_UART_COMMON_MCU_OTA_REQUEST:
    case TUYA_BLE_UART_COMMON_MCU_OTA_FILE_INFO:
    case TUYA_BLE_UART_COMMON_MCU_OTA_FILE_OFFSET:
    case TUYA_BLE_UART_COMMON_MCU_OTA_DATA:
    case TUYA_BLE_UART_COMMON_MCU_OTA_END:
        tuya_ble_common_uart_receive_ack_notify(cmd);
        tuya_ble_uart_common_mcu_ota_data_from_uart_handler(cmd, data_buffer, data_len);
        break;
    case TUYA_BLE_SEND_FILE_INFO:
    case TUYA_BLE_SEND_FILE_OFFSET:
    case TUYA_BLE_SEND_FILE_DATA:
    case TUYA_BLE_SEND_FILE_END:
        tuya_ble_common_uart_receive_ack_notify(cmd);
        tuya_ble_send_file_data_from_uart_handler(cmd, data_buffer, data_len);
    default:
        tuya_ble_custom_app_uart_common_process(p_in_data, in_len);
        break;
    };
}

#endif /* TUYA_BLE_FEATURE_UART_COMMON_ENABLE */

