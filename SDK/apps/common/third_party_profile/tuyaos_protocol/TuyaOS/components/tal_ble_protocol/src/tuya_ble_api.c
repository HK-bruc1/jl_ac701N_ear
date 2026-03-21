/**
 * @file tuya_ble_api.c
 * @brief This is tuya_ble_api file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "string.h"

#include "tal_util.h"

#include "tuya_ble_port.h"
#include "tuya_ble_api.h"
#include "tuya_ble_type.h"
#include "tuya_ble_main.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_log.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_internal_config.h"
#include "tuya_ble_mutli_tsf_protocol.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_sdk_version.h"
#include "tuya_ble_gatt_send_queue.h"
#include "tuya_ble_event.h"
#if defined(TUYA_BLE_FEATURE_PRODUCT_TEST_ENABLE) && (TUYA_BLE_FEATURE_PRODUCT_TEST_ENABLE == 1)
#include "tuya_ble_product_test.h"
#endif

#if ENABLE_BLUETOOTH_BREDR
#include "tal_bluetooth_bredr.h"
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
typedef enum {
    TUYA_BLE_UART_REV_STATE_FOUND_NULL,
    TUYA_BLE_UART_REV_STATE_FOUND_HEAD,
    TUYA_BLE_UART_REV_STATE_FOUND_CMD,
    TUYA_BLE_UART_REV_STATE_FOUND_LEN_H,
    TUYA_BLE_UART_REV_STATE_FOUND_LEN_L,
    TUYA_BLE_UART_REV_STATE_FOUND_DATA,
    TUYA_BLE_UART_REV_STATE_UNKOWN,
} tuya_ble_uart_rev_state_t;

#if (TUYA_BLE_UART_RX_BUFFER_MAX<=32)
#error "Wrong settings,please check."
#endif

#define TUYA_BLE_UART_RX_DATA_LEN_MAX (TUYA_BLE_UART_RX_BUFFER_MAX-32)

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
STATIC UINT8_T m_callback_numbers = 0;

#if TUYA_BLE_USE_OS
extern UINT32_T m_cb_queue_table[TUYA_BLE_MAX_CALLBACKS];
#else
extern tuya_ble_callback_t m_cb_table[TUYA_BLE_MAX_CALLBACKS];
#endif

STATIC volatile tuya_ble_uart_rev_state_t current_uart_rev_state = TUYA_BLE_UART_REV_STATE_FOUND_NULL;
STATIC UINT8_T UART_RX_Buffer[TUYA_BLE_UART_RX_BUFFER_MAX] = {0};
STATIC UINT8_T UART_RX_Buffer_temp[3] = {0};
STATIC UINT16_T uart_data_len =  0;
STATIC volatile UINT16_T UART_RX_Count = 0;

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




tuya_ble_status_t tuya_ble_api_test(VOID_T)
{
    return TUYA_BLE_SUCCESS;
}

#if TUYA_BLE_USE_OS

tuya_ble_status_t tuya_ble_callback_queue_register(VOID_T *cb_queue)
{
    tuya_ble_status_t ret;

    tuya_ble_device_enter_critical();
    if (m_callback_numbers == TUYA_BLE_MAX_CALLBACKS) {
        ret = TUYA_BLE_ERR_RESOURCES;
    } else {
        m_cb_queue_table[m_callback_numbers] = (UINT32_T)cb_queue;
        m_callback_numbers++;

        ret = TUYA_BLE_SUCCESS;
    }
    tuya_ble_device_exit_critical();

    return ret;
}

tuya_ble_status_t tuya_ble_event_response(tuya_ble_cb_evt_param_t *param)
{
    return tuya_ble_inter_event_response(param);
}

#else

tuya_ble_status_t tuya_ble_callback_queue_register(tuya_ble_callback_t cb)
{
    tuya_ble_status_t ret;

    tuya_ble_device_enter_critical();
    if (m_callback_numbers == TUYA_BLE_MAX_CALLBACKS) {
        ret = TUYA_BLE_ERR_RESOURCES;
    } else {
        m_cb_table[m_callback_numbers] = cb;
        m_callback_numbers++;

        ret = TUYA_BLE_SUCCESS;
    }
    tuya_ble_device_exit_critical();

    return ret;
}

UINT16_T tuya_ble_scheduler_queue_size_get(VOID_T)
{
    return tuya_ble_sched_queue_size_get();
}

UINT16_T tuya_ble_scheduler_queue_space_get(VOID_T)
{
    return tuya_ble_sched_queue_space_get();
}

UINT16_T tuya_ble_scheduler_queue_events_get(VOID_T)
{
    return tuya_ble_sched_queue_events_get();
}

#endif

BOOL_T tuya_ble_sleep_allowed_check(VOID_T)
{
#if TUYA_BLE_USE_OS
#if defined(TUYA_BLE_FEATURE_PRODUCT_TEST_ENABLE) && (TUYA_BLE_FEATURE_PRODUCT_TEST_ENABLE == 1)
    if (tuya_ble_internal_production_test_with_ble_flag_get() == 0) {
        return TRUE;
    } else {
        return FALSE;
    }
#else
    return TRUE;
#endif
#else //#if TUYA_BLE_USE_OS
    if (tuya_ble_scheduler_queue_events_get() == 0) {
#if defined(TUYA_BLE_FEATURE_PRODUCT_TEST_ENABLE) && (TUYA_BLE_FEATURE_PRODUCT_TEST_ENABLE == 1)
        if (tuya_ble_internal_production_test_with_ble_flag_get() == 0) {
            return TRUE;
        } else {
            return FALSE;
        }
#else
        return TRUE;
#endif
    } else {
        return FALSE;
    }
#endif //#if TUYA_BLE_USE_OS
}

tuya_ble_status_t tuya_ble_gatt_receive_data(UINT8_T *p_data, UINT16_T len)
{
    tuya_ble_evt_param_t event = {0};

    event.hdr.event = TUYA_BLE_EVT_MTU_DATA_RECEIVE;

    if (len > TUYA_BLE_DATA_MTU_MAX) {
        event.mtu_data.len = TUYA_BLE_DATA_MTU_MAX;
    } else {
        event.mtu_data.len = len;
    }

    if (event.mtu_data.len <= 20) {
        memcpy(event.mtu_data.data, p_data, event.mtu_data.len);
    } else {
        event.mtu_data.p_data = (UINT8_T *)tuya_ble_malloc(event.mtu_data.len);
        if (event.mtu_data.p_data == NULL) {
            return TUYA_BLE_ERR_NO_MEM;
        }

        memcpy(event.mtu_data.p_data, p_data, event.mtu_data.len);
    }

    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send ble data error,data len = %d ", event.mtu_data.len);

        if (event.mtu_data.p_data) {
            tuya_ble_free(event.mtu_data.p_data);
        }

        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}

STATIC BOOL_T tuya_ble_uart_data_unpack(UINT8_T data)
{
    BOOL_T ret = FALSE;

    UART_RX_Buffer_temp[0] = UART_RX_Buffer_temp[1];
    UART_RX_Buffer_temp[1] = UART_RX_Buffer_temp[2];
    UART_RX_Buffer_temp[2] = data;

    if (((UART_RX_Buffer_temp[0] == 0x55) || (UART_RX_Buffer_temp[0] == 0x66))
        && (UART_RX_Buffer_temp[1] == 0xAA)
        && ((UART_RX_Buffer_temp[2] == 0x00) || (UART_RX_Buffer_temp[2] == 0x01) || (UART_RX_Buffer_temp[2] == 0x10))) {
        memset(UART_RX_Buffer, 0, SIZEOF(UART_RX_Buffer));
        memcpy(UART_RX_Buffer, UART_RX_Buffer_temp, 3);
        memset(UART_RX_Buffer_temp, 0, 3);

        UART_RX_Count = 3;
        current_uart_rev_state = TUYA_BLE_UART_REV_STATE_FOUND_HEAD;
        uart_data_len = 0;

        return ret;
    }

    switch (current_uart_rev_state) {
    case TUYA_BLE_UART_REV_STATE_FOUND_NULL:
        break;
    case TUYA_BLE_UART_REV_STATE_FOUND_HEAD:
        UART_RX_Buffer[UART_RX_Count++] = data;
        current_uart_rev_state = TUYA_BLE_UART_REV_STATE_FOUND_CMD;
        break;
    case TUYA_BLE_UART_REV_STATE_FOUND_CMD:
        UART_RX_Buffer[UART_RX_Count++] = data;
        current_uart_rev_state = TUYA_BLE_UART_REV_STATE_FOUND_LEN_H;
        break;
    case TUYA_BLE_UART_REV_STATE_FOUND_LEN_H:
        UART_RX_Buffer[UART_RX_Count++] = data;

        uart_data_len = (UART_RX_Buffer[UART_RX_Count - 2] << 8) | UART_RX_Buffer[UART_RX_Count - 1];
        if (uart_data_len > TUYA_BLE_UART_RX_DATA_LEN_MAX) {
            memset(UART_RX_Buffer_temp, 0, 3);
            memset(UART_RX_Buffer, 0, SIZEOF(UART_RX_Buffer));
            UART_RX_Count = 0;
            uart_data_len = 0;

            current_uart_rev_state = TUYA_BLE_UART_REV_STATE_FOUND_NULL;
        } else if (uart_data_len > 0) {
            current_uart_rev_state = TUYA_BLE_UART_REV_STATE_FOUND_LEN_L;
        } else {
            current_uart_rev_state = TUYA_BLE_UART_REV_STATE_FOUND_DATA; //uart_data_len = 0
        }
        break;
    case TUYA_BLE_UART_REV_STATE_FOUND_LEN_L:
        UART_RX_Buffer[UART_RX_Count++] = data; //DATA
        uart_data_len--;
        if (uart_data_len == 0) {
            current_uart_rev_state = TUYA_BLE_UART_REV_STATE_FOUND_DATA;
        }
        break;
    case TUYA_BLE_UART_REV_STATE_FOUND_DATA:
        UART_RX_Buffer[UART_RX_Count++] = data; //sum data
        ret = TRUE;
        break;
    default:
        memset(UART_RX_Buffer_temp, 0, 3);
        memset(UART_RX_Buffer, 0, SIZEOF(UART_RX_Buffer));
        UART_RX_Count = 0;
        current_uart_rev_state = TUYA_BLE_UART_REV_STATE_FOUND_NULL;
        uart_data_len = 0;
        break;
    };

    return ret;
}

tuya_ble_status_t tuya_ble_common_uart_receive_data(UINT8_T *p_data, UINT16_T len)
{
    tuya_ble_status_t ret = TUYA_BLE_ERR_NOT_FOUND;
    tuya_ble_evt_param_t event = {0};
    UINT8_T *uart_evt_buffer;
    UINT16_T i;

    for (i = 0; i < len; i++) {
        if (tuya_ble_uart_data_unpack(p_data[i])) {

            uart_evt_buffer = (UINT8_T *)tuya_ble_malloc(UART_RX_Count);
            if (uart_evt_buffer == NULL) {
                TUYA_BLE_LOG_ERROR("tuya_MemGet uart evt buffer fail.");
                ret = TUYA_BLE_ERR_NO_MEM;
            } else {
                event.hdr.event = TUYA_BLE_EVT_UART_CMD;
                event.uart_cmd_data.data_len = UART_RX_Count;
                event.uart_cmd_data.p_data = uart_evt_buffer;

                memcpy(event.uart_cmd_data.p_data, &UART_RX_Buffer[0], event.uart_cmd_data.data_len);

                if (tuya_ble_event_send(&event) != 0) {
                    TUYA_BLE_LOG_ERROR("tuya_event_send uart data error.");
                    tuya_ble_free(uart_evt_buffer);
                    ret = TUYA_BLE_ERR_BUSY;
                } else {
                    ret = TUYA_BLE_SUCCESS;
                }
            }

            memset(UART_RX_Buffer_temp, 0, 3);
            memset(UART_RX_Buffer, 0, SIZEOF(UART_RX_Buffer));
            UART_RX_Count = 0;
            current_uart_rev_state = TUYA_BLE_UART_REV_STATE_FOUND_NULL;
            uart_data_len = 0;
        }
    }

    return ret;
}

tuya_ble_status_t tuya_ble_common_uart_send_full_instruction_received(UINT8_T *p_data, UINT16_T len)
{
    tuya_ble_status_t ret = TUYA_BLE_SUCCESS;
    tuya_ble_evt_param_t event;
    UINT8_T *uart_evt_buffer;

    if (len < 7) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    if ((p_data[0] != 0x55) && (p_data[0] != 0x66)) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    uart_evt_buffer = (UINT8_T *)tuya_ble_malloc(len);

    if (uart_evt_buffer == NULL) {
        TUYA_BLE_LOG_ERROR("tuya_MemGet uart evt buffer fail.");
        ret = TUYA_BLE_ERR_NO_MEM;
    } else {
        event.hdr.event = TUYA_BLE_EVT_UART_CMD;
        event.uart_cmd_data.data_len = len;
        event.uart_cmd_data.p_data = uart_evt_buffer;

        memcpy(event.uart_cmd_data.p_data, p_data, len);

        if (tuya_ble_event_send(&event) != 0) {
            TUYA_BLE_LOG_ERROR("tuya_event_send uart full cmd error.");
            tuya_ble_free(uart_evt_buffer);
            ret = TUYA_BLE_ERR_BUSY;
        }
    }

    return ret;
}

tuya_ble_status_t tuya_ble_device_update_product_id(tuya_ble_product_id_type_t type, UINT8_T len, UINT8_T *p_buf)
{
    tuya_ble_evt_param_t event;

    if (len == 0) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    if (len > TUYA_BLE_PRODUCT_ID_MAX_LEN) {
        len = TUYA_BLE_PRODUCT_ID_MAX_LEN;
    }

    event.hdr.event = TUYA_BLE_EVT_DEVICE_INFO_UPDATE;

    if (type == TUYA_BLE_PRODUCT_ID_TYPE_PID) {
        event.device_info_data.type = DEVICE_INFO_TYPE_PID;
    } else {
        event.device_info_data.type = DEVICE_INFO_TYPE_PRODUCT_KEY;
    }

    event.device_info_data.len = len;
    memcpy(event.device_info_data.data, p_buf, len);
    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send product id update error");
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_device_update_login_key(UINT8_T *p_buf, UINT8_T len)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_DEVICE_INFO_UPDATE;

    event.device_info_data.type = DEVICE_INFO_TYPE_LOGIN_KEY;

    if (len < LOGIN_KEY_LEN) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    event.device_info_data.len = LOGIN_KEY_LEN;
    memcpy(event.device_info_data.data, p_buf, LOGIN_KEY_LEN);
    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send LOGIN KEY update error");
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}

#if ((TUYA_BLE_PROTOCOL_VERSION_HIGN>=4) && (TUYA_BLE_BEACON_KEY_ENABLE))

tuya_ble_status_t tuya_ble_device_update_beacon_key(UINT8_T *p_buf, UINT8_T len)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_DEVICE_INFO_UPDATE;

    event.device_info_data.type = DEVICE_INFO_TYPE_BEACON_KEY;

    if (len < BEACON_KEY_LEN) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    event.device_info_data.len = BEACON_KEY_LEN;
    memcpy(event.device_info_data.data, p_buf, BEACON_KEY_LEN);
    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send BEACON KEY update error");
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}

#endif

tuya_ble_status_t tuya_ble_device_update_mcu_version(UINT32_T mcu_firmware_version, UINT32_T mcu_hardware_version)
{
    tuya_ble_set_external_mcu_version(mcu_firmware_version, mcu_hardware_version);
    return TUYA_BLE_SUCCESS;
}

#if defined(TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE) && (TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE == 1)

tuya_ble_status_t tuya_ble_device_update_attach_version(VOID_T *buf, UINT32_T size)
{
    tuya_ble_set_attach_version(buf, size);
    return TUYA_BLE_SUCCESS;
}

#endif

tuya_ble_status_t tuya_ble_device_update_bound_state(UINT8_T state)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_DEVICE_INFO_UPDATE;

    event.device_info_data.type = DEVICE_INFO_TYPE_BOUND;

    if ((state != 1) && (state != 0)) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    event.device_info_data.len = 1;
    event.device_info_data.data[0] = state;
    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send bound state update error");
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN == 5)

#if (TUYA_BLE_MUTI_DATA_SOURCE_SUPPORTED)

tuya_ble_status_t tuya_ble_dp_data_with_src_type_send(uint32_t sn, tuya_ble_data_source_type_t src_type, tuya_ble_dp_data_send_type_t type, tuya_ble_dp_data_send_mode_t mode, tuya_ble_dp_data_send_ack_t ack, uint8_t add_info_len, uint8_t *p_add_info, uint8_t *p_dp_data, uint32_t dp_data_len)
{
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer = NULL;
    mtp_ret ret;
    klv_node_s *list = NULL;
    uint8_t *ble_cb_evt_add_info_buffer = NULL;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if (src_type == DATA_SOURCE_TYPE_MAIN_EQUIPMENT && add_info_len != 0) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if ((dp_data_len > (TUYA_BLE_SEND_MAX_DATA_LEN - 7)) || (dp_data_len == 0)) {
        TUYA_BLE_LOG_ERROR("send dp data len error, data len = %d, max data len = %d", dp_data_len, TUYA_BLE_SEND_MAX_DATA_LEN - 7);
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_dp_data, dp_data_len, &list, 1);
    if (MTP_OK  !=  ret) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);

    if (add_info_len > 0) {
        ble_cb_evt_add_info_buffer = (uint8_t *)tuya_ble_malloc(add_info_len);
        if (ble_cb_evt_add_info_buffer == NULL) {
            return TUYA_BLE_ERR_NO_MEM;
        } else {
            memcpy(ble_cb_evt_add_info_buffer, p_add_info, add_info_len);
        }
    }

    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(dp_data_len + add_info_len + 9);
    if (ble_evt_buffer == NULL) {
        tuya_ble_free(ble_cb_evt_add_info_buffer);
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        ble_evt_buffer[0] = TUYA_BLE_DP_WRITE_WITH_SRC_TYPE_CURRENT_VERSION;
        ble_evt_buffer[1] = sn >> 24;
        ble_evt_buffer[2] = sn >> 16;
        ble_evt_buffer[3] = sn >> 8;
        ble_evt_buffer[4] = sn;

        ble_evt_buffer[5] = (uint8_t)src_type;
        ble_evt_buffer[6] = add_info_len;

        if (add_info_len > 0) {
            memcpy(ble_evt_buffer + 7, p_add_info, add_info_len);
        }

        switch (type) {
        case DP_SEND_TYPE_ACTIVE:
            ble_evt_buffer[7 + add_info_len] = 0;
            break;
        case DP_SEND_TYPE_PASSIVE:
            ble_evt_buffer[7 + add_info_len] = 1;
            break;
        default:
            ble_evt_buffer[7 + add_info_len] = 0;
            break;

        };
        if (ack == DP_SEND_WITHOUT_RESPONSE) {
            ble_evt_buffer[7 + add_info_len] |= 0x80;
        }
        ble_evt_buffer[8 + add_info_len] = mode;
        memcpy(ble_evt_buffer + 9 + add_info_len, p_dp_data, dp_data_len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_WITH_SRC_TYPE_SEND;
    evt.dp_with_src_type_send_data.sn = sn;
    evt.dp_with_src_type_send_data.type = type;
    evt.dp_with_src_type_send_data.mode = mode;
    evt.dp_with_src_type_send_data.ack = ack;
    evt.dp_with_src_type_send_data.add_info_len = add_info_len;
    evt.dp_with_src_type_send_data.src_type = src_type;
    evt.dp_with_src_type_send_data.p_data = ble_evt_buffer;
    evt.dp_with_src_type_send_data.p_add_info = ble_cb_evt_add_info_buffer;
    evt.dp_with_src_type_send_data.data_len = dp_data_len + add_info_len + 9;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_cb_evt_add_info_buffer);
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_dp_data_with_src_type_and_time_send(uint32_t sn, tuya_ble_data_source_type_t src_type, tuya_ble_dp_data_send_mode_t mode, tuya_ble_dp_data_send_time_type_t time_type, uint8_t *p_time_data, uint8_t add_info_len, uint8_t *p_add_info, uint8_t *p_dp_data, uint32_t dp_data_len)
{
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer = NULL;
    mtp_ret ret;
    klv_node_s *list = NULL;
    uint8_t *ble_cb_evt_add_info_buffer = NULL;
    uint16_t buffer_len = 0;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if (src_type == DATA_SOURCE_TYPE_MAIN_EQUIPMENT && add_info_len != 0) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if ((dp_data_len > (TUYA_BLE_SEND_MAX_DATA_LEN - 7 - 14)) || (dp_data_len == 0)) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    ret = data_2_klvlist(p_dp_data, dp_data_len, &list, 1);
    if (MTP_OK  !=  ret) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);

    if (add_info_len > 0) {
        ble_cb_evt_add_info_buffer = (uint8_t *)tuya_ble_malloc(add_info_len);
        if (ble_cb_evt_add_info_buffer == NULL) {
            return TUYA_BLE_ERR_NO_MEM;
        } else {
            memcpy(ble_cb_evt_add_info_buffer, p_add_info, add_info_len);
        }
    }


    if (time_type == DP_TIME_TYPE_UNIX_TIMESTAMP) {
        buffer_len = dp_data_len + add_info_len + 14;
    } else if (time_type == DP_TIME_TYPE_MS_STRING) {
        buffer_len = dp_data_len + add_info_len + 23;
    } else {
        tuya_ble_free(ble_cb_evt_add_info_buffer);
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(buffer_len);

    if (ble_evt_buffer == NULL) {
        tuya_ble_free(ble_cb_evt_add_info_buffer);
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        ble_evt_buffer[0] = TUYA_BLE_DP_WRITE_WITH_SRC_TYPE_CURRENT_VERSION;
        ble_evt_buffer[1] = sn >> 24;
        ble_evt_buffer[2] = sn >> 16;
        ble_evt_buffer[3] = sn >> 8;
        ble_evt_buffer[4] = sn;

        ble_evt_buffer[5] = src_type;
        ble_evt_buffer[6] = add_info_len;

        if (add_info_len > 0) {
            memcpy(ble_evt_buffer + 7, p_add_info, add_info_len);
        }

        ble_evt_buffer[7 + add_info_len] = 0;    //must 0 - WITH RESPONSE

        ble_evt_buffer[8 + add_info_len] = mode;
        ble_evt_buffer[9 + add_info_len] = time_type;

        if (time_type == DP_TIME_TYPE_UNIX_TIMESTAMP) {
            memcpy(&ble_evt_buffer[10 + add_info_len], p_time_data, 4);
            memcpy(ble_evt_buffer + add_info_len + 14, p_dp_data, dp_data_len);
        } else {
            memcpy(&ble_evt_buffer[10 + add_info_len], p_time_data, 13);
            memcpy(ble_evt_buffer + add_info_len + 23, p_dp_data, dp_data_len);
        }
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_WITH_SRC_TYPE_AND_TIME_SEND;
    evt.dp_with_src_type_and_time_send_data.sn = sn;
    evt.dp_with_src_type_and_time_send_data.type = DP_SEND_TYPE_ACTIVE;
    evt.dp_with_src_type_and_time_send_data.mode = mode;
    evt.dp_with_src_type_and_time_send_data.ack = DP_SEND_WITH_RESPONSE;
    evt.dp_with_src_type_and_time_send_data.p_data = ble_evt_buffer;
    evt.dp_with_src_type_and_time_send_data.data_len = buffer_len;
    evt.dp_with_src_type_and_time_send_data.src_type = src_type;
    evt.dp_with_src_type_and_time_send_data.add_info_len = add_info_len;
    evt.dp_with_src_type_and_time_send_data.p_add_info = ble_cb_evt_add_info_buffer;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_cb_evt_add_info_buffer);
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

#endif

tuya_ble_status_t tuya_ble_dp_data_send(UINT32_T sn, tuya_ble_dp_data_send_type_t type, tuya_ble_dp_data_send_mode_t mode, tuya_ble_dp_data_send_ack_t ack, UINT8_T *p_dp_data, UINT32_T dp_data_len)
{
    tuya_ble_evt_param_t evt;
    UINT8_T *ble_evt_buffer = NULL;
    mtp_ret ret;
    klv_node_s *list = NULL;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if (dp_data_len > TUYA_BLE_SEND_MAX_DP_DATA_LEN) {
        TUYA_BLE_LOG_ERROR("send dp data len error,data len = %d , max data len = %d", dp_data_len, TUYA_BLE_SEND_MAX_DP_DATA_LEN);
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    if ((dp_data_len > (TUYA_BLE_SEND_MAX_DATA_LEN - 7)) || (dp_data_len == 0)) {
        TUYA_BLE_LOG_ERROR("send dp data len error,data len = %d, max data len = %d", dp_data_len, TUYA_BLE_SEND_MAX_DATA_LEN - 7);
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_dp_data, dp_data_len, &list, 1);
    if (MTP_OK != ret) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);

    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(dp_data_len + 7);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        ble_evt_buffer[0] = TUYA_BLE_DP_WRITE_CURRENT_VERSION;
        ble_evt_buffer[1] = sn >> 24;
        ble_evt_buffer[2] = sn >> 16;
        ble_evt_buffer[3] = sn >> 8;
        ble_evt_buffer[4] = sn;
        switch (type) {
        case DP_SEND_TYPE_ACTIVE:
            ble_evt_buffer[5] = 0;
            break;
        case DP_SEND_TYPE_PASSIVE:
            ble_evt_buffer[5] = 1;
            break;
        default:
            ble_evt_buffer[5] = 0;
            break;

        };
        if (ack == DP_SEND_WITHOUT_RESPONSE) {
            ble_evt_buffer[5] |= 0x80;
        }
        ble_evt_buffer[6] = mode;
        memcpy(ble_evt_buffer + 7, p_dp_data, dp_data_len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_SEND;
    evt.dp_send_data.sn = sn;
    evt.dp_send_data.type = type;
    evt.dp_send_data.mode = mode;
    evt.dp_send_data.ack = ack;
    evt.dp_send_data.p_data = ble_evt_buffer;
    evt.dp_send_data.data_len = dp_data_len + 7;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_dp_data_with_time_send(UINT32_T sn, tuya_ble_dp_data_send_mode_t mode, tuya_ble_dp_data_send_time_type_t time_type, UINT8_T *p_time_data, UINT8_T *p_dp_data, UINT32_T dp_data_len)
{
    tuya_ble_evt_param_t evt;
    UINT8_T *ble_evt_buffer = NULL;
    mtp_ret ret;
    klv_node_s *list = NULL;
    UINT16_T buffer_len = 0;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if (dp_data_len > TUYA_BLE_SEND_MAX_DP_DATA_LEN) {
        TUYA_BLE_LOG_ERROR("send dp data len error,data len = %d , max data len = %d", dp_data_len, TUYA_BLE_SEND_MAX_DP_DATA_LEN);
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    if ((dp_data_len > (TUYA_BLE_SEND_MAX_DATA_LEN - 7 - 14)) || (dp_data_len == 0)) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    ret = data_2_klvlist(p_dp_data, dp_data_len, &list, 1);
    if (MTP_OK  !=  ret) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);

    if (time_type == DP_TIME_TYPE_UNIX_TIMESTAMP) {
        buffer_len = dp_data_len + 12;
    } else if (time_type == DP_TIME_TYPE_MS_STRING) {
        buffer_len = dp_data_len + 21;
    } else {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(buffer_len);

    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        ble_evt_buffer[0] = TUYA_BLE_DP_WRITE_CURRENT_VERSION;
        ble_evt_buffer[1] = sn >> 24;
        ble_evt_buffer[2] = sn >> 16;
        ble_evt_buffer[3] = sn >> 8;
        ble_evt_buffer[4] = sn;
        ble_evt_buffer[5] = 0;    //must 0 - WITH RESPONSE

        ble_evt_buffer[6] = mode;
        ble_evt_buffer[7] = time_type;

        if (time_type == DP_TIME_TYPE_UNIX_TIMESTAMP) {
            memcpy(&ble_evt_buffer[8], p_time_data, 4);
            memcpy(ble_evt_buffer + 12, p_dp_data, dp_data_len);
        } else {
            memcpy(&ble_evt_buffer[8], p_time_data, 13);
            memcpy(ble_evt_buffer + 21, p_dp_data, dp_data_len);
        }


    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_WITH_TIME_SEND;
    evt.dp_with_time_send_data.sn = sn;
    evt.dp_with_time_send_data.type = DP_SEND_TYPE_ACTIVE;
    evt.dp_with_time_send_data.mode = mode;
    evt.dp_with_time_send_data.ack = DP_SEND_WITH_RESPONSE;
    evt.dp_with_time_send_data.p_data = ble_evt_buffer;
    evt.dp_with_time_send_data.data_len = buffer_len;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

#elif (TUYA_BLE_PROTOCOL_VERSION_HIGN == 4)

#if (TUYA_BLE_MUTI_DATA_SOURCE_SUPPORTED&&TUYA_BLE_PROTOCOL_VERSION_LOW>=5)

tuya_ble_status_t tuya_ble_dp_data_with_src_type_send(uint32_t sn, tuya_ble_data_source_type_t src_type, tuya_ble_dp_data_send_type_t type, tuya_ble_dp_data_send_mode_t mode, tuya_ble_dp_data_send_ack_t ack, uint8_t add_info_len, uint8_t *p_add_info, uint8_t *p_dp_data, uint32_t dp_data_len)
{
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer = NULL;
    mtp_ret ret;
    klv_node_s *list = NULL;
    uint8_t *ble_cb_evt_add_info_buffer = NULL;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if (src_type == DATA_SOURCE_TYPE_MAIN_EQUIPMENT && add_info_len != 0) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if ((dp_data_len > (TUYA_BLE_SEND_MAX_DATA_LEN - 7)) || (dp_data_len == 0)) {
        TUYA_BLE_LOG_ERROR("send dp data len error,data len = %d , max data len = %d", dp_data_len, TUYA_BLE_SEND_MAX_DATA_LEN - 7);
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_dp_data, dp_data_len, &list, 1);
    if (MTP_OK != ret) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);

    if (add_info_len > 0) {
        ble_cb_evt_add_info_buffer = (uint8_t *)tuya_ble_malloc(add_info_len);
        if (ble_cb_evt_add_info_buffer == NULL) {
            return TUYA_BLE_ERR_NO_MEM;
        } else {
            memcpy(ble_cb_evt_add_info_buffer, p_add_info, add_info_len);
        }
    }

    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(dp_data_len + add_info_len + 9);
    if (ble_evt_buffer == NULL) {
        tuya_ble_free(ble_cb_evt_add_info_buffer);
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        ble_evt_buffer[0] = TUYA_BLE_DP_WRITE_WITH_SRC_TYPE_CURRENT_VERSION;
        ble_evt_buffer[1] = sn >> 24;
        ble_evt_buffer[2] = sn >> 16;
        ble_evt_buffer[3] = sn >> 8;
        ble_evt_buffer[4] = sn;

        ble_evt_buffer[5] = (uint8_t)src_type;
        ble_evt_buffer[6] = add_info_len;

        if (add_info_len > 0) {
            memcpy(ble_evt_buffer + 7, p_add_info, add_info_len);
        }

        switch (type) {
        case DP_SEND_TYPE_ACTIVE:
            ble_evt_buffer[7 + add_info_len] = 0;
            break;
        case DP_SEND_TYPE_PASSIVE:
            ble_evt_buffer[7 + add_info_len] = 1;
            break;
        default:
            ble_evt_buffer[7 + add_info_len] = 0;
            break;

        };
        if (ack == DP_SEND_WITHOUT_RESPONSE) {
            ble_evt_buffer[7 + add_info_len] |= 0x80;
        }
        ble_evt_buffer[8 + add_info_len] = mode;
        memcpy(ble_evt_buffer + 9 + add_info_len, p_dp_data, dp_data_len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_WITH_SRC_TYPE_SEND;
    evt.dp_with_src_type_send_data.sn = sn;
    evt.dp_with_src_type_send_data.type = type;
    evt.dp_with_src_type_send_data.mode = mode;
    evt.dp_with_src_type_send_data.ack = ack;
    evt.dp_with_src_type_send_data.add_info_len = add_info_len;
    evt.dp_with_src_type_send_data.src_type = src_type;
    evt.dp_with_src_type_send_data.p_data = ble_evt_buffer;
    evt.dp_with_src_type_send_data.p_add_info = ble_cb_evt_add_info_buffer;
    evt.dp_with_src_type_send_data.data_len = dp_data_len + add_info_len + 9;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_cb_evt_add_info_buffer);
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_dp_data_with_src_type_and_time_send(uint32_t sn, tuya_ble_data_source_type_t src_type, tuya_ble_dp_data_send_mode_t mode, tuya_ble_dp_data_send_time_type_t time_type, uint8_t *p_time_data, uint8_t add_info_len, uint8_t *p_add_info, uint8_t *p_dp_data, uint32_t dp_data_len)
{
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer = NULL;
    mtp_ret ret;
    klv_node_s *list = NULL;
    uint8_t *ble_cb_evt_add_info_buffer = NULL;
    uint16_t buffer_len = 0;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if (src_type == DATA_SOURCE_TYPE_MAIN_EQUIPMENT && add_info_len != 0) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if ((dp_data_len > (TUYA_BLE_SEND_MAX_DATA_LEN - 7 - 14)) || (dp_data_len == 0)) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    ret = data_2_klvlist(p_dp_data, dp_data_len, &list, 1);
    if (MTP_OK != ret) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);

    if (add_info_len > 0) {
        ble_cb_evt_add_info_buffer = (uint8_t *)tuya_ble_malloc(add_info_len);
        if (ble_cb_evt_add_info_buffer == NULL) {
            return TUYA_BLE_ERR_NO_MEM;
        } else {
            memcpy(ble_cb_evt_add_info_buffer, p_add_info, add_info_len);
        }
    }


    if (time_type == DP_TIME_TYPE_UNIX_TIMESTAMP) {
        buffer_len = dp_data_len + add_info_len + 14;
    } else if (time_type == DP_TIME_TYPE_MS_STRING) {
        buffer_len = dp_data_len + add_info_len + 23;
    } else {
        tuya_ble_free(ble_cb_evt_add_info_buffer);
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(buffer_len);

    if (ble_evt_buffer == NULL) {
        tuya_ble_free(ble_cb_evt_add_info_buffer);
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        ble_evt_buffer[0] = TUYA_BLE_DP_WRITE_WITH_SRC_TYPE_CURRENT_VERSION;
        ble_evt_buffer[1] = sn >> 24;
        ble_evt_buffer[2] = sn >> 16;
        ble_evt_buffer[3] = sn >> 8;
        ble_evt_buffer[4] = sn;

        ble_evt_buffer[5] = src_type;
        ble_evt_buffer[6] = add_info_len;

        if (add_info_len > 0) {
            memcpy(ble_evt_buffer + 7, p_add_info, add_info_len);
        }

        ble_evt_buffer[7 + add_info_len] = 0;    //must 0 - WITH RESPONSE

        ble_evt_buffer[8 + add_info_len] = mode;
        ble_evt_buffer[9 + add_info_len] = time_type;

        if (time_type == DP_TIME_TYPE_UNIX_TIMESTAMP) {
            memcpy(&ble_evt_buffer[10 + add_info_len], p_time_data, 4);
            memcpy(ble_evt_buffer + add_info_len + 14, p_dp_data, dp_data_len);
        } else {
            memcpy(&ble_evt_buffer[10 + add_info_len], p_time_data, 13);
            memcpy(ble_evt_buffer + add_info_len + 23, p_dp_data, dp_data_len);
        }
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_WITH_SRC_TYPE_AND_TIME_SEND;
    evt.dp_with_src_type_and_time_send_data.sn = sn;
    evt.dp_with_src_type_and_time_send_data.type = DP_SEND_TYPE_ACTIVE;
    evt.dp_with_src_type_and_time_send_data.mode = mode;
    evt.dp_with_src_type_and_time_send_data.ack = DP_SEND_WITH_RESPONSE;
    evt.dp_with_src_type_and_time_send_data.p_data = ble_evt_buffer;
    evt.dp_with_src_type_and_time_send_data.data_len = buffer_len;
    evt.dp_with_src_type_and_time_send_data.src_type = src_type;
    evt.dp_with_src_type_and_time_send_data.add_info_len = add_info_len;
    evt.dp_with_src_type_and_time_send_data.p_add_info = ble_cb_evt_add_info_buffer;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_cb_evt_add_info_buffer);
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

#endif

tuya_ble_status_t tuya_ble_dp_data_send(UINT32_T sn, tuya_ble_dp_data_send_type_t type, tuya_ble_dp_data_send_mode_t mode, tuya_ble_dp_data_send_ack_t ack, UINT8_T *p_dp_data, UINT32_T dp_data_len)
{
    tuya_ble_evt_param_t evt;
    UINT8_T *ble_evt_buffer = NULL;
    mtp_ret ret;
    klv_node_s *list = NULL;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if (dp_data_len > TUYA_BLE_SEND_MAX_DP_DATA_LEN) {
        TUYA_BLE_LOG_ERROR("send dp data len error,data len = %d , max data len = %d", dp_data_len, TUYA_BLE_SEND_MAX_DP_DATA_LEN);
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    if ((dp_data_len > (TUYA_BLE_SEND_MAX_DATA_LEN - 7)) || (dp_data_len == 0)) {
        TUYA_BLE_LOG_ERROR("send dp data len error,data len = %d , max data len = %d", dp_data_len, TUYA_BLE_SEND_MAX_DATA_LEN - 7);
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_dp_data, dp_data_len, &list, 1);
    if (MTP_OK != ret) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);

    ble_evt_buffer = (UINT8_T *)tuya_ble_malloc(dp_data_len + 7);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        ble_evt_buffer[0] = TUYA_BLE_DP_WRITE_CURRENT_VERSION;
        ble_evt_buffer[1] = sn >> 24;
        ble_evt_buffer[2] = sn >> 16;
        ble_evt_buffer[3] = sn >> 8;
        ble_evt_buffer[4] = sn;
        switch (type) {
        case DP_SEND_TYPE_ACTIVE:
            ble_evt_buffer[5] = 0;
            break;
        case DP_SEND_TYPE_PASSIVE:
            ble_evt_buffer[5] = 1;
            break;
        default:
            ble_evt_buffer[5] = 0;
            break;

        };
        if (ack == DP_SEND_WITHOUT_RESPONSE) {
            ble_evt_buffer[5] |= 0x80;
        }
        ble_evt_buffer[6] = mode;
        memcpy(ble_evt_buffer + 7, p_dp_data, dp_data_len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_SEND;
    evt.dp_send_data.sn = sn;
    evt.dp_send_data.type = type;
    evt.dp_send_data.mode = mode;
    evt.dp_send_data.ack = ack;
    evt.dp_send_data.p_data = ble_evt_buffer;
    evt.dp_send_data.data_len = dp_data_len + 7;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_dp_data_with_time_send(UINT32_T sn, tuya_ble_dp_data_send_mode_t mode, tuya_ble_dp_data_send_time_type_t time_type, UINT8_T *p_time_data, UINT8_T *p_dp_data, UINT32_T dp_data_len)
{
    tuya_ble_evt_param_t evt;
    UINT8_T *ble_evt_buffer = NULL;
    mtp_ret ret;
    klv_node_s *list = NULL;
    UINT16_T buffer_len = 0;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if (dp_data_len > TUYA_BLE_SEND_MAX_DP_DATA_LEN) {
        TUYA_BLE_LOG_ERROR("send dp data len error,data len = %d , max data len = %d", dp_data_len, TUYA_BLE_SEND_MAX_DP_DATA_LEN);
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    if ((dp_data_len > (TUYA_BLE_SEND_MAX_DATA_LEN - 7 - 14)) || (dp_data_len == 0)) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    ret = data_2_klvlist(p_dp_data, dp_data_len, &list, 1);
    if (MTP_OK != ret) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);

    if (time_type == DP_TIME_TYPE_UNIX_TIMESTAMP) {
        buffer_len = dp_data_len + 12;
    } else if (time_type == DP_TIME_TYPE_MS_STRING) {
        buffer_len = dp_data_len + 21;
    } else {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(buffer_len);

    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        ble_evt_buffer[0] = TUYA_BLE_DP_WRITE_CURRENT_VERSION;
        ble_evt_buffer[1] = sn >> 24;
        ble_evt_buffer[2] = sn >> 16;
        ble_evt_buffer[3] = sn >> 8;
        ble_evt_buffer[4] = sn;
        ble_evt_buffer[5] = 0;    //must 0 - WITH RESPONSE

        ble_evt_buffer[6] = mode;
        ble_evt_buffer[7] = time_type;

        if (time_type == DP_TIME_TYPE_UNIX_TIMESTAMP) {
            memcpy(&ble_evt_buffer[8], p_time_data, 4);
            memcpy(ble_evt_buffer + 12, p_dp_data, dp_data_len);
        } else {
            memcpy(&ble_evt_buffer[8], p_time_data, 13);
            memcpy(ble_evt_buffer + 21, p_dp_data, dp_data_len);
        }
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_WITH_TIME_SEND;
    evt.dp_with_time_send_data.sn = sn;
    evt.dp_with_time_send_data.type = DP_SEND_TYPE_ACTIVE;
    evt.dp_with_time_send_data.mode = mode;
    evt.dp_with_time_send_data.ack = DP_SEND_WITH_RESPONSE;
    evt.dp_with_time_send_data.p_data = ble_evt_buffer;
    evt.dp_with_time_send_data.data_len = buffer_len;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

#else

tuya_ble_status_t tuya_ble_dp_data_report(UINT8_T *p_data, UINT32_T len)
{
    tuya_ble_evt_param_t evt;
    UINT8_T *ble_evt_buffer;
    mtp_ret ret;
    klv_node_s *list = NULL;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if ((len > TUYA_BLE_REPORT_MAX_DP_DATA_LEN) || (len == 0)) {
        TUYA_BLE_LOG_ERROR("report dp data len error,data len = %d , max data len = %d", len, TUYA_BLE_REPORT_MAX_DP_DATA_LEN);
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_data, len, &list, 0);
    if (MTP_OK != ret) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);

    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(len);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        memcpy(ble_evt_buffer, p_data, len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_REPORTED;
    evt.reported_data.p_data = ble_evt_buffer;
    evt.reported_data.data_len = len;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_dp_data_with_flag_report(UINT16_T sn, tuya_ble_dp_data_send_mode_t mode, UINT8_T *p_data, UINT32_T len)
{
    tuya_ble_evt_param_t evt;
    UINT8_T *ble_evt_buffer = NULL;
    mtp_ret ret;
    klv_node_s *list = NULL;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if ((len > (TUYA_BLE_REPORT_MAX_DP_DATA_LEN - 3)) || (len == 0)) {
        TUYA_BLE_LOG_ERROR("report flag dp data len error,data len = %d , max data len = %d", len, (TUYA_BLE_REPORT_MAX_DP_DATA_LEN - 3));
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_data, len, &list, 0);
    if (MTP_OK != ret) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);

    ble_evt_buffer = (UINT8_T *)tuya_ble_malloc(len + 3);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        ble_evt_buffer[0] = sn >> 8;
        ble_evt_buffer[1] = sn;
        ble_evt_buffer[2] = mode;
        memcpy(ble_evt_buffer + 3, p_data, len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_WITH_FLAG_REPORTED;
    evt.flag_reported_data.sn = sn;
    evt.flag_reported_data.mode = mode;
    evt.flag_reported_data.p_data = ble_evt_buffer;
    evt.flag_reported_data.data_len = len + 3;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_dp_data_with_time_report(UINT32_T timestamp, UINT8_T *p_data, UINT32_T len)
{
    tuya_ble_evt_param_t evt;
    UINT8_T *ble_evt_buffer;
    mtp_ret ret;
    klv_node_s *list = NULL;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if ((len > TUYA_BLE_REPORT_MAX_DP_DATA_LEN) || (len == 0)) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_data, len, &list, 0);
    if (MTP_OK != ret) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);
    ble_evt_buffer = (UINT8_T *)tuya_ble_malloc(len);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        memcpy(ble_evt_buffer, p_data, len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_WITH_TIME_REPORTED;
    evt.reported_with_time_data.timestamp = timestamp;
    evt.reported_with_time_data.p_data = ble_evt_buffer;
    evt.reported_with_time_data.data_len = len;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_dp_data_with_flag_and_time_report(UINT16_T sn, tuya_ble_dp_data_send_mode_t mode, UINT32_T timestamp, UINT8_T *p_data, UINT32_T len)
{
    tuya_ble_evt_param_t evt;
    UINT8_T *ble_evt_buffer = NULL;
    mtp_ret ret;
    klv_node_s *list = NULL;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if ((len > (TUYA_BLE_REPORT_MAX_DP_DATA_LEN - 8)) || (len == 0)) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_data, len, &list, 0);
    if (MTP_OK != ret) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);
    ble_evt_buffer = (UINT8_T *)tuya_ble_malloc(len);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        memcpy(ble_evt_buffer, p_data, len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_WITH_FLAG_AND_TIME_REPORTED;
    evt.flag_reported_with_time_data.sn = sn;
    evt.flag_reported_with_time_data.mode = mode;
    evt.flag_reported_with_time_data.timestamp = timestamp;
    evt.flag_reported_with_time_data.p_data = ble_evt_buffer;
    evt.flag_reported_with_time_data.data_len = len;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_dp_data_with_time_ms_string_report(UINT8_T *time_string, UINT8_T *p_data, UINT32_T len)
{
    tuya_ble_evt_param_t evt;
    UINT8_T *ble_evt_buffer;
    mtp_ret ret;
    klv_node_s *list = NULL;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if ((len > TUYA_BLE_REPORT_MAX_DP_DATA_LEN) || (len == 0)) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_data, len, &list, 0);
    if (MTP_OK != ret) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);
    ble_evt_buffer = (UINT8_T *)tuya_ble_malloc(len);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        memcpy(ble_evt_buffer, p_data, len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_WITH_TIME_STRING_REPORTED;
    memcpy(evt.reported_with_time_string_data.time_string, time_string, 13);
    evt.reported_with_time_string_data.p_data = ble_evt_buffer;
    evt.reported_with_time_string_data.data_len = len;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_dp_data_with_flag_and_time_ms_string_report(UINT16_T sn, tuya_ble_dp_data_send_mode_t mode, UINT8_T *time_string, UINT8_T *p_data, UINT32_T len)
{
    tuya_ble_evt_param_t evt;
    UINT8_T *ble_evt_buffer = NULL;
    mtp_ret ret;
    klv_node_s *list = NULL;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if ((len > (TUYA_BLE_REPORT_MAX_DP_DATA_LEN - 17)) || (len == 0)) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_data, len, &list, 0);
    if (MTP_OK != ret) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);
    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(len);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        memcpy(ble_evt_buffer, p_data, len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_WITH_FLAG_AND_TIME_STRING_REPORTED;
    evt.flag_reported_with_time_string_data.sn = sn;
    evt.flag_reported_with_time_string_data.mode  = mode;
    memcpy(evt.flag_reported_with_time_string_data.time_string, time_string, 13);
    evt.flag_reported_with_time_string_data.p_data = ble_evt_buffer;
    evt.flag_reported_with_time_string_data.data_len = len;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

#endif

tuya_ble_status_t tuya_ble_data_passthrough(UINT8_T *p_data, UINT32_T len)
{
    tuya_ble_evt_param_t evt;
    UINT8_T *ble_evt_buffer;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if (len > TUYA_BLE_SEND_MAX_DATA_LEN) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(len);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    }

    memcpy(ble_evt_buffer, (UINT8_T *)p_data, len);
    evt.hdr.event = TUYA_BLE_EVT_DATA_PASSTHROUGH;
    evt.passthrough_data.p_data = ble_evt_buffer;
    evt.passthrough_data.data_len = len;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;

}

#if defined(TUYA_BLE_FEATURE_SPEECH_ENABLE) && (TUYA_BLE_FEATURE_SPEECH_ENABLE == 1)

tuya_ble_status_t tuya_ble_speech_cmds(tuya_ble_evt_t event, UINT8_T *p_data, UINT32_T len)
{
    tuya_ble_evt_param_t evt;
    UINT8_T *ble_evt_buffer;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if (len > TUYA_BLE_SEND_MAX_DATA_LEN) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(len);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    }

    memcpy(ble_evt_buffer, (UINT8_T *)p_data, len);
    evt.hdr.event = event;
    evt.speech_data.p_data = ble_evt_buffer;
    evt.speech_data.data_len = len;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_speech_control(UINT8_T *p_data, UINT32_T len)
{
    return tuya_ble_speech_cmds(TUYA_BLE_EVT_SPEECH_CONTROL, p_data, len);
}

tuya_ble_status_t tuya_ble_speech_raw_data_report(UINT8_T *p_data, UINT32_T len)
{
    return tuya_ble_speech_cmds(TUYA_BLE_EVT_SPEECH_RAW_DATA_REPORT, p_data, len);
}

tuya_ble_status_t tuya_ble_speech_result_write_rsp(UINT8_T *p_data, UINT32_T len)
{
    return tuya_ble_speech_cmds(TUYA_BLE_EVT_SPEECH_RESULT_WRITE_RSP, p_data, len);
}

tuya_ble_status_t tuya_ble_speech_clock_setting_rsp(UINT8_T *p_data, UINT32_T len)
{
    return tuya_ble_speech_cmds(TUYA_BLE_EVT_SPEECH_CLOCK_SETTING_RSP, p_data, len);
}

tuya_ble_status_t tuya_ble_speech_token_read_rsp(UINT8_T *p_data, UINT32_T len)
{
    return tuya_ble_speech_cmds(TUYA_BLE_EVT_SPEECH_TOKEN_READ_RSP, p_data, len);
}

tuya_ble_status_t tuya_ble_speech_token_report(UINT8_T *p_data, UINT32_T len)
{
    return tuya_ble_speech_cmds(TUYA_BLE_EVT_SPEECH_TOKEN_REPORT, p_data, len);
}

tuya_ble_status_t tuya_ble_speech_token_write_rsp(UINT8_T *p_data, UINT32_T len)
{
    return tuya_ble_speech_cmds(TUYA_BLE_EVT_SPEECH_TOKEN_WRITE_RSP, p_data, len);
}

tuya_ble_status_t tuya_ble_speech_common_setting_rsp(UINT8_T *p_data, UINT32_T len)
{
    return tuya_ble_speech_cmds(TUYA_BLE_EVT_SPEECH_COMMON_SETTING_RSP, p_data, len);
}

#endif

#if defined(TUYA_BLE_FEATURE_GPT_ENABLE) && (TUYA_BLE_FEATURE_GPT_ENABLE == 1)

tuya_ble_status_t tuya_ble_gpt_cmds(tuya_ble_evt_t event, UINT8_T *p_data, UINT32_T len)
{
    tuya_ble_evt_param_t evt;
    UINT8_T *ble_evt_buffer;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if (len > TUYA_BLE_SEND_MAX_DATA_LEN) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(len);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    }

    memcpy(ble_evt_buffer, (UINT8_T *)p_data, len);
    evt.hdr.event = event;
    evt.speech_data.p_data = ble_evt_buffer;
    evt.speech_data.data_len = len;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_gpt_control(UINT8_T *p_data, UINT32_T len)
{
    return tuya_ble_gpt_cmds(TUYA_BLE_EVT_GPT_CONTROL, p_data, len);
}

tuya_ble_status_t tuya_ble_gpt_raw_data_report(UINT8_T *p_data, UINT32_T len)
{
    return tuya_ble_gpt_cmds(TUYA_BLE_EVT_GPT_RAW_DATA_REPORT, p_data, len);
}

tuya_ble_status_t tuya_ble_gpt_result_write_rsp(UINT8_T *p_data, UINT32_T len)
{
    return tuya_ble_gpt_cmds(TUYA_BLE_EVT_GPT_RESULT_WRITE_RSP, p_data, len);
}

tuya_ble_status_t tuya_ble_gpt_raw_data_write_rsp(UINT8_T *p_data, UINT32_T len)
{
    return tuya_ble_gpt_cmds(TUYA_BLE_EVT_GPT_RAW_DATA_WRITE_RSP, p_data, len);
}

#endif

tuya_ble_status_t tuya_ble_production_test_asynchronous_response(UINT8_T channel, UINT8_T *p_data, UINT32_T len)
{
    tuya_ble_evt_param_t evt;
    UINT8_T *ble_evt_buffer;

    if (len > TUYA_BLE_SEND_MAX_DATA_LEN) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(len);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    }

    memcpy(ble_evt_buffer, (UINT8_T *)p_data, len);
    evt.hdr.event = TUYA_BLE_EVT_PRODUCTION_TEST_RESPONSE;
    evt.prod_test_res_data.p_data = ble_evt_buffer;
    evt.prod_test_res_data.data_len = len;
    evt.prod_test_res_data.channel = channel;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;

}

tuya_ble_status_t tuya_ble_net_config_response(INT16_T result_code)
{
    tuya_ble_evt_param_t evt;
    tuya_ble_connect_status_t status = tuya_ble_connect_status_get();

    if ((status != BONDING_CONN) && (status != UNBONDING_CONN)) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    evt.hdr.event = TUYA_BLE_EVT_NET_CONFIG_RESPONSE;
    evt.net_config_response_data.result_code = result_code;

    if (tuya_ble_event_send(&evt) != 0) {
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;

}

#if (!TUYA_BLE_DEVICE_REGISTER_FROM_BLE)

tuya_ble_status_t tuya_ble_ubound_response(UINT8_T result_code)
{
    tuya_ble_evt_param_t evt;
    tuya_ble_connect_status_t status = tuya_ble_connect_status_get();

    if (status != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    evt.hdr.event = TUYA_BLE_EVT_UNBOUND_RESPONSE;
    evt.ubound_res_data.result_code = result_code;

    if (tuya_ble_event_send(&evt) != 0) {
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;

}

tuya_ble_status_t tuya_ble_anomaly_ubound_response(UINT8_T result_code)
{
    tuya_ble_evt_param_t evt;
    tuya_ble_connect_status_t status = tuya_ble_connect_status_get();

    if (status != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    evt.hdr.event = TUYA_BLE_EVT_ANOMALY_UNBOUND_RESPONSE;
    evt.anomaly_ubound_res_data.result_code = result_code;

    if (tuya_ble_event_send(&evt) != 0) {
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;

}

tuya_ble_status_t tuya_ble_device_reset_response(UINT8_T result_code)
{
    tuya_ble_evt_param_t evt;
    tuya_ble_connect_status_t status = tuya_ble_connect_status_get();

    if (status != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    evt.hdr.event = TUYA_BLE_EVT_DEVICE_RESET_RESPONSE;
    evt.device_reset_res_data.result_code = result_code;

    if (tuya_ble_event_send(&evt) != 0) {
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;

}

#endif

tuya_ble_status_t tuya_ble_device_unbind(VOID_T)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_DEVICE_UNBIND;
    event.device_unbind_data.reserve = 0;

    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send device unbind error");
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_device_factory_reset(VOID_T)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_FACTORY_RESET;

    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send factory reset error");
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_time_req(UINT8_T time_type)
{
    tuya_ble_evt_param_t event;

    if (time_type > 2) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    event.hdr.event = TUYA_BLE_EVT_TIME_REQ;
    event.time_req_data.time_type = time_type;

    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send time req error");
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_remoter_proxy_auth(tuya_ble_remoter_proxy_auth_data_t data)
{
    tuya_ble_evt_param_t event;

    UINT8_T *ble_evt_buffer = tuya_ble_malloc(sizeof(tuya_ble_remoter_proxy_auth_data_unit_t) * data.num);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    }

    event.hdr.event = TUYA_BLE_EVT_REMOTER_PROXY_AUTH;
    memcpy(ble_evt_buffer, data.p_data, sizeof(tuya_ble_remoter_proxy_auth_data_unit_t) * data.num);
    event.remoter_proxy_auth_data.num = data.num;
    event.remoter_proxy_auth_data.p_data = ble_evt_buffer;
    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send time req error");
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_remoter_group_set(UINT8_T status)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_REMOTER_GROUP_SET;
    event.remoter_group_set_data.status = status;

    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send time req error");
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_remoter_group_delete(UINT8_T status)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_REMOTER_GROUP_DELETE;
    event.remoter_group_delete_data.status = status;

    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send time req error");
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_remoter_group_get(tuya_ble_remoter_group_get_data_rsp_t data)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_REMOTER_GROUP_GET;
    event.remoter_group_get_data = data;

    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send time req error");
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_ota_response(tuya_ble_ota_response_t *p_data)
{
    tuya_ble_evt_param_t evt;
    UINT8_T *ble_evt_buffer;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if (p_data->data_len > TUYA_BLE_SEND_MAX_DATA_LEN) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(p_data->data_len);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    }

    memcpy(ble_evt_buffer, (UINT8_T *)p_data->p_data, p_data->data_len);
    evt.hdr.event = TUYA_BLE_EVT_OTA_RESPONSE;
    evt.ota_response_data.type = p_data->type;
    evt.ota_response_data.p_data = ble_evt_buffer;
    evt.ota_response_data.data_len = p_data->data_len;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN >= 3)

tuya_ble_status_t tuya_ble_adv_data_connecting_request_set(UINT8_T on_off)
{
    tuya_ble_connect_status_t currnet_connect_status;
    tuya_ble_evt_param_t evt;

    if (on_off > 1) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    currnet_connect_status = tuya_ble_connect_status_get();
    if ((currnet_connect_status != BONDING_UNCONN) && (currnet_connect_status != UNBONDING_UNCONN)) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    evt.hdr.event = TUYA_BLE_EVT_CONNECTING_REQUEST;
    evt.connecting_request_data.cmd = on_off;

    if (tuya_ble_event_send(&evt) != 0) {
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;

}

#endif

VOID_T tuya_ble_connected_handler(VOID_T)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_CONNECT_STATUS_UPDATE;
    event.connect_change_evt = TUYA_BLE_CONNECTED;

    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send connect handler error");
    }
}

VOID_T tuya_ble_disconnected_handler(VOID_T)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_CONNECT_STATUS_UPDATE;
    event.connect_change_evt = TUYA_BLE_DISCONNECTED;

    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send disconnect handler error");
    }
}

#if TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE

VOID_T tuya_ble_link_encrypted_handler(VOID_T)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_LINK_STATUS_UPDATE;
    event.link_update_data.link_app_status = 1;

    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send link encrypted handler error");
    }
}

#endif

#if TUYA_BLE_BR_EDR_SUPPORTED

STATIC VOID_T tuya_ble_handle_br_edr_data_info_update_evt(int32_t evt_id, VOID_T *data)
{
    tuya_ble_br_edr_data_info_t *p_res_data = (tuya_ble_br_edr_data_info_t *)data;

    (VOID_T)evt_id;

    TUYA_BLE_LOG_INFO("tuya_ble_handle_br_edr_data_info_update_evt.");

    tuya_ble_br_edr_data_info_update_internal(p_res_data);

    if (p_res_data) {
        tuya_ble_free((UINT8_T *)p_res_data);
    }
}

tuya_ble_status_t tuya_ble_br_edr_data_info_update(tuya_ble_br_edr_data_info_t *p_data)
{
    tuya_ble_custom_evt_t custom_evt;
    tuya_ble_br_edr_data_info_t *p_res_data = NULL;
    UINT8_T *p_buffer = NULL;

    if ((p_data->name_len > sizeof(p_data->name)) || (p_data->connect_status >= BR_EDR_UNKNOW_STATUS) || (p_data->is_paired > 1)) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    p_buffer = tuya_ble_malloc(sizeof(tuya_ble_br_edr_data_info_t));

    if (p_buffer) {
        p_res_data = (tuya_ble_br_edr_data_info_t *)p_buffer;
        memcpy(p_res_data, p_data, sizeof(tuya_ble_br_edr_data_info_t));
    } else {
        return TUYA_BLE_ERR_NO_MEM;
    }

    custom_evt.evt_id = 0; //reserve
    custom_evt.data = p_res_data;
    custom_evt.custom_event_handler = tuya_ble_handle_br_edr_data_info_update_evt;

    if (tuya_ble_custom_event_send(custom_evt) != 0) {
        tuya_ble_free(p_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

#endif

void tuya_ble_sdk_reset_mac(tuya_ble_device_param_t *param_data)
{
    UINT8_T mac_temp[6];
    tal_util_str_hexstr2hexarray(param_data->mac_addr_string, MAC_STRING_LEN, mac_temp);
    TUYA_BLE_LOG_HEXDUMP_INFO("The MAC address passed in by the application", mac_temp, 6);
    tal_util_reverse_byte(mac_temp, 6); //Convert to little endian format
    memcpy(param_data->mac_addr.addr, mac_temp, MAC_LEN);
    memcpy(tuya_ble_current_para.auth_settings.mac, mac_temp, MAC_LEN);
#if ENABLE_BLUETOOTH_BREDR
    tuya_ble_current_para.auth_settings.bt_mac_len = MAC_LEN;
    memcpy(tuya_ble_current_para.auth_settings.bt_mac, mac_temp, MAC_LEN);
    tuya_ble_current_para.auth_settings.bt_mac[0]++;
#endif
    tal_util_str_hexarray2hexstr(FALSE, mac_temp, MAC_LEN, tuya_ble_current_para.auth_settings.mac_string);
}

tuya_ble_status_t tuya_ble_sdk_init(tuya_ble_device_param_t *param_data)
{
    UINT8_T device_id_temp[16];
    UINT8_T device_id_temp2[20];
    tuya_ble_gap_addr_t bt_addr;
    UINT8_T mac_temp[6];

#if (!TUYA_BLE_DEVICE_AUTH_DATA_STORE)
    if ((param_data->use_ext_license_key != 1) || ((param_data->device_id_len != 16) && (param_data->device_id_len != 20))) {
        TUYA_BLE_LOG_ERROR("tuya_ble_sdk_init param_data->device_id_len error.");
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
#endif

    if (param_data->adv_local_name_len > TUYA_BLE_ADV_LOCAL_NAME_MAX_LEN) {
        TUYA_BLE_LOG_ERROR("tuya_ble_sdk_init param_data->adv_local_name_len error.");
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    tuya_ble_storage_init();

    if (param_data->product_id_len > 0) {
        tuya_ble_current_para.pid_type = param_data->p_type;

        tuya_ble_current_para.pid_len = param_data->product_id_len;

        if (tuya_ble_current_para.pid_len > TUYA_BLE_PRODUCT_ID_MAX_LEN) {
            tuya_ble_current_para.pid_len = TUYA_BLE_PRODUCT_ID_MAX_LEN;
        }

        memcpy(tuya_ble_current_para.pid, param_data->product_id, tuya_ble_current_para.pid_len);
    } else {
#if (TUYA_BLE_PROD_SUPPORT_OEM_TYPE == TUYA_BLE_PROD_OEM_TYPE_0_5)
        tuya_ble_current_para.pid_type = tuya_ble_current_para.auth_settings.pid_type;
        tuya_ble_current_para.pid_len = tuya_ble_current_para.auth_settings.pid_len;
        memcpy(tuya_ble_current_para.pid, tuya_ble_current_para.auth_settings.factory_pid, tuya_ble_current_para.pid_len);
#endif
    }

    if (param_data->adv_local_name_len > 0) {
        memcpy(tuya_ble_current_para.adv_local_name, param_data->adv_local_name, param_data->adv_local_name_len);
        tuya_ble_current_para.adv_local_name_len = param_data->adv_local_name_len;
    } else {
        memcpy(tuya_ble_current_para.adv_local_name, "TY", 2);
        tuya_ble_current_para.adv_local_name_len = 2;
    }

#if (!TUYA_BLE_DEVICE_AUTH_DATA_STORE)
    if (param_data->device_id_len == 20) {
        TUYA_BLE_LOG_HEXDUMP_DEBUG("device_id_20 ", param_data->device_id, 20);
        tal_util_device_id_20_to_16(param_data->device_id, device_id_temp);
        TUYA_BLE_LOG_HEXDUMP_DEBUG("device_id_16 ", device_id_temp, 16);
        tal_util_device_id_16_to_20(device_id_temp, device_id_temp2);
        TUYA_BLE_LOG_HEXDUMP_DEBUG("device_id_20 ", device_id_temp2, 20);
        memcpy(tuya_ble_current_para.auth_settings.device_id, device_id_temp, DEVICE_ID_LEN);
    } else {
        memcpy(tuya_ble_current_para.auth_settings.device_id, param_data->device_id, DEVICE_ID_LEN);
    }

    memcpy(tuya_ble_current_para.auth_settings.auth_key, param_data->auth_key, AUTH_KEY_LEN);

    memcpy(tuya_ble_current_para.sys_settings.device_virtual_id, param_data->device_vid, DEVICE_VIRTUAL_ID_LEN);

    /* copy mac & mac_string into tuya_ble_current_para.auth_settings */
    tal_util_str_hexstr2hexarray(param_data->mac_addr_string, MAC_STRING_LEN, mac_temp);
    TUYA_BLE_LOG_HEXDUMP_INFO("The MAC address passed in by the application", mac_temp, 6);
    tal_util_reverse_byte(mac_temp, 6); //Convert to little endian format
    memcpy(param_data->mac_addr.addr, mac_temp, MAC_LEN);
    memcpy(tuya_ble_current_para.auth_settings.mac, mac_temp, MAC_LEN);
