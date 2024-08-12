#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".led_config.data.bss")
#pragma data_seg(".led_config.data")
#pragma const_seg(".led_config.text.const")
#pragma code_seg(".led_config.text")
#endif
#include "app_config.h"
#include "system/init.h"
#include "pwm_led/led_ui_api.h"



const led_board_cfg_t board_cfg_hw_data;

const struct led_state_map g_led_state_table[] = {};

#if 0//TCFG_PWMLED_ENABLE

int board_led_config()
{
    led_ui_manager_init();

    return 0;
}
platform_initcall(board_led_config);

#endif /* #if TCFG_PWMLED_ENABLE */
