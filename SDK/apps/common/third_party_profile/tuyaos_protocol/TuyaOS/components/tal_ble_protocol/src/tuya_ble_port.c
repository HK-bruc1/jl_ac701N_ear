/**
 * @file tuya_ble_port.c
 * @brief This is tuya_ble_port file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "string.h"
//tkl
#include "tkl_thread.h"
#include "tkl_queue.h"
//tal system
#include "tal_sleep.h"
#include "tal_ota.h"
#include "tal_system.h"
#include "tal_log.h"
#include "tal_sw_timer.h"
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
//tal bluetooth
#include "tal_bluetooth.h"

#if !defined(ENABLE_PLATFORM_USE_LIGHTWEIGHT_AES_COMPONENT) || (ENABLE_PLATFORM_USE_LIGHTWEIGHT_AES_COMPONENT == 0)
#include "tal_ble_aes.h"
#include "tal_ble_md5.h"
#else
#include "tal_ble_lightweight_aes.h"
#include "tal_ble_lightweight_md5.h"
#endif
#include "tuya_ble_config.h"
#include "tuya_ble_port.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define TY_TIMER_MAX_NUM  20

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
#pragma pack(1)
typedef struct {
    TIMER_ID timer_id;
    UINT8_T  is_occupy;
    UINT32_T ms;
    tuya_ble_timer_mode mode;
    tuya_ble_timer_handler_t timeout_handler;
} ty_timer_item_t;
#pragma pack()

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
STATIC ty_timer_item_t ty_timer_pool[TY_TIMER_MAX_NUM] = {0};
#if (defined(ENABLE_PLATFORM_USE_LIGHTWEIGHT_AES_COMPONENT) && (ENABLE_PLATFORM_USE_LIGHTWEIGHT_AES_COMPONENT == 1))
__attribute__((aligned(4))) STATIC UINT8_T sg_input_buffer[640];
#endif

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




tuya_ble_status_t tuya_ble_gap_advertising_adv_data_update(UINT8_T CONST *p_ad_data, UINT8_T ad_len)
{
    TAL_BLE_DATA_T adv_data = {0};
    adv_data.len = ad_len;
    adv_data.p_data = (VOID_T *)p_ad_data;
    return tal_ble_advertising_data_update(&adv_data, NULL);
}

tuya_ble_status_t tuya_ble_gap_advertising_scan_rsp_data_update(UINT8_T CONST *p_sr_data, UINT8_T sr_len)
{
    TAL_BLE_DATA_T scan_rsp = {0};
    scan_rsp.len = sr_len;
    scan_rsp.p_data = (VOID_T *)p_sr_data;
    return tal_ble_advertising_data_update(NULL, &scan_rsp);
}

tuya_ble_status_t tuya_ble_gap_disconnect(VOID_T)
{
    TAL_BLE_PEER_INFO_T info = {0};
    extern UINT16_T tuya_app_get_conn_handle(VOID_T);
    info.conn_handle = tuya_app_get_conn_handle();
    return tal_ble_disconnect(info);
}

tuya_ble_status_t tuya_ble_gap_addr_get(tuya_ble_gap_addr_t *p_addr)
{
    TAL_BLE_ADDR_T addr = {0};
    UINT32_T ret = tal_ble_address_get(&addr);

    p_addr->addr_type = (tuya_ble_addr_type_t)addr.type;
    memcpy(p_addr->addr, addr.addr, 6);
    return ret;
}

tuya_ble_status_t tuya_ble_gap_addr_set(tuya_ble_gap_addr_t *p_addr)
{
    TAL_BLE_ADDR_T addr = {0};
    addr.type = (TAL_BLE_ADDR_TYPE_E)p_addr->addr_type;
    memcpy(addr.addr, p_addr->addr, 6);
    return tal_ble_address_set(&addr);
}

tuya_ble_status_t tuya_ble_gatt_send_data(CONST UINT8_T *p_data, UINT16_T len)
{
    UINT8_T data_len = len;
    if (data_len > TUYA_BLE_DATA_MTU_MAX) {
        data_len = TUYA_BLE_DATA_MTU_MAX;
    }

    TAL_BLE_DATA_T data = {0};
    data.len = data_len;
    data.p_data = (VOID_T *)p_data;
    return tal_ble_server_common_send(&data);
}

TUYA_WEAK_ATTRIBUTE tuya_ble_status_t tuya_ble_device_info_characteristic_value_update(UINT8_T CONST *p_data, UINT8_T data_len)
{
    TAL_BLE_DATA_T data = {0};
    data.len = data_len;
    data.p_data = (VOID_T *)p_data;
    tal_ble_server_common_read_update(&data);
    return TUYA_BLE_SUCCESS;
}

TUYA_WEAK_ATTRIBUTE tuya_ble_status_t tuya_ble_long_range_ext_adv_data_update(UINT8_T CONST *p_data, UINT8_T data_len)
{
    return TUYA_BLE_SUCCESS;
}

TUYA_WEAK_ATTRIBUTE tuya_ble_status_t tuya_ble_link_security_request(void)
{
    return TUYA_BLE_ERR_COMMON;
}

tuya_ble_status_t tuya_ble_common_uart_init(VOID_T)
{
    return TUYA_BLE_ERR_COMMON;
}

tuya_ble_status_t tuya_ble_common_uart_send_data(CONST UINT8_T *p_data, UINT16_T len)
{
    return tal_uart_write(0, (VOID_T *)p_data, len);
}

tuya_ble_status_t tuya_ble_timer_create(VOID_T **p_timer_id, UINT32_T timeout_value_ms, tuya_ble_timer_mode mode, tuya_ble_timer_handler_t timeout_handler)
{
    UINT8_T i = 0;
    for (; i < TY_TIMER_MAX_NUM; i++) {
        if (ty_timer_pool[i].is_occupy == 0) {
            ty_timer_pool[i].is_occupy = 1;
            ty_timer_pool[i].ms = timeout_value_ms;
            ty_timer_pool[i].mode = mode;
            break;
        }
    }

    UINT32_T ret = tal_sw_timer_create((TAL_TIMER_CB)timeout_handler, NULL, p_timer_id);
    if (ret == 0) {
        ty_timer_pool[i].timer_id = *p_timer_id;
    }

    return ret;
}

tuya_ble_status_t tuya_ble_timer_delete(VOID_T *timer_id)
{
    OPERATE_RET ret = tal_sw_timer_delete(timer_id);
    if (ret == OPRT_OK) {
        for (UINT8_T i = 0; i < TY_TIMER_MAX_NUM; i++) {
            if (ty_timer_pool[i].timer_id == timer_id && ty_timer_pool[i].is_occupy == 1) {
                memset(&ty_timer_pool[i], 0, SIZEOF(ty_timer_item_t));
                break;
            }
        }
    }

    return ret;
}

tuya_ble_status_t tuya_ble_timer_start(VOID_T *timer_id)
{
    for (UINT8_T i = 0; i < TY_TIMER_MAX_NUM; i++) {
        if (ty_timer_pool[i].timer_id == timer_id) {
            return tal_sw_timer_start(timer_id, ty_timer_pool[i].ms, (TIMER_TYPE)ty_timer_pool[i].mode);
        }
    }
    return TUYA_BLE_ERR_NOT_FOUND;
}

tuya_ble_status_t tuya_ble_timer_restart(VOID_T *timer_id, UINT32_T timeout_value_ms)
{
    return tuya_ble_timer_start(timer_id);
}

tuya_ble_status_t tuya_ble_timer_stop(VOID_T *timer_id)
{
    return tal_sw_timer_stop(timer_id);
}

VOID_T tuya_ble_device_delay_ms(UINT32_T ms)
{
    tal_system_delay(ms);
}

VOID_T tuya_ble_device_delay_us(UINT32_T us)
{
    return;
}

tuya_ble_status_t tuya_ble_rand_generator(UINT8_T *p_buf, UINT8_T len)
{
    UINT32_T cnt = len / 4;
    UINT8_T  remain = len % 4;
    INT32_T  temp;

    for (UINT32_T i = 0; i < cnt; i++) {
        temp = tal_system_get_random(0xFFFFFFFF);
        memcpy(p_buf, (UINT8_T *)&temp, 4);
        p_buf += 4;
    }
    temp = tal_system_get_random(0xFFFFFFFF);
    memcpy(p_buf, (UINT8_T *)&temp, remain);

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_device_reset(VOID_T)
{
    tal_system_reset();
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_rtc_get_timestamp(UINT32_T *timestamp, INT32_T *timezone)
{
    tal_rtc_time_get(timestamp);
    *timezone = tal_utc_get_time_zone();
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_rtc_set_timestamp(UINT32_T timestamp, INT32_T timezone)
{
    tal_rtc_time_set(timestamp);
    tal_utc_set_time_zone(timezone);
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_nv_init(VOID_T)
{
    return TUYA_BLE_ERR_COMMON;
}

tuya_ble_status_t tuya_ble_nv_erase(UINT32_T addr, UINT32_T size)
{
    return tal_flash_erase(addr, size);
}

tuya_ble_status_t tuya_ble_nv_write(UINT32_T addr, CONST UINT8_T *p_data, UINT32_T size)
{
    return tal_flash_write(addr, (VOID_T *)p_data, size);
}

tuya_ble_status_t tuya_ble_nv_read(UINT32_T addr, UINT8_T *p_data, UINT32_T size)
{
    return tal_flash_read(addr, (VOID_T *)p_data, size);
}

#if TUYA_BLE_USE_OS

BOOL_T tuya_ble_os_task_create(VOID_T **pp_handle, CONST char *p_name, VOID_T(*p_routine)(VOID_T *), VOID_T *p_param, UINT16_T stack_size, UINT16_T priority)
{
    if (tkl_thread_create(pp_handle, p_name, stack_size, priority, p_routine, p_param) == OPRT_OK) {
        return TRUE;
    }
    return FALSE;
}

BOOL_T tuya_ble_os_task_delete(VOID_T *p_handle)
{
    if (tkl_thread_release(p_handle) == OPRT_OK) {
        return TRUE;
    }
    return FALSE;
}

BOOL_T tuya_ble_os_task_suspend(VOID_T *p_handle)
{
    return FALSE;
}

BOOL_T tuya_ble_os_task_resume(VOID_T *p_handle)
{
    return FALSE;
}

BOOL_T tuya_ble_os_msg_queue_create(VOID_T **pp_handle, UINT32_T msg_num, UINT32_T msg_size)
{
    if (tkl_queue_create_init(pp_handle, msg_size, msg_num) == OPRT_OK) {
        return TRUE;
    }
    return FALSE;
}

BOOL_T tuya_ble_os_msg_queue_delete(VOID_T *p_handle)
{
    return FALSE;
}

BOOL_T tuya_ble_os_msg_queue_peek(VOID_T *p_handle, UINT32_T *p_msg_num)
{
    return FALSE;
}

BOOL_T tuya_ble_os_msg_queue_send(VOID_T *p_handle, VOID_T *p_msg, UINT32_T wait_ms)
{
    return tkl_queue_post(p_handle, p_msg, wait_ms);
}

BOOL_T tuya_ble_os_msg_queue_recv(VOID_T *p_handle, VOID_T *p_msg, UINT32_T wait_ms)
{
    return tkl_queue_fetch(p_handle, p_msg, wait_ms);
}

BOOL_T tuya_ble_event_queue_send_port(tuya_ble_evt_param_t *evt, UINT32_T wait_ms)
{
    return FALSE;
}

#endif

TUYA_WEAK_ATTRIBUTE BOOL_T tuya_ble_aes128_ecb_encrypt(UINT8_T *key, UINT8_T *input, UINT16_T input_len, UINT8_T *output)
{
#if !defined(ENABLE_PLATFORM_USE_LIGHTWEIGHT_AES_COMPONENT) || (ENABLE_PLATFORM_USE_LIGHTWEIGHT_AES_COMPONENT == 0)
    UINT16_T length;
    mbedtls_aes_context aes_ctx;
    //
    if (input_len % 16) {
        return FALSE;
    }

    length = input_len;

    mbedtls_aes_init(&aes_ctx);

    mbedtls_aes_setkey_enc(&aes_ctx, key, 128);

    while (length > 0) {
        mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_ENCRYPT, input, output);
        input  += 16;
        output += 16;
        length -= 16;
    }

    mbedtls_aes_free(&aes_ctx);

    return TRUE;
#else
    uint16_t length, i;
    uint8_t *input_buffer_ptr = sg_input_buffer;
    //
    if (input_len % 16) {
        return false;
    }

    length = input_len;
    memcpy(input_buffer_ptr, input, input_len);

    for (i = 0; i < length; i += 16) {
        AES128_ECB_encrypt(input_buffer_ptr, key, output);
        input_buffer_ptr += 16;
        output += 16;
    }

    return true;
#endif
}

TUYA_WEAK_ATTRIBUTE BOOL_T tuya_ble_aes128_ecb_decrypt(UINT8_T *key, UINT8_T *input, UINT16_T input_len, UINT8_T *output)
{
#if !defined(ENABLE_PLATFORM_USE_LIGHTWEIGHT_AES_COMPONENT) || (ENABLE_PLATFORM_USE_LIGHTWEIGHT_AES_COMPONENT == 0)
    UINT16_T length;
    mbedtls_aes_context aes_ctx;

    if (input_len % 16) {
        return FALSE;
    }

    length = input_len;

    mbedtls_aes_init(&aes_ctx);
    mbedtls_aes_setkey_dec(&aes_ctx, key, 128);

    while (length > 0) {
        mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_DECRYPT, input, output);
        input  += 16;
        output += 16;
        length -= 16;
    }

    mbedtls_aes_free(&aes_ctx);

    return TRUE;
#else
    uint16_t length, i;
    uint8_t *input_buffer_ptr = sg_input_buffer;
    //
    if (input_len % 16) {
        return false;
    }

    length = input_len;
    memcpy(input_buffer_ptr, input, input_len);

    for (i = 0; i < length; i += 16) {
        AES128_ECB_decrypt(input_buffer_ptr, key, output);
        input_buffer_ptr += 16;
        output += 16;
    }

    return true;
#endif
}

TUYA_WEAK_ATTRIBUTE BOOL_T tuya_ble_aes128_cbc_encrypt(UINT8_T *key, UINT8_T *iv, UINT8_T *input, UINT16_T input_len, UINT8_T *output)
{
#if !defined(ENABLE_PLATFORM_USE_LIGHTWEIGHT_AES_COMPONENT) || (ENABLE_PLATFORM_USE_LIGHTWEIGHT_AES_COMPONENT == 0)
    mbedtls_aes_context aes_ctx;

    if (input_len % 16) {
        return FALSE;
    }

    mbedtls_aes_init(&aes_ctx);
    mbedtls_aes_setkey_enc(&aes_ctx, key, 128);
    mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_ENCRYPT, input_len, iv, input, output);
    mbedtls_aes_free(&aes_ctx);

    return TRUE;
#else
    if (input_len % 16) {
        return false;
    }

    memcpy(sg_input_buffer, input, input_len);

    AES128_CBC_encrypt_buffer(output, sg_input_buffer, input_len, key, iv);

    return true;
#endif
}

TUYA_WEAK_ATTRIBUTE BOOL_T tuya_ble_aes128_cbc_decrypt(UINT8_T *key, UINT8_T *iv, UINT8_T *input, UINT16_T input_len, UINT8_T *output)
{
#if !defined(ENABLE_PLATFORM_USE_LIGHTWEIGHT_AES_COMPONENT) || (ENABLE_PLATFORM_USE_LIGHTWEIGHT_AES_COMPONENT == 0)
    mbedtls_aes_context aes_ctx;

    if (input_len % 16) {
        return FALSE;
    }

    mbedtls_aes_init(&aes_ctx);
    mbedtls_aes_setkey_dec(&aes_ctx, key, 128);
    mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_DECRYPT, input_len, iv, input, output);
    mbedtls_aes_free(&aes_ctx);

    return TRUE;
#else
    if (input_len % 16) {
        return false;
    }

    memcpy(sg_input_buffer, input, input_len);
    AES128_CBC_decrypt_buffer(output, sg_input_buffer, input_len, key, iv);

    return true;
#endif
}

TUYA_WEAK_ATTRIBUTE BOOL_T tuya_ble_md5_crypt(UINT8_T *input, UINT16_T input_len, UINT8_T *output)
{
    mbedtls_md5_context md5_ctx;
    mbedtls_md5_init(&md5_ctx);
    mbedtls_md5_starts(&md5_ctx);
    mbedtls_md5_update(&md5_ctx, input, input_len);
    mbedtls_md5_finish(&md5_ctx, output);
    mbedtls_md5_free(&md5_ctx);

    return TRUE;
}

TUYA_WEAK_ATTRIBUTE BOOL_T tuya_ble_md5_crypt_loop(mbedtls_md5_context *ctx, ENUM_MD5_CRYPE_LOOP_STEP step, UINT8_T *input, UINT16_T input_len, UINT8_T *output)
{
    int ret = -1;

    if (MD5_CRYPT_LOOP_STEP_INIT == step) {
        mbedtls_md5_init(ctx);
        ret = mbedtls_md5_starts_ret(ctx);
    } else if (MD5_CRYPT_LOOP_STEP_UPDATE == step) {
        ret = mbedtls_md5_update_ret(ctx, input, input_len);
    } else if (MD5_CRYPT_LOOP_STEP_FINISH == step) {
        ret = mbedtls_md5_finish_ret(ctx, output);
    }

    if (ret != 0) {
        mbedtls_md5_free(ctx);
    }

    return (ret == 0) ? TRUE : FALSE;
}

BOOL_T tuya_ble_sha256_crypt(CONST UINT8_T *input, UINT16_T input_len, UINT8_T *output)
{
    return FALSE;
}

VOID_T *tuya_ble_port_malloc(UINT32_T size)
{
    return NULL;
}

VOID_T tuya_ble_port_free(VOID_T *pv)
{
    return;
}

TUYA_WEAK_ATTRIBUTE BOOL_T tuya_ble_hmac_sha1_crypt(CONST UINT8_T *key, UINT32_T key_len, CONST UINT8_T *input, UINT32_T input_len, UINT8_T *output)
{
    return FALSE;
}

TUYA_WEAK_ATTRIBUTE BOOL_T tuya_ble_hmac_sha256_crypt(CONST UINT8_T *key, UINT32_T key_len, CONST UINT8_T *input, UINT32_T input_len, UINT8_T *output)
{
    return FALSE;
}

TUYA_WEAK_ATTRIBUTE tuya_ble_status_t tuya_ble_ecc_verify_secp256r1(CONST UINT8_T *p_pk, CONST UINT8_T *p_hash, UINT32_T hash_size, CONST UINT8_T *p_sig)
{
    return TUYA_BLE_ERR_COMMON;
}

TUYA_WEAK_ATTRIBUTE tuya_ble_status_t tuya_ble_ecc_sign_secp256r1(CONST UINT8_T *p_sk, CONST UINT8_T *p_data, UINT32_T data_size, UINT8_T *p_sig)
{
    return TUYA_BLE_ERR_COMMON;
}

