/**
 * @file tal_uart.c
 * @brief This is tal_uart file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tkl_uart.h"
#include "tkl_memory.h"
#include "tal_sw_timer.h"
#include "tal_uart.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/


/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
#if (TAL_UART_MODE_CACHE == 1)
typedef struct {
    BOOL_T          is_init;
    TIMER_ID        timer_id;
    UINT32_T        rx_count;
    UINT8_T         *p_rx_buf;
    UINT32_T        port_id;
    TAL_UART_CFG_T  uart_cfg;
    TAL_UART_IRQ_CB rx_cb;
} TAL_UART_CACHE_T;
#else
typedef struct {
    BOOL_T          is_init;
    UINT8_T         *p_rx_buf;
    TAL_UART_CFG_T  uart_cfg;
    TAL_UART_IRQ_CB rx_cb;
} TAL_UART_T;
#endif

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
#if (TAL_UART_MODE_CACHE == 1)
STATIC TAL_UART_CACHE_T uart_cache[TAL_UART_CACHE_NUM] = {0};
#else
STATIC TAL_UART_T uart[TAL_UART_NUM] = {0};
#endif

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




STATIC VOID_T tkl_uart_rx_irq_cb(TUYA_UART_NUM_E port_id)
{
#if (TAL_UART_MODE_CACHE == 1)
    TAL_UART_CACHE_T *p_cache = &uart_cache[port_id];
    UINT32_T rx_buffer_size = p_cache->uart_cfg.rx_buffer_size;

    p_cache->port_id = port_id;

    if (p_cache->rx_count < rx_buffer_size) {
        UINT32_T buffer_len = rx_buffer_size - p_cache->rx_count;
        UINT32_T read_len = tkl_uart_read(port_id, &p_cache->p_rx_buf[p_cache->rx_count], buffer_len);

        p_cache->rx_count += read_len;
        if (p_cache->rx_count < rx_buffer_size) {
            tal_sw_timer_start(p_cache->timer_id, 20, TAL_TIMER_ONCE);
        } else {
            tal_sw_timer_start(p_cache->timer_id, 0, TAL_TIMER_ONCE);
        }
    } else {
        //If the buffer is exceeded, data will still be lost.
        //Double-buffering may solve the problem, but for practical purposes, larger buffer is a better solution.
    }
#else
    UINT32_T read_len = tkl_uart_read(port_id, uart[port_id].p_rx_buf, uart[port_id].uart_cfg.rx_buffer_size);
    if (uart[port_id].rx_cb != NULL) {
        uart[port_id].rx_cb(port_id, uart[port_id].p_rx_buf, read_len);
    }
#endif
}

#if (TAL_UART_MODE_CACHE == 1)

STATIC VOID_T tkl_uart_timeout_handler(TIMER_ID timer_id, VOID_T *arg)
{
    for (UINT32_T idx = 0; idx < TAL_UART_CACHE_NUM; idx++) {
        TAL_UART_CACHE_T *p_cache = &uart_cache[idx];
        if (p_cache->timer_id == timer_id) {
            if (p_cache->rx_cb != NULL) {
                if (p_cache->rx_count > 0) {
                    p_cache->rx_cb((TUYA_UART_NUM_E)p_cache->port_id, p_cache->p_rx_buf, p_cache->rx_count);

                    p_cache->rx_count = 0;
                }
            }
        }
    }


}

#endif

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_uart_init(TUYA_UART_NUM_E port_id, TAL_UART_CFG_T *cfg)
{
    OPERATE_RET ret = OPRT_OK;

    if (cfg == NULL) {
        return OPRT_INVALID_PARM;
    }

    if (cfg->rx_buffer_size == 0) {
        return OPRT_INVALID_PARM;
    }

#if (TAL_UART_MODE_CACHE == 1)
    if (uart_cache[port_id].is_init) {
        return OPRT_INIT_MORE_THAN_ONCE;
    }

    uart_cache[port_id].p_rx_buf = tkl_system_malloc(cfg->rx_buffer_size);
    if (uart_cache[port_id].p_rx_buf == NULL) {
        return OPRT_MALLOC_FAILED;
    }

    memcpy(&uart_cache[port_id].uart_cfg, cfg, sizeof(TAL_UART_CFG_T));

    ret = tkl_uart_init(port_id, &uart_cache[port_id].uart_cfg.base_cfg);
    if (ret == OPRT_OK) {
        uart_cache[port_id].is_init = TRUE;
        tkl_uart_rx_irq_cb_reg(port_id, tkl_uart_rx_irq_cb);
        tal_sw_timer_create(tkl_uart_timeout_handler, NULL, &uart_cache[port_id].timer_id);
    }
#else
    if (uart[port_id].is_init) {
        return OPRT_INIT_MORE_THAN_ONCE;
    }

    uart[port_id].p_rx_buf = tkl_system_malloc(cfg->rx_buffer_size);
    if (uart[port_id].p_rx_buf == NULL) {
        return OPRT_MALLOC_FAILED;
    }

    memcpy(&uart[port_id].uart_cfg, cfg, sizeof(TAL_UART_CFG_T));

    ret = tkl_uart_init(port_id, &uart[port_id].uart_cfg.base_cfg);
    if (ret == OPRT_OK) {
        uart[port_id].is_init = TRUE;
        tkl_uart_rx_irq_cb_reg(port_id, tkl_uart_rx_irq_cb);
    }
#endif
    return ret;
}

TUYA_WEAK_ATTRIBUTE INT32_T tal_uart_read(TUYA_UART_NUM_E port_id, UINT8_T *data, UINT32_T len)
{
    return OPRT_NOT_SUPPORTED;
}

TUYA_WEAK_ATTRIBUTE INT32_T tal_uart_write(TUYA_UART_NUM_E port_id, CONST UINT8_T *data, UINT32_T len)
{
    OPERATE_RET ret = OPRT_OK;
    ret = tkl_uart_write(port_id, (UINT8_T *)data, len);
    return ret;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_uart_deinit(TUYA_UART_NUM_E port_id)
{
    OPERATE_RET ret = OPRT_OK;

#if (TAL_UART_MODE_CACHE == 1)
    if (uart_cache[port_id].p_rx_buf) {
        tkl_system_free(uart_cache[port_id].p_rx_buf);
        uart_cache[port_id].p_rx_buf = NULL;
    }

    ret = tkl_uart_deinit(port_id);
    if (ret == OPRT_OK) {
        uart_cache[port_id].is_init = FALSE;
        tal_sw_timer_delete(uart_cache[port_id].timer_id);
    }
#else
    if (uart[port_id].p_rx_buf) {
        tkl_system_free(uart[port_id].p_rx_buf);
        uart[port_id].p_rx_buf = NULL;
    }

    ret = tkl_uart_deinit(port_id);
    if (ret == OPRT_OK) {
        uart[port_id].is_init = FALSE;
    }
#endif

    return ret;
}

TUYA_WEAK_ATTRIBUTE VOID_T tal_uart_rx_reg_irq_cb(TUYA_UART_NUM_E port_id, TAL_UART_IRQ_CB rx_cb)
{
#if (TAL_UART_MODE_CACHE == 1)
    uart_cache[port_id].rx_cb = rx_cb;
#else
    uart[port_id].rx_cb = rx_cb;
#endif
}

