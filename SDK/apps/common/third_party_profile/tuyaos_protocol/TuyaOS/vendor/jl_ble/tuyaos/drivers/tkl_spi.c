/**
 * @file tkl_spi.c
 * @brief This is tkl_spi file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "vendor/jl_ble/tuyaos/include/board.h"
#include "tkl_spi.h"
#include "tkl_memory.h"

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




OPERATE_RET tkl_spi_init(TUYA_SPI_NUM_E port, CONST TUYA_SPI_BASE_CFG_T *cfg)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_spi_deinit(TUYA_SPI_NUM_E port)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_spi_send(TUYA_SPI_NUM_E port, VOID_T *data, UINT16_T size)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_spi_recv(TUYA_SPI_NUM_E port, VOID_T *data, UINT16_T size)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_spi_transfer(TUYA_SPI_NUM_E port, VOID_T *send_buf, VOID_T *receive_buf, UINT32_T length)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_spi_transfer_with_length(TUYA_SPI_NUM_E port, VOID_T *send_buf, UINT32_T send_len, VOID_T *receive_buf, UINT32_T receive_len)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_spi_abort_transfer(TUYA_SPI_NUM_E port)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_spi_get_status(TUYA_SPI_NUM_E port, TUYA_SPI_STATUS_T *status)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_spi_irq_init(TUYA_SPI_NUM_E port, TUYA_SPI_IRQ_CB cb)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_spi_irq_enable(TUYA_SPI_NUM_E port)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_spi_irq_disable(TUYA_SPI_NUM_E port)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

INT32_T tkl_spi_get_data_count(TUYA_SPI_NUM_E port)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_spi_ioctl(TUYA_SPI_NUM_E port, UINT32_T cmd, VOID_T *args)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

