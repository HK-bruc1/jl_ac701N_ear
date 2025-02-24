#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".icsd_anc.data.bss")
#pragma data_seg(".icsd_anc.data")
#pragma const_seg(".icsd_anc.text.const")
#pragma code_seg(".icsd_anc.text")
#endif

#define USE_BOARD_CONFIG 0

#include "app_config.h"
#include "audio_config_def.h"
#if ((TCFG_AUDIO_ANC_EAR_ADAPTIVE_EN || TCFG_AUDIO_ANC_REAL_TIME_ADAPTIVE_ENABLE) && \
	 TCFG_AUDIO_ANC_ENABLE && TCFG_AUDIO_ANC_EAR_ADAPTIVE_VERSION == ANC_EXT_V2)

#include "audio_anc.h"
#include "icsd_anc_v2_config.h"
#include "icsd_common_v2.h"

#if (USE_BOARD_CONFIG == 1)
#include "icsd_anc_v2_board.c"
#endif

const u8 ICSD_ANC_VERSION = 2;
const u8 ICSD_ANC_TOOL_PRINTF = 0;
const u8 msedif_en = 0;
const u8 target_diff_en = 0;

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


void icsd_sd_cfg_set(__icsd_anc_config_data *SD_CFG, void *_ext_cfg)
{
    struct anc_ext_ear_adaptive_param *ext_cfg = (struct anc_ext_ear_adaptive_param *)_ext_cfg;
#if (USE_BOARD_CONFIG == 1)

    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>anc use board config\n");
    icsd_anc_config_board_init(SD_CFG);

    return ;
#endif
    if (SD_CFG) {
        //界面信息配置
        SD_CFG->pnc_times = ext_cfg->base_cfg->adaptive_times;
        SD_CFG->vld1 = ext_cfg->base_cfg->vld1;
        SD_CFG->vld2 = ext_cfg->base_cfg->vld2;
        SD_CFG->sz_pri_thr = ext_cfg->base_cfg->sz_pri_thr;
        SD_CFG->bypass_vol = ext_cfg->base_cfg->bypass_vol;
        SD_CFG->sz_calr_sign = ext_cfg->base_cfg->calr_sign[0];
        SD_CFG->pz_calr_sign = ext_cfg->base_cfg->calr_sign[1];
        SD_CFG->bypass_calr_sign = ext_cfg->base_cfg->calr_sign[2];
        SD_CFG->perf_calr_sign = ext_cfg->base_cfg->calr_sign[3];
        SD_CFG->train_mode = ext_cfg->train_mode;	//自适应训练模式设置 */
        SD_CFG->tonel_delay = ext_cfg->base_cfg->tonel_delay;
        SD_CFG->toner_delay = ext_cfg->base_cfg->toner_delay;
        SD_CFG->pzl_delay = ext_cfg->base_cfg->pzl_delay;
        SD_CFG->pzr_delay = ext_cfg->base_cfg->pzr_delay;
        SD_CFG->ear_recorder = ext_cfg->ff_ear_mem_param->ear_recorder;
        SD_CFG->fb_agc_en = 0;
        //其他配置
        SD_CFG->ff_yorder  = ANC_ADAPTIVE_FF_ORDER;
        SD_CFG->fb_yorder  = ANC_ADAPTIVE_FB_ORDER;
        //SD_CFG->cmp_yorder = ANC_ADAPTIVE_CMP_ORDER;
        if (ICSD_ANC_CPU == ICSD_BR28) {
            printf("ICSD BR28 SEL\n");
            SD_CFG->normal_out_sel_l = BR28_ANC_USER_TRAIN_OUT_SEL_L;
            SD_CFG->normal_out_sel_r = BR28_ANC_USER_TRAIN_OUT_SEL_R;
            SD_CFG->tone_out_sel_l   = BR28_ANC_TONE_TRAIN_OUT_SEL_L;
            SD_CFG->tone_out_sel_r   = BR28_ANC_TONE_TRAIN_OUT_SEL_R;
        } else if (ICSD_ANC_CPU == ICSD_BR50) {
            printf("ICSD BR50 SEL\n");
            SD_CFG->normal_out_sel_l = BR50_ANC_USER_TRAIN_OUT_SEL_L;
            SD_CFG->normal_out_sel_r = BR50_ANC_USER_TRAIN_OUT_SEL_R;
            SD_CFG->tone_out_sel_l   = BR50_ANC_TONE_TRAIN_OUT_SEL_L;
            SD_CFG->tone_out_sel_r   = BR50_ANC_TONE_TRAIN_OUT_SEL_R;
        } else {
            printf("CPU ERR\n");
        }
        /***************************************************/
        /**************** left channel config **************/
        /***************************************************/
        SD_CFG->adpt_cfg.pnc_times = ext_cfg->base_cfg->adaptive_times;
        // target配置
        SD_CFG->adpt_cfg.cmp_en = ext_cfg->ff_target_param->cmp_en;
        SD_CFG->adpt_cfg.target_cmp_num = ext_cfg->ff_target_param->cmp_curve_num;
        SD_CFG->adpt_cfg.target_sv = ext_cfg->ff_target_sv->data;
        SD_CFG->adpt_cfg.target_cmp_dat = ext_cfg->ff_target_cmp->data;

        // 算法配置
        SD_CFG->adpt_cfg.IIR_NUM_FLEX = 0;
        int flex_idx = 0;
        int biquad_idx = 0;
        for (int i = 0; i < SD_CFG->ff_yorder; i++) {
            if (ext_cfg->ff_iir_high[i].fixed_en) { // fix
                SD_CFG->adpt_cfg.Biquad_init_H[3 * biquad_idx + 0] = ext_cfg->ff_iir_high[i].def.gain;
                SD_CFG->adpt_cfg.Biquad_init_H[3 * biquad_idx + 1] = ext_cfg->ff_iir_high[i].def.fre;
                SD_CFG->adpt_cfg.Biquad_init_H[3 * biquad_idx + 2] = ext_cfg->ff_iir_high[i].def.q;
                SD_CFG->adpt_cfg.Biquad_init_M[3 * biquad_idx + 0] = ext_cfg->ff_iir_medium[i].def.gain;
                SD_CFG->adpt_cfg.Biquad_init_M[3 * biquad_idx + 1] = ext_cfg->ff_iir_medium[i].def.fre;
                SD_CFG->adpt_cfg.Biquad_init_M[3 * biquad_idx + 2] = ext_cfg->ff_iir_medium[i].def.q;
                SD_CFG->adpt_cfg.Biquad_init_L[3 * biquad_idx + 0] = ext_cfg->ff_iir_low[i].def.gain;
                SD_CFG->adpt_cfg.Biquad_init_L[3 * biquad_idx + 1] = ext_cfg->ff_iir_low[i].def.fre;
                SD_CFG->adpt_cfg.Biquad_init_L[3 * biquad_idx + 2] = ext_cfg->ff_iir_low[i].def.q;
                SD_CFG->adpt_cfg.biquad_type[biquad_idx] = ext_cfg->ff_iir_high[i].type;
                biquad_idx += 1;
            } else { // not fix
                SD_CFG->adpt_cfg.Vrange_H[6 * flex_idx + 0] = ext_cfg->ff_iir_high[i].lower_limit.gain;
                SD_CFG->adpt_cfg.Vrange_H[6 * flex_idx + 1] = ext_cfg->ff_iir_high[i].upper_limit.gain;
                SD_CFG->adpt_cfg.Vrange_H[6 * flex_idx + 2] = ext_cfg->ff_iir_high[i].lower_limit.fre;
                SD_CFG->adpt_cfg.Vrange_H[6 * flex_idx + 3] = ext_cfg->ff_iir_high[i].upper_limit.fre;
                SD_CFG->adpt_cfg.Vrange_H[6 * flex_idx + 4] = ext_cfg->ff_iir_high[i].lower_limit.q;
                SD_CFG->adpt_cfg.Vrange_H[6 * flex_idx + 5] = ext_cfg->ff_iir_high[i].upper_limit.q;
                SD_CFG->adpt_cfg.Vrange_M[6 * flex_idx + 0] = ext_cfg->ff_iir_medium[i].lower_limit.gain;
                SD_CFG->adpt_cfg.Vrange_M[6 * flex_idx + 1] = ext_cfg->ff_iir_medium[i].upper_limit.gain;
                SD_CFG->adpt_cfg.Vrange_M[6 * flex_idx + 2] = ext_cfg->ff_iir_medium[i].lower_limit.fre;
                SD_CFG->adpt_cfg.Vrange_M[6 * flex_idx + 3] = ext_cfg->ff_iir_medium[i].upper_limit.fre;
                SD_CFG->adpt_cfg.Vrange_M[6 * flex_idx + 4] = ext_cfg->ff_iir_medium[i].lower_limit.q;
                SD_CFG->adpt_cfg.Vrange_M[6 * flex_idx + 5] = ext_cfg->ff_iir_medium[i].upper_limit.q;
                SD_CFG->adpt_cfg.Vrange_L[6 * flex_idx + 0] = ext_cfg->ff_iir_low[i].lower_limit.gain;
                SD_CFG->adpt_cfg.Vrange_L[6 * flex_idx + 1] = ext_cfg->ff_iir_low[i].upper_limit.gain;
                SD_CFG->adpt_cfg.Vrange_L[6 * flex_idx + 2] = ext_cfg->ff_iir_low[i].lower_limit.fre;
                SD_CFG->adpt_cfg.Vrange_L[6 * flex_idx + 3] = ext_cfg->ff_iir_low[i].upper_limit.fre;
                SD_CFG->adpt_cfg.Vrange_L[6 * flex_idx + 4] = ext_cfg->ff_iir_low[i].lower_limit.q;
                SD_CFG->adpt_cfg.Vrange_L[6 * flex_idx + 5] = ext_cfg->ff_iir_low[i].upper_limit.q;
                flex_idx += 1;
            }
        }

        SD_CFG->adpt_cfg.Vrange_H[flex_idx * 6 + 0] = -ext_cfg->ff_iir_high_gains->lower_limit_gain;
        SD_CFG->adpt_cfg.Vrange_H[flex_idx * 6 + 1] = -ext_cfg->ff_iir_high_gains->upper_limit_gain;
        SD_CFG->adpt_cfg.Vrange_M[flex_idx * 6 + 0] = -ext_cfg->ff_iir_medium_gains->lower_limit_gain;
        SD_CFG->adpt_cfg.Vrange_M[flex_idx * 6 + 1] = -ext_cfg->ff_iir_medium_gains->upper_limit_gain;
        SD_CFG->adpt_cfg.Vrange_L[flex_idx * 6 + 0] = -ext_cfg->ff_iir_low_gains->lower_limit_gain;
        SD_CFG->adpt_cfg.Vrange_L[flex_idx * 6 + 1] = -ext_cfg->ff_iir_low_gains->upper_limit_gain;


        SD_CFG->adpt_cfg.IIR_NUM_FLEX = flex_idx;
        SD_CFG->adpt_cfg.IIR_NUM_FIX = biquad_idx;

        for (int i = 0; i < SD_CFG->ff_yorder; i++) {
            if (!ext_cfg->ff_iir_high[i].fixed_en) { // not fix
                SD_CFG->adpt_cfg.Biquad_init_H[3 * biquad_idx + 0] = ext_cfg->ff_iir_high[i].def.gain;
                SD_CFG->adpt_cfg.Biquad_init_H[3 * biquad_idx + 1] = ext_cfg->ff_iir_high[i].def.fre;
                SD_CFG->adpt_cfg.Biquad_init_H[3 * biquad_idx + 2] = ext_cfg->ff_iir_high[i].def.q;
                SD_CFG->adpt_cfg.Biquad_init_M[3 * biquad_idx + 0] = ext_cfg->ff_iir_medium[i].def.gain;
                SD_CFG->adpt_cfg.Biquad_init_M[3 * biquad_idx + 1] = ext_cfg->ff_iir_medium[i].def.fre;
                SD_CFG->adpt_cfg.Biquad_init_M[3 * biquad_idx + 2] = ext_cfg->ff_iir_medium[i].def.q;
                SD_CFG->adpt_cfg.Biquad_init_L[3 * biquad_idx + 0] = ext_cfg->ff_iir_low[i].def.gain;
                SD_CFG->adpt_cfg.Biquad_init_L[3 * biquad_idx + 1] = ext_cfg->ff_iir_low[i].def.fre;
                SD_CFG->adpt_cfg.Biquad_init_L[3 * biquad_idx + 2] = ext_cfg->ff_iir_low[i].def.q;
                SD_CFG->adpt_cfg.biquad_type[biquad_idx] = ext_cfg->ff_iir_high[i].type;
                biquad_idx += 1;
            }
        }

        SD_CFG->adpt_cfg.Biquad_init_H[biquad_idx * 3] = -ext_cfg->ff_iir_high_gains->def_total_gain;
        SD_CFG->adpt_cfg.Biquad_init_M[biquad_idx * 3] = -ext_cfg->ff_iir_medium_gains->def_total_gain;
        SD_CFG->adpt_cfg.Biquad_init_L[biquad_idx * 3] = -ext_cfg->ff_iir_low_gains->def_total_gain;

        for (int i = 0; i < 7; i++) {
            SD_CFG->adpt_cfg.degree_set0[i] = degree_set0[i];
            SD_CFG->adpt_cfg.degree_set1[i] = degree_set1[i];
            SD_CFG->adpt_cfg.degree_set2[i] = degree_set2[i];
        }

        SD_CFG->adpt_cfg.degree_set0[0] = ext_cfg->ff_iir_general->biquad_level_l;
        SD_CFG->adpt_cfg.degree_set1[0] = ext_cfg->ff_iir_general->biquad_level_h;
        SD_CFG->adpt_cfg.degree_set2[0] = ext_cfg->ff_target_param->cmp_curve_num;
        SD_CFG->adpt_cfg.degree_set0[6] = ext_cfg->ff_iir_general->biquad_level_l;
        SD_CFG->adpt_cfg.degree_set1[6] = ext_cfg->ff_iir_general->biquad_level_h;
        SD_CFG->adpt_cfg.degree_set2[6] = ext_cfg->ff_target_param->cmp_curve_num;

        SD_CFG->adpt_cfg.degree_set0[1] = ext_cfg->ff_iir_high_gains->iir_target_gain_limit;
        SD_CFG->adpt_cfg.degree_set1[1] = ext_cfg->ff_iir_medium_gains->iir_target_gain_limit;
        SD_CFG->adpt_cfg.degree_set2[1] = ext_cfg->ff_iir_low_gains->iir_target_gain_limit;

        SD_CFG->adpt_cfg.Weight_H = ext_cfg->ff_weight_high->data;
        SD_CFG->adpt_cfg.Weight_M = ext_cfg->ff_weight_medium->data;
        SD_CFG->adpt_cfg.Weight_L = ext_cfg->ff_weight_low->data;
        SD_CFG->adpt_cfg.Gold_csv_H = ext_cfg->ff_mse_high->data;
        SD_CFG->adpt_cfg.Gold_csv_M = ext_cfg->ff_mse_medium->data;
        SD_CFG->adpt_cfg.Gold_csv_L = ext_cfg->ff_mse_low->data;

        SD_CFG->adpt_cfg.total_gain_adj_begin = ext_cfg->ff_iir_general->total_gain_freq_l;
        SD_CFG->adpt_cfg.total_gain_adj_end = ext_cfg->ff_iir_general->total_gain_freq_h;
        SD_CFG->adpt_cfg.gain_limit_all = ext_cfg->ff_iir_general->total_gain_limit;

        // 耳道记忆曲线配置
        SD_CFG->adpt_cfg.mem_curve_nums = ext_cfg->ff_ear_mem_param->mem_curve_nums;
        SD_CFG->adpt_cfg.sz_table = ext_cfg->ff_ear_mem_sz->data;
        SD_CFG->adpt_cfg.pz_table = ext_cfg->ff_ear_mem_pz->data;
        SD_CFG->adpt_cfg.pz_table_cmp = NULL;
        SD_CFG->adpt_cfg.sz_table_cmp = NULL;
#if EXT_PRINTF_DEBUG
        for (int i = 0; i < 60; i++) {
            printf("idx=%d, mse=%d, weight=%d\n", i, (int)SD_CFG->adpt_cfg.Gold_csv_H[i], (int)SD_CFG->adpt_cfg.Weight_H[i]);
        }

        de_vrange_printf(SD_CFG->adpt_cfg.Vrange_H, SD_CFG->adpt_cfg.IIR_NUM_FLEX + SD_CFG->adpt_cfg.IIR_NUM_FIX);
        de_vrange_printf(SD_CFG->adpt_cfg.Vrange_M, SD_CFG->adpt_cfg.IIR_NUM_FLEX + SD_CFG->adpt_cfg.IIR_NUM_FIX);
        de_vrange_printf(SD_CFG->adpt_cfg.Vrange_L, SD_CFG->adpt_cfg.IIR_NUM_FLEX + SD_CFG->adpt_cfg.IIR_NUM_FIX);

        de_biquad_printf(SD_CFG->adpt_cfg.Biquad_init_H, SD_CFG->adpt_cfg.IIR_NUM_FLEX + SD_CFG->adpt_cfg.IIR_NUM_FIX);
        de_biquad_printf(SD_CFG->adpt_cfg.Biquad_init_M, SD_CFG->adpt_cfg.IIR_NUM_FLEX + SD_CFG->adpt_cfg.IIR_NUM_FIX);
        de_biquad_printf(SD_CFG->adpt_cfg.Biquad_init_L, SD_CFG->adpt_cfg.IIR_NUM_FLEX + SD_CFG->adpt_cfg.IIR_NUM_FIX);
#endif

#if TCFG_AUDIO_ANC_CH == (ANC_L_CH | ANC_R_CH)
        /***************************************************/
        /*************** right channel config **************/
        /***************************************************/
        SD_CFG->adpt_cfg_r.pnc_times = ext_cfg->base_cfg->adaptive_times;
        // target配置
        SD_CFG->adpt_cfg_r.cmp_en = ext_cfg->rff_target_param->cmp_en;
        SD_CFG->adpt_cfg_r.target_cmp_num = ext_cfg->rff_target_param->cmp_curve_num;
        SD_CFG->adpt_cfg_r.target_sv = ext_cfg->rff_target_sv->data;
        SD_CFG->adpt_cfg_r.target_cmp_dat = ext_cfg->rff_target_cmp->data;

        // 算法配置
        SD_CFG->adpt_cfg_r.IIR_NUM_FLEX = 0;
        flex_idx = 0;
        biquad_idx = 0;
        for (int i = 0; i < SD_CFG->ff_yorder; i++) {
            if (ext_cfg->rff_iir_high[i].fixed_en) { // fix
                SD_CFG->adpt_cfg_r.Biquad_init_H[3 * biquad_idx + 0] = ext_cfg->rff_iir_high[i].def.gain;
                SD_CFG->adpt_cfg_r.Biquad_init_H[3 * biquad_idx + 1] = ext_cfg->rff_iir_high[i].def.fre;
                SD_CFG->adpt_cfg_r.Biquad_init_H[3 * biquad_idx + 2] = ext_cfg->rff_iir_high[i].def.q;
                SD_CFG->adpt_cfg_r.Biquad_init_M[3 * biquad_idx + 0] = ext_cfg->rff_iir_medium[i].def.gain;
                SD_CFG->adpt_cfg_r.Biquad_init_M[3 * biquad_idx + 1] = ext_cfg->rff_iir_medium[i].def.fre;
                SD_CFG->adpt_cfg_r.Biquad_init_M[3 * biquad_idx + 2] = ext_cfg->rff_iir_medium[i].def.q;
                SD_CFG->adpt_cfg_r.Biquad_init_L[3 * biquad_idx + 0] = ext_cfg->rff_iir_low[i].def.gain;
                SD_CFG->adpt_cfg_r.Biquad_init_L[3 * biquad_idx + 1] = ext_cfg->rff_iir_low[i].def.fre;
                SD_CFG->adpt_cfg_r.Biquad_init_L[3 * biquad_idx + 2] = ext_cfg->rff_iir_low[i].def.q;
                SD_CFG->adpt_cfg_r.biquad_type[biquad_idx] = ext_cfg->rff_iir_high[i].type;
                biquad_idx += 1;
            } else { // not fix
                SD_CFG->adpt_cfg_r.Vrange_H[6 * flex_idx + 0] = ext_cfg->rff_iir_high[i].lower_limit.gain;
                SD_CFG->adpt_cfg_r.Vrange_H[6 * flex_idx + 1] = ext_cfg->rff_iir_high[i].upper_limit.gain;
                SD_CFG->adpt_cfg_r.Vrange_H[6 * flex_idx + 2] = ext_cfg->rff_iir_high[i].lower_limit.fre;
                SD_CFG->adpt_cfg_r.Vrange_H[6 * flex_idx + 3] = ext_cfg->rff_iir_high[i].upper_limit.fre;
                SD_CFG->adpt_cfg_r.Vrange_H[6 * flex_idx + 4] = ext_cfg->rff_iir_high[i].lower_limit.q;
                SD_CFG->adpt_cfg_r.Vrange_H[6 * flex_idx + 5] = ext_cfg->rff_iir_high[i].upper_limit.q;
                SD_CFG->adpt_cfg_r.Vrange_M[6 * flex_idx + 0] = ext_cfg->rff_iir_medium[i].lower_limit.gain;
                SD_CFG->adpt_cfg_r.Vrange_M[6 * flex_idx + 1] = ext_cfg->rff_iir_medium[i].upper_limit.gain;
                SD_CFG->adpt_cfg_r.Vrange_M[6 * flex_idx + 2] = ext_cfg->rff_iir_medium[i].lower_limit.fre;
                SD_CFG->adpt_cfg_r.Vrange_M[6 * flex_idx + 3] = ext_cfg->rff_iir_medium[i].upper_limit.fre;
                SD_CFG->adpt_cfg_r.Vrange_M[6 * flex_idx + 4] = ext_cfg->rff_iir_medium[i].lower_limit.q;
                SD_CFG->adpt_cfg_r.Vrange_M[6 * flex_idx + 5] = ext_cfg->rff_iir_medium[i].upper_limit.q;
                SD_CFG->adpt_cfg_r.Vrange_L[6 * flex_idx + 0] = ext_cfg->rff_iir_low[i].lower_limit.gain;
                SD_CFG->adpt_cfg_r.Vrange_L[6 * flex_idx + 1] = ext_cfg->rff_iir_low[i].upper_limit.gain;
                SD_CFG->adpt_cfg_r.Vrange_L[6 * flex_idx + 2] = ext_cfg->rff_iir_low[i].lower_limit.fre;
                SD_CFG->adpt_cfg_r.Vrange_L[6 * flex_idx + 3] = ext_cfg->rff_iir_low[i].upper_limit.fre;
                SD_CFG->adpt_cfg_r.Vrange_L[6 * flex_idx + 4] = ext_cfg->rff_iir_low[i].lower_limit.q;
                SD_CFG->adpt_cfg_r.Vrange_L[6 * flex_idx + 5] = ext_cfg->rff_iir_low[i].upper_limit.q;
                flex_idx += 1;
            }
        }

        SD_CFG->adpt_cfg_r.Vrange_H[flex_idx * 6 + 0] = -ext_cfg->rff_iir_high_gains->lower_limit_gain;
        SD_CFG->adpt_cfg_r.Vrange_H[flex_idx * 6 + 1] = -ext_cfg->rff_iir_high_gains->upper_limit_gain;
        SD_CFG->adpt_cfg_r.Vrange_M[flex_idx * 6 + 0] = -ext_cfg->rff_iir_medium_gains->lower_limit_gain;
        SD_CFG->adpt_cfg_r.Vrange_M[flex_idx * 6 + 1] = -ext_cfg->rff_iir_medium_gains->upper_limit_gain;
        SD_CFG->adpt_cfg_r.Vrange_L[flex_idx * 6 + 0] = -ext_cfg->rff_iir_low_gains->lower_limit_gain;
        SD_CFG->adpt_cfg_r.Vrange_L[flex_idx * 6 + 1] = -ext_cfg->rff_iir_low_gains->upper_limit_gain;

        SD_CFG->adpt_cfg_r.IIR_NUM_FLEX = flex_idx;
        SD_CFG->adpt_cfg_r.IIR_NUM_FIX = biquad_idx;

        for (int i = 0; i < SD_CFG->ff_yorder; i++) {
            if (!ext_cfg->rff_iir_high[i].fixed_en) { // not fix
                SD_CFG->adpt_cfg_r.Biquad_init_H[3 * biquad_idx + 0] = ext_cfg->rff_iir_high[i].def.gain;
                SD_CFG->adpt_cfg_r.Biquad_init_H[3 * biquad_idx + 1] = ext_cfg->rff_iir_high[i].def.fre;
                SD_CFG->adpt_cfg_r.Biquad_init_H[3 * biquad_idx + 2] = ext_cfg->rff_iir_high[i].def.q;
                SD_CFG->adpt_cfg_r.Biquad_init_M[3 * biquad_idx + 0] = ext_cfg->rff_iir_medium[i].def.gain;
                SD_CFG->adpt_cfg_r.Biquad_init_M[3 * biquad_idx + 1] = ext_cfg->rff_iir_medium[i].def.fre;
                SD_CFG->adpt_cfg_r.Biquad_init_M[3 * biquad_idx + 2] = ext_cfg->rff_iir_medium[i].def.q;
                SD_CFG->adpt_cfg_r.Biquad_init_L[3 * biquad_idx + 0] = ext_cfg->rff_iir_low[i].def.gain;
                SD_CFG->adpt_cfg_r.Biquad_init_L[3 * biquad_idx + 1] = ext_cfg->rff_iir_low[i].def.fre;
                SD_CFG->adpt_cfg_r.Biquad_init_L[3 * biquad_idx + 2] = ext_cfg->rff_iir_low[i].def.q;
                SD_CFG->adpt_cfg_r.biquad_type[biquad_idx] = ext_cfg->rff_iir_high[i].type;
                biquad_idx += 1;
            }
        }

        SD_CFG->adpt_cfg_r.Biquad_init_H[biquad_idx * 3] = -ext_cfg->rff_iir_high_gains->def_total_gain;
        SD_CFG->adpt_cfg_r.Biquad_init_M[biquad_idx * 3] = -ext_cfg->rff_iir_medium_gains->def_total_gain;
        SD_CFG->adpt_cfg_r.Biquad_init_L[biquad_idx * 3] = -ext_cfg->rff_iir_low_gains->def_total_gain;

        for (int i = 0; i < 7; i++) {
            SD_CFG->adpt_cfg_r.degree_set0[i] = degree_set0[i];
            SD_CFG->adpt_cfg_r.degree_set1[i] = degree_set1[i];
            SD_CFG->adpt_cfg_r.degree_set2[i] = degree_set2[i];
        }

        SD_CFG->adpt_cfg_r.degree_set0[0] = ext_cfg->rff_iir_general->biquad_level_l;
        SD_CFG->adpt_cfg_r.degree_set1[0] = ext_cfg->rff_iir_general->biquad_level_h;
        SD_CFG->adpt_cfg_r.degree_set2[0] = ext_cfg->rff_target_param->cmp_curve_num;
        SD_CFG->adpt_cfg_r.degree_set0[6] = ext_cfg->rff_iir_general->biquad_level_l;
        SD_CFG->adpt_cfg_r.degree_set1[6] = ext_cfg->rff_iir_general->biquad_level_h;
        SD_CFG->adpt_cfg_r.degree_set2[6] = ext_cfg->rff_target_param->cmp_curve_num;

        SD_CFG->adpt_cfg_r.degree_set0[1] = ext_cfg->rff_iir_high_gains->iir_target_gain_limit;
        SD_CFG->adpt_cfg_r.degree_set1[1] = ext_cfg->rff_iir_medium_gains->iir_target_gain_limit;
        SD_CFG->adpt_cfg_r.degree_set2[1] = ext_cfg->rff_iir_low_gains->iir_target_gain_limit;

        SD_CFG->adpt_cfg_r.Weight_H = ext_cfg->rff_weight_high->data;
        SD_CFG->adpt_cfg_r.Weight_M = ext_cfg->rff_weight_medium->data;
        SD_CFG->adpt_cfg_r.Weight_L = ext_cfg->rff_weight_low->data;
        SD_CFG->adpt_cfg_r.Gold_csv_H = ext_cfg->rff_mse_high->data;
        SD_CFG->adpt_cfg_r.Gold_csv_M = ext_cfg->rff_mse_medium->data;
        SD_CFG->adpt_cfg_r.Gold_csv_L = ext_cfg->rff_mse_low->data;

        SD_CFG->adpt_cfg_r.total_gain_adj_begin = ext_cfg->rff_iir_general->total_gain_freq_l;
        SD_CFG->adpt_cfg_r.total_gain_adj_end = ext_cfg->rff_iir_general->total_gain_freq_h;
        SD_CFG->adpt_cfg_r.gain_limit_all = ext_cfg->rff_iir_general->total_gain_limit;

        // 耳道记忆曲线配置
        SD_CFG->adpt_cfg_r.mem_curve_nums = ext_cfg->rff_ear_mem_param->mem_curve_nums;
        SD_CFG->adpt_cfg_r.sz_table = ext_cfg->rff_ear_mem_sz->data;
        SD_CFG->adpt_cfg_r.pz_table = ext_cfg->rff_ear_mem_pz->data;
        SD_CFG->adpt_cfg_r.pz_table_cmp = NULL;
        SD_CFG->adpt_cfg_r.sz_table_cmp = NULL;
#if EXT_PRINTF_DEBUG
        for (int i = 0; i < 60; i++) {
            printf("idx=%d, mse=%d, weight=%d\n", i, (int)SD_CFG->adpt_cfg.Gold_csv_H[i], (int)SD_CFG->adpt_cfg.Weight_H[i]);
        }

        de_vrange_printf(SD_CFG->adpt_cfg.Vrange_H, SD_CFG->adpt_cfg.IIR_NUM_FLEX + SD_CFG->adpt_cfg.IIR_NUM_FIX);
        de_vrange_printf(SD_CFG->adpt_cfg.Vrange_M, SD_CFG->adpt_cfg.IIR_NUM_FLEX + SD_CFG->adpt_cfg.IIR_NUM_FIX);
        de_vrange_printf(SD_CFG->adpt_cfg.Vrange_L, SD_CFG->adpt_cfg.IIR_NUM_FLEX + SD_CFG->adpt_cfg.IIR_NUM_FIX);

        de_biquad_printf(SD_CFG->adpt_cfg.Biquad_init_H, SD_CFG->adpt_cfg.IIR_NUM_FLEX + SD_CFG->adpt_cfg.IIR_NUM_FIX);
        de_biquad_printf(SD_CFG->adpt_cfg.Biquad_init_M, SD_CFG->adpt_cfg.IIR_NUM_FLEX + SD_CFG->adpt_cfg.IIR_NUM_FIX);
        de_biquad_printf(SD_CFG->adpt_cfg.Biquad_init_L, SD_CFG->adpt_cfg.IIR_NUM_FLEX + SD_CFG->adpt_cfg.IIR_NUM_FIX);
#endif

#endif
    }
}

#endif
