/**
 * @file tuya_ble_ota.c
 * @brief This is tuya_ble_ota file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "string.h"

#include "vendor/jl_ble/tuyaos/include/board.h"

#include "tal_memory.h"
#include "tal_log.h"
#include "tal_system.h"
#include "tal_ota.h"
#include "tal_bluetooth.h"
#include "tal_sw_timer.h"
#include "tal_util.h"
#include "tal_ble_sha256.h"
#include "tal_memory.h"
#include "tal_sleep.h"
#include "tal_flash.h"
#include "tal_watchdog.h"

#include "tuya_ble_type.h"
#include "tuya_ble_log.h"
#include "tuya_ble_api.h" //tuya_ble_ota_response
#include "tuya_ble_ota.h"
#include "tuya_ble_main.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_secure.h"
#if defined(TUYA_BLE_OTA_SIGNATURE_ENABLE) && (TUYA_BLE_OTA_SIGNATURE_ENABLE == 1)
#include "tbs_crypto_micro_ecc.h"
#endif

#if defined(TUYA_BLE_FEATURE_OTA_ENABLE) && (TUYA_BLE_FEATURE_OTA_ENABLE == 1)

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define TUYA_BLE_OTA_STATE_UNKNOWN   (-1)

#define TUYA_BLE_OTA_VERSION         (3)

#define TUYA_BLE_OTA_FILE_MD5_LEN    (16)

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
#if defined(TUYA_BLE_OTA_SIGNATURE_ENABLE) && (TUYA_BLE_OTA_SIGNATURE_ENABLE == 1)
#pragma pack(1)
typedef struct {
    UINT8_T  signature_algorithm;
    UINT8_T  key_version;
} sign_info_data_t;
#endif

typedef struct {
    UINT8_T  flag;
    UINT8_T  ota_version;
    UINT8_T  type;
    UINT32_T version;
    UINT16_T package_maxlen;
    UINT8_T  flag2;
    UINT8_T  sign_info_len;
#if defined(TUYA_BLE_OTA_SIGNATURE_ENABLE) && (TUYA_BLE_OTA_SIGNATURE_ENABLE == 1)
    sign_info_data_t sign_info_data;
#endif
} tuya_ble_ota_req_rsp_t;

typedef struct {
    UINT8_T  type;
    UINT8_T  pid[8];
    UINT32_T version;
    UINT8_T  md5[TUYA_BLE_OTA_FILE_MD5_LEN];
    UINT32_T file_len;
    UINT32_T crc32;
} tuya_ble_ota_file_info_t;

typedef struct {
    UINT8_T  type;
    UINT8_T  state;
    UINT32_T old_file_len;
    UINT32_T old_crc32;
    UINT8_T  old_md5[TUYA_BLE_OTA_FILE_MD5_LEN];
} tuya_ble_ota_file_info_rsp_t;

typedef struct {
    UINT8_T  type;
    UINT32_T offset;
} tuya_ble_ota_file_offset_t;

typedef struct {
    UINT8_T  type;
    UINT32_T offset;
} tuya_ble_ota_file_offset_rsp_t;

typedef struct {
    UINT8_T  type;
    UINT16_T pkg_id;
    UINT16_T len;
    UINT16_T crc16;
    UINT8_T  data[];
} tuya_ble_app_ota_data_t;

typedef struct {
    UINT8_T type;
    UINT8_T state;
} tuya_ble_ota_data_rsp_t;

typedef struct {
    UINT8_T  type;
    UINT8_T state;
} tuya_ble_ota_end_rsp_t;

typedef struct {
    UINT8_T  version;
    UINT8_T  type;
} tuya_ble_ota_prerare_notification_t;

typedef struct {
    UINT8_T  version;
    UINT8_T  type;
    UINT8_T  state;
} tuya_ble_ota_prerare_notification_rsp_t;

#if defined(TUYA_BLE_OTA_SIGNATURE_ENABLE) && (TUYA_BLE_OTA_SIGNATURE_ENABLE == 1)
typedef struct {
    UINT8_T  type;
    UINT8_T  pid[8];
    UINT8_T  signature_algorithm;
    UINT8_T  key_version;
    UINT8_T  signature_len;
    uint8_t  signature_data[];
} tuya_ble_ota_signature_data_t;

typedef struct {
    UINT8_T  type;
    UINT8_T state;
} tuya_ble_ota_signature_data_rsp_t;

typedef struct {
    UINT8_T  signature_algorithm;
    UINT8_T  signature_len;
    uint8_t  signature_data[64];
} tuya_ble_ota_signature_data_cache_t;
#endif

typedef struct {
    UINT32_T *p_firmware_crc32;
    UINT8_T  *p_firmware_md5;
    UINT8_T  *p_firmware_verify_type; //!< firmware verify type, 0-NONE 1-CRC32 2-MD5
} tuya_ble_ota_pri_param_t;
#pragma pack()

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
STATIC INT32_T     sg_ota_state             = TUYA_BLE_OTA_STATE_UNKNOWN;
#if !defined(TUYA_BLE_SINGLE_BANK_OTA_MODE) || (TUYA_BLE_SINGLE_BANK_OTA_MODE == 0)

STATIC BOOL_T      sg_ota_success           = FALSE;

STATIC INT32_T     sg_data_package_id       = 0;
STATIC UINT32_T    sg_data_offset           = 0;

STATIC UINT8_T     sg_firmware_verify_type  = 1; //!< firmware verify type, 0-NONE 1-CRC32 2-MD5
STATIC UINT32_T    sg_firmware_len          = 0;
STATIC UINT32_T    sg_firmware_crc32        = 0;
STATIC UINT8_T     sg_firmware_md5[TUYA_BLE_OTA_FILE_MD5_LEN]   = {0};
STATIC TUYA_OTA_FIRMWARE_INFO_T *sg_incomplete_firmware_info    = NULL;

STATIC tuya_ble_ota_pri_param_t tuya_ble_ota_pri_param __attribute__((aligned(4))) = {0};

STATIC TIMER_ID    sg_ota_disconnect_timer_id                   = NULL;
STATIC TIMER_ID    sg_ota_reset_timer_id                        = NULL;

#if defined(TUYA_BLE_OTA_SIGNATURE_ENABLE) && (TUYA_BLE_OTA_SIGNATURE_ENABLE == 1)
static void *sg_ctx = NULL;
static tuya_ble_ota_signature_data_cache_t sg_signature_data    = {0};
#endif

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/
extern UINT16_T tuya_app_get_conn_handle(VOID_T);
#if defined(TUYA_BLE_OTA_SIGNATURE_ENABLE) && (TUYA_BLE_OTA_SIGNATURE_ENABLE == 1)
static OPERATE_RET tuya_ble_ota_get_signature_in_flash(void);
OPERATE_RET tal_flash_read_for_ota(UINT32_T addr, UCHAR_T *dst, UINT32_T size);
uint32_t tuya_ble_ota_get_first_page(UINT8_T **pp_buf);
#endif
OPERATE_RET tuya_ble_ota_make_ota_firmware_valid(void);
OPERATE_RET tuya_ble_ota_make_ota_firmware_invalid(void);




STATIC UINT32_T tuya_ble_ota_enter(VOID_T)
{
    //Enter the OTA process, disconnect Related
    sg_ota_state = TUYA_BLE_OTA_REQ;
    sg_ota_success = FALSE;

    sg_data_package_id = -1;
    sg_data_offset = 0;

    sg_firmware_len = 0;
    sg_firmware_crc32 = 0;
    memset(sg_firmware_md5, 0, TUYA_BLE_OTA_FILE_MD5_LEN);


    TAL_BLE_PEER_INFO_T peer_info = {0};
    peer_info.conn_handle = tuya_app_get_conn_handle();
    TAL_BLE_CONN_PARAMS_T conn_param = {0};
    conn_param.min_conn_interval = 15 * 4 / 5;
    conn_param.max_conn_interval = 15 * 4 / 5;
    conn_param.latency = 0;
    conn_param.conn_sup_timeout = 6000 / 10;
    conn_param.connection_timeout = 0;
    tal_ble_conn_param_update(peer_info, &conn_param);

    tal_ota_start_notify(0, TUYA_OTA_FULL, TUYA_OTA_PATH_BLE);

    tal_cpu_force_wakeup();

    return OPRT_OK;
}

STATIC UINT32_T tuya_ble_ota_exit(VOID_T)
{
    if (!sg_ota_success) {
        tal_ota_end_notify(FALSE);
        tuya_ble_ota_make_ota_firmware_invalid();
    }

    tal_cpu_force_wakeup();

    tal_sw_timer_start(sg_ota_disconnect_timer_id, 1000, TAL_TIMER_ONCE);

    return OPRT_OK;
}

STATIC UINT32_T tuya_ble_ota_rsp(tuya_ble_ota_response_t *rsp, VOID_T *rsp_data, UINT16_T data_size)
{
    if (rsp->type != TUYA_BLE_OTA_DATA) {
        TAL_PR_HEXDUMP_INFO("ota_rsp_data", rsp_data, data_size);
    }

    rsp->p_data = rsp_data;
    rsp->data_len = data_size;
    return tuya_ble_ota_response(rsp);
}

STATIC UINT32_T tuya_ble_ota_req_handler(UINT8_T *cmd, UINT16_T cmd_size, tuya_ble_ota_response_t *rsp)
{
    if (sg_ota_state != TUYA_BLE_OTA_STATE_UNKNOWN) {
        TAL_PR_ERR("%s ota_state error: %d", __FUNCTION__, sg_ota_state);

        tuya_ble_ota_req_rsp_t req_rsp = {0};
        req_rsp.flag = 1; //refuse ota
        tuya_ble_ota_rsp(rsp, &req_rsp, SIZEOF(tuya_ble_ota_req_rsp_t));

        tuya_ble_ota_exit();
        return OPRT_COM_ERROR;
    }

    UINT8_T *p_type = cmd;
    if ((cmd_size != 1) || (*p_type != 0)) {
        TAL_PR_ERR("%s param error", __FUNCTION__);

        tuya_ble_ota_req_rsp_t req_rsp = {0};
        req_rsp.flag = 1; //refuse ota
        tuya_ble_ota_rsp(rsp, &req_rsp, SIZEOF(tuya_ble_ota_req_rsp_t));

        tuya_ble_ota_exit();
        return OPRT_COM_ERROR;
    }

    tuya_ble_ota_enter();

    tuya_ble_ota_req_rsp_t req_rsp = {0};
    req_rsp.flag = 0; //accept ota
    req_rsp.ota_version = TUYA_BLE_OTA_VERSION;
    req_rsp.type = 0; //0-ble, 1-mcu
    req_rsp.version = tal_common_info.firmware_version;
    tal_util_reverse_byte((VOID_T *)&req_rsp.version, SIZEOF(UINT32_T));
    req_rsp.package_maxlen = TUYA_BLE_OTA_PKG_LEN;
    tal_util_reverse_byte((VOID_T *)&req_rsp.package_maxlen, SIZEOF(UINT16_T));
    req_rsp.flag2 = 0x01; // bit0 = 1, support signature_algorithm
    req_rsp.sign_info_len = 2;
#if defined(TUYA_BLE_OTA_SIGNATURE_ENABLE) && (TUYA_BLE_OTA_SIGNATURE_ENABLE == 1)
    req_rsp.sign_info_data.signature_algorithm = 1;
    req_rsp.sign_info_data.key_version = sg_signature_key_info.key_version;
#endif
    tuya_ble_ota_rsp(rsp, &req_rsp, SIZEOF(tuya_ble_ota_req_rsp_t));

    sg_ota_state = TUYA_BLE_OTA_FILE_INFO;

    return OPRT_OK;
}

STATIC UINT32_T tuya_ble_ota_file_info_handler(UINT8_T *cmd, UINT16_T cmd_size, tuya_ble_ota_response_t *rsp)
{
    if ((sg_ota_state <= TUYA_BLE_OTA_REQ) || (sg_ota_state >= TUYA_BLE_OTA_UNKONWN)) {
        TAL_PR_ERR("%s ota_state error: %d", __FUNCTION__, sg_ota_state);
        //rsp none
        tuya_ble_ota_exit();
        return OPRT_COM_ERROR;
    }

    tuya_ble_ota_file_info_t *file_info = (VOID_T *)cmd;
    if (file_info->type != 0) {
        TAL_PR_ERR("%s file_info->type error", __FUNCTION__);
        //rsp none
        tuya_ble_ota_exit();
        return OPRT_COM_ERROR;
    }

    tal_util_reverse_byte((VOID_T *)&file_info->version, SIZEOF(UINT32_T));
    tal_util_reverse_byte((VOID_T *)&file_info->file_len, SIZEOF(UINT32_T));
    tal_util_reverse_byte((VOID_T *)&file_info->crc32, SIZEOF(UINT32_T));
    sg_firmware_len = file_info->file_len;
    sg_firmware_crc32 = file_info->crc32;
    memcpy(sg_firmware_md5, file_info->md5, TUYA_BLE_OTA_FILE_MD5_LEN);
    tuya_ble_ota_pri_param.p_firmware_crc32 = &sg_firmware_crc32;
    tuya_ble_ota_pri_param.p_firmware_md5 = sg_firmware_md5;
    tuya_ble_ota_pri_param.p_firmware_verify_type = &sg_firmware_verify_type;

    tuya_ble_ota_file_info_rsp_t file_info_rsp = {0};
    file_info_rsp.type = 0; //firmware info
    if (file_info->version <= tal_common_info.firmware_version) {
        file_info_rsp.state = 2; //version error
    } else if (file_info->file_len > BOARD_FLASH_OTA_SIZE) {
        file_info_rsp.state = 3; //size error
    } else {
        file_info_rsp.state = 0;

        sg_ota_state = TUYA_BLE_OTA_FILE_OFFSET_REQ;

        file_info_rsp.old_file_len = sg_incomplete_firmware_info->len;
        tal_util_reverse_byte((VOID_T *)&file_info_rsp.old_file_len, SIZEOF(UINT32_T));
        file_info_rsp.old_crc32 = sg_incomplete_firmware_info->crc32;
        tal_util_reverse_byte((VOID_T *)&file_info_rsp.old_crc32, SIZEOF(UINT32_T));
        memset(file_info_rsp.old_md5, 0, TUYA_BLE_OTA_FILE_MD5_LEN);
    }

    tuya_ble_ota_rsp(rsp, &file_info_rsp, SIZEOF(tuya_ble_ota_file_info_rsp_t));

    if (file_info_rsp.state != 0) {
        TAL_PR_ERR("%s errorid: %d", __FUNCTION__, file_info_rsp.state);
        tuya_ble_ota_exit();
    }

    return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_ota_erase_flash(UINT32_T offset_addr)
{
    return OPRT_NOT_SUPPORTED;
}

STATIC UINT32_T tuya_ble_ota_file_offset_handler(UINT8_T *cmd, UINT16_T cmd_size, tuya_ble_ota_response_t *rsp)
{
    if ((sg_ota_state <= TUYA_BLE_OTA_REQ) || (sg_ota_state >= TUYA_BLE_OTA_UNKONWN)) {
        TAL_PR_ERR("%s ota_state error: %d", __FUNCTION__, sg_ota_state);
        //rsp none
        tuya_ble_ota_exit();
        return OPRT_COM_ERROR;
    }

    tuya_ble_ota_file_offset_t *file_offset = (VOID_T *)cmd;
    if (file_offset->type != 0) {
        TAL_PR_ERR("%s file_offset->type error, __FUNCTION__");
        //rsp none
        tuya_ble_ota_exit();
        return OPRT_COM_ERROR;
    }

    tal_util_reverse_byte((VOID_T *)&file_offset->offset, SIZEOF(UINT32_T));

    tuya_ble_ota_file_offset_rsp_t file_offset_rsp = {0};
    file_offset_rsp.type = 0;

    if (file_offset->offset > 0) {
        if ((memcmp(sg_incomplete_firmware_info->md5, sg_firmware_md5, TUYA_BLE_OTA_FILE_MD5_LEN) == 0)
            && (file_offset->offset >= sg_incomplete_firmware_info->len)) { //The ota data from phone may not have been written to Flash
            file_offset_rsp.offset = sg_incomplete_firmware_info->len;
            sg_data_offset = sg_incomplete_firmware_info->len;
        } else {
            file_offset_rsp.offset = 0;
            sg_data_offset = 0;
        }
    }

    tal_util_reverse_byte((VOID_T *)&file_offset_rsp.offset, SIZEOF(UINT32_T));
    tuya_ble_ota_rsp(rsp, &file_offset_rsp, SIZEOF(tuya_ble_ota_file_offset_rsp_t));

    sg_ota_state = TUYA_BLE_OTA_DATA;

    tkl_ota_erase_flash(sg_data_offset);

    return OPRT_OK;
}

STATIC UINT32_T tuya_ble_ota_data_handler(UINT8_T *cmd, UINT16_T cmd_size, tuya_ble_ota_response_t *rsp)
{
    if ((sg_ota_state <= TUYA_BLE_OTA_REQ) || (sg_ota_state >= TUYA_BLE_OTA_UNKONWN)) {
        TAL_PR_ERR("%s ota_state error: %d", __FUNCTION__, sg_ota_state);

        tuya_ble_ota_data_rsp_t ota_data_rsp = {0};
        ota_data_rsp.state = 4; //unknow error
        tuya_ble_ota_rsp(rsp, &ota_data_rsp, SIZEOF(tuya_ble_ota_data_rsp_t));

        tuya_ble_ota_exit();
        return OPRT_COM_ERROR;
    }

    tuya_ble_app_ota_data_t *ota_data = (VOID_T *)cmd;
    if (ota_data->type != 0) {
        TAL_PR_ERR("%s ota_data->type error", __FUNCTION__);

        tuya_ble_ota_data_rsp_t ota_data_rsp = {0};
        ota_data_rsp.state = 4; //unknow error
        tuya_ble_ota_rsp(rsp, &ota_data_rsp, SIZEOF(tuya_ble_ota_data_rsp_t));

        tuya_ble_ota_exit();
        return OPRT_COM_ERROR;
    }

    tal_util_reverse_byte((VOID_T *)&ota_data->pkg_id, SIZEOF(UINT16_T));
    tal_util_reverse_byte((VOID_T *)&ota_data->len, SIZEOF(UINT16_T));
    tal_util_reverse_byte((VOID_T *)&ota_data->crc16, SIZEOF(UINT16_T));

    tuya_ble_ota_data_rsp_t ota_data_rsp = {0};
    ota_data_rsp.type = 0;
    if (sg_data_package_id + 1 != ota_data->pkg_id) {
        ota_data_rsp.state = 1; //package id error
    } else if (cmd_size - 7 != ota_data->len) {
        ota_data_rsp.state = 2; //size error
    } else if (tal_util_crc16(ota_data->data, ota_data->len, NULL) != ota_data->crc16) {
        ota_data_rsp.state = 3; //crc error
    } else {
        ota_data_rsp.state = 0;

        TUYA_OTA_DATA_T tal_ota_data = {0};
        tal_ota_data.total_len = sg_firmware_len;
        tal_ota_data.offset = sg_data_offset;
        tal_ota_data.data = ota_data->data;
        tal_ota_data.len = ota_data->len;
        tal_ota_data.pri_data = &tuya_ble_ota_pri_param;
        if (tal_ota_data_process(&tal_ota_data, NULL) != OPRT_OK) {
            ota_data_rsp.state = 4; //unknow error
        }

        sg_data_offset += ota_data->len;

        if (sg_data_offset < sg_firmware_len) {
            sg_ota_state = TUYA_BLE_OTA_DATA;
        } else if (sg_data_offset == sg_firmware_len) {
            sg_ota_state = TUYA_BLE_OTA_END;
        }

        sg_data_package_id++;
        TAL_PR_INFO("sg_data_package_id: %d", sg_data_package_id);
    }

    tuya_ble_ota_rsp(rsp, &ota_data_rsp, SIZEOF(tuya_ble_ota_data_rsp_t));

    if (ota_data_rsp.state != 0) {
        TAL_PR_ERR("%s errorid: %d", __FUNCTION__, ota_data_rsp.state);
        tuya_ble_ota_exit();
    }

    return OPRT_OK;
}

STATIC UINT32_T tuya_ble_ota_end_handler(UINT8_T *cmd, UINT16_T cmd_size, tuya_ble_ota_response_t *rsp)
{
    if ((sg_ota_state <= TUYA_BLE_OTA_REQ) || (sg_ota_state >= TUYA_BLE_OTA_UNKONWN)) {
        TAL_PR_ERR("%s ota_state error: %d", __FUNCTION__, sg_ota_state);

        tuya_ble_ota_end_rsp_t end_rsp = {0};
        end_rsp.state = 3; //unknow error
        tuya_ble_ota_rsp(rsp, &end_rsp, SIZEOF(tuya_ble_ota_end_rsp_t));

        tuya_ble_ota_exit();
        return OPRT_COM_ERROR;
    }

    if ((cmd_size != 1) || (*cmd != 0)) {
        TAL_PR_ERR("%s type error", __FUNCTION__);

        tuya_ble_ota_end_rsp_t end_rsp = {0};
        end_rsp.state = 3; //unknow error
        tuya_ble_ota_rsp(rsp, &end_rsp, SIZEOF(tuya_ble_ota_end_rsp_t));

        tuya_ble_ota_exit();
        return OPRT_COM_ERROR;
    }

    tuya_ble_ota_end_rsp_t end_rsp = {0};
    end_rsp.type = 0;

    UINT32_T ret = tal_ota_end_notify(TRUE);
    if (ret == 1) {
        end_rsp.state = 1; //total size error
    } else if (ret == 2) {
        end_rsp.state = 2; //crc error
    } else {
#if defined(TUYA_BLE_OTA_SIGNATURE_ENABLE) && (TUYA_BLE_OTA_SIGNATURE_ENABLE == 1)
        if (sg_signature_data.signature_len != 0) {
            uint32_t verify_ret = tuya_ble_ota_get_signature_in_flash();
            if (verify_ret == 0) {
                end_rsp.state = 0;

                tuya_ble_ota_make_ota_firmware_valid();
                sg_ota_success = TRUE;
                tuya_ble_ota_exit();
                TAL_PR_INFO("ota success");
            } else {
                end_rsp.state = 3; //signature error
            }
        } else {
            end_rsp.state = 0;

            tuya_ble_ota_make_ota_firmware_valid();
            sg_ota_success = TRUE;
            tuya_ble_ota_exit();
            TAL_PR_INFO("ota success");
        }
#else
        end_rsp.state = 0;

        tuya_ble_ota_make_ota_firmware_valid();
        sg_ota_success = TRUE;
        tuya_ble_ota_exit();
        TAL_PR_INFO("ota success");
#endif
    }

    tuya_ble_ota_rsp(rsp, &end_rsp, SIZEOF(tuya_ble_ota_end_rsp_t));

    if (end_rsp.state != 0) {
        TAL_PR_ERR("%s errorid: %d", __FUNCTION__, end_rsp.state);
        tuya_ble_ota_exit();
    }

    return OPRT_OK;
}

#if defined(TUYA_BLE_OTA_SIGNATURE_ENABLE) && (TUYA_BLE_OTA_SIGNATURE_ENABLE == 1)

STATIC UINT32_T tuya_ble_ota_signature_data_handler(UINT8_T *cmd, UINT16_T cmd_size, tuya_ble_ota_response_t *rsp)
{
    if ((sg_ota_state <= TUYA_BLE_OTA_REQ) || (sg_ota_state >= TUYA_BLE_OTA_UNKONWN)) {
        TAL_PR_ERR("%s ota_state error: %d", __FUNCTION__, sg_ota_state);

        tuya_ble_ota_signature_data_rsp_t signature_data_rsp = {0};
        signature_data_rsp.type = 0;
        signature_data_rsp.state = 3; //unknow error
        tuya_ble_ota_rsp(rsp, &signature_data_rsp, SIZEOF(tuya_ble_ota_signature_data_rsp_t));

        tuya_ble_ota_exit();
        return OPRT_COM_ERROR;
    }

    // cmd
    tuya_ble_ota_signature_data_t *cmd_param = (VOID_T *)cmd;
    sg_signature_data.signature_algorithm = cmd_param->signature_algorithm;
    sg_signature_data.signature_len = cmd_param->signature_len;
    if (sg_signature_data.signature_len <= 64) {
        memcpy(sg_signature_data.signature_data, cmd_param->signature_data, sg_signature_data.signature_len);
    }

    // rsp
    tuya_ble_ota_signature_data_rsp_t signature_data_rsp = {0};
    signature_data_rsp.type = 0;
    signature_data_rsp.state = 0;

    if (cmd_param->signature_algorithm != 1) {
        signature_data_rsp.state = 1; // signature algorithm not support
    } else if (cmd_param->key_version > sg_signature_key_info.key_version) {
        signature_data_rsp.state = 2; // key version not support
    }

    tuya_ble_ota_rsp(rsp, &signature_data_rsp, SIZEOF(tuya_ble_ota_signature_data_rsp_t));

    if (signature_data_rsp.state != 0) {
        TAL_PR_ERR("%s errorid: %d", __FUNCTION__, signature_data_rsp.state);
        tuya_ble_ota_exit();
    }

    return OPRT_OK;
}

#endif

STATIC VOID_T ota_disconnect_timeout_handler(TIMER_ID timer_id, VOID_T *arg)
{
    TAL_BLE_PEER_INFO_T info = {0};
    info.conn_handle = tuya_app_get_conn_handle();
    tal_ble_disconnect(info);

    tal_sw_timer_start(sg_ota_reset_timer_id, 1000, TAL_TIMER_ONCE);
}

STATIC VOID_T ota_reset_timeout_handler(TIMER_ID timer_id, VOID_T *arg)
{
    tal_system_reset();
}

UINT32_T tuya_ble_ota_init(VOID_T)
{
    tal_sw_timer_create(ota_disconnect_timeout_handler, NULL, &sg_ota_disconnect_timer_id);
    tal_sw_timer_create(ota_reset_timeout_handler, NULL, &sg_ota_reset_timer_id);

    tal_ota_get_old_firmware_info(&sg_incomplete_firmware_info);

#if defined(TUYA_BLE_OTA_SIGNATURE_ENABLE) && (TUYA_BLE_OTA_SIGNATURE_ENABLE == 1)
    tbs_crypto_ecc_init(ECC_ELLIPTIC_CURVE_SECP256R1);
#endif

//    TAL_PR_INFO("sg_incomplete_firmware_info->len: %d", sg_incomplete_firmware_info->len);
//    TAL_PR_INFO("sg_incomplete_firmware_info->crc32: %x", sg_incomplete_firmware_info->crc32);

    return OPRT_OK;
}

UINT32_T tuya_ble_ota_handler(tuya_ble_ota_data_t *ota)
{
    tuya_ble_ota_response_t rsp;
    rsp.type = ota->type;

    if (ota->type != TUYA_BLE_OTA_DATA) {
        TAL_PR_INFO("ota_cmd_type: %d", ota->type);
        TAL_PR_HEXDUMP_INFO("ota_cmd_data", ota->p_data, ota->data_len);
    }

    switch (ota->type) {
    case TUYA_BLE_OTA_REQ: {
        tuya_ble_ota_req_handler(ota->p_data, ota->data_len, &rsp);
    }
    break;

    case TUYA_BLE_OTA_FILE_INFO: {
        tuya_ble_ota_file_info_handler(ota->p_data, ota->data_len, &rsp);
    }
    break;

    case TUYA_BLE_OTA_FILE_OFFSET_REQ: {
        tuya_ble_ota_file_offset_handler(ota->p_data, ota->data_len, &rsp);
    }
    break;

    case TUYA_BLE_OTA_DATA: {
        tuya_ble_ota_data_handler(ota->p_data, ota->data_len, &rsp);
    }
    break;

    case TUYA_BLE_OTA_END: {
        tuya_ble_ota_end_handler(ota->p_data, ota->data_len, &rsp);
    }
    break;

#if defined(TUYA_BLE_OTA_SIGNATURE_ENABLE) && (TUYA_BLE_OTA_SIGNATURE_ENABLE == 1)
    case TUYA_BLE_OTA_SIGNATURE_DATA: {
        tuya_ble_ota_signature_data_handler(ota->p_data, ota->data_len, &rsp);
    }
    break;

    case TUYA_BLE_OTA_SIGNATURE_KEY_UPDATE: {
        // not support
    } break;
#endif

    default: {
    } break;
    }
    return OPRT_OK;
}

UINT32_T tuya_ble_ota_disconn_handler(VOID_T)
{
    if (sg_ota_state >= TUYA_BLE_OTA_REQ) {
        return tuya_ble_ota_exit();
    }
    return 1;
}

VOID_T tuya_ble_ota_set_status(INT32_T status)
{
    sg_ota_state = status;
}

INT32_T tuya_ble_ota_get_status(VOID_T)
{
    return sg_ota_state;
}

#else//for single bank ota

STATIC UINT32_T tuya_ble_ota_rsp(tuya_ble_ota_response_t *rsp, VOID_T *rsp_data, UINT16_T data_size)
{
    if (rsp->type != TUYA_BLE_OTA_DATA) {
        TAL_PR_HEXDUMP_INFO("ota_rsp_data", rsp_data, data_size);
    }

    rsp->p_data = rsp_data;
    rsp->data_len = data_size;
    return tuya_ble_ota_response(rsp);
}

STATIC UINT32_T tuya_ble_ota_prepare_notification_req_handler(UINT8_T *cmd, UINT16_T cmd_size, tuya_ble_ota_response_t *rsp)
{
    if (sg_ota_state != TUYA_BLE_OTA_STATE_UNKNOWN) {
        TAL_PR_ERR("%s ota_state error: %d", __FUNCTION__, sg_ota_state);

        tuya_ble_ota_prerare_notification_rsp_t req_rsp = {0};
        req_rsp.version = 0;
        req_rsp.type = 0;
        req_rsp.state = 1; //refuse ota
        tuya_ble_ota_rsp(rsp, &req_rsp, SIZEOF(tuya_ble_ota_prerare_notification_rsp_t));
        return OPRT_COM_ERROR;
    }

    tuya_ble_ota_prerare_notification_t *tuya_ble_ota_prerare_notification = (tuya_ble_ota_prerare_notification_t *)cmd;
    if ((cmd_size != 2) || (tuya_ble_ota_prerare_notification->version != 0) || (tuya_ble_ota_prerare_notification->type != 0)) {
        TAL_PR_ERR("%s param error", __FUNCTION__);

        tuya_ble_ota_prerare_notification_rsp_t req_rsp = {0};
        req_rsp.version = 0;
        req_rsp.type = 0;
        req_rsp.state = 1; //refuse ota
        tuya_ble_ota_rsp(rsp, &req_rsp, SIZEOF(tuya_ble_ota_prerare_notification_rsp_t));

        return OPRT_COM_ERROR;
    }

    tuya_ble_gap_addr_t addr;
    tuya_ble_gap_addr_get(&addr);
    tuya_ble_storage_write_pid(tuya_ble_current_para.pid_type, tuya_ble_current_para.pid_len, tuya_ble_current_para.pid);
    tuya_ble_storage_device_save_mac(addr.addr);
    tuya_ble_storage_device_save_ota_status(TUYA_BLE_SINGLE_BANK_OTA_STATUS_PREPARING);
    tuya_ble_device_delay_ms(1000);

    tuya_ble_ota_prerare_notification_rsp_t req_rsp = {0};
    req_rsp.version = 0;
    req_rsp.type = 0;
    req_rsp.state = 0; //accept ota
    tuya_ble_ota_rsp(rsp, &req_rsp, SIZEOF(tuya_ble_ota_prerare_notification_rsp_t));

    sg_ota_state = TUYA_BLE_OTA_PREPARE_NOTIFICATION;

    return OPRT_OK;
}

UINT32_T tuya_ble_ota_init(VOID_T)
{
    return OPRT_OK;
}

UINT32_T tuya_ble_ota_handler(tuya_ble_ota_data_t *ota)
{
    tuya_ble_ota_response_t rsp;
    rsp.type = ota->type;

    if (ota->type != TUYA_BLE_OTA_DATA) {
        TAL_PR_INFO("ota_cmd_type: %d", ota->type);
        TAL_PR_HEXDUMP_INFO("ota_cmd_data", ota->p_data, ota->data_len);
    }

    switch (ota->type) {
    case TUYA_BLE_OTA_PREPARE_NOTIFICATION: {
        tuya_ble_ota_prepare_notification_req_handler(ota->p_data, ota->data_len, &rsp);
    }
    break;

    default: {
    } break;
    }
    return OPRT_OK;
}

UINT32_T tuya_ble_ota_disconn_handler(VOID_T)
{
    if (sg_ota_state >= TUYA_BLE_OTA_REQ) {
        return tuya_ble_device_reset();
    }
    return 1;
}

VOID_T tuya_ble_ota_set_status(INT32_T status)
{
    sg_ota_state = status;
}

INT32_T tuya_ble_ota_get_status(VOID_T)
{
    return sg_ota_state;
}

#endif

#if defined(TUYA_BLE_OTA_SIGNATURE_ENABLE) && (TUYA_BLE_OTA_SIGNATURE_ENABLE == 1)

OPERATE_RET tuya_ble_ota_sha256_create_init(void **p_ctx)
{
    mbedtls_sha256_context *context = (void *) tal_malloc(sizeof(mbedtls_sha256_context));
    if (context) {
        mbedtls_sha256_init(context);
    } else {
        return OPRT_MALLOC_FAILED;
    }

    *p_ctx = context;

    return OPRT_OK;
}

OPERATE_RET tuya_ble_ota_sha256_free(void *ctx)
{
    if (ctx != NULL) {
        mbedtls_sha256_free(ctx);
        tal_free(ctx);
    }

    return OPRT_OK;
}

OPERATE_RET tuya_ble_ota_sha256_starts_ret(void *ctx, int32_t is224)
{
    if (mbedtls_sha256_starts_ret(ctx, is224) != 0) {
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

OPERATE_RET tuya_ble_ota_sha256_update_ret(void *ctx, const uint8_t *input, size_t ilen)
{
    if (mbedtls_sha256_update_ret(ctx, input, ilen) != 0) {
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

OPERATE_RET tuya_ble_ota_sha256_finish_ret(void *ctx, uint8_t output[32])
{
    if (mbedtls_sha256_finish_ret(ctx, output) != 0) {
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

static OPERATE_RET tuya_ble_ota_get_signature_in_flash(void)
{
    OPERATE_RET ret = OPRT_OK;

    // init
    ret = tuya_ble_ota_sha256_create_init(&sg_ctx);
    if (OPRT_OK != ret) {
        return ret;
    }

    ret = tuya_ble_ota_sha256_starts_ret(sg_ctx, 0);
    if (OPRT_OK != ret) {
        tuya_ble_ota_sha256_free(sg_ctx);
        return ret;
    }

    // calculate
    UINT32_T start_addr = BOARD_FLASH_OTA_START_ADDR;
    UINT32_T package_count = sg_firmware_len / TUYA_BLE_OTA_PKG_LEN;
    UINT32_T package_remainder = sg_firmware_len % TUYA_BLE_OTA_PKG_LEN;

    UINT8_T *tmp_buf = tal_malloc(TUYA_BLE_OTA_PKG_LEN);
    if (tmp_buf == NULL) {
        TAL_PR_ERR("Error: tkl_ota_get_crc32_in_flash malloc failed!");
        return 0;
    }

    for (UINT32_T idx = 0; idx < package_count; idx++) {
        tal_flash_read_for_ota(start_addr, tmp_buf, TUYA_BLE_OTA_PKG_LEN);
        if (idx == 0) {
            UINT8_T *p_buf = NULL;
            UINT32_T size = tuya_ble_ota_get_first_page(&p_buf);
            if ((size != 0) && (size < TUYA_BLE_OTA_PKG_LEN)) {
                memcpy(tmp_buf, p_buf, size);
            }
        }

        ret = tuya_ble_ota_sha256_update_ret(sg_ctx, tmp_buf, TUYA_BLE_OTA_PKG_LEN);
        if (OPRT_OK != ret) {
            tuya_ble_ota_sha256_free(sg_ctx);
            return ret;
        }
        start_addr += TUYA_BLE_OTA_PKG_LEN;
        tal_watchdog_refresh();
    }

    if (package_remainder > 0) {
        tal_flash_read_for_ota(start_addr, tmp_buf, TUYA_BLE_OTA_PKG_LEN);
        ret = tuya_ble_ota_sha256_update_ret(sg_ctx, tmp_buf, package_remainder);
        if (OPRT_OK != ret) {
            tuya_ble_ota_sha256_free(sg_ctx);
            return ret;
        }
        start_addr += package_remainder;
    }

    if (tmp_buf != NULL) {
        tal_free(tmp_buf);
    }

    // check
    uint8_t hash[32];

    ret = tuya_ble_ota_sha256_finish_ret(sg_ctx, hash);
    if (OPRT_OK != ret) {
        tuya_ble_ota_sha256_free(sg_ctx);
        return ret;
    }
    tuya_ble_ota_sha256_free(sg_ctx);

    uint8_t true_signature_key[64] = {0};
    tal_util_xor((void *)rand_num, (void *)sg_signature_key_info.key, 64, true_signature_key);
    tal_watchdog_refresh();
    return tbs_crypto_ecc_verify(true_signature_key, hash, 32, sg_signature_data.signature_data);
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tal_flash_read_for_ota(UINT32_T addr, UCHAR_T *dst, UINT32_T size)
{
    return tal_flash_read(addr, dst, size);
}

TUYA_WEAK_ATTRIBUTE uint32_t tuya_ble_ota_get_first_page(UINT8_T **pp_buf)
{
    return 0;
}

#endif // TUYA_BLE_OTA_SIGNATURE_ENABLE

TUYA_WEAK_ATTRIBUTE OPERATE_RET tuya_ble_ota_make_ota_firmware_valid(void)
{
    return OPRT_NOT_SUPPORTED;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tuya_ble_ota_make_ota_firmware_invalid(void)
{
    return OPRT_NOT_SUPPORTED;
}

#endif // TUYA_BLE_FEATURE_OTA_ENABLE

