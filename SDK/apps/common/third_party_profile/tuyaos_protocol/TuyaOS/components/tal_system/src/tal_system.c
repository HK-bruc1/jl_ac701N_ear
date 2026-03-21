/**
 * @file tal_system.c
 * @brief This is tal_system file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tkl_system.h"
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
extern OPERATE_RET tuya_init_first(VOID_T);
extern OPERATE_RET tuya_init_second(VOID_T);
extern OPERATE_RET tuya_init_third(VOID_T);
extern OPERATE_RET tuya_init_last(VOID_T);
extern OPERATE_RET tuya_main_loop(VOID_T);




UINT32_T tal_system_enter_critical(VOID_T)
{
    return tkl_system_enter_critical();
}

VOID_T tal_system_exit_critical(UINT32_T irq_mask)
{
    tkl_system_exit_critical(irq_mask);
}

TUYA_WEAK_ATTRIBUTE VOID_T tal_system_sleep(UINT32_T time_ms)
{
    tkl_system_sleep(time_ms);
}

TUYA_WEAK_ATTRIBUTE VOID_T tal_system_reset(VOID_T)
{
    tkl_system_reset();
}

TUYA_WEAK_ATTRIBUTE SYS_TICK_T tal_system_get_tick_count(VOID_T)
{
    return tkl_system_get_tick_count();
}

TUYA_WEAK_ATTRIBUTE SYS_TIME_T tal_system_get_millisecond(VOID_T)
{
    return tkl_system_get_millisecond();
}

TUYA_WEAK_ATTRIBUTE INT32_T tal_system_get_random(UINT32_T range)
{
    return tkl_system_get_random(range);
}

TUYA_WEAK_ATTRIBUTE TUYA_RESET_REASON_E tal_system_get_reset_reason(CHAR_T **describe)
{
    return tkl_system_get_reset_reason(describe);
}

VOID_T tal_system_delay(UINT32_T time_ms)
{
    tkl_system_delay(time_ms);
}

OPERATE_RET tal_system_get_cpu_info(TUYA_CPU_INFO_T **cpu_ary, INT32_T *cpu_cnt)
{
    return OPRT_NOT_SUPPORTED;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_init_first(VOID_T)
{
    return tuya_init_first();
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_init_second(VOID_T)
{
    return tuya_init_second();
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_init_third(VOID_T)
{
    return tuya_init_third();
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_init_last(VOID_T)
{
    return tuya_init_last();
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_main_loop(VOID_T)
{
    return tuya_main_loop();
}

