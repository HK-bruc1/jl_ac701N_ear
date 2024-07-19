#ifndef __LED_UI_MANAGER_H__
#define __LED_UI_MANAGER_H__

#include "led_ui_cfg.h"
#include "asm/pwm_led_hw.h"

extern u16 led_local_uuid;
extern u8 pwm_led_io_toggle;
extern struct led_mode led_mode_table[];

typedef enum {
    LED_UI_STATUS_IDLE = 0,
    LED_UI_STATUS_ACTIVE,
} LED_UI_STA;

int led_ui_manager_init(void);

void led_ui_manager_display(u8 local_action, u16 uuid, u8 mode);

void led_ui_manager_display_by_name(u8 local_action, char *name, u8 mode);

int led_ui_do_display(u16 uuid, u8 sync);

u16 led_ui_state_config_get(struct led_state_config *config, u16 offset);

u16 led_ui_action_config_get(struct led_action *config, u16 offset);

u8 led_ui_hw_info_get_by_uuid(u16 index, struct led_hw_config *hw_config);

u8 led_ui_manager_status_busy_query(void);

u32 led_ui_JBHash(u8 *data, int len);

/* --------------------------------------------------------------------------*/
/**
 * @brief  按照index获取对应可视化配置的led_io，需要在led_config_file_init之后执行
 *
 * @param  [in] index 想要获取的第几个led_io，从0开始,传参0代表获取第一个io的值
 *
 * @return
 */
/* --------------------------------------------------------------------------*/
u8 led_ui_hw_info_get_led_io(u8 index);

/* --------------------------------------------------------------------------*/
/**
 * @brief 关闭led_ui_manager
 */
/* --------------------------------------------------------------------------*/
void led_ui_manager_close(void);
#endif /* #ifndef __LED_UI_MANAGER_H__ */
