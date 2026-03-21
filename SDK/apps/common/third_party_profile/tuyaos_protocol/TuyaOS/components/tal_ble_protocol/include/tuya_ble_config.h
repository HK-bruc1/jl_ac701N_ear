/**
 * @file tuya_ble_config.h
 * @brief This is tuya_ble_config file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_CONFIG_H__
#define __TUYA_BLE_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CUSTOMIZED_TUYA_BLE_CONFIG_FILE)
#include CUSTOMIZED_TUYA_BLE_CONFIG_FILE
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
/*
 * If the application has a customized production test project(file),please define CUSTOMIZED_TUYA_BLE_APP_PRODUCT_TEST_FILE in CUSTOMIZED_TUYA_BLE_CONFIG_FILE.
 */
#ifndef CUSTOMIZED_TUYA_BLE_APP_PRODUCT_TEST_HEADER_FILE
#undef  CUSTOMIZED_TUYA_BLE_APP_PRODUCT_TEST_HEADER_FILE
#endif

/*
 * If needed,please define CUSTOMIZED_TUYA_BLE_APP_UART_COMMON_FILE in CUSTOMIZED_TUYA_BLE_CONFIG_FILE.
 */
#ifndef CUSTOMIZED_TUYA_BLE_APP_UART_COMMON_HEADER_FILE
#undef  CUSTOMIZED_TUYA_BLE_APP_UART_COMMON_HEADER_FILE
#endif

/*
 * If needed,please define TUYA_BLE_PORT_PLATFORM_HEADER_FILE in CUSTOMIZED_TUYA_BLE_CONFIG_FILE.
 */
#ifndef TUYA_BLE_PORT_PLATFORM_HEADER_FILE
#undef  TUYA_BLE_PORT_PLATFORM_HEADER_FILE
#endif

/*
 * If using an OS, be sure to call the tuya api functions and SDK-related queues within the same task
 */
#ifndef TUYA_BLE_USE_OS
#define TUYA_BLE_USE_OS                                 0
#endif

/*
* Whether to support long range.
*/
#ifndef TUYA_BLE_FEATURE_LONG_RANGE
#define TUYA_BLE_FEATURE_LONG_RANGE                     0
#endif

/*
 * If using an OS, tuya ble sdk  will create a task autonomously to process ble event.
 */
#if TUYA_BLE_USE_OS

#ifndef TUYA_BLE_SELF_BUILT_TASK
#define TUYA_BLE_SELF_BUILT_TASK                        0
#endif

#if TUYA_BLE_SELF_BUILT_TASK

#ifndef TUYA_BLE_TASK_PRIORITY
#define TUYA_BLE_TASK_PRIORITY                          1
#endif

#ifndef TUYA_BLE_TASK_STACK_SIZE
#define TUYA_BLE_TASK_STACK_SIZE  256 * 10   //!<  Task stack size ，application do not change this default value
#endif

#endif

#endif
/*
 * TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_BLE                   ble normal
 * TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_REGISTER_FROM_BLE     device register from ble
 * TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_MESH                  ble mesh
 * TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_WIFI_24G              wifi_2.4g
 * TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_WIFI_5G               wifi_5g
 * TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_ZIGBEE                zigbee
 * TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_NB                    nb-iot
 * @note:
 * for example (TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_BLE|TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_MESH|TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_WIFI_24G)
 */
//#define TUYA_BLE_DEVICE_COMMUNICATION_ABILITY  (TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_BLE|TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_WIFI_24G|TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_WIFI_5G)
#ifndef TUYA_BLE_DEVICE_COMMUNICATION_ABILITY
#if TUYA_BLE_FEATURE_LONG_RANGE
#define TUYA_BLE_DEVICE_COMMUNICATION_ABILITY  (TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_BLE \
                                                | TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_LONG_RANGE \
                                                | TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_REGISTER_FROM_BLE)
#else
#define TUYA_BLE_DEVICE_COMMUNICATION_ABILITY  (TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_BLE|TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_REGISTER_FROM_BLE)
#endif
#endif

/*
 * if 1 ,tuya ble sdk device ability self-management
 */
