/**
 * @file tal_ble_beacon.h
 * @brief This is tal_ble_beacon file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TAL_BLE_BEACON_H__
#define __TAL_BLE_BEACON_H__

#include "tuya_cloud_types.h"
#include "tuya_ble_internal_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#ifndef SUPPORT_TUYAOS
#define SUPPORT_TUYAOS                         1
#endif

#ifndef TAL_BLE_BEACON_INCLUDE_ID
#define TAL_BLE_BEACON_INCLUDE_ID              0
#endif
#ifndef TAL_BLE_BEACON_REQUEST_CONNECTION
#define TAL_BLE_BEACON_REQUEST_CONNECTION      0
#endif
#ifndef TAL_BLE_BEACON_SHARE_FLAG
#define TAL_BLE_BEACON_SHARE_FLAG              0
#endif
#ifndef TAL_BLE_BEACON_BOND_FLAG
#define TAL_BLE_BEACON_BOND_FLAG               1
#endif

#ifndef TAL_BLE_BEACON_SECURITY_V2_SUPPORT
#define TAL_BLE_BEACON_SECURITY_V2_SUPPORT     1
#endif
#ifndef TAL_BLE_BEACON_SECURITY_V2_ENABLE
#define TAL_BLE_BEACON_SECURITY_V2_ENABLE      1
#endif
#ifndef TAL_BLE_BEACON_GATEWAY_CONNECT_MODE
#define TAL_BLE_BEACON_GATEWAY_CONNECT_MODE    0 // 0-short connect, 1-long connect
#endif
#ifndef TAL_BLE_BEACON_CONNECTIVITY
#define TAL_BLE_BEACON_CONNECTIVITY            0 // 0-phone and gateway, 1-gateway, 2-phone
#endif
#ifndef TAL_BLE_BEACON_INCLUDE_DP_DATA
#define TAL_BLE_BEACON_INCLUDE_DP_DATA         0
#endif

#ifndef TAL_BLE_BEACON_INCLUDE_DP_DATA_V2
#define TAL_BLE_BEACON_INCLUDE_DP_DATA_V2      0
#endif
#ifndef TAL_BLE_BEACON_ROAMING_FLAG
#define TAL_BLE_BEACON_ROAMING_FLAG            0
#endif
#ifndef TAL_BLE_BEACON_ONLINE_STRATEGY
#define TAL_BLE_BEACON_ONLINE_STRATEGY         0 // 0-standard power consumption, 1-low power consumption(only gateway)
#endif

typedef enum {
    INCLUDE_ID_OFFSET = 0,
    REQUEST_CONNECTION_OFFSET = 1,
    SHARE_FLAG_OFFSET = 2,
    BOND_FLAG_OFFSET = 3,
    PROTOCOL_VERSION_HIGN_OFFSET = 4,

    SECURITY_V2_SUPPORT_OFFSET = 2,
    SECURITY_V2_ENABLE_OFFSET = 3,
    GATEWAY_CONNECT_MODE_OFFSET = 4,
    CONNECTIVITY_OFFSET = 5,
    INCLUDE_DP_DATA_OFFSET = 7,

    INCLUDE_DP_DATA_V2_OFFSET = 5,
    ROAMING_FLAG_OFFSET = 6,
    ONLINE_STRATEGY_OFFSET = 7,
} FRAME_CONTROL_OFFSET_E;

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
typedef void (* TAL_BLE_BEACON_CB)(void *arg);

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
void tal_ble_beacon_init(uint16_t default_adv_interval, TAL_BLE_BEACON_CB cb);

/**
 * @brief Note that both this function and tuya_ble_adv_change modify adv
 *
 * @param[in] param1:
 * @param[in] param2:
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
uint32_t tal_ble_beacon_dp_data_send(uint32_t sn, uint8_t encrypt_mode, uint16_t adv_interval, uint16_t adv_duration, uint8_t *p_dp_data, uint32_t dp_data_len);


#ifdef __cplusplus
}
#endif

#endif /* __TAL_BLE_BEACON_H__ */

