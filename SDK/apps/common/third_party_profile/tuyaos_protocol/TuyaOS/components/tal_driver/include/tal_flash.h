/**
 * @file tal_flash.h
 * @brief This is tal_flash file
 * @version 1.0
 * @date 2021-08-06
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TAL_FLASH_H__
#define __TAL_FLASH_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

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

/**
* @brief read flash
*
* @param[in] addr: flash address
* @param[out] dst: pointer of buffer
* @param[in] size: size of buffer
*
* @note This API is used for reading flash.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/

/**
 * @brief tal_flash_read
 *
 * @param[in] addr: addr
 * @param[in] *dst: *dst
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_flash_read(UINT32_T addr, UCHAR_T *dst, UINT32_T size);

/**
* @brief write flash
*
* @param[in] addr: flash address
* @param[in] src: pointer of buffer
* @param[in] size: size of buffer
*
* @note This API is used for writing flash.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/

/**
 * @brief tal_flash_write
 *
 * @param[in] addr: addr
 * @param[in] *src: *src
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_flash_write(UINT32_T addr, CONST UCHAR_T *src, UINT32_T size);

/**
* @brief erase flash
*
* @param[in] addr: flash address
* @param[in] size: size of flash block
*
* @note This API is used for erasing flash.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/

/**
 * @brief tal_flash_erase
 *
 * @param[in] addr: addr
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_flash_erase(UINT32_T addr, UINT32_T size);

/**
* @brief lock flash
*
* @param[in] addr: lock begin addr
* @param[in] size: lock area size
*
* @note This API is used for lock flash.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/

/**
 * @brief tal_flash_lock
 *
 * @param[in] addr: addr
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_flash_lock(UINT32_T addr, UINT32_T size);

/**
* @brief unlock flash
*
* @param[in] addr: unlock begin addr
* @param[in] size: unlock area size
*
* @note This API is used for unlock flash.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/

/**
 * @brief tal_flash_unlock
 *
 * @param[in] addr: addr
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_flash_unlock(UINT32_T addr, UINT32_T size);

/**
* @brief get one flash type info
*
* @param[in] type: flash type
* @param[in] info: flash info
*
* @note This API is used for unlock flash.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/

/**
 * @brief tal_flash_get_one_type_info
 *
 * @param[in] type: type
 * @param[in] info: info
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_flash_get_one_type_info(TUYA_FLASH_TYPE_E type, TUYA_FLASH_BASE_INFO_T *info);


#ifdef __cplusplus
}
#endif

#endif /* __TAL_FLASH_H__ */

