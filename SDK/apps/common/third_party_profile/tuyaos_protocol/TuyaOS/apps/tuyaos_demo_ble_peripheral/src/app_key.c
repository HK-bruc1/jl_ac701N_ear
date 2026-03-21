/**
 * @file app_key.c
 * @brief This is app_key file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */


#include "vendor/jl_ble/tuyaos/include/board.h"

#include "tal_log.h"
#include "tal_sw_timer.h"
#include "tal_gpio.h"
#include "tal_rtc.h"
#include "tal_key.h"
#include "tal_sdk_test.h"

#include "tuya_ble_api.h"
#include "tuya_ble_protocol_callback.h"

#include "app_key.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define APP_KEY_PIN BOARD_KEY_PIN

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
STATIC TIMER_ID app_key_timer_id = NULL;

STATIC VOID_T app_key_handler(UINT32_T state);
STATIC tal_key_param_t key_press_param = {
    .pin = APP_KEY_PIN,
    .valid_level = TUYA_KEY_LEVEL_LOW,
    .count_len = 3,
    .count_array = {5, 300, 500},
    .handler = app_key_handler,
};

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




STATIC VOID_T app_key_handler(UINT32_T state)
{
//    TAL_PR_INFO("Key state: %d", state);
    switch (state) {
    //Short press
    case 1: {
#if defined(TUYA_SDK_TEST) && (TUYA_SDK_TEST == 1)
        tal_ble_sdk_test_wake_up_handler();
#endif
    }
    break;

    //Long press
    case 2: {
        tuya_ble_device_factory_reset();
        tuya_ble_disconnect_and_reset_timer_start();
    }
    break;

    //Long long press timeout
    case 3: {
    } break;

    //Short press release
    case 5: {
    } break;

    //Long press release
    case 6: {
    } break;

    default: {
    } break;
    }
}

UINT32_T tal_key_get_pin_level(UINT32_T pin)
{
    // Weak function instance
    TUYA_GPIO_LEVEL_E level = TUYA_GPIO_LEVEL_LOW;
    tal_gpio_read(pin, &level);
    return level;
}

STATIC VOID_T app_key_irq_cb(VOID_T *args)
{
    TUYA_GPIO_LEVEL_E level = TUYA_GPIO_LEVEL_LOW;
    tal_gpio_read(APP_KEY_PIN, &level);

    if (level == TUYA_GPIO_LEVEL_LOW) {
        tal_sw_timer_start(app_key_timer_id, 10, TAL_TIMER_CYCLE);
    }
}

STATIC VOID_T app_key_timeout_handler(TIMER_ID timer_id, VOID_T *arg)
{
    if (!tal_key_timeout_handler(&key_press_param)) {
        tal_sw_timer_stop(app_key_timer_id);
    }
}

VOID_T app_key_init(VOID_T)
{
    TUYA_GPIO_BASE_CFG_T gpio_cfg = {
        .mode = TUYA_GPIO_PULLUP,
        .direct = TUYA_GPIO_INPUT,
        .level = TUYA_GPIO_LEVEL_LOW,
    };
    TUYA_GPIO_IRQ_T gpio_irq = {
        .mode = TUYA_GPIO_IRQ_RISE_FALL,
        .cb = app_key_irq_cb,
        .arg = NULL,
    };

    tal_gpio_init(APP_KEY_PIN, &gpio_cfg);
    tal_gpio_irq_init(APP_KEY_PIN, &gpio_irq);
    tal_gpio_irq_enable(APP_KEY_PIN);

    tal_key_init(&key_press_param);

    tal_sw_timer_create(app_key_timeout_handler, NULL, &app_key_timer_id);
}

