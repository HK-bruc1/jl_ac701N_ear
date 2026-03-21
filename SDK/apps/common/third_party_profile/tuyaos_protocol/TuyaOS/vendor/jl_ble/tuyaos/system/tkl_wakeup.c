/**
 * @file tkl_wakeup.c
 * @brief This is tkl_wakeup file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tkl_wakeup.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define WAKEUP_SOURCE_COUNT 5

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
TUYA_WAKEUP_SOURCE_BASE_CFG_T *g_wakeup_cfg = NULL;

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




OPERATE_RET tkl_wakeup_source_set(CONST TUYA_WAKEUP_SOURCE_BASE_CFG_T *param)
{
    if (param == NULL) {
        return OPRT_INVALID_PARM;
    }

    for (UINT32_T idx = 0; idx < WAKEUP_SOURCE_COUNT; idx++) {
        if (g_wakeup_cfg) {
            if (g_wakeup_cfg[idx].source == (TUYA_WAKEUP_SOURCE_E)0xFF) {
                memcpy(&g_wakeup_cfg[idx], param, SIZEOF(TUYA_WAKEUP_SOURCE_BASE_CFG_T));
                break;
            }
        }
    }

    return OPRT_OK;
}

OPERATE_RET tkl_wakeup_source_clear(CONST TUYA_WAKEUP_SOURCE_BASE_CFG_T *param)
{
    if (param == NULL) {
        return OPRT_INVALID_PARM;
    }

    for (UINT32_T idx = 0; idx < WAKEUP_SOURCE_COUNT; idx++) {
        // Only support gpio wake up
        if (g_wakeup_cfg[idx].source == param->source
            && g_wakeup_cfg[idx].wakeup_para.gpio_param.gpio_num == param->wakeup_para.gpio_param.gpio_num) {
            memset(&g_wakeup_cfg[idx], 0, SIZEOF(TUYA_WAKEUP_SOURCE_BASE_CFG_T));
            g_wakeup_cfg[idx].source = 0xFF;
        }
    }

    return OPRT_OK;
}

