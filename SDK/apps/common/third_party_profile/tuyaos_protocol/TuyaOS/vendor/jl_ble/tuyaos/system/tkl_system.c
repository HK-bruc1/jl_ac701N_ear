/**
 * @file tkl_system.c
 * @brief This is tkl_system file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "timer.h"
#include "task.h"
#include "vendor/jl_ble/tuyaos/include/board.h"
#include "tkl_system.h"
#include "tkl_memory.h"
#include "tkl_bluetooth.h"


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
// TKL
extern OPERATE_RET tal_init_first(VOID_T);
extern OPERATE_RET tal_init_second(VOID_T);
extern OPERATE_RET tal_init_third(VOID_T);
extern OPERATE_RET tal_init_last(VOID_T);
extern OPERATE_RET tal_main_loop(VOID_T);




UINT32_T tkl_system_enter_critical(VOID_T)
{
    /* TY_PRINTF("%s", __FUNCTION__); */
    return OPRT_NOT_SUPPORTED;
}

VOID_T tkl_system_exit_critical(UINT32_T irq_mask)
{
}

VOID_T tkl_system_reset(VOID_T)
{
    cpu_reset();
}

SYS_TICK_T tkl_system_get_tick_count(VOID_T)
{
    TY_PRINTF("%s", __FUNCTION__);
    return jiffies;
    //return OPRT_NOT_SUPPORTED;
}

SYS_TIME_T tkl_system_get_millisecond(VOID_T)
{
    TY_PRINTF("%s", __FUNCTION__);
    return sys_timer_get_ms();
    //return OPRT_NOT_SUPPORTED;
}

INT32_T tkl_system_get_random(UINT32_T range)
{
    //TY_PRINTF("%s", __FUNCTION__);
    return rand32();
    //return OPRT_NOT_SUPPORTED;
}

TUYA_RESET_REASON_E tkl_system_get_reset_reason(CHAR_T **describe)
{
    return TUYA_RESET_REASON_UNKNOWN;
}

VOID_T tkl_system_sleep(UINT32_T num_ms)
{
}

VOID_T tkl_system_delay(UINT32_T num_ms)
{
}

VOID_T tkl_system_log_output(CONST UINT8_T *buf, UINT32_T size)
{
    for (int i = 0; i < size; i++) {
        putchar(buf[i]);
    }
}

OPERATE_RET tkl_init_first(VOID_T)
{
    return tal_init_first();
}

extern TUYA_WAKEUP_SOURCE_BASE_CFG_T *g_wakeup_cfg;
#define NUMBER_OF_PINS  1
OPERATE_RET tkl_init_second(VOID_T)
{
    g_wakeup_cfg = tkl_system_malloc(NUMBER_OF_PINS * SIZEOF(TUYA_WAKEUP_SOURCE_BASE_CFG_T));
    if (g_wakeup_cfg) {
        for (UINT32_T idx = 0; idx < NUMBER_OF_PINS; idx++) {
            g_wakeup_cfg[idx].source = 0xFF;
        }
    }

    return tal_init_second();
}

OPERATE_RET tkl_init_third(VOID_T)
{
    return tal_init_third();
}

OPERATE_RET tkl_init_last(VOID_T)
{
    return tal_init_last();
}

OPERATE_RET tkl_main_loop(VOID_T)
{
    return tal_main_loop();
}

void tuya_all_init(void)
{
    tkl_init_first();
    tkl_init_second();
    tkl_init_third();
    tkl_init_last();
    tkl_main_loop();
}


void tuya_all_exit(void)
{
    tkl_ble_stack_deinit(TKL_BLE_ROLE_SERVER);
}




