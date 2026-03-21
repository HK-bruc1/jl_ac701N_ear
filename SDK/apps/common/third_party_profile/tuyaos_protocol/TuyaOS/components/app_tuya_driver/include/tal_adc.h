/**
 * @file tal_adc.h
 * @brief This is tal_adc file
 * @version 1.0
 * @date 2021-08-24
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TAL_ADC_H__
#define __TAL_ADC_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

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

/**
 * @brief tuya hal adc init
 *
 * @param[in] port_num: adc port number
 * @param[in] cfg: adc config
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_adc_init(TUYA_ADC_NUM_E port_num, TUYA_ADC_BASE_CFG_T *cfg);

/**
 * @brief adc deinit
 *
 * @param[in] port_num: adc port number
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_adc_deinit(TUYA_ADC_NUM_E port_num);

/**
 * @brief adc read
 *
 * @param[in] port_num: adc port number
 * @param[out] buff: points to the list of data read from the ADC register
 * @param[out] len:  buff len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_adc_read_data(TUYA_ADC_NUM_E port_num, INT32_T *buff, UINT16_T len);

/**
 * @brief read single channel
 *
 * @param[in] port_num: adc port number
 * @param[in] ch_num: channel number in one unit
 * @param[out] buf: convert result buffer
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 *
 */
OPERATE_RET tal_adc_read_single_channel(TUYA_ADC_NUM_E port_num, UINT8_T ch_num, INT32_T *buf);

/**
 * @brief get adc width
 *
 * @param[in] port_num: adc port number
 *
 * @return adc width
 */
UINT8_T tal_adc_width_get(TUYA_ADC_NUM_E port_num);

/**
 * @brief adc get temperature
 *
 * @return temperature(bat: 'C)
 */
INT32_T tal_adc_temperature_get(VOID_T);

/**
 * @brief get adc reference voltage
 *
 * @param[in] port: adc port
 *
 * @return adc reference voltage(bat: mv)
 */
UINT32_T tal_adc_ref_voltage_get(TUYA_ADC_NUM_E port_num);

/**
 * @brief read voltage
 *
 * @param[in] port_num: adc port number
 * @param[out] buff: convert voltage, voltage range to -vref - +vref
 * @param[out] len: length of buffer
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 *
 */
OPERATE_RET tal_adc_read_voltage(TUYA_ADC_NUM_E port_num, INT32_T *buff, UINT16_T len);


#ifdef __cplusplus
}
#endif

#endif /* __TAL_ADC_H__ */

