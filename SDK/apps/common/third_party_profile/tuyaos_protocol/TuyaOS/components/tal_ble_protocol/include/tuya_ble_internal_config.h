/**
 * @file tuya_ble_internal_config.h
 * @brief This is tuya_ble_internal_config file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_INTERNAL_CONFIG_H__
#define __TUYA_BLE_INTERNAL_CONFIG_H__

#include "tuya_ble_config.h"
#include "tuya_ble_port.h"
#include "tuya_ble_ota.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define MAX_NUMBER_OF_TUYA_MESSAGE        16      //!<  tuya ble message queue size

//BLE Communication protocol version v4.7
#define TUYA_BLE_PROTOCOL_VERSION_HIGN   0x04
#define TUYA_BLE_PROTOCOL_VERSION_LOW    0x08

#define TUYA_BLE_DP_WRITE_CURRENT_VERSION   0
#define TUYA_BLE_DP_WRITE_WITH_SRC_TYPE_CURRENT_VERSION 0

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN>=4)

#if (TUYA_BLE_OTA_PKG_LEN + 100 > 1536)
#define TUYA_BLE_AIR_FRAME_MAX                    (TUYA_BLE_OTA_PKG_LEN + 100)
#else
#define TUYA_BLE_AIR_FRAME_MAX                    (1536)
#endif

#define TUYA_UART_RECEIVE_MAX_DP_DATA_LEN         (512+4) // not use

#define TUYA_UART_RECEIVE_MAX_DP_BUFFER_DATA_LEN  (512+4) // not use

#define TUYA_BLE_RECEIVE_MAX_DP_DATA_LEN          (1024+5)

#define TUYA_BLE_SEND_MAX_DP_DATA_LEN              (TUYA_BLE_RECEIVE_MAX_DP_DATA_LEN - 5)

#define TUYA_BLE_SEND_MAX_DATA_LEN                 (((TUYA_BLE_AIR_FRAME_MAX-17)/16)*16-14)

#define TUYA_BLE_RECEIVE_MAX_DATA_LEN              TUYA_BLE_SEND_MAX_DATA_LEN

#else

#define TUYA_BLE_ADVANCED_ENCRYPTION_DEVICE 0

#define TUYA_BLE_AIR_FRAME_MAX                    1024

#define TUYA_UART_RECEIVE_MAX_DP_DATA_LEN         (255+4)
#define TUYA_UART_RECEIVE_MAX_DP_BUFFER_DATA_LEN  (255+4)

#define TUYA_BLE_RECEIVE_MAX_DP_DATA_LEN          (255+3)

#define TUYA_BLE_SEND_MAX_DP_DATA_LEN              TUYA_BLE_RECEIVE_MAX_DP_DATA_LEN

#define TUYA_BLE_REPORT_MAX_DP_DATA_LEN           TUYA_BLE_RECEIVE_MAX_DP_DATA_LEN

#define TUYA_BLE_TRANSMISSION_MAX_DATA_LEN       (TUYA_BLE_AIR_FRAME_MAX-29)

#define TUYA_BLE_SEND_MAX_DATA_LEN                TUYA_BLE_TRANSMISSION_MAX_DATA_LEN

#define TUYA_BLE_RECEIVE_MAX_DATA_LEN             TUYA_BLE_SEND_MAX_DATA_LEN

#endif

#define TUYA_BLE_GATT_SEND_DATA_QUEUE_SIZE  (MAX_NUMBER_OF_TUYA_MESSAGE*3)

/**
 * MACRO for memory management
 */
#define TUYA_BLE_TOTAL_HEAP_SIZE          5120

#if (TUYA_BLE_TOTAL_HEAP_SIZE<5120)
#define TUYA_BLE_BULK_DATA_MAX_READ_BLOCK_SIZE 512
#else
#define TUYA_BLE_BULK_DATA_MAX_READ_BLOCK_SIZE 1024
#endif

#ifndef TUYA_BLE_MAX_CALLBACKS
#define TUYA_BLE_MAX_CALLBACKS 1
#endif

#if (TUYA_BLE_SINGLE_BANK_OTA_MODE == 0)
#define        TUYA_BLE_AUTH_FLASH_ADDR             (TUYA_NV_START_ADDR)
#define        TUYA_BLE_AUTH_FLASH_BACKUP_ADDR      (TUYA_NV_START_ADDR+TUYA_NV_ERASE_MIN_SIZE)

