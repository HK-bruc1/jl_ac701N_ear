/**
 * @file tal_ble_x_demo.c
 * @brief This is tal_ble_x_demo file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tal_ble_x_demo.h"
#include "tuya_ble_type.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_api.h"
#include "tuya_ble_port.h"
#include "tuya_ble_main.h"
#include "tuya_ble_internal_config.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_mutli_tsf_protocol.h"
#include "tuya_ble_secure.h"
#include "tuya_ble_main.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_log.h"
#include "tuya_ble_gatt_send_queue.h"
#include "tal_feature_ext_module.h"

#if (TUYA_BLE_FEATURE_EXT_MODULE_ENABLE)

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/


/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
static ext_module_info_t ext_module_info;
static ENUM_EM_TYPE em_type = EM_TYPE_WIFI;
static UINT8_T priority[2] = {BLE, MQTT};

/**
 * @brief   Extern module dev info format, The data format is as follows:
 *
 *  -------------------------------------------------------------------------------------------------------
 * |         1byte         |      1byte      |    1byte     |    1+2+n bytes    |   ...   |   1+2+n bytes |
 * |     Include_EM_Info   |     EM_Type     |    N_TLD     |       TLD1        |   ...   |    TLDn       |
 *  -------------------------------------------------------------------------------------------------------
 *  Include_EM_Info:
 *          0x01 - include em info
 *
 *  EM_Type:
 *          0x01 - NB module
 *          0x02 - WiFi module
 *          0x03 - Cat.x module
 *          0x04 - zigbee module
 *
 *  N_TLD: the number of subsequent TLDs
 *
 *  TLD: type[1byte] + length[2bytes] + data[nbytes]
 *  ---------------------------------------------
 *  |         TYPE=1 em dev info                |
 *  ---------------------------------------------
 *  |   type    |   length  |      data         |
 *  |   0x01    |     N     |   dev info json   |
 *  ---------------------------------------------
 *
 *  ---------------------------------------------
 *  |         TYPE=2 em dev insert state        |
 *  ---------------------------------------------
 *  |   type    |   length  |         data      |
 *  |   0x02    |   0x0001  |  0-pop; 1-insert  |
 *  ---------------------------------------------
 *
 *  ---------------------------------------------
 *  |         TYPE=3 communicate priority       |
 *  ---------------------------------------------
 *  |   type    |   length  |       data        |
 *  |           |           |      LAN=0        |
 *  |           |           |      MQTT=1       |
 *  |           |           |      HTTP=2       |
 *  |   0x03    |    0x0002 |      BLE=3        |
 *  |           |           |      SIGMESH=4    |
 *  |           |           |      TUYAMESH=5   |
 *  |           |           |      BEACON=6     |
 *  ---------------------------------------------
 *
 *  ---------------------------------------------
 *  |         TYPE=5 em dev binding state       |
 *  ---------------------------------------------
 *  |   type    |   length  |       data        |
 *  |           |           |    0-unbinding    |
 *  |   0x05    |   0x0001  |    1-binding      |
 *  ---------------------------------------------
 *
 *  ---------------------------------------------
 *  |         TYPE=7 is active report state     |
 *  ---------------------------------------------
 *  |   type    |   length  |       data        |
 *  |           |           |   0-app query     |
 *  |   0x07    |   0x0001  |   1-active report |
 *  ---------------------------------------------
 * */

UINT8_T Ext_Module_Info_Example_Data[] = {
    0x01,                           /* include em info */
    0x02,                           /* em type, here is Wi-Fi*/
    0x05,                           /* TLD gpoup numbers */

    0x01, 0x00, 0x00,               /* TLD, type=1 */
    0x02, 0x00, 0x01, 0x01,         /* TLD, type=2 */
    0x03, 0x00, 0x02, 0x03, 0x01,   /* TLD, type=3 */
    0x05, 0x00, 0x01, 0x00,         /* TLD, type=5 */
    0x07, 0x00, 0x01, 0x00,         /* TLD, type=7 */
};

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




