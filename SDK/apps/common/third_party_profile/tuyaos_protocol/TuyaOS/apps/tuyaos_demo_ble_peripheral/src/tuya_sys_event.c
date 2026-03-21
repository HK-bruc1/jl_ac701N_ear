#include "app_dp_parser.h"
#include "tuya_ble_main.h"
#include "tuya_ble_protocol_callback.h"
#include "tal_bluetooth_def.h"
#include "tal_bluetooth.h"

#include "app_msg.h"
#include "bt_tws.h"
#include "key_event_deal.h"
#include "user_cfg.h"
#include "effects/audio_eq.h"
#include "audio_config.h"
#include "avctp_user.h"
#include "app_chargestore.h"
#include "app_power_manage.h"
#include "app_main.h"

tuya_tws_sync_info_t tuya_tws_sync_info;

void tuya_tws_bind_info_sync()
{
    printf("tuya_tws_bind_info_sync");
    if (tws_api_get_role() == TWS_ROLE_MASTER) {
        memcpy(tuya_tws_sync_info.login_key, tuya_ble_current_para.sys_settings.login_key, LOGIN_KEY_LEN);
        memcpy(tuya_tws_sync_info.device_virtual_id, tuya_ble_current_para.sys_settings.device_virtual_id, DEVICE_VIRTUAL_ID_LEN);
        memcpy(tuya_tws_sync_info.login_key_v2, tuya_ble_current_para.sys_settings.login_key_v2, LOGIN_KEY_V2_LEN);
        memcpy(tuya_tws_sync_info.secret_key, tuya_ble_current_para.sys_settings.secret_key, SECRET_KEY_LEN);
        tuya_tws_sync_info.bound_flag = tuya_ble_current_para.sys_settings.bound_flag;
        tuya_tws_sync_info.protocol_v2_enable = tuya_ble_current_para.sys_settings.protocol_v2_enable;

        int ret = tws_api_send_data_to_sibling(&tuya_tws_sync_info, sizeof(tuya_tws_sync_info), TWS_FUNC_ID_TUYAOS_AUTH_SYNC);
    } else {
        printf("slaver don't sync pair info");
    }
}

static void bt_tws_tuya_auth_info_received(void *data, u16 len, bool rx)
{
    printf("bt_tws_tuya_auth_info_received, rx:%d", rx);
    if (rx) {
        tuya_tws_sync_info_t *recv_info = data;
        memcpy(tuya_ble_current_para.sys_settings.login_key, recv_info->login_key, LOGIN_KEY_LEN);
        memcpy(tuya_ble_current_para.sys_settings.device_virtual_id, recv_info->device_virtual_id, DEVICE_VIRTUAL_ID_LEN);
        memcpy(tuya_ble_current_para.sys_settings.login_key_v2, recv_info->login_key_v2, LOGIN_KEY_V2_LEN);
        memcpy(tuya_ble_current_para.sys_settings.secret_key, recv_info->secret_key, SECRET_KEY_LEN);
        tuya_ble_current_para.sys_settings.bound_flag = recv_info->bound_flag;
        tuya_ble_current_para.sys_settings.protocol_v2_enable = recv_info->protocol_v2_enable;
        tuya_ble_custom_evt_send(APP_EVT_TWS_SYNC_RECV);
    }
}

REGISTER_TWS_FUNC_STUB(app_tuya_auth_stub) = {
    .func_id = TWS_FUNC_ID_TUYAOS_AUTH_SYNC,
    .func    = bt_tws_tuya_auth_info_received,
};


void bt_tws_tuya_triple_sync(void *data, u16 len, bool rx)
{
    //先是主机调用sibling 从机调用该函数 通过判断返回的三元组情况，是否给主机sibling
    u8 info_type = 3;
    u8 change_data[62];
    printf("slave rx: %d\n", rx);
    if (rx) {
        u8 slave_data[60];
        u16 slave_data_len;
        slave_data_len = tuya_get_triple_data(slave_data);
        info_type = ((u8 *)data)[0];

        printf("slave recv master triple data :[%d],info_type :[%d]\n", len, info_type);
        put_buf(data, len);
        int ret = tuya_get_triple_info_result();//获取从机的三元组情况
        put_buf(slave_data, slave_data_len);
        printf("slave self triple data:[%d],info_type:%d\n", slave_data_len, ret);
        switch (info_type) {
        case TRIPLE_NULL:
            if (ret == TRIPLE_FLASH) {
                change_data[0] = 1;
                memcpy(&change_data[1], slave_data, slave_data_len);
                tws_api_send_data_to_sibling(change_data, slave_data_len + 1, TWS_FUNC_ID_TUYAOS_TRIPLE);
            }
            printf("slave triple data 0");
            break;
        case TRIPLE_FLASH:
            if (ret != TRIPLE_FLASH) {
                printf("slave write triple data 1");
                tuya_set_triple_info(&data[1]);
            }
            break;
        case TRIPLE_VM:
            printf("slave triple data 62");
            break;
        }
    }
}

REGISTER_TWS_FUNC_STUB(app_tuya_triple_state) = {
    .func_id = TWS_FUNC_ID_TUYAOS_TRIPLE,
    .func    = bt_tws_tuya_triple_sync,
};

u8 tuya_le_role = TWS_ROLE_MASTER;
extern TAL_BLE_ADV_PARAMS_T tal_adv_param;
extern void tws_ota_app_event_deal(u8 evevt);
int tuya_bt_tws_event_handler(int *msg)
{
    struct tws_event *bt = (struct tws_event *)msg;
    int role = bt->args[0];
    int phone_link_connection = bt->args[1];
    int reason = bt->args[2];

    printf("\ntuya_bt_tws_event_handler event:0x%x\n", bt->event);
    switch (bt->event) {
    case TWS_EVENT_CONNECTED:
        //bt_ble_adv_enable(1);
        if (tws_api_get_role() == TWS_ROLE_SLAVE) {
            //master enable
            printf("\nTws Connect Slave!!!\n\n");
            /*从机ble关掉*/
            tuya_le_role = TWS_ROLE_SLAVE;
            tuya_ble_gap_disconnect();
            tal_ble_advertising_stop();
        } else {
            printf("master send\n");
            tuya_le_role = TWS_ROLE_MASTER;
            tuya_master_send_triple_info_to_slave();
        }
        tuya_tws_bind_info_sync();

        break;
    case TWS_EVENT_PHONE_LINK_DETACH:
        /*
         * 跟手机的链路LMP层已完全断开, 只有tws在连接状态才会收到此事件
         */
        break;
    case TWS_EVENT_CONNECTION_DETACH:
        /*
         * TWS连接断开
         */
        if (app_var.goto_poweroff_flag) {
            break;
        }

        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            tal_ble_advertising_start(&tal_adv_param);
        }
        break;
    case TWS_EVENT_SYNC_FUN_CMD:
        break;
    case TWS_EVENT_ROLE_SWITCH:
        printf("tuya_role_switch");
        tuya_le_role = role;
        tuya_ble_adv_change();
        break;
    }

#if OTA_TWS_SAME_TIME_ENABLE
    tws_ota_app_event_deal(bt->event);
#endif
    return 0;
}

APP_MSG_HANDLER(tuya_tws_msg_entry) = {
    .owner      = 0xff,
    .from       = MSG_FROM_TWS,
    .handler    = tuya_bt_tws_event_handler,
};



