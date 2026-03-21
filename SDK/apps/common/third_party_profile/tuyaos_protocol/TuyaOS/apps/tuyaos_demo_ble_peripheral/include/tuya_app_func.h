#ifndef __TUYA_APP_FUNC_H__
#define __TUYA_APP_FUNC_H__

#include "user_cfg.h"
#include "tuya_ble_type.h"
#include "tuya_ble_api.h"

//////////////////// user //////////////////

#define TWS_FUNC_ID_TUYAOS_TRIPLE     TWS_FUNC_ID('T', 'R', 'I', 'P')
#define TWS_FUNC_ID_TUYAOS_AUTH_SYNC  TWS_FUNC_ID('T', 'U', 'A', 'U')
#define TWS_FUNC_ID_TUYAOS_STATE      TWS_FUNC_ID('T', 'Y', 'O', 'S')

#define TUYA_KEY_1_ACTION  0//对应 enum key_action
#define TUYA_KEY_2_ACTION  1//对应 enum key_action
#define TUYA_KEY_3_ACTION  4//对应 enum key_action

#define TUYA_KEY_SYNC_VM        0
#define TUYA_BT_NAME_SYNC_VM    1
#define TUYA_EQ_INFO_SYNC_VM    2

#define EQ_CNT 10

#define TWO_BYTE_TO_DATA(x)     ((x[0] << 8) + x[1])
#define U16_TO_LITTLEENDIAN(x)  (((x & 0xff) << 8) + (x & 0xff00))
#define FOUR_BYTE_TO_DATA(x)    ((x[0] << 24) + (x[1] << 16) + (x[2] << 8) + x[3])
#define TUYA_LEGAL_CHAR(c)      ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))

enum {
    TUYA_CONN_STATE_DISCONNECT = 0,
    TUYA_CONN_STATE_CONNECTING,
    TUYA_CONN_STATE_CONNECTED,
};

enum {
    TUYA_SEND_DATA_TYPE_DT_RAW = 0,
    TUYA_SEND_DATA_TYPE_DT_BOOL,
    TUYA_SEND_DATA_TYPE_DT_VALUE,
    TUYA_SEND_DATA_TYPE_DT_STRING,
    TUYA_SEND_DATA_TYPE_DT_ENUM,
    TUYA_SEND_DATA_TYPE_DT_BITMAP,
};



typedef struct {
    uint8_t eq_onoff;
    uint8_t eq_mode;
    char eq_data[EQ_CNT];
} __eq_info;

typedef struct {
    int trn_set;
    int noise_set;
    uint8_t noise_mode;
    uint8_t noise_scenes;
    uint8_t transparency_scenes;
} __noise_info;

typedef struct {
    uint8_t left1;
    uint8_t left2;
    uint8_t left3;
    uint8_t right1;
    uint8_t right2;
    uint8_t right3;
} __key_info;

typedef struct {
    uint8_t case_battery;
    uint8_t left_battery;
    uint8_t right_battery;
} __battery_info;

typedef struct {
    uint8_t led_state;
    uint8_t tuya_conn_state;
    __eq_info eq_info;
    __noise_info noise_info;
    __key_info key_info;
    __battery_info battery_info;
} __tuya_info;

typedef struct {
    uint8_t id;
    uint8_t type;
    uint16_t len;
    uint8_t data;
} __battery_indicate_data;

typedef struct {
    uint8_t id;
    uint8_t type;
    uint16_t len;
    uint8_t data;
} __tuya_conn_state_data;

typedef struct {
    uint8_t id;
    uint8_t type;
    uint16_t len;
    uint8_t data[32];
} __tuya_bt_name_data;

typedef struct {
    uint8_t id;
    uint8_t type;
    uint16_t len;
    uint8_t data;
} __key_indicate_data;

typedef struct {
    uint8_t id;
    uint8_t type;
    uint16_t len;
    uint8_t data;
} __volume_indicate_data;

typedef struct {
    uint8_t login_key[LOGIN_KEY_LEN];
    uint8_t device_virtual_id[DEVICE_VIRTUAL_ID_LEN];
    uint8_t login_key_v2[LOGIN_KEY_V2_LEN];
    uint8_t secret_key[SECRET_KEY_LEN];
    uint8_t bound_flag;
    uint8_t protocol_v2_enable;
} tuya_tws_sync_info_t;

