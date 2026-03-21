/**
 * @file tuya_ble_feature_accessory.h
 * @brief This is tuya_ble_feature_accessory file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_FEATURE_ACCESSORY_H__
#define __TUYA_BLE_FEATURE_ACCESSORY_H__

#include "tuya_ble_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
/**@brief   Macro for defining accessory transfer protocol. */
#define FRM_ACCESSORY_INFO_READING                  0x8020      //APP->BLE
#define FRM_ACCESSORY_INFO_REPORT_REQ               0x8021      //BLE->APP
#define FRM_ACCESSORY_INFO_REPORT_RESP              0x8021      //APP->BLE
#define FRM_ACCESSORY_ACTIVE_INFO_RECEIVED          0x8022      //APP ->BLE
#define FRM_ACCESSORY_ACTIVE_INFO_RECEIVED_RESP     0x8022      //BLE ->APP

#define FRM_ACCESSORY_OTA_START_REQ                 0x0080      //APP->BLE
#define FRM_ACCESSORY_OTA_START_RESP                0x0080      //BLE->APP
#define FRM_ACCESSORY_OTA_FILE_INFOR_REQ            0x0081      //APP->BLE
#define FRM_ACCESSORY_OTA_FILE_INFOR_RESP           0x0081      //BLE->APP
#define FRM_ACCESSORY_OTA_FILE_OFFSET_REQ           0x0082      //APP->BLE
#define FRM_ACCESSORY_OTA_FILE_OFFSET_RESP          0x0082      //BLE->APP
#define FRM_ACCESSORY_OTA_DATA_REQ                  0x0083      //APP->BLE
#define FRM_ACCESSORY_OTA_DATA_RESP                 0x0083      //BLE->APP
#define FRM_ACCESSORY_OTA_END_REQ                   0x0084      //APP->BLE
#define FRM_ACCESSORY_OTA_END_RESP                  0x0084      //BLE->APP

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
 * @brief Function for handle the app to read the accessory information.
 *
 * @details    Internal use of tuya ble sdk
 *
 * @param[in] *recv_data: *recv_data
 * @param[in] recv_len: recv_len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_accessory_info_reading(UINT8_T *recv_data, UINT16_T recv_len);

/**
 * @brief This function is the application layer interface for reporting accessory information.
 *
 * @details    Internal use of tuya ble sdk
 *
 * @param[in] *p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_accessory_info_report(UINT8_T *p_data, UINT32_T len);

/**
 * @brief Function for handle accessory info report response.
 *
 * @details    Internal use of tuya ble sdk
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_accessory_info_report_response(UINT8_T *recv_data, UINT16_T recv_len);

/**
 * @brief Function for handle accessory active info received.
 *
 * @details    Internal use of tuya ble sdk
 *
 * @param[in] *recv_data: *recv_data
 * @param[in] recv_len: recv_len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_accessory_active_info_received(UINT8_T *recv_data, UINT16_T recv_len);

/**
 * @brief Function for handle accessory ota data received.
 *
 * @details    Internal use of tuya ble sdk
 *
 * @param[in] cmd: cmd
 * @param[in] UINT8_T*recv_data: UINT8_T*recv_data
 * @param[in] recv_len: recv_len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_accessory_ota_req(UINT16_T cmd, UINT8_T *recv_data, UINT32_T recv_len);


#ifdef __cplusplus
}
#endif

#endif /* __TUYA_BLE_FEATURE_ACCESSORY_H__ */

