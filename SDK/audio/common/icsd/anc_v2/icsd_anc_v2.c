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

#include "icsd_anc_v2_app.h"
#if TCFG_USER_TWS_ENABLE
#include "classic/tws_api.h"
#endif
#include "board_config.h"
#include "tone_player.h"
#include "adv_adaptive_noise_reduction.h"
#include "audio_anc.h"
#include "asm/audio_src.h"
#include "clock_manager/clock_manager.h"
#include "anc_ext_tool.h"
#include "audio_anc_debug_tool.h"

#define EXT_PRINTF_DEBUG 0 //前期调试工具使用，后期上传该宏定义相关删除
u8 const ICSD_ANC_TOOL_DEBUG = 0;
OS_SEM icsd_anc_sem;


const u8 EAR_ADAPTIVE_MODE_SIGN_TRIM_VEL =  EAR_ADAPTIVE_MODE_SIGN_TRIM;
struct anc_function	ANC_FUNC;
__icsd_anc_REG *ICSD_REG = NULL;

static void icsd_anc_v2_cmd(u8 cmd);

int (*anc_v2_printf)(const char *format, ...);
int anc_v2_printf_off(const char *format, ...)
{
    return 0;
}
//============================================================================

#if TCFG_USER_TWS_ENABLE

#define TWS_FUNC_ID_SDANCV2_M2S    TWS_FUNC_ID('I', 'C', 'M', 'S')
REGISTER_TWS_FUNC_STUB(icsd_anc_v2_m2s) = {
    .func_id = TWS_FUNC_ID_SDANCV2_M2S,
    .func    = icsd_anc_v2_m2s_cb,
};
#define TWS_FUNC_ID_SDANCV2_S2M    TWS_FUNC_ID('I', 'C', 'S', 'M')
REGISTER_TWS_FUNC_STUB(icsd_anc_v2_s2m) = {
    .func_id = TWS_FUNC_ID_SDANCV2_S2M,
    .func    = icsd_anc_v2_s2m_cb,
};
#define TWS_FUNC_ID_SDANCV2_MSYNC  TWS_FUNC_ID('I', 'C', 'M', 'N')
REGISTER_TWS_FUNC_STUB(icsd_anc_v2_msync) = {
    .func_id = TWS_FUNC_ID_SDANCV2_MSYNC,
    .func    = icsd_anc_v2_msync_cb,
};
#define TWS_FUNC_ID_SDANCV2_SSYNC  TWS_FUNC_ID('I', 'C', 'S', 'N')
REGISTER_TWS_FUNC_STUB(icsd_anc_v2_ssync) = {
    .func_id = TWS_FUNC_ID_SDANCV2_SSYNC,
    .func    = icsd_anc_v2_ssync_cb,
};

void icsd_anc_v2_tws_msync(u8 cmd)
{
    struct icsd_anc_v2_tws_packet packet;
    icsd_anc_v2_msync_packet(&packet, cmd);
    int ret = tws_api_send_data_to_sibling(packet.data, packet.len, TWS_FUNC_ID_SDANCV2_MSYNC);
}

void icsd_anc_v2_tws_ssync(u8 cmd)
{
    struct icsd_anc_v2_tws_packet packet;
    icsd_anc_v2_ssync_packet(&packet, cmd);
    int ret = tws_api_send_data_to_sibling(packet.data, packet.len, TWS_FUNC_ID_SDANCV2_SSYNC);
}

void icsd_anc_v2_tws_m2s(u8 cmd)
{
    s8 data[16];
    icsd_anc_v2_m2s_packet(data, cmd);
    int ret = tws_api_send_data_to_sibling(data, 16, TWS_FUNC_ID_SDANCV2_M2S);
}

void icsd_anc_v2_tws_s2m(u8 cmd)
{
    s8 data[16];
    icsd_anc_v2_s2m_packet(data, cmd);
    int ret = tws_api_send_data_to_sibling(data, 16, TWS_FUNC_ID_SDANCV2_S2M);
}

int icsd_anc_v2_get_tws_state()
{
    if (ICSD_REG->icsd_anc_v2_tws_balance_en) {
        return tws_api_get_tws_state();
    }
    return 0;
}

#else
int icsd_anc_v2_get_tws_state()
{
    return 0;
}

#endif

