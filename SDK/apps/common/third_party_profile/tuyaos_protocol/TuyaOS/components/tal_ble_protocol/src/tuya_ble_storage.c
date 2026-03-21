/**
 * @file tuya_ble_storage.c
 * @brief This is tuya_ble_storage file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "string.h"

#include "vendor/jl_ble/tuyaos/include/board.h"
#include "tal_util.h"

#include "tuya_ble_api.h"
#include "tuya_ble_port.h"
#include "tuya_ble_type.h"
#include "tuya_ble_main.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_internal_config.h"
#include "tuya_ble_mutli_tsf_protocol.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_log.h"
#if defined(TUYA_BLE_FEATURE_PRODUCT_TEST_ENABLE) && (TUYA_BLE_FEATURE_PRODUCT_TEST_ENABLE == 1)
#include "tuya_ble_product_test.h"
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/


/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
typedef struct {
    UINT32_T  crc;
    UINT32_T  settings_version;
    UINT8_T   h_id[H_ID_LEN];
    UINT8_T   device_id[DEVICE_ID_LEN];
    UINT8_T   mac[MAC_LEN];
    UINT8_T   auth_key[AUTH_KEY_LEN];
} tuya_ble_auth_settings_old_t;

typedef struct {
    UINT32_T  crc;
    UINT32_T  settings_version;
    tuya_ble_product_id_type_t pid_type;
    UINT8_T   pid_len;
    UINT8_T   common_pid[TUYA_BLE_PRODUCT_ID_MAX_LEN];
    UINT8_T   login_key[LOGIN_KEY_LEN];
    UINT8_T   ecc_secret_key[ECC_SECRET_KEY_LEN];
    UINT8_T   device_virtual_id[DEVICE_VIRTUAL_ID_LEN];
    UINT8_T   user_rand[PAIR_RANDOM_LEN];
    UINT8_T   bound_flag;
} tuya_ble_sys_settings_old_t;

typedef struct {
    tuya_ble_auth_settings_t flash_settings_auth;
    tuya_ble_auth_settings_t flash_settings_auth_backup;
} tuya_ble_storage_auth_settings_t;

typedef struct {
    tuya_ble_sys_settings_t flash_settings_sys;
    tuya_ble_sys_settings_t flash_settings_sys_backup;
} tuya_ble_storage_sys_settings_t;

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




STATIC UINT32_T auth_settings_crc_get(tuya_ble_auth_settings_t CONST *p_settings)
{
    // The crc is calculated from the s_dfu_settings struct, except the crc itself and the init command
    return tal_util_crc32((UINT8_T *)(p_settings) + 4, SIZEOF(tuya_ble_auth_settings_t) - 4, NULL);
}

STATIC BOOL_T auth_settings_crc_ok(tuya_ble_auth_settings_t CONST *p_settings, UINT8_T *flash_update)
{
    UINT32_T crc, crc_old;
    if (p_settings->crc != 0xFFFFFFFF) {
        // CRC is set. Content must be valid
        crc = auth_settings_crc_get(p_settings);
        if (crc == p_settings->crc) {
            return TRUE;
        } else {
            crc_old = tal_util_crc32((UINT8_T *)(p_settings) + 4, SIZEOF(tuya_ble_auth_settings_old_t) - 4, NULL);
            if (crc_old == p_settings->crc) {
                *flash_update = 1;
                return TRUE;
            }
        }
    }
    return FALSE;
}

STATIC UINT32_T sys_settings_crc_get(tuya_ble_sys_settings_t CONST *p_settings)
{
    // The crc is calculated from the s_dfu_settings struct, except the crc itself and the init command
    return tal_util_crc32((UINT8_T *)(p_settings) + 4, SIZEOF(tuya_ble_sys_settings_t) - 4, NULL);
}

STATIC BOOL_T sys_settings_crc_ok(tuya_ble_sys_settings_t CONST *p_settings, UINT8_T *flash_update)
{
    UINT32_T crc, crc_old;
    if (p_settings->crc != 0xFFFFFFFF) {
        // CRC is set. Content must be valid
        crc = sys_settings_crc_get(p_settings);
        if (crc == p_settings->crc) {
            return TRUE;
        } else {
            crc_old = tal_util_crc32((UINT8_T *)(p_settings) + 4, SIZEOF(tuya_ble_sys_settings_old_t) - 4, NULL);
            if (crc_old == p_settings->crc) {
                *flash_update = 1;
                return TRUE;
            }
        }
    }
    return FALSE;
}

UINT32_T tuya_ble_storage_load_settings(VOID_T)
{
    UINT32_T err_code = 0;
    BOOL_T settings_valid ;
    BOOL_T settings_backup_valid;
    UINT8_T auth_settings_flag = 1;
    UINT8_T sys_settings_flag = 1;
    UINT8_T auth_settings_update = 0;
    UINT8_T sys_settings_update = 0;
#if !defined(TUYA_BLE_SINGLE_BANK_OTA_MODE) || (TUYA_BLE_SINGLE_BANK_OTA_MODE == 0)
    UINT8_T auth_settings_update_backup = 0;
#endif
    UINT8_T sys_settings_update_backup = 0;
    tuya_ble_storage_auth_settings_t *p_storage_settings_auth = NULL;
    tuya_ble_storage_sys_settings_t *p_storage_settings_sys = NULL;

    p_storage_settings_auth = (tuya_ble_storage_auth_settings_t *)tuya_ble_malloc(SIZEOF(tuya_ble_storage_auth_settings_t));

    if (p_storage_settings_auth == NULL) {
        TUYA_BLE_LOG_ERROR("p_storage_settings_auth malloc failed.");
        memset(&tuya_ble_current_para.auth_settings, 0, SIZEOF(tuya_ble_auth_settings_t));
        auth_settings_flag = 0;
    } else {
        memset(p_storage_settings_auth, 0, SIZEOF(tuya_ble_storage_auth_settings_t));
    }

    if (auth_settings_flag == 1) {
        tuya_ble_nv_read(TUYA_BLE_AUTH_FLASH_ADDR, (UINT8_T *)&p_storage_settings_auth->flash_settings_auth, SIZEOF(tuya_ble_auth_settings_t));
#if !defined(TUYA_BLE_SINGLE_BANK_OTA_MODE) || (TUYA_BLE_SINGLE_BANK_OTA_MODE == 0)
        tuya_ble_nv_read(TUYA_BLE_AUTH_FLASH_BACKUP_ADDR, (UINT8_T *)&p_storage_settings_auth->flash_settings_auth_backup, SIZEOF(tuya_ble_auth_settings_t));
#endif
        settings_valid = auth_settings_crc_ok(&p_storage_settings_auth->flash_settings_auth, &auth_settings_update);
#if !defined(TUYA_BLE_SINGLE_BANK_OTA_MODE) || (TUYA_BLE_SINGLE_BANK_OTA_MODE == 0)
        settings_backup_valid = auth_settings_crc_ok(&p_storage_settings_auth->flash_settings_auth_backup, &auth_settings_update_backup);
#endif
        if (settings_valid) {
            memcpy(&tuya_ble_current_para.auth_settings, &p_storage_settings_auth->flash_settings_auth, SIZEOF(tuya_ble_auth_settings_t));
        }
#if !defined(TUYA_BLE_SINGLE_BANK_OTA_MODE) || (TUYA_BLE_SINGLE_BANK_OTA_MODE == 0)
        else if (settings_backup_valid) {
            memcpy(&tuya_ble_current_para.auth_settings, &p_storage_settings_auth->flash_settings_auth_backup, SIZEOF(tuya_ble_auth_settings_t));
        }
#endif
        else {
            memset(&tuya_ble_current_para.auth_settings, 0, SIZEOF(tuya_ble_auth_settings_t));
            auth_settings_flag = 0;
        }
#if !defined(TUYA_BLE_SINGLE_BANK_OTA_MODE) || (TUYA_BLE_SINGLE_BANK_OTA_MODE == 0)
        if (auth_settings_update == 1 || auth_settings_update_backup == 1) { // }
#else
        if (auth_settings_update == 1) {
#endif
            tuya_ble_storage_save_auth_settings();
            auth_settings_update = 0;
#if !defined(TUYA_BLE_SINGLE_BANK_OTA_MODE) || (TUYA_BLE_SINGLE_BANK_OTA_MODE == 0)
            auth_settings_update_backup = 0;
#endif
        }

        tuya_ble_free((UINT8_T *)p_storage_settings_auth);

    }

    p_storage_settings_sys = (tuya_ble_storage_sys_settings_t *)tuya_ble_malloc(SIZEOF(tuya_ble_storage_sys_settings_t));

    if (p_storage_settings_sys == NULL) {
        TUYA_BLE_LOG_ERROR("p_storage_settings_sys malloc failed.");
        memset(&tuya_ble_current_para.sys_settings, 0, SIZEOF(tuya_ble_sys_settings_t));
        sys_settings_flag = 0;
    } else {
        memset(p_storage_settings_sys, 0, SIZEOF(tuya_ble_storage_sys_settings_t));
    }

    if (sys_settings_flag == 1) {
        tuya_ble_nv_read(TUYA_BLE_SYS_FLASH_ADDR, (UINT8_T *)&p_storage_settings_sys->flash_settings_sys, SIZEOF(tuya_ble_sys_settings_t));
        tuya_ble_nv_read(TUYA_BLE_SYS_FLASH_BACKUP_ADDR, (UINT8_T *)&p_storage_settings_sys->flash_settings_sys_backup, SIZEOF(tuya_ble_sys_settings_t));

        settings_valid = sys_settings_crc_ok(&p_storage_settings_sys->flash_settings_sys, &sys_settings_update);
        settings_backup_valid = sys_settings_crc_ok(&p_storage_settings_sys->flash_settings_sys_backup, &sys_settings_update_backup);

        if (settings_valid) {
            memcpy(&tuya_ble_current_para.sys_settings, &p_storage_settings_sys->flash_settings_sys, SIZEOF(tuya_ble_sys_settings_t));
        } else if (settings_backup_valid) {
            memcpy(&tuya_ble_current_para.sys_settings, &p_storage_settings_sys->flash_settings_sys_backup, SIZEOF(tuya_ble_sys_settings_t));
        } else {
            memset(&tuya_ble_current_para.sys_settings, 0, SIZEOF(tuya_ble_sys_settings_t));
            tuya_ble_current_para.sys_settings.factory_test_flag = 0xFF;
            sys_settings_flag = 0;
        }

        if (sys_settings_update == 1 || sys_settings_update_backup == 1) {
            tuya_ble_current_para.sys_settings.factory_test_flag = 0xFF;
            tuya_ble_storage_save_sys_settings();
            sys_settings_update = 0;
            sys_settings_update_backup = 0;
        }

        tuya_ble_free((UINT8_T *)p_storage_settings_sys);

    }

    tuya_ble_current_para.pid_type = tuya_ble_current_para.sys_settings.pid_type;
    tuya_ble_current_para.pid_len = tuya_ble_current_para.sys_settings.pid_len;
    memcpy(tuya_ble_current_para.pid, tuya_ble_current_para.sys_settings.common_pid, tuya_ble_current_para.pid_len);

    return err_code;
}

UINT32_T tuya_ble_storage_save_auth_settings(VOID_T)
{
    UINT32_T err_code = 0;

    tuya_ble_current_para.auth_settings.crc = tal_util_crc32((UINT8_T *)&tuya_ble_current_para.auth_settings + 4, SIZEOF(tuya_ble_current_para.auth_settings) - 4, NULL);

    if (tuya_ble_nv_erase(TUYA_BLE_AUTH_FLASH_ADDR, TUYA_NV_ERASE_MIN_SIZE) == TUYA_BLE_SUCCESS) {
        err_code = tuya_ble_nv_write(TUYA_BLE_AUTH_FLASH_ADDR, (UINT8_T *)&tuya_ble_current_para.auth_settings, SIZEOF(tuya_ble_auth_settings_t));

        if (err_code == TUYA_BLE_SUCCESS) {
            TUYA_BLE_LOG_DEBUG("write flash_settings_auth data succeed!");

#if !defined(TUYA_BLE_SINGLE_BANK_OTA_MODE) || (TUYA_BLE_SINGLE_BANK_OTA_MODE == 0)
            if (tuya_ble_nv_erase(TUYA_BLE_AUTH_FLASH_BACKUP_ADDR, TUYA_NV_ERASE_MIN_SIZE) == TUYA_BLE_SUCCESS) {
                if (tuya_ble_nv_write(TUYA_BLE_AUTH_FLASH_BACKUP_ADDR, (UINT8_T *)&tuya_ble_current_para.auth_settings, SIZEOF(tuya_ble_auth_settings_t)) != TUYA_BLE_SUCCESS) {
                    TUYA_BLE_LOG_ERROR("write flash_settings_auth data backup failed!");
                    err_code = 1;
                }
            } else {
                TUYA_BLE_LOG_ERROR("erase flash_settings_auth data backup failed!");
                err_code = 1;
            }
#endif
        } else {
            TUYA_BLE_LOG_ERROR("write flash_settings_auth data failed!");
            err_code = 1;
        }
    } else {
        TUYA_BLE_LOG_ERROR("erase flash_settings_auth data failed!");
        err_code = 1;
    }
    return err_code;

}

UINT32_T tuya_ble_storage_save_sys_settings(VOID_T)
{
    UINT32_T err_code = 0;

#if (TUYA_BLE_DEVICE_AUTH_DATA_STORE)

    tuya_ble_current_para.sys_settings.crc = tal_util_crc32((UINT8_T *)&tuya_ble_current_para.sys_settings + 4, SIZEOF(tuya_ble_sys_settings_t) - 4, NULL);

    if (tuya_ble_nv_erase(TUYA_BLE_SYS_FLASH_ADDR, TUYA_NV_ERASE_MIN_SIZE) == TUYA_BLE_SUCCESS) {
        err_code = tuya_ble_nv_write(TUYA_BLE_SYS_FLASH_ADDR, (UINT8_T *)&tuya_ble_current_para.sys_settings, SIZEOF(tuya_ble_sys_settings_t));

        if (err_code == TUYA_BLE_SUCCESS) {
            TUYA_BLE_LOG_INFO("write flash_settings_sys data succeed!");

            if (tuya_ble_nv_erase(TUYA_BLE_SYS_FLASH_BACKUP_ADDR, TUYA_NV_ERASE_MIN_SIZE) == TUYA_BLE_SUCCESS) {
                if (tuya_ble_nv_write(TUYA_BLE_SYS_FLASH_BACKUP_ADDR, (UINT8_T *)&tuya_ble_current_para.sys_settings, SIZEOF(tuya_ble_sys_settings_t)) != TUYA_BLE_SUCCESS) {
                    TUYA_BLE_LOG_ERROR("write flash_settings_sys data backup failed!");
                    err_code = 1;
                }
            } else {
                TUYA_BLE_LOG_ERROR("erase flash_settings_sys data backup failed!");
                err_code = 1;
            }

        } else {
            TUYA_BLE_LOG_ERROR("write flash_settings_sys data failed!");
            err_code = 1;
        }
    } else {
        TUYA_BLE_LOG_ERROR("erase flash_settings_sys data failed!");
        err_code = 1;
    }

#endif

    return err_code;
}

UINT32_T tuya_ble_storage_init(VOID_T)
{
    UINT32_T err = 0;

    tuya_ble_nv_init();

#if (TUYA_BLE_DEVICE_AUTH_DATA_STORE)

    tuya_ble_storage_load_settings();

#endif
    return err;
}

#if (TUYA_BLE_DEVICE_AUTH_DATA_STORE)

tuya_ble_status_t tuya_ble_storage_write_pid(tuya_ble_product_id_type_t pid_type, UINT8_T pid_len, UINT8_T *pid)
{
    tuya_ble_status_t ret = TUYA_BLE_SUCCESS;
    UINT8_T is_write = 0;

    if (pid_len > TUYA_BLE_PRODUCT_ID_MAX_LEN) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    if ((pid_type != tuya_ble_current_para.pid_type) || (pid_len != tuya_ble_current_para.pid_len)) {
        tuya_ble_current_para.pid_type = pid_type;
        tuya_ble_current_para.pid_len = pid_len;
        memcpy(tuya_ble_current_para.pid, pid, pid_len);
        tuya_ble_current_para.sys_settings.pid_type = pid_type;
        tuya_ble_current_para.sys_settings.pid_len = pid_len;
        memcpy(tuya_ble_current_para.sys_settings.common_pid, pid, pid_len);
        is_write = 1;
    } else if (memcmp(pid, tuya_ble_current_para.pid, pid_len) != 0) {
        tuya_ble_current_para.pid_type = pid_type;
        tuya_ble_current_para.pid_len = pid_len;
        memcpy(tuya_ble_current_para.pid, pid, pid_len);
        tuya_ble_current_para.sys_settings.pid_type = pid_type;
        tuya_ble_current_para.sys_settings.pid_len = pid_len;
        memcpy(tuya_ble_current_para.sys_settings.common_pid, pid, pid_len);
        is_write = 1;
    }
#if defined(TUYA_BLE_SINGLE_BANK_OTA_MODE) && (TUYA_BLE_SINGLE_BANK_OTA_MODE == 1)
    else if (memcmp(pid, tuya_ble_current_para.sys_settings.common_pid, pid_len) != 0) {
        tuya_ble_current_para.pid_type = pid_type;
        tuya_ble_current_para.pid_len = pid_len;
        memcpy(tuya_ble_current_para.pid, pid, pid_len);
        tuya_ble_current_para.sys_settings.pid_type = pid_type;
        tuya_ble_current_para.sys_settings.pid_len = pid_len;
        memcpy(tuya_ble_current_para.sys_settings.common_pid, pid, pid_len);
        is_write = 1;
    }
#endif
    else {

    }
    if (is_write == 1) {
        if (tuya_ble_storage_save_sys_settings()) {
            ret = TUYA_BLE_ERR_BUSY;
        }
    }

    return ret;
}

tuya_ble_status_t tuya_ble_storage_write_hid(UINT8_T *hid, UINT8_T len)
{
    tuya_ble_status_t ret = TUYA_BLE_SUCCESS;

    if (len != H_ID_LEN) {
        ret = TUYA_BLE_ERR_INVALID_PARAM;
    } else {
        if (memcmp(hid, tuya_ble_current_para.auth_settings.h_id, H_ID_LEN) != 0) {
            memcpy(tuya_ble_current_para.auth_settings.h_id, hid, H_ID_LEN);
            if (tuya_ble_storage_save_auth_settings()) {
                ret = TUYA_BLE_ERR_BUSY;
            }
        }
    }

    return ret;
}

tuya_ble_status_t tuya_ble_storage_read_id_info(tuya_ble_factory_id_data_t *id)
{
    tuya_ble_status_t ret = TUYA_BLE_SUCCESS;

    id->pid_type = tuya_ble_current_para.pid_type;
    id->pid_len = tuya_ble_current_para.pid_len;
    memcpy(id->pid, tuya_ble_current_para.pid, tuya_ble_current_para.pid_len);

    memcpy(id->h_id, tuya_ble_current_para.auth_settings.h_id, H_ID_LEN);
    memcpy(id->device_id, tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN);
    memcpy(id->mac, tuya_ble_current_para.auth_settings.mac, MAC_LEN);
    memcpy(id->mac_string, tuya_ble_current_para.auth_settings.mac_string, MAC_LEN * 2);
    memcpy(id->auth_key, tuya_ble_current_para.auth_settings.auth_key, AUTH_KEY_LEN);

#if TUYA_BLE_WRITE_BT_MAC
    if (tuya_ble_current_para.auth_settings.bt_mac_len > 0) {
        memcpy(id->bt_mac, tuya_ble_current_para.auth_settings.bt_mac, MAC_LEN);
        memcpy(id->bt_mac_string, tuya_ble_current_para.auth_settings.bt_mac_string, MAC_LEN * 2);
    }
#endif
    return ret;
}

tuya_ble_status_t tuya_ble_storage_write_auth_info(tuya_ble_factory_id_data_t *data)
{
    tuya_ble_status_t ret = TUYA_BLE_SUCCESS;
    uint8_t is_write = 0;

    if (memcmp(tuya_ble_current_para.auth_settings.auth_key, data->auth_key, AUTH_KEY_LEN) != 0) {
        memcpy(tuya_ble_current_para.auth_settings.auth_key, data->auth_key, AUTH_KEY_LEN);
        is_write = 1;
    }

    if (memcmp(tuya_ble_current_para.auth_settings.device_id, data->device_id, DEVICE_ID_LEN) != 0) {
        memcpy(tuya_ble_current_para.auth_settings.device_id, data->device_id, DEVICE_ID_LEN);
        is_write = 1;
    }

    if (memcmp(tuya_ble_current_para.auth_settings.mac, data->mac, MAC_LEN) != 0) {
        memcpy(tuya_ble_current_para.auth_settings.mac, data->mac, MAC_LEN);
        memcpy(tuya_ble_current_para.auth_settings.mac_string, data->mac_string, MAC_LEN * 2);
        is_write = 1;
    }

    if (data->pid_len > 0) {
        if ((tuya_ble_current_para.auth_settings.pid_len != data->pid_len) || (memcmp(tuya_ble_current_para.auth_settings.factory_pid, data->pid, data->pid_len) != 0)) {
            memcpy(tuya_ble_current_para.auth_settings.factory_pid, data->pid, data->pid_len);
            tuya_ble_current_para.auth_settings.pid_len = data->pid_len;
            tuya_ble_current_para.auth_settings.pid_type = (uint8_t)TUYA_BLE_PRODUCT_ID_TYPE_PID;
            is_write = 1;
        }
    }

#if TUYA_BLE_WRITE_BT_MAC
    tuya_ble_current_para.auth_settings.bt_mac_len = data->bt_mac_len;
    if (data->bt_mac_len == MAC_LEN) {
        if (memcmp(tuya_ble_current_para.auth_settings.bt_mac, data->bt_mac, MAC_LEN) != 0) {
            memcpy(tuya_ble_current_para.auth_settings.bt_mac, data->bt_mac, MAC_LEN);
            memcpy(tuya_ble_current_para.auth_settings.bt_mac_string, data->bt_mac_string, MAC_LEN * 2);
            is_write = 1;
        }
    }
#endif

    if (is_write == 1) {

#if (!TUYA_BLE_DEVICE_AUTH_DATA_STORE_EXT_MEDIUM)
        if (tuya_ble_storage_save_auth_settings()) {
            ret = TUYA_BLE_ERR_BUSY;
        }
#else
        tuya_ble_current_para.auth_settings.crc = tuya_ble_crc32_compute((uint8_t *)&tuya_ble_current_para.auth_settings + 4, sizeof(tuya_ble_current_para.auth_settings) - 4, NULL);

        if (tuya_ble_storage_private_data(PRIVATE_DATA_TUYA_AUTH_TOKEN, (uint8_t *)&tuya_ble_current_para.auth_settings, sizeof(tuya_ble_auth_settings_t))) {
            ret = TUYA_BLE_ERR_BUSY;
        }
#endif // (!TUYA_BLE_DEVICE_AUTH_DATA_STORE_EXT_MEDIUM)

        else {
            /* force clear device_virtual_id info */
            /*if (tuya_ble_current_para.sys_settings.bound_flag == 1)*/
            memset(tuya_ble_current_para.sys_settings.device_virtual_id, 0, DEVICE_VIRTUAL_ID_LEN);
            memset(tuya_ble_current_para.sys_settings.login_key, 0, LOGIN_KEY_LEN);
            tuya_ble_current_para.sys_settings.bound_flag = 0;
            tuya_ble_storage_save_sys_settings();
            tuya_ble_adv_change();
            tuya_ble_connect_status_set(UNBONDING_UNCONN);
            TUYA_BLE_LOG_INFO("The state has changed, current bound flag = %d", tuya_ble_current_para.sys_settings.bound_flag);
        }
    }

    return ret;
}

