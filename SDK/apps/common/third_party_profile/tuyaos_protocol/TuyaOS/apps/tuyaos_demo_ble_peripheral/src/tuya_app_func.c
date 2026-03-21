#include "app_dp_parser.h"
#include "app_led.h"

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

#define TUYA_TRIPLE_LENGTH      (DEVICE_ID_LEN + AUTH_KEY_LEN + MAC_STRING_LEN)
extern const u8 *multi_protocol_get_license_ptr(void);

static __tuya_info tuya_info;
static uint32_t tuya_sn = 0;
tuya_triple_info triple_info;

static u8 ascii_to_hex(u8 in)
{
    if (in >= '0' && in <= '9') {
        return in - '0';
    } else if (in >= 'a' && in <= 'f') {
        return in - 'a' + 0x0a;
    } else if (in >= 'A' && in <= 'F') {
        return in - 'A' + 0x0a;
    } else {
        printf("tuya ascii to hex error, data:0x%x", in);
        return 0;
    }
}

static u16 tuya_get_one_info(const u8 *in, u8 *out)
{
    int read_len = 0;
    const u8 *p = in;

    while (TUYA_LEGAL_CHAR(*p) && *p != ',') { //read product_uuid
        *out++ = *p++;
        read_len++;
    }
    return read_len;
}

static void parse_mac_data(u8 *in, u8 *out)
{
    for (int i = 0; i < 6; i++) {
        out[i] = (ascii_to_hex(in[2 * i]) << 4) + ascii_to_hex(in[2 * i + 1]);
    }
}

static uint8_t read_tuya_product_info_from_flash(uint8_t *read_buf, u16 buflen)
{
    uint8_t *rp = read_buf;
    const uint8_t *tuya_ptr = (uint8_t *)multi_protocol_get_license_ptr();
    //printf("tuya_ptr:");
    //put_buf(tuya_ptr, 69);

    if (tuya_ptr == NULL) {
        return FALSE;
    }
    int data_len = 0;
    data_len = tuya_get_one_info(tuya_ptr, rp);
    //put_buf(rp, data_len);
    if (data_len != 16) {
        printf("read uuid err, data_len:%d", data_len);
        put_buf(rp, data_len);
        return FALSE;
    }
    tuya_ptr += 17;

    rp = read_buf + 16;

    data_len = tuya_get_one_info(tuya_ptr, rp);
    //put_buf(rp, data_len);
    if (data_len != 32) {
        printf("read key err, data_len:%d", data_len);
        put_buf(rp, data_len);
        return FALSE;
    }
    tuya_ptr += 33;

    rp = read_buf + 16 + 32;
    data_len = tuya_get_one_info(tuya_ptr, rp);
    //put_buf(rp, data_len);
    if (data_len != 12) {
        printf("read mac err, data_len:%d", data_len);
        put_buf(rp, data_len);
        return FALSE;
    }
    return TRUE;
}

void tuya_get_lic_from_flash_vm(tuya_ble_device_param_t *device_param_p)
{
    uint8_t read_buf[TUYA_TRIPLE_LENGTH] = {0};
    int flash_ret = read_tuya_product_info_from_flash(read_buf, sizeof(read_buf));
    if (flash_ret == TRUE) {
        triple_info.flag = TRIPLE_FLASH;
    } else {
        int vm_read_result = syscfg_read(VM_TUYA_TRIPLE, read_buf, sizeof(read_buf));
        printf("vm result: %d\n", vm_read_result);
        printf("read vm data:");
        put_buf(read_buf, sizeof(read_buf));
        if (vm_read_result == TUYA_TRIPLE_LENGTH) {
            triple_info.flag = TRIPLE_VM;
        } else  {
            printf("tripe  null");
            triple_info.flag = TRIPLE_NULL;
        }
    }
    if (triple_info.flag != TRIPLE_NULL) {
        uint8_t mac_data[6];
        put_buf(read_buf, sizeof(read_buf));
        memcpy(device_param_p->device_id, read_buf, DEVICE_ID_LEN);
        memcpy(device_param_p->auth_key, read_buf + DEVICE_ID_LEN, AUTH_KEY_LEN);
        memcpy(device_param_p->mac_addr_string, read_buf + DEVICE_ID_LEN + AUTH_KEY_LEN, MAC_STRING_LEN);
        parse_mac_data(read_buf + DEVICE_ID_LEN + AUTH_KEY_LEN, mac_data);
        memcpy(device_param_p->mac_addr.addr, mac_data, 6);
        device_param_p->device_id_len = DEVICE_ID_LEN;
        device_param_p->use_ext_license_key = 1;
        device_param_p->product_id_len = TUYA_BLE_PRODUCT_ID_DEFAULT_LEN;
        memcpy((u8 *)&triple_info.data, read_buf, sizeof(read_buf));
    }
}

