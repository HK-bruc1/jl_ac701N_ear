/**
 * @file tal_ble_file.h
 * @brief This is tal_ble_file file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TAL_BLE_FILE_H__
#define __TAL_BLE_FILE_H__

#include "tuya_cloud_types.h"
#include "tuya_error_code.h"
#include "tuya_ble_type.h"

#include "tal_ble_md5.h"
#ifdef __cplusplus
extern "C" {
#endif
#if TUYA_BLE_FILE_ENABLE

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
/**@brief   Macro for defining file transfer protocol. */
#define FRM_FILE_INFOR_REQ          0x0070 //APP->BLE
#define FRM_FILE_INFOR_RESP         0x0070 //BLE->APP
#define FRM_FILE_OFFSET_REQ         0x0071 //APP->BLE
#define FRM_FILE_OFFSET_RESP        0x0071 //BLE->APP
#define FRM_FILE_DATA_REQ           0x0072 //APP->BLE
#define FRM_FILE_DATA_RESP          0x0072 //BLE->APP
#define FRM_FILE_END_REQ            0x0073 //APP->BLE
#define FRM_FILE_END_RESP           0x0073 //BLE->APP

#define TUYA_BLE_FILE_PKT_MAX_LEN           (512)       // the max length of the every sub package.
#define TUYA_BLE_FILE_MD5_LEN               (16)        // the length of MD5.
#define TUYA_BLE_FILE_STORAGE_HEAD_FLAG     (0xA55A)    // the storage flag of history file.
#define TUYA_BLE_FILE_RSP_MAX_LEN           (50)        // the max length of response package.
#define TUYA_BLE_MD5_LEN    (16)

typedef enum {
    FILE_STEP_NONE = 0,
    FILE_STEP_INFO,
    FILE_STEP_OFFSET,
    FILE_STEP_DATA,
    FILE_STEP_END,
} TAL_BLE_FILE_DATA_STEP_E;

typedef enum {
    FILE_VERSION_INIT = 0,
    FILE_VERSION_HIGH,
} TAL_BLE_FILE_VERSION_FLAG_E;

typedef enum {
    FILE_RSP_SUCCESS  = 0x00,
    FILE_RSP_ERR_WITHOUT,
    FILE_RSP_ERR_VERSION,
    FILE_RSP_ERR_SIZE,
    FILE_RSP_ERR_UNKNOWN,          // other file info errors
    FILE_RSP_ALREADY_EXISTS,       // file already exist
} TAL_BLE_FILE_INFO_STATUS_E;

typedef enum {
    FILE_RSP_DATA_SUCCESS  = 0x00,
    FILE_RSP_DATA_ERR_PKT,
    FILE_RSP_DATA_ERR_LENRTH,
    FILE_RSP_DATA_ERR_CRC,
    FILE_RSP_DATA_ERR_WFLASH,
    FILE_RSP_DATA_ERR_UNKNOWN,          // other file data errors
} TAL_BLE_FILE_DATA_STATUS_E;

typedef enum {
    FILE_RSP_END_SUCCESS  = 0x00,
    FILE_RSP_END_ERR_TOTAL_LENGTH,
    FILE_RSP_END_ERR_MD5,
    FILE_RSP_END_ERR_UNKNOWN,          // other file end errors
} TAL_BLE_FILE_END_STATUS_E;

typedef enum {
    FILE_MD5_RET_INIT = 0,
    FILE_MD5_RET_UPDATE,
    FILE_MD5_RET_RESULT,
} TAL_BLE_FILE_MD5_STATUS_E;

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
#pragma pack(1)
typedef struct {
    UINT8_T  type;
    UINT16_T file_id;
    UINT8_T  id_len;
    UINT8_T *id_buf;
    UINT32_T file_ver;
    UINT32_T file_len;
    UINT8_T  file_md5[TUYA_BLE_FILE_MD5_LEN];

    UINT32_T data_len;
    UINT16_T pkt_id;
    UINT16_T pkt_len;
    UINT16_T pkt_crc16;
} TAL_BLE_FILE_INFO_T;

typedef struct {
    UINT8_T  type;
    UINT16_T file_id;
    UINT8_T  status;
    UINT16_T pkt_maxlen;
    UINT32_T stored_len;
    UINT8_T  stored_md5[TUYA_BLE_FILE_MD5_LEN];
} TAL_BLE_FILE_INFO_RSP_T;

typedef struct {
    UINT8_T  type;
    UINT16_T file_id;
    UINT32_T offset;
} TAL_BLE_FILE_OFFSET_REQ_T;

typedef struct {
    UINT8_T  type;
    UINT16_T file_id;
    UINT32_T offset;
} TAL_BLE_FILE_OFFSET_REP_T;

typedef struct {
    UINT8_T  type;
    UINT16_T file_id;
    UINT16_T pkt_id;
    UINT16_T pkt_len;
    UINT16_T pkt_crc16;
    UINT8_T *data;
} TAL_BLE_FILE_DATA_REQ_T;

typedef struct {
    UINT8_T  type;
    UINT16_T file_id;
    UINT8_T  status;
} TAL_BLE_FILE_DATA_RSP_T;

typedef struct {
    UINT8_T  type;
    UINT16_T file_id;
} TAL_BLE_FILE_END_REQ_T;

typedef struct {
    UINT8_T  type;
    UINT16_T file_id;
    UINT8_T  status;
} TAL_BLE_FILE_END_RSP_T;

typedef struct {
    UINT32_T file_addr; /*file store next addr ,cur_file_addr = (file_addr -(file_len/0x1000+1)*0x1000*/
    UINT8_T  type;      /*cur file info */
    UINT16_T file_id;
    UINT32_T file_ver;
    UINT32_T file_len;
    UINT8_T  file_md5[TUYA_BLE_MD5_LEN];
    UINT8_T  id_len;
    UINT8_T  *id_buf;
} TAL_BLE_FILE_INFO_DATA_T;

typedef struct {
    mbedtls_md5_context ctx_storage; //md5 loop cac .
    UINT16_T cur_file_id; //operation data .
} TAL_BLE_FILE_MD5_INFO_T;
#pragma pack()

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/

/**@brief   Function for tuya ble handle file request.
 * @param[in] cmd           Request command.
 * @param[in] recv_data     Pointer to the buffer with received data.
 * @param[in] recv_len      Length of received data.
 */
VOID_T tuya_ble_handle_file_req(UINT16_T cmd, UINT8_T *recv_data, UINT32_T recv_len);

/**@brief   Function for respond to app requests.
 * @param[in] p_data   The pointer to the response data.
 */
tuya_ble_status_t tuya_ble_file_response(tuya_ble_file_response_t *p_data);

#endif


#ifdef __cplusplus
}
#endif

#endif /* __TAL_BLE_FILE_H__ */

