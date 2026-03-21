/**
 * @file tal_key.h
 * @brief This is tal_key file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TAL_KEY_H__
#define __TAL_KEY_H__

#include "tuya_cloud_types.h"
#include "tuya_error_code.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
typedef enum {
    TUYA_KEY_LEVEL_LOW  = 0,
    TUYA_KEY_LEVEL_HIGH = 1,
} tal_key_level_t;

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
/*
state=0 - idle
state=1 - press1 - short press
state=2 - press2 - long press
state=3 - release - long long press timeout
state=4 - release0 - release before count0
state=5 - release1 - short press release
state=6 - release2 - long press release
*/
typedef void (*tal_key_handler_t)(UINT32_T state);

typedef struct {
    UINT32_T count;
    UINT32_T check_idx;
    UINT32_T check_record;
    UINT32_T state;
} tal_key_inter_param_t;

typedef struct {
    UINT32_T pin;
    tal_key_level_t valid_level;
    UINT8_T  count_len;
    UINT32_T count_array[10];
    tal_key_handler_t handler;
    tal_key_inter_param_t inter; // Internal variables, no assignment
} tal_key_param_t;

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/
//__weak, to redefine by user

/**
 * @brief tal_key_get_pin_level
 *
 * @param[in] pin: pin
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tal_key_get_pin_level(UINT32_T pin);

/**
 * @brief tal_key_init
 *
 * @param[in] param: param
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tal_key_init(tal_key_param_t *param);

/**
 * @brief tal_key_timeout_handler
 *
 * @param[in] param: param
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tal_key_timeout_handler(tal_key_param_t *param);


#ifdef __cplusplus
}
#endif

#endif /* __TAL_KEY_H__ */

