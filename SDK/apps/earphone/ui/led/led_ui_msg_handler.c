#include "app_msg.h"
#include "app_main.h"
#include "bt_tws.h"
#include "asm/charge.h"
#include "battery_manager.h"
#include "led_ui_tws_sync.h"
#include "led_ui_api.h"


#define LOG_TAG             "[LED_UI]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"


#if (TCFG_PWMLED_ENABLE)

#define LED_RED_BRIGHTNESS      100
#define LED_BLUE_BRIGHTNESS     100

#define LED_BREATHING_SPEED     100

enum {
    STA_FLAG_POWERON = 1,
    STA_FLAG_POWEROFF,
    STA_FLAG_PHONE_INCOMING,
};

#if TCFG_LED_RED_ENABLE
// 红灯常亮
const struct led_state_item red_led_on[] = {
#if TCFG_LED_BLUE_ENABLE
    { LED_BLUE, LED_OFF,  0,    LED_BLUE_BRIGHTNESS, 0,  LED_ACTION_CONTINUE },
#endif
    { LED_RED,  LED_ON,   0,    LED_RED_BRIGHTNESS,  0,  LED_ACTION_END      },
};

const struct led_state_item red_led_off[] = {
    { LED_RED,  LED_OFF,  0,    0,                      0,  LED_ACTION_END },
};

// 红灯亮1s
const struct led_state_item red_led_on_1s[] = {
#if TCFG_LED_BLUE_ENABLE
    { LED_BLUE, LED_OFF,  0,    LED_BLUE_BRIGHTNESS, 0,  LED_ACTION_CONTINUE },
#endif
    { LED_RED,  LED_ON,   1000,    LED_RED_BRIGHTNESS,  0,  LED_ACTION_END      },
};

// 红灯亮2s
const struct led_state_item red_led_on_2s[] = {
#if TCFG_LED_BLUE_ENABLE
    { LED_BLUE, LED_OFF,  0,    LED_BLUE_BRIGHTNESS, 0,  LED_ACTION_CONTINUE },
#endif
    { LED_RED,  LED_ON,   2000,    LED_RED_BRIGHTNESS,  0,  LED_ACTION_END      },
};

// 红灯亮3s
const struct led_state_item red_led_on_3s[] = {
#if TCFG_LED_BLUE_ENABLE
    { LED_BLUE, LED_OFF,  0,    LED_BLUE_BRIGHTNESS, 0,  LED_ACTION_CONTINUE },
#endif
    { LED_RED,  LED_ON,   3000,    LED_RED_BRIGHTNESS,  0,  LED_ACTION_END      },
};

// 红灯闪1下
const struct led_state_item red_led_flashes_1_times[] = {
#if TCFG_LED_BLUE_ENABLE
    { LED_BLUE, LED_OFF,  0,    0,                      0,  LED_ACTION_CONTINUE    },
#endif
    { LED_RED,  LED_ON,   300,  LED_RED_BRIGHTNESS,     0,  LED_ACTION_WAIT        },
    { LED_RED,  LED_OFF,  0,    0,                      0,  LED_ACTION_END         },
};

// 红灯闪2下
const struct led_state_item red_led_flashes_2_times[] = {
#if TCFG_LED_BLUE_ENABLE
    { LED_BLUE, LED_OFF,  0,    0,                      0,  LED_ACTION_CONTINUE    },
#endif
    { LED_RED,  LED_ON,   300,  LED_RED_BRIGHTNESS,     0,  LED_ACTION_WAIT        },
    { LED_RED,  LED_OFF,  300,  0,                      0,  LED_ACTION_WAIT        },
    { LED_RED,  LED_ON,   300,  LED_RED_BRIGHTNESS,     0,  LED_ACTION_WAIT        },
    { LED_RED,  LED_OFF,  0,    0,                      0,  LED_ACTION_END         },
};

// 红灯闪3下
const struct led_state_item red_led_flashes_3_times[] = {
#if TCFG_LED_BLUE_ENABLE
    { LED_BLUE, LED_OFF,  0,    0,                      0,  LED_ACTION_CONTINUE    },
#endif
    { LED_RED,  LED_ON,   300,  LED_RED_BRIGHTNESS,     0,  LED_ACTION_WAIT        },
    { LED_RED,  LED_OFF,  300,  0,                      0,  LED_ACTION_WAIT        },
    { LED_RED,  LED_ON,   300,  LED_RED_BRIGHTNESS,     0,  LED_ACTION_WAIT        },
    { LED_RED,  LED_OFF,  300,  0,                      0,  LED_ACTION_WAIT        },
    { LED_RED,  LED_ON,   300,  LED_RED_BRIGHTNESS,     0,  LED_ACTION_WAIT        },
    { LED_RED,  LED_OFF,  0,    0,                      0,  LED_ACTION_END         },
};
#endif //TCFG_LED_RED_ENABLE



