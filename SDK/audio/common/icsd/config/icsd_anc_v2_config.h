#ifndef _ANC_V2_CONFIG_H
#define _ANC_V2_CONFIG_H

extern const u8 ICSD_EP_TYPE_V2;
extern const u8 ICSD_ANC_TOOL_PRINTF;
extern const int FF_objFunc_type;
extern const float target_cmp_dat[];
extern const u8 ICSD_ANC_V2_BYPASS_ON_FIRST;

extern const float gfq_bypass[];
extern const u8 tap_bypass;
extern const u8 type_bypass[];
extern const float fs_bypass;
extern double bbbaa_bypass[1 * 5];

//
extern const float fgq_init_table[10 * 3];
extern const float fgq_init_table_fb[10 * 3];
extern const float fgq_init_table2[10 * 3];
extern const float tg_cmp[];
extern const float errweight[];
extern const float ref_spl_thr[];
extern const float err_spl_thr[];


//
extern const int cmp_idx_begin;
extern const int cmp_idx_end;
extern const int cmp_total_len;
extern const int cmp_en;
extern const float pz_gain;
extern const float target_sv[];
extern const float tg_cmp[];


extern const float Gold_csv_Perf_Range[];
extern float degree_set0[];
extern float degree_set1[];
extern float degree_set2[];
extern const float FSTOP_IDX;
extern const float FSTOP_IDX2;

//
extern const float cmp_Vrange_tmp_m[];
extern const float cmp_biquad_init_tmp_m[];
extern const float cmp_weight_m[];
extern const float cmp_mse_tar_m[];
//extern const int cmp_iir_type[];
extern const int ICSD_CMP_IIR_NUM_FLEX;
extern const int ICSD_CMP_IIR_NUM_FIX;
extern const int ICSD_CMP_IIR_COEF;
extern const float CMP_FSTOP_IDX;
extern const float CMP_FSTOP_IDX2;
extern const int   CMP_objFunc_type;
extern const u8 msedif_en;
extern const u8 target_diff_en;
//
extern const float pz_table[];
extern const float sz_table[];
extern const float left_pz_table[];
extern const float left_sz_table[];
extern const float right_pz_table[];
extern const float right_sz_table[];


struct icsd_anc_v2_tool_data {
    int h_len;
    int yorderb;//int fb_yorder;
    int yorderf;//int ff_yorder;
    int yorderc;//int cmp_yorder;
    float *h_freq;
    float *data_out1;//float *hszpz_out_l;
    float *data_out2;//float *hpz_out_l;
    float *data_out3;//float *htarget_out_l;
    float *data_out4;//float *fb_fgq_l;
    float *data_out5;//float *ff_fgq_l;
    float *data_out6;//float *hszpz_out_r;
    float *data_out7;//float *hpz_out_r;
    float *data_out8;//float *htarget_out_r;
    float *data_out9;//float *fb_fgq_r;,
    float *data_out10;//float *ff_fgq_r;
    float *data_out11;//float *cmp_fgq_l;
    float *data_out12;//float *cmp_fgq_r;
    float *data_out13;//float *tool_target_out_l;
    float *data_out14;//float *tool_target_out_r;
    float *wz_temp;

    float *target_before_cmp_l;
    float *target_before_cmp_r;
    float *cmp_form_anc_train_l;
    float *cmp_form_anc_train_r;

    u8 *lff_iir_type;
    u8 *lfb_iir_type;
    u8 *lcmp_iir_type;
    u8 *rff_iir_type;
    u8 *rfb_iir_type;
    u8 *rcmp_iir_type;
    s8 calr_sign[4];// 顺序 sz_calr_sign、pz_calr_sign、bypass_calr_sign、perf_calr_sign
    u8 result_l;   // 左耳结果
    u8 result_r;   // 右耳结果
    u8 anc_combination;//FF,FB,CMP

};


// anc tool buffer
typedef struct {
    // target配置
    int   cmp_en;
    int   target_cmp_num;
    u8    pnc_times;

    float  pz_gain;
    float *target_sv;
    float *target_cmp_dat;

    // 算法配置
    u8    biquad_type[10];
    u8    fb_biquad_type[10];
    u8    cmp_biquad_type[10];
    float Vrange_H[62];
    float Vrange_M[62];
    float Vrange_L[62];
    float Biquad_init_H[31];
    float Biquad_init_M[31];
    float Biquad_init_L[31];
    float degree_set0[7];
    float degree_set1[7];
    float degree_set2[7];

    float *Weight_H;
    float *Weight_M;
    float *Weight_L;
    float *Gold_csv_H;
    float *Gold_csv_M;
    float *Gold_csv_L;

    float total_gain_adj_begin;
    float total_gain_adj_end;
    float gain_limit_all;
    int   IIR_NUM_FLEX;
    int   IIR_NUM_FIX;

    // 耳道记忆曲线配置
    u8    mem_curve_nums;
    float *pz_table;
    float *sz_table;
    float *pz_table_cmp;
    float *sz_table_cmp;
} adpt_anc_cfg;


typedef struct {
    //工具界面
    u16	  tonel_delay;
    u16	  toner_delay;
    u16	  pzl_delay;
    u16	  pzr_delay;
    u8    train_mode;

    s8	  sz_calr_sign;
    s8	  pz_calr_sign;
    s8    bypass_calr_sign;
    s8    perf_calr_sign;

    float bypass_vol;
    float sz_pri_thr;
    s8	  vld1;
    s8    vld2;
    u8    ear_recorder;
    u8    pnc_times;
    //其他配置
    s8 	  tool_ffgain_sign;
    s8 	  tool_fbgain_sign;
    u8 	  ff_yorder;
    u8    fb_yorder;
    u8    normal_out_sel_l;
    u8    normal_out_sel_r;
    u8    tone_out_sel_l;
    u8    tone_out_sel_r;
    u8    fb_agc_en;

    float debug[10];
    adpt_anc_cfg adpt_cfg;
    adpt_anc_cfg adpt_cfg_r;
} __icsd_anc_config_data;
extern __icsd_anc_config_data	*SD_CFG;       // ANC
extern __icsd_anc_config_data	*RTANC_SD_CFG;


void icsd_sd_cfg_set(__icsd_anc_config_data *SD_CFG, void *_ext_cfg);


enum {
    TFF_TFB = 0,
    TFF_DFB,
    DFF_TFB,
    DFF_DFB,
};

void anc_buffer_init(float *freq, float fs, float flen, __icsd_pnc_cmp *_pnc_cmp, struct icsd_ff_candidate_v2 *FF_CANDI_V2, struct icsd_ff_candidate_v2 *CMP_CANDI_V2, struct icsd_De_param_v2 *DE_PARAM_V2, struct icsd_target_param *tar_param, adpt_anc_cfg *adpt_cfg);

#endif/*_SD_ANC_LIB_V2_H*/