int tuya_get_triple_info_result()
{
    return triple_info.flag;
}

u16 tuya_get_triple_data(u8 *data)
{
    memcpy(data, triple_info.data, TUYA_TRIPLE_LENGTH);
    return TUYA_TRIPLE_LENGTH;
}

void tuya_triple_vm_write(void)
{
    int vm_read_result = syscfg_write(VM_TUYA_TRIPLE, triple_info.data, TUYA_TRIPLE_LENGTH);
    if (vm_read_result != TUYA_TRIPLE_LENGTH) {
        printf("tuya_triple_vm_write err!\n");
    }
}

extern void tws_conn_send_event(int code, const char *format, ...);
void tuya_set_triple_info(u8 *data)
{
    printf("--------------------------------------------");
    printf("Tuya triple function is no available now!!!");
    printf("--------------------------------------------");
    memcpy(triple_info.data, data, TUYA_TRIPLE_LENGTH);
    int err;
    int msg[5];

    msg[0] = (int)tuya_triple_vm_write;
    msg[1] = 0;
    err = os_taskq_post_type("app_core", Q_CALLBACK, 2, msg);
    //tws_conn_send_event(TWS_EVENT_CHANGE_TRIPLE, "111", 0, 1, 0);
}

int tuya_get_lic_data_len(void)
{
    if (triple_info.flag) {
        return TUYA_TRIPLE_LENGTH;
    }
    return 0;
}

u8 tuya_get_lic_data_type(void)
{
    return (u8)tuya_get_triple_info_result();
}

int tuya_get_lic_data(u8 *buff)
{
    return tuya_get_triple_data(buff);
}

int tuya_set_lic_data(u8 *buff, u16 len)
{
    triple_info.flag = TRIPLE_VM;
    tuya_set_triple_info(buff);
    return len;
}

void tuya_master_send_triple_info_to_slave()
{
    u8 data[62]  = {0};
    u16 data_len;
    if (tws_api_get_role() == TWS_ROLE_MASTER) {
        int ret = tuya_get_triple_info_result();
        printf("master read from [%d]\n", ret);
        data[0] = ret;
        data_len = tuya_get_triple_data(&data[1]);
        printf("master send triple data");
        put_buf(data, data_len + 1);
        tws_api_send_data_to_sibling(data, data_len + 1, TWS_FUNC_ID_TUYAOS_TRIPLE);
    }
}

static void tuya_sn_increase(void)
{
    tuya_sn += 1;
}

void tuya_battry_indicate_case(uint8_t bat_value)
{
    __battery_indicate_data p_dp_data;

    p_dp_data.id = 2;
    p_dp_data.type = TUYA_SEND_DATA_TYPE_DT_VALUE;
    p_dp_data.len = U16_TO_LITTLEENDIAN(1);
    p_dp_data.data = bat_value;

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN == 0x04)
    tuya_ble_dp_data_send(tuya_sn, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITH_RESPONSE, (uint8_t *)&p_dp_data, 5);
    tuya_sn_increase();
#endif
}

void tuya_battry_indicate_left(uint8_t bat_value)
{
    __battery_indicate_data p_dp_data;

    p_dp_data.id = 3;
    p_dp_data.type = TUYA_SEND_DATA_TYPE_DT_VALUE;
    p_dp_data.len = U16_TO_LITTLEENDIAN(1);
    p_dp_data.data = bat_value;

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN == 0x04)
    tuya_ble_dp_data_send(tuya_sn, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITH_RESPONSE, (uint8_t *)&p_dp_data, 5);
    tuya_sn_increase();
#endif
}

void tuya_battry_indicate_right(uint8_t bat_value)
{
    __battery_indicate_data p_dp_data;

    p_dp_data.id = 4;
    p_dp_data.type = TUYA_SEND_DATA_TYPE_DT_VALUE;
    p_dp_data.len = U16_TO_LITTLEENDIAN(1);
    p_dp_data.data = bat_value;

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN == 0x04)
    tuya_ble_dp_data_send(tuya_sn, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITH_RESPONSE, (uint8_t *)&p_dp_data, 5);
    tuya_sn_increase();
#endif
}


void tuya_battery_indicate(u8 left, u8 right, u8 chargebox)
{
    tuya_battry_indicate_right(right);
    tuya_battry_indicate_left(left);
    tuya_battry_indicate_case(chargebox);
}

