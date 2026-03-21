/**
 * @file tuya_ble_data_handler.c
 * @brief This is tuya_ble_data_handler file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "string.h"
#include "stdlib.h"

#include "tal_util.h"
#include "tal_utc.h"
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
#if defined(TUYA_BLE_FEATURE_BULKDATA_ENABLE) && (TUYA_BLE_FEATURE_BULKDATA_ENABLE == 1)
#include "tuya_ble_bulkdata.h"
#endif
#if defined(TUYA_BLE_FEATURE_PRODUCT_TEST_ENABLE) && (TUYA_BLE_FEATURE_PRODUCT_TEST_ENABLE == 1)
#include "tuya_ble_product_test.h"
#endif
#if defined(TUYA_BLE_FEATURE_UART_COMMON_ENABLE) && (TUYA_BLE_FEATURE_UART_COMMON_ENABLE == 1)
#include "tuya_ble_uart_common.h"
#endif
#if (TUYA_BLE_FILE_ENABLE != 0)
#include "tal_ble_file.h"
#endif
#if (TUYA_BLE_FEATURE_WEATHER_ENABLE != 0)
#include "tuya_ble_weather.h"
#endif
#if ( TUYA_BLE_FEATURE_IOT_CHANNEL_ENABLE != 0)
#include "tuya_ble_iot_channel.h"
#endif
#if (TUYA_BLE_FEATURE_EXT_MODULE_ENABLE != 0)
#include "tal_feature_ext_module.h"
#endif
#if (TUYA_BLE_FEATURE_APP_PASSTHROUGH_ENABLE != 0)
#include "tal_ble_app_passthrough.h"
#endif
#if (TUYA_BLE_ACCESSORY_MOUNT_SUPPORTED != 0)
#include "tuya_ble_feature_accessory.h"
#endif
#if defined(TUYA_BLE_FEATURE_ENABLE_GROUP) && (TUYA_BLE_FEATURE_ENABLE_GROUP == 1)
#include "tal_ble_group.h"
#endif // TUYA_BLE_FEATURE_ENABLE_GROUP

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/


/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
STATIC UINT32_T tuya_ble_firmware_version = 0;
STATIC UINT32_T tuya_ble_hardware_version = 0;

STATIC UINT32_T tuya_ble_mcu_firmware_version = 0;
STATIC UINT32_T tuya_ble_mcu_hardware_version = 0;

#if defined(TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE) && (TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE == 1)
STATIC UINT32_T tuya_ble_attach_ota_size = 0;
STATIC UINT8_T *tuya_ble_attach_ota_buf = NULL;
#endif

STATIC  tuya_ble_r_air_recv_packet  air_recv_packet;

STATIC frm_trsmitr_proc_s ty_trsmitr_proc;
STATIC frm_trsmitr_proc_s ty_trsmitr_proc_send;
STATIC UINT32_T send_packet_data_len = 20;

UINT8_T tuya_ble_pair_rand[6] = {0};
UINT8_T tuya_ble_pair_rand_valid = 0;

STATIC UINT32_T tuya_ble_receive_sn = 0;
STATIC UINT32_T tuya_ble_send_sn = 1;

tuya_ble_ota_status_t tuya_ble_ota_status;

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN == 4 || TUYA_BLE_PROTOCOL_VERSION_HIGN == 5)

#if (TUYA_BLE_SECURE_CONNECTION_TYPE == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)

STATIC UINT8_T dev_cert_data[1024];
STATIC UINT8_T current_ser_cert_pub_key[64];
STATIC UINT8_T current_dev_random_for_sign[6];
STATIC tuya_ble_auth_status_t current_auth_status;

#endif

#endif

STATIC char current_timems_string[14] = "000000000000";

STATIC tuya_ble_timer_t tuya_ble_xtimer_disconnect;

#if TUYA_BLE_BR_EDR_SUPPORTED

STATIC tuya_ble_br_edr_data_info_t br_edr_data_info;
STATIC UINT8_T br_edr_data_init_flag = 0;

#endif

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/
STATIC VOID_T tuya_ble_handle_pair_req(UINT8_T *recv_data, UINT16_T recv_len);




STATIC UINT32_T get_ble_send_sn(VOID_T)
{
    UINT32_T sn;
    tuya_ble_device_enter_critical();
    sn = tuya_ble_send_sn++;
    tuya_ble_device_exit_critical();
    return sn;
}

STATIC VOID_T set_ble_receive_sn(UINT32_T sn)
{
    tuya_ble_device_enter_critical();
    tuya_ble_receive_sn = sn;
    tuya_ble_device_exit_critical();
}

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN == 4 || TUYA_BLE_PROTOCOL_VERSION_HIGN == 5)

#if (TUYA_BLE_SECURE_CONNECTION_TYPE == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)

VOID_T tuya_ble_auth_data_reset(VOID_T)
{
    tuya_ble_device_enter_critical();
    current_auth_status = TUYA_BLE_AUTH_STATUS_PHASE_NONE;
    memset(current_ser_cert_pub_key, 0, SIZEOF(current_ser_cert_pub_key));
    memset(current_dev_random_for_sign, 0, SIZEOF(current_dev_random_for_sign));
    tuya_ble_device_exit_critical();
}

#endif

#endif

VOID_T tuya_ble_reset_ble_sn(VOID_T)
{
    tuya_ble_device_enter_critical();
    tuya_ble_receive_sn = 0;
    tuya_ble_send_sn = 1;
    send_packet_data_len = 20; //Resolve the offline unbinding error
    tuya_ble_device_exit_critical();
}

VOID_T tuya_ble_set_device_version(UINT32_T firmware_version, UINT32_T hardware_version)
{
    tuya_ble_firmware_version = firmware_version;
    tuya_ble_hardware_version = hardware_version;
}

VOID_T tuya_ble_set_external_mcu_version(UINT32_T firmware_version, UINT32_T hardware_version)
{
    tuya_ble_device_enter_critical();
    tuya_ble_mcu_firmware_version = firmware_version;
    tuya_ble_mcu_hardware_version = hardware_version;
    tuya_ble_device_exit_critical();
}

#if defined(TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE) && (TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE == 1)

VOID_T tuya_ble_set_attach_version(UINT8_T *buf, UINT32_T size)
{
    tuya_ble_device_enter_critical();
    tuya_ble_attach_ota_size = size;
    tuya_ble_attach_ota_buf = buf;
    tuya_ble_device_exit_critical();
}

#endif

VOID_T tuya_ble_ota_status_set(tuya_ble_ota_status_t status)
{
    tuya_ble_ota_status = status;
}

tuya_ble_ota_status_t tuya_ble_ota_status_get(VOID_T)
{
    return tuya_ble_ota_status;
}

VOID_T tuya_ble_pair_rand_clear(VOID_T)
{
    tuya_ble_device_enter_critical();
    memset(tuya_ble_pair_rand, 0, SIZEOF(tuya_ble_pair_rand));
    tuya_ble_pair_rand_valid = 0;
    tuya_ble_device_exit_critical();
}

UINT8_T tuya_ble_pair_rand_valid_get(VOID_T)
{
    return tuya_ble_pair_rand_valid;
}

UINT32_T tuya_ble_send_packet_data_length_get(VOID_T)
{
    return send_packet_data_len;
}

STATIC BOOL_T buffer_value_is_all_x(UINT8_T *buffer, UINT16_T len, UINT8_T value)
{
    BOOL_T ret = TRUE;
    for (UINT16_T i = 0; i < len; i++) {
        if (buffer[i] !=  value) {
            ret = FALSE;
            break;
        }
    }
    return ret;
}

VOID_T tuya_ble_air_recv_packet_free(VOID_T)
{
    if (air_recv_packet.recv_data) {
        tuya_ble_free(air_recv_packet.recv_data);
        air_recv_packet.recv_data = NULL;
        air_recv_packet.recv_len_max = 0;
        air_recv_packet.recv_len = 0;
    }
}

STATIC UINT32_T ble_data_unpack(UINT8_T *buf, UINT32_T len)
{
    STATIC UINT32_T offset = 0;
    mtp_ret ret;

    ret = trsmitr_recv_pkg_decode(&ty_trsmitr_proc, buf, len);
    if (MTP_OK != ret && MTP_TRSMITR_CONTINUE != ret) {
        air_recv_packet.recv_len_max = 0;
        air_recv_packet.recv_len = 0;
        if (air_recv_packet.recv_data) {
            tuya_ble_free(air_recv_packet.recv_data);
            air_recv_packet.recv_data = NULL;
        }

        return 1;
    }

    if (FRM_PKG_FIRST == ty_trsmitr_proc.pkg_desc) {
        if (air_recv_packet.recv_data) {
            tuya_ble_free(air_recv_packet.recv_data);
            air_recv_packet.recv_data = NULL;
        }
        air_recv_packet.recv_len_max = get_trsmitr_frame_total_len(&ty_trsmitr_proc);
        if ((air_recv_packet.recv_len_max > TUYA_BLE_AIR_FRAME_MAX) || (air_recv_packet.recv_len_max == 0)) {
            air_recv_packet.recv_len_max = 0;
            air_recv_packet.recv_len = 0;
            TUYA_BLE_LOG_ERROR("ble_data_unpack total size [%d ]error.", air_recv_packet.recv_len_max);
            return 2;
        }
        air_recv_packet.recv_len = 0;
        air_recv_packet.recv_data = tuya_ble_malloc(air_recv_packet.recv_len_max);
        if (air_recv_packet.recv_data == NULL) {
            TUYA_BLE_LOG_ERROR("ble_data_unpack malloc failed.");
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
            TUYA_BLE_LOG_ERROR("ble_data_unpack error.");
            air_recv_packet.recv_len_max = 0;
            air_recv_packet.recv_len = 0;
            return 2;
        }
    } else {
        ret = MTP_INVALID_PARAM;
        TUYA_BLE_LOG_ERROR("ble_data_unpack[] error:MTP_INVALID_PARAM");
        tuya_ble_air_recv_packet_free();
    }

    if (ret == MTP_OK) {
        offset = 0;
        TUYA_BLE_LOG_DEBUG("ble_data_unpack[%d]", air_recv_packet.recv_len);

        return 0;
    } else {
        return 2;
    }
}

STATIC UINT8_T ble_cmd_data_crc_check(UINT8_T *input, UINT16_T len)
{
    UINT16_T data_len = 0;
    UINT16_T crc16 = 0xFFFF;
    UINT16_T crc16_cal = 0;

    data_len = (input[10] << 8) | input[11];

    if (data_len  >= TUYA_BLE_RECEIVE_MAX_DATA_LEN) {
        return 1;
    }

    crc16_cal = tal_util_crc16(input, 12 + data_len,  &crc16);

    TUYA_BLE_LOG_DEBUG("crc16_cal[0x%04x]", crc16_cal);
    crc16 = (input[12 + data_len] << 8) | input[13 + data_len];
    TUYA_BLE_LOG_DEBUG("crc16[0x%04x]", crc16);
    if (crc16 == crc16_cal) {
        return 0;
    } else {
        return 1;
    }
}

BOOL_T tuya_ble_cryption_v2_command_check(tuya_ble_parameters_settings_t *current_para, UINT16_T command, UINT8_T encry_mode)
{
#if (TUYA_BLE_PROTOCOL_VERSION_HIGN == 5)
    if (current_para->sys_settings.bound_flag) {
#if (TUYA_BLE_DEVICE_SHARED == 0)
        // If Enable V2 protocol, we cannot use the key except KEY14, KEY15, KEY16
        if (encry_mode < BLE_ENCRYPTION_MODE_KEY_14) {
            return false;
        }
#endif

        // If try to unbind device anomaly, we can only use key_16
        if (command == FRM_ANOMALY_UNBONDING_REQ && encry_mode != BLE_ENCRYPTION_MODE_KEY_16) {
            return false;
        }
    } else {
        // V5.0
        if (encry_mode < BLE_ENCRYPTION_MODE_KEY_11 && command != FRM_FACTORY_TEST_CMD) {
            return false;
        }

        // Only Allow to encrypt the "device-info-get" command while using key 11
        if (BLE_ENCRYPTION_MODE_KEY_11 == encry_mode) {
            if (command != FRM_QRY_DEV_INFO_REQ) {
                return false;
            }
        }

        // Only Allow to encrypt the "pair-req" command while using key 12
        if (BLE_ENCRYPTION_MODE_KEY_12 == encry_mode) {
            if (command != PAIR_REQ) {
                return false;
            }
        }

        if (encry_mode == BLE_ENCRYPTION_MODE_SESSION_KEY_ENC) {
            return false;
        }
    }
#else
    if (current_para->sys_settings.protocol_v2_enable) {
#if (TUYA_BLE_DEVICE_SHARED == 0)
        // If Enable V2 protocol, we cannot use the key except KEY14, KEY15, KEY16
        if (encry_mode < BLE_ENCRYPTION_MODE_KEY_14) {
            return false;
        }
#endif

        // If try to unbind device anomaly, we can only use key_16
        if (command == FRM_ANOMALY_UNBONDING_REQ && encry_mode != BLE_ENCRYPTION_MODE_KEY_16) {
            return false;
        }
    } else {
        // Only Allow to encrypt the "device-info-get" command while using key 11
        if (BLE_ENCRYPTION_MODE_KEY_11 == encry_mode) {
            if (command != FRM_QRY_DEV_INFO_REQ) {
                return false;
            }
        }

        // Only Allow to encrypt the "pair-req" command while using key 12
        if (BLE_ENCRYPTION_MODE_KEY_12 == encry_mode) {
            if (command != PAIR_REQ) {
                return false;
            }
        }

        // Only Allow to encrypt the "device-info-get" command and "anomaly unbonding" command while using key 1
        if (BLE_ENCRYPTION_MODE_KEY_1 == encry_mode) {
            if (command != FRM_QRY_DEV_INFO_REQ && command != FRM_ANOMALY_UNBONDING_REQ) {
                return false;
            }
        }

        // Only Allow to encrypt the "pair-req" command while using key 2
        if (BLE_ENCRYPTION_MODE_KEY_2 == encry_mode) {
            if (command != PAIR_REQ) {
                return false;
            }
        }

        if (encry_mode == BLE_ENCRYPTION_MODE_SESSION_KEY_ENC) {
            return false;
        }
    }
#endif

    return true;
}

VOID_T tuya_ble_common_data_rx_proc(UINT8_T *buf, UINT16_T len)
{
    UINT8_T temp = 0;
    UINT32_T current_sn = 0;
    UINT16_T current_cmd = 0;
    tuya_ble_evt_param_t evt = {0};
    UINT8_T *ble_evt_buffer = NULL;
    UINT8_T current_encry_mode = 0;
    UINT16_T p_version = 0;
    BOOL_T is_cmd_with_encry_mode_correct = TRUE;

    if (ble_data_unpack(buf, len)) {
        return;
    }

    if (air_recv_packet.recv_len > TUYA_BLE_AIR_FRAME_MAX) {
        TUYA_BLE_LOG_ERROR("air_recv_packet.recv_len bigger than TUYA_BLE_AIR_FRAME_MAX.");
        tuya_ble_air_recv_packet_free();
        return;
    }

    if (ty_trsmitr_proc.version < 2) {
        TUYA_BLE_LOG_ERROR("ty_ble_rx_proc version not compatibility!");
        tuya_ble_air_recv_packet_free();
        return;
    }

    if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
        if ((ENCRYPTION_MODE_NONE == air_recv_packet.recv_data[0]) || (ENCRYPTION_MODE_FTM_KEY == air_recv_packet.recv_data[0])) {
            TUYA_BLE_LOG_ERROR("ty_ble_rx_proc data encryption mode error since bound_flag = 1.");
            tuya_ble_air_recv_packet_free();
            return;
        }
    }

    current_encry_mode = air_recv_packet.recv_data[0];

    TUYA_BLE_LOG_DEBUG("--------------------------------- decryped mode: %d", current_encry_mode);
    TUYA_BLE_LOG_HEXDUMP_DEBUG("received encry data", (UINT8_T *)air_recv_packet.recv_data, air_recv_packet.recv_len);

    air_recv_packet.de_encrypt_buf = NULL;
    air_recv_packet.de_encrypt_buf = (UINT8_T *)tuya_ble_malloc(air_recv_packet.recv_len);
    if (air_recv_packet.de_encrypt_buf == NULL) {
        TUYA_BLE_LOG_ERROR("air_recv_packet.de_encrypt_buf malloc failed.");
        tuya_ble_air_recv_packet_free();
        return;
    } else {
        p_version = (TUYA_BLE_PROTOCOL_VERSION_HIGN << 8) + TUYA_BLE_PROTOCOL_VERSION_LOW;

        air_recv_packet.decrypt_buf_len = 0;
        temp = tuya_ble_decryption(p_version, (UINT8_T *)air_recv_packet.recv_data, air_recv_packet.recv_len,  &air_recv_packet.decrypt_buf_len,
                                   (UINT8_T *)air_recv_packet.de_encrypt_buf, &tuya_ble_current_para, tuya_ble_pair_rand, TUYA_BLE_PROTOCOL_VERSION_HIGN);

        tuya_ble_air_recv_packet_free();
    }

    if (temp != 0) {
        TUYA_BLE_LOG_ERROR("ble receive data decryption error code = %d", temp);
        tuya_ble_free(air_recv_packet.de_encrypt_buf);
        return;
    }

    TUYA_BLE_LOG_HEXDUMP_DEBUG("decryped data", (UINT8_T *)air_recv_packet.de_encrypt_buf, air_recv_packet.decrypt_buf_len);

    if (ble_cmd_data_crc_check((UINT8_T *)air_recv_packet.de_encrypt_buf, air_recv_packet.decrypt_buf_len) != 0) {
        TUYA_BLE_LOG_ERROR("ble receive data crc check error!");
        tuya_ble_free(air_recv_packet.de_encrypt_buf);
        return;
    }

#if TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE

    if (current_cmd == FRM_QRY_DEV_INFO_REQ) {
        tuya_ble_reset_ble_sn();
    }

#endif

    current_sn  = air_recv_packet.de_encrypt_buf[0] << 24;
    current_sn += air_recv_packet.de_encrypt_buf[1] << 16;
    current_sn += air_recv_packet.de_encrypt_buf[2] << 8;
    current_sn += air_recv_packet.de_encrypt_buf[3];

    if (current_sn <= tuya_ble_receive_sn) {
        TUYA_BLE_LOG_ERROR("ble receive SN error!");
        tuya_ble_gap_disconnect();
        tuya_ble_free(air_recv_packet.de_encrypt_buf);
        return;
    } else {
        set_ble_receive_sn(current_sn);
    }

    current_cmd = ((air_recv_packet.de_encrypt_buf[8] << 8) | air_recv_packet.de_encrypt_buf[9]);

    if (current_cmd == FRM_FACTORY_TEST_CMD) {
        // Check recvived cmd with encry mode corresponding relationship
#if (TUYA_BLE_PROD_TEST_SUPPORT_ENCRYPTION != 0)
        if (current_encry_mode != ENCRYPTION_MODE_FTM_KEY) {
            is_cmd_with_encry_mode_correct = FALSE;
        }
#else
        if (current_encry_mode != ENCRYPTION_MODE_NONE) {
            is_cmd_with_encry_mode_correct = FALSE;
        }
#endif

        if (!is_cmd_with_encry_mode_correct) {
            tuya_ble_free(air_recv_packet.de_encrypt_buf);
            TUYA_BLE_LOG_ERROR("ble receive cmd error on prod factory test state, need encrypt!");
            return;
        }
    }

    if ((BONDING_CONN != tuya_ble_connect_status_get())
        && (FRM_QRY_DEV_INFO_REQ != current_cmd)
        && (PAIR_REQ != current_cmd)
        && (FRM_LOGIN_KEY_REQ != current_cmd)
        && (FRM_FACTORY_TEST_CMD != current_cmd)
        && (FRM_NET_CONFIG_INFO_REQ != current_cmd)
        && (FRM_ANOMALY_UNBONDING_REQ != current_cmd)
        && (FRM_AUTHENTICATE_PHASE_1_REQ != current_cmd)
        && (FRM_AUTHENTICATE_PHASE_2_REQ != current_cmd)
        && (FRM_AUTHENTICATE_PHASE_3_REQ != current_cmd)) {
        tuya_ble_free(air_recv_packet.de_encrypt_buf);
        TUYA_BLE_LOG_ERROR("ble receive cmd error on current bond state!");
        return;
    }

    if (tuya_ble_cryption_v2_command_check(&tuya_ble_current_para, current_cmd, current_encry_mode) == FALSE) {
        tuya_ble_gap_disconnect();
        tuya_ble_free(air_recv_packet.de_encrypt_buf);
        return;
    }

    if (BONDING_CONN == tuya_ble_connect_status_get()) {
#if (TUYA_BLE_FEATURE_SYSTEM_CONNECTION_ENABLE)
        if ((current_encry_mode == ENCRYPTION_MODE_KEY_4) && (current_cmd == FRM_QRY_DEV_INFO_REQ)) {
            // for system connection enable
        } else
#endif
            if (current_encry_mode != ENCRYPTION_MODE_SESSION_KEY) {
                TUYA_BLE_LOG_ERROR("Wrong Status Using in Wrong States 1");
                tuya_ble_gap_disconnect();
                tuya_ble_free(air_recv_packet.de_encrypt_buf);
                return;
            }
    }

    if (tuya_ble_ota_status_get() != TUYA_BLE_OTA_STATUS_NONE) {
        if (!((current_cmd >= FRM_OTA_START_REQ) && (current_cmd <= FRM_OTA_END_REQ))) {
            tuya_ble_free(air_recv_packet.de_encrypt_buf);
            TUYA_BLE_LOG_ERROR("ble receive cmd error on ota state!");
            return;
        }
    }

    ble_evt_buffer = (UINT8_T *)tuya_ble_malloc(air_recv_packet.decrypt_buf_len + 1);
    if (ble_evt_buffer == NULL) {
        TUYA_BLE_LOG_ERROR("ty_ble_rx_proc no mem.");
        tuya_ble_free(air_recv_packet.de_encrypt_buf);
        return;
    } else {
        memset(ble_evt_buffer, 0, air_recv_packet.decrypt_buf_len + 1);
    }

    ble_evt_buffer[0] = current_encry_mode;
    memcpy(ble_evt_buffer + 1, (UINT8_T *)air_recv_packet.de_encrypt_buf, air_recv_packet.decrypt_buf_len);

    if (current_cmd == PAIR_REQ) {
        tuya_ble_handle_pair_req(ble_evt_buffer, air_recv_packet.decrypt_buf_len + 1);
        tuya_ble_free(ble_evt_buffer);
    } else {
        evt.hdr.event = TUYA_BLE_EVT_BLE_CMD;
        evt.ble_cmd_data.cmd = current_cmd;
        evt.ble_cmd_data.p_data = ble_evt_buffer;
        evt.ble_cmd_data.data_len = air_recv_packet.decrypt_buf_len + 1;
        TUYA_BLE_LOG_DEBUG("BLE EVENT SEND-CMD:0x%02x - LEN:0x%02x", current_cmd, air_recv_packet.decrypt_buf_len + 1);

        if (tuya_ble_event_send(&evt) != 0) {
            TUYA_BLE_LOG_ERROR("ble event send fail!");
            tuya_ble_free(ble_evt_buffer);
        }
    }

    tuya_ble_free(air_recv_packet.de_encrypt_buf);
}

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN == 5)

__TUYA_BLE_WEAK UINT32_T tuya_ble_device_ability_value(VOID_T)
{
    /*
    --------------------------------------------------------------------------- |
    |       unused     |   p_buf[85]      |    p_buf[86]     |    p_buf[87]     |
    |---------------------------------------------------------------------------|
    |   bit31~bit24    |   bit23~bit16    |   bit15~bit8     |   bit7~bit0      |
    ----------------------------------------------------------------------------|
    */

    return 0;
}

STATIC VOID_T tuya_ble_handle_dev_info_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T p_buf[200];
    UINT8_T payload_len = 0;
    UINT32_T ack_sn = 0;
    UINT8_T encry_mode = 0;
    UINT32_T version_temp_s, version_temp_h;
    tuya_ble_gap_addr_t mac_addr;
    UINT16_T data_len = recv_data[11] << 8 | recv_data[12];
#if (TUYA_BLE_DEVICE_ABILITY_SELF_MANAGEMENT == 0)
    UINT32_T device_ability = 0;
#endif

    if (data_len != 2) {
        TUYA_BLE_LOG_ERROR("get device infor cmd error with v4, data_len = %d", data_len);
        tuya_ble_gap_disconnect();
        return;
    }

    memset(p_buf, 0, SIZEOF(p_buf));

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    TUYA_BLE_LOG_DEBUG("get device infor-%d", tuya_ble_current_para.sys_settings.bound_flag);

    send_packet_data_len = recv_data[13] << 8 | recv_data[14];

    if ((send_packet_data_len == 0) || (send_packet_data_len > TUYA_BLE_DATA_MTU_MAX)) {
        send_packet_data_len = 20;
    }

    TUYA_BLE_LOG_DEBUG("current app mtu data len = %d", send_packet_data_len);

    tuya_ble_rand_generator(tuya_ble_pair_rand, 6);
    tuya_ble_pair_rand_valid = 1;

#if TUYA_BLE_THREE_PART_VERSION_NUMBER_ENABLE
    version_temp_s = tuya_ble_firmware_version >> 8;
    version_temp_h = tuya_ble_hardware_version >> 8;
    p_buf[4] = 0x05;
#else
    version_temp_s = tuya_ble_firmware_version;
    version_temp_h = tuya_ble_hardware_version;
    p_buf[4] = 0x00;
#endif

    p_buf[0] = (version_temp_s >> 8) & 0xff;
    p_buf[1] = (version_temp_s & 0xff);
    p_buf[2] = TUYA_BLE_PROTOCOL_VERSION_HIGN;
    p_buf[3] = TUYA_BLE_PROTOCOL_VERSION_LOW;

#if (TUYA_BLE_SECURE_CONNECTION_TYPE == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)
    p_buf[4] |= 0x02;
    if (TUYA_BLE_ADVANCED_ENCRYPTION_AUTH_ON_CONNECT) {
        p_buf[4] |= 0x08;
    }
#endif

    if (TUYA_BLE_BEACON_KEY_ENABLE) {
        p_buf[4] |= 0x10;
    }

#if TUYA_BLE_VOS_ENABLE
    p_buf[4] |= 0x40;  //alexa enable
