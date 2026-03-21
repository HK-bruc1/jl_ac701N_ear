/**
 * @file tuya_ble_mem.h
 * @brief This is tuya_ble_mem file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_MEM_H__
#define __TUYA_BLE_MEM_H__

#include "tuya_cloud_types.h"
#include "tuya_ble_type.h"

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
 * @brief *tuya_ble_malloc
 *
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T *tuya_ble_malloc(UINT16_T size);

/**
 * @brief tuya_ble_free
 *
 * @param[in] *ptr: *ptr
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_free(UINT8_T *ptr);


#ifdef __cplusplus
}
#endif

#endif /* __TUYA_BLE_MEM_H__ */

