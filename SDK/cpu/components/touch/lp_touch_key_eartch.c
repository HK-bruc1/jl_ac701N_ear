#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".lp_touch_key_eartch.data.bss")
#pragma data_seg(".lp_touch_key_eartch.data")
#pragma const_seg(".lp_touch_key_eartch.text.const")
#pragma code_seg(".lp_touch_key_eartch.text")
#endif


#if TCFG_LP_EARTCH_KEY_ENABLE


#include "eartouch_manage.h"

#if TCFG_LP_EARTCH_DETECT_RELY_AUDIO
#include "icsd_adt_app.h"
#endif


#define EARTCH_DET_IN_EAR_BY_TOUCH_TRIGGER_EITHER		    0//触发其中一个通道则可认为入耳，单点和双点触摸均可兼容
#define EARTCH_DET_IN_EAR_BY_TOUCH_TRIGGER_TOGETHER		    1//同时触发两个通过才可认为入耳，仅用于双点触摸
#define EARTCH_DET_IN_EAR_TOUCH_LOGIC                       EARTCH_DET_IN_EAR_BY_TOUCH_TRIGGER_EITHER


void __attribute__((weak)) eartch_notify_event(u32 state)
{
    eartouch_event_handler(state);
}

u32 lp_touch_key_eartch_get_state(void)
{
    return M2P_CTMU_OUTEAR_VALUE_H;
}

void lp_touch_key_eartch_set_state(u32 state)
{
    M2P_CTMU_OUTEAR_VALUE_H = state;
}

u32 lp_touch_key_eartch_state_is_out_ear(void)
{
    if (M2P_CTMU_OUTEAR_VALUE_H == 0) {
        return 1;
    }
    return 0;
}

u32 lp_touch_key_eartch_touch_trigger_together(void)
{
    log_debug("eartch_touch_detect_result = 0x%x\n", P2M_CTMU_EARTCH_EVENT);
    if (P2M_CTMU_EARTCH_EVENT == M2P_CTMU_EARTCH_CH) {
        return 1;
    }
    return 0;
}

u32 lp_touch_key_eartch_touch_trigger_either(void)
{
    log_debug("eartch_touch_detect_result = 0x%x\n", P2M_CTMU_EARTCH_EVENT);
    if (P2M_CTMU_EARTCH_EVENT) {
        return 1;
    }
    return 0;
}

u32 lp_touch_key_eartch_touch_is_invalid(void)
{
    log_debug("eartch_touch_algo_invalid = 0x%x\n", P2M_CTMU_EARTCH_L_IIR_VALUE);
    if (P2M_CTMU_EARTCH_L_IIR_VALUE) {
        return 1;
    }
    return 0;
}

void lp_touch_key_eartch_set_touch_valid_time(u32 time)
{
    M2P_CTMU_OUTEAR_VALUE_L = time; //(x + 2) * 8 * 20;
}

static void lp_touch_key_eartch_notify_event(u32 state)
{
    if (get_charge_online_flag()) {
        state = EARTCH_MUST_BE_OUT_EAR;
    }

    u32 user_state;

    switch (state) {
    case EARTCH_MUST_BE_OUT_EAR:
        lp_touch_key_eartch_set_state(0);
        user_state = EARTOUCH_STATE_OUT_EAR;
        break;
    case EARTCH_MUST_BE_IN_EAR:
        lp_touch_key_eartch_set_state(1);
        user_state = EARTOUCH_STATE_IN_EAR;
        break;
    case EARTCH_MAY_BE_OUT_EAR:
        user_state = EARTOUCH_STATE_OUT_EAR;
        break;
    case EARTCH_MAY_BE_IN_EAR:
        user_state = EARTOUCH_STATE_IN_EAR;
        break;
    default :
        user_state = EARTOUCH_STATE_OUT_EAR;
        break;
    }

    if (__this->eartch.det_last_state != user_state) {
        __this->eartch.det_last_state = user_state;
        eartch_notify_event(user_state);
    }
}


#if TCFG_LP_EARTCH_DETECT_RELY_AUDIO

