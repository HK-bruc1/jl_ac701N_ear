#ifndef __LED_UI_CFG__
#define __LED_UI_CFG__

#include "generic/typedef.h"

enum led_next_action_table {
    ACTION_WAIT = 0,
    ACTION_CONTINUE,
    ACTION_END,
    ACTION_LOOP,
};

enum led_logic_table {
    PWM_LOW_LOGIC = 0, //pwm低电平亮灯
    PWM_HIGH_LOGIC,
    IO_LOW_LOGIC,
    IO_HIGH_LOGIC,
};

struct led_hw_config {
    u8 led_name;
    u8 gpio;
    u8 led_logic; //enum led_logic_table
    u8 bright;

    u8 len;
    u16 led_uuid;
    u16 io_uuid;
    //u8 led_logic; //enum led_logic_table
} _GNU_PACKED_;

enum {
    LED_TURN_ON = 1,
    LED_TURN_OFF,
};

struct led_action {
    u16 led_uuid;
    u8 on_off; 			//1: 亮, 2: 灭
    u16 time; 			//0 ~ 60000
    u8 bright; 			//0 ~ 100
    u8 breath_rate; 	//0 ~ 20
    u8 next; 			//enum led_next_action_table
} _GNU_PACKED_;

struct led_state_config {
    u8 len;
    u16 uuid;
    struct led_action action[0];
} _GNU_PACKED_;

void pwm_led_ui_action_pre();

void led_ui_pwm_pre_set(struct led_action *action);
#endif /* #ifndef __LED_UI_CFG__ */
