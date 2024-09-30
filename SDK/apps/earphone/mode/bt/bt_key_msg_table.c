#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".key_msg_handler.data.bss")
#pragma data_seg(".key_msg_handler.data")
#pragma const_seg(".key_msg_handler.text.const")
#pragma code_seg(".key_msg_handler.text")
#endif
#include "key_driver.h"
#include "app_main.h"
#include "init.h"
#include "app_default_msg_handler.h"
#include "a2dp_player.h"
#include "tws_dual_share.h"
#include "low_latency.h"
#include "poweroff.h"
#include "bt_key_func.h"
#include "low_latency.h"
#if ((TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_UNICAST_SINK_EN | LE_AUDIO_JL_UNICAST_SINK_EN)))
#include "app_le_connected.h"
#endif


#define LOG_TAG             "[EARPHONE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"



#if TCFG_ADKEY_ENABLE
const int adkey_msg_table[10][KEY_ACTION_MAX] = {
    //短按, 长按, hold, 长按抬起,
    //双击, 3击, 4击, 5击,
    //按住3s, 按住5s
    [0] = {
        APP_MSG_MUSIC_PP,   APP_MSG_CALL_HANGUP,   APP_MSG_NULL,   APP_MSG_NULL,
        APP_MSG_GOTO_NEXT_MODE,       APP_MSG_NULL,   APP_MSG_NULL,   APP_MSG_NULL,
        APP_MSG_NULL,       APP_MSG_POWER_OFF,
    },
    [1] = {
        APP_MSG_MUSIC_PREV, APP_MSG_VOL_DOWN,   APP_MSG_VOL_DOWN,   APP_MSG_NULL,
        APP_MSG_NULL,       APP_MSG_NULL,   APP_MSG_NULL,   APP_MSG_NULL,
        APP_MSG_NULL,       APP_MSG_NULL,
    },
    [2] = {
        APP_MSG_MUSIC_NEXT, APP_MSG_VOL_UP,   APP_MSG_VOL_UP,   APP_MSG_NULL,
        APP_MSG_NULL,       APP_MSG_NULL,   APP_MSG_NULL,   APP_MSG_NULL,
        APP_MSG_NULL,       APP_MSG_NULL,
    },
    [3] = {
        APP_MSG_LOW_LANTECY,   APP_MSG_NULL,   APP_MSG_NULL,   APP_MSG_NULL,
        APP_MSG_NULL,       APP_MSG_NULL,   APP_MSG_NULL,   APP_MSG_NULL,
        APP_MSG_NULL,       APP_MSG_NULL,
    },
    [4] = {
        APP_MSG_ANC_SWITCH, APP_MSG_NULL,   APP_MSG_NULL,   APP_MSG_NULL,
        APP_MSG_NULL,       APP_MSG_NULL,   APP_MSG_NULL,   APP_MSG_NULL,
        APP_MSG_NULL,       APP_MSG_NULL,
    },
    [5] = {
        APP_MSG_NULL,       APP_MSG_NULL,   APP_MSG_NULL,   APP_MSG_NULL,
        APP_MSG_NULL,       APP_MSG_NULL,   APP_MSG_NULL,   APP_MSG_NULL,
        APP_MSG_NULL,       APP_MSG_NULL,
    },
    [6] = {
        APP_MSG_NULL,       APP_MSG_NULL,   APP_MSG_NULL,   APP_MSG_NULL,
        APP_MSG_NULL,       APP_MSG_NULL,   APP_MSG_NULL,   APP_MSG_NULL,
        APP_MSG_NULL,       APP_MSG_NULL,
    },
    [7] = {
        APP_MSG_NULL,       APP_MSG_NULL,   APP_MSG_NULL,   APP_MSG_NULL,
        APP_MSG_NULL,       APP_MSG_NULL,   APP_MSG_NULL,   APP_MSG_NULL,
        APP_MSG_NULL,       APP_MSG_NULL,
    },
    [8] = {
        APP_MSG_NULL,       APP_MSG_NULL,   APP_MSG_NULL,   APP_MSG_NULL,
        APP_MSG_NULL,       APP_MSG_NULL,   APP_MSG_NULL,   APP_MSG_NULL,
        APP_MSG_NULL,       APP_MSG_NULL,
    },
    [9] = {
        APP_MSG_NULL,       APP_MSG_NULL,   APP_MSG_NULL,   APP_MSG_NULL,
        APP_MSG_NULL,       APP_MSG_NULL,   APP_MSG_NULL,   APP_MSG_NULL,
        APP_MSG_NULL,       APP_MSG_NULL,
    },
};
#endif



