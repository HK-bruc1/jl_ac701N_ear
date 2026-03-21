/**
 * @file tal_util.h
 * @brief This is tal_util file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TAL_UTIL_H__
#define __TAL_UTIL_H__

#include "tuya_cloud_types.h"
#include "tuya_error_code.h"

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
 * @brief tal_util_check_sum8
 *
 * @param[in] buf: buf
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tal_util_check_sum8(UINT8_T *buf, UINT32_T size);

/**
 * @brief tal_util_check_sum16
 *
 * @param[in] buf: buf
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT16_T tal_util_check_sum16(UINT8_T *buf, UINT32_T size);

/**
 * @brief tal_util_crc8
 *
 * @param[in] buf: buf
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tal_util_crc8(UINT8_T *buf, UINT32_T size);

/**
 * @brief tal_util_crc16
 *
 * @param[in] buf: buf
 * @param[in] size: size
 * @param[in] p_crc: p_crc
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT16_T tal_util_crc16(UINT8_T *buf, UINT32_T size, UINT16_T *p_crc);

/**
 * @brief tal_util_crc32
 *
 * @param[in] buf: buf
 * @param[in] size: size
 * @param[in] p_crc: p_crc
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tal_util_crc32(UINT8_T *buf, UINT32_T size, UINT32_T *p_crc);

/**
 * @brief tal_util_xor
 *
 * @param[in] buf1: buf1
 * @param[in] buf2: buf2
 * @param[in] size: size
 * @param[in] out_buf: out_buf
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tal_util_xor(UINT8_T *buf1, UINT8_T *buf2, UINT32_T size, UINT8_T *out_buf);

/**
 * @brief tal_util_intarray2int
 *
 * @param[in] intArray: intArray
 * @param[in] startIdx: startIdx
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tal_util_intarray2int(UINT8_T *intArray, UINT32_T startIdx, UINT32_T size);

/**
 * @brief tal_util_int2intarray
 *
 * @param[in] num: num
 * @param[in] intArray: intArray
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tal_util_int2intarray(UINT32_T num, UINT8_T *intArray, UINT32_T size);

/**
 * @brief tal_util_device_id_20_to_16
 *
 * @param[in] *in: *in
 * @param[out] *out: *out
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tal_util_device_id_20_to_16(UINT8_T *in, UINT8_T *out);

/**
 * @brief tal_util_device_id_16_to_20
 *
 * @param[in] *in: *in
 * @param[out] *out: *out
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tal_util_device_id_16_to_20(UINT8_T *in, UINT8_T *out);

/**
 * @brief tal_util_reverse_byte
 *
 * @param[in] buf: void* Prevent warnings when calling this API
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tal_util_reverse_byte(void *buf, UINT32_T size);

/**
 * @brief tal_util_count_one_in_num
 *
 * @param[in] num: num
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tal_util_count_one_in_num(UINT32_T num);

/**
 * @brief tal_util_buffer_value_is_all_x
 *
 * @param[in] *buf: *buf
 * @param[in] size: size
 * @param[in] x_value: x_value
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
BOOL_T tal_util_buffer_value_is_all_x(CONST UINT8_T *buf, UINT32_T size, UINT8_T x_value);

/**
 * @brief tal_util_is_word_aligned
 *
 * @param[in] p: p
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
BOOL_T tal_util_is_word_aligned(VOID_T CONST *p);

/**
 * @brief tal_util_search_symbol_index
 *
 * @param[in] *buf: *buf
 * @param[in] size: size
 * @param[in] symbol: symbol
 * @param[in] index[]: index[]
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
INT32_T tal_util_search_symbol_index(UINT8_T *buf, UINT32_T size, UINT8_T symbol, UINT8_T index[]);

/**
 * @brief tal_util_ecc_key_pem2hex
 *
 * @param[in] *pem: *pem
 * @param[in] *key: *key
 * @param[in] *key_len: *key_len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
INT32_T tal_util_ecc_key_pem2hex(const UINT8_T *pem, UINT8_T *key, UINT16_T *key_len);

/**
 * @brief tal_util_ecc_sign_secp256r1_extract_raw_from_der
 *
 * @param[in] *der: *der
 * @param[in] *raw_rs: *raw_rs
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
INT32_T tal_util_ecc_sign_secp256r1_extract_raw_from_der(const UINT8_T *der, UINT8_T *raw_rs);

/**
 * @brief tal_util_shell_sort
 *
 * @param[in] buf: buf
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tal_util_shell_sort(INT_T *buf, INT_T size);

/**
 * @brief tal_util_str_hexchar2int
 *
 * @param[in] hexChar: hexChar
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tal_util_str_hexchar2int(UINT8_T hexChar);

/**
 * @brief tal_util_str_int2hexchar
 *
 * @param[in] isHEX: isHEX
 * @param[in] intNum: intNum
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tal_util_str_int2hexchar(BOOL_T isHEX, UINT8_T intNum);

/**
 * @brief tal_util_str_hexstr2int
 *
 * @param[in] hexStr: hexStr
 * @param[in] size: size
 * @param[in] num: num
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tal_util_str_hexstr2int(UINT8_T *hexStr, UINT32_T size, UINT32_T *num);

/**
 * @brief tal_util_str_int2hexstr
 *
 * @param[in] isHEX: isHEX
 * @param[in] num: num
 * @param[in] hexStr: hexStr
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tal_util_str_int2hexstr(BOOL_T isHEX, UINT32_T num, UINT8_T *hexStr, UINT32_T size);

/**
 * @brief tal_util_str_intstr2int
 *
 * @param[in] intStr: intStr
 * @param[in] size: size
 * @param[out] num: num
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tal_util_str_intstr2int(UINT8_T *intStr, UINT32_T size, UINT32_T *num);

/**
 * @brief tal_util_str_intstr2int_with_negative
 *
 * @param[in] intStr: intStr
 * @param[in] size: size
 * @param[out] num: num
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tal_util_str_intstr2int_with_negative(CHAR_T *intStr, UINT32_T size, INT32_T *num);

/**
 * @brief tal_util_str_int2intstr
 *
 * @param[in] num: num
 * @param[in] intStr: intStr
 * @param[in] size: size
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tal_util_str_int2intstr(UINT32_T num, UINT8_T *intStr, UINT32_T size);

/**
 * @brief tal_util_str_hexstr2hexarray
 *
 * @param[in] hexStr: hexStr
 * @param[in] size: size
 * @param[in] hexArray: hexArray
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tal_util_str_hexstr2hexarray(UINT8_T *hexStr, UINT32_T size, UINT8_T *hexArray);

/**
 * @brief tal_util_str_hexarray2hexstr
 *
 * @param[in] isHEX: isHEX
 * @param[in] hexArray: hexArray
 * @param[in] size: size
 * @param[in] hexStr: hexStr
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tal_util_str_hexarray2hexstr(BOOL_T isHEX, UINT8_T *hexArray, UINT32_T size, UINT8_T *hexStr);

/**
 * @brief tal_util_get_value_by_key
 *
 * @param[in] *input_buf: *input_buf
 * @param[in] input_len: input_len
 * @param[in] *key: *key
 * @param[in] key_len: key_len
 * @param[in] *value_data: *value_data
 * @param[in] *value_len: *value_len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tal_util_get_value_by_key(UINT8_T *input_buf, UINT16_T input_len, UINT8_T *key, UINT16_T key_len, UINT8_T *value_data, UINT16_T *value_len);

/**
 * @brief tal_util_get_value_by_key_to_int
 *
 * @param[in] *input_buf: *input_buf
 * @param[in] input_len: input_len
 * @param[in] *key: *key
 * @param[in] key_len: key_len
 * @param[in] *result: *result
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tal_util_get_value_by_key_to_int(UINT8_T *input_buf, UINT16_T input_len, UINT8_T *key, UINT8_T key_len, UINT32_T *result);

/**
 * @brief tal_util_get_value_by_key_to_hex
 *
 * @param[in] *input_buf: *input_buf
 * @param[in] input_len: input_len
 * @param[in] *key: *key
 * @param[in] key_len: key_len
 * @param[in] *result: *result
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tal_util_get_value_by_key_to_hex(UINT8_T *input_buf, UINT16_T input_len, UINT8_T *key, UINT8_T key_len, UINT32_T *result);

/**
 * @brief tal_util_get_value_by_key_to_bool
 *
 * @param[in] *input_buf: *input_buf
 * @param[in] input_len: input_len
 * @param[in] *key: *key
 * @param[in] key_len: key_len
 * @param[in] *result: *result
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT8_T tal_util_get_value_by_key_to_bool(UINT8_T *input_buf, UINT16_T input_len, UINT8_T *key, UINT8_T key_len, UINT32_T *result);

/**
 * @brief tal_util_adv_report_parse
 *
 * @param[in] type: type
 * @param[in] *input_buf: *input_buf
 * @param[in] input_len: input_len
 * @param[out] **output_buf: **output_buf
 * @param[out] *output_len: *output_len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_util_adv_report_parse(UINT8_T type, UINT8_T *input_buf, UINT16_T input_len, UINT8_T **output_buf, UINT8_T *output_len);


#ifdef __cplusplus
}
#endif

#endif /* __TAL_UTIL_H__ */

