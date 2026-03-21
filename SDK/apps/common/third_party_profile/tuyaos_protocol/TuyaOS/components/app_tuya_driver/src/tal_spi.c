/**
 * @file tal_spi.c
 * @brief This is tal_spi file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tkl_spi.h"
#include "tal_spi.h"

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




TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_spi_init(TUYA_SPI_NUM_E port_num, TUYA_SPI_BASE_CFG_T *cfg)
{
    return tkl_spi_init(port_num, cfg);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_spi_xfer(TUYA_SPI_NUM_E port_num, VOID_T *send_buf, VOID_T *recv_buf, UINT32_T length)
{
    return tkl_spi_transfer(port_num, send_buf, recv_buf, length);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_spi_xfer_with_length(TUYA_SPI_NUM_E port_num, VOID_T *send_buf, UINT32_T send_len, VOID_T *recv_buf, UINT32_T recv_len)
{
    return tkl_spi_transfer_with_length(port_num, send_buf, send_len, recv_buf, recv_len);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_spi_xfer_msg(TUYA_SPI_NUM_E port_num, TAL_SPI_MSG_T *msg, UINT8_T num)
{
    OPERATE_RET ret = 0;

    for (UINT8_T i = 0; i < num; i++) {
        ret |= tkl_spi_transfer(port_num, msg[i].send_buf, msg[i].recv_buf, msg[i].length);
    }

    return ret;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_spi_control(TUYA_SPI_NUM_E port_num, UINT8_T cmd, VOID_T *arg)
{
    return OPRT_NOT_SUPPORTED;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_spi_deinit(TUYA_SPI_NUM_E port_num)
{
    return tkl_spi_deinit(port_num);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_spi_send(TUYA_SPI_NUM_E port_num, VOID_T *data, UINT16_T size)
{
    return tkl_spi_send(port_num, data, size);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_spi_recv(TUYA_SPI_NUM_E port_num, VOID_T *data, UINT16_T size)
{
    return tkl_spi_recv(port_num, data, size);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_spi_abort_transfer(TUYA_SPI_NUM_E port_num)
{
    return tkl_spi_abort_transfer(port_num);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_spi_get_status(TUYA_SPI_NUM_E port_num, TUYA_SPI_STATUS_T *status)
{
    return tkl_spi_get_status(port_num, status);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_spi_irq_init(TUYA_SPI_NUM_E port_num, CONST TUYA_SPI_IRQ_CB cb)
{
    return tkl_spi_irq_init(port_num, cb);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_spi_irq_enable(TUYA_SPI_NUM_E port_num)
{
    return tkl_spi_irq_enable(port_num);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_spi_irq_disable(TUYA_SPI_NUM_E port_num)
{
    return tkl_spi_irq_disable(port_num);
}

