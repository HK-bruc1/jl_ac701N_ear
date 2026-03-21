/**
 * @file tuya_ble_data_handler.h
 * @brief This is tuya_ble_data_handler file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_DATA_HANDLER_H__
#define __TUYA_BLE_DATA_HANDLER_H__

#include "tuya_cloud_types.h"
#include "tuya_ble_internal_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define FRM_QRY_DEV_INFO_REQ                        0x0000  //APP->BLE
#define FRM_QRY_DEV_INFO_RESP                       0x0000  //BLE->APP
#define PAIR_REQ                                    0x0001  //APP->BLE
#define PAIR_RESP                                   0x0001  //BLE->APP
#define FRM_CMD_SEND                                0x0002  //APP->BLE
#define FRM_CMD_RESP                                0x0002  //BLE->APP
#define FRM_STATE_QUERY                             0x0003  //APP->BLE
#define FRM_STATE_QUERY_RESP                        0x0003  //BLE->APP
#define FRM_LOGIN_KEY_REQ                           0x0004  //APP->BLE
#define FRM_LOGIN_KEY_RESP                          0x0004  //BLE->APP
#define FRM_UNBONDING_REQ                           0x0005  //APP->BLE
#define FRM_UNBONDING_RESP                          0x0005  //BLE->APP
#define FRM_DEVICE_RESET                            0x0006  //APP->BLE
#define FRM_DEVICE_RESET_RESP                       0x0006  //BLE->APP

#define FRM_OTA_START_REQ                           0x000C //APP->BLE
#define FRM_OTA_START_RESP                          0x000C //BLE->APP
#define FRM_OTA_FILE_INFOR_REQ                      0x000D //APP->BLE
#define FRM_OTA_FILE_INFOR_RESP                     0x000D //BLE->APP
#define FRM_OTA_FILE_OFFSET_REQ                     0x000E //APP->BLE
#define FRM_OTA_FILE_OFFSET_RESP                    0x000E //BLE->APP
#define FRM_OTA_DATA_REQ                            0x000F //APP->BLE
#define FRM_OTA_DATA_RESP                           0x000F //BLE->APP
#define FRM_OTA_END_REQ                             0x0010 //APP->BLE
#define FRM_OTA_END_RESP                            0x0010 //BLE->APP
#define FRM_OTA_PREPARE_NOTIFICATION_REQ            0x0011 //APP->BLE
#define FRM_OTA_PREPARE_NOTIFICATION_RESP           0x0011 //BLE->APP

#define FRM_FACTORY_TEST_CMD                        0x0012 //APP->BLE
#define FRM_FACTORY_TEST_RESP                       0x0012 //BLE->APP

#define FRM_ANOMALY_UNBONDING_REQ                   0x0014 //APP->BLE
#define FRM_ANOMALY_UNBONDING_RESP                  0x0014 //BLE->APP

#define FRM_AUTHENTICATE_PHASE_1_REQ                0x0015 //APP->BLE
#define FRM_AUTHENTICATE_PHASE_1_RESP               0x0015 //BLE->APP

#define FRM_AUTHENTICATE_PHASE_2_REQ                0x0016 //APP->BLE
#define FRM_AUTHENTICATE_PHASE_2_RESP               0x0016 //BLE->APP

#define FRM_AUTHENTICATE_PHASE_3_REQ                0x0017 //APP->BLE
#define FRM_AUTHENTICATE_PHASE_3_RESP               0x0017 //BLE->APP

#if defined(TUYA_BLE_OTA_SIGNATURE_ENABLE) && (TUYA_BLE_OTA_SIGNATURE_ENABLE == 1)
#define FRM_OTA_SIGNATURE_DATA_REQ                  0x0018 //APP->BLE
#define FRM_OTA_SIGNATURE_DATA_RESP                 0x0018 //BLE->APP

#define FRM_OTA_SIGNATURE_KEY_UPDATE_REQ            0x0019 //APP->BLE
#define FRM_OTA_SIGNATURE_KEY_UPDATE_RESP           0x0019 //BLE->APP
#endif

#define FRM_NET_CONFIG_INFO_REQ                     0x0021 //APP->BLE
#define FRM_NET_CONFIG_INFO_RESP                    0x0021 //BLE->APP

#define FRM_NET_CONFIG_RESPONSE_REPORT_REQ          0x0022 //APP->BLE
#define FRM_NET_CONFIG_RESPONSE_REPORT_RESP         0x0022 //BLE->APP

#define FRM_DATA_PASSTHROUGH_REQ                    0x0023 //APP<->BLE

#define FRM_DP_DATA_WRITE_REQ                       0x0027     //APP->BLE
#define FRM_DP_DATA_WRITE_RESP                      0x0027     //BLE->APP

#define FRM_DP_DATA_WITH_SRC_TYPE_WRITE_REQ         0x0028  //APP->BLE
#define FRM_DP_DATA_WITH_SRC_TYPE_WRITE_RESP        0x0028  //BLE->APP

#define FRM_WITH_SRC_TYPE_DP_DATA_QUERY             0x0029  //APP->BLE
#define FRM_WITH_SRC_TYPE_DP_DATA_QUERY_RESP        0x0029  //BLE->APP

#define FRM_EM_DEV_INFO_QUERY_REQ                   0x0050     //APP->BLE
#define FRM_EM_DEV_INFO_QUERY_RESP                  0x0050     //BLE->APP
#define FRM_EM_ACTIVE_INFO_RECEIVED                 0x0051     //APP->BLE
#define FRM_EM_ACTIVE_INFO_RECEIVED_RESP            0x0051     //BLE->APP

#if TUYA_BLE_BR_EDR_SUPPORTED
#define TUYA_BLE_FRM_BR_EDR_DATA_INFO_QUREY         0x0060  //APP->BLE
#define TUYA_BLE_FRM_BR_EDR_DATA_INFO_RESP          0x0060  //BLE->APP
#endif

#if defined(TUYA_BLE_FEATURE_SPEECH_ENABLE) && (TUYA_BLE_FEATURE_SPEECH_ENABLE == 1)
#define FRM_SPEECH_CONTROL_REQ                      0x0030 //APP->BLE
#define FRM_SPEECH_CONTROL_RESP                     0x0030
#define FRM_SPEECH_RAW_DATA_REPORT_REQ              0x0031 //APP<-BLE
#define FRM_SPEECH_RAW_DATA_REPORT_RSP              0x0031
#define FRM_SPEECH_RESULT_WRITE_REQ                 0x0032 //APP->BLE
#define FRM_SPEECH_RESULT_WRITE_RESP                0x0032
#define FRM_SPEECH_CLOCK_SETTING_REQ                0x0033 //APP->BLE
#define FRM_SPEECH_CLOCK_SETTING_RESP               0x0033
#define FRM_SPEECH_TOKEN_READ_REQ                   0x0034 //APP->BLE
#define FRM_SPEECH_TOKEN_READ_RESP                  0x0034
#define FRM_SPEECH_TOKEN_REPORT_REQ                 0x0035 //APP<-BLE
#define FRM_SPEECH_TOKEN_REPORT_RESP                0x0035
#define FRM_SPEECH_TOKEN_WRITE_REQ                  0x0036 //APP->BLE
#define FRM_SPEECH_TOKEN_WRITE_RESP                 0x0036
#define FRM_SPEECH_COMMON_SETTING_REQ               0x0037 //APP->BLE
#define FRM_SPEECH_COMMON_SETTING_RESP              0x0037
#endif

#if defined(TUYA_BLE_FEATURE_GPT_ENABLE) && (TUYA_BLE_FEATURE_GPT_ENABLE == 1)
#define FRM_GPT_CONTROL_REQ                         0x0090 //APP->BLE
#define FRM_GPT_CONTROL_RESP                        0x0090
#define FRM_GPT_RAW_DATA_REPORT_REQ                 0x0091 //APP<-BLE
#define FRM_GPT_RAW_DATA_REPORT_RESP                0x0091
#define FRM_GPT_RESULT_WRITE_REQ                    0x0092 //APP->BLE
#define FRM_GPT_RESULT_WRITE_RESP                   0x0092
#define FRM_GPT_RAW_DATA_WRITE_REQ                  0x0093 //APP->BLE
#define FRM_GPT_RAW_DATA_WRITE_RESP                 0x0093
#endif

#define FRM_STAT_REPORT                             0x8001  //BLE->APP
#define FRM_STAT_REPORT_RESP                        0x8001  //APP->BLE

#define FRM_STAT_WITH_TIME_REPORT                   0x8003  //BLE->APP
#define FRM_STAT_WITH_TIME_REPORT_RESP              0x8003  //APP->BLE

#define FRM_DATA_WITH_FLAG_REPORT                   0x8004  //BLE->APP
#define FRM_DATA_WITH_FLAG_REPORT_RESP              0x8004  //APP->BLE

#define FRM_DATA_WITH_FLAG_AND_TIME_REPORT          0x8005  //BLE->APP
#define FRM_DATA_WITH_FLAG_AND_TIME_REPORT_RESP     0x8005  //APP->BLE

#define FRM_DP_DATA_SEND_REQ                        0x8006  //BLE->APP
#define FRM_DP_DATA_SEND_RESP                       0x8006  //APP->BLE

#define FRM_DP_DATA_WITH_TIME_SEND_REQ              0x8007  //BLE->APP
#define FRM_DP_DATA_WITH_TIME_SEND_RESP             0x8007  //APP->BLE

#define FRM_DP_DATA_WITH_SRC_TYPE_SEND_REQ          0x8008  //BLE->APP
#define FRM_DP_DATA_WITH_SRC_TYPE_SEND_RESP         0x8008  //APP->BLE

#define FRM_DP_DATA_WITH_SRC_TYPE_AND_TIME_SEND_REQ     0x8009 //BLE->APP
#define FRM_DP_DATA_WITH_SRC_TYPE_AND_TIME_SEND_RESP    0x8009 //APP->BLE

#define FRM_GET_UNIX_TIME_MS_REQ                    0x8010  //BLE->APP
#define FRM_GET_UNIX_TIME_MS_RESP                   0x8010  //APP->BLE
#define FRM_GET_UNIX_TIME_CHAR_MS_REQ               0x8011  //BLE->APP
#define FRM_GET_UNIX_TIME_CHAR_MS_RESP              0x8011  //APP->BLE
#define FRM_GET_UNIX_TIME_CHAR_DATE_REQ             0x8012  //BLE->APP
#define FRM_GET_UNIX_TIME_CHAR_DATE_RESP            0x8012  //APP->BLE

#define FRM_GET_APP_LOCAL_TIME_REQ                     0x8013  //BLE->APP
#define FRM_GET_APP_LOCAL_TIME_RESP                    0x8013  //APP->BLE
#define FRM_GET_UNIX_TIME_WITH_DST_REQ              0x8014  //BLE->APP [timestamp + timezone + daylight saving time]
#define FRM_GET_UNIX_TIME_WITH_DST_RESP             0x8014  //APP->BLE [timestamp + timezone + daylight saving time]

#define FRM_WEATHER_DATA_REQUEST                       0x8017  //BLE->APP
#define FRM_WEATHER_DATA_REQUEST_RESP                 0x8017  //APP->BLE

#define FRM_WEATHER_DATA_RECEIVED                     0x8018  //APP->BLE
#define FRM_WEATHER_DATA_RECEIVED_RESP                 0x8018  //BLE->APP

#define FRM_IOT_DATA_REQUEST                              0x8019  //BLE->APP
#define FRM_IOT_DATA_REQUEST_RESP                        0x8019  //APP->BLE

#define FRM_IOT_DATA_RECEIVED                         0x801A  //APP->BLE
#define FRM_IOT_DATA_RECEIVED_RESP                     0x801A  //BLE->APP

#if (TUYA_BLE_FEATURE_APP_PASSTHROUGH_ENABLE != 0)
#define FRM_APP_DOWNSTREAM_PASSTHROUGH_REQ             0x801B  //APP->BLE
#define FRM_APP_DOWNSTREAM_PASSTHROUGH_RESP         0x801B  //BLE->APP

#define FRM_APP_UPSTREAM_PASSTHROUGH_REQ             0x801C  //BLE->APP
#define FRM_APP_UPSTREAM_PASSTHROUGH_RESP             0x801C  //APP->BLE
#endif

#define FRM_REMOTER_PROXY_AUTH_REQ                  0x8030  //BLE->APP, refer to FRM_GET_APP_LOCAL_TIME_REQ
#define FRM_REMOTER_PROXY_AUTH_RESP                 0x8030  //APP->BLE
#define FRM_REMOTER_GROUP_SET_REQ                   0x8031  //APP->BLE, refer to FRM_ANOMALY_UNBONDING_REQ
#define FRM_REMOTER_GROUP_SET_RESP                  0x8031  //BLE->APP
#define FRM_REMOTER_GROUP_DELETE_REQ                0x8032  //APP->BLE
#define FRM_REMOTER_GROUP_DELETE_RESP               0x8032  //BLE->APP
#define FRM_REMOTER_GROUP_GET_REQ                   0x8033  //APP->BLE
#define FRM_REMOTER_GROUP_GET_RESP                  0x8033  //BLE->APP

typedef enum {
    TUYA_BLE_OTA_STATUS_NONE,
    TUYA_BLE_OTA_STATUS_START,
    TUYA_BLE_OTA_STATUS_FILE_INFO,
    TUYA_BLE_OTA_STATUS_FILE_OFFSET,
    TUYA_BLE_OTA_STATUS_FILE_DATA,
    TUYA_BLE_OTA_STATUS_FILE_END,
    TUYA_BLE_OTA_STATUS_MAX,
} tuya_ble_ota_status_t;

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
typedef struct {
    UINT32_T send_len;
    UINT8_T  *send_data;
    UINT32_T encrypt_data_buf_len;
    UINT8_T  *encrypt_data_buf;
} tuya_ble_r_air_send_packet;

typedef struct {
    UINT32_T recv_len;
    UINT32_T recv_len_max;
    UINT8_T  *recv_data;
    UINT32_T decrypt_buf_len;
    UINT8_T  *de_encrypt_buf;
} tuya_ble_r_air_recv_packet;

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/

/**
 * @brief tuya_ble_air_recv_packet_free
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_air_recv_packet_free(VOID_T);

/**
 * @brief tuya_ble_set_device_version
 *
 * @param[in] firmware_version: firmware_version
 * @param[in] hardware_version: hardware_version
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_set_device_version(UINT32_T firmware_version, UINT32_T hardware_version);

/**
 * @brief tuya_ble_set_external_mcu_version
 *
 * @param[in] firmware_version: firmware_version
 * @param[in] hardware_version: hardware_version
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_set_external_mcu_version(UINT32_T firmware_version, UINT32_T hardware_version);

#if defined(TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE) && (TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE == 1)

/**
 * @brief tuya_ble_set_attach_version
 *
 * @param[in] buf: buf
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_set_attach_version(UINT8_T *buf, UINT32_T size);

#endif

/**
 * @brief tuya_ble_comm_data_send
 *
 * @param[in] cmd: cmd
 * @param[in] ack_sn: ack_sn
 * @param[in] *data: *data
 * @param[in] len: len
 * @param[in] encryption_mode: encryption_mode
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tuya_ble_comm_data_send(UINT16_T cmd, UINT32_T ack_sn, UINT8_T *data, UINT16_T len, UINT8_T encryption_mode);

/**
 * @brief tuya_ble_evt_process
 *
 * @param[in] cmd: cmd
 * @param[in] UINT8_T*recv_data: UINT8_T*recv_data
 * @param[in] recv_len: recv_len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_evt_process(UINT16_T cmd, UINT8_T *recv_data, UINT32_T recv_len);

/**
 * @brief tuya_ble_reset_ble_sn
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_reset_ble_sn(VOID_T);

/**
 * @brief tuya_ble_common_data_rx_proc
 *
 * @param[in] *buf: *buf
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_common_data_rx_proc(UINT8_T *buf, UINT16_T len);

/**
 * @brief tuya_ble_pair_rand_clear
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_pair_rand_clear(VOID_T);

/**
 * @brief tuya_ble_pair_rand_valid_get
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tuya_ble_pair_rand_valid_get(VOID_T);

/**
 * @brief tuya_ble_disconnect_timer_init
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_disconnect_timer_init(VOID_T);

/**
 * @brief tuya_ble_send_packet_data_length_get
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tuya_ble_send_packet_data_length_get(VOID_T);

/**
 * @brief tuya_ble_handle_accessory_info_report_evt
 *
 * @param[in] *evt: *evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_accessory_info_report_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief tuya_ble_device_unbond
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_device_unbond(VOID_T);

#if (TUYA_BLE_SECURE_CONNECTION_TYPE==TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)

/**
 * @brief tuya_ble_auth_data_reset
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_auth_data_reset(VOID_T);

#endif

#if TUYA_BLE_BR_EDR_SUPPORTED

/**
 * @brief tuya_ble_br_edr_data_info_update_internal
 *
 * @param[in] *p_data: *p_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_br_edr_data_info_update_internal(tuya_ble_br_edr_data_info_t *p_data);

#endif


#ifdef __cplusplus
}
#endif

#endif /* __TUYA_BLE_DATA_HANDLER_H__ */