#ifndef TUYA_BLE_DEVICE_ABILITY_SELF_MANAGEMENT
#define TUYA_BLE_DEVICE_ABILITY_SELF_MANAGEMENT         1
#endif

/*
 * Whether it is a shared device.
 */
#ifndef TUYA_BLE_DEVICE_SHARED
#define TUYA_BLE_DEVICE_SHARED                          0
#endif

/*
 * If it is 1, then sdk need to perform the unbind operation,otherwise sdk do not need.
 */
#ifndef TUYA_BLE_DEVICE_UNBIND_MODE
#define TUYA_BLE_DEVICE_UNBIND_MODE                     1
#endif

/*
 * If TUYA_BLE_WIFI_DEVICE_REGISTER_MODE is 1,Mobile app must first sends instructions to query network status after sending the pairing request .
 */
#ifndef TUYA_BLE_WIFI_DEVICE_REGISTER_MODE
#define TUYA_BLE_WIFI_DEVICE_REGISTER_MODE              1
#endif

/*
 * if 1 ,tuya ble sdk authorization self-management
 */
#ifndef TUYA_BLE_DEVICE_AUTH_SELF_MANAGEMENT
#define  TUYA_BLE_DEVICE_AUTH_SELF_MANAGEMENT           1
#endif

/*
 * TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY     encrypt with auth key
 * TUYA_BLE_SECURE_CONNECTION_WITH_ECC          encrypt with ECDH
 * TUYA_BLE_SECURE_CONNECTION_WTIH_PASSTHROUGH  no encrypt
 * TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION  advanced encrypt(security chip ) with auth key
 * @note : only choose one
 */
#ifndef  TUYA_BLE_SECURE_CONNECTION_TYPE
#define  TUYA_BLE_SECURE_CONNECTION_TYPE  TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY
#endif

/*
 * MACRO for advanced encryption,if 1 will perform two-way authentication on every connection,otherwise perform certification at the first registration.
 */
#if (TUYA_BLE_SECURE_CONNECTION_TYPE==TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)

#ifndef TUYA_BLE_ADVANCED_ENCRYPTION_AUTH_ON_CONNECT
#define TUYA_BLE_ADVANCED_ENCRYPTION_AUTH_ON_CONNECT    0
#endif

#endif

/*
 * if 1 ,ble sdk will update mac address with with the address of the authorization information.
 */
#ifndef  TUYA_BLE_DEVICE_MAC_UPDATE
#define  TUYA_BLE_DEVICE_MAC_UPDATE                     1
#endif

/*
 * if 1 ,after update mac will reset.
 */
#ifndef  TUYA_BLE_DEVICE_MAC_UPDATE_RESET
#define  TUYA_BLE_DEVICE_MAC_UPDATE_RESET               0
#endif

/*
 * if defined ,include UART module
 */
//#define TUYA_BLE_UART

/*
 * if defined ,include product test module
 */
//#define TUYA_BLE_PRODUCTION_TEST

/*
 * gatt mtu max sizes
 */
#ifndef TUYA_BLE_DATA_MTU_MAX
#define TUYA_BLE_DATA_MTU_MAX                           244
#endif

/*
 * if defined ,enable sdk log output
 */

#ifndef TUYA_BLE_LOG_ENABLE
#define TUYA_BLE_LOG_ENABLE                             0
#endif

#ifndef TUYA_BLE_LOG_COLORS_ENABLE
#define TUYA_BLE_LOG_COLORS_ENABLE                      0
#endif

#ifndef TUYA_BLE_LOG_LEVEL
#define TUYA_BLE_LOG_LEVEL                              TUYA_BLE_LOG_LEVEL_DEBUG
#endif

/*
 * If it is 1, then sdk need to request beacon key from app when binding .
 */
#ifndef TUYA_BLE_BEACON_KEY_ENABLE
#define TUYA_BLE_BEACON_KEY_ENABLE                      0
#endif

/*
 * Enable the weather module.
 */
#ifndef TUYA_BLE_FEATURE_WEATHER_ENABLE
#define TUYA_BLE_FEATURE_WEATHER_ENABLE                 0
#endif

/*
 * Enable the bulkdata module.
 */
