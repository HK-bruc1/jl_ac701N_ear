/**
 * @file tal_ble_beacon.c
 * @brief This is tal_ble_beacon file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tal_ble_beacon.h"
#include "tal_ble_ccm.h"
#if defined(SUPPORT_TUYAOS) && (SUPPORT_TUYAOS == 1)
#include "tal_bluetooth.h"
#include "tal_sw_timer.h"
#else
#include "ty_ble.h"
#include "tuya_ble_port.h"
#endif

#include "tuya_ble_secure.h" // tuya_ble_device_id_encrypt_v4
#include "tuya_ble_internal_config.h" // TUYA_BLE_PROTOCOL_VERSION_HIGN
#include "tuya_ble_main.h" // tuya_ble_current_para.sys_settings.login_key
#include "tuya_ble_log.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define TAL_BLE_BEACON_DATA_LEN_MAX  31

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
static uint16_t sg_default_adv_interval = 0;
#if defined(SUPPORT_TUYAOS) && (SUPPORT_TUYAOS == 1)
static TIMER_ID sg_ble_beacon_timer_id = NULL;
#else
static tuya_ble_timer_t sg_ble_beacon_timer_id = NULL;
#endif

static TAL_BLE_BEACON_CB sg_ble_beacon_cb = NULL;

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/
#if defined(SUPPORT_TUYAOS) && (SUPPORT_TUYAOS == 1)
static void ble_beacon_timeout_handler(TIMER_ID timer_id, void *arg);
#else
static void ble_beacon_timeout_handler(tuya_ble_timer_t timer);
#endif




void tal_ble_beacon_init(uint16_t default_adv_interval, TAL_BLE_BEACON_CB cb)
{
    sg_default_adv_interval = default_adv_interval;
    sg_ble_beacon_cb = cb;
#if defined(SUPPORT_TUYAOS) && (SUPPORT_TUYAOS == 1)
    tal_sw_timer_create(ble_beacon_timeout_handler, NULL, &sg_ble_beacon_timer_id);
#else
    tuya_ble_timer_create(&sg_ble_beacon_timer_id, 2000, TUYA_BLE_TIMER_SINGLE_SHOT, ble_beacon_timeout_handler);
#endif
}

#if defined(SUPPORT_TUYAOS) && (SUPPORT_TUYAOS == 1)

static void ble_beacon_timeout_handler(TIMER_ID timer_id, void *arg)
{
    tuya_ble_adv_change();
    // Adv Interval
    TAL_BLE_ADV_PARAMS_T adv_param = {
        .adv_interval_min = sg_default_adv_interval * 8 / 5,
        .adv_interval_max = sg_default_adv_interval * 8 / 5,
        .adv_type = TAL_BLE_ADV_TYPE_CS_UNDIR,
    };
    tal_ble_advertising_start(&adv_param);

    if (sg_ble_beacon_cb != NULL) {
        sg_ble_beacon_cb(NULL);
    }
}

#else

static void ble_beacon_timeout_handler(tuya_ble_timer_t timer)
{
    tuya_ble_adv_change();
    // Adv Interval
    ty_ble_adv_param_t param = {
        .adv_interval_min = sg_default_adv_interval,
        .adv_interval_max = sg_default_adv_interval,
        .adv_type = 0x01,
    };
    ty_ble_set_adv_param(&param);

    if (sg_ble_beacon_cb != NULL) {
        sg_ble_beacon_cb(NULL);
    }
}

#endif

uint32_t tal_ble_beacon_dp_data_send(uint32_t sn, uint8_t encrypt_mode, uint16_t adv_interval, uint16_t adv_duration, uint8_t *p_dp_data, uint32_t dp_data_len)
{
    uint8_t adv_len = 0;
    uint8_t iv[12] = {0};
    uint8_t mic[4] = {0};
    uint8_t key_md5[16] = {0};
    uint8_t scan_rsp[TAL_BLE_BEACON_DATA_LEN_MAX] = {0x17, 0xFF, 0xD0, 0x07};
    uint8_t scan_rsp_length = 0;

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN >= 5)
    uint8_t adv_data[TAL_BLE_BEACON_DATA_LEN_MAX] = {0x02, 0x01, 0x06, 0x00, 0x16, 0x50, 0xFD};

    // Frame Control
    adv_data[7] = (TUYA_BLE_PROTOCOL_VERSION_HIGN      << PROTOCOL_VERSION_HIGN_OFFSET) | \
                  (TAL_BLE_BEACON_INCLUDE_ID           << INCLUDE_ID_OFFSET) | \
                  (TAL_BLE_BEACON_REQUEST_CONNECTION   << REQUEST_CONNECTION_OFFSET) | \
                  (TAL_BLE_BEACON_SHARE_FLAG           << SHARE_FLAG_OFFSET) | \
                  (TAL_BLE_BEACON_BOND_FLAG            << BOND_FLAG_OFFSET);
    adv_data[8] = (TAL_BLE_BEACON_SECURITY_V2_SUPPORT  << SECURITY_V2_SUPPORT_OFFSET) | \
                  (TAL_BLE_BEACON_SECURITY_V2_ENABLE   << SECURITY_V2_ENABLE_OFFSET) | \
                  (TAL_BLE_BEACON_GATEWAY_CONNECT_MODE << GATEWAY_CONNECT_MODE_OFFSET) | \
                  (TAL_BLE_BEACON_CONNECTIVITY         << CONNECTIVITY_OFFSET) | \
                  (TAL_BLE_BEACON_INCLUDE_DP_DATA      << INCLUDE_DP_DATA_OFFSET);
    adv_data[9] = (TAL_BLE_BEACON_INCLUDE_DP_DATA_V2   << INCLUDE_DP_DATA_V2_OFFSET) | \
                  (TAL_BLE_BEACON_ROAMING_FLAG         << ROAMING_FLAG_OFFSET) | \
                  (TAL_BLE_BEACON_ONLINE_STRATEGY      << ONLINE_STRATEGY_OFFSET);

    // Frame Counter
    adv_data[11] = (sn >> 24) & 0xFF;
    adv_data[12] = (sn >> 16) & 0xFF;
    adv_data[13] = (sn >>  8) & 0xFF;
    adv_data[14] = (sn >>  0) & 0xFF;

    // Dp Data
    if (encrypt_mode != 0) {
        if (dp_data_len > 12) {
            TUYA_BLE_LOG_ERROR("dp_data_len length error!!!");
            return -2; // OPRT_INVALID_PARM
        }

        iv[0] = adv_data[7];
        iv[1] = adv_data[8];
        iv[2] = adv_data[11];
        iv[3] = adv_data[12];
        iv[4] = adv_data[13];
        iv[5] = adv_data[14];
        memcpy(iv + 6, iv, 6);

        if (encrypt_mode == 1) {
            adv_data[10] = 0x20; //DP Control

            tuya_ble_md5_crypt(tuya_ble_current_para.sys_settings.login_key_v2, LOGIN_KEY_V2_LEN, key_md5);
        } else if (encrypt_mode == 2) {
            adv_data[10] = 0x40; //DP Control

            tuya_ble_md5_crypt(tuya_ble_current_para.sys_settings.beacon_key, BEACON_KEY_LEN, key_md5);
        }

        TUYA_BLE_LOG_HEXDUMP_INFO("login_key_v2", tuya_ble_current_para.sys_settings.login_key_v2, LOGIN_KEY_V2_LEN);
        TUYA_BLE_LOG_HEXDUMP_INFO("beacon_key", tuya_ble_current_para.sys_settings.beacon_key, LOGIN_KEY_V2_LEN);
        TUYA_BLE_LOG_HEXDUMP_INFO("key_md5", key_md5, 16);
        aes_ccm_encrypt_and_tag(key_md5,
                                iv, 12,
                                NULL, 0,
                                p_dp_data, dp_data_len,
                                adv_data + 15,
                                mic, 4);

        TUYA_BLE_LOG_HEXDUMP_INFO("mic", mic, 4);
        adv_data[15 + dp_data_len + 0] = mic[0];
        adv_data[15 + dp_data_len + 1] = mic[1];
        adv_data[15 + dp_data_len + 2] = mic[2];
        adv_data[15 + dp_data_len + 3] = mic[3];

        adv_len = 15 + dp_data_len + 4;
    } else {
        if (dp_data_len > 16) {
            TUYA_BLE_LOG_ERROR("dp_data_len length error!!!");
            return -2; // OPRT_INVALID_PARM
        }

        adv_data[10] = 0x00; //DP Control

        memcpy(&adv_data[15], p_dp_data, dp_data_len);

        adv_len = 15 + dp_data_len;
    }

    adv_data[3] = adv_len - 4;
#else
    uint8_t adv_data[31] = {0x02, 0x01, 0x06, 0x03, 0x02, 0x50, 0xFD, 0x00, 0x16, 0x50, 0xFD};

    // Frame Control
    adv_data[11] = (TUYA_BLE_PROTOCOL_VERSION_HIGN      << PROTOCOL_VERSION_HIGN_OFFSET) | \
                   (TAL_BLE_BEACON_INCLUDE_ID           << INCLUDE_ID_OFFSET) | \
                   (TAL_BLE_BEACON_REQUEST_CONNECTION   << REQUEST_CONNECTION_OFFSET) | \
                   (TAL_BLE_BEACON_SHARE_FLAG           << SHARE_FLAG_OFFSET) | \
                   (TAL_BLE_BEACON_BOND_FLAG            << BOND_FLAG_OFFSET);
    adv_data[12] = (TAL_BLE_BEACON_SECURITY_V2_SUPPORT  << SECURITY_V2_SUPPORT_OFFSET) | \
                   (TAL_BLE_BEACON_SECURITY_V2_ENABLE   << SECURITY_V2_ENABLE_OFFSET) | \
                   (TAL_BLE_BEACON_GATEWAY_CONNECT_MODE << GATEWAY_CONNECT_MODE_OFFSET) | \
                   (TAL_BLE_BEACON_CONNECTIVITY         << CONNECTIVITY_OFFSET) | \
                   (TAL_BLE_BEACON_INCLUDE_DP_DATA      << INCLUDE_DP_DATA_OFFSET);

    // Frame Counter
    adv_data[14] = (sn >> 24) & 0xFF;
    adv_data[15] = (sn >> 16) & 0xFF;
    adv_data[16] = (sn >>  8) & 0xFF;
    adv_data[17] = (sn >>  0) & 0xFF;

    // Dp Data
    if (encrypt_mode) {
        if (dp_data_len > 12) {
            TUYA_BLE_LOG_ERROR("dp_data_len length error!!!");
            return -2; // OPRT_INVALID_PARM
        }

        iv[0] = adv_data[11];
        iv[1] = adv_data[12];
        iv[2] = adv_data[14];
        iv[3] = adv_data[15];
        iv[4] = adv_data[16];
        iv[5] = adv_data[17];
        memcpy(iv + 6, iv, 6);

        if (encrypt_mode == 1) {
            adv_data[13] = 0x20; //DP Control

            tuya_ble_md5_crypt(tuya_ble_current_para.sys_settings.login_key_v2, LOGIN_KEY_V2_LEN, key_md5);
        } else if (encrypt_mode == 2) {
            adv_data[13] = 0x40; //DP Control

            tuya_ble_md5_crypt(tuya_ble_current_para.sys_settings.beacon_key, BEACON_KEY_LEN, key_md5);
        }

        TUYA_BLE_LOG_HEXDUMP_INFO("login_key_v2", tuya_ble_current_para.sys_settings.login_key_v2, LOGIN_KEY_V2_LEN);
        TUYA_BLE_LOG_HEXDUMP_INFO("beacon_key", tuya_ble_current_para.sys_settings.beacon_key, LOGIN_KEY_V2_LEN);
        TUYA_BLE_LOG_HEXDUMP_INFO("key_md5", key_md5, 16);

        aes_ccm_encrypt_and_tag(key_md5,
                                iv, 12,
                                NULL, 0,
                                p_dp_data, dp_data_len,
                                adv_data + 18,
                                mic, 4);

        adv_data[18 + dp_data_len + 0] = mic[0];
        adv_data[18 + dp_data_len + 1] = mic[1];
        adv_data[18 + dp_data_len + 2] = mic[2];
        adv_data[18 + dp_data_len + 3] = mic[3];

        adv_len = 18 + dp_data_len + 4;
    } else {
        if (dp_data_len > 16) {
            TUYA_BLE_LOG_ERROR("dp_data_len length error!!!");
            return -2; // OPRT_INVALID_PARM
        }

        adv_data[13] = 0x00; //DP Control

        memcpy(&adv_data[18], p_dp_data, dp_data_len);

        adv_len = 18 + dp_data_len;
    }

    adv_data[7] = adv_len - 8;
#endif

    scan_rsp[4] = tuya_ble_secure_connection_type();
    scan_rsp[5] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY >> 8;
    scan_rsp[6] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY & 0xFF;

    if (tuya_ble_current_para.pid_len == 20) {
        scan_rsp[7] |= 0x01 ;
    } else {
        scan_rsp[7] &= (~0x01);
    }

    tuya_ble_device_id_encrypt_v4(&tuya_ble_current_para, adv_data, scan_rsp, TUYA_BLE_PROTOCOL_VERSION_HIGN);

    if (tuya_ble_current_para.adv_local_name_len > 0) {
        scan_rsp[24] = tuya_ble_current_para.adv_local_name_len + 1;
        scan_rsp[25] = 0x09;
        memcpy(&scan_rsp[26], tuya_ble_current_para.adv_local_name, tuya_ble_current_para.adv_local_name_len);
    }

    scan_rsp_length = ((tuya_ble_current_para.adv_local_name_len == 0) ? (-2) : (tuya_ble_current_para.adv_local_name_len) + 26);



    TUYA_BLE_LOG_HEXDUMP_INFO("adv_data", adv_data, adv_len);

    tuya_ble_gap_advertising_adv_data_update(adv_data, adv_len);
    tuya_ble_gap_advertising_scan_rsp_data_update(scan_rsp, scan_rsp_length);

#if defined(SUPPORT_TUYAOS) && (SUPPORT_TUYAOS == 1)
    // Adv Interval
    TAL_BLE_ADV_PARAMS_T adv_param = {
        .adv_interval_min = adv_interval * 8 / 5,
        .adv_interval_max = adv_interval * 8 / 5,
        .adv_type = TAL_BLE_ADV_TYPE_CS_UNDIR,
    };
    tal_ble_advertising_start(&adv_param);

    // Adv Duration
    tal_sw_timer_start(sg_ble_beacon_timer_id, adv_duration, TAL_TIMER_ONCE);
#else
    // Adv Interval
    ty_ble_adv_param_t param = {
        .adv_interval_min = adv_interval,
        .adv_interval_max = adv_interval,
        .adv_type = 0x01,
    };
    ty_ble_set_adv_param(&param);

    // Adv Duration
    tuya_ble_timer_start(sg_ble_beacon_timer_id);
#endif

    return 0;
}

