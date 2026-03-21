/**
 * @file tkl_uart.c
 * @brief This is tkl_uart file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "vendor/jl_ble/tuyaos/include/board.h"
#include "tkl_uart.h"
#include "tkl_gpio.h"

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




OPERATE_RET tkl_uart_init(TUYA_UART_NUM_E port_id, TUYA_UART_BASE_CFG_T *cfg)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_uart_deinit(TUYA_UART_NUM_E port_id)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

INT32_T tkl_uart_write(TUYA_UART_NUM_E port_id, VOID_T *buff, UINT16_T len)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

VOID_T tkl_uart_rx_irq_cb_reg(TUYA_UART_NUM_E port_id, TUYA_UART_IRQ_CB rx_cb)
{
}

VOID_T tkl_uart_tx_irq_cb_reg(TUYA_UART_NUM_E port_id, TUYA_UART_IRQ_CB tx_cb)
{
}

INT32_T tkl_uart_read(TUYA_UART_NUM_E port_id, VOID_T *buff, UINT16_T len)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_uart_set_tx_int(TUYA_UART_NUM_E port_id, BOOL_T enable)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_uart_set_rx_flowctrl(TUYA_UART_NUM_E port_id, BOOL_T enable)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_uart_wait_for_data(TUYA_UART_NUM_E port_id, INT32_T timeout_ms)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_uart_ioctl(TUYA_UART_NUM_E port_id, UINT32_T cmd, VOID *arg)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

