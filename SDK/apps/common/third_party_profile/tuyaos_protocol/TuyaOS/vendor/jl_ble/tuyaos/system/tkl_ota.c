/**
 * @file tkl_ota.c
 * @brief This is tkl_ota file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "vendor/jl_ble/tuyaos/include/board.h"
#include "tkl_flash.h"
#include "tkl_bluetooth.h"
#include "tkl_system.h"
#include "tkl_ota.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/


/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




OPERATE_RET tkl_ota_get_ability(UINT32_T *image_size, TUYA_OTA_TYPE_E *type)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_OK;
}

OPERATE_RET tkl_ota_start_notify(UINT_T image_size, TUYA_OTA_TYPE_E type, TUYA_OTA_PATH_E path)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_OK;
}

OPERATE_RET tkl_ota_erase_flash(UINT32_T offset_addr)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_OK;
}

OPERATE_RET tkl_ota_data_process(TUYA_OTA_DATA_T *pack, UINT32_T *remain_len)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_OK;
}

OPERATE_RET tkl_ota_end_notify(BOOL_T reset)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_OK;
}

OPERATE_RET tkl_ota_get_old_firmware_info(TUYA_OTA_FIRMWARE_INFO_T **info)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_OK;
}

OPERATE_RET tuya_ble_ota_make_ota_firmware_valid(void)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_OK;
}

OPERATE_RET tuya_ble_ota_make_ota_firmware_invalid(void)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_OK;
}

