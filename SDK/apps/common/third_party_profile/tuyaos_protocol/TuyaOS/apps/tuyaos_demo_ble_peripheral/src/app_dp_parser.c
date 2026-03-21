/**
 * @file app_dp_parser.c
 * @brief This is app_dp_parser file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */


#include "string.h"

#include "tal_log.h"
#include "tal_util.h"

#include "tuya_ble_api.h"
#include "tuya_ble_mutli_tsf_protocol.h"

#include "app_dp_parser.h"
#include "app_led.h"

#include "app_msg.h"
#include "bt_tws.h"
#include "key_event_deal.h"
#include "user_cfg.h"
#include "effects/audio_eq.h"
#include "audio_config.h"
#include "avctp_user.h"
#include "app_chargestore.h"
#include "app_power_manage.h"
#include "app_main.h"


/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
demo_dp_t g_cmd = {0};
demo_dp_t g_rsp = {0};
UINT32_T  g_sn  = 0;

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/

OPERATE_RET app_dp_parser(UINT8_T *buf, UINT32_T size)
{
    memcpy(&g_cmd, buf, size);
    tal_util_reverse_byte(&g_cmd.dp_data_len, SIZEOF(UINT16_T));
    memcpy(&g_rsp, &g_cmd, size);

    TAL_PR_HEXDUMP_INFO("dp_cmd", (VOID_T *)&g_cmd, (g_cmd.dp_data_len + 4));

    switch (g_cmd.dp_id) {
    case WR_BASIC_LED: {
    } break;

    case WR_BASIC_CHARGE_STATE: {
    } break;

    case WR_BASIC_TEMPERATURE: {
    } break;

    case WR_BASIC_WELCOME: {
    } break;

    case WR_BASIC_CUSTOM_DATA: {
    } break;

    default: {
    } break;
    }

    app_dp_report(g_cmd.dp_id, g_cmd.dp_data, g_cmd.dp_data_len);

    return OPRT_OK;
}

OPERATE_RET app_dp_report(UINT8_T dp_id, UINT8_T *buf, UINT32_T size)
{
    memset(&g_rsp, 0, SIZEOF(demo_dp_t));

    g_rsp.dp_id = dp_id;

    switch (dp_id) {
    //dp_type = bool
    case WR_BASIC_LED: {
        g_rsp.dp_type = DT_BOOL;
        g_rsp.dp_data_len = DT_BOOL_LEN;
        memcpy(g_rsp.dp_data, buf, DT_BOOL_LEN);
    }
    break;

    //dp_type = enum
    case WR_BASIC_CHARGE_STATE: {
        g_rsp.dp_type = DT_ENUM;
        g_rsp.dp_data_len = DT_ENUM_LEN;
        memcpy(g_rsp.dp_data, buf, DT_ENUM_LEN);
    }
    break;

    //dp_type = value
    case WR_BASIC_TEMPERATURE: {
        g_rsp.dp_type = DT_VALUE;
        g_rsp.dp_data_len = DT_VALUE_LEN;
        memcpy(g_rsp.dp_data, buf, DT_VALUE_LEN);
    }
    break;

    //dp_type = string
    case WR_BASIC_WELCOME: {
        g_rsp.dp_type = DT_STRING;
        g_rsp.dp_data_len = size;
        memcpy(g_rsp.dp_data, buf, size);
    }
    break;

    //dp_type = raw
    case WR_BASIC_CUSTOM_DATA: {
        g_rsp.dp_type = DT_RAW;
        g_rsp.dp_data_len = size;
        memcpy(g_rsp.dp_data, buf, size);
    }
    break;

    //dp_type = fault
    case WR_BASIC_FAULT_ALARM: {
        g_rsp.dp_type = DT_BITMAP;
        g_rsp.dp_data_len = DT_BITMAP_MAX;
        memcpy(g_rsp.dp_data, buf, DT_BITMAP_MAX);
    }
    break;

    default: {
    } break;
    }

    UINT16_T rsp_len = g_rsp.dp_data_len + 4;

    tal_util_reverse_byte(&g_rsp.dp_data_len, SIZEOF(UINT16_T));

    TAL_PR_HEXDUMP_INFO("dp_rsp", (VOID_T *)&g_rsp, rsp_len);

    return tuya_ble_dp_data_send(g_sn++, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, (VOID_T *)&g_rsp, rsp_len);
}