void lp_touch_key_eartch_trigger_audio_detect(void)
{
    u32 state = lp_touch_key_eartch_get_state();
    log_debug("audio_ear_in_detect_open ---> %d\n", state);
    audio_ear_in_detect_open(state);
}

void lp_touch_key_eartch_close_audio_detect(void)
{
    log_debug("audio_ear_in_detect_close\n");
    audio_ear_in_detect_close();
}

void lp_touch_key_eartch_audio_detect_valid(void *priv)
{
    __this->eartch.audio_det_invalid_timeout = 0;
}

void lp_touch_key_eartch_audio_detect_valid_deal(void)
{
    u32 state = 0;
    u32 in_ear_valid = BIT((__this->pdata->eartch_audio_det_filter_param + 1)) - 1;//BIT(6+1)-1=0b111111;
    u32 det_valid = __this->eartch.audio_det_valid & in_ear_valid;
    log_debug("eartch_audio_detect_result: %x\n", det_valid);

    if (det_valid == 0) {
        state = EARTCH_MUST_BE_OUT_EAR;
    } else if (det_valid == in_ear_valid) {
        state = EARTCH_MUST_BE_IN_EAR;
    } else {
        return;
    }
    if ((__this->eartch.touch_invalid == 0) && (__this->eartch.audio_det_invalid_timeout == 0)) {
        lp_touch_key_eartch_close_audio_detect();
        lp_touch_key_eartch_notify_event(state);
    }
}

void lp_touch_key_eartch_audio_detect_cbfunc(u32 det_result)
{
    __this->eartch.audio_det_valid <<= 1;
    __this->eartch.audio_det_valid += det_result;
    lp_touch_key_eartch_audio_detect_valid_deal();
}

#else


void lp_touch_key_eartch_touch_valid_deal(void)
{
#if (EARTCH_DET_IN_EAR_TOUCH_LOGIC == EARTCH_DET_IN_EAR_BY_TOUCH_TRIGGER_TOGETHER)
    if (lp_touch_key_eartch_state_is_out_ear()) {
        if (lp_touch_key_eartch_touch_trigger_together()) {
            lp_touch_key_eartch_notify_event(EARTCH_MUST_BE_IN_EAR);
        } else {
            lp_touch_key_eartch_notify_event(EARTCH_MUST_BE_OUT_EAR);
        }
    } else
#endif
    {
        if (lp_touch_key_eartch_touch_trigger_either()) {
            lp_touch_key_eartch_notify_event(EARTCH_MUST_BE_IN_EAR);
        } else {
            lp_touch_key_eartch_notify_event(EARTCH_MUST_BE_OUT_EAR);
        }
    }
}

#endif


void lp_touch_key_eartch_check_touch_valid(void *priv)
{
    if (lp_touch_key_eartch_touch_is_invalid()) {
        return;
    }

    lp_touch_key_eartch_touch_valid_deal();

    if (__this->eartch.check_touch_valid_timer) {
        sys_hi_timer_del(__this->eartch.check_touch_valid_timer);
        __this->eartch.check_touch_valid_timer = 0;
    }
}

void lp_touch_key_eartch_touch_invalid_timeout(void *priv)
{
    __this->eartch.touch_invalid_timeout = 0;
    log_debug("eartch_touch_invalid_timeout");
#if TCFG_LP_EARTCH_DETECT_RELY_AUDIO
    lp_touch_key_eartch_notify_event(EARTCH_MAY_BE_IN_EAR);
#else
    lp_touch_key_eartch_notify_event(EARTCH_MUST_BE_IN_EAR);
#endif
}

