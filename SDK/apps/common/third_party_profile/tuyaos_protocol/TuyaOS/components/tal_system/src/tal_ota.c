/**
 * @file tal_ota.c
 * @brief This is tal_ota file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tkl_ota.h"
#include "tal_ota.h"

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
TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_ota_get_ability(UINT32_T *image_size, TUYA_OTA_TYPE_E *type)
{
    return tkl_ota_get_ability(image_size, type);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_ota_start_notify(UINT32_T image_size, TUYA_OTA_TYPE_E type, TUYA_OTA_PATH_E path)
{
    return tkl_ota_start_notify(image_size, type, path);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_ota_data_process(TUYA_OTA_DATA_T *pack, UINT32_T *remain_len)
{
    return tkl_ota_data_process(pack, remain_len);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_ota_end_notify(BOOL_T reset)
{
    return tkl_ota_end_notify(reset);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_ota_get_old_firmware_info(TUYA_OTA_FIRMWARE_INFO_T **info)
{
    return tkl_ota_get_old_firmware_info(info);
}