tuya_ble_status_t tuya_ble_ext_module_info_report(ext_module_info_t *p_data)
{
    UINT8_T *tld_buf = NULL;
    UINT16_T tld_len = 0;
    UINT16_T need_malloc_len;
    UINT8_T em_n_tld = 0;
    tuya_ble_status_t ret;

    if (p_data->inc_em_info != false && p_data->inc_em_info != true) {
        TAL_PR_ERR("em param err-1");
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    if (p_data->inc_em_info && p_data->em_type >= EM_TYPE_MAX) {
        TAL_PR_ERR("em param err-2");
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    need_malloc_len = (3 + (TLD_TYPE_MAX * 4) + p_data->em_dev_basic_info_len + p_data->communication_priority_len);
    tld_buf = (UINT8_T *)tuya_ble_malloc(need_malloc_len);
    if (tld_buf == NULL) {
        TAL_PR_ERR("em param err-3");
        return TUYA_BLE_ERR_NO_MEM;
    }

    tld_buf[tld_len++] = p_data->inc_em_info;  /* include emt info */

    if (p_data->inc_em_info != 0) {
        if ((p_data->em_insert_state != 0 && p_data->em_insert_state != 1)) {
            tuya_ble_free(tld_buf);
            TAL_PR_ERR("em param err-4");
            return TUYA_BLE_ERR_INVALID_PARAM;
        }

        tld_buf[tld_len++] = p_data->em_type;  /* ext module type */
        tld_buf[tld_len++] = 0x00; /* TLD numbers, auto calc */

        /* tld- em insert state */
        tld_buf[tld_len++] = TLD_TYPE_EM_INSERT_STATE;
        tld_buf[tld_len++] = 0x00;
        tld_buf[tld_len++] = 0x01;
        tld_buf[tld_len++] = p_data->em_insert_state; // 0-pop  1-insert
        em_n_tld += 1;

        if (p_data->em_insert_state != 0) {
            /* tld- em dev basic info */
            tld_buf[tld_len++] = TLD_TYPE_EM_BASIC_INFO;
            tld_buf[tld_len++] = (UINT8_T)(p_data->em_dev_basic_info_len >> 8);
            tld_buf[tld_len++] = (UINT8_T)(p_data->em_dev_basic_info_len);

            if ((p_data->em_dev_basic_info != NULL) && (p_data->em_dev_basic_info_len != 0)) {
                memcpy(&tld_buf[tld_len], p_data->em_dev_basic_info, p_data->em_dev_basic_info_len);
                tld_len += p_data->em_dev_basic_info_len;
            }

            em_n_tld += 1;

            /* tls- em dev binding state */
            if (p_data->em_binding_state != 0 && p_data->em_binding_state != 1) {
                tuya_ble_free(tld_buf);
                TAL_PR_ERR("em param err-5");
                return TUYA_BLE_ERR_INVALID_PARAM;
            }

            tld_buf[tld_len++] = TLD_TYPE_EM_BINDING_STATE;
            tld_buf[tld_len++] = 0x00;
            tld_buf[tld_len++] = 0x01;
            tld_buf[tld_len++] = p_data->em_binding_state; // 0-unbinding 1-binding
            em_n_tld += 1;

            /* tld- dev communicate priority */
            if (p_data->communication_priority_len == 0 || \
                p_data->communication_priority_len > COMMUNICATE_PRIORITY_MAX) {
                tuya_ble_free(tld_buf);
                TAL_PR_ERR("em param err-6 %d %d", p_data->communication_priority_len, p_data->communication_priority);
                return TUYA_BLE_ERR_INVALID_PARAM;
            }

            tld_buf[tld_len++] = TLD_TYPE_EM_COMMUNICATE_PRIORITY;
            tld_buf[tld_len++] = 0x00;
            tld_buf[tld_len++] = p_data->communication_priority_len;
            memcpy(&tld_buf[tld_len], p_data->communication_priority, p_data->communication_priority_len);
            tld_len += p_data->communication_priority_len;
            em_n_tld += 1;
        }

        /* tld- active report / passive query */
        if (p_data->active_report_state != 0 && p_data->active_report_state != 1) {
            tuya_ble_free(tld_buf);
            TAL_PR_ERR("em param err-7");
            return TUYA_BLE_ERR_INVALID_PARAM;
        }

        tld_buf[tld_len++] = TLD_TYPE_EM_REPORT_STATE;
        tld_buf[tld_len++] = 0x00;
        tld_buf[tld_len++] = 0x01;
        tld_buf[tld_len++] = p_data->active_report_state;
        em_n_tld += 1;
    }

    if (em_n_tld < TLD_TYPE_MAX) {
        tld_buf[2] = em_n_tld;
    } else {
        tuya_ble_free(tld_buf);
        TAL_PR_ERR("em param err-8");
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    ret = tuya_ble_ext_module_info_asynchronous_response(tld_buf, tld_len);
    // TAL_PR_HEXDUMP_INFO("ext_module_info_rsp", tld_buf, tld_len); //## ext_module_info_rsp

    tuya_ble_free(tld_buf);
    return ret;
}

static tuya_ble_status_t tuya_ble_ext_module_get_communicate_priprity(ENUM_EM_TYPE emt, UINT8_T *p_priority, UINT8_T *priority_len)
{
    tuya_ble_status_t ret = TUYA_BLE_SUCCESS;
    UINT8_T pri[16] = {0};
    UINT8_T pri_len = 2;

    if (p_priority == NULL || priority_len == NULL) {
        return TUYA_BLE_ERR_INVALID_ADDR;
    }

    pri[0] = BLE;
    switch (emt) {
    case EM_TYPE_NB:
        pri[1] = HTTP;
        break;

    case EM_TYPE_WIFI:
        pri[1] = MQTT;
        break;

    case EM_TYPE_CAT:
        pri[1] = MQTT;
        break;

    case EM_TYPE_ZIGBEE:
        pri[1] = MQTT;
        break;

    default:
        ret = TUYA_BLE_ERR_INVALID_PARAM;
        break;
    }

    memcpy(p_priority, pri, pri_len);
    *priority_len = pri_len;

    return ret;
}

tuya_ble_status_t tuya_ble_ext_module_info_report_custom_all(ENUM_EM_TYPE emt, BOOL_T insert_state, BOOL_T binding_state, BOOL_T active_report_state, VOID_T *basic_info, UINT16_T basic_info_len, VOID_T *communication_priority, UINT8_T priority_group_len)
{
    ext_module_info_t em_info;

    memset((CHAR_T *)&em_info, 0x00, sizeof(ext_module_info_t));

    em_info.inc_em_info = true;
    em_info.em_type = emt;

    if (communication_priority == NULL || priority_group_len == 0) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    em_info.communication_priority_len = priority_group_len;
    memcpy(em_info.communication_priority, communication_priority, priority_group_len);

    if ((basic_info != NULL) && (basic_info_len != 0)) {
        memcpy(em_info.em_dev_basic_info, basic_info, basic_info_len);
        em_info.em_dev_basic_info_len = basic_info_len;
    } else {
        em_info.em_dev_basic_info = NULL;
        em_info.em_dev_basic_info_len = 0;
    }

    em_info.em_insert_state = insert_state;
    em_info.em_binding_state = binding_state;
    em_info.active_report_state = active_report_state;

    return tuya_ble_ext_module_info_report(&em_info);
}

tuya_ble_status_t tuya_ble_ext_module_info_report_custom(ENUM_EM_TYPE emt, BOOL_T insert_state, BOOL_T binding_state, BOOL_T active_report_state)
{
    ext_module_info_t em_info;

    memset((CHAR_T *)&em_info, 0x00, sizeof(ext_module_info_t));

    em_info.inc_em_info = true;
    em_info.em_type = emt;
    tuya_ble_ext_module_get_communicate_priprity(emt, em_info.communication_priority, &em_info.communication_priority_len);

    em_info.em_dev_basic_info = NULL;
    em_info.em_dev_basic_info_len = 0;

    em_info.em_insert_state = insert_state;
    em_info.em_binding_state = binding_state;
    em_info.active_report_state = active_report_state;

    return tuya_ble_ext_module_info_report(&em_info);
}

tuya_ble_status_t tuya_ble_ext_module_info_report_example_1(VOID_T)
{
    memset((CHAR_T *)&ext_module_info, 0x00, sizeof(ext_module_info_t));

    ext_module_info.inc_em_info = true;
    ext_module_info.em_type = em_type;

    ext_module_info.em_dev_basic_info = NULL;
    ext_module_info.em_dev_basic_info_len = 0;

    ext_module_info.communication_priority_len = sizeof(priority);
    memcpy(ext_module_info.communication_priority, priority, ext_module_info.communication_priority_len);

    ext_module_info.em_insert_state = true;
    ext_module_info.em_binding_state = false;
    ext_module_info.active_report_state = false;

    return tuya_ble_ext_module_info_report(&ext_module_info);
}

tuya_ble_status_t tuya_ble_ext_module_info_report_example_2(VOID_T)
{
    memset((CHAR_T *)&ext_module_info, 0x00, sizeof(ext_module_info_t));

    ext_module_info.inc_em_info = true;
    ext_module_info.em_type = em_type;

    ext_module_info.em_dev_basic_info = NULL;
    ext_module_info.em_dev_basic_info_len = 0;

    ext_module_info.communication_priority_len = sizeof(priority);
    memcpy(ext_module_info.communication_priority, priority, ext_module_info.communication_priority_len);

    ext_module_info.em_insert_state = true;
    ext_module_info.em_binding_state = true;
    ext_module_info.active_report_state = false;

    return tuya_ble_ext_module_info_report(&ext_module_info);
}

tuya_ble_status_t tuya_ble_ext_module_info_report_example_3(VOID_T)
{
    memset((CHAR_T *)&ext_module_info, 0x00, sizeof(ext_module_info_t));

    ext_module_info.inc_em_info = true;
    ext_module_info.em_type = em_type;

    ext_module_info.em_dev_basic_info = NULL;
    ext_module_info.em_dev_basic_info_len = 0;

    ext_module_info.em_insert_state = false;
    ext_module_info.em_binding_state = true;
    ext_module_info.active_report_state = false;

    return tuya_ble_ext_module_info_report(&ext_module_info);
}

tuya_ble_status_t tuya_ble_ext_module_info_report_example_4(VOID_T)
{
    memset((CHAR_T *)&ext_module_info, 0x00, sizeof(ext_module_info_t));

    ext_module_info.inc_em_info = true;
    ext_module_info.em_type = em_type;

    ext_module_info.em_dev_basic_info = NULL;
    ext_module_info.em_dev_basic_info_len = 0;

    ext_module_info.communication_priority_len = sizeof(priority);
    memcpy(ext_module_info.communication_priority, priority, ext_module_info.communication_priority_len);

    ext_module_info.em_insert_state     = true;
    ext_module_info.em_binding_state    = true;
    ext_module_info.active_report_state = true;

    return tuya_ble_ext_module_info_report(&ext_module_info);
}

#endif /* #if (TUYA_BLE_FEATURE_EXT_MODULE_ENABLE) */

