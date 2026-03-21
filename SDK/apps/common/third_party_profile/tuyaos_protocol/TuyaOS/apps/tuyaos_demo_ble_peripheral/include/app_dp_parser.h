/**
 * @file app_dp_parser.h
 * @brief This is app_dp_parser file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __APP_DP_PARSER_H__
#define __APP_DP_PARSER_H__

#include "tuya_cloud_types.h"
#include "tuya_app_func.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
//WR-write_report, OW-only_write, OR-only_report
#define  WR_BASIC_LED                   101
#define  WR_BASIC_CHARGE_STATE          102
#define  WR_BASIC_TEMPERATURE           103
#define  WR_BASIC_WELCOME               104
#define  WR_BASIC_CUSTOM_DATA           105
#define  WR_BASIC_FAULT_ALARM           106

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
#pragma pack(1)
typedef struct {
    UINT8_T  dp_id;
    UINT8_T  dp_type;
    UINT16_T dp_data_len;
    UINT8_T  dp_data[600];
} demo_dp_t;
#pragma pack()

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
extern demo_dp_t g_cmd;
extern demo_dp_t g_rsp;
extern UINT32_T  g_sn;

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/

/**
 * @brief
 *
 * @param[in] param1:
 * @param[in] param2:
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_dp_parser(UINT8_T *buf, UINT32_T size);

/**
 * @brief app_dp_report
 *
 * @param[in] dp_id: dp_id
 * @param[in] buf: buf
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_dp_report(UINT8_T dp_id, UINT8_T *buf, UINT32_T size);



#ifdef __cplusplus
}
#endif

#endif /* __APP_DP_PARSER_H__ */