#endif

    p_buf[5] = tuya_ble_current_para.sys_settings.bound_flag;
    memcpy(&p_buf[6], tuya_ble_pair_rand, 6);
    p_buf[12] = (version_temp_h >> 8) & 0xff;
    p_buf[13] = (version_temp_h & 0xff);

    tuya_ble_register_key_generate(&p_buf[14], &tuya_ble_current_para);

    p_buf[46] = (tuya_ble_firmware_version >> 16) & 0xff;
    p_buf[47] = (tuya_ble_firmware_version >> 8) & 0xff;
    p_buf[48] = (tuya_ble_firmware_version & 0xff);
    p_buf[49] = (tuya_ble_hardware_version >> 16) & 0xff;
    p_buf[50] = (tuya_ble_hardware_version >> 8) & 0xff;
    p_buf[51] = (tuya_ble_hardware_version & 0xff);

    p_buf[52] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY >> 8;
    p_buf[53] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY & 0xFF; //communication ability

#if TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE
    if (TY_LINK_ENCRYPTED == tuya_ble_link_status_get()) {
        p_buf[54] |= 0x01;
    }
#endif

    p_buf[54] |= 0x02;
    p_buf[54] |= 0x04;

    memcpy(&p_buf[55], tuya_ble_current_para.sys_settings.device_virtual_id, DEVICE_VIRTUAL_ID_LEN);

    p_buf[77] = (tuya_ble_mcu_firmware_version >> 16) & 0xff;
    p_buf[78] = (tuya_ble_mcu_firmware_version >> 8) & 0xff;
    p_buf[79] = (tuya_ble_mcu_firmware_version & 0xff);
    p_buf[80] = (tuya_ble_mcu_hardware_version >> 16) & 0xff;
    p_buf[81] = (tuya_ble_mcu_hardware_version >> 8) & 0xff;
    p_buf[82] = (tuya_ble_mcu_hardware_version & 0xff);

    p_buf[83] = TUYA_BLE_WIFI_DEVICE_REGISTER_MODE;

    memset(&p_buf[84], 0, 4);

#if (TUYA_BLE_DEVICE_ABILITY_SELF_MANAGEMENT)

#if (TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE)
    p_buf[87] |= 0x08;

    if (TUYA_BLE_LINK_LAYER_FORCED_ENCRYPTION) {
        p_buf[87] |= 0x80;
    }
#endif

#if TUYA_BLE_BR_EDR_SUPPORTED
    p_buf[87] |= 0x40;
#endif

#if (TUYA_BLE_FEATURE_EXT_MODULE_ENABLE)
    p_buf[87] |= 0x10;
#endif

#if TUYA_BLE_ACCESSORY_MOUNT_SUPPORTED
    p_buf[86] |= 0x02;
#endif

#if TUYA_BLE_FEATURE_LOCAL_TIMER_ENABLE
    p_buf[87] |= 0x20;
#endif

    if (TUYA_BLE_CONNECTIVITY == 0) {
        p_buf[86] &= (~0x10);
        p_buf[86] &= (~0x08);
    } else if (TUYA_BLE_CONNECTIVITY == 1) {
        p_buf[86] &= (~0x10);
        p_buf[86] |= 0x08;
    } else if (TUYA_BLE_CONNECTIVITY == 2) {
        p_buf[86] |= 0x10;
        p_buf[86] &= (~0x08);
    }

#if TUYA_BLE_GATEWAY_CONN_MODE
    p_buf[86] |= 0x20;
#endif

#if TUYA_BLE_FEATURE_REPEATER_ENABLE
    p_buf[85] |= 0x01;
#endif

#if TUYA_BLE_SINGLE_BANK_OTA_MODE
    p_buf[85] |= 0x02;
#endif

#if TUYA_BLE_FEATURE_LONG_RANGE
    p_buf[85] |= 0x10;
#endif

#if TUYA_BLE_FEATURE_ENABLE_GROUP
    p_buf[85] |= 0x20;
#endif

#if TUYA_BLE_BEACON_KEY_ENABLE
    p_buf[87] |= 0x04;
#endif

#else
    device_ability = tuya_ble_device_ability_value();
    p_buf[85] = (device_ability >> 16) & 0xff;
    p_buf[86] = (device_ability >> 8) & 0xff;
    p_buf[87] = (device_ability) & 0xff;
#endif

    tuya_ble_gap_addr_get(&mac_addr);
    p_buf[88] = mac_addr.addr_type;
    memcpy(&p_buf[89], mac_addr.addr, 6);

    p_buf[95] = strlen(TUYA_BLE_APP_FIRMWARE_KEY);
    if ((p_buf[95] == 8) || (p_buf[95] == 16)) {
        memcpy(&p_buf[96], TUYA_BLE_APP_FIRMWARE_KEY, p_buf[95]);
    } else {
        p_buf[95] = 0;
    }

    payload_len = 96 + p_buf[95];

    p_buf[payload_len++] = 0; // zigbee_mac_len

#if defined(TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE) && (TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE == 1)
    p_buf[payload_len++] = tuya_ble_attach_ota_size * 4;

    if ((tuya_ble_attach_ota_size != 0) && (tuya_ble_attach_ota_buf != NULL)) {
        UINT8_T *tmp_buf = tal_malloc(tuya_ble_attach_ota_size * 4);
        if (tmp_buf) {
            for (UINT32_T idx = 0; idx < tuya_ble_attach_ota_size; idx++) {
                tmp_buf[idx * 4 + 0] = tuya_ble_attach_ota_buf[idx * (4 + sizeof(TUYA_OTA_FIRMWARE_INFO_T)) + 0];
                tmp_buf[idx * 4 + 1] = tuya_ble_attach_ota_buf[idx * (4 + sizeof(TUYA_OTA_FIRMWARE_INFO_T)) + 3];
                tmp_buf[idx * 4 + 2] = tuya_ble_attach_ota_buf[idx * (4 + sizeof(TUYA_OTA_FIRMWARE_INFO_T)) + 2];
                tmp_buf[idx * 4 + 3] = tuya_ble_attach_ota_buf[idx * (4 + sizeof(TUYA_OTA_FIRMWARE_INFO_T)) + 1];
            }

            memcpy(&p_buf[payload_len], tmp_buf, tuya_ble_attach_ota_size * 4);
            payload_len += tuya_ble_attach_ota_size * 4;

            tal_free(tmp_buf);
        }
    }
#else
    p_buf[payload_len++] = 0; // attach_len
#endif

    p_buf[payload_len++] = 2; // PacketMaxSize_len

    p_buf[payload_len++] = 5; // PacketMaxSize

    p_buf[payload_len++] = 0; // PacketMaxSize

    encry_mode = recv_data[0];

    if (tuya_ble_comm_data_send(FRM_QRY_DEV_INFO_RESP, ack_sn, p_buf, payload_len,  encry_mode) == 2) {
        tuya_ble_pair_rand_clear();
    } else {
#if TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE
        if (TY_LINK_ENCRYPTED != tuya_ble_link_status_get()) {
            tuya_ble_connect_monitor_timer_stop();
            tuya_ble_link_security_request();
            tuya_ble_link_status_set(TY_LINK_ENCRYPTED_REQUEST);
        }
#endif
    }
}

#if (TUYA_BLE_SECURE_CONNECTION_TYPE == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)

STATIC VOID_T tuya_ble_handle_authenticate_phase1(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T p_buf[7];
    UINT8_T encry_mode = 0;
    UINT32_T ack_sn = 0;
    UINT8_T sig_data[64];
    UINT16_T data_len = 0;

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    data_len = (recv_data[11] << 8) + recv_data[12];

    if (((tuya_ble_current_para.sys_settings.bound_flag == 0) && (recv_data[13] == 0)) || ((tuya_ble_current_para.sys_settings.bound_flag == 1) && (recv_data[13] == 1))) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase1 -> invalid state.");
        tuya_ble_gap_disconnect();
        return;
    }

    if ((data_len != 1 && data_len != 130) || (current_auth_status != TUYA_BLE_AUTH_STATUS_PHASE_NONE)) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase1 -> invalid state or data length.");
        tuya_ble_gap_disconnect();
        return;
    }

    p_buf[0] = 0;

    tuya_ble_auth_data_reset();

    if (recv_data[13] == 1) {
        if (data_len == 130) {
            memcpy(current_ser_cert_pub_key, recv_data + 15, 64);
            memcpy(sig_data, recv_data + 79, 64);
            TUYA_BLE_LOG_HEXDUMP_DEBUG("received server public key:", current_ser_cert_pub_key, SIZEOF(current_ser_cert_pub_key));
            TUYA_BLE_LOG_HEXDUMP_DEBUG("received server public key signature data:", sig_data, SIZEOF(sig_data));
            if (!tuya_ble_server_cert_data_verify(recv_data + 14, 65, sig_data)) {
                p_buf[0] = 1;
                TUYA_BLE_LOG_ERROR("server public key signature data verify failed.");
            } else {
                TUYA_BLE_LOG_DEBUG("server public key signature data verify succeed.");
            }
        } else {
            TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase1 -> invalid state.");
            tuya_ble_gap_disconnect();
            return;
        }
    } else {
        memcpy(current_ser_cert_pub_key, tuya_ble_current_para.sys_settings.server_cert_pub_key, 64);
        TUYA_BLE_LOG_HEXDUMP_DEBUG("get server public key from tuya ble current para:", current_ser_cert_pub_key, SIZEOF(current_ser_cert_pub_key));
    }

    if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    } else {
        encry_mode = ENCRYPTION_MODE_KEY_2;
    }

    if (p_buf[0] == 0) {
        tuya_ble_rand_generator(current_dev_random_for_sign, 6);
        TUYA_BLE_LOG_HEXDUMP_DEBUG("device generate random:", current_dev_random_for_sign, SIZEOF(current_dev_random_for_sign));
        memcpy(p_buf + 1, current_dev_random_for_sign, 6);
        current_auth_status = TUYA_BLE_AUTH_STATUS_PHASE_1;
        tuya_ble_comm_data_send(FRM_AUTHENTICATE_PHASE_1_RESP, ack_sn, p_buf, 7, encry_mode);
    } else {
        tuya_ble_comm_data_send(FRM_AUTHENTICATE_PHASE_1_RESP, ack_sn, p_buf, 1, encry_mode);
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase1 -> auth failed.");
        tuya_ble_gap_disconnect();
    }
}

STATIC VOID_T tuya_ble_handle_authenticate_phase2(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T encry_mode = 0;
    UINT32_T ack_sn = 0;
    UINT8_T sig_data[64] = {0};
    UINT16_T data_len = 0;
    UINT32_T dev_cert_data_len = 0;

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    data_len = (recv_data[11] << 8) + recv_data[12];

    if ((data_len != 65) || (current_auth_status != TUYA_BLE_AUTH_STATUS_PHASE_1)) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase2 -> invalid state or data length.");
        tuya_ble_gap_disconnect();
        return;
    }
    memset(dev_cert_data,  0, SIZEOF(dev_cert_data));

    dev_cert_data[0] = 0;

    if (recv_data[13] == 0) {
        memcpy(sig_data, recv_data + 14, 64);
        TUYA_BLE_LOG_HEXDUMP_DEBUG("received signature data for device random:", sig_data, SIZEOF(sig_data));
    } else {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase2 -> invalid data type.");
        tuya_ble_gap_disconnect();
        return;
    }

    if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    } else {
        encry_mode = ENCRYPTION_MODE_KEY_2;
    }
    TUYA_BLE_LOG_HEXDUMP_DEBUG("current device random:", current_dev_random_for_sign, SIZEOF(current_dev_random_for_sign));
    TUYA_BLE_LOG_HEXDUMP_DEBUG("verify signature data of device random use server public key:", current_ser_cert_pub_key, SIZEOF(current_ser_cert_pub_key));
    if (!tuya_ble_sig_data_verify(current_ser_cert_pub_key, current_dev_random_for_sign, 6, sig_data)) {
        dev_cert_data[0] = 1;
        tuya_ble_comm_data_send(FRM_AUTHENTICATE_PHASE_2_RESP, ack_sn, dev_cert_data, 1, encry_mode);
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase2 -> device random signature data verify failed.");
        tuya_ble_gap_disconnect();
        return;

    } else {
        TUYA_BLE_LOG_DEBUG("tuya_ble_handle_authenticate_phase2 -> device random signature data verify succeed.");
    }

    if (tuya_ble_current_para.sys_settings.bound_flag == 0) {
        dev_cert_data_len = tuya_ble_get_dev_crt_len();

        if (dev_cert_data_len == 0) {
            TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase2 -> get dev cert length error.");
            tuya_ble_gap_disconnect();
            return;
        } else {
            TUYA_BLE_LOG_DEBUG("tuya_ble_handle_authenticate_phase2 -> get dev cert length = %d .", dev_cert_data_len);
            dev_cert_data[0] = 0;
            dev_cert_data[1] = 0;
            if (tuya_ble_get_dev_crt_der(dev_cert_data + 2, dev_cert_data_len) != TUYA_BLE_SUCCESS) {
                TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase2 -> get dev cert data error.");
                tuya_ble_gap_disconnect();
                return;
            }
            TUYA_BLE_LOG_HEXDUMP_DEBUG("get device cert :", dev_cert_data + 2, dev_cert_data_len);
            tuya_ble_comm_data_send(FRM_AUTHENTICATE_PHASE_2_RESP, ack_sn, dev_cert_data, dev_cert_data_len + 2, encry_mode);
        }
    } else {
        dev_cert_data[0] = 0;
        tuya_ble_comm_data_send(FRM_AUTHENTICATE_PHASE_2_RESP, ack_sn, dev_cert_data, 1, encry_mode);
        TUYA_BLE_LOG_DEBUG("tuya_ble_handle_authenticate_phase2 -> no need send dev cert because in bound state.");
    }

    current_auth_status = TUYA_BLE_AUTH_STATUS_PHASE_2;

}

STATIC VOID_T tuya_ble_handle_authenticate_phase3(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T encry_mode = 0;
    UINT32_T ack_sn = 0;
    UINT8_T p_buf[65] = {0};
    UINT16_T data_len = 0;
    UINT8_T random[6] = {0};
    UINT8_T random_hash[32] = {0};

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    data_len = (recv_data[11] << 8) + recv_data[12];

    if ((data_len != 7 && data_len != 1) || (current_auth_status != TUYA_BLE_AUTH_STATUS_PHASE_2)) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase3 -> invalid state or data length.");
        tuya_ble_gap_disconnect();
        return;
    }

    if (recv_data[13] != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase3 -> dev cert verify failed.");
        tuya_ble_gap_disconnect();
        return;
    } else {
        TUYA_BLE_LOG_HEXDUMP_DEBUG("received server random for signature :", random, 6);
        memcpy(random, &recv_data[14], 6);
    }

    if (tuya_ble_ecc_sign_secp256r1(NULL, random, 6, p_buf + 1) != TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase3 -> server random sign failed.");
        tuya_ble_gap_disconnect();
        return;
    } else {
        TUYA_BLE_LOG_HEXDUMP_DEBUG("tuya_ble_handle_authenticate_phase3 -> server random sign succeed.", p_buf + 1, 64);
    }

    p_buf[0] = 0;

    if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    } else {
        encry_mode = ENCRYPTION_MODE_KEY_2;
    }

    tuya_ble_comm_data_send(FRM_AUTHENTICATE_PHASE_3_RESP, ack_sn, p_buf, 65, encry_mode);

    current_auth_status = TUYA_BLE_AUTH_STATUS_PHASE_3;

}

#endif

STATIC VOID_T tuya_ble_handle_pair_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T p_buf[1];
    UINT8_T encry_mode = 0;
    UINT32_T ack_sn = 0;
    UINT16_T data_len = 0;
    tuya_ble_cb_evt_param_t event;

    encry_mode = recv_data[0];

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    data_len = (recv_data[11] << 8) + recv_data[12];

#if (TUYA_BLE_SECURE_CONNECTION_TYPE == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)

#if (TUYA_BLE_ADVANCED_ENCRYPTION_AUTH_ON_CONNECT)

    if (current_auth_status != TUYA_BLE_AUTH_STATUS_PHASE_3) {
        TUYA_BLE_LOG_ERROR("PAIR_REQ failed, because current_auth_status error!");  //ID not match, and disconnected.
        tuya_ble_gap_disconnect();
        return;
    }

#else

    if (tuya_ble_current_para.sys_settings.bound_flag == 0) {
        if (current_auth_status != TUYA_BLE_AUTH_STATUS_PHASE_3) {
            TUYA_BLE_LOG_ERROR("PAIR_REQ failed, because current_auth_status error!");  //ID not match, and disconnected.
            tuya_ble_gap_disconnect();
            return;
        }
    }

#endif

#endif

    if (0 == memcmp(&recv_data[13], tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN)) {
#if (TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE == 0)
        tuya_ble_connect_monitor_timer_stop();
#endif

#if (TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE && TUYA_BLE_LINK_LAYER_FORCED_ENCRYPTION)
        if (TY_LINK_ENCRYPTED != tuya_ble_link_status_get()) {
            TUYA_BLE_LOG_INFO("Pair failed because link is not encrypted!");
            p_buf[0] = 1;
            goto pair_exit;
        }
#endif

        if (1 == tuya_ble_get_adv_connect_request_bit_status()) {
            TUYA_BLE_LOG_INFO("ble adv data update,because the last broadcast data connection request flag was set!");
            tuya_ble_adv_change();
        }

        if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
            TUYA_BLE_LOG_INFO("PAIR_REQ already bound!");
            p_buf[0] = 2;
            tuya_ble_connect_status_set(BONDING_CONN);

            if (encry_mode == BLE_ENCRYPTION_MODE_KEY_12 && data_len >= 0x0062) {
                memcpy(tuya_ble_current_para.sys_settings.login_key_v2, recv_data + 75, LOGIN_KEY_V2_LEN);
                memcpy(tuya_ble_current_para.sys_settings.secret_key, recv_data + 75 + LOGIN_KEY_V2_LEN, SECRET_KEY_LEN);
                if (tuya_ble_cryption_v2_data_verify(&tuya_ble_current_para, recv_data + 75)) {
                } else {
                    memset(tuya_ble_current_para.sys_settings.login_key_v2, 0, LOGIN_KEY_V2_LEN);
                    memset(tuya_ble_current_para.sys_settings.secret_key, 0, SECRET_KEY_LEN);
                    tuya_ble_gap_disconnect();
                    return;
                }
                tuya_ble_storage_save_sys_settings();
                tuya_ble_adv_change();
            } else if (encry_mode == BLE_ENCRYPTION_MODE_KEY_12 && (recv_data[57] == 0) && data_len >= 0x0052) {
                memcpy(tuya_ble_current_para.sys_settings.login_key_v2, recv_data + 59, LOGIN_KEY_V2_LEN);
                memcpy(tuya_ble_current_para.sys_settings.secret_key, recv_data + 59 + LOGIN_KEY_V2_LEN, SECRET_KEY_LEN);
                if (tuya_ble_cryption_v2_data_verify(&tuya_ble_current_para, recv_data + 59)) {
                } else {
                    memset(tuya_ble_current_para.sys_settings.login_key_v2, 0, LOGIN_KEY_V2_LEN);
                    memset(tuya_ble_current_para.sys_settings.secret_key, 0, SECRET_KEY_LEN);
                    tuya_ble_gap_disconnect();
                    return;
                }
                tuya_ble_storage_save_sys_settings();
                tuya_ble_adv_change();
            }
            TUYA_BLE_LOG_INFO("Re-Bond ok-%d", tuya_ble_current_para.sys_settings.bound_flag);
        } else { // not bond
#if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE)

            memcpy(tuya_ble_current_para.sys_settings.login_key, recv_data + 29, LOGIN_KEY_LEN);
            memcpy(tuya_ble_current_para.sys_settings.device_virtual_id, recv_data + 29 + LOGIN_KEY_LEN, DEVICE_VIRTUAL_ID_LEN);

            if (encry_mode == BLE_ENCRYPTION_MODE_KEY_12 && data_len >= 0x0062) {
                memcpy(tuya_ble_current_para.sys_settings.login_key_v2, recv_data + 75, LOGIN_KEY_V2_LEN);
                memcpy(tuya_ble_current_para.sys_settings.secret_key, recv_data + 75 + LOGIN_KEY_V2_LEN, SECRET_KEY_LEN);
                memcpy(tuya_ble_current_para.sys_settings.login_key, tuya_ble_current_para.sys_settings.login_key_v2, LOGIN_KEY_LEN);
                if (tuya_ble_cryption_v2_data_verify(&tuya_ble_current_para, recv_data + 75)) {
                } else {
                    memset(tuya_ble_current_para.sys_settings.login_key_v2, 0, LOGIN_KEY_V2_LEN);
                    memset(tuya_ble_current_para.sys_settings.secret_key, 0, SECRET_KEY_LEN);
                    tuya_ble_gap_disconnect();
                    return;
                }
            } else if (encry_mode == BLE_ENCRYPTION_MODE_KEY_12 && (recv_data[57] == 0) && data_len >= 0x0052) {
                memcpy(tuya_ble_current_para.sys_settings.login_key_v2, recv_data + 59, LOGIN_KEY_V2_LEN);
                memcpy(tuya_ble_current_para.sys_settings.secret_key, recv_data + 59 + LOGIN_KEY_V2_LEN, SECRET_KEY_LEN);
                memcpy(tuya_ble_current_para.sys_settings.login_key, tuya_ble_current_para.sys_settings.login_key_v2, LOGIN_KEY_LEN);
                if (tuya_ble_cryption_v2_data_verify(&tuya_ble_current_para, recv_data + 59)) {
                } else {
                    memset(tuya_ble_current_para.sys_settings.login_key_v2, 0, LOGIN_KEY_V2_LEN);
                    memset(tuya_ble_current_para.sys_settings.secret_key, 0, SECRET_KEY_LEN);
                    tuya_ble_gap_disconnect();
                    return;
                }
            }

            if (TUYA_BLE_BEACON_KEY_ENABLE) {
                if (data_len < 61) {
                    p_buf[0] = 1;
                    goto pair_exit;
                } else {
                    if (recv_data[57] != BEACON_KEY_LEN) {
                        p_buf[0] = 1;
                        goto pair_exit;
                    } else if (recv_data[57] == BEACON_KEY_LEN) {
                        memcpy(tuya_ble_current_para.sys_settings.beacon_key, recv_data + 58, BEACON_KEY_LEN);
                    }
                }

            }

            tuya_ble_current_para.sys_settings.bound_flag = 1;

#if (TUYA_BLE_SECURE_CONNECTION_TYPE == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)

            if (!tal_util_buffer_value_is_all_x(current_ser_cert_pub_key, 64, 0)) {
                memcpy(tuya_ble_current_para.sys_settings.server_cert_pub_key, current_ser_cert_pub_key, 64);
            } else {
                p_buf[0] = 1;
                tuya_ble_current_para.sys_settings.bound_flag = 0;
                goto pair_exit;
            }

#endif

            uint16_t recv_data_idx = 95;
#if (TUYA_BLE_BEACON_KEY_ENABLE)
            recv_data_idx += 16;
#endif
            uint8_t app_flag_length = recv_data[recv_data_idx];
            recv_data_idx++;
            recv_data_idx += app_flag_length;

#if defined(TUYA_BLE_FEATURE_ENABLE_GROUP) && (TUYA_BLE_FEATURE_ENABLE_GROUP == 1)
            uint8_t node_id_len = recv_data[recv_data_idx];
            recv_data_idx ++;
            if (node_id_len == 2) {
                tuya_ble_current_para.sys_settings.node_id = (recv_data[recv_data_idx] << 8) + recv_data[recv_data_idx + 1];
                recv_data_idx += node_id_len;
            } else {
                TUYA_BLE_LOG_ERROR("node_id_len error");
            }

            uint8_t ble_net_key_len = recv_data[recv_data_idx];
            recv_data_idx ++;
            if (ble_net_key_len == 16) {
                uint32_t tal_ble_group_info_rx(uint8_t *buf, uint32_t size);
                tal_ble_group_info_rx(&recv_data[recv_data_idx - 4], 20);
                recv_data_idx += ble_net_key_len;
            } else {
                TUYA_BLE_LOG_ERROR("ble_net_key_len error");
            }
#endif // TUYA_BLE_FEATURE_ENABLE_GROUP

            tuya_ble_storage_save_sys_settings();

            tuya_ble_connect_status_set(BONDING_CONN);

            tuya_ble_adv_change();
            TUYA_BLE_LOG_INFO("PAIR_REQ ok-%d", tuya_ble_current_para.sys_settings.bound_flag);

            event.evt = TUYA_BLE_CB_EVT_UPDATE_LOGIN_KEY_VID;
            event.device_login_key_vid_data.login_key_len = LOGIN_KEY_LEN;
            event.device_login_key_vid_data.vid_len = DEVICE_VIRTUAL_ID_LEN;

            if (TUYA_BLE_BEACON_KEY_ENABLE) {
                event.device_login_key_vid_data.beacon_key_len = BEACON_KEY_LEN;
                memcpy(event.device_login_key_vid_data.beacon_key, tuya_ble_current_para.sys_settings.beacon_key, BEACON_KEY_LEN);
            }

            memcpy(event.device_login_key_vid_data.login_key, tuya_ble_current_para.sys_settings.login_key, LOGIN_KEY_LEN);
            memcpy(event.device_login_key_vid_data.vid, tuya_ble_current_para.sys_settings.device_virtual_id, DEVICE_VIRTUAL_ID_LEN);

            if (tuya_ble_cb_event_send(&event) != 0) {
                TUYA_BLE_LOG_ERROR("tuya ble send cb event failed.");
            } else {
                TUYA_BLE_LOG_DEBUG("tuya ble send cb event succeed.");
            }
#else
            tuya_ble_connect_status_set(UNBONDING_CONN);
#endif
            p_buf[0] = 0x00;
        }

#if (TUYA_BLE_BEACON_KEY_ENABLE)
        UINT8_T source_type = recv_data[74];
#else
        UINT8_T source_type = recv_data[58];
