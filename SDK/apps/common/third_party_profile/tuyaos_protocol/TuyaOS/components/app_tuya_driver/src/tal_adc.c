/**
 * @file tal_adc.c
 * @brief This is tal_adc file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tkl_adc.h"
#include "tal_adc.h"

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




TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_adc_init(TUYA_ADC_NUM_E port_num, TUYA_ADC_BASE_CFG_T *cfg)
{
    return tkl_adc_init(port_num, cfg);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_adc_deinit(TUYA_ADC_NUM_E port_num)
{
    return tkl_adc_deinit(port_num);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_adc_read_data(TUYA_ADC_NUM_E port_num, INT32_T *buff, UINT16_T len)
{
    return tkl_adc_read_data(port_num, buff, len);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_adc_read_single_channel(TUYA_ADC_NUM_E port_num, UINT8_T ch_num, INT32_T *buf)
{
    return tkl_adc_read_single_channel(port_num, ch_num, buf);
}

TUYA_WEAK_ATTRIBUTE UINT8_T tal_adc_width_get(TUYA_ADC_NUM_E port_num)
{
    return tkl_adc_width_get(port_num);
}

TUYA_WEAK_ATTRIBUTE INT32_T tal_adc_temperature_get(VOID_T)
{
    return tkl_adc_temperature_get();
}

TUYA_WEAK_ATTRIBUTE UINT32_T tal_adc_ref_voltage_get(TUYA_ADC_NUM_E port_num)
{
    return tkl_adc_ref_voltage_get(port_num);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_adc_read_voltage(TUYA_ADC_NUM_E port_num, INT32_T *buff, UINT16_T len)
{
    return tkl_adc_read_voltage(port_num, buff, len);
}

