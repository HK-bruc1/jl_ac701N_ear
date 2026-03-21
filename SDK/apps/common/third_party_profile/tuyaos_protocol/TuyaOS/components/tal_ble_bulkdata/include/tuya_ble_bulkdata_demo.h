/**
 * @file tuya_ble_bulkdata_demo.h
 * @brief This is tuya_ble_bulkdata_demo file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_BULKDATA_DEMO_H__
#define __TUYA_BLE_BULKDATA_DEMO_H__

#include "tuya_cloud_types.h"
#include "tuya_ble_bulkdata.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(TUYA_BLE_FEATURE_BULKDATA_ENABLE) && (TUYA_BLE_FEATURE_BULKDATA_ENABLE == 1)

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#ifndef TUYA_BLE_BULKDATA_TYPE_NUM
#define TUYA_BLE_BULKDATA_TYPE_NUM                  (1)
#endif

typedef enum {
    NEED_PARSING_BY_APP = 0,
    NEED_PARSING_BY_PRODUCT,
} TUYA_BLE_BULKDATA_FLAG_T;

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
#pragma pack(1)
typedef struct {
    UINT8_T type;
    UINT8_T flag;
} TUYA_BLE_BULKDATA_EXTERNAL_PARAM_T;

typedef struct {
    UINT32_T total_length;
    UINT32_T total_crc32;
    UINT32_T block_length;
} TUYA_BLE_BULKDATA_INFO_T;
#pragma pack()

typedef VOID_T(*TUYA_BLE_BULKDATA_INFO_CB)(TUYA_BLE_BULKDATA_INFO_T *info);
typedef VOID_T(*TUYA_BLE_BULKDATA_REPORT_CB)(UINT8_T *p_block_buf, UINT32_T block_length, UINT32_T block_number);
typedef VOID_T(*TUYA_BLE_BULKDATA_ERASE_CB)(UINT8_T type, UINT8_T *status);

typedef struct {
    TUYA_BLE_BULKDATA_INFO_CB info_cb;
    TUYA_BLE_BULKDATA_REPORT_CB report_cb;
    TUYA_BLE_BULKDATA_ERASE_CB erase_cb;
} TUYA_BLE_BULKDATA_CB_T;

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/

/**
 * @brief tuya_ble_bulk_data_init
 *
 * @param[in] type: defined by product, it starts with 1 generally.
 * @param[in] flag: 0x00-need parsing by App, 0x01-need parsing by product, else-not define.
 * @param[in] cb: callbacks
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tuya_ble_bulk_data_init(TUYA_BLE_BULKDATA_EXTERNAL_PARAM_T *param, TUYA_BLE_BULKDATA_CB_T *cb);

/**
 * @brief The device application calls this function to generate bulkdata test data.
 *
 * @param[in] timestep: timestep
 * @param[in] buf: buf
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tuya_ble_bulk_data_generation(UINT32_T timestep, UINT8_T *buf, UINT32_T size);

/**
 * @brief Function for handling the bulk data events.
 *        The bulk data reading steps are:
 *
 * @note The application must call this function where it receives the @ref TUYA_BLE_CB_EVT_BULK_DATA event.
 *
 * @param[in] evt: Event data received from the SDK.
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_bulk_data_demo_handler(tuya_ble_bulk_data_request_t *p_data);

#endif // TUYA_BLE_FEATURE_BULKDATA_ENABLE


#ifdef __cplusplus
}
#endif

#endif /* __TUYA_BLE_BULKDATA_DEMO_H__ */