#ifndef TUYA_BLE_FEATURE_BULKDATA_ENABLE
#define TUYA_BLE_FEATURE_BULKDATA_ENABLE                1
#endif

/*
 * Enable the speech.
 */
#ifndef TUYA_BLE_FEATURE_SPEECH_ENABLE
#define TUYA_BLE_FEATURE_SPEECH_ENABLE                  0
#endif

/*
 * Enable the gpt.
 */
#ifndef TUYA_BLE_FEATURE_GPT_ENABLE
#define TUYA_BLE_FEATURE_GPT_ENABLE                     0
#endif

/*
 * Enable the feature scene module.
 */
#ifndef TUYA_BLE_FEATURE_SCENE_ENABLE
#define TUYA_BLE_FEATURE_SCENE_ENABLE                   0
#endif

#if (TUYA_BLE_FEATURE_SCENE_ENABLE != 0)
#define TUYA_BLE_FEATURE_IOT_CHANNEL_ENABLE             1
#endif

/*
 * Enable the ble+x module.
 */
#ifndef TUYA_BLE_FEATURE_EXT_MODULE_ENABLE
#define TUYA_BLE_FEATURE_EXT_MODULE_ENABLE              0
#endif

/*
 * Enable the uart common module.
 */
#ifndef TUYA_BLE_FEATURE_UART_COMMON_ENABLE
#define TUYA_BLE_FEATURE_UART_COMMON_ENABLE             0
#endif

/*
 * Enable the product test module.
 */
#ifndef TUYA_BLE_FEATURE_PRODUCT_TEST_ENABLE
#define TUYA_BLE_FEATURE_PRODUCT_TEST_ENABLE            0
#endif

/*
 * Enable the contact list module.
 */
#ifndef TUYA_BLE_FEATURE_CONTACT_LIST_ENABLE
#define TUYA_BLE_FEATURE_CONTACT_LIST_ENABLE            0
#endif

/*
 * Enable the local timer module.
 */
#ifndef TUYA_BLE_FEATURE_LOCAL_TIMER_ENABLE
#define TUYA_BLE_FEATURE_LOCAL_TIMER_ENABLE             0
#endif

/*
 * Enable the repeater module.
 */
#ifndef TUYA_BLE_FEATURE_REPEATER_ENABLE
#define TUYA_BLE_FEATURE_REPEATER_ENABLE                0
#endif

/*
 * Enable system connection.
 */
#ifndef TUYA_BLE_FEATURE_SYSTEM_CONNECTION_ENABLE
#define TUYA_BLE_FEATURE_SYSTEM_CONNECTION_ENABLE       0
#endif

/*
 * Enable the ota module.
 */
#ifndef TUYA_BLE_FEATURE_OTA_ENABLE
#define TUYA_BLE_FEATURE_OTA_ENABLE                     0
#endif

/*
 * Enable the ota module.
 */
#ifndef TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE
#define TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE              0
#endif

#if (TUYA_BLE_FEATURE_LOCAL_TIMER_ENABLE != 0)
#define TUYA_BLE_FEATURE_APP_PASSTHROUGH_ENABLE         1
#endif

#if (TUYA_BLE_FEATURE_REPEATER_ENABLE != 0)
#define TUYA_BLE_FEATURE_APP_PASSTHROUGH_ENABLE         1
#endif

/*
 * Enable support the link layer encryption.
 */
#ifndef TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE
#define TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE   0
#endif

/*
 * Forced the link layer encryption.
 */
#if TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE

#ifndef TUYA_BLE_LINK_LAYER_FORCED_ENCRYPTION
#define TUYA_BLE_LINK_LAYER_FORCED_ENCRYPTION           0
#endif

#endif

/*
 * Whether to automatically perform a time request after the connection is established.
 * - 0  Not request.
 * - 1  Request cloud time.
 * - 2  Request phone local time.
 */
#ifndef TUYA_BLE_AUTO_REQUEST_TIME_CONFIGURE
#define TUYA_BLE_AUTO_REQUEST_TIME_CONFIGURE            1
#endif

