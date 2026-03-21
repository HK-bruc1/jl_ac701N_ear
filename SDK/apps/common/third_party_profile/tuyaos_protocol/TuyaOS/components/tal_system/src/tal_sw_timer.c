/**
 * @file tal_sw_timer.c
 * @brief This is tal_sw_timer file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tkl_memory.h"
#include "tkl_system.h"
#include "tal_sw_timer.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define TIMER_MALLOC        tkl_system_malloc
#define TIMER_FREE          tkl_system_free

#define ETIMER_HALF_INT32U              (0x7FFFFFFFUL)
#define ETIME_IS_EXPIRED(expired, now)  ((((UINT32_T)((now)-(expired))) < ETIMER_HALF_INT32U) ? 1 : 0)

typedef enum {
    ETIMER_IDLE = 0,
    ETIMER_RUNNING,
} TIMER_STATUS_E;

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
typedef struct __ETIMER {
    struct __ETIMER *next;
    VOID_T          *cb_arg;
    TAL_TIMER_CB    callback;
    UINT32_T        period_time;
    UINT32_T        expired_time;
    TIMER_TYPE      type;
    TIMER_STATUS_E  status;
} __ETIMER_T;

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
STATIC __ETIMER_T *timer_list;

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_sw_timer_init(VOID_T)
{
    return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_sw_timer_create(TAL_TIMER_CB func, VOID_T *arg, TIMER_ID *timer_id)
{
    __ETIMER_T *timer;

    if (func == NULL || timer_id == NULL) {
        return OPRT_INVALID_PARM;
    }

    timer = (__ETIMER_T *)TIMER_MALLOC(sizeof(__ETIMER_T));
    if (timer == NULL) {
        return OPRT_MALLOC_FAILED;
    }

    memset(timer, 0, sizeof(__ETIMER_T));
    timer->cb_arg = arg;
    timer->callback = func;
    *timer_id = (TIMER_ID)timer;

    return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_sw_timer_delete(TIMER_ID timer_id)
{
    __ETIMER_T *timer = (__ETIMER_T *)timer_id;

    if (timer == NULL) {
        return OPRT_INVALID_PARM;
    }

    TKL_ENTER_CRITICAL();
    __ETIMER_T **next_timer = &timer_list;
    // find and remove timer on single list
    for (; *next_timer; next_timer = &(*next_timer)->next) {
        __ETIMER_T *entry = *next_timer;
        if (entry == timer) {
            *next_timer = timer->next;
            break;
        }
    }
    TKL_EXIT_CRITICAL();
    TIMER_FREE(timer);

    return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_sw_timer_start(TIMER_ID timer_id, TIME_MS time_ms, TIMER_TYPE timer_type)
{
    UINT32_T curr_time = tkl_system_get_millisecond();
    __ETIMER_T *timer = (__ETIMER_T *)timer_id;

    if (timer == NULL ||  time_ms >= ETIMER_HALF_INT32U) {
        return OPRT_INVALID_PARM;
    }

    timer->expired_time = curr_time + time_ms;
    timer->period_time = time_ms;
    timer->type = timer_type;

    TKL_ENTER_CRITICAL();

    __ETIMER_T **next_timer = &timer_list;
    // remove the existing target timer
    if (timer->status == ETIMER_RUNNING) {
        for (; *next_timer; next_timer = &(*next_timer)->next) {
            if (timer == *next_timer) {
                *next_timer = timer->next;
                break;
            }
        }
    }

    // insert timer
    for (next_timer = &timer_list; ; next_timer = &(*next_timer)->next) {
        // first timer insert
        if (*next_timer == NULL) {
            timer->next = NULL;
            *next_timer = timer;
            break;
        }
        if (timer->expired_time - (*next_timer)->expired_time > ETIMER_HALF_INT32U) { //get min timer
            timer->next = *next_timer;
            *next_timer = timer;
            break;
        }
    }

    timer->status = ETIMER_RUNNING;
    TKL_EXIT_CRITICAL();

    return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_sw_timer_stop(TIMER_ID timer_id)
{
    __ETIMER_T *timer = (__ETIMER_T *)timer_id;

    if (timer == NULL || timer->status != ETIMER_RUNNING) {
        return OPRT_INVALID_PARM;
    }

    TKL_ENTER_CRITICAL();

    __ETIMER_T **next_timer = &timer_list;
    timer->status = ETIMER_IDLE;
    // modify the timer type to prevent the timer from being unable to be stopped in the callback
    timer->type = TAL_TIMER_ONCE;
    // find and remove timer from timer list
    for (; *next_timer; next_timer = &(*next_timer)->next) {
        __ETIMER_T *entry = *next_timer;
        if (entry == timer) {
            *next_timer = timer->next;
            entry->next = NULL;
            break;
        }
    }
    TKL_EXIT_CRITICAL();

    return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_sw_timer_trigger(TIMER_ID timer_id)
{
    return tal_sw_timer_start(timer_id, 1, TAL_TIMER_ONCE);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_sw_timer_release(VOID_T)
{
    return OPRT_NOT_SUPPORTED;
}

TUYA_WEAK_ATTRIBUTE BOOL_T tal_sw_timer_is_running(TIMER_ID timer_id)
{
    __ETIMER_T *timer = (__ETIMER_T *)timer_id;

    if (timer == NULL) {
        return FALSE;
    }

    return (timer->status == ETIMER_RUNNING) ? TRUE : FALSE;
}

TUYA_WEAK_ATTRIBUTE INT32_T tal_sw_timer_get_num(VOID_T)
{
    UINT32_T nums = 0;
    __ETIMER_T *timer = timer_list;

    TKL_ENTER_CRITICAL();
    for (; timer != NULL; timer = timer->next) {
        nums++;
    }
    TKL_EXIT_CRITICAL();
    return nums;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_sw_timer_remain_time_get(TIMER_ID timer_id, UINT32_T *remain_time)
{
    UINT32_T current_ms;
    __ETIMER_T *timer = (__ETIMER_T *)timer_id;

    if (timer == NULL || remain_time == NULL ||
        timer->status != ETIMER_RUNNING) {
        return OPRT_INVALID_PARM;
    }

    current_ms = tkl_system_get_millisecond();

    if (ETIME_IS_EXPIRED(timer->expired_time, current_ms)) {
        *remain_time = 0;
    } else {
        *remain_time = timer->expired_time - current_ms;
    }

    return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE UINT32_T tal_sw_timer_get_nearest_timer_remaining_time(VOID_T)
{
    UINT32_T remain_time = 0xFFFFFFFF;

    tal_sw_timer_remain_time_get(timer_list, &remain_time);

    return remain_time;
}

TUYA_WEAK_ATTRIBUTE VOID_T tal_sw_timer_loop_handler(VOID_T)
{
    VOID_T *parg = NULL;
    __ETIMER_T *entry = NULL;
    TAL_TIMER_CB pcallback = NULL;


    __ETIMER_T **next_etimer = &timer_list;
    for (; *next_etimer; next_etimer = &(*next_etimer)->next) {
        TKL_ENTER_CRITICAL();
        entry = *next_etimer;

        // expired time check
        if (!ETIME_IS_EXPIRED(entry->expired_time, tkl_system_get_millisecond())) {
            TKL_EXIT_CRITICAL();
            break;
        }
        // find expired timer
        *next_etimer = entry->next;
        entry->next = NULL;
        entry->status = ETIMER_IDLE;
        if (entry->type == TAL_TIMER_CYCLE) {
            tal_sw_timer_start((TIMER_ID)entry, entry->period_time, entry->type);
        }
        parg = entry->cb_arg;
        pcallback = entry->callback;
        TKL_EXIT_CRITICAL();
        break;
    }
    if (pcallback != NULL) {
        pcallback((TIMER_ID)entry, parg);
    }
}

TUYA_WEAK_ATTRIBUTE VOID_T tuya_etimer_loop_handler(VOID_T)
{
    tal_sw_timer_loop_handler();
}