#if ENABLE_BLUETOOTH_BREDR
    tuya_ble_current_para.auth_settings.bt_mac_len = MAC_LEN;
    memcpy(tuya_ble_current_para.auth_settings.bt_mac, mac_temp, MAC_LEN);
    tuya_ble_current_para.auth_settings.bt_mac[0]++;
#endif
    tal_util_str_hexarray2hexstr(FALSE, mac_temp, MAC_LEN, tuya_ble_current_para.auth_settings.mac_string);

    tuya_ble_current_para.sys_settings.bound_flag = param_data->bound_flag;

    if (tuya_ble_current_para.sys_settings.bound_flag) {
        memcpy(tuya_ble_current_para.sys_settings.login_key, param_data->login_key, LOGIN_KEY_LEN);

#if ((TUYA_BLE_PROTOCOL_VERSION_HIGN >= 4) && (TUYA_BLE_BEACON_KEY_ENABLE))

        memcpy(tuya_ble_current_para.sys_settings.beacon_key, param_data->beacon_key, BEACON_KEY_LEN);

#endif
    }

#else

    if ((param_data->use_ext_license_key == 1) && ((param_data->device_id_len == 16) || (param_data->device_id_len == 20))) {
        if (param_data->device_id_len == 20) {
            TUYA_BLE_LOG_HEXDUMP_DEBUG("device_id_20 ", param_data->device_id, 20);
            tal_util_device_id_20_to_16(param_data->device_id, device_id_temp);
            TUYA_BLE_LOG_HEXDUMP_DEBUG("device_id_16 ", device_id_temp, 16);
            tal_util_device_id_16_to_20(device_id_temp, device_id_temp2);
            TUYA_BLE_LOG_HEXDUMP_DEBUG("device_id_20 ", device_id_temp2, 20);
            memcpy(tuya_ble_current_para.auth_settings.device_id, device_id_temp, DEVICE_ID_LEN);
        } else {
            memcpy(tuya_ble_current_para.auth_settings.device_id, param_data->device_id, DEVICE_ID_LEN);
        }

        memcpy(tuya_ble_current_para.auth_settings.auth_key, param_data->auth_key, AUTH_KEY_LEN);

        /* copy mac & mac_string into tuya_ble_current_para.auth_settings */
        tal_util_str_hexstr2hexarray(param_data->mac_addr_string, MAC_STRING_LEN, mac_temp);
        TUYA_BLE_LOG_HEXDUMP_INFO("The MAC address passed in by the application", mac_temp, 6);
        tal_util_reverse_byte(mac_temp, 6); //Convert to little endian format
        memcpy(param_data->mac_addr.addr, mac_temp, MAC_LEN);
        memcpy(tuya_ble_current_para.auth_settings.mac, mac_temp, MAC_LEN);
#if ENABLE_BLUETOOTH_BREDR
        tuya_ble_current_para.auth_settings.bt_mac_len = MAC_LEN;
        memcpy(tuya_ble_current_para.auth_settings.bt_mac, mac_temp, MAC_LEN);
        tuya_ble_current_para.auth_settings.bt_mac[0]++;
#endif
        tal_util_str_hexarray2hexstr(FALSE, mac_temp, MAC_LEN, tuya_ble_current_para.auth_settings.mac_string);
    }

