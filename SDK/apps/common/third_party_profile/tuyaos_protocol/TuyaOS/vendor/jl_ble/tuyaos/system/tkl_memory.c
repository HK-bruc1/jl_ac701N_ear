/**
 * @file tkl_memory.c
 * @brief This is tkl_memory file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "system/includes.h"
/* #include "stdio.h" */
/* #include "string.h" */

#include "vendor/jl_ble/tuyaos/include/board.h"
#include "tkl_system.h"
#include "tkl_memory.h"

#include "tuya_mem_heap.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/


/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
UINT32_T __irq_mask;

STATIC VOID_T heap_enter_critical(VOID_T);
STATIC VOID_T heap_exit_critical(VOID_T);
STATIC VOID_T myprintf(char *fmt, ...);
heap_context_t heap_context = {
    .enter_critical = heap_enter_critical,
    .exit_critical = heap_exit_critical,
    .dbg_output = myprintf,
};

UINT8_T heap_pool[BOARD_HEAP_SIZE] = {0};
HEAP_HANDLE heap_handle = 0;

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




STATIC VOID_T heap_enter_critical(VOID_T)
{
    __irq_mask = tkl_system_enter_critical();
}

STATIC VOID_T heap_exit_critical(VOID_T)
{
    tkl_system_exit_critical(__irq_mask);
}

STATIC VOID_T myprintf(char *fmt, ...)
{
    char buf[256] = {0};
    va_list args;

    va_start(args, fmt);
    UINT32_T len = sprintf(buf, fmt, args);
    va_end(args);
}

VOID_T tuya_memory_init(VOID_T)
{
    tuya_mem_heap_init(&heap_context);
    tuya_mem_heap_create(heap_pool, BOARD_HEAP_SIZE, &heap_handle);
}

VOID_T *tkl_system_malloc(CONST SIZE_T size)
{
    return tuya_mem_heap_malloc(heap_handle, size);
}

VOID_T tkl_system_free(VOID_T *ptr)
{
    tuya_mem_heap_free(heap_handle, ptr);
}

VOID_T *tkl_system_calloc(size_t nitems, size_t size)
{
    return 0;
}

VOID_T *tkl_system_realloc(VOID_T *ptr, size_t size)
{
    return 0;
}

INT32_T tkl_system_get_free_heap_size(VOID_T)
{
    return 0;
}

