/**
 * @file tuya_ble_product_test_over_air.c
 * @brief This is tuya_ble_product_test_over_air file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "string.h"

#include "tal_util.h"
#include "tal_sw_timer.h"
#include "tal_system.h"
#include "tal_sleep.h"
//tal driver
#include "tal_rtc.h"
#include "tal_utc.h"
#include "tal_uart.h"
#include "tal_flash.h"
#include "tal_adc.h"
#include "tal_gpio.h"
#include "tal_pwm.h"
#include "tal_spi.h"
#include "tal_i2c.h"
#include "tal_watchdog.h"

#include "tuya_ble_api.h"
#include "tuya_ble_product_test_over_air.h"
#include "tuya_sdk_callback.h"
#include "tuya_ble_main.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/


/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
typedef struct {
    BOOL_T    flag;
    UINT8_T mode;
} test_enter_sleep_t;

typedef struct {
    test_enter_sleep_t enter_sleep;
    uint16_t adv_interval;
} test_param_t;

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
STATIC UINT8_T tx_buffer[128] = {0x66, 0xAA, 0x00, 0xF0};

STATIC test_param_t test_param = {0};
STATIC TIMER_ID sg_test_enter_sleep_timer_id = NULL;
STATIC UINT8_T sg_test_enter_sleep_timer_is_init = 0;

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




VOID_T tal_sdk_product_test_enter_sleep_handler(TIMER_ID timer_id, VOID_T *arg)
{
#if defined(TUYA_BLE_FEATURE_LONG_RANGE) && (TUYA_BLE_FEATURE_LONG_RANGE == 1)
    if (test_param.enter_sleep.mode == 0) {
        tal_ble_advertising_stop();
        tal_ble_ext_advertising_stop(tal_ext_adv_extended_adv);
    } else if (test_param.enter_sleep.mode == 1) {
        tal_ble_ext_advertising_stop(tal_ext_adv_extended_adv);
    } else if (test_param.enter_sleep.mode == 2) {
        tal_ble_advertising_stop();
    }
#else
    if (test_param.enter_sleep.mode == 0) {
        tal_ble_advertising_stop();
    }
#endif

    tal_uart_deinit(TUYA_UART_NUM_0);

//    tal_oled_clear(); //It will prolong the time it takes to enter sleep
    tal_i2c_deinit(TUYA_I2C_NUM_0);

    tal_pwm_deinit(TUYA_PWM_NUM_0);
    tal_adc_deinit(TUYA_ADC_NUM_0);

    tal_watchdog_stop();

    tal_cpu_allow_sleep();
}

VOID_T tuya_ble_prod_sleep_timer_init(VOID_T)
{
    if (sg_test_enter_sleep_timer_is_init) {
        return;
    }

    if (tal_sw_timer_create(tal_sdk_product_test_enter_sleep_handler, NULL, &sg_test_enter_sleep_timer_id) != OPRT_OK) {
        TAL_PR_ERR("tuya_ble_xTimer_prod_monitor creat failed");
    } else {
        sg_test_enter_sleep_timer_is_init = 1;
    }
}

VOID_T tuya_ble_prod_sleep_timer_start(VOID_T)
{
    if (sg_test_enter_sleep_timer_is_init) {
        if (tal_sw_timer_start(sg_test_enter_sleep_timer_id, 200, TAL_TIMER_ONCE) != OPRT_OK) {
            TAL_PR_ERR("tuya_ble_xTimer_prod_monitor start failed");
        }
    }
}

STATIC VOID_T tuya_ble_auc_common_test_sleep_mode(UINT8_T channel, UINT8_T *p_in_data, UINT16_T in_len)
{
    /* if dev is binding, can't entry ftm mode */
    if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
        TAL_PR_INFO("AUC ENTER, BUT DEV IS BINDING");

        tuya_ble_device_delay_ms(200);
        tuya_ble_device_reset();
        return;
    }

    if (channel != 0) {
        return;
    }

    UINT8_T tmp_buf[2] = {0x01};
    UINT8_T *p_buf = &p_in_data[10];

    UINT8_T  sleep_mode = p_buf[0];
    UINT16_T adv_interval = (p_buf[1] << 8) + p_buf[2];

    TAL_PR_INFO("AUC TEST SLEEP MODE!");

    // enter sleep, now only support sleep_mode 0
    if (sleep_mode == 0) {
        // stop adv
        tal_ble_advertising_stop();
#if defined(TUYA_BLE_FEATURE_LONG_RANGE) && (TUYA_BLE_FEATURE_LONG_RANGE == 1)
        tal_ble_ext_advertising_stop(tal_ext_adv_extended_adv);
#endif
        test_param.enter_sleep.mode = sleep_mode;
        test_param.adv_interval = adv_interval;
        tuya_ble_prod_sleep_timer_init();
        tuya_ble_prod_sleep_timer_start();
    } else {
        tmp_buf[1] = 0x01;
        TAL_PR_ERR("AUC TEST SLEEP MODE NOT SUPPORT, sleep_mode: %d!", sleep_mode);
    }

    // rsp
    tuya_ble_product_test_rsp(channel, PRODUCT_TEST_CMD_COMMON_TEST, tmp_buf, sizeof(tmp_buf));
}