#define        TUYA_BLE_SYS_FLASH_ADDR              (TUYA_NV_START_ADDR+TUYA_NV_ERASE_MIN_SIZE*2)
#define        TUYA_BLE_SYS_FLASH_BACKUP_ADDR       (TUYA_NV_START_ADDR+TUYA_NV_ERASE_MIN_SIZE*3)
#else
#define        TUYA_BLE_AUTH_FLASH_ADDR             (TUYA_NV_BLE_AUTH_FLASH_START_ADDR)
#define        TUYA_BLE_SYS_FLASH_ADDR              (TUYA_NV_BLE_SYS_FLASH_START_ADDR)
#define        TUYA_BLE_SYS_FLASH_BACKUP_ADDR       (TUYA_NV_BLE_SYS_FLASH_BACKUP_START_ADDR)
#endif

/**
 * 1 - device register from ble  0 - from others
 * @note:
 */
#define TUYA_BLE_DEVICE_REGISTER_FROM_BLE  (TUYA_BLE_DEVICE_COMMUNICATION_ABILITY&TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_REGISTER_FROM_BLE)

#define TUYA_BLE_DEVICE_AUTH_DATA_STORE                     TUYA_BLE_DEVICE_AUTH_SELF_MANAGEMENT

/**
 * if 1 ,tuya ble sdk authorization storage into extern medium, For example security chip
 */
#define TUYA_BLE_DEVICE_AUTH_DATA_STORE_EXT_MEDIUM          0

/**
 * if 1 ,used ble dongle for product test, the transmit datas will encryption
 */
#define TUYA_BLE_PROD_TEST_SUPPORT_ENCRYPTION                0

/**
 * if >=1, the product support oem, when execute prod test will write in pid and json config file
 *
 * Noticed!!! if enable the macro, need to realization 'tuya_ble_status_t tuya_ble_prod_storage_oem_info(UINT8_T *para,UINT16_T len)'
 * function yourself at the custom_app_product_test.c
 */
#ifndef TUYA_BLE_PROD_SUPPORT_OEM_TYPE
#if (TUYA_SDK_DEBUG_MODE)
#define TUYA_BLE_PROD_SUPPORT_OEM_TYPE                        TUYA_BLE_PROD_OEM_TYPE_NONE
#else
#define TUYA_BLE_PROD_SUPPORT_OEM_TYPE                        TUYA_BLE_PROD_OEM_TYPE_0_5
#endif
#endif

#if (TUYA_BLE_SYS_FLASH_BACKUP_ADDR>=(TUYA_NV_AREA_SIZE+TUYA_NV_START_ADDR))
#if !defined(TUYA_BLE_SINGLE_BANK_OTA_MODE) || (TUYA_BLE_SINGLE_BANK_OTA_MODE == 0)
#error "Storage Memory overflow!"
#endif
#endif

#if TUYA_BLE_VOS_ENABLE

#if (((TUYA_NV_VOS_TOKEN_START_ADDR+TUYA_NV_VOS_TOKEN_AREA_SIZE)>TUYA_NV_START_ADDR) && (TUYA_NV_VOS_TOKEN_START_ADDR<(TUYA_NV_AREA_SIZE+TUYA_NV_START_ADDR)))
#if !defined(TUYA_BLE_SINGLE_BANK_OTA_MODE) || (TUYA_BLE_SINGLE_BANK_OTA_MODE == 0)
#error "Vos token storage memory overflow!"
#endif
#endif

#if (TUYA_NV_VOS_TOKEN_AREA_SIZE<(2048))
#error "Vos token storage memory is not enough!"
#endif

#endif

#if ((TUYA_BLE_SECURE_CONNECTION_TYPE==TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_V2) \
    || (TUYA_BLE_SECURE_CONNECTION_TYPE==TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION_V2) \
    || (TUYA_BLE_SECURE_CONNECTION_TYPE==TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_FOR_QR_CODE_V2))
#define TUYA_BLE_ADV_LOCAL_NAME_MAX_LEN  TUYA_BLE_ADV_LOCAL_NAME_MAX_SPACE_LEN
#else
#define TUYA_BLE_ADV_LOCAL_NAME_MAX_LEN  5
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

#endif /* __TUYA_BLE_INTERNAL_CONFIG_H__ */