#endif

        switch (source_type) {
        case MOBILE_APP:
            tuya_ble_connect_source_type_set(MOBILE_APP);
            TUYA_BLE_LOG_INFO("Mobile app connect.");
            break;
        case GATEWAY:
            tuya_ble_connect_source_type_set(GATEWAY);
            TUYA_BLE_LOG_INFO("Gateway connect.");
            break;
        case APPLET:
            tuya_ble_connect_source_type_set(APPLET);
            TUYA_BLE_LOG_INFO("Applet connect.");
            break;
        case LOCK_FITTING:
            tuya_ble_connect_source_type_set(LOCK_FITTING);
            TUYA_BLE_LOG_INFO("Applet connect.");
            break;
        case IOS_APP:
            tuya_ble_connect_source_type_set(IOS_APP);
            TUYA_BLE_LOG_INFO("Applet connect.");
            break;
        default:
            tuya_ble_connect_source_type_set(UNKNOW_TYPES);
            break;
        }

        event.evt = TUYA_BLE_CB_EVT_CONNECT_STATUS;
        event.connect_status = tuya_ble_connect_status_get();
        if (tuya_ble_cb_event_send(&event) != 0) {
            TUYA_BLE_LOG_ERROR("tuya ble send cb event failed.");
        } else {
            TUYA_BLE_LOG_INFO("tuya ble send cb event succeed.");
        }
    } else {
        TUYA_BLE_LOG_ERROR("PAIR_REQ device id not match!");  //ID not match, and disconnected.
        p_buf[0] = 0x01;

    }
pair_exit:
    if (p_buf[0] == 0) {
#if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE)
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
#else
        encry_mode = recv_data[0];
#endif
    } else if (p_buf[0] == 1) {
        encry_mode = recv_data[0];
    } else if (p_buf[0] == 2) {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    } else {

    }

    tuya_ble_comm_data_send(PAIR_RESP, ack_sn, p_buf, 1, encry_mode);

    if (encry_mode == ENCRYPTION_MODE_SESSION_KEY) {
        if (TUYA_BLE_AUTO_REQUEST_TIME_CONFIGURE == 1) {
            tuya_ble_comm_data_send(FRM_GET_UNIX_TIME_CHAR_MS_REQ, 0, NULL, 0, encry_mode);
            TUYA_BLE_LOG_INFO("send FRM_GET_UNIX_TIME_CHAR_MS_REQ cmd to app.\n");
        } else if (TUYA_BLE_AUTO_REQUEST_TIME_CONFIGURE == 2) {
            tuya_ble_comm_data_send(FRM_GET_APP_LOCAL_TIME_REQ, 0, NULL, 0, encry_mode);
            TUYA_BLE_LOG_INFO("send FRM_GET_APP_LOCAL_TIME_REQ cmd to app.\n");
        } else if (TUYA_BLE_AUTO_REQUEST_TIME_CONFIGURE == 3) {
            UINT8_T buf[1] = {1};
            tuya_ble_comm_data_send(FRM_GET_UNIX_TIME_WITH_DST_REQ, 0, buf, SIZEOF(UINT8_T), encry_mode);
            TUYA_BLE_LOG_INFO("send FRM_GET_UNIX_TIME_WITH_DST_REQ cmd to app.\n");
        }
    }

    if (p_buf[0] == 1) {
        tuya_ble_gap_disconnect();
    }

}

#endif  //TUYA_BLE_PROTOCOL_VERSION_HIGN == 5

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN == 4)

__TUYA_BLE_WEAK UINT32_T tuya_ble_device_ability_value(VOID_T)
{
    /*
    --------------------------------------------------------------------------- |
    |       unused     |   p_buf[85]      |    p_buf[86]     |    p_buf[87]     |
    |---------------------------------------------------------------------------|
    |   bit31~bit24    |   bit23~bit16    |   bit15~bit8     |   bit7~bit0      |
    ----------------------------------------------------------------------------|
    */

    return 0;
}

STATIC VOID_T tuya_ble_handle_dev_info_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T p_buf[200];
    UINT8_T payload_len = 0;
    UINT32_T ack_sn = 0;
    UINT8_T encry_mode = 0;
    UINT32_T version_temp_s, version_temp_h;
    tuya_ble_gap_addr_t mac_addr;
    UINT16_T data_len = recv_data[11] << 8 | recv_data[12];
#if (TUYA_BLE_DEVICE_ABILITY_SELF_MANAGEMENT == 0)
    UINT32_T device_ability = 0;
#endif

    if (data_len != 2) {
        TUYA_BLE_LOG_ERROR("get device infor cmd error with v4, data_len = %d", data_len);
        tuya_ble_gap_disconnect();
        return;
    }

    memset(p_buf, 0, SIZEOF(p_buf));

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    TUYA_BLE_LOG_DEBUG("get device infor-%d", tuya_ble_current_para.sys_settings.bound_flag);

    send_packet_data_len = recv_data[13] << 8 | recv_data[14];

    if ((send_packet_data_len == 0) || (send_packet_data_len > TUYA_BLE_DATA_MTU_MAX)) {
        send_packet_data_len = 20;
    }

    TUYA_BLE_LOG_DEBUG("current app mtu data len = %d", send_packet_data_len);

    tuya_ble_rand_generator(tuya_ble_pair_rand, 6);
    tuya_ble_pair_rand_valid = 1;

#if TUYA_BLE_THREE_PART_VERSION_NUMBER_ENABLE
    version_temp_s = tuya_ble_firmware_version >> 8;
    version_temp_h = tuya_ble_hardware_version >> 8;
    p_buf[4] = 0x05;
#else
    version_temp_s = tuya_ble_firmware_version;
    version_temp_h = tuya_ble_hardware_version;
    p_buf[4] = 0x00;
#endif

    p_buf[0] = (version_temp_s >> 8) & 0xff;
    p_buf[1] = (version_temp_s & 0xff);
    p_buf[2] = TUYA_BLE_PROTOCOL_VERSION_HIGN;
    p_buf[3] = TUYA_BLE_PROTOCOL_VERSION_LOW;

#if (TUYA_BLE_SECURE_CONNECTION_TYPE == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)

    p_buf[4] |= 0x02;
    if (TUYA_BLE_ADVANCED_ENCRYPTION_AUTH_ON_CONNECT) {
        p_buf[4] |= 0x08;
    }

#endif

#if (TUYA_BLE_PROTOCOL_VERSION_LOW >= 1)

    if (TUYA_BLE_BEACON_KEY_ENABLE) {
        p_buf[4] |= 0x10;
    }

#endif

    p_buf[4] |= 0x05;

    p_buf[5] = tuya_ble_current_para.sys_settings.bound_flag;
    memcpy(&p_buf[6], tuya_ble_pair_rand, 6);
    p_buf[12] = (version_temp_h >> 8) & 0xff;
    p_buf[13] = (version_temp_h & 0xff);

    tuya_ble_register_key_generate(&p_buf[14], &tuya_ble_current_para);

    p_buf[46] = (tuya_ble_firmware_version >> 16) & 0xff;
    p_buf[47] = (tuya_ble_firmware_version >> 8) & 0xff;
    p_buf[48] = (tuya_ble_firmware_version & 0xff);
    p_buf[49] = (tuya_ble_hardware_version >> 16) & 0xff;
    p_buf[50] = (tuya_ble_hardware_version >> 8) & 0xff;
    p_buf[51] = (tuya_ble_hardware_version & 0xff);

    p_buf[52] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY >> 8;
    p_buf[53] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY & 0xFF; //communication ability

#if (TUYA_BLE_PROTOCOL_VERSION_LOW >= 4)

#if TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE
    if (TY_LINK_ENCRYPTED == tuya_ble_link_status_get()) {
        p_buf[54] |= 0x01;
    }
#endif

#endif

#if (TUYA_BLE_PROTOCOL_VERSION_LOW >= 7)
    p_buf[54] |= 0x02;

    if (tuya_ble_current_para.sys_settings.protocol_v2_enable) {
        p_buf[54] |= 0x04;
    }
#endif

    memcpy(&p_buf[55], tuya_ble_current_para.sys_settings.device_virtual_id, DEVICE_VIRTUAL_ID_LEN);

    p_buf[77] = (tuya_ble_mcu_firmware_version >> 16) & 0xff;
    p_buf[78] = (tuya_ble_mcu_firmware_version >> 8) & 0xff;
    p_buf[79] = (tuya_ble_mcu_firmware_version & 0xff);
    p_buf[80] = (tuya_ble_mcu_hardware_version >> 16) & 0xff;
    p_buf[81] = (tuya_ble_mcu_hardware_version >> 8) & 0xff;
    p_buf[82] = (tuya_ble_mcu_hardware_version & 0xff);

    p_buf[83] = TUYA_BLE_WIFI_DEVICE_REGISTER_MODE;

#if (TUYA_BLE_PROTOCOL_VERSION_LOW >= 2)

    memset(&p_buf[84], 0, 4);

#if (TUYA_BLE_DEVICE_ABILITY_SELF_MANAGEMENT)

#if (TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE)
    p_buf[87] |= 0x08;

    if (TUYA_BLE_LINK_LAYER_FORCED_ENCRYPTION && (TUYA_BLE_PROTOCOL_VERSION_LOW >= 4)) {
        p_buf[87] |= 0x80;
    }
#endif

#if TUYA_BLE_BR_EDR_SUPPORTED
    p_buf[87] |= 0x40;
#endif

#if (TUYA_BLE_FEATURE_EXT_MODULE_ENABLE)
    p_buf[87] |= 0x10;
#endif

#if (TUYA_BLE_PROTOCOL_VERSION_LOW >= 5)

#if TUYA_BLE_ACCESSORY_MOUNT_SUPPORTED
    p_buf[86] |= 0x02;
#endif

#endif

#if TUYA_BLE_FEATURE_LOCAL_TIMER_ENABLE
    p_buf[87] |= 0x20;
#endif

#if TUYA_BLE_SINGLE_BANK_OTA_MODE
    p_buf[85] |= 0x02;
#endif

#if TUYA_BLE_FEATURE_LONG_RANGE
    p_buf[85] |= 0x10;
#endif

#if TUYA_BLE_FEATURE_ENABLE_GROUP
    p_buf[85] |= 0x20;
#endif

#if (TUYA_BLE_PROTOCOL_VERSION_LOW >= 6)

    if (TUYA_BLE_CONNECTIVITY == 0) {
        p_buf[86] &= (~0x10);
        p_buf[86] &= (~0x08);
    } else if (TUYA_BLE_CONNECTIVITY == 1) {
        p_buf[86] &= (~0x10);
        p_buf[86] |= 0x08;
    } else if (TUYA_BLE_CONNECTIVITY == 2) {
        p_buf[86] |= 0x10;
        p_buf[86] &= (~0x08);
    }

#if TUYA_BLE_GATEWAY_CONN_MODE
    p_buf[86] |= 0x20;
#endif

#endif

#else
    device_ability = tuya_ble_device_ability_value();
    p_buf[85] = (device_ability >> 16) & 0xff;
    p_buf[86] = (device_ability >> 8) & 0xff;
    p_buf[87] = (device_ability) & 0xff;
#endif

    tuya_ble_gap_addr_get(&mac_addr);

    p_buf[88] = mac_addr.addr_type;

    memcpy(&p_buf[89], mac_addr.addr, 6);

    p_buf[95] = strlen(TUYA_BLE_APP_FIRMWARE_KEY);

    if ((p_buf[95] == 8) || (p_buf[95] == 16)) {
        memcpy(&p_buf[96], TUYA_BLE_APP_FIRMWARE_KEY, p_buf[95]);
    } else {
        p_buf[95] = 0;
    }

    payload_len = 96 + p_buf[95];

    p_buf[payload_len++] = 0; // zigbee_mac_len

#if defined(TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE) && (TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE == 1)
    p_buf[payload_len++] = tuya_ble_attach_ota_size * 4;

    if ((tuya_ble_attach_ota_size != 0) && (tuya_ble_attach_ota_buf != NULL)) {
        UINT8_T *tmp_buf = tal_malloc(tuya_ble_attach_ota_size * 4);
        if (tmp_buf) {
            for (UINT32_T idx = 0; idx < tuya_ble_attach_ota_size; idx++) {
                tmp_buf[idx * 4 + 0] = tuya_ble_attach_ota_buf[idx * (4 + sizeof(TUYA_OTA_FIRMWARE_INFO_T)) + 0];
                tmp_buf[idx * 4 + 1] = tuya_ble_attach_ota_buf[idx * (4 + sizeof(TUYA_OTA_FIRMWARE_INFO_T)) + 3];
                tmp_buf[idx * 4 + 2] = tuya_ble_attach_ota_buf[idx * (4 + sizeof(TUYA_OTA_FIRMWARE_INFO_T)) + 2];
                tmp_buf[idx * 4 + 3] = tuya_ble_attach_ota_buf[idx * (4 + sizeof(TUYA_OTA_FIRMWARE_INFO_T)) + 1];
            }

            memcpy(&p_buf[payload_len], tmp_buf, tuya_ble_attach_ota_size * 4);
            payload_len += tuya_ble_attach_ota_size * 4;

            tal_free(tmp_buf);
        }
    }
#else
    p_buf[payload_len++] = 0; // attach_len
#endif

    p_buf[payload_len++] = 2; // PacketMaxSize_len

    p_buf[payload_len++] = 5; // PacketMaxSize

    p_buf[payload_len++] = 0; // PacketMaxSize

#else

    p_buf[84] = strlen(TUYA_BLE_APP_FIRMWARE_KEY);

    if ((p_buf[84] == 8) || (p_buf[84] == 16)) {
        memcpy(&p_buf[85], TUYA_BLE_APP_FIRMWARE_KEY, p_buf[84]);
    } else {
        p_buf[84] = 0;
    }

    payload_len = 85 + p_buf[84];

#endif

    encry_mode = recv_data[0];

    if (tuya_ble_comm_data_send(FRM_QRY_DEV_INFO_RESP, ack_sn, p_buf, payload_len,  encry_mode) == 2) {
        tuya_ble_pair_rand_clear();
    } else {
#if TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE
        if (TY_LINK_ENCRYPTED != tuya_ble_link_status_get()) {
            tuya_ble_connect_monitor_timer_stop();
            tuya_ble_link_security_request();
            tuya_ble_link_status_set(TY_LINK_ENCRYPTED_REQUEST);
        }
#endif
    }
}

#if (TUYA_BLE_SECURE_CONNECTION_TYPE == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)

STATIC VOID_T tuya_ble_handle_authenticate_phase1(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T p_buf[7];
    UINT8_T encry_mode = 0;
    UINT32_T ack_sn = 0;
    UINT8_T sig_data[64];
    UINT16_T data_len = 0;

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    data_len = (recv_data[11] << 8) + recv_data[12];

    if (((tuya_ble_current_para.sys_settings.bound_flag == 0) && (recv_data[13] == 0)) || ((tuya_ble_current_para.sys_settings.bound_flag == 1) && (recv_data[13] == 1))) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase1 -> invalid state.");
        tuya_ble_gap_disconnect();
        return;
    }

    if ((data_len != 1 && data_len != 130) || (current_auth_status != TUYA_BLE_AUTH_STATUS_PHASE_NONE)) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase1 -> invalid state or data length.");
        tuya_ble_gap_disconnect();
        return;
    }

    p_buf[0] = 0;

    tuya_ble_auth_data_reset();

    if (recv_data[13] == 1) {
        if (data_len == 130) {
            memcpy(current_ser_cert_pub_key, recv_data + 15, 64);
            memcpy(sig_data, recv_data + 79, 64);
            TUYA_BLE_LOG_HEXDUMP_DEBUG("received server public key:", current_ser_cert_pub_key, SIZEOF(current_ser_cert_pub_key));
            TUYA_BLE_LOG_HEXDUMP_DEBUG("received server public key signature data:", sig_data, SIZEOF(sig_data));
            if (!tuya_ble_server_cert_data_verify(recv_data + 14, 65, sig_data)) {
                p_buf[0] = 1;
                TUYA_BLE_LOG_ERROR("server public key signature data verify failed.");
            } else {
                TUYA_BLE_LOG_DEBUG("server public key signature data verify succeed.");
            }
        } else {
            TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase1 -> invalid state.");
            tuya_ble_gap_disconnect();
            return;
        }
    } else {
        memcpy(current_ser_cert_pub_key, tuya_ble_current_para.sys_settings.server_cert_pub_key, 64);
        TUYA_BLE_LOG_HEXDUMP_DEBUG("get server public key from tuya ble current para:", current_ser_cert_pub_key, SIZEOF(current_ser_cert_pub_key));
    }

    if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    } else {
        encry_mode = ENCRYPTION_MODE_KEY_2;
    }

    if (p_buf[0] == 0) {
        tuya_ble_rand_generator(current_dev_random_for_sign, 6);
        TUYA_BLE_LOG_HEXDUMP_DEBUG("device generate random:", current_dev_random_for_sign, SIZEOF(current_dev_random_for_sign));
        memcpy(p_buf + 1, current_dev_random_for_sign, 6);
        current_auth_status = TUYA_BLE_AUTH_STATUS_PHASE_1;
        tuya_ble_comm_data_send(FRM_AUTHENTICATE_PHASE_1_RESP, ack_sn, p_buf, 7, encry_mode);
    } else {
        tuya_ble_comm_data_send(FRM_AUTHENTICATE_PHASE_1_RESP, ack_sn, p_buf, 1, encry_mode);
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase1 -> auth failed.");
        tuya_ble_gap_disconnect();
    }
}

STATIC VOID_T tuya_ble_handle_authenticate_phase2(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T encry_mode = 0;
    UINT32_T ack_sn = 0;
    UINT8_T sig_data[64] = {0};
    UINT16_T data_len = 0;
    UINT32_T dev_cert_data_len = 0;

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    data_len = (recv_data[11] << 8) + recv_data[12];

    if ((data_len != 65) || (current_auth_status != TUYA_BLE_AUTH_STATUS_PHASE_1)) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase2 -> invalid state or data length.");
        tuya_ble_gap_disconnect();
        return;
    }
    memset(dev_cert_data,  0, SIZEOF(dev_cert_data));

    dev_cert_data[0] = 0;

    if (recv_data[13] == 0) {
        memcpy(sig_data, recv_data + 14, 64);
        TUYA_BLE_LOG_HEXDUMP_DEBUG("received signature data for device random:", sig_data, SIZEOF(sig_data));
    } else {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase2 -> invalid data type.");
        tuya_ble_gap_disconnect();
        return;
    }

    if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    } else {
        encry_mode = ENCRYPTION_MODE_KEY_2;
    }
    TUYA_BLE_LOG_HEXDUMP_DEBUG("current device random:", current_dev_random_for_sign, SIZEOF(current_dev_random_for_sign));
    TUYA_BLE_LOG_HEXDUMP_DEBUG("verify signature data of device random use server public key:", current_ser_cert_pub_key, SIZEOF(current_ser_cert_pub_key));
    if (!tuya_ble_sig_data_verify(current_ser_cert_pub_key, current_dev_random_for_sign, 6, sig_data)) {
        dev_cert_data[0] = 1;
        tuya_ble_comm_data_send(FRM_AUTHENTICATE_PHASE_2_RESP, ack_sn, dev_cert_data, 1, encry_mode);
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase2 -> device random signature data verify failed.");
        tuya_ble_gap_disconnect();
        return;

    } else {
        TUYA_BLE_LOG_DEBUG("tuya_ble_handle_authenticate_phase2 -> device random signature data verify succeed.");
    }

    if (tuya_ble_current_para.sys_settings.bound_flag == 0) {
        dev_cert_data_len = tuya_ble_get_dev_crt_len();

        if (dev_cert_data_len == 0) {
            TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase2 -> get dev cert length error.");
            tuya_ble_gap_disconnect();
            return;
        } else {
            TUYA_BLE_LOG_DEBUG("tuya_ble_handle_authenticate_phase2 -> get dev cert length = %d .", dev_cert_data_len);
            dev_cert_data[0] = 0;
            dev_cert_data[1] = 0;
            if (tuya_ble_get_dev_crt_der(dev_cert_data + 2, dev_cert_data_len) != TUYA_BLE_SUCCESS) {
                TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase2 -> get dev cert data error.");
                tuya_ble_gap_disconnect();
                return;
            }
            TUYA_BLE_LOG_HEXDUMP_DEBUG("get device cert :", dev_cert_data + 2, dev_cert_data_len);
            tuya_ble_comm_data_send(FRM_AUTHENTICATE_PHASE_2_RESP, ack_sn, dev_cert_data, dev_cert_data_len + 2, encry_mode);
        }
    } else {
        dev_cert_data[0] = 0;
        tuya_ble_comm_data_send(FRM_AUTHENTICATE_PHASE_2_RESP, ack_sn, dev_cert_data, 1, encry_mode);
        TUYA_BLE_LOG_DEBUG("tuya_ble_handle_authenticate_phase2 -> no need send dev cert because in bound state.");
    }

    current_auth_status = TUYA_BLE_AUTH_STATUS_PHASE_2;

}

STATIC VOID_T tuya_ble_handle_authenticate_phase3(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T encry_mode = 0;
    UINT32_T ack_sn = 0;
    UINT8_T p_buf[65] = {0};
    UINT16_T data_len = 0;
    UINT8_T random[6] = {0};
    UINT8_T random_hash[32] = {0};

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    data_len = (recv_data[11] << 8) + recv_data[12];

    if ((data_len != 7 && data_len != 1) || (current_auth_status != TUYA_BLE_AUTH_STATUS_PHASE_2)) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase3 -> invalid state or data length.");
        tuya_ble_gap_disconnect();
        return;
    }

    if (recv_data[13] != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase3 -> dev cert verify failed.");
        tuya_ble_gap_disconnect();
        return;
    } else {
        TUYA_BLE_LOG_HEXDUMP_DEBUG("received server random for signature :", random, 6);
        memcpy(random, &recv_data[14], 6);
    }

    if (tuya_ble_ecc_sign_secp256r1(NULL, random, 6, p_buf + 1) != TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_authenticate_phase3 -> server random sign failed.");
        tuya_ble_gap_disconnect();
        return;
    } else {
        TUYA_BLE_LOG_HEXDUMP_DEBUG("tuya_ble_handle_authenticate_phase3 -> server random sign succeed.", p_buf + 1, 64);
    }

    p_buf[0] = 0;

    if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    } else {
        encry_mode = ENCRYPTION_MODE_KEY_2;
    }

    tuya_ble_comm_data_send(FRM_AUTHENTICATE_PHASE_3_RESP, ack_sn, p_buf, 65, encry_mode);

    current_auth_status = TUYA_BLE_AUTH_STATUS_PHASE_3;

}

#endif

STATIC VOID_T tuya_ble_handle_pair_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T p_buf[1];
    UINT8_T encry_mode = 0;
    UINT32_T ack_sn = 0;
    UINT16_T data_len = 0;
    tuya_ble_cb_evt_param_t event;

    encry_mode = recv_data[0];

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    data_len = (recv_data[11] << 8) + recv_data[12];

#if (TUYA_BLE_SECURE_CONNECTION_TYPE == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)

#if (TUYA_BLE_ADVANCED_ENCRYPTION_AUTH_ON_CONNECT)

    if (current_auth_status != TUYA_BLE_AUTH_STATUS_PHASE_3) {
        TUYA_BLE_LOG_ERROR("PAIR_REQ failed, because current_auth_status error!");  //ID not match, and disconnected.
        tuya_ble_gap_disconnect();
        return;
    }

#else

    if (tuya_ble_current_para.sys_settings.bound_flag == 0) {
        if (current_auth_status != TUYA_BLE_AUTH_STATUS_PHASE_3) {
            TUYA_BLE_LOG_ERROR("PAIR_REQ failed, because current_auth_status error!");  //ID not match, and disconnected.
            tuya_ble_gap_disconnect();
            return;
        }
    }

#endif