STATIC VOID_T tuya_ble_auc_common_test(UINT8_T channel, UINT8_T *p_in_data, UINT16_T in_len)
{
    switch (p_in_data[9]) {
    case COMMON_TEST_SUB_CMD: {
        tuya_ble_auc_common_test_sleep_mode(channel, p_in_data, in_len);
    }
    break;

    default: {
    } break;
    }
}

TUYA_WEAK_ATTRIBUTE VOID_T tuya_ble_custom_app_production_test_process(UINT8_T channel, UINT8_T *p_in_data, UINT16_T in_len)
{
    ty_product_test_cmd_t *cmd = (VOID_T *)p_in_data;
    tal_util_reverse_byte((VOID_T *)&cmd->len, SIZEOF(UINT16_T));
    tal_util_reverse_byte((VOID_T *)&cmd->sub_id, SIZEOF(UINT16_T));

    if ((cmd->type != 3) || (cmd->len < 3)) {
        return;
    }

//    UINT16_T data_len = cmd->len - 3;

    switch (cmd->sub_id) {
    case PRODUCT_TEST_CMD_ENTER: {
        UINT8_T tmp_buf[] = "{\"ret\":true}";
        tuya_ble_product_test_rsp(channel, cmd->sub_id, tmp_buf, strlen((VOID_T *)tmp_buf));
    }
    break;

    case PRODUCT_TEST_CMD_EXIT: {
        UINT8_T tmp_buf[] = "{\"ret\":true}";
        tuya_ble_product_test_rsp(channel, cmd->sub_id, tmp_buf, strlen((VOID_T *)tmp_buf));
    }
    break;

    case PRODUCT_TEST_CMD_COMMON_TEST:
        tuya_ble_auc_common_test(channel, p_in_data, in_len);
        break;

    default: {
    } break;
    }
}

UINT32_T tuya_ble_product_test_rsp(UINT8_T channel, UINT16_T cmdId, UINT8_T *buf, UINT16_T size)
{
    UINT32_T len = 4;

    tx_buffer[len] = (size + 3) >> 8;
    len++;
    tx_buffer[len] = (size + 3) & 0xFF;
    len++;

    tx_buffer[len] = 0x03;
    len++;

    tx_buffer[len] = cmdId >> 8;
    len++;
    tx_buffer[len] = cmdId & 0xFF;
    len++;

    if (size > 0) {
        memcpy(&tx_buffer[len], buf, size);
        len += size;
    }

    tx_buffer[len] = tal_util_check_sum8(tx_buffer, len);
    len += 1;

    tuya_ble_production_test_asynchronous_response(channel, tx_buffer, len);

    return 0;
}

