/**
 * @file tal_repeater.h
 * @brief This is tal_repeater file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TAL_REPEATER_H__
#define __TAL_REPEATER_H__

#include "tuya_cloud_types.h"
#include "tuya_error_code.h"
#include "tuya_ble_type.h"
#include "tkl_bluetooth_def.h"
#include "tal_bluetooth_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (TUYA_BLE_FEATURE_REPEATER_ENABLE != 0)

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define  TAL_REPEATER_SLAVE_MAX_NUM             4
#define  TAL_REPEATER_ADV_MAX_NUM               20
#define  TAL_REPEATER_DATA_LEN_MAX              1500

#define  TAL_REPEATER_CMD_SET_SCAN              0x00
#define  TAL_REPEATER_CMD_CONN                  0x01
#define  TAL_REPEATER_CMD_DISCONN               0x02
#define  TAL_REPEATER_CMD_RX_DATA               0x03
#define  TAL_REPEATER_CMD_GET_RSSI              0x04
#define  TAL_REPEATER_CMD_GET_INFO              0x05
#define  TAL_REPEATER_CMD_REPORT_ADV            0x80
#define  TAL_REPEATER_CMD_REPORT_CONN_STATE     0x81
#define  TAL_REPEATER_CMD_REPORT_MTU            0x82
#define  TAL_REPEATER_CMD_TX_DATA               0x83

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
 * @brief tal_repeater_init
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_repeater_init(void);

/**
 * @brief tal_repeater_get_rssi_report_handler
 *
 * @param[in] link_rssi: link_rssi
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_repeater_get_rssi_report_handler(TAL_BLE_CONN_RSSI_EVT_T link_rssi);

/**
 * @brief tal_repeater_adv_report_handler
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_repeater_adv_report_handler(void);

/**
 * @brief tal_repeater_conn_success_report_handler
 *
 * @param[in] connect: connect
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_repeater_conn_success_report_handler(TAL_BLE_CONNECT_EVT_T *connect);

/**
 * @brief tal_repeater_conn_success_handler
 *
 * @param[in] connect: connect
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_repeater_gateway_conn_success_handler(TAL_BLE_CONNECT_EVT_T *connect);

/**
 * @brief tal_repeater_disconn_report_handler
 *
 * @param[in] disconnect: disconnect
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_repeater_disconn_report_handler(TAL_BLE_DISCONNECT_EVT_T *disconnect);

/**
 * @brief tal_repeater_disconn_handler
 *
 * @param[in] disconnect: disconnect
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_repeater_gateway_disconn_handler(TAL_BLE_DISCONNECT_EVT_T *disconnect);

/**
 * @brief tal_repeater_conn_failed_report_handler
 *
 * @param[in] peer_addr: peer_addr
 * @param[in] status: status
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_repeater_conn_failed_report_handler(TAL_BLE_ADDR_T *peer_addr, uint8_t status);

/**
 * @brief tal_repeater_mtu_report_handler
 *
 * @param[in] mtu_evt: mtu_evt
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_repeater_mtu_report_handler(TAL_BLE_EXCHANGE_MTU_EVT_T *mtu_evt);

/**
 * @brief tal_repeater_data_report_handler
 *
 * @param[in] *data: *data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_repeater_data_report_handler(TAL_BLE_DATA_REPORT_T *data);

/**
 * @brief tal_repeater_handler
 *
 * @param[in] data: data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_repeater_handler(tuya_ble_app_passthrough_data_t *data);

/**
 * @brief tal_repeater_adv_report_cb
 *
 * @param[in] adv_report: adv_report
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_repeater_adv_report_cb(TAL_BLE_ADV_REPORT_T *adv_report);

/**
 * @brief tal_repeater_slave_info_add
 *
 * @param[in] connect: connect
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_repeater_slave_info_add(TAL_BLE_CONNECT_EVT_T *connect);

/**
 * @brief tal_repeater_slave_info_delete
 *
 * @param[in] disconnect: disconnect
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_repeater_slave_info_delete(TAL_BLE_DISCONNECT_EVT_T *disconnect);

/**
 * @brief tal_repeater_slave_info_get_connhandle_from_mac
 *
 * @param[in] peer_addr: peer_addr
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
uint16_t tal_repeater_slave_info_get_connhandle_from_mac(TAL_BLE_ADDR_T peer_addr);

/**
 * @brief tal_repeater_slave_info_get_charhandle_from_mac
 *
 * @param[in] peer_addr: peer_addr
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
uint16_t tal_repeater_slave_info_get_charhandle_from_mac(TAL_BLE_ADDR_T peer_addr);

/**
 * @brief tal_repeater_slave_info_get_mac_from_connhandle
 *
 * @param[in] conn_handle: conn_handle
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
TAL_BLE_ADDR_T tal_repeater_slave_info_get_mac_from_connhandle(uint16_t conn_handle);

/**
 * @brief tal_repeater_slave_info_set_mtu
 *
 * @param[in] peer_addr: peer_addr
 * @param[in] mtu: mtu
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_repeater_slave_info_set_mtu(TAL_BLE_ADDR_T peer_addr, uint16_t mtu);

/**
 * @brief tal_repeater_slave_info_get_mtu
 *
 * @param[in] peer_addr: peer_addr
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
uint16_t tal_repeater_slave_info_get_mtu(TAL_BLE_ADDR_T peer_addr);

/**
 * @brief tal_repeater_slave_info_count
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
uint32_t tal_repeater_slave_info_count(void);

/**
 * @brief tal_repeater_slave_info_disconnect_all
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_repeater_slave_info_disconnect_all(void);

/**
 * @brief tal_repeater_slave_info_get_valid
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
uint32_t tal_repeater_slave_info_get_valid(void);

/**
 * @brief tal_repeater_slave_info_list
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_repeater_slave_info_list(void);

#endif // TUYA_BLE_FEATURE_REPEATER_ENABLE


#ifdef __cplusplus
}
#endif

#endif /* __TAL_REPEATER_H__ */

