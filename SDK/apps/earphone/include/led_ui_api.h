#ifndef LED_UI_API_H
#define LED_UI_API_H

#include "generic/typedef.h"
#include "generic/list.h"
#include "asm/pwm_led_hw.h"

#define LED_RED     1
#define LED_BLUE    0
#define LED_ALL     2

#define LED_ON      1
#define LED_OFF     0

#define LED_STATE_TABLE(table)  ARRAY_SIZE(table), table


enum {
    LED_MSG_STATE_END = 1,
};

enum led_disp_mode : u8 {
    DISP_NON_INTR          = 0x01,
    DISP_INTR_CURRENT      = 0x02,
    DISP_RECOVERABLE       = 0x04,
    DISP_RECOVER_CURRENT   = 0x08,
};


enum {
    PERIOD_SINGLE_FLASH = 10, 	//周期单闪
    PERIOD_DUAL_FLASH, 		//周期双灯互闪
    PERIOD_DOUBLE_FLASH, 	//周期双闪
    PERIOD_SINGLE_BREATHE,  //周期单灯呼吸
    PERIOD_DUAL_BREATHE, 	//周期双灯呼吸
};

enum {
    LED_ACTION_WAIT,
    LED_ACTION_CONTINUE,
    LED_ACTION_LOOP,
    LED_ACTION_END,
};

struct led_state_time {
    u8 mode;
    u16 period;
    u16 red_bright;
    u16 blue_bright;
    s16 light_time;
    s16 gap_time;
    s16 first_light_time;
    s16 second_light_time;
    s16 red_light_delay_time;
    s16 blue_light_delay_time;
    s16 blink_delay_time;
};

struct led_state_item {
    u8 led_name;
    u8 on_off;
    u16 time_msec;
    u8 brightiness;
    u8 breathing_rate;
    u8 action;
};

enum led_state_name {
    LED_STA_NONE = 0,
    LED_STA_RED_ON,
    LED_STA_RED_OFF,
    LED_STA_RED_ON_1S,
    LED_STA_RED_ON_2S,
    LED_STA_RED_ON_3S,
    LED_STA_RED_BREATHING,
    LED_STA_RED_SLOW_FLASH,
    LED_STA_RED_FAST_FLASH,
    LED_STA_RED_FLASH_1TIMES,
    LED_STA_RED_FLASH_2TIMES,
    LED_STA_RED_FLASH_3TIMES,
    LED_STA_RED_FLASH_1TIMES_PER_5S,
    LED_STA_RED_FLASH_2TIMES_PER_5S,

    LED_STA_BLUE_ON,
    LED_STA_BLUE_OFF,
    LED_STA_BLUE_ON_1S,
    LED_STA_BLUE_ON_2S,
    LED_STA_BLUE_ON_3S,
    LED_STA_BLUE_BREATHING,
    LED_STA_BLUE_SLOW_FLASH,
    LED_STA_BLUE_FAST_FLASH,
    LED_STA_BLUE_FLASH_1TIMES,
    LED_STA_BLUE_FLASH_2TIMES,
    LED_STA_BLUE_FLASH_3TIMES,
    LED_STA_BLUE_FLASH_1TIMES_PER_5S,
    LED_STA_BLUE_FLASH_2TIMES_PER_5S,

    LED_STA_ALL_ON,
    LED_STA_ALL_OFF,
    LED_STA_ALTERNATING_SLOW_FLASH,
    LED_STA_ALTERNATING_FAST_FLASH,
};

struct led_state_map {
    enum led_state_name name;
    u8 arg0;
    const void *arg1;
};

struct led_state_obj {
    struct list_head entry;
    const struct led_state_time *time;
    const struct led_state_item *table;
    enum led_disp_mode disp_mode;
    enum led_state_name name;
    u16 timer;
    u8 table_size;
    u8 led_name;
    u8 callback_flag;
};


struct led_state_obj *led_ui_get_state();

void led_ui_add_state(struct led_state_obj *s);

void led_ui_state_machine_run();

void led_ui_set_state(enum led_state_name state_name,
                      enum led_disp_mode disp_mode, u8 callback_flag);

enum led_state_name led_ui_get_state_name();

#endif