#endif

    if (0 == memcmp(&recv_data[13], tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN)) {
#if (TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE == 0)
        tuya_ble_connect_monitor_timer_stop();
#endif

#if (TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE && TUYA_BLE_LINK_LAYER_FORCED_ENCRYPTION)
        if (TY_LINK_ENCRYPTED != tuya_ble_link_status_get()) {
            TUYA_BLE_LOG_INFO("Pair failed because link is not encrypted!");
            p_buf[0] = 1;
            goto pair_exit;
        }
#endif

        if (1 == tuya_ble_get_adv_connect_request_bit_status()) {
            TUYA_BLE_LOG_INFO("ble adv data update,because the last broadcast data connection request flag was set!");
            tuya_ble_adv_change();
        }

        if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
            TUYA_BLE_LOG_INFO("PAIR_REQ already bound!");
            p_buf[0] = 2;
            tuya_ble_connect_status_set(BONDING_CONN);
#if (TUYA_BLE_PROTOCOL_VERSION_LOW >= 7)
            if (encry_mode == BLE_ENCRYPTION_MODE_KEY_12 && data_len >= 0x0062) {
                memcpy(tuya_ble_current_para.sys_settings.login_key_v2, recv_data + 75, LOGIN_KEY_V2_LEN);
                memcpy(tuya_ble_current_para.sys_settings.secret_key, recv_data + 75 + LOGIN_KEY_V2_LEN, SECRET_KEY_LEN);
                if (tuya_ble_cryption_v2_data_verify(&tuya_ble_current_para, recv_data + 75)) {
                    tuya_ble_current_para.sys_settings.protocol_v2_enable = TRUE;
                } else {
                    memset(tuya_ble_current_para.sys_settings.login_key_v2, 0, LOGIN_KEY_V2_LEN);
                    memset(tuya_ble_current_para.sys_settings.secret_key, 0, SECRET_KEY_LEN);
                    tuya_ble_current_para.sys_settings.protocol_v2_enable = FALSE;
                    tuya_ble_gap_disconnect();
                    return;
                }
                tuya_ble_storage_save_sys_settings();
                tuya_ble_adv_change();
            } else if (encry_mode == BLE_ENCRYPTION_MODE_KEY_12 && (recv_data[57] == 0) && data_len >= 0x0052) {
                memcpy(tuya_ble_current_para.sys_settings.login_key_v2, recv_data + 59, LOGIN_KEY_V2_LEN);
                memcpy(tuya_ble_current_para.sys_settings.secret_key, recv_data + 59 + LOGIN_KEY_V2_LEN, SECRET_KEY_LEN);
                if (tuya_ble_cryption_v2_data_verify(&tuya_ble_current_para, recv_data + 59)) {
                    tuya_ble_current_para.sys_settings.protocol_v2_enable = TRUE;
                } else {
                    memset(tuya_ble_current_para.sys_settings.login_key_v2, 0, LOGIN_KEY_V2_LEN);
                    memset(tuya_ble_current_para.sys_settings.secret_key, 0, SECRET_KEY_LEN);
                    tuya_ble_current_para.sys_settings.protocol_v2_enable = FALSE;
                    tuya_ble_gap_disconnect();
                    return;
                }
                tuya_ble_storage_save_sys_settings();
                tuya_ble_adv_change();
            }
#endif
            TUYA_BLE_LOG_INFO("Re-Bond ok-%d, v2: %d", tuya_ble_current_para.sys_settings.bound_flag, tuya_ble_current_para.sys_settings.protocol_v2_enable);
        } else { // not bond
#if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE)

#if (TUYA_BLE_PROTOCOL_VERSION_LOW >= 1)

            if (TUYA_BLE_BEACON_KEY_ENABLE) {
                if (data_len < 61) {
                    p_buf[0] = 1;
                    goto pair_exit;
                } else {
                    if (recv_data[57] != BEACON_KEY_LEN) {
                        p_buf[0] = 1;
#if (TUYA_BLE_PROTOCOL_VERSION_LOW < 7)
                        goto pair_exit;
#endif
                    } else if (recv_data[57] == BEACON_KEY_LEN) {
                        memcpy(tuya_ble_current_para.sys_settings.beacon_key, recv_data + 58, BEACON_KEY_LEN);
                    }
                }

            }

#endif

            memcpy(tuya_ble_current_para.sys_settings.login_key, recv_data + 29, LOGIN_KEY_LEN);
            memcpy(tuya_ble_current_para.sys_settings.device_virtual_id, recv_data + 29 + LOGIN_KEY_LEN, DEVICE_VIRTUAL_ID_LEN);

#if (TUYA_BLE_PROTOCOL_VERSION_LOW >= 7)
            if (encry_mode == BLE_ENCRYPTION_MODE_KEY_12 && data_len >= 0x0062) {
                memcpy(tuya_ble_current_para.sys_settings.login_key_v2, recv_data + 75, LOGIN_KEY_V2_LEN);
                memcpy(tuya_ble_current_para.sys_settings.secret_key, recv_data + 75 + LOGIN_KEY_V2_LEN, SECRET_KEY_LEN);
                memcpy(tuya_ble_current_para.sys_settings.login_key, tuya_ble_current_para.sys_settings.login_key_v2, LOGIN_KEY_LEN);
                if (tuya_ble_cryption_v2_data_verify(&tuya_ble_current_para, recv_data + 75)) {
                    tuya_ble_current_para.sys_settings.protocol_v2_enable = TRUE;
                } else {
                    memset(tuya_ble_current_para.sys_settings.login_key_v2, 0, LOGIN_KEY_V2_LEN);
                    memset(tuya_ble_current_para.sys_settings.secret_key, 0, SECRET_KEY_LEN);
                    tuya_ble_current_para.sys_settings.protocol_v2_enable = FALSE;
                    tuya_ble_gap_disconnect();
                    return;
                }
            } else if (encry_mode == BLE_ENCRYPTION_MODE_KEY_12 && (recv_data[57] == 0) && data_len >= 0x0052) {
                memcpy(tuya_ble_current_para.sys_settings.login_key_v2, recv_data + 59, LOGIN_KEY_V2_LEN);
                memcpy(tuya_ble_current_para.sys_settings.secret_key, recv_data + 59 + LOGIN_KEY_V2_LEN, SECRET_KEY_LEN);
                memcpy(tuya_ble_current_para.sys_settings.login_key, tuya_ble_current_para.sys_settings.login_key_v2, LOGIN_KEY_LEN);
                if (tuya_ble_cryption_v2_data_verify(&tuya_ble_current_para, recv_data + 59)) {
                    tuya_ble_current_para.sys_settings.protocol_v2_enable = TRUE;
                } else {
                    memset(tuya_ble_current_para.sys_settings.login_key_v2, 0, LOGIN_KEY_V2_LEN);
                    memset(tuya_ble_current_para.sys_settings.secret_key, 0, SECRET_KEY_LEN);
                    tuya_ble_current_para.sys_settings.protocol_v2_enable = FALSE;
                    tuya_ble_gap_disconnect();
                    return;
                }
            }
#endif
            tuya_ble_current_para.sys_settings.bound_flag = 1;

#if (TUYA_BLE_SECURE_CONNECTION_TYPE == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)

            if (!tal_util_buffer_value_is_all_x(current_ser_cert_pub_key, 64, 0)) {
                memcpy(tuya_ble_current_para.sys_settings.server_cert_pub_key, current_ser_cert_pub_key, 64);
            } else {
                p_buf[0] = 1;
                tuya_ble_current_para.sys_settings.bound_flag = 0;
                goto pair_exit;
            }

#endif

#if (TUYA_BLE_PROTOCOL_VERSION_LOW >= 8)

            uint16_t recv_data_idx = 95;
#if (TUYA_BLE_BEACON_KEY_ENABLE)
            recv_data_idx += 16;
#endif
            uint8_t app_flag_length = recv_data[recv_data_idx];
            recv_data_idx++;
            recv_data_idx += app_flag_length;

#if defined(TUYA_BLE_FEATURE_ENABLE_GROUP) && (TUYA_BLE_FEATURE_ENABLE_GROUP == 1)
            uint8_t node_id_len = recv_data[recv_data_idx];
            recv_data_idx ++;
            if (node_id_len == 2) {
                tuya_ble_current_para.sys_settings.node_id = (recv_data[recv_data_idx] << 8) + recv_data[recv_data_idx + 1];
                recv_data_idx += node_id_len;
            } else {
                TUYA_BLE_LOG_ERROR("node_id_len error");
            }

            uint8_t ble_net_key_len = recv_data[recv_data_idx];
            recv_data_idx ++;
            if (ble_net_key_len == 16) {
                uint32_t tal_ble_group_info_rx(uint8_t *buf, uint32_t size);
                tal_ble_group_info_rx(&recv_data[recv_data_idx - 4], 20);
                recv_data_idx += ble_net_key_len;
            } else {
                TUYA_BLE_LOG_ERROR("ble_net_key_len error");
            }
#endif // TUYA_BLE_FEATURE_ENABLE_GROUP
#endif // TUYA_BLE_PROTOCOL_VERSION_LOW

            tuya_ble_storage_save_sys_settings();

            tuya_ble_connect_status_set(BONDING_CONN);

            tuya_ble_adv_change();
            TUYA_BLE_LOG_INFO("PAIR_REQ ok-%d, v2: %d", tuya_ble_current_para.sys_settings.bound_flag, tuya_ble_current_para.sys_settings.protocol_v2_enable);

            event.evt = TUYA_BLE_CB_EVT_UPDATE_LOGIN_KEY_VID;
            event.device_login_key_vid_data.login_key_len = LOGIN_KEY_LEN;
            event.device_login_key_vid_data.vid_len = DEVICE_VIRTUAL_ID_LEN;

#if (TUYA_BLE_PROTOCOL_VERSION_LOW >= 1)
            if (TUYA_BLE_BEACON_KEY_ENABLE) {
                event.device_login_key_vid_data.beacon_key_len = BEACON_KEY_LEN;
                memcpy(event.device_login_key_vid_data.beacon_key, tuya_ble_current_para.sys_settings.beacon_key, BEACON_KEY_LEN);
            }
#endif

            memcpy(event.device_login_key_vid_data.login_key, tuya_ble_current_para.sys_settings.login_key, LOGIN_KEY_LEN);
            memcpy(event.device_login_key_vid_data.vid, tuya_ble_current_para.sys_settings.device_virtual_id, DEVICE_VIRTUAL_ID_LEN);

            if (tuya_ble_cb_event_send(&event) != 0) {
                TUYA_BLE_LOG_ERROR("tuya ble send cb event failed.");
            } else {
                TUYA_BLE_LOG_DEBUG("tuya ble send cb event succeed.");
            }
#else
            tuya_ble_connect_status_set(UNBONDING_CONN);
#endif
            p_buf[0] = 0x00;
        }

#if (TUYA_BLE_PROTOCOL_VERSION_LOW >= 5)
#if (TUYA_BLE_BEACON_KEY_ENABLE)
        UINT8_T source_type = recv_data[74];
#else
        UINT8_T source_type = recv_data[58];
#endif

        switch (source_type) {
        case MOBILE_APP:
            tuya_ble_connect_source_type_set(MOBILE_APP);
            TUYA_BLE_LOG_INFO("Mobile app connect.");
            break;
        case GATEWAY:
            tuya_ble_connect_source_type_set(GATEWAY);
            TUYA_BLE_LOG_INFO("Gateway connect.");
            break;
        case APPLET:
            tuya_ble_connect_source_type_set(APPLET);
            TUYA_BLE_LOG_INFO("Applet connect.");
            break;
        case LOCK_FITTING:
            tuya_ble_connect_source_type_set(LOCK_FITTING);
            TUYA_BLE_LOG_INFO("Applet connect.");
            break;
        case IOS_APP:
            tuya_ble_connect_source_type_set(IOS_APP);
            TUYA_BLE_LOG_INFO("Applet connect.");
            break;
        default:
            tuya_ble_connect_source_type_set(UNKNOW_TYPES);
            break;
        }
#endif

        event.evt = TUYA_BLE_CB_EVT_CONNECT_STATUS;
        event.connect_status = tuya_ble_connect_status_get();
        if (tuya_ble_cb_event_send(&event) != 0) {
            TUYA_BLE_LOG_ERROR("tuya ble send cb event failed.");
        } else {
            TUYA_BLE_LOG_INFO("tuya ble send cb event succeed.");
        }
    } else {
        TUYA_BLE_LOG_ERROR("PAIR_REQ device id not match!");  //ID not match, and disconnected.
        p_buf[0] = 0x01;

    }

pair_exit:
    if (p_buf[0] == 0) {
#if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE)
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
#else
        encry_mode = recv_data[0];
#endif
    } else if (p_buf[0] == 1) {
        encry_mode = recv_data[0];
    } else if (p_buf[0] == 2) {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    } else {

    }

    tuya_ble_comm_data_send(PAIR_RESP, ack_sn, p_buf, 1, encry_mode);

    if (encry_mode == ENCRYPTION_MODE_SESSION_KEY) {
        if (TUYA_BLE_AUTO_REQUEST_TIME_CONFIGURE == 1) {
            tuya_ble_comm_data_send(FRM_GET_UNIX_TIME_CHAR_MS_REQ, 0, NULL, 0, encry_mode);
            TUYA_BLE_LOG_INFO("send FRM_GET_UNIX_TIME_CHAR_MS_REQ cmd to app.\n");
        } else if (TUYA_BLE_AUTO_REQUEST_TIME_CONFIGURE == 2) {
            tuya_ble_comm_data_send(FRM_GET_APP_LOCAL_TIME_REQ, 0, NULL, 0, encry_mode);
            TUYA_BLE_LOG_INFO("send FRM_GET_APP_LOCAL_TIME_REQ cmd to app.\n");
        } else if (TUYA_BLE_AUTO_REQUEST_TIME_CONFIGURE == 3) {
            UINT8_T buf[1] = {1};
            tuya_ble_comm_data_send(FRM_GET_UNIX_TIME_WITH_DST_REQ, 0, buf, SIZEOF(UINT8_T), encry_mode);
            TUYA_BLE_LOG_INFO("send FRM_GET_UNIX_TIME_WITH_DST_REQ cmd to app.\n");
        }
    }

    if (p_buf[0] == 1) {
        tuya_ble_gap_disconnect();
    }

}

#endif  //TUYA_BLE_PROTOCOL_VERSION_HIGN == 4

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN == 3)

#if (TUYA_BLE_PROTOCOL_VERSION_LOW >= 2)

STATIC VOID_T tuya_ble_handle_dev_info_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T p_buf[100];
    UINT8_T payload_len = 0;
    UINT32_T ack_sn = 0;
    UINT8_T encry_mode = 0;
    UINT32_T version_temp_s, version_temp_h;

    memset(p_buf, 0, SIZEOF(p_buf));

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    TUYA_BLE_LOG_DEBUG("get device infor-%d,v2: %d", tuya_ble_current_para.sys_settings.bound_flag, tuya_ble_current_para.sys_settings.protocol_v2_enable);

    send_packet_data_len = 20;

    tuya_ble_rand_generator(tuya_ble_pair_rand, 6);
    tuya_ble_pair_rand_valid = 1;

    if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE) {
        version_temp_s = tuya_ble_firmware_version;
        version_temp_h = tuya_ble_hardware_version;
        p_buf[4] = 0x00;
    } else {
        version_temp_s = tuya_ble_firmware_version >> 8;
        version_temp_h = tuya_ble_hardware_version >> 8;
        p_buf[4] = 0x05;
    }
    p_buf[0] = (version_temp_s >> 8) & 0xff;
    p_buf[1] = (version_temp_s & 0xff);
    p_buf[2] = TUYA_BLE_PROTOCOL_VERSION_HIGN;
    p_buf[3] = TUYA_BLE_PROTOCOL_VERSION_LOW;
    if (TUYA_BLE_ADVANCED_ENCRYPTION_DEVICE == 1) {
        p_buf[4] |= 0x02;
    }
    p_buf[4] |= 0x40;
    if (tuya_ble_current_para.sys_settings.protocol_v2_enable) {
        p_buf[4] |= 0x80 ;
    } else {
        p_buf[4] &= (~0x80);
    }

    p_buf[5] = tuya_ble_current_para.sys_settings.bound_flag;
    memcpy(&p_buf[6], tuya_ble_pair_rand, 6);
    p_buf[12] = (version_temp_h >> 8) & 0xff;
    p_buf[13] = (version_temp_h & 0xff);

    tuya_ble_register_key_generate(&p_buf[14], &tuya_ble_current_para);

    p_buf[46] = (tuya_ble_firmware_version >> 16) & 0xff;
    p_buf[47] = (tuya_ble_firmware_version >> 8) & 0xff;
    p_buf[48] = (tuya_ble_firmware_version & 0xff);
    p_buf[49] = (tuya_ble_hardware_version >> 16) & 0xff;
    p_buf[50] = (tuya_ble_hardware_version >> 8) & 0xff;
    p_buf[51] = (tuya_ble_hardware_version & 0xff);

    p_buf[52] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY >> 8;
    p_buf[53] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY & 0xFF; //communication ability

    p_buf[54] = 0x00;

    memcpy(&p_buf[55], tuya_ble_current_para.sys_settings.device_virtual_id, DEVICE_VIRTUAL_ID_LEN);

    p_buf[77] = (tuya_ble_mcu_firmware_version >> 16) & 0xff;
    p_buf[78] = (tuya_ble_mcu_firmware_version >> 8) & 0xff;
    p_buf[79] = (tuya_ble_mcu_firmware_version & 0xff);
    p_buf[80] = (tuya_ble_mcu_hardware_version >> 16) & 0xff;
    p_buf[81] = (tuya_ble_mcu_hardware_version >> 8) & 0xff;
    p_buf[82] = (tuya_ble_mcu_hardware_version & 0xff);

    p_buf[83] = TUYA_BLE_WIFI_DEVICE_REGISTER_MODE;

    payload_len = 99;

    encry_mode = recv_data[0];

    if (tuya_ble_comm_data_send(FRM_QRY_DEV_INFO_RESP, ack_sn, p_buf, payload_len,  encry_mode) == 2) {
        tuya_ble_pair_rand_clear();
    }
}

#else //version <= 3.1 TUYA_BLE_PROTOCOL_VERSION_LOW<2

STATIC VOID_T tuya_ble_handle_dev_info_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T p_buf[100];
    UINT8_T payload_len = 0;
    UINT32_T ack_sn = 0;
    UINT8_T encry_mode = 0;
    UINT32_T version_temp_s, version_temp_h;

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    TUYA_BLE_LOG_DEBUG("get device infor-%d", tuya_ble_current_para.sys_settings.bound_flag);

    send_packet_data_len = 20;

    tuya_ble_rand_generator(tuya_ble_pair_rand, 6);
    tuya_ble_pair_rand_valid = 1;

    if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE) {
        version_temp_s = tuya_ble_firmware_version;
        version_temp_h = tuya_ble_hardware_version;
        p_buf[4] = 0x00;
    } else {
        version_temp_s = tuya_ble_firmware_version >> 8;
        version_temp_h = tuya_ble_hardware_version >> 8;
        p_buf[4] = 0x05;
    }
    p_buf[0] = (version_temp_s >> 8) & 0xff;
    p_buf[1] = (version_temp_s & 0xff);
    p_buf[2] = TUYA_BLE_PROTOCOL_VERSION_HIGN;
    p_buf[3] = TUYA_BLE_PROTOCOL_VERSION_LOW;
    if (TUYA_BLE_ADVANCED_ENCRYPTION_DEVICE == 1) {
        p_buf[4] |= 0x02;
    }

    p_buf[5] = tuya_ble_current_para.sys_settings.bound_flag;
    memcpy(&p_buf[6], tuya_ble_pair_rand, 6);
    p_buf[12] = (version_temp_h >> 8) & 0xff;
    p_buf[13] = (version_temp_h & 0xff);
    memcpy(&p_buf[14], tuya_ble_current_para.auth_settings.auth_key, AUTH_KEY_LEN);

    p_buf[46] = (tuya_ble_firmware_version >> 16) & 0xff;
    p_buf[47] = (tuya_ble_firmware_version >> 8) & 0xff;
    p_buf[48] = (tuya_ble_firmware_version & 0xff);
    p_buf[49] = (tuya_ble_hardware_version >> 16) & 0xff;
    p_buf[50] = (tuya_ble_hardware_version >> 8) & 0xff;
    p_buf[51] = (tuya_ble_hardware_version & 0xff);

    p_buf[52] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY >> 8;
    p_buf[53] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY & 0xFF; //communication ability

    p_buf[54] = 0x00;

    memcpy(&p_buf[55], tuya_ble_current_para.sys_settings.device_virtual_id, DEVICE_VIRTUAL_ID_LEN);

    p_buf[77] = (tuya_ble_mcu_firmware_version >> 16) & 0xff;
    p_buf[78] = (tuya_ble_mcu_firmware_version >> 8) & 0xff;
    p_buf[79] = (tuya_ble_mcu_firmware_version & 0xff);
    p_buf[80] = (tuya_ble_mcu_hardware_version >> 16) & 0xff;
    p_buf[81] = (tuya_ble_mcu_hardware_version >> 8) & 0xff;
    p_buf[82] = (tuya_ble_mcu_hardware_version & 0xff);

    payload_len = 99;

    encry_mode = recv_data[0];

    if (tuya_ble_comm_data_send(FRM_QRY_DEV_INFO_RESP, ack_sn, p_buf, payload_len,  encry_mode) == 2) {
        tuya_ble_pair_rand_clear();
    }
}

#endif

STATIC VOID_T tuya_ble_handle_pair_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T p_buf[1];
    UINT8_T encry_mode = 0;
    UINT32_T ack_sn = 0;
    UINT16_T data_len = 0;
    tuya_ble_cb_evt_param_t event;

    encry_mode = recv_data[0];

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];
    data_len = (recv_data[11] << 8) + recv_data[12];

    if (0 == memcmp(&recv_data[13], tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN)) {
        tuya_ble_connect_monitor_timer_stop();

        if (1 == tuya_ble_get_adv_connect_request_bit_status()) {
            TUYA_BLE_LOG_INFO("ble adv data update,because the last broadcast data connection request flag was set!");
            tuya_ble_adv_change();
        }

        if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
            TUYA_BLE_LOG_INFO("PAIR_REQ already bound!");
            p_buf[0] = 2;
            tuya_ble_connect_status_set(BONDING_CONN);
#if (TUYA_BLE_PROTOCOL_VERSION_LOW >= 8)
            if (encry_mode == BLE_ENCRYPTION_MODE_KEY_12 && data_len >= 0x0050) {
                memcpy(tuya_ble_current_para.sys_settings.login_key_v2, recv_data + 57, LOGIN_KEY_V2_LEN);
                memcpy(tuya_ble_current_para.sys_settings.secret_key, recv_data + 57 + LOGIN_KEY_V2_LEN, SECRET_KEY_LEN);
                if (tuya_ble_cryption_v2_data_verify(&tuya_ble_current_para, recv_data + 57)) {
                    tuya_ble_current_para.sys_settings.protocol_v2_enable = TRUE;
                } else {
                    memset(tuya_ble_current_para.sys_settings.login_key_v2, 0, LOGIN_KEY_V2_LEN);
                    memset(tuya_ble_current_para.sys_settings.secret_key, 0, SECRET_KEY_LEN);
                    tuya_ble_current_para.sys_settings.protocol_v2_enable = FALSE;
                    tuya_ble_gap_disconnect();
                    return;
                }
                tuya_ble_storage_save_sys_settings();
                tuya_ble_adv_change();
            }
#endif
        } else {
#if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE)
            memcpy(tuya_ble_current_para.sys_settings.login_key, recv_data + 29, LOGIN_KEY_LEN);
            memcpy(tuya_ble_current_para.sys_settings.device_virtual_id, recv_data + 29 + LOGIN_KEY_LEN, DEVICE_VIRTUAL_ID_LEN);
#if (TUYA_BLE_PROTOCOL_VERSION_LOW >= 8)
            if (encry_mode == BLE_ENCRYPTION_MODE_KEY_12 && data_len >= 0x0050) {
                memcpy(tuya_ble_current_para.sys_settings.login_key_v2, recv_data + 57, LOGIN_KEY_V2_LEN);
                memcpy(tuya_ble_current_para.sys_settings.secret_key, recv_data + 57 + LOGIN_KEY_V2_LEN, SECRET_KEY_LEN);
                memcpy(tuya_ble_current_para.sys_settings.login_key, tuya_ble_current_para.sys_settings.login_key_v2, LOGIN_KEY_LEN);
                if (tuya_ble_cryption_v2_data_verify(&tuya_ble_current_para, recv_data + 57)) {
                    tuya_ble_current_para.sys_settings.protocol_v2_enable = TRUE;
                } else {
                    memset(tuya_ble_current_para.sys_settings.login_key_v2, 0, LOGIN_KEY_V2_LEN);
                    memset(tuya_ble_current_para.sys_settings.secret_key, 0, SECRET_KEY_LEN);
                    tuya_ble_current_para.sys_settings.protocol_v2_enable = FALSE;
                    tuya_ble_gap_disconnect();
                    return;
                }
            }
#endif
            tuya_ble_current_para.sys_settings.bound_flag = 1;

            tuya_ble_storage_save_sys_settings();

            tuya_ble_connect_status_set(BONDING_CONN);

            tuya_ble_adv_change();
            TUYA_BLE_LOG_INFO("PAIR_REQ ok-%d", tuya_ble_current_para.sys_settings.bound_flag);

            event.evt = TUYA_BLE_CB_EVT_UPDATE_LOGIN_KEY_VID;
            event.device_login_key_vid_data.login_key_len = LOGIN_KEY_LEN;
            event.device_login_key_vid_data.vid_len = DEVICE_VIRTUAL_ID_LEN;
            memcpy(event.device_login_key_vid_data.login_key, tuya_ble_current_para.sys_settings.login_key, LOGIN_KEY_LEN);
            memcpy(event.device_login_key_vid_data.vid, tuya_ble_current_para.sys_settings.device_virtual_id, DEVICE_VIRTUAL_ID_LEN);
            if (tuya_ble_cb_event_send(&event) != 0) {
                TUYA_BLE_LOG_ERROR("tuya ble send cb event failed.");
            } else {
                TUYA_BLE_LOG_DEBUG("tuya ble send cb event succeed.");
            }
#else  //TUYA_BLE_DEVICE_REGISTER_FROM_BLE
            tuya_ble_connect_status_set(UNBONDING_CONN);
#endif
            p_buf[0] = 0x00;
        }
        event.evt = TUYA_BLE_CB_EVT_CONNECT_STATUS;
        event.connect_status = tuya_ble_connect_status_get();
        if (tuya_ble_cb_event_send(&event) != 0) {
            TUYA_BLE_LOG_ERROR("tuya ble send cb event failed.");
        } else {
            TUYA_BLE_LOG_INFO("tuya ble send cb event succeed.");
        }
    } else {
        TUYA_BLE_LOG_ERROR("PAIR_REQ device id not match!");  //ID not match, and disconnected.
        p_buf[0] = 0x01;

    }

    if (p_buf[0] == 0) {
#if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE)
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
#else
        encry_mode = recv_data[0];
#endif
    } else if (p_buf[0] == 1) {
        encry_mode = recv_data[0];
    } else if (p_buf[0] == 2) {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    } else {

    }

    tuya_ble_comm_data_send(PAIR_RESP, ack_sn, p_buf, 1, encry_mode);

    if (encry_mode == ENCRYPTION_MODE_SESSION_KEY) {
        tuya_ble_comm_data_send(FRM_GET_UNIX_TIME_CHAR_MS_REQ, 0, NULL, 0, encry_mode);
        TUYA_BLE_LOG_INFO("send FRM_GET_UNIX_TIME_CHAR_MS_REQ cmd to app.\n");
    }

    if (p_buf[0] == 1) {
        tuya_ble_gap_disconnect();
    }

}

#endif

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN == 2)

STATIC VOID_T tuya_ble_handle_dev_info_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T p_buf[70];
    UINT8_T payload_len = 0;
    UINT32_T ack_sn = 0;
    UINT8_T encry_mode = 0;

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    TUYA_BLE_LOG_INFO("get device infor-%d", tuya_ble_current_para.sys_settings.bound_flag);

    send_packet_data_len = 20;

    tuya_ble_rand_generator(tuya_ble_pair_rand, 6);
    tuya_ble_pair_rand_valid = 1;

    p_buf[0] = (tuya_ble_firmware_version >> 8) & 0xff;
    p_buf[1] = (tuya_ble_firmware_version & 0xff);
    p_buf[2] = TUYA_BLE_PROTOCOL_VERSION_HIGN;
    p_buf[3] = TUYA_BLE_PROTOCOL_VERSION_LOW;
    if (TUYA_BLE_ADVANCED_ENCRYPTION_DEVICE == 1) {
        p_buf[4] = 0x02;
    } else {
        p_buf[4] = 0x00;
    }
    p_buf[5] = tuya_ble_current_para.sys_settings.bound_flag;
    memcpy(&p_buf[6], tuya_ble_pair_rand, 6);
    p_buf[12] = (tuya_ble_hardware_version >> 8) & 0xff;
    p_buf[13] = (tuya_ble_hardware_version & 0xff);
    memcpy(&p_buf[14], tuya_ble_current_para.auth_settings.auth_key, AUTH_KEY_LEN);

    payload_len = 46;

    encry_mode = recv_data[0];

    if (tuya_ble_comm_data_send(FRM_QRY_DEV_INFO_RESP, ack_sn, p_buf, payload_len,  encry_mode) == 2) {
        tuya_ble_pair_rand_clear();
    }

}