#if EXT_PRINTF_DEBUG
static void de_vrange_printf(float *vrange, int order)
{
    printf(" ff oreder = %d\n", order);
    for (int i = 0; i < order; i++) {
        printf("%d >>>>>>>>>>> g:%d, %d, f:%d, %d, q:%d, %d\n", i, (int)(vrange[6 * i + 0] * 1000), (int)(vrange[6 * i + 1] * 1000)
               , (int)(vrange[6 * i + 2] * 1000), (int)(vrange[6 * i + 3] * 1000)
               , (int)(vrange[6 * i + 4] * 1000), (int)(vrange[6 * i + 5] * 1000));
    }
}

static void de_biquad_printf(float *biquad, int order)
{
    printf(" ff oreder = %d\n", order);
    for (int i = 0; i < order; i++) {
        printf("%d >>>>> g:%d, f:%d, q:%d\n", i, (int)(biquad[3 * i + 0] * 1000), (int)(biquad[3 * i + 1]), (int)(biquad[3 * i + 2] * 1000));
    }
    printf("total gain = %d\n", (int)(biquad[order * 3] * 1000));
}
#endif

static void icsd_anc_v2_time_data_init(u8 *buf)
{
    if (ICSD_REG) {
        if (ICSD_REG->anc_time_data == NULL) {
            ICSD_REG->anc_time_data = (__icsd_time_data *)buf;
        }
    }
}

