/**
 * @file tal_ble_x_demo.h
 * @brief This is tal_ble_x_demo file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TAL_BLE_X_DEMO_H__
#define __TAL_BLE_X_DEMO_H__

#include "tuya_cloud_types.h"
#include "tuya_ble_type.h"
#include "tuya_error_code.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
/**@brief    ext module type. */
typedef enum {
    EM_TYPE_NB = 1,
    EM_TYPE_WIFI,
    EM_TYPE_CAT,
    EM_TYPE_ZIGBEE,

    EM_TYPE_MAX,
} ENUM_EM_TYPE;

/**@brief    ext module tld type. */
typedef enum {
    TLD_TYPE_EM_BASIC_INFO = 1,
    TLD_TYPE_EM_INSERT_STATE,
    TLD_TYPE_EM_COMMUNICATE_PRIORITY,
    TLD_TYPE_EM_OTA_PRIORITY,
    TLD_TYPE_EM_BINDING_STATE,
    TLD_TYPE_EM_OTA_CHANNLE_VERSION,
    TLD_TYPE_EM_REPORT_STATE,

    TLD_TYPE_MAX,
} ENUM_EM_TLD_TYPE;

/**@brief    communication priority. */
typedef enum {
    LAN = 0,
    MQTT,
    HTTP,
    BLE,
    SIGMESH,
    TUYA_MESH,
    BEACON,

    COMMUNICATE_PRIORITY_MAX,
} ENUM_COMMUNICATE_PRIORITY;

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
/**@brief    The struct of report extern module info. */
typedef struct {
    /* include emt info, 0-include 1-incude */
    BOOL_T inc_em_info;

    /* ext module type */
    ENUM_EM_TYPE em_type;

    /* TLD - ext module hardware insert state. 0-pop 1-insert */
    BOOL_T em_insert_state;

    /* TLD - ext module binding state. 0-unbinding 1-binding */
    BOOL_T em_binding_state;

    /* TLD - info report state. 0-app query 1-dev active report */
    BOOL_T active_report_state;

    /* TLD - ext module dev basic info, only ble+nb need this field, others set to NULL or zore */
    CHAR_T *em_dev_basic_info;
    UINT16_T em_dev_basic_info_len;

    /* TLD - communication priortity, such as ble > mqtt, For details see ENUM_COMMUNICATE_PRIORITY */
    UINT8_T communication_priority[COMMUNICATE_PRIORITY_MAX];
    UINT8_T communication_priority_len;
} ext_module_info_t;

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/
/**
 * @brief   Function for handling report ext module dev info.
 *
 * @note    The application must call this function where it receives the @ref TUYA_BLE_CB_EVT_QUERY_EXT_MODULE_DEV_INFO event.
 *
 * @param[in] p_em_info     Report ext module info, For details see the struct of ext_module_info_t
 *
 */
tuya_ble_status_t tuya_ble_ext_module_info_report(ext_module_info_t *p_data);

/**
 * @brief   The complete method for handling report ext module dev info.
 *
 * Equipment form:      extern module type, see the enum of ENUM_EM_TYPE
 * EM hardware state:   insert state
 * EM active state:     binding state
 * Active report:       0-app query report, 1-dev active report
 * EM dev info:         ext module basic info
 * EM dev info len:     The length of ext module basic info
 * communication_priority: The point of communication priority
 * priority_group_len:  The length of communication priority
 */
tuya_ble_status_t tuya_ble_ext_module_info_report_custom_all(ENUM_EM_TYPE emt, BOOL_T insert_state, BOOL_T binding_state, BOOL_T active_report_state, VOID_T *basic_info, UINT16_T basic_info_len, VOID_T *communication_priority, UINT8_T priority_group_len);

/**
 * @brief   The lightweight method for handling report ext module dev info
 *
 * Equipment form:      extern module type, see the enum of ENUM_EM_TYPE
 * EM dev info:         NULL [unwanted]
 * EM hardware state:   insert state  0-pop, 1-insert
 * EM active state:     binding state 0-unbinding, 1-binding
 * Active report:       0-app query report, 1-dev active report
 *
 */
tuya_ble_status_t tuya_ble_ext_module_info_report_custom(ENUM_EM_TYPE emt, BOOL_T insert_state, BOOL_T binding_state, BOOL_T active_report_state);

/**
 * @brief  Function for some examples
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_ext_module_info_report_example_1(VOID_T);

/**
 * @brief tuya_ble_ext_module_info_report_example_2
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_ext_module_info_report_example_2(VOID_T);

/**
 * @brief tuya_ble_ext_module_info_report_example_3
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_ext_module_info_report_example_3(VOID_T);

/**
 * @brief tuya_ble_ext_module_info_report_example_4
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_ext_module_info_report_example_4(VOID_T);


#ifdef __cplusplus
}
#endif

#endif /* __TAL_BLE_X_DEMO_H__ */

