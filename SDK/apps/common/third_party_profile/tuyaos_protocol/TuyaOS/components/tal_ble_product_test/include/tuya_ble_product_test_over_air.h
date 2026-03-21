/**
 * @file tuya_ble_product_test_over_air.h
 * @brief This is tuya_ble_product_test_over_air file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_PRODUCT_TEST_OVER_AIR_H__
#define __TUYA_BLE_PRODUCT_TEST_OVER_AIR_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
//RFU-remain_for_future_use, PID-product_id, FW-firmware, FPM-finger_print_module
#define PRODUCT_TEST_CMD_ENTER                  0xFF02
#define PRODUCT_TEST_CMD_EXIT                   0xFF03
#define PRODUCT_TEST_CMD_LED                    0x0001
#define PRODUCT_TEST_CMD_RELAY                  0x0002
#define PRODUCT_TEST_CMD_KEY                    0x0003
#define PRODUCT_TEST_CMD_SWITCH_SENSOR          0x0004
#define PRODUCT_TEST_CMD_RFU                    0x0005
#define PRODUCT_TEST_CMD_ANALOG_SENSOR          0x0006
#define PRODUCT_TEST_CMD_MOTOR                  0x0007
#define PRODUCT_TEST_CMD_BATTERY_PARAM          0x0008
#define PRODUCT_TEST_CMD_BATTERY_CALIBRATION    0x0009
#define PRODUCT_TEST_CMD_LED_RGBCW              0x000A
#define PRODUCT_TEST_CMD_BURNIN                 0x000B
#define PRODUCT_TEST_CMD_INFRARED_TX            0x000C
#define PRODUCT_TEST_CMD_INFRARED_RX_ENTER      0x000D
#define PRODUCT_TEST_CMD_INFRARED_RX_EXIT       0x000E
#define PRODUCT_TEST_CMD_WRITE_SN               0x000F
#define PRODUCT_TEST_CMD_READ_SN                0x0010
#define PRODUCT_TEST_CMD_BLE_RSSI               0x0011
#define PRODUCT_TEST_CMD_WIFI_RSSI              0x0012
#define PRODUCT_TEST_CMD_GSENSOR                0x0013
#define PRODUCT_TEST_CMD_READ_MAC               0x0014
#define PRODUCT_TEST_CMD_READ_PID               0x0015
#define PRODUCT_TEST_CMD_READ_FW_INFO           0x0016
#define PRODUCT_TEST_CMD_FPM                    0x0017
#define PRODUCT_TEST_CMD_COMMON_TEST            0x001A

#define COMMON_TEST_SUB_CMD                     0x01

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
    UINT16_T sub_id;
    UINT8_T  value[];
} ty_product_test_cmd_t;
#pragma pack()

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/

/**
 * @brief
 *
 * @param[in] param1:
 * @param[in] param2:
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_prod_sleep_timer_init(VOID_T);

/**
 * @brief
 *
 * @param[in] param1:
 * @param[in] param2:
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_custom_app_production_test_process(UINT8_T channel, UINT8_T *p_in_data, UINT16_T in_len);

/**
 * @brief tuya_ble_product_test_rsp
 *
 * @param[in] channel: channel
 * @param[in] cmdId: cmdId
 * @param[in] buf: buf
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tuya_ble_product_test_rsp(UINT8_T channel, UINT16_T cmdId, UINT8_T *buf, UINT16_T size);


#ifdef __cplusplus
}
#endif

#endif /* __TUYA_BLE_PRODUCT_TEST_OVER_AIR_H__ */