#if TCFG_LED_BLUE_ENABLE
// 蓝灯常亮
const struct led_state_item blue_led_on[] = {
#if TCFG_LED_RED_ENABLE
    { LED_RED,  LED_OFF,  0,    0,                      0,  LED_ACTION_CONTINUE    },
#endif
    { LED_BLUE, LED_ON,   0,    LED_BLUE_BRIGHTNESS,     0,  LED_ACTION_END         },
};
// 蓝灯灭
const struct led_state_item blue_led_off[] = {
    { LED_BLUE,  LED_OFF,  0,    0,                      0,  LED_ACTION_END },
};

// 蓝灯亮1s
const struct led_state_item blue_led_on_1s[] = {
#if TCFG_LED_RED_ENABLE
    { LED_RED, LED_OFF,  0,    0, 0,  LED_ACTION_CONTINUE },
#endif
    { LED_BLUE,  LED_ON,   1000,    LED_BLUE_BRIGHTNESS,  0,  LED_ACTION_END      },
};

// 蓝灯亮2s
const struct led_state_item blue_led_on_2s[] = {
#if TCFG_LED_RED_ENABLE
    { LED_RED, LED_OFF,  0,    0, 0,  LED_ACTION_CONTINUE },
#endif
    { LED_BLUE,  LED_ON,   2000,    LED_BLUE_BRIGHTNESS,  0,  LED_ACTION_END      },
};

// 蓝灯亮3s
const struct led_state_item blue_led_on_3s[] = {
#if TCFG_LED_RED_ENABLE
    { LED_RED, LED_OFF,  0,    0, 0,  LED_ACTION_CONTINUE },
#endif
    { LED_BLUE,  LED_ON,   3000,    LED_BLUE_BRIGHTNESS,  0,  LED_ACTION_END      },
};

// 蓝灯闪1下
const struct led_state_item blue_led_flashes_1_times[] = {
#if TCFG_LED_RED_ENABLE
    { LED_RED, LED_OFF,  0,    0,                      0,  LED_ACTION_CONTINUE    },
#endif
    { LED_BLUE,  LED_ON,   300,  LED_BLUE_BRIGHTNESS,     0,  LED_ACTION_WAIT        },
    { LED_BLUE,  LED_OFF,  0,    0,                      0,  LED_ACTION_END         },
};

// 蓝灯闪2下
const struct led_state_item blue_led_flashes_2_times[] = {
#if TCFG_LED_RED_ENABLE
    { LED_RED, LED_OFF,  0,    0,                      0,  LED_ACTION_CONTINUE    },
#endif
    { LED_BLUE,  LED_ON,   300,  LED_BLUE_BRIGHTNESS,     0,  LED_ACTION_WAIT        },
    { LED_BLUE,  LED_OFF,  300,  0,                      0,  LED_ACTION_WAIT        },
    { LED_BLUE,  LED_ON,   300,  LED_BLUE_BRIGHTNESS,     0,  LED_ACTION_WAIT        },
    { LED_BLUE,  LED_OFF,  0,    0,                      0,  LED_ACTION_END         },
};

// 蓝灯闪3下
const struct led_state_item blue_led_flashes_3_times[] = {
#if TCFG_LED_RED_ENABLE
    { LED_RED, LED_OFF,  0,    0,                      0,  LED_ACTION_CONTINUE    },
#endif
    { LED_BLUE,  LED_ON,   300,  LED_BLUE_BRIGHTNESS,     0,  LED_ACTION_WAIT        },
    { LED_BLUE,  LED_OFF,  300,  0,                      0,  LED_ACTION_WAIT        },
    { LED_BLUE,  LED_ON,   300,  LED_BLUE_BRIGHTNESS,     0,  LED_ACTION_WAIT        },
    { LED_BLUE,  LED_OFF,  300,  0,                      0,  LED_ACTION_WAIT        },
    { LED_BLUE,  LED_ON,   300,  LED_BLUE_BRIGHTNESS,     0,  LED_ACTION_WAIT        },
    { LED_BLUE,  LED_OFF,  0,    0,                      0,  LED_ACTION_END         },
};