STATIC VOID_T tuya_ble_handle_pair_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T p_buf[1];
    UINT8_T encry_mode = 0;
    UINT32_T ack_sn = 0;
    tuya_ble_cb_evt_param_t event;

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    if (0 == memcmp(&recv_data[13], tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN)) {
        tuya_ble_connect_monitor_timer_stop();

        if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
            TUYA_BLE_LOG_ERROR("PAIR_REQ already bound!");
            p_buf[0] = 2;
        } else {

#if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE)

            memcpy(tuya_ble_current_para.sys_settings.login_key, recv_data + 29, LOGIN_KEY_LEN);
            tuya_ble_current_para.sys_settings.bound_flag = 1;
            tuya_ble_storage_save_sys_settings();
            tuya_ble_adv_change();
            TUYA_BLE_LOG_INFO("PAIR_REQ ok-%d", tuya_ble_current_para.sys_settings.bound_flag);

            event.evt = TUYA_BLE_CB_EVT_UPDATE_LOGIN_KEY_VID;
            event.device_login_key_vid_data.login_key_len = LOGIN_KEY_LEN;
            event.device_login_key_vid_data.vid_len = 0;
            memcpy(event.device_login_key_vid_data.login_key, tuya_ble_current_para.sys_settings.login_key, LOGIN_KEY_LEN);

            if (tuya_ble_cb_event_send(&event) != 0) {
                TUYA_BLE_LOG_ERROR("tuya ble send cb event failed.");
            } else {
                TUYA_BLE_LOG_DEBUG("tuya ble send cb event succeed.");
            }

#else
            tuya_ble_connect_status_set(UNBONDING_CONN);
#endif
            p_buf[0] = 0x00;

        }

        if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
            tuya_ble_connect_status_set(BONDING_CONN);
        }

        event.evt = TUYA_BLE_CB_EVT_CONNECT_STATUS;
        event.connect_status = tuya_ble_connect_status_get();
        if (tuya_ble_cb_event_send(&event) != 0) {
            TUYA_BLE_LOG_ERROR("tuya ble send cb event failed.");
        } else {
            TUYA_BLE_LOG_INFO("tuya ble send cb event succeed.");
        }
    } else {
        TUYA_BLE_LOG_ERROR("PAIR_REQ device id not match!");  //ID not match, and disconnected.
        p_buf[0] = 0x01;

    }

    if (p_buf[0] == 0) {
#if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE)
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
#else
        encry_mode = recv_data[0];
#endif
    } else if (p_buf[0] == 1) {
        encry_mode = recv_data[0];
    } else if (p_buf[0] == 2) {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    } else {

    }

    tuya_ble_comm_data_send(PAIR_RESP, ack_sn, p_buf, 1, encry_mode);

    if (encry_mode == ENCRYPTION_MODE_SESSION_KEY) {
        tuya_ble_comm_data_send(FRM_GET_UNIX_TIME_CHAR_MS_REQ, 0, NULL, 0, encry_mode);
        TUYA_BLE_LOG_INFO("send FRM_GET_UNIX_TIME_CHAR_MS_REQ cmd to app.");
    }

    if (p_buf[0] == 1) {
        tuya_ble_gap_disconnect();
    }

}

#endif

STATIC VOID_T tuya_ble_handle_net_config_info_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T p_buf[1];
    UINT8_T encry_mode = 0;
    UINT32_T ack_sn = 0;
    tuya_ble_cb_evt_param_t event;
    UINT16_T data_len;

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];
    data_len = (recv_data[11] << 8) + recv_data[12];
    encry_mode = recv_data[0];

    event.evt = TUYA_BLE_CB_EVT_NETWORK_INFO;
    UINT8_T *ble_cb_evt_buffer = (UINT8_T *)tuya_ble_malloc(data_len);
    if (ble_cb_evt_buffer == NULL) {
        p_buf[0] = 1;
        TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
        tuya_ble_comm_data_send(FRM_NET_CONFIG_INFO_RESP, ack_sn, p_buf, 1, encry_mode);
        return;
    } else {
        memset(ble_cb_evt_buffer, 0, data_len);
        memcpy(ble_cb_evt_buffer, &recv_data[13], data_len);
    }
    event.network_data.data_len = data_len;
    event.network_data.p_data = ble_cb_evt_buffer;

    if (tuya_ble_cb_event_send(&event) != 0) {
        tuya_ble_free(ble_cb_evt_buffer);
        TUYA_BLE_LOG_ERROR("tuya ble send cb event failed.");
        p_buf[0] = 1;
        tuya_ble_comm_data_send(FRM_NET_CONFIG_INFO_RESP, ack_sn, p_buf, 1, encry_mode);
        return;
    } else {
        p_buf[0] = 0;
    }

    tuya_ble_comm_data_send(FRM_NET_CONFIG_INFO_RESP, ack_sn, p_buf, 1, encry_mode);
}

STATIC VOID_T tuya_ble_handle_ble_passthrough_data_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    tuya_ble_cb_evt_param_t event;
    UINT16_T data_len;

    data_len = (recv_data[11] << 8) + recv_data[12];

    event.evt = TUYA_BLE_CB_EVT_DATA_PASSTHROUGH;

    UINT8_T *ble_cb_evt_buffer = (UINT8_T *)tuya_ble_malloc(data_len);
    if (ble_cb_evt_buffer == NULL) {
        TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
        return;
    } else {
        memcpy(ble_cb_evt_buffer, &recv_data[13], data_len);
    }
    event.ble_passthrough_data.data_len = data_len;
    event.ble_passthrough_data.p_data = ble_cb_evt_buffer;

    if (tuya_ble_cb_event_send(&event) != 0) {
        tuya_ble_free(ble_cb_evt_buffer);
        TUYA_BLE_LOG_ERROR("tuya ble send cb event failed.");
    } else {

    }

}

#if defined(TUYA_BLE_FEATURE_SPEECH_ENABLE) && (TUYA_BLE_FEATURE_SPEECH_ENABLE == 1)

STATIC VOID_T tuya_ble_handle_ble_speech_rxs(tuya_ble_cb_evt_t evt, UINT8_T *recv_data, UINT16_T recv_len)
{
    tuya_ble_cb_evt_param_t event;
    UINT16_T data_len;

    data_len = (recv_data[11] << 8) + recv_data[12];

    event.evt = evt;

    UINT8_T *ble_cb_evt_buffer = (UINT8_T *)tuya_ble_malloc(data_len);
    if (ble_cb_evt_buffer == NULL) {
        TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
        return;
    } else {
        memcpy(ble_cb_evt_buffer, &recv_data[13], data_len);
    }
    event.ble_speech_data.data_len = data_len;
    event.ble_speech_data.p_data = ble_cb_evt_buffer;

    if (tuya_ble_cb_event_send(&event) != 0) {
        tuya_ble_free(ble_cb_evt_buffer);
        TUYA_BLE_LOG_ERROR("tuya ble send cb event failed.");
    } else {

    }
}

#endif

#if defined(TUYA_BLE_FEATURE_GPT_ENABLE) && (TUYA_BLE_FEATURE_GPT_ENABLE == 1)

STATIC VOID_T tuya_ble_handle_ble_gpt_rxs(tuya_ble_cb_evt_t evt, UINT8_T *recv_data, UINT16_T recv_len)
{
    tuya_ble_cb_evt_param_t event;
    UINT16_T data_len;

    data_len = (recv_data[11] << 8) + recv_data[12];

    event.evt = evt;

    UINT8_T *ble_cb_evt_buffer = (UINT8_T *)tuya_ble_malloc(data_len);
    if (ble_cb_evt_buffer == NULL) {
        TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
        return;
    } else {
        memcpy(ble_cb_evt_buffer, &recv_data[13], data_len);
    }
    event.ble_speech_data.data_len = data_len;
    event.ble_speech_data.p_data = ble_cb_evt_buffer;

    if (tuya_ble_cb_event_send(&event) != 0) {
        tuya_ble_free(ble_cb_evt_buffer);
        TUYA_BLE_LOG_ERROR("tuya ble send cb event failed.");
    } else {

    }
}

#endif

STATIC VOID_T tuya_ble_handle_ble_factory_test_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT16_T data_len;
    UINT32_T ack_sn;
    UINT8_T sum;
    UINT8_T encry_mode;
    (VOID_T)encry_mode;

    data_len = (recv_data[11] << 8) + recv_data[12];

    if (data_len < 7) {
        return;
    }

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    encry_mode = recv_data[0];

    if ((recv_data[13] == 0x66) && (recv_data[14] == 0xAA)) {
        sum = tal_util_check_sum8(&recv_data[13], data_len - 1);
        if (sum == recv_data[13 + data_len - 1]) {
#if defined(TUYA_BLE_FEATURE_PRODUCT_TEST_ENABLE) && (TUYA_BLE_FEATURE_PRODUCT_TEST_ENABLE == 1)
            tuya_ble_app_production_test_process(1, &recv_data[13], data_len);
#endif
        }
    }

}

STATIC VOID_T tuya_ble_handle_ota_req(UINT16_T cmd, UINT8_T *recv_data, UINT32_T recv_len)
{
    tuya_ble_cb_evt_param_t event;
    UINT16_T data_len;
    tuya_ble_ota_data_type_t cmd_type;
    UINT8_T ota_channel;
    UINT8_T *ota_data_buf = NULL;

    data_len = (recv_data[11] << 8) + recv_data[12];

    if (data_len == 0) {
        return;
    }

    ota_channel = recv_data[13];
    ota_data_buf = &recv_data[13];

#if defined(TUYA_BLE_SINGLE_BANK_OTA_MODE) && (TUYA_BLE_SINGLE_BANK_OTA_MODE == 1)
    if (cmd == FRM_OTA_PREPARE_NOTIFICATION_REQ && recv_data[13] == 0) {
        ota_channel = recv_data[14];

        // If the ota_channel is not equal to 0
        if (ota_channel != 0) { // Currently, single-bank upgrades only support the Bluetooth channel
            return;
        }
    }
#endif


#if defined(TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE) && (TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE == 1)
    if ((ota_channel == 0) || (ota_channel == 1) || (ota_channel >= 10 && ota_channel <= 19)) { // }
#else
    if ((ota_channel == 0) || (ota_channel == 1)) {
#endif
#if defined(TUYA_BLE_FEATURE_UART_COMMON_ENABLE) && (TUYA_BLE_FEATURE_UART_COMMON_ENABLE == 1)
        if (ota_channel == 1) { //extern mcu ota
            tuya_ble_uart_common_mcu_ota_data_from_ble_handler(cmd, ota_data_buf + 1, data_len - 1);
            return;
        }
#endif
        event.evt = TUYA_BLE_CB_EVT_OTA_DATA;

        UINT8_T *ble_cb_evt_buffer = (UINT8_T *)tuya_ble_malloc(data_len);
        if (ble_cb_evt_buffer == NULL) {
            TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
            return;
        } else {
            memcpy(ble_cb_evt_buffer, ota_data_buf, data_len);
        }

        switch (cmd) {
        case FRM_OTA_START_REQ:
            cmd_type = TUYA_BLE_OTA_REQ;
            break;
        case FRM_OTA_FILE_INFOR_REQ:
            cmd_type = TUYA_BLE_OTA_FILE_INFO;
            break;
        case FRM_OTA_FILE_OFFSET_REQ:
            cmd_type = TUYA_BLE_OTA_FILE_OFFSET_REQ;
            break;
        case FRM_OTA_DATA_REQ:
            cmd_type = TUYA_BLE_OTA_DATA;
            break;
        case FRM_OTA_END_REQ:
            cmd_type = TUYA_BLE_OTA_END;
            break;
#if defined(TUYA_BLE_SINGLE_BANK_OTA_MODE) && (TUYA_BLE_SINGLE_BANK_OTA_MODE == 1)
        case FRM_OTA_PREPARE_NOTIFICATION_REQ:
            cmd_type = TUYA_BLE_OTA_PREPARE_NOTIFICATION;
            break;
#endif
#if defined(TUYA_BLE_OTA_SIGNATURE_ENABLE) && (TUYA_BLE_OTA_SIGNATURE_ENABLE == 1)
        case FRM_OTA_SIGNATURE_DATA_REQ:
            cmd_type = TUYA_BLE_OTA_SIGNATURE_DATA;
            break;
        case FRM_OTA_SIGNATURE_KEY_UPDATE_REQ:
            cmd_type = TUYA_BLE_OTA_SIGNATURE_KEY_UPDATE;
            break;
#endif
        default:
            cmd_type = TUYA_BLE_OTA_UNKONWN;
            break;
        }

        event.ota_data.type = cmd_type;
        event.ota_data.data_len = data_len;
        event.ota_data.p_data = ble_cb_evt_buffer;

        if (tuya_ble_cb_event_send(&event) != 0) {
            tuya_ble_free(ble_cb_evt_buffer);
            TUYA_BLE_LOG_ERROR("tuya_ble_handle_ota_req-tuya ble send cb event failed.");

        }
    }
}

#if (TUYA_BLE_FEATURE_APP_PASSTHROUGH_ENABLE != 0)

STATIC VOID_T tuya_ble_handle_business_passthrough_data(UINT16_T cmd, UINT8_T *p_recv_data, UINT32_T recv_len)
{
    UINT16_T data_len = (p_recv_data[11] << 8) + p_recv_data[12];
    if (data_len <= 4) { //flag + subcmd + data (data may not exist)
        TUYA_BLE_LOG_ERROR("Invalid app frame data length : current length = %d .", data_len);
        return;
    }

    switch (cmd) {
    case FRM_APP_DOWNSTREAM_PASSTHROUGH_REQ: {
        tal_ble_app_passthrough_handler(p_recv_data + 13, data_len);
    }
    break;

    case FRM_APP_UPSTREAM_PASSTHROUGH_RESP: {
    } break;

    default: {
    } break;
    }
}

#endif // TUYA_BLE_FEATURE_APP_PASSTHROUGH_ENABLE

STATIC VOID_T tuya_ble_handle_unix_time_char_ms_resp(UINT8_T *recv_data, UINT16_T recv_len)
{
    INT16_T zone_temp = 0;
    UINT64_T time_stamp_ms;
    UINT32_T time_stamp;
    tuya_ble_cb_evt_param_t event;

    memset(&event, 0, SIZEOF(tuya_ble_cb_evt_param_t));

    if (recv_len < 30) {
        TUYA_BLE_LOG_ERROR("received unix time char cmd data length error!");
        return;
    }

    if (!buffer_value_is_all_x(&recv_data[13], 13, 0)) {
        memcpy(current_timems_string, &recv_data[13], 13);
        zone_temp  = (INT16_T)((recv_data[26] << 8) | recv_data[27]);
        time_stamp_ms = atoll(current_timems_string);
        TUYA_BLE_LOG_INFO("received unix time_zone = %d\n", zone_temp);
        time_stamp = time_stamp_ms / 1000;
        if (time_stamp_ms % 1000 >= 500) {
            time_stamp += 1;
        }

        tuya_ble_rtc_set_timestamp(time_stamp, zone_temp);

        event.evt = TUYA_BLE_CB_EVT_TIME_STAMP;

        memcpy(event.timestamp_data.timestamp_string, current_timems_string, 13);
        event.timestamp_data.time_zone = zone_temp;
        if (tuya_ble_cb_event_send(&event) != 0) {
            TUYA_BLE_LOG_ERROR("tuya_ble_handle_unix_time_char_ms_resp-tuya ble send cb event failed.");
        }

    }

}

STATIC VOID_T tuya_ble_handle_unix_time_date_resp(UINT8_T *recv_data, UINT16_T recv_len)
{
    INT16_T zone_temp = 0;
    UINT32_T time_stamp;
    tuya_ble_cb_evt_param_t event;
    tal_utc_date_t date = {0};

    if (recv_len < 24) {
        TUYA_BLE_LOG_ERROR("received unix time date cmd data length error!");
        return;
    }

    memset(&event, 0, SIZEOF(tuya_ble_cb_evt_param_t));

    if (!buffer_value_is_all_x(&recv_data[13], 7, 0)) {
        date.year = 2000 + recv_data[13];
        date.month = recv_data[14];
        date.day = recv_data[15];
        date.hour = recv_data[16];
        date.min = recv_data[17];
        date.sec = recv_data[18];
        date.dayIndex = recv_data[19];

        time_stamp = tal_utc_date2timestamp(&date, FALSE);

        zone_temp  = (INT16_T)((recv_data[20] << 8) | recv_data[21]);

        TUYA_BLE_LOG_INFO("received unix time_zone = %d", zone_temp);

        tuya_ble_rtc_set_timestamp(time_stamp, zone_temp);

        event.evt = TUYA_BLE_CB_EVT_TIME_NORMAL;
        event.time_normal_data.nYear = recv_data[13];
        event.time_normal_data.nMonth = recv_data[14];
        event.time_normal_data.nDay = recv_data[15];
        event.time_normal_data.nHour = recv_data[16];
        event.time_normal_data.nMin = recv_data[17];
        event.time_normal_data.nSec = recv_data[18];
        event.time_normal_data.DayIndex = recv_data[19];
        event.time_normal_data.time_zone = zone_temp;

        if (tuya_ble_cb_event_send(&event) != 0) {
            TUYA_BLE_LOG_ERROR("tuya_ble_handle_unix_time_date_resp-tuya ble send cb event failed.");
        }

    }

}

STATIC VOID_T tuya_ble_handle_app_local_time_data_resp(UINT8_T *recv_data, UINT16_T recv_len)
{
    INT16_T zone_temp = 0;
    UINT32_T time_stamp;
    tuya_ble_cb_evt_param_t event;
    tal_utc_date_t date = {0};

    if (recv_len < 24) {
        TUYA_BLE_LOG_ERROR("received app local time date cmd data length error!");
        return;
    }

    memset(&event, 0, SIZEOF(tuya_ble_cb_evt_param_t));

    if (!buffer_value_is_all_x(&recv_data[13], 7, 0)) {
        date.year = 2000 + recv_data[13];
        date.month = recv_data[14];
        date.day = recv_data[15];
        date.hour = recv_data[16];
        date.min = recv_data[17];
        date.sec = recv_data[18];
        date.dayIndex = recv_data[19];

        time_stamp = tal_utc_date2timestamp(&date, FALSE);

        zone_temp  = (INT16_T)((recv_data[20] << 8) | recv_data[21]);

        TUYA_BLE_LOG_INFO("received app local time_zone = %d", zone_temp);

        tuya_ble_rtc_set_timestamp(time_stamp, zone_temp);

        event.evt = TUYA_BLE_CB_EVT_APP_LOCAL_TIME_NORMAL;
        event.time_normal_data.nYear = recv_data[13];
        event.time_normal_data.nMonth = recv_data[14];
        event.time_normal_data.nDay = recv_data[15];
        event.time_normal_data.nHour = recv_data[16];
        event.time_normal_data.nMin = recv_data[17];
        event.time_normal_data.nSec = recv_data[18];
        event.time_normal_data.DayIndex = recv_data[19];
        event.time_normal_data.time_zone = zone_temp;

        if (tuya_ble_cb_event_send(&event) != 0) {
            TUYA_BLE_LOG_ERROR("tuya_ble_handle_app_local_time_date_resp-tuya ble send cb event failed.");
        }

    }
}

STATIC VOID_T tuya_ble_handle_remoter_proxy_auth_data_resp(UINT8_T *recv_data, UINT16_T recv_len)
{
    tuya_ble_cb_evt_param_t event;
    UINT16_T data_len;

    data_len = (recv_data[11] << 8) | (recv_data[12] << 0);
    if (data_len % sizeof(tuya_ble_remoter_proxy_auth_data_rsp_unit_t) != 0) {
        TUYA_BLE_LOG_ERROR("received app remoter proxy auth date cmd data length error!");
        return;
    }

    UINT8_T *ble_cb_evt_buffer = (UINT8_T *)tuya_ble_malloc(data_len);
    if (ble_cb_evt_buffer == NULL) {
        TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
        return;
    } else {
        memcpy(ble_cb_evt_buffer, &recv_data[13], data_len);
    }

    memset(&event, 0, SIZEOF(tuya_ble_cb_evt_param_t));
    event.evt = TUYA_BLE_CB_EVT_REMOTER_PROXY_AUTH_RESP;
    event.remoter_proxy_auth_data_rsp.num = data_len / sizeof(tuya_ble_remoter_proxy_auth_data_rsp_unit_t);
    event.remoter_proxy_auth_data_rsp.p_data = ble_cb_evt_buffer;
    if (tuya_ble_cb_event_send(&event) != 0) {
        tuya_ble_free(ble_cb_evt_buffer);
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_app_local_time_date_resp-tuya ble send cb event failed.");
    }
}

STATIC VOID_T tuya_ble_handle_remoter_group_set_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    tuya_ble_cb_evt_param_t event;

    if (recv_len != 7) {
        TUYA_BLE_LOG_ERROR("received app remoter proxy auth date cmd data length error!");
        return;
    }

    memset(&event, 0, SIZEOF(tuya_ble_cb_evt_param_t));
    event.evt = TUYA_BLE_CB_EVT_REMOTER_GROUP_SET;
    memcpy(event.remoter_group_set_data.mac, recv_data, 6);
    event.remoter_group_set_data.id = recv_data[6];

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_app_local_time_date_resp-tuya ble send cb event failed.");
    }
}

STATIC VOID_T tuya_ble_handle_remoter_group_delete_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    tuya_ble_cb_evt_param_t event;

    UINT16_T data_len = (recv_data[11] << 8) | (recv_data[12] << 0);
    UINT8_T *p_data = &recv_data[13];
    if (data_len != 7) {
        TUYA_BLE_LOG_ERROR("received app remoter proxy auth date cmd data length error!");
        return;
    }

    memset(&event, 0, SIZEOF(tuya_ble_cb_evt_param_t));
    event.evt = TUYA_BLE_CB_EVT_REMOTER_GROUP_DELETE;
    memcpy(event.remoter_group_set_data.mac, p_data,  6);
    event.remoter_group_set_data.id = p_data[6];

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_app_local_time_date_resp-tuya ble send cb event failed.");
    }
}

STATIC VOID_T tuya_ble_handle_remoter_group_get_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    tuya_ble_cb_evt_param_t event;

    if (recv_len != 6) {
        TUYA_BLE_LOG_ERROR("received app remoter proxy auth date cmd data length error!");
        return;
    }

    memset(&event, 0, SIZEOF(tuya_ble_cb_evt_param_t));
    event.evt = TUYA_BLE_CB_EVT_REMOTER_GROUP_GET;
    memcpy(event.remoter_group_set_data.mac, recv_data, 6);

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_app_local_time_date_resp-tuya ble send cb event failed.");
    }
}

STATIC VOID_T tuya_ble_handle_unix_time_with_dst_date_resp(UINT8_T *recv_data, UINT16_T recv_len)
{
    INT16_T zone_temp = 0;
    UINT32_T time_stamp;
    UINT8_T years_of_dst;
    UINT16_T dst_content_len;
    tuya_ble_cb_evt_param_t event;
    char current_time_string[11] = "0000000000";
    UINT8_T *ble_cb_evt_buffer = NULL;

    if (recv_len < 28) {
        TUYA_BLE_LOG_ERROR("received extend unix time date cmd data length error!");
        return;
    }

    memset(&event, 0, SIZEOF(tuya_ble_cb_evt_param_t));

    if (!buffer_value_is_all_x(&recv_data[13], 10, 0)) {
        memcpy(current_time_string, &recv_data[13], 10);
        time_stamp = atol(current_time_string);

        zone_temp  = (INT16_T)((recv_data[23] << 8) | recv_data[24]);
        TUYA_BLE_LOG_INFO("received unix time_zone = %d\n", zone_temp);

        tuya_ble_rtc_set_timestamp(time_stamp, zone_temp);

        /* daylight saving time */
        years_of_dst = recv_data[25];
        dst_content_len = (years_of_dst * 20);
        TUYA_BLE_LOG_INFO("received dst years= %d ", years_of_dst);
        TUYA_BLE_LOG_HEXDUMP_INFO("dst data ", &recv_data[26], dst_content_len);

        if (years_of_dst > 0) {
            ble_cb_evt_buffer = (UINT8_T *)tuya_ble_malloc(dst_content_len);
            if (ble_cb_evt_buffer == NULL) {
                TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
                return;
            } else {
                memcpy(ble_cb_evt_buffer, &recv_data[26], dst_content_len);
            }
        }

        event.evt = TUYA_BLE_CB_EVT_TIME_STAMP_WITH_DST;
        event.timestamp_with_dst_data.timestamp = time_stamp;
        event.timestamp_with_dst_data.time_zone = zone_temp;
        event.timestamp_with_dst_data.n_years_dst = years_of_dst;
        event.timestamp_with_dst_data.p_data = ble_cb_evt_buffer;
        event.timestamp_with_dst_data.data_len = dst_content_len;

        if (tuya_ble_cb_event_send(&event) != 0) {
            if (years_of_dst > 0) {
                tuya_ble_free(ble_cb_evt_buffer);
            }

            TUYA_BLE_LOG_ERROR("tuya_ble_handle_unix_time_with_dst_resp-tuya ble send cb event failed.");
        }
    }
}

STATIC VOID_T tuya_ble_handle_dp_query_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T p_buf[1];
    UINT16_T dp_num = 0;
    UINT32_T ack_sn = 0;
    tuya_ble_cb_evt_param_t event;
    UINT8_T *ble_cb_evt_buffer = NULL;

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    dp_num = (recv_data[11] << 8) | recv_data[12];

    p_buf[0] = 0x00;

    tuya_ble_comm_data_send(FRM_STATE_QUERY_RESP, ack_sn, p_buf, 1, ENCRYPTION_MODE_SESSION_KEY);

    event.evt = TUYA_BLE_CB_EVT_DP_QUERY;

    if (dp_num > 0) {
        ble_cb_evt_buffer = (UINT8_T *)tuya_ble_malloc(dp_num);
        if (ble_cb_evt_buffer == NULL) {
            TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
            return;
        } else {
            memcpy(ble_cb_evt_buffer, &recv_data[13], dp_num);
        }
    }
    event.dp_query_data.p_data = ble_cb_evt_buffer;
    event.dp_query_data.data_len = dp_num;

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_query_req-tuya ble send cb event failed.");
        if (dp_num > 0) {
            tuya_ble_free(ble_cb_evt_buffer);
        }
    } else {

    }
}

