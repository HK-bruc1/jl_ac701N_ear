/**
 * @file tal_i2c.c
 * @brief This is tal_i2c file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tkl_i2c.h"
#include "tal_i2c.h"

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




TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_i2c_init(TUYA_I2C_NUM_E port_num, TUYA_IIC_BASE_CFG_T *cfg)
{
    return tkl_i2c_init(port_num, cfg);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_i2c_control(TUYA_I2C_NUM_E port_num, UINT8_T cmd, VOID_T *arg)
{
    return OPRT_NOT_SUPPORTED;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_i2c_deinit(TUYA_I2C_NUM_E port_num)
{
    return tkl_i2c_deinit(port_num);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_i2c_master_send(TUYA_I2C_NUM_E port_num, UINT16_T addr, VOID_T *buf, UINT16_T count)
{
    return tkl_i2c_master_send(port_num, addr, buf, count, TRUE);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_i2c_master_receive(TUYA_I2C_NUM_E port_num, UINT16_T addr, VOID_T *buf, UINT16_T count)
{
    return tkl_i2c_master_receive(port_num, addr, buf, count, TRUE);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_i2c_irq_init(TUYA_I2C_NUM_E port_num, CONST TUYA_I2C_IRQ_CB cb)
{
    return tkl_i2c_irq_init(port_num, cb);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_i2c_irq_enable(TUYA_I2C_NUM_E port_num)
{
    return tkl_i2c_irq_enable(port_num);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_i2c_irq_disable(TUYA_I2C_NUM_E port_num)
{
    return tkl_i2c_irq_disable(port_num);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_i2c_set_slave_addr(TUYA_I2C_NUM_E port_num, UINT16_T dev_addr)
{
    return tkl_i2c_set_slave_addr(port_num, dev_addr);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_i2c_slave_send(TUYA_I2C_NUM_E port_num, CONST VOID *data, UINT32_T size)
{
    return tkl_i2c_slave_send(port_num, data, size);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_i2c_slave_receive(TUYA_I2C_NUM_E port_num, VOID *data, UINT32_T size)
{
    return tkl_i2c_slave_receive(port_num, data, size);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_i2c_get_status(TUYA_I2C_NUM_E port_num, TUYA_IIC_STATUS_T *status)
{
    return tkl_i2c_get_status(port_num, status);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_i2c_reset(TUYA_I2C_NUM_E port_num)
{
    return tkl_i2c_reset(port_num);
}

