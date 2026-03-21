/**
 * @file tal_gpio_test.c
 * @brief This is tal_gpio_test file
 * @version 1.0
 * @date 2022-06-24
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tal_gpio_test.h"
#include "tal_gpio.h"
#include "tal_util.h"

#if (TAL_GPIO_TEST_ENABLE)

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define IS_NUM(c)               (c >= '0' && c <= '9')
#define CHAR2NUM(c)             (c - '0')

#define TAL_GPIO_TEST_NUM       sizeof(tal_gpio_test_map)/sizeof(tal_gpio_test_map[0])

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
typedef struct {
    UINT32_T pin_idx;
    UINT32_T pin;
} tal_gpio_test_map_t;

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
STATIC tal_gpio_test_map_t tal_gpio_test_map[] = TAL_GPIO_TEST_MAP;

typedef struct {
    UINT32_T pin_num;
    UINT32_T pin[TAL_GPIO_TEST_NUM];
    UINT8_T  pin_array[TAL_GPIO_TEST_NUM];
    UINT8_T  map[TAL_GPIO_TEST_NUM];
    UINT8_T  ret[TAL_GPIO_TEST_NUM + 1];
} tal_gpio_test_info_t;

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/
STATIC INLINE VOID_T tal_gpio_test_init_output(UINT32_T pin_id);
STATIC INLINE VOID_T tal_gpio_test_init_input(UINT32_T pin_id);
STATIC INLINE VOID_T tal_gpio_test_pin_clear(UINT32_T pin_id);
STATIC INLINE TUYA_GPIO_LEVEL_E tal_gpio_test_pin_pin_get(UINT32_T pin_id);
STATIC UINT8_T tal_gpio_test_pin_is_exist(UINT8_T *ret, UINT8_T num);
STATIC UINT32_T tal_gpio_test_get_pin_from_pin_idx(UINT8_T pin_idx);




STATIC VOID_T tal_gpio_test_start(tal_gpio_test_info_t *p_gpio_test)
{
    TUYA_GPIO_LEVEL_E tmp_level = TUYA_GPIO_LEVEL_HIGH;
    UINT32_T i = 0, j = 0;
    UINT32_T index = 1;

    UINT32_T pin_num = p_gpio_test->pin_num;
    UINT32_T *pin = p_gpio_test->pin;
    UINT8_T  *map = p_gpio_test->map;
    UINT8_T  *ret = p_gpio_test->ret;

    ret[0] = 0;

    //In particular, there is only one pin to be tested
    if (pin_num == 1) {
        tal_gpio_test_init_input(pin[0]);
        tmp_level = tal_gpio_test_pin_pin_get(pin[0]);
        if (TUYA_GPIO_LEVEL_LOW == tmp_level) {
            ret[0]++;
            ret[index++] = 0;
        }
    } else {
        for (j = 0; j < pin_num; j++) {
            // set all gpio input
            for (i = 0; i < pin_num; i++) {
                tal_gpio_test_init_input(pin[i]);
            }

            // set gpio output, and clear pin
            tal_gpio_test_init_output(pin[j]);
            tal_gpio_test_pin_clear(pin[j]);

            for (i = 0; i < pin_num; i++) {
                if (j != i) {
                    tmp_level = tal_gpio_test_pin_pin_get(pin[i]);
                    // in the same group
                    if (map[i] == map[j]) {
                        if ((TUYA_GPIO_LEVEL_LOW != tmp_level) && (0 == tal_gpio_test_pin_is_exist(ret, i))) {
                            //record wrong num
                            ret[0]++;
                            ret[index++] = i;
                        }
                    } else {
                        if ((TUYA_GPIO_LEVEL_LOW == tmp_level) && (0 == tal_gpio_test_pin_is_exist(ret, i))) {
                            //record wrong num
                            ret[0]++;
                            ret[index++] = i;
                        }
                    }
                }
            }
        }
    }
}

UINT32_T tal_gpio_test_handler(CHAR_T *error_sequence, UINT8_T *para, UINT8_T len)
{
    tal_gpio_test_info_t tal_gpio_test_info = {0};
    UINT8_T pin_cnt = 0;
    UINT8_T group_id = 1;

    if (len == 0) {
        return 1;
    }

    for (INT_T i = 0; i < len; i++) {
        if (para[i] == '"' && IS_NUM(para[i + 1])) {
            if (IS_NUM(para[i + 2])) {
                UINT8_T pin_num = CHAR2NUM(para[i + 1]) * 10 + CHAR2NUM(para[i + 2]);
                tal_gpio_test_info.pin[pin_cnt] = tal_gpio_test_get_pin_from_pin_idx(pin_num);
                tal_gpio_test_info.map[pin_cnt] = group_id;
                tal_gpio_test_info.pin_array[pin_cnt] = pin_num;
            } else {
                tal_gpio_test_info.pin[pin_cnt] = tal_gpio_test_get_pin_from_pin_idx(CHAR2NUM(para[i + 1]));
                tal_gpio_test_info.map[pin_cnt] = group_id;
                tal_gpio_test_info.pin_array[pin_cnt] = CHAR2NUM(para[i + 1]);
            }
            pin_cnt++;
        }

        if (para[i] == ',' && IS_NUM(para[i + 1])) {
            if (IS_NUM(para[i + 2])) {
                UINT8_T pin_num = CHAR2NUM(para[i + 1]) * 10 + CHAR2NUM(para[i + 2]);
                tal_gpio_test_info.pin[pin_cnt] = tal_gpio_test_get_pin_from_pin_idx(pin_num);
                tal_gpio_test_info.map[pin_cnt] = group_id;
                tal_gpio_test_info.pin_array[pin_cnt] = pin_num;
            } else {
                tal_gpio_test_info.pin[pin_cnt] = tal_gpio_test_get_pin_from_pin_idx(CHAR2NUM(para[i + 1]));
                tal_gpio_test_info.map[pin_cnt] = group_id;
                tal_gpio_test_info.pin_array[pin_cnt] = CHAR2NUM(para[i + 1]);
            }
            pin_cnt++;
        }

        if (para[i] == '"' && (para[i + 1] == ',' || para[i + 1] == ']')) {
            group_id++;
        }
    }

    tal_gpio_test_info.pin_num = pin_cnt;

    tal_gpio_test_start(&tal_gpio_test_info);

    uint32_t failed_count = tal_gpio_test_info.ret[0];

    if (failed_count == 0) {
        return 1;
    } else {
        UINT8_T j = 0;
        UINT8_T idx = 0;
        UINT8_T next_idx = 0;

        for (UINT8_T k = 0; k < failed_count; k++) {
            idx = tal_gpio_test_info.ret[k + 1];
            if (k != failed_count - 1) {
                next_idx = tal_gpio_test_info.ret[k + 2];
            }

            UINT8_T tmp_pin_idx_str[3] = {0};
            UINT8_T tmp_pin_idx = tal_gpio_test_info.pin_array[idx];
            tal_util_str_int2intstr(tmp_pin_idx, tmp_pin_idx_str, 3);

            if (error_sequence != NULL) {
                memcpy(&error_sequence[j], (VOID_T *)tmp_pin_idx_str, strlen((VOID_T *)tmp_pin_idx_str));
                j += strlen((VOID_T *)tmp_pin_idx_str);

                if (k == failed_count - 1) {
                    error_sequence[j++] = '\"';
                } else if (tal_gpio_test_info.map[idx] != tal_gpio_test_info.map[next_idx]) {
                    memcpy(&error_sequence[j], "\",\"", 3);
                    j += 3;
                } else {
                    error_sequence[j++] = ',';
                }
            }
        }

        if (error_sequence != NULL && j > 0) {
            error_sequence[j - 1] = '\0';
        }

        return 0;
    }
}

STATIC INLINE VOID_T tal_gpio_test_init_output(UINT32_T pin_id)
{
    TUYA_GPIO_BASE_CFG_T gpio_cfg = {
        .mode = TUYA_GPIO_PUSH_PULL,
        .direct = TUYA_GPIO_OUTPUT,
        .level = TUYA_GPIO_LEVEL_LOW,
    };
    tal_gpio_init(pin_id, &gpio_cfg);
}

STATIC INLINE VOID_T tal_gpio_test_init_input(UINT32_T pin_id)
{
    TUYA_GPIO_BASE_CFG_T gpio_cfg = {
        .mode = TUYA_GPIO_PULLUP,
        .direct = TUYA_GPIO_INPUT,
        .level = TUYA_GPIO_LEVEL_LOW,
    };
    tal_gpio_init(pin_id, &gpio_cfg);
}

STATIC INLINE VOID_T tal_gpio_test_pin_clear(UINT32_T pin_id)
{
    tal_gpio_write(pin_id, TUYA_GPIO_LEVEL_LOW);
}

STATIC INLINE TUYA_GPIO_LEVEL_E tal_gpio_test_pin_pin_get(UINT32_T pin_id)
{
    TUYA_GPIO_LEVEL_E gpio_level = TUYA_GPIO_LEVEL_HIGH;
    tal_gpio_read(pin_id, &gpio_level);
    return gpio_level;
}

STATIC UINT8_T tal_gpio_test_pin_is_exist(UINT8_T *ret, UINT8_T num)
{
    for (UINT8_T i = 0; i < ret[0]; i++) {
        if (ret[i + 1] == num) {
            return 1;
        }
    }
    return 0;
}

STATIC UINT32_T tal_gpio_test_get_pin_from_pin_idx(UINT8_T pin_idx)
{
    for (int i = 0; i < TAL_GPIO_TEST_NUM; i++) {
        if (pin_idx == tal_gpio_test_map[i].pin_idx) {
            return tal_gpio_test_map[i].pin;
        }
    }

    return 0;
}

#endif  /* TAL_GPIO_TEST_ENABLE */

