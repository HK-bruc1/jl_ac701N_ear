/**
 * @file tuya_ble_mem.c
 * @brief This is tuya_ble_mem file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "string.h"

#include "tal_memory.h"

#include "tuya_ble_type.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_internal_config.h"

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




VOID_T *tuya_ble_malloc(UINT16_T size)
{
    UINT8_T *ptr = tal_malloc(size);
    if (ptr) {
        memset(ptr, 0x0, size); //allocate buffer need init
    }
    return ptr;
}

tuya_ble_status_t tuya_ble_free(UINT8_T *ptr)
{
    if (ptr == NULL) {
        return TUYA_BLE_SUCCESS;
    }

    tal_free(ptr);
    return TUYA_BLE_SUCCESS;
}

VOID_T *tuya_ble_calloc_n(UINT32_T n, UINT32_T size)
{
    VOID_T *ptr = NULL;
    ptr = tal_malloc(n * size);
    if (ptr != NULL) {
        memset(ptr, 0, n * size);
    }
    return ptr;
}

VOID_T tuya_ble_free_n(VOID_T *ptr)
{
    tal_free(ptr);
}