tuya_ble_status_t tuya_ble_storage_write_rf_param(uint8_t country_code, uint32_t tx_dBm)
{
    tuya_ble_status_t ret = TUYA_BLE_SUCCESS;
    uint8_t is_write = 0;

    if (tuya_ble_current_para.auth_settings.country_code != country_code) {
        tuya_ble_current_para.auth_settings.country_code = country_code;
        is_write = 1;
    }

    if (tuya_ble_current_para.auth_settings.tx_dBm != tx_dBm) {
        tuya_ble_current_para.auth_settings.tx_dBm = tx_dBm;
        is_write = 1;
    }

    if (is_write == 1) {

#if (!TUYA_BLE_DEVICE_AUTH_DATA_STORE_EXT_MEDIUM)
        if (tuya_ble_storage_save_auth_settings()) {
            ret = TUYA_BLE_ERR_BUSY;
        }
#else
        tuya_ble_current_para.auth_settings.crc = tuya_ble_crc32_compute((uint8_t *)&tuya_ble_current_para.auth_settings + 4, sizeof(tuya_ble_current_para.auth_settings) - 4, NULL);

        if (tuya_ble_storage_private_data(PRIVATE_DATA_TUYA_AUTH_TOKEN, (uint8_t *)&tuya_ble_current_para.auth_settings, sizeof(tuya_ble_auth_settings_t))) {
            ret = TUYA_BLE_ERR_BUSY;
        }
#endif // (!TUYA_BLE_DEVICE_AUTH_DATA_STORE_EXT_MEDIUM)

        else {
            /* force clear device_virtual_id info */
            /*if (tuya_ble_current_para.sys_settings.bound_flag == 1)*/
            memset(tuya_ble_current_para.sys_settings.device_virtual_id, 0, DEVICE_VIRTUAL_ID_LEN);
            memset(tuya_ble_current_para.sys_settings.login_key, 0, LOGIN_KEY_LEN);
            tuya_ble_current_para.sys_settings.bound_flag = 0;
            tuya_ble_storage_save_sys_settings();
            tuya_ble_adv_change();
            tuya_ble_connect_status_set(UNBONDING_UNCONN);
            TUYA_BLE_LOG_INFO("The state has changed, current bound flag = %d", tuya_ble_current_para.sys_settings.bound_flag);
        }
    }

    return ret;
}

