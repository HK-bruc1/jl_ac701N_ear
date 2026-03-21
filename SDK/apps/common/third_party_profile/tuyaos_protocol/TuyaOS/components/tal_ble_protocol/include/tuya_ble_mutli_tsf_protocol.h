/**
 * @file tuya_ble_mutli_tsf_protocol.h
 * @brief This is tuya_ble_mutli_tsf_protocol file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_MUTLI_TSF_PROTOCOL_H__
#define __TUYA_BLE_MUTLI_TSF_PROTOCOL_H__

#include "tuya_cloud_types.h"
#include "tuya_ble_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#ifdef  __MUTLI_TSF_PROTOCOL_GLOBALS
#define __MUTLI_TSF_PROTOCOL_EXT
#else
#define __MUTLI_TSF_PROTOCOL_EXT extern
#endif

#define SNGL_PKG_TRSFR_LMT  TUYA_BLE_DATA_MTU_MAX // single package transfer limit

//#define FRM_TYPE_OFFSET (0x0f << 4)
#define FRM_VERSION_OFFSET (0x0f << 4)
#define FRM_SEQ_OFFSET  (0x0f << 0)

#define FRAME_SEQ_LMT 16

#define FRM_PKG_INIT 0   // frame package init
#define FRM_PKG_FIRST 1  // frame package first
#define FRM_PKG_MIDDLE 2 // frame package middle
#define FRM_PKG_END 3    // frame package end

#define MTP_OK  0
#define MTP_INVALID_PARAM 1
#define MTP_COM_ERROR 2
#define MTP_TRSMITR_CONTINUE 3
#define MTP_TRSMITR_ERROR 4
#define MTP_MALLOC_ERR 5

#define DT_RAW     0
#define DT_BOOL    1
#define DT_VALUE   2
#define DT_INT     DT_VALUE
#define DT_STRING  3
#define DT_ENUM    4
#define DT_BITMAP  5
#define DT_CHAR    7       //Not currently supported
#define DT_UCHAR   8       //Not currently supported
#define DT_SHORT   9       //Not currently supported
#define DT_USHORT  10      //Not currently supported
#define DT_LMT    DT_USHORT

#define DT_VALUE_LEN 4 // int
#define DT_BOOL_LEN 1
#define DT_ENUM_LEN 1
#define DT_BITMAP_MAX 4 // 1/2/4
#define DT_STR_MAX 255
#define DT_RAW_MAX 255
#define DT_INT_LEN DT_VALUE_LEN

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
// frame total len
typedef UINT32_T frame_total_t;

// frame subpackage num
typedef UINT32_T frame_subpkg_num_t;

// frame sequence
typedef UINT8_T frame_seq_t;

// frame subpackage len
typedef UINT16_T frame_subpkg_len_t;

// frame package description
typedef UINT8_T frm_pkg_desc_t;

// mutil tsf ret code
typedef INT32_T mtp_ret;

// dp type description,please refer to the relevant help documents of Tuya iot platform for the usage of various types of dp points.
typedef UINT8_T dp_type;

// frame transmitter process
typedef struct {
    frame_total_t  total;         //4 bytes, total length of data, not including header
    UINT8_T  version;             //1 byte, protocol major version number
    frame_seq_t    seq;           //1 byte
    frm_pkg_desc_t pkg_desc;      //1 byte, Current packet frame type (init/first/middle/end)
    frame_subpkg_num_t subpkg_num; //4 bytes, current sub-packet number
    UINT32_T pkg_trsmitr_cnt;     //  package process count ,Number of bytes sent
    frame_subpkg_len_t subpkg_len; //1 byte, the data length in the current sub-packet
    UINT8_T subpkg[SNGL_PKG_TRSFR_LMT];
} frm_trsmitr_proc_s;

typedef struct s_klv_node {
    struct s_klv_node *next;
    UINT8_T id;
    dp_type type;
    UINT16_T len;
    UINT8_T *data;
} klv_node_s;

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/

/***********************************************************
*  Function: make_klv_list
*  description:
*  Input:
*  Output:
*  Return:
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \

/**
 * @brief *make_klv_list
 *
 * @param[in] *list: *list
 * @param[in] id: id
 * @param[in] type: type
 * @param[in] *data: *data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
klv_node_s *make_klv_list(klv_node_s *list, UINT8_T id, dp_type type, VOID_T *p_data, UINT16_T len);

/***********************************************************
*  Function: free_klv_list
*  description:
*  Input: list
*  Output:
*  Return:
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \

/**
 * @brief free_klv_list
 *
 * @param[in] *list: *list
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T free_klv_list(klv_node_s *list);

/***********************************************************
*  Function: klvlist_2_data
*  description:
*  Input: list
*  Output: data len
*  Return:
*  Note:data need free.
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \

/**
 * @brief klvlist_2_data
 *
 * @param[in] *list: *list
 * @param[in] **data: **data
 * @param[in] *len: *len
 * @param[in] type: type
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
mtp_ret klvlist_2_data(klv_node_s *list, UINT8_T **data, UINT32_T *len, UINT8_T type);

/***********************************************************
*  Function: data_2_klvlist
*  description:
*  Input: data len
*  Output: list
*  Return:
*  Note: list need to call free_klv_list to free.
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \

/**
 * @brief data_2_klvlist
 *
 * @param[in] *data: *data
 * @param[in] len: len
 * @param[in] **list: **list
 * @param[in] type: type
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
mtp_ret data_2_klvlist(UINT8_T *data, UINT32_T len, klv_node_s **list, UINT8_T type);

/***********************************************************
*  Function: create_trsmitr_init
*  description: create a transmitter and initialize
*  Input: none
*  Output:
*  Return: transmitter handle
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \

/**
 * @brief *create_trsmitr_init
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
frm_trsmitr_proc_s *create_trsmitr_init(VOID_T);

/***********************************************************
*  Function: trsmitr_init
*  description: init a transmitter
*  Input: transmitter handle
*  Output:
*  Return:
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \

/**
 * @brief trsmitr_init
 *
 * @param[in] *frm_trsmitr: *frm_trsmitr
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T trsmitr_init(frm_trsmitr_proc_s *frm_trsmitr);

/***********************************************************
*  Function: delete_trsmitr
*  description: delete transmitter
*  Input: transmitter handle
*  Output:
*  Return:
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \

/**
 * @brief delete_trsmitr
 *
 * @param[in] *frm_trsmitr: *frm_trsmitr
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T delete_trsmitr(frm_trsmitr_proc_s *frm_trsmitr);

/***********************************************************
*  Function: get_trsmitr_frame_total_len
*  description: get a transmitter total data len
*  Input: transmitter handle
*  Output:
*  Return: frame_total_t
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \

/**
 * @brief get_trsmitr_frame_total_len
 *
 * @param[in] *frm_trsmitr: *frm_trsmitr
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
frame_total_t get_trsmitr_frame_total_len(frm_trsmitr_proc_s *frm_trsmitr);

/***********************************************************
*  Function: get_trsmitr_frame_type
*  description:
*  Input: transmitter handle
*  Output:
*  Return: frame_type_t
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \

/**
 * @brief get_trsmitr_frame_version
 *
 * @param[in] *frm_trsmitr: *frm_trsmitr
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T get_trsmitr_frame_version(frm_trsmitr_proc_s *frm_trsmitr);

/***********************************************************
*  Function: get_trsmitr_frame_seq
*  description:
*  Input: transmitter handle
*  Output:
*  Return: frame_seq_t
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \

/**
 * @brief get_trsmitr_frame_seq
 *
 * @param[in] *frm_trsmitr: *frm_trsmitr
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
frame_seq_t get_trsmitr_frame_seq(frm_trsmitr_proc_s *frm_trsmitr);

/***********************************************************
*  Function: get_trsmitr_subpkg_len
*  description:
*  Input: transmitter handle
*  Output:
*  Return: frame_subpkg_len_t
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \

/**
 * @brief get_trsmitr_subpkg_len
 *
 * @param[in] *frm_trsmitr: *frm_trsmitr
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
frame_subpkg_len_t get_trsmitr_subpkg_len(frm_trsmitr_proc_s *frm_trsmitr);

/***********************************************************
*  Function: get_trsmitr_subpkg
*  description:
*  Input: transmitter handle
*  Output:
*  Return: subpackage buf
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \

/**
 * @brief *get_trsmitr_subpkg
 *
 * @param[in] *frm_trsmitr: *frm_trsmitr
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T *get_trsmitr_subpkg(frm_trsmitr_proc_s *frm_trsmitr);

/***********************************************************
*  Function: trsmitr_send_pkg_encode
*  description: frm_trsmitr->transmitter handle
*               type->frame type
*               buf->data buf
*               len->data len
*  Input:
*  Output:
*  Return: MTP_OK->buf send up
*          MTP_TRSMITR_CONTINUE->need call again to be continue
*          other->error
*  Note: could get from encode data len and encode data by calling method
         get_trsmitr_subpkg_len() and get_trsmitr_subpkg()
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \

/**
 * @brief trsmitr_send_pkg_encode
 *
 * @param[in] *frm_trsmitr: *frm_trsmitr
 * @param[in] version: version
 * @param[in] *buf: *buf
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
mtp_ret trsmitr_send_pkg_encode(frm_trsmitr_proc_s *frm_trsmitr, UINT8_T version, UINT8_T *buf, UINT32_T len);

/***********************************************************
*  Function: trsmitr_send_pkg_encode_with_packet_length
*  description: Encoding function for specifying sub-packet length
*
*
*
*  Input:
*  Output:
*  Return: MTP_OK->buf send up
*          MTP_TRSMITR_CONTINUE->need call again to be continue
*          other->error
*  Note: could get from encode data len and encode data by calling method
         get_trsmitr_subpkg_len() and get_trsmitr_subpkg()
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \

/**
 * @brief trsmitr_send_pkg_encode_with_packet_length
 *
 * @param[in] *frm_trsmitr: *frm_trsmitr
 * @param[in] pkg_len_max: pkg_len_max
 * @param[in] version: version
 * @param[in] *buf: *buf
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
mtp_ret trsmitr_send_pkg_encode_with_packet_length(frm_trsmitr_proc_s *frm_trsmitr, UINT32_T pkg_len_max, UINT8_T version, UINT8_T *buf, UINT32_T len);

/***********************************************************
*  Function: trsmitr_recv_pkg_decode
*  description: frm_trsmitr->transmitter handle
*               raw_data->raw encode data
*               raw_data_len->raw encode data len
*  Input:
*  Output:
*  Return: MTP_OK->buf receive up
*          MTP_TRSMITR_CONTINUE->need call again to be continue
*          other->error
*  Note: could get from decode data len and decode data by calling method
         get_trsmitr_subpkg_len() and get_trsmitr_subpkg()
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \

/**
 * @brief trsmitr_recv_pkg_decode
 *
 * @param[in] *frm_trsmitr: *frm_trsmitr
 * @param[in] *raw_data: *raw_data
 * @param[in] raw_data_len: raw_data_len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
mtp_ret trsmitr_recv_pkg_decode(frm_trsmitr_proc_s *frm_trsmitr, UINT8_T *raw_data, UINT16_T raw_data_len);


#ifdef __cplusplus
}
#endif

#endif /* __TUYA_BLE_MUTLI_TSF_PROTOCOL_H__ */

