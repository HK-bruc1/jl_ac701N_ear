/**
 * @file tuya_ble_bulkdata.h
 * @brief This is tuya_ble_bulkdata file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_BULKDATA_H__
#define __TUYA_BLE_BULKDATA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_cloud_types.h"
#include "tuya_ble_type.h"
#include "tuya_ble_internal_config.h"

#if defined(TUYA_BLE_FEATURE_BULKDATA_ENABLE) && (TUYA_BLE_FEATURE_BULKDATA_ENABLE == 1)

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define FRM_BULK_DATA_READ_INFO_REQ          0x0007  //APP->BLE
#define FRM_BULK_DATA_READ_INFO_RESP         0x0007  //BLE->APP
#define FRM_BULK_DATA_READ_DATA_REQ          0x0008  //APP->BLE
#define FRM_BULK_DATA_READ_DATA_RESP         0x0008  //BLE->APP
#define FRM_BULK_DATA_SEND_DATA              0x0009  //BLE->APP
#define FRM_BULK_DATA_ERASE_DATA_REQ         0x000A  //APP->BLE
#define FRM_BULK_DATA_ERASE_DATA_RESP        0x000A  //BLE->APP
#define FRM_BULK_DATA_SYNC_REQUEST           0x000B  //BLE->APP

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
 * @brief Function for get the block size used for bulk data reading.
 * @note  The block size depends on the mtu value negotiated after the BLE connection is established.
 *        If the return value is 0, stop the current bulk data transmission.
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tuya_ble_bulk_data_read_block_size_get(VOID_T);

/**
 * @brief Function for handling the bulk data request.
 *
 * @param[in] cmd: Request command.
 * @param[in] p_recv_data: Pointer to the buffer with received data.
 * @param[in] recv_data_len: Length of received data.
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_bulk_data_req(UINT16_T cmd, UINT8_T *p_recv_data, UINT32_T recv_data_len);

/**
 * @brief Function for respond to app requests.
 *
 * @param[in] p_data: The pointer to the response data.
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_bulk_data_response(tuya_ble_bulk_data_response_t *p_data);

/**
 * @brief Function for handling the bulk data events.
 *
 * @param[in] p_evt: Event received from the application.
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_bulk_data_evt(tuya_ble_evt_param_t *p_evt);

/**
 * @brief tuya_ble_bulk_data_sync_request.
 *
 * @param[in] request_bulkdata_count: request_bulkdata_count.
 * @param[in] p_request_bulkdata_info: p_request_bulkdata_info must be global variable.
 *
 */
VOID_T tuya_ble_bulk_data_sync_request(UINT8_T request_bulkdata_count, UINT8_T *p_request_bulkdata_info);

#endif // TUYA_BLE_FEATURE_BULKDATA_ENABLE


#ifdef __cplusplus
}
#endif

#endif /* __TUYA_BLE_BULKDATA_H__ */

