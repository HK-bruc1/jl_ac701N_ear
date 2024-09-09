#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".icsd_cmp.data.bss")
#pragma data_seg(".icsd_cmp.data")
#pragma const_seg(".icsd_cmp.text.const")
#pragma code_seg(".icsd_cmp.text")
#endif

#include "app_config.h"
#include "audio_config_def.h"
#if (TCFG_AUDIO_ANC_EAR_ADAPTIVE_EN && \
	 TCFG_AUDIO_ANC_ENABLE && \
	 TCFG_AUDIO_ANC_EAR_ADAPTIVE_VERSION == ANC_EXT_V2)

#include "audio_anc.h"

#if ANC_EAR_ADAPTIVE_CMP_EN

#include "icsd_cmp_config.h"
#include "icsd_anc_v2_app.h"

const int cmp_objFunc_type = 1;
const float cmp_GAIN_LIMIT_ALL = 30;

/***************************************************************/
// cmp
/***************************************************************/
const u8 icsd_cmp_type[] = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 };
const float icsd_cmp_Vrange_M[] = {   // 62
    -10, 20, 20,   100,  0.8, 1.2,
    -10, 20, 100,  300,  0.8, 1.2,
    -10, 20, 300,  800,  0.6, 1.1,
    -10, 20, 800,  1500,  0.8, 1.2,
    0.1, 1.1,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
};
const float icsd_cmp_Biquad_init_M[] = { // 217
    -4.67, 3076, 1.2,
    -3.874,  6506,  1.0,
    //
    0.75, 50, 1.0,
    3.81,  202, 1.0,
    1.59,  410, 0.8,
    0.40,  888,  0.7,
    0.307946,
};

const float icsd_cmp_Weight_M[] = { //354
    5.000, 5.000, 5.000, 5.000, 5.00,  5.00,  5.00,  5.00,  5.000, 5.000, 5.000, 5.000, 5.000, 5.000, 5.000,
    5.000, 5.000, 5.000, 5.000, 5.00,  5.00,  5.00,  5.00,  5.000, 5.000, 5.000, 5.000, 5.000, 5.000, 5.000,
    5.000, 5.000, 5.000, 5.000, 5.00,  5.00,  5.00,  5.00,  5.000, 5.000, 5.000, 5.000, 5.000, 5.000, 5.000,
    2.000, 2.000, 2.000, 2.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000,
};
const float icsd_cmp_Gold_csv_M[] = {
    -20, -20, -20, -20, -20, -20,  -20, -20, -20, -20, -20,  -20, -20, -20, -20,
    -20, -20, -20, -20, -20, -20,  -20, -20, -20, -20, -20,  -20, -20, -20, -20,
    -20, -20, -20, -20, -20, -20,  -20, -20, -20, -20, -20,  -20, -20, -20, -20,
    -20, -20, -20, -20, -20, -20,  -20, -20, -20, -20, -20,  -20, -20, -20, -20,
    -20, -20, -20, -20, -20, -20,  -20, -20, -20, -20, -20,  -20, -20, -20, -20,
};

const float icsd_cmp_degree_set_0[] = {
    3, -3, 50, 2200, 50, 2700, 5,
};

const int   cmp_IIR_NUM_FLEX = 4;
const int   cmp_IIR_NUM_FIX  = ANC_ADAPTIVE_CMP_ORDER - cmp_IIR_NUM_FLEX;
const int   cmp_IIR_COEF = cmp_IIR_NUM_FLEX * 3 + 1;
const float cmp_FSTOP_IDX = 2000;
const float cmp_FSTOP_IDX2 = 2000;


#endif
#endif
