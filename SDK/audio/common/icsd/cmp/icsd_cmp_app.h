#ifndef _ICSD_CMP_APP_H
#define _ICSD_CMP_APP_H

#include "icsd_cmp_config.h"
#include "icsd_common_v2_app.h"

#define ANC_ADAPTIVE_CMP_ORDER			6			/*ANC自适应CMP滤波器阶数，原厂指定*/

enum ANC_EAR_ADAPTIVE_CMP_CH {
    ANC_EAR_ADAPTIVE_CMP_CH_L = 0,
    ANC_EAR_ADAPTIVE_CMP_CH_R,
};

struct anc_cmp_param_output {

    float l_gain;
    double *l_coeff;

    float r_gain;
    double *r_coeff;
};

int audio_anc_ear_adaptive_cmp_open(void);

void audio_anc_ear_adaptive_cmp_run(__afq_output *p);

int audio_anc_ear_adaptive_cmp_close(void);

//获取FGQ
float *audio_anc_ear_adaptive_cmp_output_get(enum ANC_EAR_ADAPTIVE_CMP_CH ch);

//获取IIR_COEFF
int audio_rtanc_adaptive_cmp_output_get(struct anc_cmp_param_output *output);

u8 audio_anc_ear_adaptive_cmp_result_get(void);

#endif