#endif // TCFG_LED_BLUE_ENABLE


// 红蓝灯全亮
const struct led_state_item led_all_on[] = {
    { LED_RED,  LED_ON,  0,    LED_RED_BRIGHTNESS,        0,  LED_ACTION_CONTINUE    },
    { LED_BLUE, LED_ON,  0,    LED_BLUE_BRIGHTNESS,       0,  LED_ACTION_END         },
};

// 红蓝灯全灭
const struct led_state_item led_all_off[] = {
    { LED_RED,  LED_OFF,  0,    0,                      0,  LED_ACTION_CONTINUE    },
    { LED_BLUE, LED_OFF,  0,    0,                      0,  LED_ACTION_END         },
};


/*
 *--------硬件支持的周期闪灯时间配置--------
*/
const struct led_state_time time_single_slow_flash = {
    .mode = PERIOD_SINGLE_FLASH,
    .red_bright = LED_RED_BRIGHTNESS * 5,
    .blue_bright = LED_BLUE_BRIGHTNESS * 5,
    .period = 2000,
    .light_time = 1000,
};

const struct led_state_time time_single_fast_flash = {
    .mode = PERIOD_SINGLE_FLASH,
    .red_bright = LED_RED_BRIGHTNESS * 5,
    .blue_bright = LED_BLUE_BRIGHTNESS * 5,
    .period = 600,
    .light_time = 300,
};

// 红蓝灯交替慢闪
const struct led_state_time time_dual_slow_flash = {
    .mode = PERIOD_DUAL_FLASH,
    .red_bright = LED_RED_BRIGHTNESS * 5,
    .blue_bright = LED_BLUE_BRIGHTNESS * 5,
    .period = 2000,
    .light_time = 100,
};

// 周期闪2次
const struct led_state_time time_single_double_flash = {
    .mode = PERIOD_SINGLE_FLASH,
    .red_bright = LED_RED_BRIGHTNESS * 5,
    .blue_bright = LED_BLUE_BRIGHTNESS * 5,
    .period = 2000,
    .light_time = 1000,
};

const struct led_state_map g_led_state_table[] = {
#if TCFG_LED_RED_ENABLE
    { LED_STA_RED_ON, LED_STATE_TABLE(red_led_on) },
    { LED_STA_RED_OFF, LED_STATE_TABLE(red_led_off) },
    { LED_STA_RED_FLASH_3TIMES, LED_STATE_TABLE(red_led_flashes_3_times) },
    { LED_STA_RED_FAST_FLASH, LED_RED, &time_single_fast_flash },
    { LED_STA_RED_SLOW_FLASH, LED_RED, &time_single_slow_flash },

#endif

#if TCFG_LED_BLUE_ENABLE
    { LED_STA_BLUE_ON_3S, LED_STATE_TABLE(blue_led_on_3s) },
    { LED_STA_BLUE_SLOW_FLASH, LED_BLUE, &time_single_slow_flash },
    { LED_STA_BLUE_FAST_FLASH, LED_BLUE, &time_single_fast_flash },
#endif

    {0, 0, 0} // END
};

void led_state_tws_sync_handler(int arg, int err)
{
    u8 flag = arg;

    switch (flag) {
    case STA_FLAG_PHONE_INCOMING:
        led_ui_set_state(LED_STA_RED_FAST_FLASH, DISP_RECOVERABLE, 0);
        break;
    }
}

TWS_SYNC_CALL_REGISTER(tws_led_ui_sync) = {
    .uuid = 0x5AEC3215,
    .task_name = "app_core",
    .func = led_state_tws_sync_handler,
};

void led_ui_tws_state_sync(u8 flag, int msec)
{
    if ((tws_api_get_role() == TWS_ROLE_MASTER)) {
        int err = tws_api_sync_call_by_uuid(0x5AEC3215, flag, msec);
        if (err < 0) {
            led_state_tws_sync_handler(flag, err);
        }
    }
}

