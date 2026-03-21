/**
 * @file tal_flash.c
 * @brief This is tal_flash file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tkl_flash.h"
#include "tkl_memory.h"
#include "tal_flash.h"

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




TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_flash_read(UINT32_T addr, UCHAR_T *dst, UINT32_T size)
{
    return tkl_flash_read(addr, dst, size);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_flash_write(UINT32_T addr, CONST UCHAR_T *src, UINT32_T size)
{
    return tkl_flash_write(addr, src, size);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_flash_erase(UINT32_T addr, UINT32_T size)
{
    return tkl_flash_erase(addr, size);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_flash_lock(UINT32_T addr, UINT32_T size)
{
    return tkl_flash_lock(addr, size);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_flash_unlock(UINT32_T addr, UINT32_T size)
{
    return tkl_flash_unlock(addr, size);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_flash_get_one_type_info(TUYA_FLASH_TYPE_E type, TUYA_FLASH_BASE_INFO_T *info)
{
    return tkl_flash_get_one_type_info(type, info);
}

