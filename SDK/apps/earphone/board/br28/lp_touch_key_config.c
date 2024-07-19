#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".lp_touch_key_config.data.bss")
#pragma data_seg(".lp_touch_key_config.data")
#pragma const_seg(".lp_touch_key_config.text.const")
#pragma code_seg(".lp_touch_key_config.text")
#endif
#include "app_config.h"
#include "syscfg_id.h"
#include "key_driver.h"
#include "cpu/includes.h"
#include "system/init.h"
#include "asm/lp_touch_key_api.h"
#include "iokey_config.h"

#if TCFG_LP_TOUCH_KEY_ENABLE

static u8 lp_touch_port_table[5] = {
    IO_PORTB_00,
    IO_PORTB_01,
    IO_PORTB_02,
    IO_PORTB_04,
    IO_PORTB_05,
};

static struct lp_touch_key_platform_data lp_touch_key_config;

void lp_touch_key_cfg_dump(void)
{
    printf("lp_touch_key_config.slide_mode_en           = %d", lp_touch_key_config.slide_mode_en);
    printf("lp_touch_key_config.slide_mode_key_value    = %d", lp_touch_key_config.slide_mode_key_value);
    printf("lp_touch_key_config.hv_level                = %d", lp_touch_key_config.hv_level);
    printf("lp_touch_key_config.lv_level                = %d", lp_touch_key_config.lv_level);
    printf("lp_touch_key_config.cur_level               = %d", lp_touch_key_config.cur_level);
    printf("lp_touch_key_config.charge_mode_keep_touch  = %d", lp_touch_key_config.charge_mode_keep_touch);
    printf("lp_touch_key_config.long_press_reset_time   = %d", lp_touch_key_config.long_press_reset_time);
    printf("lp_touch_key_config.softoff_long_time       = %d", lp_touch_key_config.softoff_long_time);
    for (u8 ch = 0; ch < 5; ch ++) {
        if (lp_touch_key_config.ch[ch].enable == 0) {
            continue;
        }
        printf("lp_touch_key_config.ch[%d].enable           = %d", ch, lp_touch_key_config.ch[ch].enable);
        printf("lp_touch_key_config.ch[%d].port             = %d", ch, lp_touch_key_config.ch[ch].port);
        printf("lp_touch_key_config.ch[%d].key_value        = %d", ch, lp_touch_key_config.ch[ch].key_value);
        printf("lp_touch_key_config.ch[%d].wakeup_enable    = %d", ch, lp_touch_key_config.ch[ch].wakeup_enable);
        printf("lp_touch_key_config.ch[%d].algo_range_max   = %d", ch, lp_touch_key_config.ch[ch].algo_range_max);
        printf("lp_touch_key_config.ch[%d].algo_cfg0        = %d", ch, lp_touch_key_config.ch[ch].algo_cfg0);
        printf("lp_touch_key_config.ch[%d].algo_cfg1        = %d", ch, lp_touch_key_config.ch[ch].algo_cfg1);
        printf("lp_touch_key_config.ch[%d].algo_cfg2        = %d", ch, lp_touch_key_config.ch[ch].algo_cfg2);
        printf("lp_touch_key_config.ch[%d].range_sensity    = %d", ch, lp_touch_key_config.ch[ch].range_sensity);
    }
    printf("lp_touch_key_config.eartch_en               = %d", lp_touch_key_config.eartch_en);
    printf("lp_touch_key_config.eartch_ch               = %d", lp_touch_key_config.eartch_ch);
    printf("lp_touch_key_config.eartch_ref_ch           = %d", lp_touch_key_config.eartch_ref_ch);
    printf("lp_touch_key_config.eartch_soft_inear_val   = %d", lp_touch_key_config.eartch_soft_inear_val);
    printf("lp_touch_key_config.eartch_soft_outear_val  = %d", lp_touch_key_config.eartch_soft_outear_val);
}

int board_lp_touch_key_config()
{
    printf("read lp touch cfg info !\n\n");

    u8 cfg_buf[80];
    memset(cfg_buf, 0, sizeof(cfg_buf));
    int len = syscfg_read(CFG_LP_TOUCH_KEY_ID, cfg_buf, sizeof(cfg_buf));
    if (len < 0) {
        printf("read touch key cfg error !\n");
        return 1;
    }
    //put_buf(cfg_buf, sizeof(cfg_buf));
    memset((u8 *)&lp_touch_key_config, 0, sizeof(struct lp_touch_key_platform_data));
    lp_touch_key_config.slide_mode_en           = cfg_buf[1];
    lp_touch_key_config.slide_mode_key_value    = uuid2keyValue(cfg_buf[2] | (cfg_buf[3] << 8));
    lp_touch_key_config.hv_level                = cfg_buf[5];
    lp_touch_key_config.lv_level                = cfg_buf[6];
    lp_touch_key_config.cur_level               = cfg_buf[7];
    lp_touch_key_config.charge_mode_keep_touch  = cfg_buf[9];
    if (cfg_buf[10]) {
        lp_touch_key_config.long_press_reset_time = cfg_buf[11] | (cfg_buf[12] << 8);
    }
    lp_touch_key_config.softoff_long_time       = cfg_buf[13] | (cfg_buf[14] << 8);
    for (u8 i = 0; i < 5; i ++) {
        if (cfg_buf[15 + i * 14] != 0x0d) {
            break;
        }
        u8 ch = cfg_buf[16 + i * 14];
        lp_touch_key_config.ch[ch].enable           = 1;
        lp_touch_key_config.ch[ch].port             = lp_touch_port_table[ch];
        lp_touch_key_config.ch[ch].key_value        = uuid2keyValue(cfg_buf[17 + i * 14] | (cfg_buf[18 + i * 14] << 8));
        lp_touch_key_config.ch[ch].wakeup_enable    = cfg_buf[19 + i * 14];
        lp_touch_key_config.ch[ch].algo_range_max   = cfg_buf[20 + i * 14] | (cfg_buf[21 + i * 14] << 8);
        lp_touch_key_config.ch[ch].algo_cfg0        = cfg_buf[22 + i * 14] | (cfg_buf[23 + i * 14] << 8);
        lp_touch_key_config.ch[ch].algo_cfg1        = cfg_buf[24 + i * 14] | (cfg_buf[25 + i * 14] << 8);
        lp_touch_key_config.ch[ch].algo_cfg2        = cfg_buf[26 + i * 14] | (cfg_buf[27 + i * 14] << 8);
        lp_touch_key_config.ch[ch].range_sensity    = cfg_buf[28 + i * 14];
    }

#if TCFG_LP_EARTCH_KEY_ENABLE

    memset(cfg_buf, 0, sizeof(cfg_buf));
    len = syscfg_read(CFG_LP_TOUCH_KEY_EARTCH_ID, cfg_buf, sizeof(cfg_buf));
    if (len < 0) {
        printf("read touch key eartch cfg error !\n");
        return 1;
    }
    //put_buf(cfg_buf, sizeof(cfg_buf));
    lp_touch_key_config.eartch_en               = 1;
    lp_touch_key_config.eartch_ch               = cfg_buf[1];
    lp_touch_key_config.eartch_ref_ch           = cfg_buf[2];
    lp_touch_key_config.eartch_soft_inear_val   = cfg_buf[4] | (cfg_buf[5] << 8);
    lp_touch_key_config.eartch_soft_outear_val  = cfg_buf[6] | (cfg_buf[7] << 8);
#endif

    lp_touch_key_cfg_dump();

    lp_touch_key_init(&lp_touch_key_config);
    return 0;
}
platform_initcall(board_lp_touch_key_config);

#endif