static int ui_battery_msg_handler(int *msg)
{
    switch (msg[0]) {
    case BAT_MSG_CHARGE_START:
        led_ui_set_state(LED_STA_RED_ON, DISP_RECOVERABLE, 0);
        break;
    case BAT_MSG_CHARGE_FULL:
        led_ui_set_state(LED_STA_RED_ON, 0, 0);
        break;
    case BAT_MSG_CHARGE_CLOSE:
        led_ui_set_state(LED_STA_ALL_OFF, 0, 0);
        break;
    case BAT_MSG_CHARGE_ERR:
        break;
    case BAT_MSG_CHARGE_LDO5V_OFF:
        break;
    case POWER_EVENT_POWER_NORMAL:
        break;
    case POWER_EVENT_POWER_WARNING:
        led_ui_set_state(LED_STA_RED_FLASH_1TIMES_PER_5S, DISP_RECOVERABLE, 0);
        break;
    case POWER_EVENT_POWER_LOW:
        break;
    case POWER_EVENT_POWER_CHANGE:
        break;
    case POWER_EVENT_SYNC_TWS_VBAT_LEVEL:
        break;
    case POWER_EVENT_POWER_CHARGE:
        break;
    }
    return 0;
}

static int ui_app_msg_handler(int *msg)
{
    switch (msg[0]) {
    case APP_MSG_POWER_ON:
        r_printf("LED_STA_POWERON\n");
        led_ui_set_state(LED_STA_BLUE_ON_3S, DISP_NON_INTR, STA_FLAG_POWERON);
        break;
    case APP_MSG_POWER_OFF:
        led_ui_set_state(LED_STA_RED_FLASH_3TIMES, DISP_NON_INTR, STA_FLAG_POWEROFF);
        break;
    }
    return 0;
}

#if TCFG_USER_TWS_ENABLE
static int ui_tws_msg_handler(int *msg)
{
    struct tws_event *evt = (struct tws_event *)msg;
    u8 role = evt->args[0];
    u8 state = evt->args[1];
    u8 *bt_addr = evt->args + 3;

    switch (evt->event) {
    case TWS_EVENT_CONNECTED:
        if (role == TWS_ROLE_MASTER) {
            led_tws_state_sync(led_ui_get_state_name(), 200, 1);
        }
        break;

    }
    return 0;
}
#endif


static int ui_bt_stack_msg_handler(int *msg)
{
    struct bt_event *bt = (struct bt_event *)msg;

    int tws_role = tws_api_get_role_async();
    if (tws_role == TWS_ROLE_SLAVE) {
        return 0;
    }

    switch (bt->event) {
    case BT_STATUS_INIT_OK:
        //led_ui_set_state(led_sta_bt_init_ok, 1);
        break;
    case BT_STATUS_FIRST_CONNECTED:
        led_ui_set_state(LED_STA_ALL_OFF, DISP_RECOVERABLE, 0);
        break;
    case BT_STATUS_SECOND_CONNECTED:
        break;
    case BT_STATUS_FIRST_DISCONNECT:
    case BT_STATUS_SECOND_DISCONNECT:
        //led_ui_set_state(led_sta_bt_disconnect, 1);
        break;
    case BT_STATUS_PHONE_INCOME:
        break;
    }
    return 0;
}

void led_interrupt_test1(void *p)
{
    led_ui_set_state(LED_STA_RED_FLASH_3TIMES,
                     DISP_INTR_CURRENT | DISP_RECOVER_CURRENT, 0);
}

void ui_pwm_led_msg_handler(int *msg)
{
    switch (msg[0]) {
    case LED_MSG_STATE_END:
        if (msg[1] == STA_FLAG_POWERON) {
            if (bt_get_total_connect_dev() == 0) {
                led_ui_set_state(LED_STA_BLUE_SLOW_FLASH, DISP_RECOVERABLE, 0);
            } else {
                led_ui_set_state(LED_STA_ALL_OFF, DISP_RECOVERABLE, 0);
            }
            break;
        }
        led_ui_state_machine_run();
        break;
    }

}


static int ui_led_msg_handler(int *msg)
{
    switch (msg[0]) {
#if TCFG_USER_TWS_ENABLE
    case MSG_FROM_TWS:
        ui_tws_msg_handler(msg + 1);
        break;
#endif
    case MSG_FROM_BT_STACK:
        ui_bt_stack_msg_handler(msg + 1);
        break;
    case MSG_FROM_BT_HCI:
//       ui_bt_hci_msg_handler(msg + 1);
        break;
    case MSG_FROM_APP:
        ui_app_msg_handler(msg + 1);
        break;
    case MSG_FROM_KEY:
        break;
    case MSG_FROM_BATTERY:
        ui_battery_msg_handler(msg + 1);
        break;
    case MSG_FROM_PWM_LED:
        ui_pwm_led_msg_handler(msg + 1);
        break;
    default:
        break;
    }
    return 0;
}

APP_MSG_HANDLER(led_msg_entry) = {
    .owner      = 0xff,
    .from       = 0xff,
    .handler    = ui_led_msg_handler,
};


#endif