void battery_indicate(u8 cur_battery_level)
{
#if TCFG_CHARGESTORE_ENABLE
    u8 chargestore = chargestore_get_power_level();
#else
    u8 chargestore = 0;
#endif
    u8 local_charge_bit = get_charge_online_flag() ? BIT(7) : 0;
#if TCFG_USER_TWS_ENABLE
    u8 data = get_tws_sibling_bat_persent();
    printf("battery_indicate %d %d %d \n", cur_battery_level, data, chargestore);
    if (get_bt_tws_connect_status()) {
        /* chargestore_get_tws_remote_info(&data); */
        if (tws_api_get_local_channel() == 'L') {
            tuya_battery_indicate((cur_battery_level * 10) | local_charge_bit, data, chargestore);
        } else {
            tuya_battery_indicate(data, (cur_battery_level * 10) | local_charge_bit, chargestore);
        }
    } else
#endif
    {
        if (tws_api_get_local_channel() == 'L') {
            tuya_battery_indicate((cur_battery_level * 10) | local_charge_bit, 0xff, chargestore);
        } else {
            tuya_battery_indicate(0xff, (cur_battery_level * 10) | local_charge_bit, chargestore);
        }
    }
}

/* 音乐播放状态上报 */
void tuya_play_status_indicate(u8 status)
{
    printf("tuya_play_status_indicate:%d, sn:%d", status, tuya_sn);
    __play_status_indicate_data p_dp_data;

    p_dp_data.id = 7;
    p_dp_data.type = TUYA_SEND_DATA_TYPE_DT_BOOL;
    p_dp_data.len = U16_TO_LITTLEENDIAN(1);
    p_dp_data.data = status;

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN == 0x04)
    tuya_ble_dp_data_send(tuya_sn, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITH_RESPONSE, (uint8_t *)&p_dp_data, 5);
    tuya_sn_increase();
#endif
}

static void tuya_conn_state_indicate()
{
    __tuya_conn_state_data p_dp_data;

    p_dp_data.id = 33;
    p_dp_data.type = TUYA_SEND_DATA_TYPE_DT_ENUM;
    p_dp_data.len = U16_TO_LITTLEENDIAN(1);
    p_dp_data.data = tuya_info.tuya_conn_state;

    printf("tuya_conn_state_indicate state:%d", tuya_info.tuya_conn_state);
#if (TUYA_BLE_PROTOCOL_VERSION_HIGN == 0x04)
    tuya_ble_dp_data_send(tuya_sn, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITH_RESPONSE, (uint8_t *)&p_dp_data, 5);
    tuya_sn_increase();
#endif
}

// 经典蓝牙连接传1,断开传0
void tuya_conn_state_set_and_indicate(uint8_t state)
{
    if (state == 1) {
        tuya_info.tuya_conn_state = TUYA_CONN_STATE_CONNECTED;
    } else {
        tuya_info.tuya_conn_state = TUYA_CONN_STATE_DISCONNECT;
    }
    tuya_conn_state_indicate();
}

static void tuya_bt_name_indicate()
{
    __tuya_bt_name_data p_dp_data;

    uint8_t name_len = strlen(bt_get_local_name());

    p_dp_data.id = 43;
    p_dp_data.type = TUYA_SEND_DATA_TYPE_DT_RAW;
    p_dp_data.len = U16_TO_LITTLEENDIAN(name_len);
    memcpy(p_dp_data.data, bt_get_local_name(), name_len);

    printf("tuya_bt_name_indicate state:%s", p_dp_data.data);
    tuya_ble_dp_data_send(tuya_sn, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITH_RESPONSE, (uint8_t *)&p_dp_data, 4 + name_len);
    tuya_sn_increase();
}

void tuya_data_parse_in_app_core(tuya_ble_cb_evt_param_t *event)
{
    tuya_data_parse(event);
    free(event->dp_received_data.p_data);
    free(event);
}