typedef struct {
    uint8_t id;
    uint8_t type;
    uint16_t len;
    uint8_t data;
} __play_status_indicate_data;

typedef struct {
    uint8_t id;
    uint8_t type;
    uint16_t len;
    uint8_t data;
} __eq_onoff_indicate_data;

typedef struct {
    uint8_t id;
    uint8_t type;
    uint16_t len;
    uint8_t version;
    uint8_t eq_num;
    uint8_t eq_mode;
    uint8_t eq_data[10];
} __eq_indicate_data;

typedef struct {
    uint8_t id;
    uint8_t type;
    uint16_t len;
    uint8_t intr_type;
} __recoder_interrupt_data;

typedef struct {
    uint8_t id;
    uint8_t type;
    uint16_t len;
    uint8_t str[4];
} __recoder_recoder_data;

typedef struct {
    uint8_t id;
    uint8_t type;
    uint16_t len;
    uint8_t data[6 + MAC_LEN * 2];
} __buds_mode_switch_response;

/////////////// tuya_app_func  api

struct TUYA_SYNC_INFO {
    char eq_info[EQ_CNT + 1];
    u8 tuya_eq_flag;
    char bt_name[LOCAL_NAME_LEN];
    u8 tuya_bt_name_flag;
    u8 anc_mode;
    u8 volume;
    u8 volume_flag;
    u8 key_r1;
    u8 key_r2;
    u8 key_r3;
    u8 key_l1;
    u8 key_l2;
    u8 key_l3;
    u8 key_change_flag;
    u8 find_device;
    u8 device_conn_flag;
    u8 device_disconn_flag;
    u8 phone_conn_flag;
    u8 phone_disconn_flag;
    u8 key_reset;
};

enum {
    APP_TWS_TUYA_SYNC_EQ  = 0,
    APP_TWS_TUYA_SYNC_ANC = 1,
    APP_TWS_TUYA_SYNC_VOLUME = 2,
    APP_TWS_TUYA_SYNC_KEY_R1 = 3,
    APP_TWS_TUYA_SYNC_KEY_R2 = 4,
    APP_TWS_TUYA_SYNC_KEY_R3 = 5,
    APP_TWS_TUYA_SYNC_KEY_L1 = 6,
    APP_TWS_TUYA_SYNC_KEY_L2 = 7,
    APP_TWS_TUYA_SYNC_KEY_L3 = 8,
    APP_TWS_TUYA_SYNC_FIND_DEVICE = 9,
    APP_TWS_TUYA_SYNC_DEVICE_CONN_FLAG = 10,
    APP_TWS_TUYA_SYNC_DEVICE_DISCONN_FLAG = 11,
    APP_TWS_TUYA_SYNC_PHONE_CONN_FLAG = 12,
    APP_TWS_TUYA_SYNC_PHONE_DISCONN_FLAG = 13,
    APP_TWS_TUYA_SYNC_BT_NAME = 14,
    APP_TWS_TUYA_SYNC_KEY_RESET = 15,
};


extern void tuya_key_info_indicate(void);
extern void tuya_key_reset_indicate();
extern void tuya_battry_indicate_case(uint8_t bat_value);
extern void tuya_battry_indicate_left(uint8_t bat_value);
extern void tuya_battry_indicate_right(uint8_t bat_value);
extern void tuya_battery_indicate(u8 left, u8 right, u8 chargebox);
extern void battery_indicate(u8 cur_battery_level);
extern void tuya_conn_state_set_and_indicate(uint8_t state);
extern void tuya_data_parse(tuya_ble_cb_evt_param_t *event);
extern void tuya_info_indicate(void);

enum {
    TRIPLE_NULL = 0,
    TRIPLE_FLASH,
    TRIPLE_VM,
};

typedef struct {
    u8  flag;
    u8  data[0x50];
} tuya_triple_info;

extern void tuya_tws_bind_info_sync();
extern u16 tuya_get_triple_data(u8 *data);
extern int tuya_get_triple_info_result();
extern void tuya_triple_vm_write(void);
extern void tuya_set_triple_info(u8 *data);
extern int tuya_get_lic_data_len(void);
extern u8 tuya_get_lic_data_type(void);
extern int tuya_get_lic_data(u8 *buff);
extern int tuya_set_lic_data(u8 *buff, u16 len);
extern void tuya_get_lic_from_flash_vm(tuya_ble_device_param_t *device_param_p);
extern void tuya_master_send_triple_info_to_slave();


#endif

