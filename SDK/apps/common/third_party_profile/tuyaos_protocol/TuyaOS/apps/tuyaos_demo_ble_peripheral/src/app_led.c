/**
 * @file app_led.c
 * @brief This is app_led file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */


#include "tal_log.h"
#include "tal_sw_timer.h"
#include "tal_gpio.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/


/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
STATIC TIMER_ID app_led_timer_id = NULL;

STATIC UINT32_T s_app_led_count = 0;

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




STATIC VOID_T app_led_timeout_handler(TIMER_ID timer_id, VOID_T *arg)
{
    tal_gpio_write(17, (s_app_led_count == 0));
    tal_gpio_write(18, (s_app_led_count == 1));
    tal_gpio_write(19, (s_app_led_count == 2));
    tal_gpio_write(20, (s_app_led_count == 3));

    s_app_led_count++;
    if (s_app_led_count == 4) {
        s_app_led_count = 0;
    }
}

VOID_T app_led_timer_init(VOID_T)
{
    tal_sw_timer_create(app_led_timeout_handler, NULL, &app_led_timer_id);
}

VOID_T app_led_timer_start(VOID_T)
{
    tal_sw_timer_start(app_led_timer_id, 200, TAL_TIMER_CYCLE);
}

VOID_T app_led_timer_stop(VOID_T)
{
    tal_sw_timer_stop(app_led_timer_id);
}