/*
* Whether to support connectivity select.
*/
#ifndef TUYA_BLE_CONNECTIVITY
#define TUYA_BLE_CONNECTIVITY                           0
#endif

/*
 * Whether to use three-part version number.
 */
#ifndef TUYA_BLE_THREE_PART_VERSION_NUMBER_ENABLE
#define TUYA_BLE_THREE_PART_VERSION_NUMBER_ENABLE       1
#endif

/*
 * Whether to support mount accessories.
 */
#ifndef TUYA_BLE_ACCESSORY_MOUNT_SUPPORTED
#define TUYA_BLE_ACCESSORY_MOUNT_SUPPORTED              0
#endif

/*
 * Whether to support muti data source.
 */
#ifndef TUYA_BLE_MUTI_DATA_SOURCE_SUPPORTED
#define TUYA_BLE_MUTI_DATA_SOURCE_SUPPORTED             0
#endif

/*
* Whether to support gateway connect mode.
*/
#ifndef TUYA_BLE_GATEWAY_CONN_MODE
#define TUYA_BLE_GATEWAY_CONN_MODE                      0
#endif

/*
* Whether to support single bank ota mode.
*/
#ifndef TUYA_BLE_SINGLE_BANK_OTA_MODE
#define TUYA_BLE_SINGLE_BANK_OTA_MODE                   0
#endif

/*
* Whether to enable ota signature.
*/
#ifndef TUYA_BLE_OTA_SIGNATURE_ENABLE
#define TUYA_BLE_OTA_SIGNATURE_ENABLE                   0
#endif

/*
* Whether to enable group.
*/
#ifndef TUYA_BLE_FEATURE_ENABLE_GROUP
#define TUYA_BLE_FEATURE_ENABLE_GROUP                   0
#endif

/*
 * Indicates whether currently running in bootloader.
 */
#ifndef TUYA_BLE_SINGLE_BANK_RUNNING_IN_BOOTLOADER
#define TUYA_BLE_SINGLE_BANK_RUNNING_IN_BOOTLOADER        0
#endif

/*
* uart protocol rx buffer max size configuration.
*/
#ifndef TUYA_BLE_UART_RX_BUFFER_MAX
#define TUYA_BLE_UART_RX_BUFFER_MAX                     300
#endif

//nv
/* The minimum size of flash erasure. May be a flash sector size. */
#ifndef TUYA_NV_ERASE_MIN_SIZE
#define TUYA_NV_ERASE_MIN_SIZE                          (0x1000)
#endif

/* the flash write granularity, unit: byte*/
#ifndef TUYA_NV_WRITE_GRAN
#define TUYA_NV_WRITE_GRAN                              (4)
#endif

/* start address */
#ifndef TUYA_NV_START_ADDR
#define TUYA_NV_START_ADDR                              BOARD_FLASH_TUYA_INFO_START_ADDR
#endif

#if (TUYA_BLE_SINGLE_BANK_OTA_MODE)
/* start address */
#ifndef TUYA_NV_BLE_AUTH_FLASH_START_ADDR
#define TUYA_NV_BLE_AUTH_FLASH_START_ADDR                               BOARD_FLASH_TUYA_AUTH_INFO_START_ADDR
#endif

/* start address */
#ifndef TUYA_NV_BLE_SYS_FLASH_START_ADDR
#define TUYA_NV_BLE_SYS_FLASH_START_ADDR                                BOARD_FLASH_TUYA_SYS_INFO_START_ADDR
#endif

/* start address */
#ifndef TUYA_NV_BLE_SYS_FLASH_BACKUP_START_ADDR
#define TUYA_NV_BLE_SYS_FLASH_BACKUP_START_ADDR                         BOARD_FLASH_TUYA_SYS_INFO_BACKUP_START_ADDR
#endif
#endif

/* area size. */
#ifndef TUYA_NV_AREA_SIZE
#define TUYA_NV_AREA_SIZE                               (4*TUYA_NV_ERASE_MIN_SIZE)
#endif

#ifndef TUYA_BLE_APP_FIRMWARE_KEY
#define TUYA_BLE_APP_FIRMWARE_KEY                       ""
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

#endif /* __TUYA_BLE_CONFIG_H__ */
