/**
 * @file tuya_ble_scene.h
 * @brief This is tuya_ble_scene file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_SCENE_H__
#define __TUYA_BLE_SCENE_H__

#include "tuya_cloud_types.h"
#include "tuya_ble_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define SCENE_ID_MAX_LENGTH             ( 24 )

#define SUPPORT_REQUEST_SCENE_MAX_NUMS  ( 100 )

/**@brief    Request scene type. */
typedef enum {
    REQUEST_SCENE_DATA = 1, /**< request scene list data. */
    REQUEST_SCENE_CONTROL,  /**< request scene control. */

    REQUEST_SCENE_CMD_MAX,
} request_scene_cmd;

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
/**@brief    Request scene list data structure. */
typedef struct {
    UINT8_T nums;                   /**< request scene numbers, rang from [1-SUPPORT_REQUEST_SCENE_MAX_NUMS]. */
    UINT16_T name_unicode_length;   /**< request scene name length, The encoding format is unicode, It must be a multiple of 2. */
    UINT32_T check_code;            /**< scene check code, algorithm-crc32. */
} request_scene_list_data_t;

/**@brief    Request scene control structure. */
typedef struct {
    UINT8_T scene_id_length;                /**< scene id length. */
    UINT8_T scene_id[SCENE_ID_MAX_LENGTH];  /**< scene id, Max length equal to SCENE_ID_MAX_LENGTH. */
} request_scene_control_t;

/**@brief    Request scene cmd structure. */
typedef struct {
    request_scene_cmd scene_cmd;                    /**< request scene cmd. */

    union {
        request_scene_list_data_t   scene_data;     /**< request scene list data info. */
        request_scene_control_t     scene_control;  /**< request scene ctrl info. */
    };
} tuya_ble_request_scene_t;

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/

/**
 * @brief Function for request scene
 *
 * @param[in] req_data: For details, see the struct of tuya_ble_request_scene_t
 *
 * @return TUYA_BLE_SUCCESS on success. Others on error, please refer to tuya_ble_status_t
 */
tuya_ble_status_t tuya_ble_feature_scene_request(tuya_ble_request_scene_t *req_data);

/**
 * @brief Function for handler scene request response
 *
 * @note Internal use of tuya ble sdk
 *
 * @param[in] recv_data: The point of received response data
 * @param[in] recv_len: The numbers of data
 *
 */
VOID_T tuya_ble_handle_scene_request_response(UINT8_T *recv_data, UINT16_T recv_len);

/**
 * @brief Function for handler scene data received
 *
 * @note Internal use of tuya ble sdk
 *
 * @param[in] recv_data: The point of recvived scene data
 * @param[in] recv_len: The length of data
 *
 */
VOID_T tuya_ble_handle_scene_data_received(UINT8_T *p_data, UINT16_T data_len);

/**
 * @brief How to use tuya_ble_scene component function, steps are:
 *
 * Step1: Add tuya_ble_iot_channel.c and tuya_ble_scene.c file into project.
 *
 * Step2: Define TUYA_BLE_FEATURE_IOT_CHANNEL_ENABLE and TUYA_BLE_FEATURE_SCENE_ENABLE macro in the <custom_tuya_ble_config.h> file, such as:
 *
 * @code
        // If it is 1, the sdk support common iot data channel
        #define TUYA_BLE_FEATURE_IOT_CHANNEL_ENABLE        ( 1 )

        #if ( TUYA_BLE_FEATURE_IOT_CHANNEL_ENABLE != 0 )
        // If it is 1, the sdk support scene list data request and control
        #define TUYA_BLE_FEATURE_SCENE_ENABLE            ( 1 )
        #endif
 * @endcode
 *
 * Step3: In tuya_cb_handler() function add callback events, The following is the code how to process the callback events.
 *
 * @code
        case TUYA_BLE_CB_EVT_SCENE_REQ_RESPONSE:
            TUYA_APP_LOG_INFO("received scene request response result code=[%d], err_code=[%d] sub_cmd=[%d]", \
                                event->scene_req_response_data.status, \
                                event->scene_req_response_data.err_code, \
                                event->scene_req_response_data.scene_cmd);
            break;

        case TUYA_BLE_CB_EVT_SCENE_DATA_RECEIVED:
            TUYA_APP_LOG_INFO("received scene data result, status=[%d] err_code=[%d] need_update=[%d] check_code=[0x%X]", \
                                event->scene_data_received_data.status, \
                                event->scene_data_received_data.err_code, \
                                event->scene_data_received_data.need_update, \
                                event->scene_data_received_data.check_code);

            if (event->scene_data_received_data.status == 0 && event->scene_data_received_data.need_update) {
                // TODO .. update scene check code.

                if (event->scene_data_received_data.data_len != 0) {
                    UINT8_T *iot_scene_object;
                    UINT16_T scene_id_len, scene_name_len;
                    UINT16_T object_len = 0;

                    for (;;) {
                        iot_scene_object = (UINT8_T *)(event->scene_data_received_data.p_data + object_len);

                        scene_id_len = iot_scene_object[0];
                        scene_name_len = (iot_scene_object[1+scene_id_len] << 8) + iot_scene_object[2+scene_id_len];

                        TUYA_APP_LOG_HEXDUMP_DEBUG("scene id :", &iot_scene_object[1], scene_id_len);
                        TUYA_APP_LOG_HEXDUMP_DEBUG("scene name unicode :", &iot_scene_object[3+scene_id_len], scene_name_len);

                        object_len += (3 + scene_id_len + scene_name_len);
                        if (object_len >= event->scene_data_received_data.data_len)
                            break;
                    }
                }
            }
            break;

        case TUYA_BLE_CB_EVT_SCENE_CTRL_RESULT_RECEIVED:
            TUYA_APP_LOG_INFO("received scene ctrl result, status=[%d] err_code=[%d]", \
                                event->scene_ctrl_received_data.status, \
                                event->scene_ctrl_received_data.err_code);
            TUYA_APP_LOG_HEXDUMP_DEBUG("scene ctrl id :", event->scene_ctrl_received_data.p_scene_id, event->scene_ctrl_received_data.scene_id_len);
            break;
 * @endcode
 *
 * Step4: Developers could call tuya_ble_feature_scene_request(tuya_ble_request_scene_t *req_data) api to request scene data,
 *          input parameter see the struct of tuya_ble_request_scene_t for details. For example request scene list data:
 *
 * @code
        tuya_ble_request_scene_t req_scene_data;

        req_scene_data.scene_cmd = REQUEST_SCENE_DATA;
        req_scene_data.scene_data.nums = 1;
        req_scene_data.scene_data.name_unicode_length = 10;
        req_scene_data.scene_data.check_code = 0;

        tuya_ble_feature_scene_request(&req_scene_data);
 * @endcode
 *
 */


#ifdef __cplusplus
}
#endif

#endif /* __TUYA_BLE_SCENE_H__ */

