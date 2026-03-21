/**
 * @file tal_ble_app_passthrough.h
 * @brief This is tal_ble_app_passthrough file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TAL_BLE_APP_PASSTHROUGH_H__
#define __TAL_BLE_APP_PASSTHROUGH_H__

#include "tuya_cloud_types.h"
#include "tuya_error_code.h"
#include "tuya_ble_type.h"
#include "tuya_ble_config.h"
#include "tuya_ble_internal_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#if ( (TUYA_BLE_PROTOCOL_VERSION_HIGN>=4) && (TUYA_BLE_FEATURE_APP_PASSTHROUGH_ENABLE != 0) )

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
typedef enum {
    TAL_BLE_APP_PASSTHROUGH_SUBCMD_LOCAL_TIMER = 0x0000,
    TAL_BLE_APP_PASSTHROUGH_SUBCMD_EARPHONE_GAIN = 0x0001,
    TAL_BLE_APP_PASSTHROUGH_SUBCMD_BLE_PROXY = 0x0010,
    TAL_BLE_APP_PASSTHROUGH_SUBCMD_REPEATER = 0x1000,
    TAL_BLE_APP_PASSTHROUGH_SUBCMD_CONTACT_LIST = 0x8000,
    TAL_BLE_APP_PASSTHROUGH_SUBCMD_WORLD_TIME = 0x8001,
    TAL_BLE_APP_PASSTHROUGH_SUBCMD_STOCK_MARKET = 0x8002,
    TAL_BLE_APP_PASSTHROUGH_SUBCMD_MESH_PROXY = 0x9000,
} TAL_BLE_APP_PASSTHROUGH_SUBCMD_E;

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/

/**
 * @brief tal_ble_app_passthrough_handler
 *
 * @param[in] buf: buf
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
void tal_ble_app_passthrough_handler(uint8_t *buf, uint32_t size);

/**
 * @brief tal_ble_app_passthrough_disconnect_handler
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
void tal_ble_app_passthrough_disconnect_handler(void);

#endif // TUYA_BLE_FEATURE_APP_PASSTHROUGH_ENABLE

/**
 * @brief tal_ble_app_passthrough_data_send
 *
 * @param[in] p_app_frame_data: p_app_frame_data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tal_ble_app_passthrough_data_send(tuya_ble_app_passthrough_data_t *p_data);


#ifdef __cplusplus
}
#endif

#endif /* __TAL_BLE_APP_PASSTHROUGH_H__ */