tuya_ble_status_t tuya_ble_storage_write_auth_key_device_id_mac(UINT8_T *auth_key, UINT8_T auth_key_len, UINT8_T *device_id, UINT8_T device_id_len, UINT8_T *mac, UINT8_T mac_len, UINT8_T *mac_string, UINT8_T mac_string_len, UINT8_T *pid, UINT8_T pid_len)
{
    tuya_ble_status_t ret = TUYA_BLE_SUCCESS;
    UINT8_T is_write = 0;

    if (((auth_key_len != AUTH_KEY_LEN) && (auth_key_len != 0)) || ((device_id_len != DEVICE_ID_LEN) && (device_id_len != 0)) || ((mac_len != MAC_LEN) && (mac_len != 0))) {
        ret = TUYA_BLE_ERR_INVALID_PARAM;
    } else {
        if (auth_key_len == AUTH_KEY_LEN) {
            if (memcmp(tuya_ble_current_para.auth_settings.auth_key, auth_key, AUTH_KEY_LEN) != 0) {
                memcpy(tuya_ble_current_para.auth_settings.auth_key, auth_key, AUTH_KEY_LEN);
                is_write = 1;
            }
        }
        if (device_id_len == DEVICE_ID_LEN) {
            if (memcmp(tuya_ble_current_para.auth_settings.device_id, device_id, DEVICE_ID_LEN) != 0) {
                memcpy(tuya_ble_current_para.auth_settings.device_id, device_id, DEVICE_ID_LEN);
                is_write = 1;
            }
        }
        if (mac_len == MAC_LEN) {
            if (memcmp(tuya_ble_current_para.auth_settings.mac, mac, MAC_LEN) != 0) {
                memcpy(tuya_ble_current_para.auth_settings.mac, mac, MAC_LEN);
                memcpy(tuya_ble_current_para.auth_settings.mac_string, mac_string, MAC_LEN * 2);
                is_write = 1;
            }
        }
        if (pid_len > 0) {
            if ((tuya_ble_current_para.auth_settings.pid_len != pid_len) || (memcmp(tuya_ble_current_para.auth_settings.factory_pid, pid, pid_len) != 0)) {
                memcpy(tuya_ble_current_para.auth_settings.factory_pid, pid, pid_len);
                tuya_ble_current_para.auth_settings.pid_len = pid_len;
                tuya_ble_current_para.auth_settings.pid_type = (UINT8_T)TUYA_BLE_PRODUCT_ID_TYPE_PID;
                is_write = 1;
            }
        }

        if (is_write == 1) {

#if (!TUYA_BLE_DEVICE_AUTH_DATA_STORE_EXT_MEDIUM)
            if (tuya_ble_storage_save_auth_settings()) {
                ret = TUYA_BLE_ERR_BUSY;
            }
#else
            tuya_ble_current_para.auth_settings.crc = tal_util_crc32((UINT8_T *)&tuya_ble_current_para.auth_settings + 4, SIZEOF(tuya_ble_current_para.auth_settings) - 4, NULL);

            if (tuya_ble_storage_private_data(PRIVATE_DATA_TUYA_AUTH_TOKEN, (UINT8_T *)&tuya_ble_current_para.auth_settings, SIZEOF(tuya_ble_auth_settings_t))) {
                ret = TUYA_BLE_ERR_BUSY;
            }
#endif // (!TUYA_BLE_DEVICE_AUTH_DATA_STORE_EXT_MEDIUM)
            memset(tuya_ble_current_para.sys_settings.device_virtual_id, 0, DEVICE_VIRTUAL_ID_LEN);
            memset(tuya_ble_current_para.sys_settings.login_key, 0, LOGIN_KEY_LEN);
            tuya_ble_current_para.sys_settings.bound_flag = 0;
            tuya_ble_storage_save_sys_settings();
            tuya_ble_adv_change();
            tuya_ble_connect_status_set(UNBONDING_UNCONN);
            TUYA_BLE_LOG_INFO("The state has changed, current bound flag = %d", tuya_ble_current_para.sys_settings.bound_flag);
        }
    }

    return ret;
}

#endif

