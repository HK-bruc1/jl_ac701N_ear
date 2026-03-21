/**
 * @file tuya_ble_iot_channel.h
 * @brief This is tuya_ble_iot_channel file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_IOT_CHANNEL_H__
#define __TUYA_BLE_IOT_CHANNEL_H__

#include "tuya_cloud_types.h"
#include "tuya_ble_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
/**@brief   Request iot data sub cmd. */
typedef enum {
    IOT_SUBCMD_REQ_SCENE_DATA = 1,  /**< request scene data. */
    IOT_SUBCMD_REQ_SCENE_CONTROL,   /**< request scene control. */

    IOT_SUBCMD_MAX,
} request_iot_data_subcmd;

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
/**@brief   iot data format-TLD structure. */
typedef struct {
    UINT8_T type;
    UINT16_T length;
    UINT8_T *data;
} tuya_ble_iot_data_tld_t;

/**@brief   iot data received packet structure. */
typedef struct {
    UINT32_T recv_len;
    UINT32_T recv_len_max;
    UINT8_T  *recv_data;
} tuya_ble_iot_data_recv_packet;

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/

/**
 * @brief Function for get the iot channel subpackage transmission max size.
 *
 * @note The size depends on the mtu value negotiated after the BLE connection is established.
 *       If the return value is 0, means dev is unbond.
 *
 * @return subpackage size used for iot channel data transmission.
 */
UINT32_T tuya_ble_iot_channel_subpackage_maxsize_get(VOID_T);

/**
 * @brief Function for handler iot data request response
 *
 * @note Internal use of tuya ble sdk
 *
 * @param[in] recv_data: The point of received response data
 * @param[in] recv_len: The numbers of data
 *
 */
VOID_T tuya_ble_handle_iot_data_request_response(UINT8_T *recv_data, UINT16_T recv_len);

/**
 * @brief Function for handler iot data received
 *
 * @note Internal use of tuya ble sdk
 *
 * @param[in] recv_data: The point of received iot data
 * @param[in] recv_len: The numbers of data
 *
 */
VOID_T tuya_ble_handle_iot_data_received(UINT8_T *recv_data, UINT16_T recv_len);

/**
 * @brief Function for free received iot subpackage data cache
 *
 * @note Internal use of tuya ble sdk
 *
 */
VOID_T tuya_ble_iot_data_recv_packet_free(VOID_T);


#ifdef __cplusplus
}
#endif

#endif /* __TUYA_BLE_IOT_CHANNEL_H__ */

