/**
 * @file tal_gpio.c
 * @brief This is tal_gpio file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tkl_gpio.h"
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


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_gpio_init(TUYA_GPIO_NUM_E pin_id, TUYA_GPIO_BASE_CFG_T *cfg)
{
    return tkl_gpio_init(pin_id, cfg);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_gpio_deinit(TUYA_GPIO_NUM_E pin_id)
{
    return tkl_gpio_deinit(pin_id);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_gpio_write(TUYA_GPIO_NUM_E pin_id, TUYA_GPIO_LEVEL_E level)
{
    return tkl_gpio_write(pin_id, level);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_gpio_read(TUYA_GPIO_NUM_E pin_id, TUYA_GPIO_LEVEL_E *level)
{
    return tkl_gpio_read(pin_id, level);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_gpio_irq_init(TUYA_GPIO_NUM_E pin_id, TUYA_GPIO_IRQ_T *cfg)
{
    return tkl_gpio_irq_init(pin_id, cfg);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_gpio_irq_enable(TUYA_GPIO_NUM_E pin_id)
{
    return tkl_gpio_irq_enable(pin_id);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_gpio_irq_disable(TUYA_GPIO_NUM_E pin_id)
{
    return tkl_gpio_irq_disable(pin_id);
}