int bt_get_phone_state(void *device)
{
    int state = btstack_bt_get_call_status(device);
    if (state == BT_SIRI_STATE &&
        btstack_get_call_esco_status(device) != BT_ESCO_STATUS_OPEN) {
        state = BT_CALL_HANGUP;
    }
    return state;
}

int bt_key_power_msg_remap(int *msg)
{
    int app_msg = APP_MSG_NULL;
    u8 key_action = APP_MSG_KEY_ACTION(msg[0]);

    if (tws_api_get_role() == TWS_ROLE_SLAVE) {
        return APP_MSG_NULL;
    }

    void *devices[2];
    void *active_device = NULL;
    void *incoming_device = NULL;
    void *outgoing_device = NULL;
    void *siri_device = NULL;

    int num = btstack_get_conn_devices(devices, 2);
    for (int i = 0; i < num; i++) {
        int state = bt_get_phone_state(devices[i]);
        if (state == BT_CALL_ACTIVE) {
            active_device = devices[i];
        } else if (state == BT_CALL_INCOMING) {
            incoming_device = devices[i];
        } else if (state == BT_CALL_OUTGOING || state == BT_CALL_ALERT) {
            outgoing_device = devices[i];
        } else if (state == BT_SIRI_STATE) {
            siri_device = devices[i];
        }
    }

    if (active_device) {
        switch (key_action) {
        case KEY_ACTION_CLICK:
#if TCFG_IOKEY_ENABLE || TCFG_ADKEY_ENABLE
            app_msg = APP_MSG_CALL_HANGUP;
#endif
            break;
        case KEY_ACTION_DOUBLE_CLICK:
#if TCFG_LP_TOUCH_KEY_ENABLE
            app_msg = APP_MSG_CALL_HANGUP;
#endif
            break;
        default:
            break;
        }
    } else if (incoming_device) {
        switch (key_action) {
        case KEY_ACTION_CLICK:
#if TCFG_IOKEY_ENABLE || TCFG_ADKEY_ENABLE
            app_msg = APP_MSG_CALL_ANSWER;
#endif
            break;
        case KEY_ACTION_DOUBLE_CLICK:
#if TCFG_LP_TOUCH_KEY_ENABLE
            app_msg = APP_MSG_CALL_ANSWER;
#endif
            break;
        case KEY_ACTION_HOLD_1SEC:
            app_msg = APP_MSG_CALL_HANGUP;
            break;
        default:
            break;
        }
    } else if (outgoing_device) {
        switch (key_action) {
        case KEY_ACTION_CLICK:
#if TCFG_IOKEY_ENABLE || TCFG_ADKEY_ENABLE
            app_msg = APP_MSG_CALL_HANGUP;
#endif
            break;
        case KEY_ACTION_HOLD_1SEC:
#if TCFG_LP_TOUCH_KEY_ENABLE
            app_msg = APP_MSG_CALL_HANGUP;
#endif
            break;
        default:
            break;
        }
    } else if (siri_device) {
        switch (key_action) {
        case KEY_ACTION_CLICK:
            app_msg = APP_MSG_CLOSE_SIRI;
            break;
        default:
            break;
        }

    } else {
        /* 非通话相关状态 */
        int tws_state = tws_api_get_tws_state();
#if ((TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_UNICAST_SINK_EN | LE_AUDIO_JL_UNICAST_SINK_EN)))
        int tws_cig_state = is_cig_phone_conn();
        if (tws_state & TWS_STA_PHONE_CONNECTED || tws_cig_state) { //已连接手机经典蓝牙或者cig
#else
        if (tws_state & TWS_STA_PHONE_CONNECTED) { //已连接手机经典蓝牙
#endif
            char channel = tws_api_get_local_channel();
            switch (key_action) {
            case KEY_ACTION_CLICK:
#if TCFG_IOKEY_ENABLE || TCFG_ADKEY_ENABLE
                app_msg = APP_MSG_MUSIC_PP;
#endif
                break;
            case KEY_ACTION_DOUBLE_CLICK:
#if TCFG_LP_TOUCH_KEY_ENABLE
                app_msg = APP_MSG_MUSIC_PP;
#else
                // TWS连上情况下, 左耳双击上一曲, 右耳双击下一曲
                if (tws_state & TWS_STA_SIBLING_CONNECTED) {
                    if ((channel == 'L' && msg[1] != APP_KEY_MSG_FROM_TWS) ||
                        (channel == 'R' && msg[1] == APP_KEY_MSG_FROM_TWS)) {
                        app_msg = APP_MSG_MUSIC_PREV;
                        break;
                    }
                }
                app_msg = APP_MSG_MUSIC_NEXT;
#endif
                break;
            case KEY_ACTION_LONG:
            case KEY_ACTION_HOLD:
#if TCFG_IOKEY_ENABLE
                // TWS连上情况下, 按住左耳上一曲
                if (tws_state & TWS_STA_SIBLING_CONNECTED) {
                    if ((channel == 'L' && msg[1] != APP_KEY_MSG_FROM_TWS) ||
                        (channel == 'R' && msg[1] == APP_KEY_MSG_FROM_TWS)) {
                        app_msg = APP_MSG_VOL_DOWN;
                        break;
                    }
                }
                app_msg = APP_MSG_VOL_UP;
#endif
                break;
            case KEY_ACTION_HOLD_1SEC:
#if TCFG_LP_TOUCH_KEY_ENABLE
                // TWS连上情况下, 按住左耳上一曲
                if (tws_state & TWS_STA_SIBLING_CONNECTED) {
                    if ((channel == 'L' && msg[1] != APP_KEY_MSG_FROM_TWS) ||
                        (channel == 'R' && msg[1] == APP_KEY_MSG_FROM_TWS)) {
                        app_msg = APP_MSG_MUSIC_PREV;
                        break;
                    }
                }
                app_msg = APP_MSG_MUSIC_NEXT;
#endif
                break;
            case KEY_ACTION_TRIPLE_CLICK:
                if (tws_state & TWS_STA_SIBLING_CONNECTED) {
                    if ((channel == 'L' && msg[1] != APP_KEY_MSG_FROM_TWS) ||
                        (channel == 'R' && msg[1] == APP_KEY_MSG_FROM_TWS)) {
                        app_msg = APP_MSG_LOW_LANTECY;
                        break;
                    }
                }
                app_msg = APP_MSG_OPEN_SIRI;
                break;
            default:
                break;
            }
        } else {
            switch (key_action) {
            case KEY_ACTION_DOUBLE_CLICK:
#if CONFIG_TWS_PAIR_MODE == CONFIG_TWS_PAIR_BY_CLICK
                if (tws_state & TWS_STA_TWS_UNPAIRED) { // TWS未配对
                    app_msg = APP_MSG_TWS_START_PAIR;
                }
#endif
                break;
            default:
                break;
            }
        }
    }

    switch (key_action) {
    case KEY_ACTION_TWS_HOLD_5SEC:
        //app_msg = APP_MSG_TWS_POWER_OFF;
        break;
    default:
        break;
    }
    printf("bt_key_msg_remap, key_action: %d, app_msg: %d\n", key_action, app_msg);
    return app_msg;
}

int bt_key_next_msg_remap(int *msg)
{
    int app_msg = APP_MSG_NULL;
    u8 key_action  = APP_MSG_KEY_ACTION(msg[0]);

    if (tws_api_get_role() == TWS_ROLE_SLAVE) {
        return APP_MSG_NULL;
    }
    if (tws_api_get_tws_state() & TWS_STA_PHONE_DISCONNECTED) {
        return APP_MSG_NULL;
    }

    int state = bt_get_call_status();

    switch (key_action) {
    case KEY_ACTION_CLICK:
        if (state == BT_CALL_HANGUP) {
            app_msg = APP_MSG_MUSIC_NEXT;
        }
        break;
    case KEY_ACTION_LONG:
    case KEY_ACTION_HOLD:
        app_msg = APP_MSG_VOL_UP;
        break;
    }

    return app_msg;
}

int bt_key_prev_msg_remap(int *msg)
{
    int app_msg = APP_MSG_NULL;
    u8 key_action  = APP_MSG_KEY_ACTION(msg[0]);

    if (tws_api_get_role() == TWS_ROLE_SLAVE) {
        return APP_MSG_NULL;
    }
    if (tws_api_get_tws_state() & TWS_STA_PHONE_DISCONNECTED) {
        return APP_MSG_NULL;
    }

    int state = bt_get_call_status();

    switch (key_action) {
    case KEY_ACTION_CLICK:
        if (state == BT_CALL_HANGUP) {
            app_msg = APP_MSG_MUSIC_PREV;
        }
        break;
    case KEY_ACTION_LONG:
    case KEY_ACTION_HOLD:
        app_msg = APP_MSG_VOL_DOWN;
        break;
    }

    return app_msg;
}

int bt_key_mode_msg_remap(int *msg)
{
    int app_msg = APP_MSG_NULL;
    u8 key_action  = APP_MSG_KEY_ACTION(msg[0]);

    switch (key_action) {
    case KEY_ACTION_CLICK:
        app_msg = APP_MSG_GOTO_NEXT_MODE;
        break;
    }

    return app_msg;
}

int bt_key_slider_msg_remap(int *msg)
{
    int app_msg = APP_MSG_NULL;
    u8 key_action  = APP_MSG_KEY_ACTION(msg[0]);

    switch (key_action) {
    case KEY_SLIDER_UP:
        app_msg = APP_MSG_VOL_UP;
        break;
    case KEY_SLIDER_DOWN:
        app_msg = APP_MSG_VOL_DOWN;
        break;
    }
    return app_msg;
}




const struct key_remap_table bt_mode_key_table[] = {
#if TCFG_IOKEY_ENABLE
    { .key_value = KEY_POWER,   .remap_func = bt_key_power_msg_remap },
    //{ .key_value = KEY_NEXT,    .remap_func = bt_key_next_msg_remap },
    //{ .key_value = KEY_PREV,    .remap_func = bt_key_prev_msg_remap },
#endif
#if TCFG_LP_TOUCH_KEY_ENABLE
    { .key_value = KEY_POWER,   .remap_func = bt_key_power_msg_remap },
    { .key_value = KEY_SLIDER,  .remap_func = bt_key_slider_msg_remap },
#endif

#if TCFG_ADKEY_ENABLE
    { .key_value = KEY_AD_NUM0, .remap_table = adkey_msg_table[0] },
    { .key_value = KEY_AD_NUM1, .remap_table = adkey_msg_table[1] },
    { .key_value = KEY_AD_NUM2, .remap_table = adkey_msg_table[2] },
    { .key_value = KEY_AD_NUM3, .remap_table = adkey_msg_table[3] },
    { .key_value = KEY_AD_NUM4, .remap_table = adkey_msg_table[4] },
    { .key_value = KEY_AD_NUM5, .remap_table = adkey_msg_table[5] },
    { .key_value = KEY_AD_NUM6, .remap_table = adkey_msg_table[6] },
    { .key_value = KEY_AD_NUM7, .remap_table = adkey_msg_table[7] },
    { .key_value = KEY_AD_NUM8, .remap_table = adkey_msg_table[8] },
    { .key_value = KEY_AD_NUM9, .remap_table = adkey_msg_table[9] },
#endif

    { .key_value = 0xff }
};

