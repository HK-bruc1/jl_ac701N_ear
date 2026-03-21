/**
 * @file tkl_thread.c
 * @brief This is tkl_thread file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2024-2024 Tuya Inc. All Rights Reserved.
 *
 */

#include "tkl_thread.h"
// #include "FreeRTOS.h"
// #include "task.h"


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


int os_task_create_affinity_core(void (*task)(void *p_arg),
                                 void *p_arg,
                                 unsigned char prio,
                                 unsigned int stksize,
                                 int qsize,
                                 const char *name,
                                 unsigned char core);

OPERATE_RET tkl_thread_create(TKL_THREAD_HANDLE *thread,
                              CONST CHAR_T *name,
                              UINT_T stack_size,
                              UINT_T priority,
                              CONST THREAD_FUNC_T func,
                              VOID_T *CONST arg)
{
    return os_task_create_affinity_core(func, arg, /*(TaskHandle_t*)thread, */priority, stack_size, 256, name, 0);
}

OPERATE_RET tkl_thread_release(CONST TKL_THREAD_HANDLE thread)
{
    return OPRT_NOT_SUPPORTED;
}


