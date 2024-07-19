#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".led_config.data.bss")
#pragma data_seg(".led_config.data")
#pragma const_seg(".led_config.text.const")
#pragma code_seg(".led_config.text")
#endif
#include "app_config.h"
#include "system/init.h"
#include "led/led_ui_manager.h"

#if 0//TCFG_PWMLED_ENABLE

int board_led_config()
{
    led_ui_manager_init();

    return 0;
}
platform_initcall(board_led_config);

#endif /* #if TCFG_PWMLED_ENABLE */
