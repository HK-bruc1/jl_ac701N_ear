#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".icsd_anc.data.bss")
#pragma data_seg(".icsd_anc.data")
#pragma const_seg(".icsd_anc.text.const")
#pragma code_seg(".icsd_anc.text")
#endif

#include "app_config.h"
#include "audio_config_def.h"
#if (TCFG_AUDIO_ANC_EAR_ADAPTIVE_EN && \
	 TCFG_AUDIO_ANC_ENABLE && \
	 TCFG_AUDIO_ANC_EAR_ADAPTIVE_VERSION == ANC_EXT_V2)

#include "audio_anc.h"
#include "icsd_anc_v2_config.h"
#include "icsd_common_v2.h"

const u8 ICSD_ANC_VERSION = 2;



struct icsd_anc_v2_tool_data *TOOL_DATA = NULL;

const int cmp_idx_begin = 0;
const int cmp_idx_end = 59;
const int cmp_total_len = 60;
const int cmp_en = 0;
const float pz_gain = 0;
const u8 ICSD_ANC_V2_BYPASS_ON_FIRST = 0;//播放提示音过程中打开BYPASS节省BYPASS稳定时间

/***************************************************************/
/****************** ANC MODE bypass fgq ************************/
/***************************************************************/
const float gfq_bypass[] = {
    -10,  5000,  1,
};
const u8 tap_bypass = 1;
const u8 type_bypass[] = {3};
const float fs_bypass = 375e3;
double bbbaa_bypass[1 * 5];

void icsd_anc_v2_board_config()
{
    double a0, a1, a2, b0, b1, b2;
    for (int i = 0; i < tap_bypass; i++) {
        icsd_biquad2ab_out_v2(gfq_bypass[3 * i], gfq_bypass[3 * i + 1], fs_bypass, gfq_bypass[3 * i + 2], &a0, &a1, &a2, &b0, &b1, &b2, type_bypass[i]);
        bbbaa_bypass[5 * i + 0] = b0 / a0;
        bbbaa_bypass[5 * i + 1] = b1 / a0;
        bbbaa_bypass[5 * i + 2] = b2 / a0;
        bbbaa_bypass[5 * i + 3] = a1 / a0;
        bbbaa_bypass[5 * i + 4] = a2 / a0;
    }
}

const int FF_objFunc_type  = 3;
const int CMP_objFunc_type = 1;

/***************************************************************/
// cmp
/***************************************************************/
const float cmp_Vrange_tmp_m[] = {   // 62
    -10, 20, 20,   100,  0.8, 1.2,
    -10, 20, 100,  300,  0.8, 1.2,
    -10, 20, 300,  800,  0.6, 1.1,
    -10, 20, 800,  1500,  0.8, 1.2,
    1.2, 0.1,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
};
const float cmp_biquad_init_tmp_m[] = { // 217
    -4.67, 3076, 1.2,
    -3.874,  6506,  1.0,
    //
    0.75, 50, 1.0,
    3.81,  202, 1.0,
    1.59,  410, 0.8,
    0.40,  888,  0.7,
    0.307946,
};

const float cmp_weight_m[] = { //354
    5.000, 5.000, 5.000, 5.000, 5.00,  5.00,  5.00,  5.00,  5.000, 5.000, 5.000, 5.000, 5.000, 5.000, 5.000,
    5.000, 5.000, 5.000, 5.000, 5.00,  5.00,  5.00,  5.00,  5.000, 5.000, 5.000, 5.000, 5.000, 5.000, 5.000,
    5.000, 5.000, 5.000, 5.000, 5.00,  5.00,  5.00,  5.00,  5.000, 5.000, 5.000, 5.000, 5.000, 5.000, 5.000,
    2.000, 2.000, 2.000, 2.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000,
};
const float cmp_mse_tar_m[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

const int ICSD_CMP_IIR_NUM_FLEX = 4;
const int ICSD_CMP_IIR_NUM_FIX  = 2;
const int ICSD_CMP_IIR_COEF = ICSD_CMP_IIR_NUM_FLEX * 3 + 1;
const float CMP_FSTOP_IDX = 2000;
const float CMP_FSTOP_IDX2 = 2000;

/***************************************************************/
/********************** rt_anc_filrer config *******************/
/***************************************************************/
//0, hp, 1, lp, 2, bp, 3, hs, 4, ls

const float fgq_init_table[] = {
    -4.000000, 26.000000, 0.800000,
    -1.000000, 59.000000, 1.000000,
    -4.600000, 129.000000, 0.600000,
    -2.000000, 412.000000, 1.200000,
    -1.400000, 709.000000, 1.000000,
    3.200000, 1335.000000, 0.900000,
    -7.000000, 1809.000000, 1.000000,
    -4.000000, 3365.000000, 1.000000,
    -6.000000, 6000.000000, 1.500000,
    -14.000000, 12000.000000, 1.000000,
};

const float fgq_init_table_fb[] = {
    -5.622, 55, 1,
    -2.427, 1816, 1,
    10.845, 136, 0.5,
    -5.868, 450, 0.8,
    -13.241, 3235, 0.7,
};

/***************************************************************/
/*********************** DE 算法 *******************************/
/***************************************************************/
const float FSTOP_IDX = 2700;
const float FSTOP_IDX2 = 2700;

const float Gold_csv_Perf_Range[] = {
    2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,
    2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,
    2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,
    2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,   2.0,
};

//degree_level, gain_limit, limit_mse_begin, limit_mse_end, over_mse_begin, over_mse_end,biquad_cut
float degree_set0[] = {3, -3, 50, 2200, 50, 2700, 5};
float degree_set1[] = {8, 10, 50, 2200, 50, 2700, 8};
float degree_set2[] = {11, 5, 50, 2200, 50, 2700, 11};

#endif
