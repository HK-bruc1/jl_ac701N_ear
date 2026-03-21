/**
 * @file tuya_ble_attach_ota_port.c
 * @brief This is tuya_ble_attach_ota_port file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "vendor/jl_ble/tuyaos/include/board.h"

#include "tal_flash.h"
#include "tal_util.h"
#include "tuya_ble_api.h"
#include "tuya_ble_attach_ota_port.h"

#if defined(TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE) && (TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE == 1)

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define TUYA_BLE_ATTACH_OTA_PORT_INFO_ADDR          BOARD_FLASH_SDK_TEST_START_ADDR
#define TUYA_BLE_ATTACH_OTA_PORT_SIZE               (0x1000000)
#define TUYA_BLE_ATTACH_OTA_PORT_ERASE_MIN_SIZE     TUYA_NV_ERASE_MIN_SIZE
#define TUYA_BLE_ATTACH_OTA_PORT_FILE_MD5_LEN       (16)

#define TUYA_BLE_ATTACH_OTA_PORT_MCU_FVER_NUM       0x00010000
#define TUYA_BLE_ATTACH_OTA_PORT_ATTACH_FVER_NUM    0x00010000

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
typedef struct {
    UINT32_T *p_attach_ota_idx;
    UINT32_T  attach_ota_version;
    UINT32_T *p_firmware_crc32;
    UINT8_T  *p_firmware_md5;
} tuya_ble_attach_ota_pri_param_t;

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
STATIC UINT32_T sg_firmware_len = 0;
STATIC UINT32_T sg_data_crc32 = 0;
STATIC tuya_ble_attach_ota_pri_param_t *tuya_ble_attach_ota_pri_param __attribute__((aligned(4))) = NULL;

TUYA_BLE_ATTACH_OTA_INFO_T tuya_ble_attach_ota_info[TUYA_BLE_ATTACH_OTA_PORT_NUM] = {0};
UINT32_T  sg_attach_ota_idx = 0;
UINT32_T *p_attach_ota_idx = &sg_attach_ota_idx;

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




UINT32_T tuya_ble_attach_ota_port_info_save(VOID_T)
{
    tal_flash_erase(TUYA_BLE_ATTACH_OTA_PORT_INFO_ADDR, TUYA_BLE_ATTACH_OTA_PORT_ERASE_MIN_SIZE);
    tal_flash_write(TUYA_BLE_ATTACH_OTA_PORT_INFO_ADDR, (VOID_T *) & (*p_attach_ota_idx), SIZEOF(UINT32_T));
    tal_flash_write(TUYA_BLE_ATTACH_OTA_PORT_INFO_ADDR + 4, (VOID_T *)tuya_ble_attach_ota_info, SIZEOF(TUYA_BLE_ATTACH_OTA_INFO_T)*TUYA_BLE_ATTACH_OTA_PORT_NUM);
    return OPRT_OK;
}

UINT32_T tuya_ble_attach_ota_port_info_load(VOID_T)
{
    tal_flash_read(TUYA_BLE_ATTACH_OTA_PORT_INFO_ADDR, (VOID_T *) & (*p_attach_ota_idx), SIZEOF(UINT32_T));
    if ((*p_attach_ota_idx) < TUYA_BLE_ATTACH_OTA_PORT_NUM) {
        tal_flash_read(TUYA_BLE_ATTACH_OTA_PORT_INFO_ADDR + 4, (VOID_T *)tuya_ble_attach_ota_info, SIZEOF(TUYA_BLE_ATTACH_OTA_INFO_T)*TUYA_BLE_ATTACH_OTA_PORT_NUM);
    } else {
        (*p_attach_ota_idx) = 0;
        for (UINT32_T idx = 0; idx < TUYA_BLE_ATTACH_OTA_PORT_NUM; idx++) {
            if (idx == 0) {
                tuya_ble_attach_ota_info[idx].type = 1;
                tuya_ble_attach_ota_info[idx].version = TUYA_BLE_ATTACH_OTA_PORT_MCU_FVER_NUM;
            } else {
                tuya_ble_attach_ota_info[idx].type = idx + 9;
                tuya_ble_attach_ota_info[idx].version = TUYA_BLE_ATTACH_OTA_PORT_ATTACH_FVER_NUM;
            }
            memset(&tuya_ble_attach_ota_info[idx].firmware_info, 0, sizeof(TUYA_OTA_FIRMWARE_INFO_T));
        }
        tuya_ble_attach_ota_port_info_save();
    }

//    TAL_PR_INFO("sg_attach_ota_idx-%d", (*p_attach_ota_idx));
//    for (UINT32_T idx=0; idx<TUYA_BLE_ATTACH_OTA_PORT_NUM; idx++) {
//        TAL_PR_INFO("idx-%d, type-%02d, version-%08x, len-%08x, crc32-%08x", idx, tuya_ble_attach_ota_info[idx].type, tuya_ble_attach_ota_info[idx].version,
//            tuya_ble_attach_ota_info[idx].firmware_info.len, tuya_ble_attach_ota_info[idx].firmware_info.crc32);
//    }

    return OPRT_OK;
}

OPERATE_RET tuya_ble_attach_ota_port_start_notify(UINT_T image_size, TUYA_OTA_TYPE_E type, TUYA_OTA_PATH_E path)
{
    return OPRT_OK;
}

OPERATE_RET tuya_ble_attach_ota_port_data_process(TUYA_OTA_DATA_T *pack, UINT32_T *remain_len)
{
    OPERATE_RET ret = OPRT_OK;

    tuya_ble_attach_ota_pri_param = pack->pri_data;

    if ((*p_attach_ota_idx) != (*tuya_ble_attach_ota_pri_param->p_attach_ota_idx)) {
        TAL_PR_ERR("tuya_ble_attach_ota_port_data_process type error");
        return OPRT_INVALID_PARM;
    }
    p_attach_ota_idx = tuya_ble_attach_ota_pri_param->p_attach_ota_idx;

    BOOL_T flag_4k = FALSE;
    if ((pack->offset == 0) || ((pack->offset + pack->len) >= (((pack->offset / TUYA_BLE_ATTACH_OTA_PORT_ERASE_MIN_SIZE) + 1)*TUYA_BLE_ATTACH_OTA_PORT_ERASE_MIN_SIZE))) {
        // Step1: Erase Flash
//        if (pack->offset == 0) {
//            tal_flash_erase(BOARD_FLASH_OTA_START_ADDR, TUYA_BLE_ATTACH_OTA_PORT_ERASE_MIN_SIZE);
//        } else {
//            UINT32_T erase_addr = BOARD_FLASH_OTA_START_ADDR + (((pack->offset/TUYA_BLE_ATTACH_OTA_PORT_ERASE_MIN_SIZE) + 1)*TUYA_BLE_ATTACH_OTA_PORT_ERASE_MIN_SIZE);
//            tal_flash_erase(erase_addr, TUYA_BLE_ATTACH_OTA_PORT_ERASE_MIN_SIZE);
//        }

        flag_4k = TRUE;
    }

    sg_data_crc32 = tal_util_crc32(pack->data, pack->len, &sg_data_crc32);

    // Step2: Write Flash
//    tal_flash_write(BOARD_FLASH_OTA_START_ADDR + pack->offset, pack->data, pack->len);

    sg_firmware_len = pack->total_len;

    memcpy(tuya_ble_attach_ota_info[(*p_attach_ota_idx)].firmware_info.md5, tuya_ble_attach_ota_pri_param->p_firmware_md5, TUYA_BLE_ATTACH_OTA_PORT_FILE_MD5_LEN);
    tuya_ble_attach_ota_info[(*p_attach_ota_idx)].firmware_info.len = pack->offset + pack->len;
    tuya_ble_attach_ota_info[(*p_attach_ota_idx)].firmware_info.crc32 = sg_data_crc32;

    if (flag_4k) {
        tuya_ble_attach_ota_port_info_save();
    }

    return ret;
}

OPERATE_RET tuya_ble_attach_ota_port_end_notify(BOOL_T reset)
{
    OPERATE_RET ret = OPRT_OK;

    if (sg_firmware_len != tuya_ble_attach_ota_info[(*p_attach_ota_idx)].firmware_info.len) {
        return 1;
    }

    if (sg_data_crc32 != *(tuya_ble_attach_ota_pri_param->p_firmware_crc32)) {
        return 2;
    }

    sg_firmware_len = 0;
    sg_data_crc32 = 0;

    tuya_ble_attach_ota_info[(*p_attach_ota_idx)].version = tuya_ble_attach_ota_pri_param->attach_ota_version;

    (*p_attach_ota_idx)++;

    tuya_ble_attach_ota_port_info_save();

    return ret;
}

OPERATE_RET tuya_ble_attach_ota_port_get_old_firmware_info(TUYA_OTA_FIRMWARE_INFO_T **info)
{
    OPERATE_RET ret = OPRT_OK;

    tuya_ble_attach_ota_port_info_load();

    tuya_ble_device_update_mcu_version(tuya_ble_attach_ota_info[0].version, 0x00010000);
    tuya_ble_device_update_attach_version(&tuya_ble_attach_ota_info[1], TUYA_BLE_ATTACH_OTA_PORT_NUM - 1);

    *info = &tuya_ble_attach_ota_info[(*p_attach_ota_idx)].firmware_info;

    if (tuya_ble_attach_ota_info[(*p_attach_ota_idx)].firmware_info.len <= TUYA_BLE_ATTACH_OTA_PORT_SIZE) {
        sg_data_crc32 = tuya_ble_attach_ota_info[(*p_attach_ota_idx)].firmware_info.crc32;
    }

    return ret;
}

#endif // TUYA_BLE_FEATURE_ATTACH_OTA_ENABLE

