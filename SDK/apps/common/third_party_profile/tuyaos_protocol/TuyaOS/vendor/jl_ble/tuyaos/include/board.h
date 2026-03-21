/**
 * @file board.h
 * @brief This is board file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "apps/tuyaos_demo_ble_peripheral/app_config.h"
#include "tal_log.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#ifndef BOARD_ENABLE_LOG
#define BOARD_ENABLE_LOG                        (0)
#endif

#if (BOARD_ENABLE_LOG)
#define TY_PRINTF(fmt, ...)                 TAL_PR_INFO(fmt, ##__VA_ARGS__)
#define TY_HEXDUMP(title, buf, size)        TAL_PR_HEXDUMP_INFO(title, buf, size)
#else
#define TY_PRINTF(fmt, ...)
#define TY_HEXDUMP(title, buf, size)
#endif

//RAM
#ifndef BOARD_HEAP_SIZE
#define BOARD_HEAP_SIZE                         (1024*5)
#endif

#ifndef BOARD_UART_TX_BUF_SIZE
#define BOARD_UART_TX_BUF_SIZE                  (512)
#endif

// Flash
#ifndef BOARD_FLASH_STORAGE_A_ADDR
#define BOARD_FLASH_STORAGE_A_ADDR              (0x66000)
#endif

#ifndef BOARD_FLASH_STORAGE_B_ADDR
#define BOARD_FLASH_STORAGE_B_ADDR              (0x67000)
#endif

#ifndef BOARD_FLASH_SDK_TEST_START_ADDR
#define BOARD_FLASH_SDK_TEST_START_ADDR        (0x70000)
#endif

#ifndef BOARD_FLASH_TUYA_INFO_START_ADDR
#define BOARD_FLASH_TUYA_INFO_START_ADDR        (0x74000)
#endif

// #ifndef BOARD_FLASH_OTA_INFO_ADDR
// #define BOARD_FLASH_OTA_INFO_ADDR               (0x7C000)
// #endif

#ifndef BOARD_FLASH_OTA_START_ADDR
#define BOARD_FLASH_OTA_START_ADDR              (0x46000)
#endif

#ifndef BOARD_FLASH_OTA_END_ADDR
#define BOARD_FLASH_OTA_END_ADDR                (0x66000)
#endif

#ifndef BOARD_FLASH_OTA_SIZE
#define BOARD_FLASH_OTA_SIZE                    (BOARD_FLASH_OTA_END_ADDR - BOARD_FLASH_OTA_START_ADDR)
#endif

// #ifndef BOARD_FLASH_MAC_START_ADDR
// #define BOARD_FLASH_MAC_START_ADDR              (0)
// #endif

// PIN
#ifndef BOARD_POWER_ON_PIN
#define BOARD_POWER_ON_PIN                      (TUYA_GPIO_NUM_5)
#endif

#ifndef BOARD_KEY_PIN
#define BOARD_KEY_PIN                           (TUYA_GPIO_NUM_14)
#endif

#ifndef BOARD_GPIO_IRQ_NUM
#define BOARD_GPIO_IRQ_NUM                      (2)
#endif

// Timer
#ifndef BOARD_TIMER_MAX_NUM
#define BOARD_TIMER_MAX_NUM                     (20)
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

#endif /* __BOARD_H__ */