STATIC VOID_T tuya_ble_xtimer_disconncet_callback(tuya_ble_timer_t timer)
{
    TUYA_BLE_LOG_DEBUG("ble disconncet");
    tuya_ble_gap_disconnect();
    tuya_ble_adv_change();
    tuya_ble_connect_status_set(UNBONDING_UNCONN);
}

VOID_T tuya_ble_disconnect_timer_init(VOID_T)
{
    if (tuya_ble_timer_create(&tuya_ble_xtimer_disconnect, 200, TUYA_BLE_TIMER_SINGLE_SHOT, tuya_ble_xtimer_disconncet_callback) != TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_ERROR("tuya_ble_xtimer_connect_monitor creat failed");
    }
}

VOID_T tuya_ble_disconnect_timer_start(VOID_T)
{
    if (tuya_ble_timer_start(tuya_ble_xtimer_disconnect) != TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_ERROR("tuya_ble_xtimer_connect_monitor start failed");
    }
}

VOID_T tuya_ble_device_unbond(VOID_T)
{
    tuya_ble_connect_status_set(UNBONDING_UNCONN);

    memset(tuya_ble_current_para.sys_settings.login_key, 0, LOGIN_KEY_LEN);
    memset(tuya_ble_current_para.sys_settings.beacon_key, 0, BEACON_KEY_LEN);
    memset(tuya_ble_current_para.sys_settings.login_key_v2, 0, LOGIN_KEY_V2_LEN);
    memset(tuya_ble_current_para.sys_settings.secret_key, 0, SECRET_KEY_LEN);
    memset(tuya_ble_current_para.sys_settings.res, 0, sizeof(tuya_ble_current_para.sys_settings.res));
    tuya_ble_current_para.sys_settings.bound_flag = 0;

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN<5)
#if (TUYA_BLE_PROTOCOL_VERSION_LOW >= 7)
    tuya_ble_current_para.sys_settings.protocol_v2_enable = FALSE;
#endif
#endif

    tuya_ble_storage_save_sys_settings();

#if defined(TUYA_BLE_FEATURE_ENABLE_GROUP) && (TUYA_BLE_FEATURE_ENABLE_GROUP == 1)
    tal_ble_group_unbond_handler();
#endif // TUYA_BLE_FEATURE_ENABLE_GROUP

    tuya_ble_disconnect_timer_start();
    TUYA_BLE_LOG_INFO("tuya_ble_device_unbond current bound flag = %d", tuya_ble_current_para.sys_settings.bound_flag);
}

STATIC VOID_T tuya_ble_handle_unbond_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T p_buf[1];
    UINT8_T encry_mode = 0;
    UINT32_T ack_sn = 0;
    tuya_ble_cb_evt_param_t event;

#if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE)
    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    encry_mode = ENCRYPTION_MODE_SESSION_KEY;

    p_buf[0] = 0;

    tuya_ble_comm_data_send(FRM_UNBONDING_RESP, ack_sn, p_buf, 1, encry_mode);

#if (TUYA_BLE_DEVICE_UNBIND_MODE)
    tuya_ble_device_unbond();

    event.evt = TUYA_BLE_CB_EVT_CONNECT_STATUS;
    event.connect_status = tuya_ble_connect_status_get();

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_unbond_req-tuya ble send cb event (connect status update) failed.");
    } else {

    }
#else
    tuya_ble_gap_disconnect();
#endif

#endif

    event.evt = TUYA_BLE_CB_EVT_UNBOUND;
    event.unbound_data.data = 0;

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_unbond_req-tuya ble send cb event (unbound req) failed.");
    } else {

    }

}

STATIC VOID_T tuya_ble_handle_anomaly_unbond_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T p_buf[1];
    UINT8_T encry_mode = 0;
    UINT32_T ack_sn = 0;
    tuya_ble_cb_evt_param_t event;

#if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE)

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    encry_mode = ENCRYPTION_MODE_AN_UNBOND_KEY;

    p_buf[0] = 0;

    tuya_ble_comm_data_send(FRM_ANOMALY_UNBONDING_RESP, ack_sn, p_buf, 1, encry_mode);
    tuya_ble_device_unbond();

    event.evt = TUYA_BLE_CB_EVT_CONNECT_STATUS;
    event.connect_status = tuya_ble_connect_status_get();

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_anomaly_unbond_req-tuya ble send cb event (connect status update) failed.");
    } else {

    }

#endif

    event.evt = TUYA_BLE_CB_EVT_ANOMALY_UNBOUND;
    event.anomaly_unbound_data.data = 0;

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_anomaly_unbond_req-tuya ble send cb event (unbound req) failed.");
    } else {

    }

}

STATIC VOID_T tuya_ble_handle_device_reset_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T p_buf[1];
    UINT8_T encry_mode = 0;
    UINT32_T ack_sn = 0;
    tuya_ble_cb_evt_param_t event;

#if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE)

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    encry_mode = ENCRYPTION_MODE_SESSION_KEY;

    p_buf[0] = 0;

    tuya_ble_comm_data_send(FRM_DEVICE_RESET_RESP, ack_sn, p_buf, 1, encry_mode);

#if (TUYA_BLE_DEVICE_UNBIND_MODE)
    memset(tuya_ble_current_para.sys_settings.device_virtual_id, 0, DEVICE_VIRTUAL_ID_LEN);
    tuya_ble_device_unbond();

    event.evt = TUYA_BLE_CB_EVT_CONNECT_STATUS;
    event.connect_status = tuya_ble_connect_status_get();

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_device_reset_req-tuya ble send cb event (connect status update) failed.");
    } else {

    }
#else
    tuya_ble_gap_disconnect();
#endif

#endif

    event.evt = TUYA_BLE_CB_EVT_DEVICE_RESET;
    event.device_reset_data.data = 0;

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_device_reset_req-tuya ble send cb event device reset req failed.");
    } else {

    }

}

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN == 5)

#if (TUYA_BLE_MUTI_DATA_SOURCE_SUPPORTED)

STATIC VOID_T tuya_ble_handle_dp_query_with_src_type_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    tuya_ble_data_source_type_t src_type;
    UINT8_T add_info_len = 0;
    UINT16_T dp_num = 0;
    UINT16_T data_len = 0;
    UINT32_T ack_sn = 0;
    UINT8_T *p_buf = NULL;
    UINT8_T *ble_cb_evt_buffer = NULL;
    UINT8_T *ble_cb_evt_add_info_buffer = NULL;
    tuya_ble_cb_evt_param_t event;

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    data_len = (recv_data[11] << 8) | recv_data[12];

    src_type = recv_data[14];

    add_info_len = recv_data[15];

    dp_num = recv_data[16 + add_info_len];

    p_buf = (UINT8_T *)tuya_ble_malloc(add_info_len + 4);
    if (p_buf == NULL) {
        TUYA_BLE_LOG_ERROR("p_buf malloc failed.");
        return;
    }

    memcpy(p_buf, recv_data + 13, 3 + add_info_len); //copy VERSION,SRC_TYPE,ADD_INFO_LEN,ADD_INFO

    if (recv_data[13] != 0) {
        TUYA_BLE_LOG_ERROR("cmd dp query version not support");

        p_buf[3 + add_info_len] = 2;

        tuya_ble_comm_data_send(FRM_WITH_SRC_TYPE_DP_DATA_QUERY_RESP, ack_sn, p_buf, 4 + add_info_len, ENCRYPTION_MODE_SESSION_KEY);
        tuya_ble_free(p_buf);
        return;
    } else {
        p_buf[3 + add_info_len] = 0x00;

        tuya_ble_comm_data_send(FRM_WITH_SRC_TYPE_DP_DATA_QUERY_RESP, ack_sn, p_buf, 4 + add_info_len, ENCRYPTION_MODE_SESSION_KEY);
        tuya_ble_free(p_buf);
    }
    event.evt = TUYA_BLE_CB_EVT_WITH_SRC_TYPE_DP_QUERY;

    if (dp_num > 0) {
        ble_cb_evt_buffer = (UINT8_T *)tuya_ble_malloc(dp_num);
        if (ble_cb_evt_buffer == NULL) {
            TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
            return;
        } else {
            memcpy(ble_cb_evt_buffer, &recv_data[17 + add_info_len], dp_num);
        }
    }
    event.dp_query_data_with_src_type.p_data = ble_cb_evt_buffer;
    event.dp_query_data_with_src_type.data_len = dp_num;

    if (add_info_len > 0) {
        ble_cb_evt_add_info_buffer = (UINT8_T *)tuya_ble_malloc(add_info_len);
        if (ble_cb_evt_add_info_buffer == NULL) {
            if (dp_num > 0) {
                tuya_ble_free(ble_cb_evt_buffer);
            }
            TUYA_BLE_LOG_ERROR("ble_cb_evt_add_info_buffer malloc failed.");
            return;
        } else {
            memcpy(ble_cb_evt_add_info_buffer, &recv_data[16], add_info_len);
        }
    }

    event.dp_query_data_with_src_type.p_add_info = ble_cb_evt_add_info_buffer;
    event.dp_query_data_with_src_type.add_info_len = add_info_len;
    event.dp_query_data_with_src_type.src_type = src_type;

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_query_req-tuya ble send cb event failed.");
        if (dp_num > 0) {
            tuya_ble_free(ble_cb_evt_buffer);
        }
        if (add_info_len > 0) {
            tuya_ble_free(ble_cb_evt_add_info_buffer);
        }
    } else {

    }
}

STATIC VOID_T tuya_ble_handle_dp_data_write_with_src_type_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    mtp_ret ret;
    klv_node_s *list = NULL;
    UINT8_T *p_buf;
    UINT8_T *ble_cb_evt_add_info_buffer = NULL;
    UINT8_T *ble_cb_evt_buffer = NULL;
    UINT16_T data_len = 0;
    UINT8_T add_info_len;
    UINT32_T ack_sn = 0;
    tuya_ble_cb_evt_param_t event;

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    data_len = (recv_data[11] << 8) | recv_data[12];

    if (data_len < 7 || data_len < 7 + recv_data[19]) {
        TUYA_BLE_LOG_ERROR("frame error,could not be processed.");
        return;
    }

    add_info_len = recv_data[19];

    p_buf = (UINT8_T *)tuya_ble_malloc(8 + add_info_len);
    if (p_buf == NULL) {
        TUYA_BLE_LOG_ERROR("p_buf malloc failed.");
        return;
    }

    if ((data_len > TUYA_BLE_RECEIVE_MAX_DP_DATA_LEN + 2 + add_info_len)) {
        TUYA_BLE_LOG_ERROR("cmd dp write error,receive data len == %d", data_len);

        memcpy(&p_buf[0], &recv_data[13], 7 + add_info_len);
        p_buf[7 + add_info_len] = 1;

        tuya_ble_comm_data_send(FRM_DP_DATA_WITH_SRC_TYPE_WRITE_RESP, ack_sn, p_buf, 8 + add_info_len, ENCRYPTION_MODE_SESSION_KEY);
        tuya_ble_free(p_buf);
        return;
    } else if (recv_data[13] != 0) {
        TUYA_BLE_LOG_ERROR("cmd dp write version not support");

        memcpy(&p_buf[0], &recv_data[13], 7 + add_info_len);
        p_buf[7 + add_info_len] = 1;

        tuya_ble_comm_data_send(FRM_DP_DATA_WITH_SRC_TYPE_WRITE_RESP, ack_sn, p_buf, 8 + add_info_len, ENCRYPTION_MODE_SESSION_KEY);
        tuya_ble_free(p_buf);
        return;
    } else {

    }

    TUYA_BLE_LOG_HEXDUMP_DEBUG("cmd_dp_write data : ", recv_data + 13, data_len);
    memcpy(&p_buf[0], &recv_data[13], 7 + add_info_len); //version...add_info
    ret = data_2_klvlist(&recv_data[20 + add_info_len], data_len - 7 - add_info_len, &list, 1);
    if (MTP_OK != ret) {
        TUYA_BLE_LOG_ERROR("cmd rx fail-%d", ret);
        p_buf[7 + add_info_len] = 1;
        tuya_ble_comm_data_send(FRM_DP_DATA_WITH_SRC_TYPE_WRITE_RESP, ack_sn, p_buf, 8 + add_info_len, ENCRYPTION_MODE_SESSION_KEY);
        tuya_ble_free(p_buf);
        return;
    }

    free_klv_list(list);

    event.evt = TUYA_BLE_CB_EVT_WITH_SRC_TYPE_DP_DATA_RECEIVED;

    ble_cb_evt_buffer = (UINT8_T *)tuya_ble_malloc(data_len - 7 - add_info_len);
    if (ble_cb_evt_buffer == NULL) {
        TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
        p_buf[7 + add_info_len] = 1;
        tuya_ble_comm_data_send(FRM_DP_DATA_WITH_SRC_TYPE_WRITE_RESP, ack_sn, p_buf, 8 + add_info_len, ENCRYPTION_MODE_SESSION_KEY);
        tuya_ble_free(p_buf);
        return;
    }

    ble_cb_evt_add_info_buffer = (UINT8_T *)tuya_ble_malloc(add_info_len);
    if (ble_cb_evt_add_info_buffer == NULL) {
        TUYA_BLE_LOG_ERROR("ble_cb_evt_add_info_buffer malloc failed.");
        p_buf[7 + add_info_len] = 1;
        tuya_ble_comm_data_send(FRM_DP_DATA_WITH_SRC_TYPE_WRITE_RESP, ack_sn, p_buf, 8 + add_info_len, ENCRYPTION_MODE_SESSION_KEY);
        tuya_ble_free(p_buf);
        tuya_ble_free(ble_cb_evt_buffer);
        return;
    }

    memcpy(ble_cb_evt_buffer, &recv_data[20 + add_info_len], data_len - 7 - add_info_len);
    memcpy(ble_cb_evt_add_info_buffer, &recv_data[20], add_info_len);
    event.dp_data_with_src_type_received_data.sn = (recv_data[14] << 24) | (recv_data[15] << 16) | (recv_data[16] << 8) | recv_data[17];
    event.dp_data_with_src_type_received_data.p_data = ble_cb_evt_buffer;
    event.dp_data_with_src_type_received_data.data_len = data_len - 7 - add_info_len;
    event.dp_data_with_src_type_received_data.add_info_len = add_info_len;
    event.dp_data_with_src_type_received_data.p_add_info = ble_cb_evt_add_info_buffer;
    event.dp_data_with_src_type_received_data.src_type = recv_data[18];

    if (tuya_ble_cb_event_send(&event) != 0) {
        tuya_ble_free(ble_cb_evt_buffer);
        tuya_ble_free(ble_cb_evt_add_info_buffer);
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_write_req-tuya ble send cb event failed.");
        p_buf[7 + add_info_len] = 1;
        tuya_ble_comm_data_send(FRM_DP_DATA_WITH_SRC_TYPE_WRITE_RESP, ack_sn, p_buf, 8 + add_info_len, ENCRYPTION_MODE_SESSION_KEY);
        tuya_ble_free(p_buf);
    } else {
        p_buf[7 + add_info_len] = 0;
        tuya_ble_comm_data_send(FRM_DP_DATA_WITH_SRC_TYPE_WRITE_RESP, ack_sn, p_buf, 8 + add_info_len, ENCRYPTION_MODE_SESSION_KEY);
        tuya_ble_free(p_buf);
    }

}

STATIC VOID_T tuya_ble_handle_dp_data_with_src_type_send_response(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T *ble_cb_evt_add_info_buffer = NULL;
    tuya_ble_cb_evt_param_t event;
    UINT16_T data_len = recv_data[11] << 8 | recv_data[12];
    UINT8_T add_info_len = 0;

    if (data_len < 10 || data_len < 10 + recv_data[19]) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_send_response- invalid data len received.");
        return;
    }
    add_info_len = recv_data[19];
    if (add_info_len > 0) {
        ble_cb_evt_add_info_buffer = (UINT8_T *)tuya_ble_malloc(add_info_len);

        if (ble_cb_evt_add_info_buffer == NULL) {
            TUYA_BLE_LOG_ERROR("ble_cb_evt_add_info_buffer malloc failed.");
            return;
        } else {
            memcpy(ble_cb_evt_add_info_buffer, recv_data + 20, add_info_len);
        }
    }
    event.evt = TUYA_BLE_CB_EVT_DP_DATA_WITH_SRC_TYPE_SEND_RESPONSE;
    event.dp_with_src_type_send_response_data.sn = recv_data[14] << 24 | recv_data[15] << 16 | recv_data[16] << 8 | recv_data[17];
    event.dp_with_src_type_send_response_data.ack = ((recv_data[20 + add_info_len] & 0x80) ? DP_SEND_WITHOUT_RESPONSE : DP_SEND_WITH_RESPONSE);
    event.dp_with_src_type_send_response_data.src_type = recv_data[18];
    event.dp_with_src_type_send_response_data.add_info_len = add_info_len;
    event.dp_with_src_type_send_response_data.p_add_info = ble_cb_evt_add_info_buffer;

    if ((recv_data[20 + add_info_len] & 0x0F) == 1) {
        event.dp_with_src_type_send_response_data.type = DP_SEND_TYPE_PASSIVE;
    } else if ((recv_data[20 + add_info_len] & 0x0F) == 0) {
        event.dp_with_src_type_send_response_data.type = DP_SEND_TYPE_ACTIVE;
    } else {
        tuya_ble_free(ble_cb_evt_add_info_buffer);
        return;
    }

    if (recv_data[21 + add_info_len] > 3) {
        tuya_ble_free(ble_cb_evt_add_info_buffer);
        return;
    }
    event.dp_with_src_type_send_response_data.mode = recv_data[21 + add_info_len];
    event.dp_with_src_type_send_response_data.status = recv_data[22 + add_info_len];

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_send_response-tuya ble send cb event failed.");
        tuya_ble_free(ble_cb_evt_add_info_buffer);
    } else {

    }
}

STATIC VOID_T tuya_ble_handle_dp_data_with_src_type_and_time_send_response(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T *ble_cb_evt_add_info_buffer = NULL;
    tuya_ble_cb_evt_param_t event;
    UINT16_T data_len = recv_data[11] << 8 | recv_data[12];
    UINT8_T add_info_len = 0;

    if (data_len < 10 || data_len < 10 + recv_data[19]) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_with_time_send_response- invalid data len received.");
        return;
    }

    add_info_len = recv_data[19];
    if (add_info_len > 0) {
        ble_cb_evt_add_info_buffer = (UINT8_T *)tuya_ble_malloc(add_info_len);

        if (ble_cb_evt_add_info_buffer == NULL) {
            TUYA_BLE_LOG_ERROR("ble_cb_evt_add_info_buffer malloc failed.");
            return;
        } else {
            memcpy(ble_cb_evt_add_info_buffer, recv_data + 20, add_info_len);
        }
    }

    event.evt = TUYA_BLE_CB_EVT_DP_DATA_WITH_SRC_TYPE_AND_TIME_SEND_RESPONSE;
    event.dp_with_src_type_and_time_send_response_data.sn = recv_data[14] << 24 | recv_data[15] << 16 | recv_data[16] << 8 | recv_data[17];
    event.dp_with_src_type_and_time_send_response_data.ack = ((recv_data[20 + add_info_len] & 0x80) ? DP_SEND_WITHOUT_RESPONSE : DP_SEND_WITH_RESPONSE);
    event.dp_with_src_type_and_time_send_response_data.src_type = recv_data[18];
    event.dp_with_src_type_and_time_send_response_data.add_info_len = add_info_len;
    event.dp_with_src_type_and_time_send_response_data.p_add_info = ble_cb_evt_add_info_buffer;

    if ((recv_data[20 + add_info_len] & 0x0F) == 1) {
        event.dp_with_src_type_and_time_send_response_data.type = DP_SEND_TYPE_PASSIVE;
    } else if ((recv_data[20 + add_info_len] & 0x0F) == 0) {
        event.dp_with_src_type_and_time_send_response_data.type = DP_SEND_TYPE_ACTIVE;
    } else {
        tuya_ble_free(ble_cb_evt_add_info_buffer);
        return;
    }

    if (recv_data[21 + add_info_len] > 3) {
        tuya_ble_free(ble_cb_evt_add_info_buffer);
        return;
    }
    event.dp_with_src_type_and_time_send_response_data.mode = recv_data[21 + add_info_len];
    event.dp_with_src_type_and_time_send_response_data.status = recv_data[22 + add_info_len];

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_with_time_send_response-tuya ble send cb event failed.");
        tuya_ble_free(ble_cb_evt_add_info_buffer);
    } else {

    }
}

#endif /* end of #if (TUYA_BLE_MUTI_DATA_SOURCE_SUPPORTED&&TUYA_BLE_PROTOCOL_VERSION_LOW >= 5) */

STATIC VOID_T tuya_ble_handle_dp_data_write_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    mtp_ret ret;
    klv_node_s *list = NULL;
    UINT8_T p_buf[6];
    UINT16_T data_len = 0;
    UINT32_T ack_sn = 0;
    tuya_ble_cb_evt_param_t event;

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    data_len = (recv_data[11] << 8) | recv_data[12];

    if ((data_len < 5) || (data_len > TUYA_BLE_RECEIVE_MAX_DP_DATA_LEN)) {
        TUYA_BLE_LOG_ERROR("cmd dp write error,receive data len == %d", data_len);
        if (data_len < 5) {
            memcpy(&p_buf[0], &recv_data[13], data_len);
        } else {
            memcpy(&p_buf[0], &recv_data[13], 5);
        }
        p_buf[5] = 1;
        tuya_ble_comm_data_send(FRM_DP_DATA_WRITE_RESP, ack_sn, p_buf, 6, ENCRYPTION_MODE_SESSION_KEY);
        return;
    } else if (recv_data[13] != 0) {
        TUYA_BLE_LOG_ERROR("cmd dp write version not support");

        memcpy(&p_buf[0], &recv_data[13], 5);

        p_buf[5] = 1;
        tuya_ble_comm_data_send(FRM_DP_DATA_WRITE_RESP, ack_sn, p_buf, 6, ENCRYPTION_MODE_SESSION_KEY);
        return;
    } else {

    }

    TUYA_BLE_LOG_HEXDUMP_DEBUG("cmd_dp_write data : ", recv_data + 13, data_len);
    memcpy(&p_buf[0], &recv_data[13], 5);
    ret = data_2_klvlist(&recv_data[18], data_len - 5, &list, 1);
    if (MTP_OK != ret) {
        TUYA_BLE_LOG_ERROR("cmd rx fail-%d", ret);
        p_buf[5] = 1;
        tuya_ble_comm_data_send(FRM_DP_DATA_WRITE_RESP, ack_sn, p_buf, 6, ENCRYPTION_MODE_SESSION_KEY);
        return;
    }

    free_klv_list(list);

    event.evt = TUYA_BLE_CB_EVT_DP_DATA_RECEIVED;

    UINT8_T *ble_cb_evt_buffer = (UINT8_T *)tuya_ble_malloc(data_len - 5);
    if (ble_cb_evt_buffer == NULL) {
        TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
        p_buf[5] = 0x01;
        tuya_ble_comm_data_send(FRM_DP_DATA_WRITE_RESP, ack_sn, p_buf, 6, ENCRYPTION_MODE_SESSION_KEY);
        return;
    } else {
        memcpy(ble_cb_evt_buffer, &recv_data[18], data_len - 5);
        event.dp_received_data.sn = (recv_data[14] << 24) | (recv_data[15] << 16) | (recv_data[16] << 8) | recv_data[17];
        event.dp_received_data.p_data = ble_cb_evt_buffer;
        event.dp_received_data.data_len = data_len - 5;
    }

    if (tuya_ble_cb_event_send(&event) != 0) {
        tuya_ble_free(ble_cb_evt_buffer);
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_write_req-tuya ble send cb event failed.");
        p_buf[5] = 0x01;
        tuya_ble_comm_data_send(FRM_DP_DATA_WRITE_RESP, ack_sn, p_buf, 6, ENCRYPTION_MODE_SESSION_KEY);
    } else {
        p_buf[5] = 0x00;
        tuya_ble_comm_data_send(FRM_DP_DATA_WRITE_RESP, ack_sn, p_buf, 6, ENCRYPTION_MODE_SESSION_KEY);
    }

}

STATIC VOID_T tuya_ble_handle_dp_data_send_response(UINT8_T *recv_data, UINT16_T recv_len)
{
    tuya_ble_cb_evt_param_t event;
    UINT16_T data_len = recv_data[11] << 8 | recv_data[12];

    if (data_len != 8) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_send_response- invalid data len received.");
        return;
    }

    event.evt = TUYA_BLE_CB_EVT_DP_DATA_SEND_RESPONSE;
    event.dp_send_response_data.sn = recv_data[14] << 24 | recv_data[15] << 16 | recv_data[16] << 8 | recv_data[17];
    event.dp_send_response_data.ack = ((recv_data[18] & 0x80) ? DP_SEND_WITHOUT_RESPONSE : DP_SEND_WITH_RESPONSE);
    if ((recv_data[18] & 0x0F) == 1) {
        event.dp_send_response_data.type = DP_SEND_TYPE_PASSIVE;
    } else if ((recv_data[18] & 0x0F) == 0) {
        event.dp_send_response_data.type = DP_SEND_TYPE_ACTIVE;
    } else {
        return;
    }

    if (recv_data[19] > 3) {
        return;
    }
    event.dp_send_response_data.mode = recv_data[19];
    event.dp_send_response_data.status = recv_data[20];

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_send_response-tuya ble send cb event failed.");
    } else {

    }
}

STATIC VOID_T tuya_ble_handle_dp_data_with_time_send_response(UINT8_T *recv_data, UINT16_T recv_len)
{
    tuya_ble_cb_evt_param_t event;
    UINT16_T data_len = recv_data[11] << 8 | recv_data[12];

    if (data_len != 8) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_with_time_send_response- invalid data len received.");
        return;
    }

    event.evt = TUYA_BLE_CB_EVT_DP_DATA_WITH_TIME_SEND_RESPONSE;
    event.dp_with_time_send_response_data.sn = recv_data[14] << 24 | recv_data[15] << 16 | recv_data[16] << 8 | recv_data[17];
    event.dp_with_time_send_response_data.ack = ((recv_data[18] & 0x80) ? DP_SEND_WITHOUT_RESPONSE : DP_SEND_WITH_RESPONSE);
    if ((recv_data[18] & 0x0F) == 1) {
        event.dp_with_time_send_response_data.type = DP_SEND_TYPE_PASSIVE;
    } else if ((recv_data[18] & 0x0F) == 0) {
        event.dp_with_time_send_response_data.type = DP_SEND_TYPE_ACTIVE;
    } else {
        return;
    }

    if (recv_data[19] > 3) {
        return;
    }
    event.dp_with_time_send_response_data.mode = recv_data[19];
    event.dp_with_time_send_response_data.status = recv_data[20];

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_with_time_send_response-tuya ble send cb event failed.");
    } else {

    }
}

