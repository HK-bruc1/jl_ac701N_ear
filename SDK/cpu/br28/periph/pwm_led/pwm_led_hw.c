#include "cpu/includes.h"
#include "asm/pwm_led_hw.h"
#include "gpio.h"
#include "gpadc.h"
#include "asm/power_interface.h"
#include "system/timer.h"
#include "app_config.h"


#if 1

#define LOG_TAG_CONST       PWM_LED
#define LOG_TAG             "[PWM_LED]"
/* #define LOG_ERROR_ENABLE */
/* #define LOG_DEBUG_ENABLE */
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"
#else

#define log_debug   printf

#endif

const u8 led_clk_div_table[8] =  {1, 4, 16, 64, 2, 8, 32, 128};

extern u32 __get_lrc_hz();


static void pwm_led_get_next_dir_level(u8 cur_dir, u8 cur_level, u8 *dir, u8 *level)
{
    u8 alternate_out = (JL_PLED->CON2 >> 4) & 0b1111;

    if (JL_PLED->CON0 & BIT(1)) {//呼吸变化模式
        *dir = !cur_dir;
        if (alternate_out) {
            if (cur_dir) {
                *level = cur_level;
            } else {
                *level = !cur_level;
            }
        } else {
            *level = cur_level;
        }
    } else {
        *dir = cur_dir;
        if (alternate_out) {
            *level = !cur_level;
        } else {
            *level = cur_level;
        }
    }
}

static u16 pwm_led_get_cur_status_cnt_max(u8 cur_dir, u8 cur_level)
{
    u16 keep_prd;
    u16 cnt_max = 0;
    if (JL_PLED->CON0 & BIT(1)) {//呼吸变化模式
        u16 h_pwm_duty_prd = (JL_PLED->BRI_DUTY0H >> 8) | JL_PLED->BRI_DUTY0L;
        u16 l_pwm_duty_prd = (JL_PLED->BRI_DUTY1H >> 8) | JL_PLED->BRI_DUTY1L;
        if (cur_dir) {
            keep_prd = (JL_PLED->DUTY3 << 8) | JL_PLED->DUTY2;
            if (cur_level) {
                cnt_max = keep_prd + l_pwm_duty_prd;
            } else {
                cnt_max = keep_prd + h_pwm_duty_prd;
            }
        } else {
            if (cur_level) {
                keep_prd = JL_PLED->DUTY0;
                cnt_max = keep_prd + l_pwm_duty_prd;
            } else {
                keep_prd = JL_PLED->DUTY1;
                cnt_max = keep_prd + h_pwm_duty_prd;
            }
        }
    } else {
        /* cnt_max = JL_PLED->DUTY3; */
        cnt_max = 0xff;
    }
    return cnt_max;
}


/*
 * @brief  获取PWM_LED模块状态信息
 */
void pwm_led_get_sync_status(struct pwm_led_status_t *status)
{
    if (!status) {
        return;
    }

    status->cnt_max = 0;

    if ((JL_PLED->CON0 & BIT(0)) == 0) {
        return;
    }

    u8 div_reg_val = (JL_PLED->CON0 >> 4) & 0xf;

    u32 div = led_clk_div_table[div_reg_val];

    u32 lrc_freq = __get_lrc_hz();

    u32 prd_div = JL_PLED->PRD_DIVL | ((JL_PLED->CON3 & 0b1111) << 8);
    u32 pwm_led_ctl_unit = (prd_div + 1) * div * 1000000 / lrc_freq; //单位us


    JL_PLED->CON4 |= BIT(4);
    while (!(JL_PLED->CON4 & BIT(5)));
    u32 cnt_rd = JL_PLED->CNT_RD;

    u8 next_dir = 0;
    u8 next_level = 0;

    status->dir   = !!(cnt_rd & BIT(17));
    status->level = !!(cnt_rd & BIT(16));
    status->cur_cnt  = cnt_rd & 0xff;
    status->cnt_max = pwm_led_get_cur_status_cnt_max(status->dir, status->level);
    pwm_led_get_next_dir_level(status->dir, status->level, (u8 *)&next_dir, (u8 *)&next_level);
    status->next_cnt_max = pwm_led_get_cur_status_cnt_max(next_dir, next_level);
    status->cnt_unit = pwm_led_ctl_unit;

    log_debug("dir:%d,", status->dir);
    log_debug("level:%d,", status->level);
    log_debug("cur_cnt:%d,", status->cur_cnt);
    log_debug("cnt_max:%d,", status->cnt_max);
    log_debug("next_cnt_max:%d,", status->next_cnt_max);
    log_debug("cnt_unit:%d,", status->cnt_unit);

}

/*
 * @brief  设置PWM_LED暂停同步
 * @arg status 另一个样机的pwm_led的状态
 * @arg how_long_ago 另一个样机的pwm_led的状态是多久之前的, 单位us
 * @arg sync_time 如果为快的样机，则通过该变量获取同步暂停时间, 单位us
 * @return 0:成功  1:失败
 */
