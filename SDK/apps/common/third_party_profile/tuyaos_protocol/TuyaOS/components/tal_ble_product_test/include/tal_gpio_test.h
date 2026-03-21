/**
 * @file tal_gpio_test.h
 * @brief This is tal_gpio_test file
 * @version 1.0
 * @date 2022-06-24
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TAL_GPIO_TEST_H__
#define __TAL_GPIO_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_cloud_types.h"
#include "apps/tuyaos_demo_ble_peripheral/app_config.h"
#include "vendor/jl_ble/tuyaos/include/board.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#ifndef TAL_GPIO_TEST_ENABLE
#define TAL_GPIO_TEST_ENABLE       0
#endif

/*
 * You have to define it. It means the MAP of the chip GPIO index. used during GPIO testing. If it is not defined, the GPIO test will not work properly.
 */
#ifndef TAL_GPIO_TEST_MAP
#define TAL_GPIO_TEST_MAP           { \
                                        {0,0} \
                                    }
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
 * @brief Depending on the user configuration, automatic tests return 0 on success and 1 on failure.
 *
 * @param[in] error_sequence: error_sequence
 * @param[in] *para: *para
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
UINT32_T tal_gpio_test_handler(CHAR_T *error_sequence, UINT8_T *para, UINT8_T len);


#ifdef __cplusplus
}
#endif

#endif /* __TAL_GPIO_TEST_H__ */

