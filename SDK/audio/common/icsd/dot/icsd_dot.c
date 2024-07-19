#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".icsd_dot.data.bss")
#pragma data_seg(".icsd_dot.data")
#pragma const_seg(".icsd_dot.text.const")
#pragma code_seg(".icsd_dot.text")
#endif

#include "app_config.h"
#if ((defined TCFG_AUDIO_FIT_DET_ENABLE) && TCFG_AUDIO_FIT_DET_ENABLE && \
	 TCFG_AUDIO_ANC_ENABLE)
#include "icsd_dot.h"
#include "icsd_dot_app.h"
#include "audio_anc.h"

u16	DOT_FUNCTION_DEBUG = 0;
struct icsd_dot_parm DOT_PARM;

int (*dot_printf)(const char *format, ...) = _dot_printf;
void dot_printf_off(char *format, ...) {}

void icsd_dot_output(int dot_db)
{
    float norm_thr = DOT_PARM.norm_thr;
    float loose_thr = DOT_PARM.loose_thr;

    dot_printf("========================================= \n");
    dot_printf("                                dot db = %d \n", dot_db);
    dot_printf("========================================= \n");
    if (dot_db > norm_thr) {
        dot_printf(" dot: tight ");
    } else if (dot_db > loose_thr) {
        dot_printf(" dot: norm ");
    } else { // < loose_thr
        dot_printf(" dot: loose ");
    }
    audio_icsd_dot_end(dot_db);
}

void icsd_dot_data_debug_en(u8 en)
{
    if (en) {
        DOT_FUNCTION_DEBUG |= DOT_DEBUG_DATA;
    } else {
        DOT_FUNCTION_DEBUG &= ~DOT_DEBUG_DATA;
    }
}

void icsd_dot_line_debug_en(u8 en)
{
    if (en) {
        DOT_FUNCTION_DEBUG |= DOT_DEBUG_LINE;
    } else {
        DOT_FUNCTION_DEBUG &= ~DOT_DEBUG_LINE;
    }
}

void icsd_dot_post_anc_task_cmd(u8 cmd)
{
    printf("icsd_dot_post_anc_task_cmd:%d----------------------------------------", cmd);
    os_taskq_post_msg("anc", 2, ANC_MSG_DOT, cmd);
}

void icsd_dot_tone_start(u32 slience_frames, u16 sample_rate)
{
    printf("icsd_dot_tone_start\n");
    icsd_dot_start(slience_frames, sample_rate);
}

void icsd_dot_set_alogm(void *_param, int alogm)
{
    audio_anc_t *param = _param;
    param->gains.alogm = alogm;

    param->anc_fade_gain = 0;
    param->gains.l_ffgain = 0;
    param->gains.l_fbgain = 0;
    param->gains.l_cmpgain = 0;
    param->gains.r_ffgain = 0;
    param->gains.r_fbgain = 0;
    param->gains.r_cmpgain = 0;
    anc_user_train_process(param);
}

void icsd_dot_parm_init()
{
    DOT_PARM.dot_len = 14;
    DOT_PARM.start_point = 4;
    DOT_PARM.norm_thr = -2;
    DOT_PARM.loose_thr = -5;
}

#if 0
void icsd_dot_demo(audio_anc_t *param)
{
    printf("===================icsd_dot_init===================\n");
    icsd_dot_version();
    struct icsd_dot_libfmt libfmt;
    icsd_dot_get_libfmt(&libfmt);
    dot_printf("DOT RAM SIZE:%d\n", libfmt.lib_alloc_size);
    icsd_dot_malloc(libfmt.lib_alloc_size);
    fmt.tone_delay = 700;
    fmt.dot_dma_on = anc_dma_on;
    icsd_dot_set_infmt(&fmt);
    //icsd_dot_set_alogm(param, DOT_ANC_DMA_SR_SEL);
    icsd_dot_parm_init();
    icsd_dot_data_debug_en(0);
    icsd_dot_line_debug_en(0);
}
#endif

#endif/*TCFG_AUDIO_FIT_DET_ENABLE*/
