/**
 * @file tkl_i2c.c
 * @brief This is tkl_i2c file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "vendor/jl_ble/tuyaos/include/board.h"
#include "tkl_i2c.h"

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




OPERATE_RET tkl_i2c_init(TUYA_I2C_NUM_E port, CONST TUYA_IIC_BASE_CFG_T *cfg)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_i2c_deinit(TUYA_I2C_NUM_E port)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_i2c_irq_init(TUYA_I2C_NUM_E port, TUYA_I2C_IRQ_CB cb)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_i2c_irq_enable(TUYA_I2C_NUM_E port)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_i2c_irq_disable(TUYA_I2C_NUM_E port)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_i2c_master_send(TUYA_I2C_NUM_E port, UINT16_T dev_addr, CONST VOID_T *data, UINT32_T size, BOOL_T xfer_pending)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_i2c_master_receive(TUYA_I2C_NUM_E port, UINT16_T dev_addr, VOID *data, UINT32_T size, BOOL_T xfer_pending)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_i2c_set_slave_addr(TUYA_I2C_NUM_E port, UINT16_T dev_addr)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_i2c_slave_send(TUYA_I2C_NUM_E port, CONST VOID *data, UINT32_T size)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_i2c_slave_receive(TUYA_I2C_NUM_E port, VOID *data, UINT32_T size)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_i2c_get_status(TUYA_I2C_NUM_E port, TUYA_IIC_STATUS_T *status)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_i2c_reset(TUYA_I2C_NUM_E port)
{
    TY_PRINTF("%s", __FUNCTION__);
    return OPRT_NOT_SUPPORTED;
}

