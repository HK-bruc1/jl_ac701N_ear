#ifndef __PWM_LED_API_H__
#define __PWM_LED_API_H__


#include "typedef.h"

#include "cpu/pwm_led_debug.h"
#include "pwm_led/led_ui_api.h"

void pwm_led_status_display(const struct led_state_item *action);


void pwm_led_cbfunc(u32 cnt);
#endif