u32 pwm_led_set_sync(struct pwm_led_status_t *status, u32 how_long_ago, u32 *sync_time)
{
    extern void log_pwm_led_info();
    log_pwm_led_info();

    if (!status) {
        return 1;
    }

    if (status->cnt_max == 0) {
        return 1;
    }

    if (status->next_cnt_max == 0) {
        return 1;
    }

    u8 div_reg_val = (JL_PLED->CON0 >> 4) & 0xf;

    u32 div = led_clk_div_table[div_reg_val];


    u32 lrc_freq = __get_lrc_hz();

    u32 prd_div = JL_PLED->PRD_DIVL | ((JL_PLED->CON3 & 0b1111) << 8);
    u32 pwm_led_ctl_unit = (prd_div + 1) * div * 1000000 / lrc_freq; //单位us

    if ((status->cnt_unit == 0) || (pwm_led_ctl_unit == 0)) {
        return 1;
    }

    if ((JL_PLED->CON0 & BIT(0)) == 0) {
        return 1;
    }

    /* u8 dir   = status->dir; */
    /* u8 level = status->level; */
    u16 cnt_delta = how_long_ago / status->cnt_unit;
    u16 cnt = status->cur_cnt;

    u8 table_idle = 0;
    u16 cnt_max_table[2];
    cnt_max_table[0] = status->cnt_max;
    cnt_max_table[1] = status->next_cnt_max;
    u16 cnt_max = cnt_max_table[0];

    if (JL_PLED->CON0 & BIT(1)) {//呼吸变化模式
        for (u16 i = 0; i < cnt_delta; i ++) {
            cnt ++;
            if (cnt > cnt_max) {
                cnt = 0;
                table_idle = !table_idle;
                cnt_max = cnt_max_table[table_idle];
            }
        }
    } else {
        cnt += cnt_delta;
        cnt %= (cnt_max + 1);
    }
    u16 progress = 1000 * cnt / cnt_max;


    JL_PLED->CON4 |= BIT(4);
    while (!(JL_PLED->CON4 & BIT(5)));
    u32 cnt_rd = JL_PLED->CNT_RD;

    u8 cur_dir   = !!(cnt_rd & BIT(17));
    u8 cur_level = !!(cnt_rd & BIT(16));
    u16 cur_cnt  = cnt_rd & 0xff;
    u16 cur_cnt_max  = pwm_led_get_cur_status_cnt_max(cur_dir, cur_level);

    u16 cur_progress = 1000 * cur_cnt / cur_cnt_max;

    log_debug("cur_dir:%d,", cur_dir);
    log_debug("cur_level:%d,", cur_level);
    log_debug("cur_cnt:%d,", cur_cnt);
    log_debug("cur_cnt_max:%d,", cur_cnt_max);
    log_debug("cur_progress:%d,", cur_progress);
    log_debug("cur_ctl_unit:%d,", pwm_led_ctl_unit);


    u32 wait_time = 0;
    u16 diff;
    if (JL_PLED->CON0 & BIT(1)) {//呼吸变化模式
        if ((cnt_max_table[0] > cnt_max_table[1]) && (cnt_max == cnt_max_table[1]) && (cur_dir == 1)) {
            wait_time = ((cnt_max - cnt) + (cur_progress * cnt_max_table[0] / 1000)) * status->cnt_unit;
        }
        if ((cnt_max_table[0] < cnt_max_table[1]) && (cnt_max == cnt_max_table[0]) && (cur_dir == 1)) {
            wait_time = ((cnt_max - cnt) + (cur_progress * cnt_max_table[1] / 1000)) * status->cnt_unit;
        }
        if (cur_progress > progress) {
            diff = cur_progress - progress;
            wait_time = (diff * cnt_max / 1000) * status->cnt_unit;
        }
    } else {
        if ((cur_progress > progress) && ((cur_progress - progress) <= 500)) {
            diff = cur_progress - progress;
            wait_time = (diff * cnt_max / 1000) * status->cnt_unit;
        }
        if ((cur_progress < progress) && ((progress - cur_progress) >= 500)) {
            diff = 1000 - (progress - cur_progress);
            wait_time = (diff * cnt_max / 1000) * status->cnt_unit;
        }
    }

    *sync_time = 0;
    u16 sync = wait_time / pwm_led_ctl_unit;
    if (sync) {
        *sync_time = wait_time;
        JL_PLED->CNT_SYNC = sync;
        log_debug("sync = %d\n", sync);
    }

#if 0
    if (sync) {
        u16 dec = cur_cnt_max / sync;
        JL_PLED->CNT_DEC = dec > 255 ? 0 : dec;
        log_debug("dec = %d\n", dec);
    }
#endif

    return 0;
}