#elif (TUYA_BLE_PROTOCOL_VERSION_HIGN == 4)

#if (TUYA_BLE_MUTI_DATA_SOURCE_SUPPORTED&&TUYA_BLE_PROTOCOL_VERSION_LOW >= 5)

STATIC VOID_T tuya_ble_handle_dp_query_with_src_type_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    tuya_ble_data_source_type_t src_type;
    UINT8_T add_info_len = 0;
    UINT16_T dp_num = 0;
    UINT16_T data_len = 0;
    UINT32_T ack_sn = 0;
    UINT8_T *p_buf = NULL;
    UINT8_T *ble_cb_evt_buffer = NULL;
    UINT8_T *ble_cb_evt_add_info_buffer = NULL;
    tuya_ble_cb_evt_param_t event;

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    data_len = (recv_data[11] << 8) | recv_data[12];

    src_type = recv_data[14];

    add_info_len = recv_data[15];

    dp_num = recv_data[16 + add_info_len];

    p_buf = (UINT8_T *)tuya_ble_malloc(add_info_len + 4);
    if (p_buf == NULL) {
        TUYA_BLE_LOG_ERROR("p_buf malloc failed.");
        return;
    }

    memcpy(p_buf, recv_data + 13, 3 + add_info_len); //copy VERSION,SRC_TYPE,ADD_INFO_LEN,ADD_INFO

    if (recv_data[13] != 0) {
        TUYA_BLE_LOG_ERROR("cmd dp query version not support");

        p_buf[3 + add_info_len] = 2;

        tuya_ble_comm_data_send(FRM_WITH_SRC_TYPE_DP_DATA_QUERY_RESP, ack_sn, p_buf, 4 + add_info_len, ENCRYPTION_MODE_SESSION_KEY);
        tuya_ble_free(p_buf);
        return;
    } else {
        p_buf[3 + add_info_len] = 0x00;

        tuya_ble_comm_data_send(FRM_WITH_SRC_TYPE_DP_DATA_QUERY_RESP, ack_sn, p_buf, 4 + add_info_len, ENCRYPTION_MODE_SESSION_KEY);
        tuya_ble_free(p_buf);
    }
    event.evt = TUYA_BLE_CB_EVT_WITH_SRC_TYPE_DP_QUERY;

    if (dp_num > 0) {
        ble_cb_evt_buffer = (UINT8_T *)tuya_ble_malloc(dp_num);
        if (ble_cb_evt_buffer == NULL) {
            TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
            return;
        } else {
            memcpy(ble_cb_evt_buffer, &recv_data[17 + add_info_len], dp_num);
        }
    }
    event.dp_query_data_with_src_type.p_data = ble_cb_evt_buffer;
    event.dp_query_data_with_src_type.data_len = dp_num;

    if (add_info_len > 0) {
        ble_cb_evt_add_info_buffer = (UINT8_T *)tuya_ble_malloc(add_info_len);
        if (ble_cb_evt_add_info_buffer == NULL) {
            if (dp_num > 0) {
                tuya_ble_free(ble_cb_evt_buffer);
            }
            TUYA_BLE_LOG_ERROR("ble_cb_evt_add_info_buffer malloc failed.");
            return;
        } else {
            memcpy(ble_cb_evt_add_info_buffer, &recv_data[16], add_info_len);
        }
    }

    event.dp_query_data_with_src_type.p_add_info = ble_cb_evt_add_info_buffer;
    event.dp_query_data_with_src_type.add_info_len = add_info_len;
    event.dp_query_data_with_src_type.src_type = src_type;

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_query_req-tuya ble send cb event failed.");
        if (dp_num > 0) {
            tuya_ble_free(ble_cb_evt_buffer);
        }
        if (add_info_len > 0) {
            tuya_ble_free(ble_cb_evt_add_info_buffer);
        }
    } else {

    }
}

STATIC VOID_T tuya_ble_handle_dp_data_write_with_src_type_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    mtp_ret ret;
    klv_node_s *list = NULL;
    UINT8_T *p_buf;
    UINT8_T *ble_cb_evt_add_info_buffer = NULL;
    UINT8_T *ble_cb_evt_buffer = NULL;
    UINT16_T data_len = 0;
    UINT8_T add_info_len;
    UINT32_T ack_sn = 0;
    tuya_ble_cb_evt_param_t event;

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    data_len = (recv_data[11] << 8) | recv_data[12];

    if (data_len < 7 || data_len < 7 + recv_data[19]) {
        TUYA_BLE_LOG_ERROR("frame error,could not be processed.");
        return;
    }

    add_info_len = recv_data[19];

    p_buf = (UINT8_T *)tuya_ble_malloc(8 + add_info_len);
    if (p_buf == NULL) {
        TUYA_BLE_LOG_ERROR("p_buf malloc failed.");
        return;
    }

    if ((data_len > TUYA_BLE_RECEIVE_MAX_DP_DATA_LEN + 2 + add_info_len)) {
        TUYA_BLE_LOG_ERROR("cmd dp write error,receive data len == %d", data_len);

        memcpy(&p_buf[0], &recv_data[13], 7 + add_info_len);
        p_buf[7 + add_info_len] = 1;

        tuya_ble_comm_data_send(FRM_DP_DATA_WITH_SRC_TYPE_WRITE_RESP, ack_sn, p_buf, 8 + add_info_len, ENCRYPTION_MODE_SESSION_KEY);
        tuya_ble_free(p_buf);
        return;
    } else if (recv_data[13] != 0) {
        TUYA_BLE_LOG_ERROR("cmd dp write version not support");

        memcpy(&p_buf[0], &recv_data[13], 7 + add_info_len);
        p_buf[7 + add_info_len] = 1;

        tuya_ble_comm_data_send(FRM_DP_DATA_WITH_SRC_TYPE_WRITE_RESP, ack_sn, p_buf, 8 + add_info_len, ENCRYPTION_MODE_SESSION_KEY);
        tuya_ble_free(p_buf);
        return;
    } else {

    }

    TUYA_BLE_LOG_HEXDUMP_DEBUG("cmd_dp_write data : ", recv_data + 13, data_len);
    memcpy(&p_buf[0], &recv_data[13], 7 + add_info_len); //version...add_info
    ret = data_2_klvlist(&recv_data[20 + add_info_len], data_len - 7 - add_info_len, &list, 1);
    if (MTP_OK != ret) {
        TUYA_BLE_LOG_ERROR("cmd rx fail-%d", ret);
        p_buf[7 + add_info_len] = 1;
        tuya_ble_comm_data_send(FRM_DP_DATA_WITH_SRC_TYPE_WRITE_RESP, ack_sn, p_buf, 8 + add_info_len, ENCRYPTION_MODE_SESSION_KEY);
        tuya_ble_free(p_buf);
        return;
    }

    free_klv_list(list);

    event.evt = TUYA_BLE_CB_EVT_WITH_SRC_TYPE_DP_DATA_RECEIVED;

    ble_cb_evt_buffer = (UINT8_T *)tuya_ble_malloc(data_len - 7 - add_info_len);
    if (ble_cb_evt_buffer == NULL) {
        TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
        p_buf[7 + add_info_len] = 1;
        tuya_ble_comm_data_send(FRM_DP_DATA_WITH_SRC_TYPE_WRITE_RESP, ack_sn, p_buf, 8 + add_info_len, ENCRYPTION_MODE_SESSION_KEY);
        tuya_ble_free(p_buf);
        return;
    }

    ble_cb_evt_add_info_buffer = (UINT8_T *)tuya_ble_malloc(add_info_len);
    if (ble_cb_evt_add_info_buffer == NULL) {
        TUYA_BLE_LOG_ERROR("ble_cb_evt_add_info_buffer malloc failed.");
        p_buf[7 + add_info_len] = 1;
        tuya_ble_comm_data_send(FRM_DP_DATA_WITH_SRC_TYPE_WRITE_RESP, ack_sn, p_buf, 8 + add_info_len, ENCRYPTION_MODE_SESSION_KEY);
        tuya_ble_free(p_buf);
        tuya_ble_free(ble_cb_evt_buffer);
        return;
    }

    memcpy(ble_cb_evt_buffer, &recv_data[20 + add_info_len], data_len - 7 - add_info_len);
    memcpy(ble_cb_evt_add_info_buffer, &recv_data[20], add_info_len);
    event.dp_data_with_src_type_received_data.sn = (recv_data[14] << 24) | (recv_data[15] << 16) | (recv_data[16] << 8) | recv_data[17];
    event.dp_data_with_src_type_received_data.p_data = ble_cb_evt_buffer;
    event.dp_data_with_src_type_received_data.data_len = data_len - 7 - add_info_len;
    event.dp_data_with_src_type_received_data.add_info_len = add_info_len;
    event.dp_data_with_src_type_received_data.p_add_info = ble_cb_evt_add_info_buffer;
    event.dp_data_with_src_type_received_data.src_type = recv_data[18];

    if (tuya_ble_cb_event_send(&event) != 0) {
        tuya_ble_free(ble_cb_evt_buffer);
        tuya_ble_free(ble_cb_evt_add_info_buffer);
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_write_req-tuya ble send cb event failed.");
        p_buf[7 + add_info_len] = 1;
        tuya_ble_comm_data_send(FRM_DP_DATA_WITH_SRC_TYPE_WRITE_RESP, ack_sn, p_buf, 8 + add_info_len, ENCRYPTION_MODE_SESSION_KEY);
        tuya_ble_free(p_buf);
    } else {
        p_buf[7 + add_info_len] = 0;
        tuya_ble_comm_data_send(FRM_DP_DATA_WITH_SRC_TYPE_WRITE_RESP, ack_sn, p_buf, 8 + add_info_len, ENCRYPTION_MODE_SESSION_KEY);
        tuya_ble_free(p_buf);
    }

}

STATIC VOID_T tuya_ble_handle_dp_data_with_src_type_send_response(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T *ble_cb_evt_add_info_buffer = NULL;
    tuya_ble_cb_evt_param_t event;
    UINT16_T data_len = recv_data[11] << 8 | recv_data[12];
    UINT8_T add_info_len = 0;

    if (data_len < 10 || data_len < 10 + recv_data[19]) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_send_response- invalid data len received.");
        return;
    }
    add_info_len = recv_data[19];
    if (add_info_len > 0) {
        ble_cb_evt_add_info_buffer = (UINT8_T *)tuya_ble_malloc(add_info_len);

        if (ble_cb_evt_add_info_buffer == NULL) {
            TUYA_BLE_LOG_ERROR("ble_cb_evt_add_info_buffer malloc failed.");
            return;
        } else {
            memcpy(ble_cb_evt_add_info_buffer, recv_data + 20, add_info_len);
        }
    }
    event.evt = TUYA_BLE_CB_EVT_DP_DATA_WITH_SRC_TYPE_SEND_RESPONSE;
    event.dp_with_src_type_send_response_data.sn = recv_data[14] << 24 | recv_data[15] << 16 | recv_data[16] << 8 | recv_data[17];
    event.dp_with_src_type_send_response_data.ack = ((recv_data[20 + add_info_len] & 0x80) ? DP_SEND_WITHOUT_RESPONSE : DP_SEND_WITH_RESPONSE);
    event.dp_with_src_type_send_response_data.src_type = recv_data[18];
    event.dp_with_src_type_send_response_data.add_info_len = add_info_len;
    event.dp_with_src_type_send_response_data.p_add_info = ble_cb_evt_add_info_buffer;

    if ((recv_data[20 + add_info_len] & 0x0F) == 1) {
        event.dp_with_src_type_send_response_data.type = DP_SEND_TYPE_PASSIVE;
    } else if ((recv_data[20 + add_info_len] & 0x0F) == 0) {
        event.dp_with_src_type_send_response_data.type = DP_SEND_TYPE_ACTIVE;
    } else {
        tuya_ble_free(ble_cb_evt_add_info_buffer);
        return;
    }

    if (recv_data[21 + add_info_len] > 3) {
        tuya_ble_free(ble_cb_evt_add_info_buffer);
        return;
    }
    event.dp_with_src_type_send_response_data.mode = recv_data[21 + add_info_len];
    event.dp_with_src_type_send_response_data.status = recv_data[22 + add_info_len];

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_send_response-tuya ble send cb event failed.");
        tuya_ble_free(ble_cb_evt_add_info_buffer);
    } else {

    }
}

STATIC VOID_T tuya_ble_handle_dp_data_with_src_type_and_time_send_response(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T *ble_cb_evt_add_info_buffer = NULL;
    tuya_ble_cb_evt_param_t event;
    UINT16_T data_len = recv_data[11] << 8 | recv_data[12];
    UINT8_T add_info_len = 0;

    if (data_len < 10 || data_len < 10 + recv_data[19]) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_with_time_send_response- invalid data len received.");
        return;
    }

    add_info_len = recv_data[19];
    if (add_info_len > 0) {
        ble_cb_evt_add_info_buffer = (UINT8_T *)tuya_ble_malloc(add_info_len);

        if (ble_cb_evt_add_info_buffer == NULL) {
            TUYA_BLE_LOG_ERROR("ble_cb_evt_add_info_buffer malloc failed.");
            return;
        } else {
            memcpy(ble_cb_evt_add_info_buffer, recv_data + 20, add_info_len);
        }
    }

    event.evt = TUYA_BLE_CB_EVT_DP_DATA_WITH_SRC_TYPE_AND_TIME_SEND_RESPONSE;
    event.dp_with_src_type_and_time_send_response_data.sn = recv_data[14] << 24 | recv_data[15] << 16 | recv_data[16] << 8 | recv_data[17];
    event.dp_with_src_type_and_time_send_response_data.ack = ((recv_data[20 + add_info_len] & 0x80) ? DP_SEND_WITHOUT_RESPONSE : DP_SEND_WITH_RESPONSE);
    event.dp_with_src_type_and_time_send_response_data.src_type = recv_data[18];
    event.dp_with_src_type_and_time_send_response_data.add_info_len = add_info_len;
    event.dp_with_src_type_and_time_send_response_data.p_add_info = ble_cb_evt_add_info_buffer;

    if ((recv_data[20 + add_info_len] & 0x0F) == 1) {
        event.dp_with_src_type_and_time_send_response_data.type = DP_SEND_TYPE_PASSIVE;
    } else if ((recv_data[20 + add_info_len] & 0x0F) == 0) {
        event.dp_with_src_type_and_time_send_response_data.type = DP_SEND_TYPE_ACTIVE;
    } else {
        tuya_ble_free(ble_cb_evt_add_info_buffer);
        return;
    }

    if (recv_data[21 + add_info_len] > 3) {
        tuya_ble_free(ble_cb_evt_add_info_buffer);
        return;
    }
    event.dp_with_src_type_and_time_send_response_data.mode = recv_data[21 + add_info_len];
    event.dp_with_src_type_and_time_send_response_data.status = recv_data[22 + add_info_len];

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_with_time_send_response-tuya ble send cb event failed.");
        tuya_ble_free(ble_cb_evt_add_info_buffer);
    } else {

    }
}

#endif /* end of #if (TUYA_BLE_MUTI_DATA_SOURCE_SUPPORTED&&TUYA_BLE_PROTOCOL_VERSION_LOW >= 5) */

STATIC VOID_T tuya_ble_handle_dp_data_write_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    mtp_ret ret;
    klv_node_s *list = NULL;
    UINT8_T p_buf[6];
    UINT16_T data_len = 0;
    UINT32_T ack_sn = 0;
    tuya_ble_cb_evt_param_t event;

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    data_len = (recv_data[11] << 8) | recv_data[12];

    if ((data_len < 5) || (data_len > TUYA_BLE_RECEIVE_MAX_DP_DATA_LEN)) {
        TUYA_BLE_LOG_ERROR("cmd dp write error,receive data len == %d", data_len);
        if (data_len < 5) {
            memcpy(&p_buf[0], &recv_data[13], data_len);
        } else {
            memcpy(&p_buf[0], &recv_data[13], 5);
        }
        p_buf[5] = 1;
        tuya_ble_comm_data_send(FRM_DP_DATA_WRITE_RESP, ack_sn, p_buf, 6, ENCRYPTION_MODE_SESSION_KEY);
        return;
    } else if (recv_data[13] != 0) {
        TUYA_BLE_LOG_ERROR("cmd dp write version not support");

        memcpy(&p_buf[0], &recv_data[13], 5);

        p_buf[5] = 1;
        tuya_ble_comm_data_send(FRM_DP_DATA_WRITE_RESP, ack_sn, p_buf, 6, ENCRYPTION_MODE_SESSION_KEY);
        return;
    } else {

    }

    TUYA_BLE_LOG_HEXDUMP_DEBUG("cmd_dp_write data : ", recv_data + 13, data_len);
    memcpy(&p_buf[0], &recv_data[13], 5);
    ret = data_2_klvlist(&recv_data[18], data_len - 5, &list, 1);
    if (MTP_OK != ret) {
        TUYA_BLE_LOG_ERROR("cmd rx fail-%d", ret);
        p_buf[5] = 1;
        tuya_ble_comm_data_send(FRM_DP_DATA_WRITE_RESP, ack_sn, p_buf, 6, ENCRYPTION_MODE_SESSION_KEY);
        return;
    }

    free_klv_list(list);

    event.evt = TUYA_BLE_CB_EVT_DP_DATA_RECEIVED;

    UINT8_T *ble_cb_evt_buffer = (UINT8_T *)tuya_ble_malloc(data_len - 5);
    if (ble_cb_evt_buffer == NULL) {
        TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
        p_buf[5] = 0x01;
        tuya_ble_comm_data_send(FRM_DP_DATA_WRITE_RESP, ack_sn, p_buf, 6, ENCRYPTION_MODE_SESSION_KEY);
        return;
    } else {
        memcpy(ble_cb_evt_buffer, &recv_data[18], data_len - 5);
        event.dp_received_data.sn = (recv_data[14] << 24) | (recv_data[15] << 16) | (recv_data[16] << 8) | recv_data[17];
        event.dp_received_data.p_data = ble_cb_evt_buffer;
        event.dp_received_data.data_len = data_len - 5;
    }

    if (tuya_ble_cb_event_send(&event) != 0) {
        tuya_ble_free(ble_cb_evt_buffer);
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_write_req-tuya ble send cb event failed.");
        p_buf[5] = 0x01;
        tuya_ble_comm_data_send(FRM_DP_DATA_WRITE_RESP, ack_sn, p_buf, 6, ENCRYPTION_MODE_SESSION_KEY);
    } else {
        p_buf[5] = 0x00;
        tuya_ble_comm_data_send(FRM_DP_DATA_WRITE_RESP, ack_sn, p_buf, 6, ENCRYPTION_MODE_SESSION_KEY);
    }

}

STATIC VOID_T tuya_ble_handle_dp_data_send_response(UINT8_T *recv_data, UINT16_T recv_len)
{
    tuya_ble_cb_evt_param_t event;
    UINT16_T data_len = recv_data[11] << 8 | recv_data[12];

    if (data_len != 8) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_send_response- invalid data len received.");
        return;
    }

    event.evt = TUYA_BLE_CB_EVT_DP_DATA_SEND_RESPONSE;
    event.dp_send_response_data.sn = recv_data[14] << 24 | recv_data[15] << 16 | recv_data[16] << 8 | recv_data[17];
    event.dp_send_response_data.ack = ((recv_data[18] & 0x80) ? DP_SEND_WITHOUT_RESPONSE : DP_SEND_WITH_RESPONSE);
    if ((recv_data[18] & 0x0F) == 1) {
        event.dp_send_response_data.type = DP_SEND_TYPE_PASSIVE;
    } else if ((recv_data[18] & 0x0F) == 0) {
        event.dp_send_response_data.type = DP_SEND_TYPE_ACTIVE;
    } else {
        return;
    }

    if (recv_data[19] > 3) {
        return;
    }
    event.dp_send_response_data.mode = recv_data[19];
    event.dp_send_response_data.status = recv_data[20];

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_send_response-tuya ble send cb event failed.");
    } else {

    }
}

STATIC VOID_T tuya_ble_handle_dp_data_with_time_send_response(UINT8_T *recv_data, UINT16_T recv_len)
{
    tuya_ble_cb_evt_param_t event;
    UINT16_T data_len = recv_data[11] << 8 | recv_data[12];

    if (data_len != 8) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_with_time_send_response- invalid data len received.");
        return;
    }

    event.evt = TUYA_BLE_CB_EVT_DP_DATA_WITH_TIME_SEND_RESPONSE;
    event.dp_with_time_send_response_data.sn = recv_data[14] << 24 | recv_data[15] << 16 | recv_data[16] << 8 | recv_data[17];
    event.dp_with_time_send_response_data.ack = ((recv_data[18] & 0x80) ? DP_SEND_WITHOUT_RESPONSE : DP_SEND_WITH_RESPONSE);
    if ((recv_data[18] & 0x0F) == 1) {
        event.dp_with_time_send_response_data.type = DP_SEND_TYPE_PASSIVE;
    } else if ((recv_data[18] & 0x0F) == 0) {
        event.dp_with_time_send_response_data.type = DP_SEND_TYPE_ACTIVE;
    } else {
        return;
    }

    if (recv_data[19] > 3) {
        return;
    }
    event.dp_with_time_send_response_data.mode = recv_data[19];
    event.dp_with_time_send_response_data.status = recv_data[20];

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_with_time_send_response-tuya ble send cb event failed.");
    } else {

    }
}

#else //(TUYA_BLE_PROTOCOL_VERSION_HIGN<4)

STATIC VOID_T tuya_ble_handle_dp_write_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    mtp_ret ret;
    klv_node_s *list = NULL;
    UINT8_T p_buf[1];
    UINT16_T data_len = 0;
    UINT32_T ack_sn = 0;
    tuya_ble_cb_evt_param_t event;

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    data_len = (recv_data[11] << 8) | recv_data[12];

    if ((data_len == 0) || (data_len > TUYA_BLE_RECEIVE_MAX_DP_DATA_LEN)) {
        TUYA_BLE_LOG_ERROR("cmd dp write receive data len == %d", data_len);
        p_buf[0] = 0x01;
        tuya_ble_comm_data_send(FRM_CMD_RESP, ack_sn, p_buf, 1, ENCRYPTION_MODE_SESSION_KEY);
        return;
    }
    TUYA_BLE_LOG_HEXDUMP_DEBUG("cmd_dp_write data : ", recv_data + 13, data_len);
    ret = data_2_klvlist(&recv_data[13], data_len, &list, 0);
    if (MTP_OK != ret) {
        TUYA_BLE_LOG_ERROR("cmd rx fail-%d", ret);
        p_buf[0] = 0x01;
        tuya_ble_comm_data_send(FRM_CMD_RESP, ack_sn, p_buf, 1, ENCRYPTION_MODE_SESSION_KEY);
        return;
    }

    free_klv_list(list);
    p_buf[0] = 0x00;

    tuya_ble_comm_data_send(FRM_CMD_RESP, ack_sn, p_buf, 1, ENCRYPTION_MODE_SESSION_KEY);

    event.evt = TUYA_BLE_CB_EVT_DP_WRITE;

    UINT8_T *ble_cb_evt_buffer = (UINT8_T *)tuya_ble_malloc(data_len);
    if (ble_cb_evt_buffer == NULL) {
        TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
        return;
    } else {
        memcpy(ble_cb_evt_buffer, &recv_data[13], data_len);
    }
    event.dp_write_data.p_data = ble_cb_evt_buffer;
    event.dp_write_data.data_len = data_len;

    if (tuya_ble_cb_event_send(&event) != 0) {
        tuya_ble_free(ble_cb_evt_buffer);
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_write_req-tuya ble send cb event failed.");
    } else {

    }

}

STATIC VOID_T tuya_ble_handle_dp_data_report_res(UINT8_T *recv_data, UINT16_T recv_len)
{
    tuya_ble_cb_evt_param_t event;

    event.evt = TUYA_BLE_CB_EVT_DP_DATA_REPORT_RESPONSE;
    event.dp_response_data.status = recv_data[13];

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_report_res-tuya ble send cb event failed.");
    } else {

    }
}

STATIC VOID_T tuya_ble_handle_dp_data_with_time_report_res(UINT8_T *recv_data, UINT16_T recv_len)
{
    tuya_ble_cb_evt_param_t event;

    event.evt = TUYA_BLE_CB_EVT_DP_DATA_WTTH_TIME_REPORT_RESPONSE;
    event.dp_response_data.status = recv_data[13];

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_report_res-tuya ble send cb event failed.");
    } else {

    }
}

STATIC VOID_T tuya_ble_handle_dp_data_with_flag_report_res(UINT8_T *recv_data, UINT16_T recv_len)
{
    tuya_ble_cb_evt_param_t event;
    UINT16_T data_len = recv_data[11] << 8 | recv_data[12];

    if (data_len != 4) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_with_flag_report_res- invalid data len received.");
        return;
    }

    event.evt = TUYA_BLE_CB_EVT_DP_DATA_WITH_FLAG_REPORT_RESPONSE;
    event.dp_with_flag_response_data.sn = recv_data[13] << 8 | recv_data[14];
    event.dp_with_flag_response_data.mode = recv_data[15];
    event.dp_with_flag_response_data.status = recv_data[16];

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_with_flag_report_res-tuya ble send cb event failed.");
    } else {

    }
}

STATIC VOID_T tuya_ble_handle_dp_data_with_flag_and_time_report_res(UINT8_T *recv_data, UINT16_T recv_len)
{
    tuya_ble_cb_evt_param_t event;
    UINT16_T data_len = recv_data[11] << 8 | recv_data[12];

    if (data_len != 4) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_with_flag_and_time_report_res- invalid data len received.");
        return;
    }

    event.evt = TUYA_BLE_CB_EVT_DP_DATA_WITH_FLAG_AND_TIME_REPORT_RESPONSE;
    event.dp_with_flag_and_time_response_data.sn = recv_data[13] << 8 | recv_data[14];
    event.dp_with_flag_and_time_response_data.mode = recv_data[15];
    event.dp_with_flag_and_time_response_data.status = recv_data[16];

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_with_flag_and_time_report_res-tuya ble send cb event failed.");
    } else {

    }
}

#endif

#if TUYA_BLE_BR_EDR_SUPPORTED