void tuya_data_parse(tuya_ble_cb_evt_param_t *event)
{
    if (strcmp(os_current_task(), "app_core")) {
        tuya_ble_cb_evt_param_t *event_p = zalloc(sizeof(*event_p));
        memcpy(event_p, event, sizeof(*event_p));
        event_p->dp_received_data.p_data = zalloc(event->dp_received_data.data_len);
        memcpy(event_p->dp_received_data.p_data, event->dp_received_data.p_data, event->dp_received_data.data_len);
        printf("tuya_data_parse post to app_core\n");
        int msg[3] = {(int)tuya_data_parse_in_app_core, 1, (int)event_p};
        int ret = os_taskq_post_type("app_core", Q_CALLBACK, 3, msg);
        if (ret) {
            printf("tuya_data_parse post to app_core err \n");
        }
        return;
    }
    tws_api_auto_role_switch_disable();
    uint32_t sn = event->dp_received_data.sn;
    printf("tuya_data_parse, p_data:0x%x, len:%d", (int)event->dp_received_data.p_data, event->dp_received_data.data_len);
    put_buf(event->dp_received_data.p_data, event->dp_received_data.data_len);
    uint16_t buf_len = event->dp_received_data.data_len;

    uint8_t dp_id = event->dp_received_data.p_data[0];
    uint8_t type = event->dp_received_data.p_data[1];
    uint16_t data_len = TWO_BYTE_TO_DATA((&event->dp_received_data.p_data[2]));
    uint8_t *data = &event->dp_received_data.p_data[4];
    printf("<--------------  tuya_data_parse  -------------->");
    printf("sn = %d, id = %d, type = %d, data_len = %d, data:", sn, dp_id, type, data_len);
    u8 key_value_record[2][6] = {0};
    put_buf(data, data_len);
    u8 value = 0;
    switch (dp_id) {
    case 1:
        //iot播报模式
        printf("tuya iot broadcast set to: %d\n", data[0]);
        break;
    case 5:
        //音量设置
        printf("tuya voice set to :%d\n", data[3]);
        break;
    case 6:
        //切换控制
        printf("tuya change_control: %d\n", data[0]);
        break;
    case 7:
        //播放/暂停
        printf("tuya play state:%d\n", data[0]);
        break;
    case 8:
        //设置降噪模式
        printf("tuya noise_mode: %d\n", data[0]);
        break;
    case 9:
        //设置降噪场景
        printf("tuya noise_scenes:%d\n", data[0]);
        break;
    case 10:
        //设置通透模式
        printf("tuya transparency_scenes: %d\n", data[0]);
        break;
    case 11:
        //设置降噪强度
        printf("tuya noise_set: %d\n", data[3]);
        break;
    case 12:
        //寻找设备
        printf("tuya find device set:%d\n", data[0]);
        break;
    case 13:
        //设备断连提醒
        printf("tuya device disconnect notify set:%d", data[0]);
        break;
    case 14:
        //设备重连提醒
        printf("tuya device reconnect notify set:%d", data[0]);
        break;
    case 15:
        //手机断连提醒
        printf("tuya phone disconnect notify set:%d", data[0]);
        break;
    case 16:
        //手机重连提醒
        printf("tuya phone reconnect notify set:%d", data[0]);
        break;
    case 17:
        //设置eq模式
        printf("tuya eq_mode set:0x%x", data[0]);
        break;
    case 18:
        // 设置eq参数.此处也会设置eq模式
        printf("tuya eq_param set");
        if (tuya_info.eq_info.eq_onoff == 0) {
            printf("tuya eq_data set fail, eq_onoff is 0!");
            return;
        }
        break;
    case 19:
        //左按键1
        break;
    case 20:
        //右按键1
        break;
    case 21:
        //左按键2
        break;
    case 22:
        //右按键2
        break;
    case 23:
        //左按键3
        break;
    case 24:
        //右按键3
        break;
    case 25:
        //按键重置
        printf("tuya key reset, sn:%d", sn);
        return;
    case 26:
        //入耳检测
        printf("tuya inear_detection set:%d\n", data[0]);
        break;
    case 27:
        //贴合度检测
        printf("tuya fit_detection set:%d\n", data[0]);
        break;
    case 31:
        //倒计时
        u32 count_time = FOUR_BYTE_TO_DATA(data);
        printf("tuya count down:%d\n", count_time);
        break;
    case 42:
        break;
    case 43:
        //蓝牙名字
        break;
    case 44:
        // 设置eq开关
        printf("tuya eq_onoff set:%d\n", data[0]);
        break;
    case 45:
        //设置通透强度
        printf("tuya trn_set:%d\n", data[3]);
        break;
    case 62:    // record interrupt (Enum)
        printf("record interrupt:%x\n", data[0]);
        // 0:interrupt start    1:interrupt end
        break;
    case 63:
        printf("tuya recoder req:%x\n", data[0]);
        break;
    case 65:    // buds_mode_switch_request
        printf("buds_mode_switch_request\n");
        break;
    default:
        printf("unknow control msg len = %d\n, data:", data_len);
        break;
    }
    tuya_ble_dp_data_send(sn, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITH_RESPONSE, event->dp_received_data.p_data, data_len + 4);
}

void tuya_info_indicate()
{
    if (bt_a2dp_get_status() == 1) {
        tuya_play_status_indicate(1);
    } else {
        tuya_play_status_indicate(0);
    }
    tuya_conn_state_indicate();
    tuya_bt_name_indicate();
    battery_indicate(get_self_battery_level() + 1);
}

