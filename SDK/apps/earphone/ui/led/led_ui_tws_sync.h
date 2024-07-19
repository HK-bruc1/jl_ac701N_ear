#ifndef __LED_UI_TWS_SYNC_H__
#define __LED_UI_TWS_SYNC_H__


bool led_tws_sync_check(void);

void led_tws_state_sync(int led_state, int msec, u8 force_sync);

void led_tws_state_sync_by_name(char *name, int msec, u8 force_sync);

void pwm_led_state_sync_del(void);
#endif /* #ifndef __LED_UI_TWS_SYNC_H__ */
