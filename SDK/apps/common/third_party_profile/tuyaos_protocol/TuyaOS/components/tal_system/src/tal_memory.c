/**
 * @file tal_memory.c
 * @brief This is tal_memory file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "string.h"
#include "tkl_memory.h"
#include "tal_memory.h"
#include "tal_system.h"

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




TUYA_WEAK_ATTRIBUTE VOID_T *tal_malloc(SIZE_T size)
{
    UINT8_T *ptr = tkl_system_malloc(size);
    if (ptr == NULL) {
        tal_system_reset();
    }
    return ptr;
}

TUYA_WEAK_ATTRIBUTE VOID_T tal_free(VOID_T *ptr)
{
    tkl_system_free(ptr);
}

TUYA_WEAK_ATTRIBUTE VOID_T *tal_calloc(SIZE_T nitems, SIZE_T size)
{
    return tkl_system_calloc(nitems, size);
}

TUYA_WEAK_ATTRIBUTE VOID_T *tal_realloc(VOID_T *ptr, SIZE_T size)
{
    return tkl_system_realloc(ptr, size);
}

TUYA_WEAK_ATTRIBUTE INT32_T tal_system_get_free_heap_size(VOID_T)
{
    return tkl_system_get_free_heap_size();
}

