/**
 * @file tal_sdk_test.h
 * @brief This is tal_sdk_test file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TAL_SDK_TEST_H__
#define __TAL_SDK_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_cloud_types.h"
#if defined(TUYA_SDK_TEST) && (TUYA_SDK_TEST == 1)

#if (1 == TUYA_SDK_TEST_TYPE)
#include "tuya_ble_type.h"
#include "tal_bluetooth_def.h"
#include "tal_sw_timer.h"
#elif (4 == TUYA_SDK_TEST_TYPE)
#include "tuya_ble_type.h"
#include "tal_bluetooth_def.h"
#include "tal_bluetooth_bredr.h"
#endif
#include "tal_utc.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
//GID - Group ID, CID - Command ID
#define TEST_GID_SYSTEM                 0x01
#define TEST_GID_DEVICE_INFO            0x02
#define TEST_GID_ADV                    0x03
#define TEST_GID_SCAN                   0x04
#define TEST_GID_CONN                   0x05
#define TEST_GID_DATA                   0x06
#define TEST_GID_ELSE                   0x07
#define TEST_GID_GPIO                   0x10
#define TEST_GID_UART                   0x11
#define TEST_GID_PWM                    0x12
#define TEST_GID_ADC                    0x13
#define TEST_GID_SPI                    0x14
#define TEST_GID_IIC                    0x15
#define TEST_GID_RTC                    0x16
#define TEST_GID_FLASH                  0x17
#define TEST_GID_WATCHDOG               0x18
#define TEST_GID_POWERMANGER            0x19

#define TEST_GID_CUSTOM                 0x30

#if (4 == TUYA_SDK_TEST_TYPE)
#define TEST_GID_BREDR_SYSTEM           0x20
#define TEST_GID_BREDR_CONNECT          0x21
#define TEST_GID_BREDR_AVTCP            0x22
#define TEST_GID_BREDR_HFP              0x23
#define TEST_GID_BREDR_EQ               0x24
#endif

// TEST_GID_SYSTEM
#define TEST_CID_SHAKE_HAND             0x00
#define TEST_CID_RESET                  0x01
#define TEST_CID_FACTORY_RESET          0x02
#define TEST_CID_GET_TIME               0x03
#define TEST_CID_REQ_TIME               0x04
#define TEST_CID_GET_RAND               0x05
#define TEST_CID_GET_QUEUE_SIZE         0x06
#define TEST_CID_POWER_ON               0x80

// TEST_GID_DEVICE_INFO
#define TEST_CID_SET_MAC                0x00
#define TEST_CID_GET_MAC                0x01
#define TEST_CID_SET_PID                0x02
#define TEST_CID_GET_PID                0x03
#define TEST_CID_SET_VERSION            0x04
#define TEST_CID_GET_VERSION            0x05

// TEST_GID_ADV
#define TEST_CID_ADV_ENABLE             0x00
#define TEST_CID_SET_ADV_INTERVAL       0x01
#define TEST_CID_SET_ADV_PARAM          0x02
#define TEST_CID_SET_ADV_DATA           0x03
#define TEST_CID_CONN_REQUEST           0x04
#define TEST_CID_MESH_FAST_PROV         0x05
#define TEST_CID_SET_EXT_ADV_PARAM      0x06

// TEST_GID_SCAN
#define TEST_CID_SCAN_START             0x00
#define TEST_CID_SCAN_STOP              0x01
#define TEST_CID_ADV_REPORT             0x80

// TEST_GID_CONN
#define TEST_CID_CONN                   0x00
#define TEST_CID_SET_CONN_INTERVAL      0x01
#define TEST_CID_SET_CONN_PARAM         0x02
#define TEST_CID_DISCONN                0x03
#define TEST_CID_NET_STATE              0x04
#define TEST_CID_GET_MESH_ADDR          0x05
#define TEST_CID_UNBIND_MODE            0x80
#define TEST_CID_CONN_PARAM_UPDATE      0x81
#define TEST_CID_MTU_UPDATE             0x82

// TEST_GID_DATA
#define TEST_CID_MASTER_SEND            0x00
#define TEST_CID_SLAVE_SEND             0x01
#define TEST_CID_DP_REPORT              0x02
#define TEST_CID_LONG_DP_REPORT         0x03
#define TEST_CID_DP_REPORT_TIME         0x04
#define TEST_CID_LONG_DP_REPORT_TIME    0x05
#define TEST_CID_DP_PASSTHROUGH         0x06
#define TEST_CID_MESH_DP_REPORT         0x07
#define TEST_CID_MESH_DP_REPORT_RSP     0x08
#define TEST_CID_ROAMING_DP_REPORT      0x09
#define TEST_CID_DP_REPORT_RSP          0x80
#define TEST_CID_DP_REPORT_TIME_RSP     0x81
#define TEST_CID_DP_WRITE               0x82
#define TEST_CID_LONG_DP_WRITE          0x83
#define TEST_CID_MESH_DP_WRITE          0x84

// TEST_GID_ELSE
#define TEST_CID_GET_WEATHER            0x00
#define TEST_CID_SET_BULK_DATA          0x01
#define TEST_CID_GET_SCENE_LIST         0x02
#define TEST_CID_REQ_SCENE_CTRL         0x03
#define TEST_CID_SET_ATTACH_OTA_VERSION 0x04
#define TEST_CID_SEND_BLE_GROUP_DATA    0x05
#define TEST_CID_GET_WEATHER_RSP        0x80
#define TEST_CID_GET_WEATHER_DATA       0x81
#define TEST_CID_GET_REMOTER_DATA       0x82
#define TEST_CID_GET_LOCAL_TIMER        0x83
#define TEST_CID_GET_LOCAL_TIMER_RSP    0x84
#define TEST_CID_GET_LOCAL_TIMER_PARSER 0x85
#define TEST_CID_GET_SCENE_RSP          0x86

// TEST_GID_GPIO
#define TEST_CID_PIN_DEINIT             0x00
#define TEST_CID_OUTPUT_HIGH            0x01
#define TEST_CID_OUTPUT_LOW             0x02
#define TEST_CID_PIN_READ               0x03

// TEST_GID_UART
#define TEST_CID_SET_BAUDRATE           0x00
#define TEST_CID_TX_UART_DATA           0x01
#define TEST_CID_RX_UART_PORT           0x80
#define TEST_CID_RX_UART_DATA           0x81

// TEST_GID_PWM
#define TEST_CID_PWM_DEINIT             0x00
#define TEST_CID_SET_FREQ_DUTY          0x01

// TEST_GID_ADC
#define TEST_CID_ADC_DEINIT             0x00
#define TEST_CID_READ_ADC_DATA          0x01
#define TEST_CID_READ_ADC_DATA_RSP      0x80
#define TEST_CID_READ_VOLTAGE_RSP       0x81

// TEST_GID_SPI
#define TEST_CID_TX_SPI_DATA            0x00
#define TEST_CID_RX_SPI_DATA            0x80

// TEST_GID_IIC
#define TEST_CID_TX_IIC_DATA            0x00
#define TEST_CID_RX_IIC_DATA            0x80

// TEST_GID_RTC
#define TEST_CID_SET_RTC_TIME           0x00
#define TEST_CID_GET_RTC_TIME           0x01
#define TEST_CID_START_RTC              0x02
#define TEST_CID_STOP_RTC               0x03

// TEST_GID_FLASH
#define TEST_CID_READ_FLASH_DATA        0x00
#define TEST_CID_ERASE_FLASH_DATA       0x01
#define TEST_CID_WRITE_FLASH_DATA       0x02

// TEST_GID_WATCHDOG
#define TEST_CID_START_WDG              0x00
#define TEST_CID_FEED_WDG               0x01
#define TEST_CID_STOP_WDG               0x02

// TEST_GID_POWERMANGER
#define TEST_CID_ENTER_SLEEP            0x00
#define TEST_CID_WAKEUP_SRC_SET         0x01

// TEST_GID_CUSTOM
#define TEST_CID_REPEATER               0x00

#if (4 == TUYA_SDK_TEST_TYPE)
// TEST_GID_BREDR_SYSTEM
#define TEST_CID_BREDR_SET_MAC          0x00
#define TEST_CID_BREDR_GET_MAC          0x01
#define TEST_CID_BREDR_SET_NAME         0x02
#define TEST_CID_BREDR_GET_NAME         0x03
#define TEST_CID_BREDR_GET_HOST_MAC     0x04
#define TEST_CID_BREDR_GET_HOST_NAME    0x05

// TEST_GID_BREDR_CONNECT
#define TEST_CID_BREDR_GET_STATUS       0x00
#define TEST_CID_BREDR_PAIR_MODE        0x01
#define TEST_CID_BREDR_PAIR_KEY         0x02
#define TEST_CID_BREDR_SCAN             0x03
#define TEST_CID_BREDR_LINK             0x04
#define TEST_CID_BREDR_DISCONNECT       0x05
#define TEST_CID_BREDR_UNBOND           0x06

// TEST_GID_BREDR_AVTCP
#define TEST_CID_BREDR_AVTCP_EN         0x00
#define TEST_CID_BREDR_AVTCP_CTL        0x01

// TEST_GID_BREDR_HFP
#define TEST_CID_BREDR_HFP_EN           0x00
#define TEST_CID_BREDR_HFP_INCOMING     0x01
#define TEST_CID_BREDR_HFP_OUTGOING     0x02
#define TEST_CID_BREDR_HFP_ACTIVE       0x03
#define TEST_CID_BREDR_HFP_HANDUP       0x04
#define TEST_CID_BREDR_HFP_VOLUME       0x05
#define TEST_CID_BREDR_HFP_BATTERY      0x06
#define TEST_CID_BREDR_HFP_REJECT       0x07

// TEST_GID_BREDR_EQ

#endif

#define TEST_ID_GET(GID, CID)           ((GID << 8) | CID)

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/

#if (1 == TUYA_SDK_TEST_TYPE)

/**
 * @brief tal_sdk_test_ble_evt_callback
 *
 * @param[in] *p_event: *p_event
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tal_sdk_test_ble_evt_callback(TAL_BLE_EVT_PARAMS_T *p_event);

/**
 * @brief tal_sdk_test_ble_protocol_callback
 *
 * @param[in] event: event
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tal_sdk_test_ble_protocol_callback(tuya_ble_cb_evt_param_t *event);

#elif (2 == TUYA_SDK_TEST_TYPE)

/**
 * @brief tal_sdk_test_mesh_data_write
 *
 * @param[in] dp_id: dp_id
 * @param[in] dp_type: dp_type
 * @param[in] *dp_data: *dp_data
 * @param[in] dp_len: dp_len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tal_sdk_test_mesh_data_write(UINT8_T dp_id, UINT8_T dp_type, UINT8_T *dp_data, UINT16_T dp_len);

#elif (4 == TUYA_SDK_TEST_TYPE)
/**
 * @brief tal_sdk_test_ble_evt_callback
 *
 * @param[in] *p_event: *p_event
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tal_sdk_test_ble_evt_callback(TAL_BLE_EVT_PARAMS_T *p_event);

/**
 * @brief tal_sdk_test_ble_protocol_callback
 *
 * @param[in] event: event
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tal_sdk_test_ble_protocol_callback(tuya_ble_cb_evt_param_t *event);

/**
 * @brief tal_sdk_test_bredr_evt_callback
 *
 * @param[in] event: event
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tal_sdk_test_bredr_evt_callback(TUYA_BT_BREDR_EVENT_T *p_event);

#endif

/**
 * @brief tal_sdk_test_get_time_rsp
 *
 * @param[in] *date: *date
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tal_sdk_test_get_time_rsp(tal_utc_date_t *date);

/**
 * @brief tal_sdk_test_init
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tal_sdk_test_init(VOID_T);

/**
 * @brief test_group_system
 *
 * @param[in] cmd: cmd
 * @param[in] *cmd_data: *cmd_data
 * @param[in] cmd_data_len: cmd_data_len
 * @param[in] *p_rsp_data: *p_rsp_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_group_system(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data);

/**
 * @brief test_group_device_info
 *
 * @param[in] cmd: cmd
 * @param[in] *cmd_data: *cmd_data
 * @param[in] cmd_data_len: cmd_data_len
 * @param[in] *p_rsp_data: *p_rsp_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_group_device_info(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data);

/**
 * @brief test_group_adv
 *
 * @param[in] cmd: cmd
 * @param[in] *cmd_data: *cmd_data
 * @param[in] cmd_data_len: cmd_data_len
 * @param[in] *p_rsp_data: *p_rsp_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_group_adv(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data);

/**
 * @brief test_group_scan
 *
 * @param[in] cmd: cmd
 * @param[in] *cmd_data: *cmd_data
 * @param[in] cmd_data_len: cmd_data_len
 * @param[in] *p_rsp_data: *p_rsp_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_group_scan(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data);

/**
 * @brief test_group_conn
 *
 * @param[in] cmd: cmd
 * @param[in] *cmd_data: *cmd_data
 * @param[in] cmd_data_len: cmd_data_len
 * @param[in] *p_rsp_data: *p_rsp_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_group_conn(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data);

/**
 * @brief test_group_data
 *
 * @param[in] cmd: cmd
 * @param[in] *cmd_data: *cmd_data
 * @param[in] cmd_data_len: cmd_data_len
 * @param[in] *p_rsp_data: *p_rsp_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_group_data(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data);

/**
 * @brief test_group_else
 *
 * @param[in] cmd: cmd
 * @param[in] *cmd_data: *cmd_data
 * @param[in] cmd_data_len: cmd_data_len
 * @param[in] *p_rsp_data: *p_rsp_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_group_else(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data);

/**
 * @brief test_group_gpio
 *
 * @param[in] cmd: cmd
 * @param[in] *cmd_data: *cmd_data
 * @param[in] cmd_data_len: cmd_data_len
 * @param[in] *p_rsp_data: *p_rsp_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_group_gpio(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data);

/**
 * @brief test_group_uart
 *
 * @param[in] cmd: cmd
 * @param[in] *cmd_data: *cmd_data
 * @param[in] cmd_data_len: cmd_data_len
 * @param[in] *p_rsp_data: *p_rsp_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_group_uart(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data);

/**
 * @brief test_group_pwm
 *
 * @param[in] cmd: cmd
 * @param[in] *cmd_data: *cmd_data
 * @param[in] cmd_data_len: cmd_data_len
 * @param[in] *p_rsp_data: *p_rsp_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_group_pwm(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data);

/**
 * @brief test_group_adc
 *
 * @param[in] cmd: cmd
 * @param[in] *cmd_data: *cmd_data
 * @param[in] cmd_data_len: cmd_data_len
 * @param[in] *p_rsp_data: *p_rsp_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_group_adc(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data);

/**
 * @brief test_group_spi
 *
 * @param[in] cmd: cmd
 * @param[in] *cmd_data: *cmd_data
 * @param[in] cmd_data_len: cmd_data_len
 * @param[in] *p_rsp_data: *p_rsp_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_group_spi(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data);

/**
 * @brief test_group_iic
 *
 * @param[in] cmd: cmd
 * @param[in] *cmd_data: *cmd_data
 * @param[in] cmd_data_len: cmd_data_len
 * @param[in] *p_rsp_data: *p_rsp_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_group_iic(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data);

/**
 * @brief test_group_rtc
 *
 * @param[in] cmd: cmd
 * @param[in] *cmd_data: *cmd_data
 * @param[in] cmd_data_len: cmd_data_len
 * @param[in] *p_rsp_data: *p_rsp_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_group_rtc(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data);

/**
 * @brief test_group_flash
 *
 * @param[in] cmd: cmd
 * @param[in] *cmd_data: *cmd_data
 * @param[in] cmd_data_len: cmd_data_len
 * @param[in] *p_rsp_data: *p_rsp_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_group_flash(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data);

/**
 * @brief test_group_watchdog
 *
 * @param[in] cmd: cmd
 * @param[in] *cmd_data: *cmd_data
 * @param[in] cmd_data_len: cmd_data_len
 * @param[in] *p_rsp_data: *p_rsp_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_group_watchdog(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data);

/**
 * @brief test_group_powermanger
 *
 * @param[in] cmd: cmd
 * @param[in] *cmd_data: *cmd_data
 * @param[in] cmd_data_len: cmd_data_len
 * @param[in] *p_rsp_data: *p_rsp_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_group_powermanger(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data);

#if (4 == TUYA_SDK_TEST_TYPE)

/**
 * @brief test_group_br_edr_system
 *
 * @param[in] cmd: cmd
 * @param[in] *cmd_data: *cmd_data
 * @param[in] cmd_data_len: cmd_data_len
 * @param[in] *p_rsp_data: *p_rsp_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_group_br_edr_system(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data);

/**
 * @brief test_group_br_edr_connect
 *
 * @param[in] cmd: cmd
 * @param[in] *cmd_data: *cmd_data
 * @param[in] cmd_data_len: cmd_data_len
 * @param[in] *p_rsp_data: *p_rsp_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_group_br_edr_connect(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data);

/**
 * @brief test_group_br_edr_avtcp
 *
 * @param[in] cmd: cmd
 * @param[in] *cmd_data: *cmd_data
 * @param[in] cmd_data_len: cmd_data_len
 * @param[in] *p_rsp_data: *p_rsp_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_group_br_edr_avtcp(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data);

/**
 * @brief test_group_br_edr_hfp
 *
 * @param[in] cmd: cmd
 * @param[in] *cmd_data: *cmd_data
 * @param[in] cmd_data_len: cmd_data_len
 * @param[in] *p_rsp_data: *p_rsp_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_group_br_edr_hfp(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data);

/**
 * @brief test_group_br_edr_eq
 *
 * @param[in] cmd: cmd
 * @param[in] *cmd_data: *cmd_data
 * @param[in] cmd_data_len: cmd_data_len
 * @param[in] *p_rsp_data: *p_rsp_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_group_br_edr_eq(UINT8_T cmd, UINT8_T *cmd_data, UINT32_T cmd_data_len, UINT8_T *p_rsp_data);

#endif

/**
 * @brief test_cmd_send
 *
 * @param[in] cmdId: cmdId
 * @param[in] buf: buf
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET test_cmd_send(UINT16_T cmdId, UINT8_T *buf, UINT16_T size);

/**
 * @brief tal_sdk_test_enter_sleep_handler
 *
 * @param[in] timer_id: timer_id
 * @param[in] arg: arg
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tal_sdk_test_enter_sleep_handler(TIMER_ID timer_id, VOID_T *arg);

/**
 * @brief tal_ble_sdk_test_wake_up_handler
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tal_ble_sdk_test_wake_up_handler(VOID_T);

#endif /* TUYA_SDK_TEST */


#ifdef __cplusplus
}
#endif

#endif /* __TAL_SDK_TEST_H__ */

