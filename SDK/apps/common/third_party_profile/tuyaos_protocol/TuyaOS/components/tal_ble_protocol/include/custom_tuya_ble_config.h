/**
 * @file custom_tuya_ble_config.h
 * @brief This is custom_tuya_ble_config file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __CUSTOM_TUYA_BLE_CONFIG_H__
#define __CUSTOM_TUYA_BLE_CONFIG_H__

#include "apps/tuyaos_demo_ble_peripheral/app_config.h"
#include "tuya_ble_type.h"
#include "tuya_iot_config.h"
#include "vendor/jl_ble/tuyaos/include/board.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define CUSTOMIZED_TUYA_BLE_APP_PRODUCT_TEST_HEADER_FILE    "tuya_ble_product_test_over_air.h"
#define CUSTOMIZED_TUYA_BLE_APP_UART_COMMON_HEADER_FILE     "tuya_ble_app_uart_module_handler.h"

#if defined(TUYA_SDK_TEST) && (TUYA_SDK_TEST == 1)

#ifndef TUYA_BLE_FEATURE_WEATHER_ENABLE
#define TUYA_BLE_FEATURE_WEATHER_ENABLE      1
#endif

#ifndef TUYA_BLE_FEATURE_BULKDATA_ENABLE
#define TUYA_BLE_FEATURE_BULKDATA_ENABLE     1
#endif

#ifndef TUYA_BLE_FEATURE_SCENE_ENABLE
#define TUYA_BLE_FEATURE_SCENE_ENABLE        1
#endif

#endif

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* __CUSTOM_TUYA_BLE_CONFIG_H__ */

