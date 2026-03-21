/**
 * @file tal_local_timer.h
 * @brief This is tal_local_timer file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TAL_LOCAL_TIMER_H__
#define __TAL_LOCAL_TIMER_H__

#include "tuya_cloud_types.h"
#include "tuya_error_code.h"
#include "tuya_ble_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define TAL_LOCAL_TIMER_MAX_NUM         5

typedef enum {
    TAL_LOCAL_TIMER_INFO_START_ID = 0,
    TAL_LOCAL_TIMER_DP_START_ID = 0,
    TAL_LOCAL_TIMER_INFO_COUNT_ID = 127,
} tuya_ble_timer_kv_id_t;

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
#pragma pack(1)
typedef struct {
    uint8_t  type;
    uint16_t len;
    uint8_t  year; // low 2 numbers
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  loop;
    uint32_t timer_id;
    uint8_t *dp_data;
    uint32_t crc32;
} tal_local_timer_info_t;
#pragma pack()

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
extern tal_local_timer_info_t g_local_timer_info[];

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/

/**
 * @brief tal_local_timer_init
 *
 * @param[in] buf: buf
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
uint32_t tal_local_timer_init(void);

/**
 * @brief tal_local_timer_handler
 *
 * @param[in] buf: buf
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
uint32_t tal_local_timer_handler(tuya_ble_app_passthrough_data_t *data);

/**
 * @brief tal_local_timer_parser_event
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
void tal_local_timer_parser_event(void);

/**
 * @brief tal_local_timer_storage_init
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
uint32_t tal_local_timer_storage_init(void);

/**
 * @brief tal_local_timer_info_write
 *
 * @param[in] data: data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
uint32_t tal_local_timer_info_write(tal_local_timer_info_t *data);

/**
 * @brief tal_local_timer_info_delete
 *
 * @param[in] timer_id: timer_id
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
uint32_t tal_local_timer_info_delete(uint32_t timer_id);

/**
 * @brief tal_local_timer_dp_item_read
 *
 * @param[in] id: id
 * @param[in] buf: buf
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
uint32_t tal_local_timer_dp_item_read(uint8_t id, uint8_t *buf, uint32_t size);

/**
 * @brief tal_local_timer_info_delete_all
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
uint32_t tal_local_timer_info_delete_all(void);

/**
 * @brief tal_local_timer_get_num
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
uint32_t tal_local_timer_get_num(void);


#ifdef __cplusplus
}
#endif

#endif /* __TAL_LOCAL_TIMER_H__ */