#endif

    if (tuya_ble_gap_addr_get(&bt_addr) == TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_HEXDUMP_INFO("current device MAC address : ", bt_addr.addr, 6);

        if (TUYA_BLE_DEVICE_MAC_UPDATE) {
            if (param_data->device_id_len == 0) {
                bt_addr.addr_type = TUYA_BLE_ADDRESS_TYPE_RANDOM;
                memcpy(mac_temp, tuya_ble_current_para.auth_settings.mac, 6);
            } else {
                bt_addr.addr_type = param_data->mac_addr.addr_type;
                memcpy(mac_temp, param_data->mac_addr.addr, 6);
            }

            if ((!tal_util_buffer_value_is_all_x(mac_temp, 6, 0)) && (memcmp(mac_temp, bt_addr.addr, 6))) {
                memcpy(bt_addr.addr, mac_temp, 6);
                if (tuya_ble_gap_addr_set(&bt_addr) != TUYA_BLE_SUCCESS) {
                    TUYA_BLE_LOG_ERROR("GAP ADDR SET failed!");
                } else {
                    TUYA_BLE_LOG_INFO("GAP ADDR SET SUCCESSED!");
                    if (TUYA_BLE_DEVICE_MAC_UPDATE_RESET) {
                        tuya_ble_device_delay_ms(500);
                        tuya_ble_device_reset();
                    }
                }
            }
        }
    } else {
        TUYA_BLE_LOG_WARNING("GAP ADDR GET failed!");
    }

