#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".key_wakeup.data.bss")
#pragma data_seg(".key_wakeup.data")
#pragma const_seg(".key_wakeup.text.const")
#pragma code_seg(".key_wakeup.text")
#endif
#include "asm/power_interface.h"
#include "app_config.h"
#include "gpio.h"

void key_active_set(u8 port);

static void key_wakeup_callback(P33_IO_WKUP_EDGE edge)
{
    key_active_set(0);
}

static struct _p33_io_wakeup_config port0 = {
    .pullup_down_mode = PORT_INPUT_PULLUP_10K,
    .filter      		= PORT_FLT_DISABLE,
    .edge               = FALLING_EDGE,
    .gpio               = IO_PORTB_01,
    .callback			= key_wakeup_callback,
};

void key_wakeup_init()
{
#if (!TCFG_LP_TOUCH_KEY_ENABLE)
    p33_io_wakeup_port_init(&port0);
    p33_io_wakeup_enable(IO_PORTB_01, 1);

#endif
}
