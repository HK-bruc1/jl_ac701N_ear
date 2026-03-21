/**
 * @file tal_feature_ext_module.h
 * @brief This is tal_feature_ext_module file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TAL_FEATURE_EXT_MODULE_H__
#define __TAL_FEATURE_EXT_MODULE_H__

#include "tuya_cloud_types.h"
#include "tuya_ble_type.h"
#include "tuya_error_code.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/


/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
/**@brief    ext module active data received packet structure. */
typedef struct {
    UINT32_T recv_len;
    UINT32_T recv_len_max;
    UINT8_T  *recv_data;
} tuya_ble_em_active_data_recv_packet;

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/
/**
 * @brief   Function for asynchronous response extern module dev info
 *
 * @details
 *
* @param[in] p_data     The point of ext module dev info. The data format is as follows:
 *  --------------------------------------------------------------------------------------------------------
 * |         1byte         |      1byte      |    1byte     |    1+2+n bytes    |   ...     |   1+2+n bytes |
 * |     Include_EM_Info   |     EM_Type     |    N_TLD     |       TLD1        |   ...     |     TLDn      |
 *  --------------------------------------------------------------------------------------------------------
 *  Include_EM_Info:
 *          0x01 - include em info
 *
 *  EM_Type:
 *          0x01 - NB module
 *          0x02 - WiFi module
 *          0x03 - Cat.x module
 *          0x04 - zigbee module
 *
 *  N_TLD: the number of subsequent TLDs
 *
 *  TLD: type[1byte] + length[2bytes] + data[nbytes]
 *  ---------------------------------------------
 *  |         TYPE=1 em dev info                |
 *  ---------------------------------------------
 *  |   type    |   length  |      data         |
 *  |   0x01    |     N     |   dev info json   |
 *  ---------------------------------------------
 *
 *  ---------------------------------------------
 *  |         TYPE=2 em dev insert state        |
 *  ---------------------------------------------
 *  |   type    |   length  |         data      |
 *  |   0x02    |   0x01    |  0-pop; 1-insert  |
 *  ---------------------------------------------
 *
 *  ---------------------------------------------
 *  |         TYPE=3 communicate priority       |
 *  ---------------------------------------------
 *  |   type    |   length  |       data        |
 *  |           |           |      LAN=0        |
 *  |           |           |      MQTT=1       |
 *  |           |           |      HTTP=2       |
 *  |   0x03    |    0x02   |      BLE=3        |
 *  |           |           |      SIGMESH=4    |
 *  |           |           |      TUYAMESH=5   |
 *  |           |           |      BEACON=6     |
 *  ---------------------------------------------
 *
 *  ---------------------------------------------
 *  |         TYPE=5 em dev binding state       |
 *  ---------------------------------------------
 *  |   type    |   length  |         data      |
 *  |           |           |    0-unbinding    |
 *  |   0x05    |   0x01    |    1-binding      |
 *  ---------------------------------------------
 *
 * @param[in] len       The length of ext module dev info
 *
 * */

/**
 * @brief tuya_ble_ext_module_info_asynchronous_response
 *
 * @param[in] *p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_ext_module_info_asynchronous_response(UINT8_T *p_data, UINT32_T len);

/**
 * @brief   Function for handler app query extern module dev info when connected.
 *
 * @details Internal use of tuya ble sdk
 *
 * @param[in] recv_data     The point of recvived scene data
 * @param[in] recv_len      The length of data
 *
 * */

/**
 * @brief tuya_ble_handle_ext_module_dev_info_query_req
 *
 * @param[in] *recv_data: *recv_data
 * @param[in] recv_len: recv_len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_ext_module_dev_info_query_req(UINT8_T *recv_data, UINT16_T recv_len);

/**
 * @brief   Function for free received iot subpackage data cache
 *
 * @details Internal use of tuya ble sdk
 *
 * */

/**
 * @brief tuya_ble_em_active_data_recv_packet_free
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_em_active_data_recv_packet_free(VOID_T);

/**
 * @brief   Function for handler extern module active data received
 *
 * @details Internal use of tuya ble sdk
 *
 * @param[in] recv_data     The point of received data
 * @param[in] recv_len      The length of data
 *
 * */

/**
 * @brief tuya_ble_handle_ext_module_active_data_received
 *
 * @param[in] *recv_data: *recv_data
 * @param[in] recv_len: recv_len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tuya_ble_handle_ext_module_active_data_received(UINT8_T *recv_data, UINT16_T recv_len);


#ifdef __cplusplus
}
#endif

#endif /* __TAL_FEATURE_EXT_MODULE_H__ */

