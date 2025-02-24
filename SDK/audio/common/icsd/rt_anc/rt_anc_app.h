#ifndef _RT_ANC_APP_H_
#define _RT_ANC_APP_H_

#include "typedef.h"
#include "asm/anc.h"
#include "anc_ext_tool.h"

#define AUDIO_RT_ANC_PARAM_BY_TOOL_DEBUG		1		//RT ANC 参数使用工具debug 调试

//RT ANC 状态
enum {
    RT_ANC_STATE_CLOSE = 0,
    RT_ANC_STATE_OPEN,
    RT_ANC_STATE_SUSPEND,
};

void rt_anc_function_init(void);

int audio_adt_rtanc_set_infmt(void);

void audio_adt_rtanc_output_handle(void *rt_param_l, void *rt_param_r);

void audio_rtanc_debug_param_set(u8 cmd, float param);

u8 audio_rtanc_app_func_en_get(void);

int audio_rtanc_adaptive_en(u8 en);

//对外API
void audio_anc_real_time_adaptive_suspend(void);

void audio_anc_real_time_adaptive_resume(void);

u8 audio_anc_real_time_adaptive_busy_get(void);

void audio_anc_real_time_adaptive_init(audio_anc_t *param);

int audio_anc_real_time_adaptive_open(void);

int audio_anc_real_time_adaptive_close(void);

#endif


