/**
 * @file tuya_ble_log.h
 * @brief This is tuya_ble_log file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_LOG_H__
#define __TUYA_BLE_LOG_H__

#include "tal_log.h"
#include "tuya_ble_config.h"
#include "custom_tuya_ble_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#if TUYA_BLE_LOG_ENABLE

#define TUYA_BLE_LOG_ERROR(fmt, ...)                        TAL_PR_ERR(fmt, ##__VA_ARGS__)
#define TUYA_BLE_LOG_WARNING(fmt, ...)                      TAL_PR_WARN(fmt, ##__VA_ARGS__)
#define TUYA_BLE_LOG_INFO(fmt, ...)                         TAL_PR_INFO(fmt, ##__VA_ARGS__)
#define TUYA_BLE_LOG_DEBUG(fmt, ...)                        TAL_PR_DEBUG(fmt, ##__VA_ARGS__)

#define TUYA_BLE_LOG_HEXDUMP_ERROR(title, buf, size)        TAL_PR_HEXDUMP_ERR(title, buf, size)
#define TUYA_BLE_LOG_HEXDUMP_WARNING(title, buf, size)      TAL_PR_HEXDUMP_WARN(title, buf, size)
#define TUYA_BLE_LOG_HEXDUMP_INFO(title, buf, size)         TAL_PR_HEXDUMP_INFO(title, buf, size)
#define TUYA_BLE_LOG_HEXDUMP_DEBUG(title, buf, size)        TAL_PR_HEXDUMP_DEBUG(title, buf, size)

#else

#define TUYA_BLE_LOG_ERROR(fmt, ...)
#define TUYA_BLE_LOG_WARNING(fmt, ...)
#define TUYA_BLE_LOG_INFO(fmt, ...)
#define TUYA_BLE_LOG_DEBUG(fmt, ...)

#define TUYA_BLE_LOG_HEXDUMP_ERROR(title, buf, size)
#define TUYA_BLE_LOG_HEXDUMP_WARNING(title, buf, size)
#define TUYA_BLE_LOG_HEXDUMP_INFO(title, buf, size)
#define TUYA_BLE_LOG_HEXDUMP_DEBUG(title, buf, size)

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

#endif /* __TUYA_BLE_LOG_H__ */