static void icsd_anc_config_data_init(void *config_ptr, struct icsd_anc_v2_infmt *fmt, struct anc_ext_ear_adaptive_param *ext_cfg)
{
    if (ext_cfg->time_domain_show_en) {
        icsd_anc_v2_time_data_init(ext_cfg->time_domain_buf);
    }

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
        SD_CFG->tool_ffgain_sign = fmt->tool_ffgain_sign;
        SD_CFG->tool_fbgain_sign = fmt->tool_fbgain_sign;
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
        if (ext_cfg->ff_dut_pz_cmp) {
            SD_CFG->adpt_cfg.pz_table_cmp = ext_cfg->ff_dut_pz_cmp->data;
        } else {
            SD_CFG->adpt_cfg.pz_table_cmp = NULL;
        }
        if (ext_cfg->ff_dut_sz_cmp) {
            SD_CFG->adpt_cfg.sz_table_cmp = ext_cfg->ff_dut_sz_cmp->data;
        } else {
            SD_CFG->adpt_cfg.sz_table_cmp = NULL;
        }
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
        if (ext_cfg->rff_dut_pz_cmp) {
            SD_CFG->adpt_cfg_r.pz_table_cmp = ext_cfg->rff_dut_pz_cmp->data;
        } else {
            SD_CFG->adpt_cfg_r.pz_table_cmp = NULL;
        }
        if (ext_cfg->rff_dut_sz_cmp) {
            SD_CFG->adpt_cfg_r.sz_table_cmp = ext_cfg->rff_dut_sz_cmp->data;
        } else {
            SD_CFG->adpt_cfg_r.sz_table_cmp = NULL;
        }
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

static void icsd_anc_v2_open(void *_param, struct anc_ext_ear_adaptive_param *ext_cfg)
{
    audio_anc_t *param = (audio_anc_t *)_param;
    anc_v2_printf("icsd_anc_open\n");
    struct icsd_anc_v2_libfmt libfmt;
    icsd_anc_v2_get_libfmt(&libfmt);
    if (ICSD_REG->anc_v2_ram_addr == 0) {
        ICSD_REG->anc_v2_ram_addr = zalloc(libfmt.lib_alloc_size);
        printf("anc_v2_ram_addr:%d>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", (int)ICSD_REG->anc_v2_ram_addr);
    }
    struct icsd_anc_v2_infmt infmt = {0};
    infmt.alloc_ptr = ICSD_REG->anc_v2_ram_addr;
    infmt.ff_gain = 3;
    infmt.fb_gain = icsd_anc_v2_readfbgain();
    infmt.tool_ffgain_sign = 1;
    infmt.tool_fbgain_sign = 1;
    anc_adaptive_iir_t *anc_iir = (anc_adaptive_iir_t *)anc_ear_adaptive_iir_get();
#if (TCFG_AUDIO_ANC_CH & ANC_L_CH)
    if (anc_iir->result & ANC_ADAPTIVE_RESULT_LFF) {
        ICSD_REG->lff_fgq_last = (float *)zalloc(((ANC_MAX_ORDER * 3) + 1) * sizeof(float));
        ICSD_REG->lff_fgq_last[0] = anc_iir->lff_gain;
        for (int i = 0; i < ANC_ADAPTIVE_FF_ORDER; i++) {
            memcpy((u8 *)(ICSD_REG->lff_fgq_last + 1 + (i * 3)), (u8 *)(anc_iir->lff[i].a), 12);
        }
        infmt.ff_fgq_l = ICSD_REG->lff_fgq_last;
        infmt.target_out_l = anc_iir->l_target;
    }
#endif
#if (TCFG_AUDIO_ANC_CH & ANC_R_CH)
    if (anc_iir->result & ANC_ADAPTIVE_RESULT_RFF) {
        ICSD_REG->rff_fgq_last = (float *)zalloc(((ANC_MAX_ORDER * 3) + 1) * sizeof(float));
        for (int i = 0; i < ANC_ADAPTIVE_FF_ORDER; i++) {
            memcpy((u8 *)(ICSD_REG->rff_fgq_last + 1 + (i * 3)), (u8 *)(anc_iir->rff[i].a), 12);
        }
        infmt.ff_fgq_r = ICSD_REG->rff_fgq_last;
        infmt.target_out_r = anc_iir->r_target;
    }
#endif
    if (param->gains.gain_sign & ANCL_FF_SIGN) {
        infmt.tool_ffgain_sign = -1;
    }
    if (param->gains.gain_sign & ANCL_FB_SIGN) {
        infmt.tool_fbgain_sign = -1;
    }
    icsd_anc_v2_set_infmt(&infmt);
    icsd_anc_config_data_init(NULL, &infmt, ext_cfg);
    anc_v2_printf("ram init: %d\n", libfmt.lib_alloc_size);
    ICSD_REG->v2_user_train_state = 0;
    ICSD_REG->icsd_anc_v2_function = 0;
}



void icsd_anc_v2_end(void *_param)
{
    clock_free("ANC_ADAP");
    audio_anc_t *param = _param;
    mem_stats();
    if (ICSD_REG) {
        if (ICSD_REG->lff_fgq_last) {
            free(ICSD_REG->lff_fgq_last);
            ICSD_REG->lff_fgq_last = NULL;
        }
        if (ICSD_REG->rff_fgq_last) {
            free(ICSD_REG->rff_fgq_last);
            ICSD_REG->rff_fgq_last = NULL;
        }
        ICSD_REG->v2_user_train_state &= ~ANC_USER_TRAIN_DMA_EN;
        ICSD_REG->icsd_anc_v2_contral = 0;
        anc_v2_printf("ICSD ANC V2 END train time finish:%dms\n", (int)((jiffies - ICSD_REG->train_time_v2) * 10));
    }
    anc_user_train_cb(ANC_ON,  0);
}

void icsd_anc_v2_forced_exit()
{
    if (ICSD_REG) {
        if (!(ICSD_REG->icsd_anc_v2_contral & ICSD_ANC_FORCED_EXIT)) {
            anc_v2_printf("icsd_anc_forced_exit\n");
            DeAlorithm_disable();
            if (ICSD_REG->icsd_anc_v2_contral & ICSD_ANC_INITED) {
                ICSD_REG->icsd_anc_v2_contral |= ICSD_ANC_FORCED_EXIT;
                ANC_FUNC.icsd_anc_v2_cmd(ANC_V2_CMD_FORCED_EXIT);
            } else {
                ICSD_REG->icsd_anc_v2_contral |= ICSD_ANC_FORCED_BEFORE_INIT;
            }
        }
    }
}

u16 icsd_anc_timeout_add(void *priv, void (*func)(void *priv), u32 msec)
{
    return sys_hi_timeout_add(priv, func, msec);
}

static void anc_function_init()
{
    printf("anc_function_init\n");
    ANC_FUNC.anc_dma_done_ppflag = anc_dma_done_ppflag;
    ANC_FUNC.local_irq_disable = local_irq_disable;
    ANC_FUNC.local_irq_enable = local_irq_enable;
    ANC_FUNC.jiffies_usec = jiffies_usec;
    ANC_FUNC.jiffies_usec2offset = jiffies_usec2offset;
    ANC_FUNC.audio_anc_debug_send_data = audio_anc_debug_send_data;
    ANC_FUNC.audio_anc_debug_busy_get = audio_anc_debug_busy_get;

    ANC_FUNC.icsd_anc_v2_get_tws_state = icsd_anc_v2_get_tws_state;
#if TCFG_USER_TWS_ENABLE
    ANC_FUNC.tws_api_get_role = tws_api_get_role;
    ANC_FUNC.icsd_anc_v2_tws_m2s = icsd_anc_v2_tws_m2s;
    ANC_FUNC.icsd_anc_v2_tws_s2m = icsd_anc_v2_tws_s2m;
    ANC_FUNC.icsd_anc_v2_tws_msync = icsd_anc_v2_tws_msync;
    ANC_FUNC.icsd_anc_v2_tws_ssync = icsd_anc_v2_tws_ssync;
#endif

    ANC_FUNC.icsd_anc_v2_cmd = icsd_anc_v2_cmd;
    ANC_FUNC.icsd_anc_timeout_add = icsd_anc_timeout_add;
    ANC_FUNC.icsd_anc_v2_dma_2ch_on = icsd_anc_v2_dma_2ch_on;
    ANC_FUNC.icsd_anc_v2_dma_4ch_on = icsd_anc_v2_dma_4ch_on;
    ANC_FUNC.icsd_anc_v2_end = icsd_anc_v2_end;
    ANC_FUNC.icsd_anc_v2_output = icsd_anc_v2_output;
    ANC_FUNC.icsd_anc_sz_output = anc_ear_adaptive_sz_output;
}

void icsd_anc_v2_init(void *_hdl, struct anc_ext_ear_adaptive_param *ext_cfg)
{
    printf("icsd_anc_v2_init\n");
    if (ICSD_REG == NULL) {
        printf("ICSD REG malloc\n");
        ICSD_REG = zalloc(sizeof(__icsd_anc_REG));
    }
    anc_function_init();

    struct icsd_anc_init_cfg *hdl = (struct icsd_anc_init_cfg *)_hdl;
    audio_anc_t *param = hdl->param;

    anc_v2_printf = _anc_v2_printf;
    ICSD_REG->icsd_anc_v2_contral |= ICSD_ANC_INITED;
    if (ICSD_REG->icsd_anc_v2_contral & ICSD_ANC_FORCED_BEFORE_INIT) {
        ICSD_REG->icsd_anc_v2_contral &= ~ICSD_ANC_FORCED_BEFORE_INIT;
        icsd_anc_v2_forced_exit();
    }
    if (ICSD_REG->icsd_anc_v2_contral & ICSD_ANC_FORCED_EXIT) {
        os_sem_set(&icsd_anc_sem, 0);
        os_sem_post(&icsd_anc_sem);
        return;
    }

    ICSD_REG->icsd_anc_v2_id = hdl->seq;
    ICSD_REG->icsd_anc_v2_tws_balance_en = hdl->tws_sync;
    anc_v2_printf("ICSD ANC v2 INIT: id %d, seq %d, tws_balance %d \n", ICSD_REG->icsd_anc_v2_id, hdl->seq, hdl->tws_sync);
    mem_stats();
    icsd_anc_v2_open(param, ext_cfg);

#if ANC_USER_TRAIN_TONE_MODE
    ICSD_REG->v2_user_train_state |=	ANC_USER_TRAIN_TONEMODE;
#endif

#if ANC_EARPHONE_CHECK_EN
    ICSD_REG->icsd_anc_v2_function |= ANC_EARPHONE_CHECK;
#endif
    ICSD_REG->v2_user_train_state |=	ANC_USER_TRAIN_DMA_EN;

    //金机曲线功能==================================
    struct icsd_fb_ref *fb_parm = NULL;
    struct icsd_ff_ref *ff_parm = NULL;
    if (param->gains.adaptive_ref_en) {
        anc_adaptive_fb_ref_data_get((u8 **)(&fb_parm));
        anc_adaptive_ff_ref_data_get((u8 **)(&ff_parm));
    }
    //==============================================
    icsd_set_adaptive_run_busy(1);
    icsd_anc_v2_mode_init();
    os_sem_set(&icsd_anc_sem, 0);
    os_sem_post(&icsd_anc_sem);
}


void icsd_anc_v2_cmd(u8 cmd)
{
    os_taskq_post_msg("anc", 2, ANC_MSG_ICSD_ANC_V2_CMD, cmd);
}
#endif/*TCFG_AUDIO_ANC_EAR_ADAPTIVE_EN*/
