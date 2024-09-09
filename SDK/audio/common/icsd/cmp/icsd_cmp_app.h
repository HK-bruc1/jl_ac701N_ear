#ifndef _ICSD_CMP_APP_H
#define _ICSD_CMP_APP_H

#include "audio_anc.h"
#include "icsd_cmp_config.h"
#include "icsd_common_v2_app.h"

enum ANC_EAR_ADAPTIVE_CMP_CH {
    ANC_EAR_ADAPTIVE_CMP_CH_L = 0,
    ANC_EAR_ADAPTIVE_CMP_CH_R,
};

int audio_anc_ear_adaptive_cmp_open(void);

void audio_anc_ear_adaptive_cmp_run(__afq_output *p);

int audio_anc_ear_adaptive_cmp_close(void);

float *audio_anc_ear_adaptive_cmp_output_get(enum ANC_EAR_ADAPTIVE_CMP_CH ch);

u8 audio_anc_ear_adaptive_cmp_result_get(void);

#endif