#if ENABLE_BLUETOOTH_BREDR
#if TUYA_BLE_WRITE_BT_MAC
    TUYA_BT_GAP_ADDR_T bt_mac;
    tal_bt_gap_address_get(&bt_mac);
    if ((tuya_ble_current_para.auth_settings.bt_mac_len == 6) && (memcmp(bt_mac.addr, tuya_ble_current_para.auth_settings.bt_mac, 6) != 0)) {
        memcpy(bt_mac.addr, tuya_ble_current_para.auth_settings.bt_mac, 6);
        if (tal_bt_gap_address_set(&bt_mac) == OPRT_OK) {
            TUYA_BLE_LOG_HEXDUMP_INFO("WRITE BT MAC ADDR SUCCESSED!", bt_mac.addr, 6);
            tuya_ble_device_delay_ms(500);
            tuya_ble_device_reset();
        } else {
            tal_bt_gap_address_get(&bt_mac);
            TUYA_BLE_LOG_HEXDUMP_ERROR("WRITE BT MAC ADDR FAILED!", bt_mac.addr, 6);
        }
    }
#else
    TUYA_BT_GAP_ADDR_T bt_mac;
    tal_bt_gap_address_get(&bt_mac);

    if ((tal_util_buffer_value_is_all_x(bt_mac.addr, 6, 0x00) == TRUE) || (tal_util_buffer_value_is_all_x(bt_mac.addr, 6, 0xFF) == TRUE)) {
        memcpy(bt_mac.addr, tuya_ble_current_para.auth_settings.mac, 6);
        if (tal_bt_gap_address_set(&bt_mac) == OPRT_OK) {
            TUYA_BLE_LOG_HEXDUMP_INFO("WRITE BT MAC ADDR SUCCESSED!", bt_mac.addr, 6);
            tuya_ble_device_delay_ms(500);
            tuya_ble_device_reset();
        } else {
            tal_bt_gap_address_get(&bt_mac);
            TUYA_BLE_LOG_HEXDUMP_ERROR("WRITE BT MAC ADDR FAILED!", bt_mac.addr, 6);
        }
    }
