/**
 * @file tal_key.c
 * @brief This is tal_key file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tal_util.h"
#include "tal_key.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define KEY_CHECK_TOTAL        5
#define KEY_CHECK_RELEASE      3

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




TUYA_WEAK_ATTRIBUTE UINT32_T tal_key_get_pin_level(UINT32_T pin)
{
    return 0;
}

UINT32_T tal_key_init(tal_key_param_t *param)
{
    //Param check
    for (UINT32_T idx = 0; idx < param->count_len; idx++) {
        if (param->count_array[idx] == 0) {
            return 1;
        }
    }
    return 0;
}

UINT32_T tal_key_timeout_handler(tal_key_param_t *param)
{
    BOOL_T key_pressed = true;

    param->inter.count++;

    param->inter.check_idx++;
    if (param->inter.check_idx == KEY_CHECK_TOTAL) {
        param->inter.check_idx = 0;
    }

    // Record the level
    if (tal_key_get_pin_level(param->pin) == param->valid_level) {
        param->inter.check_record &= ~(1 << param->inter.check_idx);
    } else {
        param->inter.check_record |= (1 << param->inter.check_idx);
    }
    // Invalid times are greater than N
    if (tal_util_count_one_in_num(param->inter.check_record) >= KEY_CHECK_RELEASE) {
        key_pressed = false;
    }

    if (key_pressed == true) {
        for (UINT32_T idx = 0; idx < param->count_len; idx++) {
            if (param->inter.count == param->count_array[idx]) {
                param->inter.state = idx + 1;

                if (idx < (param->count_len - 1)) {
                    param->handler(param->inter.state);
                }
                break;
            }
        }
    } else {
        for (UINT32_T idx = 0; idx < param->count_len; idx++) {
            if (idx == 0) {
                if (param->inter.count <= param->count_array[idx]) {
                    param->inter.state = idx + 1 + param->count_len;
                    break;
                }
            } else {
                if ((param->inter.count > param->count_array[idx - 1]) \
                    && (param->inter.count <= param->count_array[idx])) {
                    param->inter.state = idx + 1 + param->count_len;
                    break;
                }
            }
        }
    }

    // Release
    if (param->inter.state >= param->count_len) {
        param->handler(param->inter.state);

        param->inter.count = 0;
        param->inter.check_idx = 0;
        param->inter.check_record = 0;
        param->inter.state = 0;
        return 0;
    }
    return 1;
}