static void lp_touch_key_eartch_event_deal(u32 event)
{
    if (event) {

#if (EARTCH_DET_IN_EAR_TOUCH_LOGIC == EARTCH_DET_IN_EAR_BY_TOUCH_TRIGGER_EITHER)
        if (__this->eartch.touch_invalid == 0) {
#else
        if (__this->eartch.touch_invalid == 1) {
#endif

            if (lp_touch_key_eartch_state_is_out_ear()) {
                __this->eartch.touch_invalid_timeout = sys_hi_timeout_add(NULL, lp_touch_key_eartch_touch_invalid_timeout, __this->pdata->eartch_touch_filter_time);
            }

#if TCFG_LP_EARTCH_DETECT_RELY_AUDIO
            lp_touch_key_eartch_trigger_audio_detect();
#endif
        }

#if TCFG_LP_EARTCH_DETECT_RELY_AUDIO
        if (__this->audio_det_invalid_timeout) {
            sys_hi_timeout_del(__this->eartch.audio_det_invalid_timeout);
            __this->eartch.audio_det_invalid_timeout = 0;
        }
#else
        if (__this->eartch.check_touch_valid_timer) {
            sys_hi_timer_del(__this->eartch.check_touch_valid_timer);
            __this->eartch.check_touch_valid_timer = 0;
        }
#endif

        __this->eartch.touch_invalid ++;

    } else {
        if (__this->eartch.touch_invalid) {
            __this->eartch.touch_invalid --;
        }

#if (EARTCH_DET_IN_EAR_TOUCH_LOGIC == EARTCH_DET_IN_EAR_BY_TOUCH_TRIGGER_TOGETHER)

        if (__this->eartch.touch_invalid_timeout) {
            sys_hi_timeout_del(__this->eartch.touch_invalid_timeout);
            __this->eartch.touch_invalid_timeout = 0;
            log_debug("sys_hi_timeout_del  eartch_touch_invalid_timeout");
        }
#endif

        if (__this->eartch.touch_invalid == 0) {

#if (EARTCH_DET_IN_EAR_TOUCH_LOGIC == EARTCH_DET_IN_EAR_BY_TOUCH_TRIGGER_EITHER)

            if (__this->eartch.touch_invalid_timeout) {
                sys_hi_timeout_del(__this->eartch.touch_invalid_timeout);
                __this->eartch.touch_invalid_timeout = 0;
                log_debug("sys_hi_timeout_del  eartch_touch_invalid_timeout");
            }
#endif

#if TCFG_LP_EARTCH_DETECT_RELY_AUDIO

            if (__this->eartch.audio_det_invalid_timeout == 0) {
                __this->eartch.audio_det_invalid_timeout = sys_hi_timeout_add(NULL, lp_touch_key_eartch_audio_detect_valid, __this->pdata->eartch_audio_det_valid_time);
            } else {
                sys_hi_timeout_modify(__this->eartch.audio_det_invalid_timeout, __this->pdata->eartch_audio_det_valid_time);
            }
#else

            if (lp_touch_key_eartch_touch_is_invalid()) {
                if (__this->eartch.check_touch_valid_timer == 0) {
                    __this->eartch.check_touch_valid_timer = sys_hi_timer_add(NULL, lp_touch_key_eartch_check_touch_valid, __this->pdata->eartch_check_touch_valid_time);
                } else {
                    sys_hi_timer_modify(__this->eartch.check_touch_valid_timer, __this->pdata->eartch_check_touch_valid_time);
                }
            } else {
                if (__this->eartch.check_touch_valid_timer) {
                    sys_hi_timer_del(__this->eartch.check_touch_valid_timer);
                    __this->eartch.check_touch_valid_timer = 0;
                }
                lp_touch_key_eartch_touch_valid_deal();
            }
#endif

        }
    }

    log_debug("eartch_touch_invalid = %d\n", __this->eartch.touch_invalid);

}

void lp_touch_key_eartch_init(void)
{
    u32 valid_time = 16 * __this->pdata->lpctmu_pdata->sample_scan_time;
    if (__this->pdata->eartch_touch_valid_time > valid_time) {
        valid_time = __this->pdata->eartch_touch_valid_time - valid_time;
        valid_time /= __this->pdata->lpctmu_pdata->sample_scan_time;
        valid_time /= 8;
    } else {
        valid_time = 6;
    }
    lp_touch_key_eartch_set_touch_valid_time(valid_time);

    __this->eartch.det_last_state = 0xff;
}

void lp_touch_key_eartch_state_reset(void)
{
    lp_touch_key_eartch_set_state(0);
    __this->eartch.det_last_state = EARTOUCH_STATE_OUT_EAR;
}

void lp_touch_key_testbox_inear_trim(u32 flag)
{
}

#endif