VOID_T tuya_ble_br_edr_data_info_update_internal(tuya_ble_br_edr_data_info_t *p_data)
{
    if (br_edr_data_init_flag == 0) {
        br_edr_data_init_flag = 1;
    }

    memcpy(&br_edr_data_info, p_data, sizeof(tuya_ble_br_edr_data_info_t));
}

STATIC VOID_T tuya_ble_handle_br_edr_data_info_req(UINT8_T *recv_data, UINT16_T recv_len)
{
    UINT8_T p_buf[50];
    UINT8_T encry_mode = 0;
    UINT32_T ack_sn = 0;
    UINT16_T data_len;

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    } else {
        encry_mode = ENCRYPTION_MODE_KEY_2;
    }
    memset(p_buf, 0, SIZEOF(p_buf));
    if (br_edr_data_init_flag) {
        p_buf[0] = 0;
        memcpy(&p_buf[1], br_edr_data_info.mac, 6);
        p_buf[7] = br_edr_data_info.dev_ability;
        if (br_edr_data_info.is_paired) {
            p_buf[8] |= 0x01;
        }
        p_buf[9] = br_edr_data_info.connect_status;
        p_buf[10] = br_edr_data_info.name_len;
        if (p_buf[10] > 32) {
            p_buf[10] = 32;
        }
        memcpy(&p_buf[11], br_edr_data_info.name, p_buf[10]);
        data_len = p_buf[10] + 11;
    } else {
        p_buf[0] = 1;
        data_len = 11;
    }

    TAL_PR_HEXDUMP_DEBUG("tuya_ble_handler_bredr_data_info:", p_buf, data_len);
    tuya_ble_comm_data_send(TUYA_BLE_FRM_BR_EDR_DATA_INFO_RESP, ack_sn,  p_buf, data_len,  encry_mode);
}

#endif

VOID_T tuya_ble_evt_process(UINT16_T cmd, UINT8_T *recv_data, UINT32_T recv_len)
{
    switch (cmd) {
#if (TUYA_BLE_PROTOCOL_VERSION_HIGN == 5)
#if (TUYA_BLE_MUTI_DATA_SOURCE_SUPPORTED)
    case FRM_WITH_SRC_TYPE_DP_DATA_QUERY:
        tuya_ble_handle_dp_query_with_src_type_req(recv_data, recv_len);
        break;
    case FRM_DP_DATA_WITH_SRC_TYPE_WRITE_REQ:
        tuya_ble_handle_dp_data_write_with_src_type_req(recv_data, recv_len);
        break;
    case FRM_DP_DATA_WITH_SRC_TYPE_SEND_RESP:
        tuya_ble_handle_dp_data_with_src_type_send_response(recv_data, recv_len);
        break;
    case FRM_DP_DATA_WITH_SRC_TYPE_AND_TIME_SEND_RESP:
        tuya_ble_handle_dp_data_with_src_type_and_time_send_response(recv_data, recv_len);
        break;
#endif
    case FRM_DP_DATA_WRITE_REQ:
        tuya_ble_handle_dp_data_write_req(recv_data, recv_len);
        break;
    case FRM_DP_DATA_SEND_RESP:
        tuya_ble_handle_dp_data_send_response(recv_data, recv_len);
        break;
    case FRM_DP_DATA_WITH_TIME_SEND_RESP:
        tuya_ble_handle_dp_data_with_time_send_response(recv_data, recv_len);
        break;

#if (TUYA_BLE_SECURE_CONNECTION_TYPE == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)

    case FRM_AUTHENTICATE_PHASE_1_REQ:
        tuya_ble_handle_authenticate_phase1(recv_data, recv_len);
        break;
    case FRM_AUTHENTICATE_PHASE_2_REQ:
        tuya_ble_handle_authenticate_phase2(recv_data, recv_len);
        break;
    case FRM_AUTHENTICATE_PHASE_3_REQ:
        tuya_ble_handle_authenticate_phase3(recv_data, recv_len);
        break;

#endif

#if (TUYA_BLE_FEATURE_WEATHER_ENABLE != 0)
    case FRM_WEATHER_DATA_REQUEST_RESP:
        tuya_ble_handle_weather_data_request_response(recv_data, recv_len);
        break;
    case FRM_WEATHER_DATA_RECEIVED:
        tuya_ble_handle_weather_data_received(recv_data, recv_len);
        break;
#endif

#if (TUYA_BLE_FEATURE_IOT_CHANNEL_ENABLE != 0)
    case FRM_IOT_DATA_REQUEST_RESP:
        tuya_ble_handle_iot_data_request_response(recv_data, recv_len);
        break;
    case FRM_IOT_DATA_RECEIVED:
        tuya_ble_handle_iot_data_received(recv_data, recv_len);
        break;
#endif

#if (TUYA_BLE_FEATURE_APP_PASSTHROUGH_ENABLE != 0)
    case FRM_APP_DOWNSTREAM_PASSTHROUGH_REQ:
    case FRM_APP_UPSTREAM_PASSTHROUGH_RESP:
        tuya_ble_handle_business_passthrough_data(cmd, recv_data, recv_len);
        break;
#endif

#elif (TUYA_BLE_PROTOCOL_VERSION_HIGN == 4)
#if (TUYA_BLE_MUTI_DATA_SOURCE_SUPPORTED&&TUYA_BLE_PROTOCOL_VERSION_LOW >= 5)
    case FRM_WITH_SRC_TYPE_DP_DATA_QUERY:
        tuya_ble_handle_dp_query_with_src_type_req(recv_data, recv_len);
        break;
    case FRM_DP_DATA_WITH_SRC_TYPE_WRITE_REQ:
        tuya_ble_handle_dp_data_write_with_src_type_req(recv_data, recv_len);
        break;
    case FRM_DP_DATA_WITH_SRC_TYPE_SEND_RESP:
        tuya_ble_handle_dp_data_with_src_type_send_response(recv_data, recv_len);
        break;
    case FRM_DP_DATA_WITH_SRC_TYPE_AND_TIME_SEND_RESP:
        tuya_ble_handle_dp_data_with_src_type_and_time_send_response(recv_data, recv_len);
        break;
#endif
    case FRM_DP_DATA_WRITE_REQ:
        tuya_ble_handle_dp_data_write_req(recv_data, recv_len);
        break;
    case FRM_DP_DATA_SEND_RESP:
        tuya_ble_handle_dp_data_send_response(recv_data, recv_len);
        break;
    case FRM_DP_DATA_WITH_TIME_SEND_RESP:
        tuya_ble_handle_dp_data_with_time_send_response(recv_data, recv_len);
        break;

#if (TUYA_BLE_SECURE_CONNECTION_TYPE == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)

    case FRM_AUTHENTICATE_PHASE_1_REQ:
        tuya_ble_handle_authenticate_phase1(recv_data, recv_len);
        break;
    case FRM_AUTHENTICATE_PHASE_2_REQ:
        tuya_ble_handle_authenticate_phase2(recv_data, recv_len);
        break;
    case FRM_AUTHENTICATE_PHASE_3_REQ:
        tuya_ble_handle_authenticate_phase3(recv_data, recv_len);
        break;

#endif

#if (TUYA_BLE_FEATURE_WEATHER_ENABLE != 0)
    case FRM_WEATHER_DATA_REQUEST_RESP:
        tuya_ble_handle_weather_data_request_response(recv_data, recv_len);
        break;
    case FRM_WEATHER_DATA_RECEIVED:
        tuya_ble_handle_weather_data_received(recv_data, recv_len);
        break;
#endif

#if (TUYA_BLE_FEATURE_IOT_CHANNEL_ENABLE != 0)
    case FRM_IOT_DATA_REQUEST_RESP:
        tuya_ble_handle_iot_data_request_response(recv_data, recv_len);
        break;
    case FRM_IOT_DATA_RECEIVED:
        tuya_ble_handle_iot_data_received(recv_data, recv_len);
        break;
#endif

#if (TUYA_BLE_FEATURE_APP_PASSTHROUGH_ENABLE != 0)
    case FRM_APP_DOWNSTREAM_PASSTHROUGH_REQ:
    case FRM_APP_UPSTREAM_PASSTHROUGH_RESP:
        tuya_ble_handle_business_passthrough_data(cmd, recv_data, recv_len);
        break;
#endif

#else
    case FRM_CMD_SEND:
        tuya_ble_handle_dp_write_req(recv_data, recv_len);
        break;
    case FRM_STAT_REPORT_RESP:
        tuya_ble_handle_dp_data_report_res(recv_data, recv_len);
        break;
    case FRM_DATA_WITH_FLAG_REPORT_RESP:
        tuya_ble_handle_dp_data_with_flag_report_res(recv_data, recv_len);
        break;
    case FRM_STAT_WITH_TIME_REPORT_RESP:
        tuya_ble_handle_dp_data_with_time_report_res(recv_data, recv_len);
        break;
    case FRM_DATA_WITH_FLAG_AND_TIME_REPORT_RESP:
        tuya_ble_handle_dp_data_with_flag_and_time_report_res(recv_data, recv_len);
        break;
#endif

    case FRM_STATE_QUERY:
        tuya_ble_handle_dp_query_req(recv_data, recv_len);
        break;
    case FRM_QRY_DEV_INFO_REQ:
        tuya_ble_handle_dev_info_req(recv_data, recv_len);
        break;
    case PAIR_REQ:
        tuya_ble_handle_pair_req(recv_data, recv_len);
        break;
    case FRM_NET_CONFIG_INFO_REQ:
        tuya_ble_handle_net_config_info_req(recv_data, recv_len);
        break;
    case FRM_DATA_PASSTHROUGH_REQ:
        tuya_ble_handle_ble_passthrough_data_req(recv_data, recv_len);
        break;
#if defined(TUYA_BLE_FEATURE_SPEECH_ENABLE) && (TUYA_BLE_FEATURE_SPEECH_ENABLE == 1)
    case FRM_SPEECH_CONTROL_REQ:
        tuya_ble_handle_ble_speech_rxs(TUYA_BLE_CB_EVT_SPEECH_CONTROL, recv_data, recv_len);
        break;
    case FRM_SPEECH_RAW_DATA_REPORT_RSP:
        tuya_ble_handle_ble_speech_rxs(TUYA_BLE_CB_EVT_SPEECH_RAW_DATA_REPORT_RSP, recv_data, recv_len);
        break;
    case FRM_SPEECH_RESULT_WRITE_REQ:
        tuya_ble_handle_ble_speech_rxs(TUYA_BLE_CB_EVT_SPEECH_RESULT_WRITE, recv_data, recv_len);
        break;
    case FRM_SPEECH_CLOCK_SETTING_REQ:
        tuya_ble_handle_ble_speech_rxs(TUYA_BLE_CB_EVT_SPEECH_CLOCK_SETTING, recv_data, recv_len);
        break;
    case FRM_SPEECH_TOKEN_READ_REQ:
        tuya_ble_handle_ble_speech_rxs(TUYA_BLE_CB_EVT_SPEECH_TOKEN_READ, recv_data, recv_len);
        break;
    case FRM_SPEECH_TOKEN_REPORT_RESP:
        tuya_ble_handle_ble_speech_rxs(TUYA_BLE_CB_EVT_SPEECH_TOKEN_REPORT_RSP, recv_data, recv_len);
        break;
    case FRM_SPEECH_TOKEN_WRITE_REQ:
        tuya_ble_handle_ble_speech_rxs(TUYA_BLE_CB_EVT_SPEECH_TOKEN_WRITE, recv_data, recv_len);
        break;
    case FRM_SPEECH_COMMON_SETTING_REQ:
        tuya_ble_handle_ble_speech_rxs(TUYA_BLE_CB_EVT_SPEECH_COMMON_SETTING, recv_data, recv_len);
        break;
#endif
#if defined(TUYA_BLE_FEATURE_GPT_ENABLE) && (TUYA_BLE_FEATURE_GPT_ENABLE == 1)
    case FRM_GPT_CONTROL_REQ:
        tuya_ble_handle_ble_gpt_rxs(TUYA_BLE_CB_EVT_GPT_CONTROL, recv_data, recv_len);
        break;
    case FRM_GPT_RAW_DATA_REPORT_RESP:
        tuya_ble_handle_ble_gpt_rxs(TUYA_BLE_CB_EVT_GPT_RAW_DATA_REPORT_RSP, recv_data, recv_len);
        break;
    case FRM_GPT_RESULT_WRITE_REQ:
        tuya_ble_handle_ble_gpt_rxs(TUYA_BLE_CB_EVT_GPT_RESULT_WRITE, recv_data, recv_len);
        break;
    case FRM_GPT_RAW_DATA_WRITE_REQ:
        tuya_ble_handle_ble_gpt_rxs(TUYA_BLE_CB_EVT_GPT_RAW_DATA_WRITE, recv_data, recv_len);
        break;
#endif
#if defined(TUYA_BLE_FEATURE_BULKDATA_ENABLE) && (TUYA_BLE_FEATURE_BULKDATA_ENABLE == 1)
    case FRM_BULK_DATA_READ_INFO_REQ:
    case FRM_BULK_DATA_READ_DATA_REQ:
    case FRM_BULK_DATA_ERASE_DATA_REQ:
        TUYA_BLE_LOG_INFO("RECEIVED BULK DATA CMD:0x%02x DATA LEN:0x%02x", cmd, recv_len);
        tuya_ble_handle_bulk_data_req(cmd, recv_data, recv_len);
        break;
#endif
    case FRM_OTA_START_REQ:
    case FRM_OTA_FILE_INFOR_REQ:
    case FRM_OTA_FILE_OFFSET_REQ:
    case FRM_OTA_DATA_REQ:
    case FRM_OTA_END_REQ:
#if defined(TUYA_BLE_SINGLE_BANK_OTA_MODE) && (TUYA_BLE_SINGLE_BANK_OTA_MODE == 1)
    case FRM_OTA_PREPARE_NOTIFICATION_REQ:
#endif
#if defined(TUYA_BLE_OTA_SIGNATURE_ENABLE) && (TUYA_BLE_OTA_SIGNATURE_ENABLE == 1)
    case FRM_OTA_SIGNATURE_DATA_REQ:
    case FRM_OTA_SIGNATURE_KEY_UPDATE_REQ:
#endif
        TUYA_BLE_LOG_INFO("RECEIVED OTA CMD:0x%02x DATA LEN:0x%02x", cmd, recv_len);
        tuya_ble_handle_ota_req(cmd, recv_data, recv_len);
        break;
#if defined(TUYA_BLE_FILE_ENABLE) && (TUYA_BLE_FILE_ENABLE == 1)
    case FRM_FILE_INFOR_REQ:
    case FRM_FILE_OFFSET_REQ:
    case FRM_FILE_DATA_REQ:
    case FRM_FILE_END_REQ:
        TUYA_BLE_LOG_INFO("RECV FILE CMD:0x%02x, DATA LEN:0x%02x", cmd, recv_len);
        tuya_ble_handle_file_req(cmd, recv_data, recv_len);
        break;
#endif
    case FRM_GET_UNIX_TIME_CHAR_MS_RESP:
        tuya_ble_handle_unix_time_char_ms_resp(recv_data, recv_len);
        break;
    case FRM_GET_UNIX_TIME_CHAR_DATE_RESP:
        tuya_ble_handle_unix_time_date_resp(recv_data, recv_len);
        break;
    case FRM_GET_APP_LOCAL_TIME_RESP:
        tuya_ble_handle_app_local_time_data_resp(recv_data, recv_len);
        break;
    case FRM_REMOTER_PROXY_AUTH_RESP:
        tuya_ble_handle_remoter_proxy_auth_data_resp(recv_data, recv_len);
        break;
    case FRM_REMOTER_GROUP_SET_REQ:
        tuya_ble_handle_remoter_group_set_req(recv_data, recv_len);
        break;
    case FRM_REMOTER_GROUP_DELETE_REQ:
        tuya_ble_handle_remoter_group_delete_req(recv_data, recv_len);
        break;
    case FRM_REMOTER_GROUP_GET_REQ:
        tuya_ble_handle_remoter_group_get_req(recv_data, recv_len);
        break;
    case FRM_GET_UNIX_TIME_WITH_DST_RESP:
        tuya_ble_handle_unix_time_with_dst_date_resp(recv_data, recv_len);
        break;
    case FRM_UNBONDING_REQ:
        TUYA_BLE_LOG_INFO("RECEIVED FRM_UNBONDING_REQ");
        tuya_ble_handle_unbond_req(recv_data, recv_len);
        break;
    case FRM_ANOMALY_UNBONDING_REQ:
        TUYA_BLE_LOG_INFO("RECEIVED FRM_ANOMALY_UNBONDING_REQ");
        tuya_ble_handle_anomaly_unbond_req(recv_data, recv_len);
        break;
    case FRM_DEVICE_RESET:
        TUYA_BLE_LOG_INFO("RECEIVED FRM_DEVICE_RESET_REQ");
        tuya_ble_handle_device_reset_req(recv_data, recv_len);
        break;
    case FRM_FACTORY_TEST_CMD:
        tuya_ble_handle_ble_factory_test_req(recv_data, recv_len);
        break;
#if TUYA_BLE_BR_EDR_SUPPORTED
    case TUYA_BLE_FRM_BR_EDR_DATA_INFO_QUREY:
        TUYA_BLE_LOG_INFO("RECEIVED TUYA_BLE_FRM_BR_EDR_DATA_INFO_QUREY");
        tuya_ble_handle_br_edr_data_info_req(recv_data, recv_len);
        break;
#endif
#if (TUYA_BLE_FEATURE_EXT_MODULE_ENABLE)
    case FRM_EM_DEV_INFO_QUERY_REQ:
        tuya_ble_handle_ext_module_dev_info_query_req(recv_data, recv_len);
        break;
    case FRM_EM_ACTIVE_INFO_RECEIVED:
        tuya_ble_handle_ext_module_active_data_received(recv_data, recv_len);
        break;
#endif

#if (TUYA_BLE_ACCESSORY_MOUNT_SUPPORTED != 0)
    case FRM_ACCESSORY_INFO_READING:
        tuya_ble_handle_accessory_info_reading(recv_data, recv_len);
        break;
    case FRM_ACCESSORY_INFO_REPORT_RESP:
        tuya_ble_handle_accessory_info_report_response(recv_data, recv_len);
        break;
    case FRM_ACCESSORY_ACTIVE_INFO_RECEIVED:
        tuya_ble_handle_accessory_active_info_received(recv_data, recv_len);
        break;

    case FRM_ACCESSORY_OTA_START_REQ:
    case FRM_ACCESSORY_OTA_FILE_INFOR_REQ:
    case FRM_ACCESSORY_OTA_FILE_OFFSET_REQ:
    case FRM_ACCESSORY_OTA_DATA_REQ:
    case FRM_ACCESSORY_OTA_END_REQ:
        TUYA_BLE_LOG_INFO("RECEIVED ACCESSORY OTA CMD:0x%02x DATA LEN:0x%02x", cmd, recv_len);
        tuya_ble_handle_accessory_ota_req(cmd, recv_data, recv_len);
#endif
    default:
        TUYA_BLE_LOG_WARNING("RECEIVED UNKNOWN BLE EVT CMD-0x%04x", cmd);
        break;
    }
}

UINT8_T tuya_ble_comm_data_send(UINT16_T cmd, UINT32_T ack_sn, UINT8_T *data, UINT16_T len, UINT8_T encryption_mode)
{
    mtp_ret ret;
    UINT16_T send_len = 0;
    UINT8_T *p_buf = NULL;
    UINT8_T iv[16];
    UINT16_T rand_value = 0, i = 0;
    UINT16_T crc16 = 0;
    UINT16_T en_len  = 0;
    UINT32_T out_len = 0;
    UINT32_T temp_len = 0;
    UINT32_T package_number = 0;
    UINT16_T p_version = 0;
    tuya_ble_r_air_send_packet  air_send_packet;

    memset(&air_send_packet, 0, SIZEOF(air_send_packet));

    tuya_ble_connect_status_t currnet_connect_status = tuya_ble_connect_status_get();

    if ((currnet_connect_status == BONDING_UNCONN) || (currnet_connect_status ==  UNBONDING_UNCONN)) {
        TUYA_BLE_LOG_ERROR("tuya ble commData_send failed,because ble not in connect status.");
        return 2;
    }

    if ((encryption_mode >= ENCRYPTION_MODE_MAX) || (len > TUYA_BLE_SEND_MAX_DATA_LEN)) {
        return 1;
    }

    if (encryption_mode != ENCRYPTION_MODE_NONE) {
        for (i = 0; i < 16; i += 2) {
            tuya_ble_rand_generator((void *)&rand_value, sizeof(uint16_t));
            iv[i + 0] = rand_value >> 8;
            iv[i + 1] = rand_value;
        }
        en_len = 17;
    } else {
        en_len = 1;
        memset(iv, 0, SIZEOF(iv));
    }

    air_send_packet.send_len = 14 + len;

    if (air_send_packet.send_len % 16 == 0) {
        temp_len = 0;
    } else {
        temp_len = 16 - air_send_packet.send_len % 16;
    }

    temp_len += air_send_packet.send_len;

    if (temp_len > (TUYA_BLE_AIR_FRAME_MAX - en_len)) {
        TUYA_BLE_LOG_ERROR("The length of the send to ble exceeds the maximum length.");
        air_send_packet.send_len = 0;
        return 1;
    }

    air_send_packet.send_data = NULL;

    air_send_packet.send_data = (UINT8_T *)tuya_ble_malloc(temp_len); //must temp_len

    if (air_send_packet.send_data == NULL) {
        TUYA_BLE_LOG_ERROR("air_send_packet.send_data malloc failed return 3.");
        air_send_packet.send_len = 0;
        return 3;
    } else {
        memset(air_send_packet.send_data,  0, temp_len);
    }

    UINT32_T send_sn = get_ble_send_sn();

    air_send_packet.send_data[0] = send_sn >> 24;
    air_send_packet.send_data[1] = send_sn >> 16;
    air_send_packet.send_data[2] = send_sn >> 8;
    air_send_packet.send_data[3] = send_sn;

    air_send_packet.send_data[4] = ack_sn >> 24;
    air_send_packet.send_data[5] = ack_sn >> 16;
    air_send_packet.send_data[6] = ack_sn >> 8;
    air_send_packet.send_data[7] = ack_sn;

    air_send_packet.send_data[8] = cmd >> 8;
    air_send_packet.send_data[9] = cmd;

    air_send_packet.send_data[10] = len >> 8;
    air_send_packet.send_data[11] = len;

    memcpy(&air_send_packet.send_data[12], data, len);

    crc16 = tal_util_crc16(air_send_packet.send_data, 12 + len,  NULL);

    air_send_packet.send_data[12 + len] = crc16 >> 8;
    air_send_packet.send_data[13 + len] = crc16;

    TUYA_BLE_LOG_DEBUG("--------------------------------- encryped mode: %d", encryption_mode);
    TUYA_BLE_LOG_HEXDUMP_DEBUG("ble_commData_send plain data", (UINT8_T *)air_send_packet.send_data, air_send_packet.send_len); //

    air_send_packet.encrypt_data_buf = NULL;

    air_send_packet.encrypt_data_buf = (UINT8_T *)tuya_ble_malloc(temp_len + en_len);

    if (air_send_packet.encrypt_data_buf == NULL) {
        TUYA_BLE_LOG_ERROR("air_send_packet.encrypt_data_buf malloc failed.");
        tuya_ble_free(air_send_packet.send_data);
        return 3;
    } else {
        air_send_packet.encrypt_data_buf_len = 0;
        memset(air_send_packet.encrypt_data_buf, 0, temp_len + en_len);
    }

    air_send_packet.encrypt_data_buf[0] = encryption_mode;

    if (encryption_mode != ENCRYPTION_MODE_NONE) {
        memcpy(&air_send_packet.encrypt_data_buf[1], iv, 16);
    } else {

    }

    p_version = (TUYA_BLE_PROTOCOL_VERSION_HIGN << 8) + TUYA_BLE_PROTOCOL_VERSION_LOW;
    if (tuya_ble_encryption(p_version, encryption_mode, iv, (UINT8_T *)air_send_packet.send_data,  air_send_packet.send_len,  &out_len,
                            (UINT8_T *)(air_send_packet.encrypt_data_buf + en_len), &tuya_ble_current_para, tuya_ble_pair_rand) == 0) {
        if ((encryption_mode != ENCRYPTION_MODE_NONE) && (out_len != temp_len)) {
            TUYA_BLE_LOG_ERROR("ble_commData_send encryed error.");
            tuya_ble_free(air_send_packet.send_data);
            tuya_ble_free(air_send_packet.encrypt_data_buf);
            return 1;
        }

        air_send_packet.encrypt_data_buf_len = en_len + out_len;

        TUYA_BLE_LOG_HEXDUMP_DEBUG("ble_commData_send encryped data", (UINT8_T *)air_send_packet.encrypt_data_buf, air_send_packet.encrypt_data_buf_len); //
    } else {
        TUYA_BLE_LOG_ERROR("ble_commData_send encryed fail.");
        tuya_ble_free(air_send_packet.send_data);
        tuya_ble_free(air_send_packet.encrypt_data_buf);
        return 1;
    }

    tuya_ble_free(air_send_packet.send_data);
    package_number = 0;
    if ((send_packet_data_len == 0) || (send_packet_data_len > TUYA_BLE_DATA_MTU_MAX)) {
        send_packet_data_len = 20;
    }
    trsmitr_init(&ty_trsmitr_proc_send);
    do {
        ret = trsmitr_send_pkg_encode_with_packet_length(&ty_trsmitr_proc_send, send_packet_data_len, TUYA_BLE_PROTOCOL_VERSION_HIGN,
                (UINT8_T *)(air_send_packet.encrypt_data_buf), air_send_packet.encrypt_data_buf_len);
        if (MTP_OK != ret && MTP_TRSMITR_CONTINUE != ret) {
            tuya_ble_free(air_send_packet.encrypt_data_buf);
            return 1;
        }
        send_len = get_trsmitr_subpkg_len(&ty_trsmitr_proc_send);
        p_buf = get_trsmitr_subpkg(&ty_trsmitr_proc_send);
        package_number++;
        tuya_ble_gatt_send_data_enqueue(p_buf, send_len);

    } while (ret == MTP_TRSMITR_CONTINUE);

    TUYA_BLE_LOG_INFO("ble_commData_send len = %d, package_number = %d, protocol version : 0x%02x, error code : 0x%02x", air_send_packet.encrypt_data_buf_len, package_number, TUYA_BLE_PROTOCOL_VERSION_HIGN, ret);

    tuya_ble_free(air_send_packet.encrypt_data_buf);

    return 0;
}

