/**
 * @file tal_oled.h
 * @brief This is tal_oled file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TAL_OLED_H__
#define __TAL_OLED_H__

#include "tuya_cloud_types.h"
#include "tuya_error_code.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
//OLED
#ifndef TAL_OLED_TYPE
#define TAL_OLED_TYPE           0
#endif

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
 * @brief tal_oled_init
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_oled_init(VOID_T);

/**
 * @brief tal_oled_clear
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_oled_clear(VOID_T);

/**
 * @brief tal_oled_clear_page
 *
 * @param[in] page: page
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_oled_clear_page(UINT8_T page);

/**
 * @brief tal_oled_show_num
 *
 * @param[in] x: 0~127
 * @param[in] y: 0~63
 * @param[in] len: number of digits
 * @param[in] size: 16/12
 * @param[in] num: (0~4294967295)
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_oled_show_num(UINT8_T x, UINT8_T y, UINT32_T num, UINT8_T len, UINT8_T size);

/**
 * @brief show string(宽度为8bit)
 *
 * @param[in] x: x
 * @param[in] y: y
 * @param[in] *chr: *chr
 * @param[in] INT8_T_size: INT8_T_size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_oled_show_string(UINT8_T x, UINT8_T y, INT8_T *chr, UINT8_T INT8_T_size);

/**
 * @brief show string(宽度为6bit)
 *
 * @param[in] x: x
 * @param[in] y: y
 * @param[in] *chr: *chr
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_oled_show_string_continue(UINT8_T x, UINT8_T y, CHAR_T *chr);

/**
 * @brief tal_oled_show_mac
 *
 * @param[in] x: x
 * @param[in] y: y
 * @param[in] *mac: *mac
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_oled_show_mac(UINT8_T x, UINT8_T y, UINT8_T *mac);

/**
 * @brief tal_oled_show_rssi
 *
 * @param[in] x: x
 * @param[in] y: y
 * @param[in] rssi: rssi
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_oled_show_rssi(UINT8_T x, UINT8_T y, INT32_T rssi);

/**
 * @brief tal_oled_check_i2c_port_num
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tal_oled_check_i2c_port_num(VOID_T);

#if defined(TAL_OLED_TYPE) && (TAL_OLED_TYPE == 1)

/**
 * @brief tal_oled_show_chinese
 *
 * @param[in] x: x
 * @param[in] y: y
 * @param[in] size: chinese size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tal_oled_show_chinese(UINT8_T x, UINT8_T y, UINT8_T size, CONST UINT8_T font8X16[]);
/**
 * @brief tal_oled_show_bmp
 *
 * @param[in] x1: x1 coordinate
 * @param[in] y1: y1 coordinate
 * @param[in] x2: x2 coordinate
 * @param[in] y2: y2 coordinate
 * @param[in] BMP: picture
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tal_oled_show_bmp(UINT8_T x1, UINT8_T y1, UINT8_T x2, UINT8_T y2, const UINT8_T BMP[]);
/**
 * @brief tal_oled_show_bmp
 *
 * @param[in] x1: x1 coordinate
 * @param[in] y1: y1 coordinate
 * @param[in] x2: x2 coordinate
 * @param[in] y2: y2 coordinate
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_oled_clear_data(UINT8_T x1, UINT8_T y1, UINT8_T x2, UINT8_T y2);
/**
 * @brief tal_oled_show_group
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_oled_show_group(UINT8_T x, UINT8_T y, volatile UINT16_T group);
/**
 * @brief tal_oled_clear_position_data
 *
 * @param[in] length: 8 or 16
 * @param[in] high:   8 or 16
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_oled_clear_position_data(UINT8_T x, UINT8_T y, UINT8_T length, UINT8_T high);

/**
 * @brief tal_display_switch
 *
 * @param[in] val: 1 open the display ,0 close the display
 *
 * @return none
 */
VOID_T tal_display_switch(UINT8_T val);

#endif


#ifdef __cplusplus
}
#endif

#endif /* __TAL_OLED_H__ */