#endif
#endif
    tuya_ble_set_device_version(param_data->firmware_version, param_data->hardware_version);

    tuya_ble_set_external_mcu_version(0, 0); //Initialize to 0
#if defined(TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE) && (TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE == 1)
    tuya_ble_set_attach_version(NULL, 0);
#endif

    if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
        tuya_ble_connect_status_set(BONDING_UNCONN);
    } else {
        tuya_ble_connect_status_set(UNBONDING_UNCONN);
    }

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN<5)
#if (TUYA_BLE_PROTOCOL_VERSION_LOW<7)
    tuya_ble_current_para.sys_settings.protocol_v2_enable = 0;  // if not using v2 protocol, we need to assign 0 to protocol_v2_enable
#endif
#endif

    tuya_ble_adv_change();

    tuya_ble_event_init();

    tuya_ble_gatt_send_queue_init();

    tuya_ble_common_uart_init();

    tuya_ble_connect_monitor_timer_init();

    tuya_ble_disconnect_timer_init();

    TUYA_BLE_LOG_HEXDUMP_INFO("auth settings mac", tuya_ble_current_para.auth_settings.mac, MAC_LEN);
    TUYA_BLE_LOG_HEXDUMP_INFO("product_id", tuya_ble_current_para.pid, tuya_ble_current_para.pid_len);
    TUYA_BLE_LOG_HEXDUMP_DEBUG("device_uuid", tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN);
    TUYA_BLE_LOG_HEXDUMP_DEBUG("device_authkey", tuya_ble_current_para.auth_settings.auth_key, AUTH_KEY_LEN);
#if (TUYA_BLE_PROTOCOL_VERSION_HIGN<5)
    TUYA_BLE_LOG_INFO("bond_flag = %d, V2 Enable: %d", tuya_ble_current_para.sys_settings.bound_flag, tuya_ble_current_para.sys_settings.protocol_v2_enable);
#else
    TUYA_BLE_LOG_INFO("bond_flag = %d", tuya_ble_current_para.sys_settings.bound_flag);
#endif

    TUYA_BLE_LOG_INFO("tuya ble sdk version : "TUYA_BLE_SDK_VERSION_STR);

    return TUYA_BLE_SUCCESS;
}

