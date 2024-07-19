#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".icsd_adt.data.bss")
#pragma data_seg(".icsd_adt.data")
#pragma const_seg(".icsd_adt.text.const")
#pragma code_seg(".icsd_adt.text")
#endif

#include "app_config.h"
#if ((defined TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN) && TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN && \
	 TCFG_AUDIO_ANC_ENABLE)
#include "icsd_adt.h"
#include "icsd_adt_app.h"

//==============================================//
//    功能使能
//==============================================//
#if TCFG_AUDIO_ANC_WIND_NOISE_DET_ENABLE && \
TCFG_AUDIO_SPEAK_TO_CHAT_ENABLE == 0 && \
TCFG_AUDIO_WIDE_AREA_TAP_ENABLE == 0
u8 ICSD_WIND_MODE = 1;//独立风噪模式, 资源占用小，不支持广域和免摘共存
#else
u8 ICSD_WIND_MODE = 0;//独立风噪模式
#endif
u8 ICSD_WIND_EN  =  TCFG_AUDIO_ANC_WIND_NOISE_DET_ENABLE;//风噪检测使能
u8 ICSD_ENVNL_EN =  0;//环境声检测使能

//ADT_RESULT | WIND_MSCOHERE | WIND_CORR | WIND_PWR_ERR_REF;
const u8 DEBUG_ADT_WIND_EN  = 0;//ADT_RESULT;
const u8 DEBUG_ADT_ENVNL_EN = 0;//ADT_RESULT;
//==============================================//
//    通用配置
//==============================================//
#ifndef ADT_CLIENT_BOARD
//3033
//const float ref_gain_sel  = 10;
//const float err_gain_sel  = 2;
//H3 HUILAN
//const float ref_gain_sel  = 4;
//const float err_gain_sel  = 1;
//E108D
const float ref_gain_sel  = 3;
const float err_gain_sel  = 3;
#endif
const float tlk_gain_sel  = 3;
//==============================================//
//    环境声参数配置
//==============================================//
const float env_alpha = 0.25;//0 ~ 1之间,越大越快
//==============================================//
//    风噪检测参数配置
//==============================================//


u8 ICSD_WIND_DEBUG = 0;//ICSD_WIND_MIC_DATA;
const u8  ICSD_WIND_3MIC = 0; //当前不支持3mic
const u8  wind_offset   = 1;
const u16 wind_ref_thr  = 120;
const u16 wind_err_thr  = 120;
const int cali_wind_thr = -10;
const u16 correrr_thr = 300;
const int wind_sat_thr = 10000;
const u8 wind_lowfreq_point = 10;
const u8 wind_pwr_cnt_thr = 0;
const float wind_margin_dB = -1;
const float wind_pwr_ref_thr = 15;
const float wind_pwr_err_thr = 20;
const float wind_iir_alpha = 16;
const float wind_lpf_alpha = 0.0625;
const float wind_hpf_alpha = 0.001;
const float wind_max_min_diff_thr = 32;
const float wind_timepwr_diff_thr0 = 100;
const float wind_timepwr_diff_thr1 = 400;
const float wind_ref2err_diff_thr = 50;//50;
const float wind_ref2err_diffmin_thr = 15;
const u8 wind_num_thr = 4;
const float wind_tlk_corr = 0.35;//0.3 ~ 1
const u8 wind_tlk_corr_num = 6;//7
const float wind_tlk_corr_avr = 0.35;//0.3;
const u8 wind_corr_select = 0;
const u8 wind_fcorr_fpoint = 10;
const float wind_fcorr_min_thr = 0.6;
const int wind_ref2err_cnt = 13;
const u8 wind_stable_cnt_thr = 1;
const u8 wind_lvl_scale = 4;
const u8 icsd_wind_num_thr2 = 3;
const u8 wind_out_mode = 1;//1:滤波输出(滤波模式) 0:连续3包无风输出0(快速模式)
const float wind_ref_cali_table[25] = {0};
/*
{
  6,6,6,6,6,
  6,6,6,6,6,
  6,6,6,6,6,
  6,6,6,6,6,
  6,6,6,6,6
};*/


//==============================================//
//    智能免摘参数配置
//==============================================//
const u8 VDT_TRAIN_EN = 0;

#ifndef ADT_CLIENT_BOARD
const u8 ADT_TWS_MODE = 1;
const u8 ADT_DET_VER  = 6;
u8 ADT_PATH_CONFIG = ADT_PATH_DAC_SRC_EN | ADT_PATH_3M_EN;

const u16 adt_dov_thr = 100;
const u16 adt_dov_diff_thr = 3;
const u8 final_speech_even_thr = 4;     //dov
const u8 final_speech_even_thr_env = 8;
const u8 env_spl_flt_thr = 200;
const float dc_alpha_thr = 0.125;
const u8 ana_len_start = 2;//ana_len_start * 92hz
const u8 icsd_ana_len = 8;//max:16
const float wdac_senc = 0.7; //越小播音乐时灵敏度越高

const float ang_thr[10] = {0, 0, 18, 13, 8, 5, 2, 0, -2, -4};
const float wref[8]   = {1.07, 0.72, 0.67, 0.63, 0.57, 0.49, 0.45, 0.43};//播放外部噪声 PNC
const float wref_tran[8]   = {2.36, 2.29, 2.72, 2.98, 3.06, 2.93, 2.70, 2.51};//播放外部噪声 TRANS
const float wref_ancs[8]   = {0.58, 0.10, 0.03, 0.03, 0.04, 0.05, 0.04, 0.03};//播放外部噪声 ANC ON
const float wdac[8] = { 0.13, 0.21, 0.37, 0.76, 0.84, 0.79, 0.72, 0.65}; //播放音乐/100
const float fdac[8] = { 0.18, 0.051, 0.02, 0.02, 0.02, 0.02, 0.02, 0.05}; //播放音乐/1000 手动修改
u8 pnc_ref_pwr_thr = 60;
u8 pnc_err_pwr_thr = 100;
u8 tpc_ref_pwr_thr = 60;
u8 tpc_err_pwr_thr = 100;
u8 anc_ref_pwr_thr = 60;
u8 anc_err_pwr_thr = 80;

/* float angle_fb_dn[5] = {15, 15, 15, 15, 15}; */
/* float angle_fb_up[5] = {-130, -130, -130, -130, -130}; */
float angle_fb_dn[5] = {0, 0, 0, 0, 0};
float angle_fb_up[5] = {-110, -110, -110, -110, -110};
u8 icsd_get_ang_flag(float *_target_angle)
{
    u8 af1 = _target_angle[1] > angle_fb_dn[1] || _target_angle[1] < angle_fb_up[1];
    u8 af2 = _target_angle[2] > angle_fb_dn[2] || _target_angle[2] < angle_fb_up[2];
    u8 af3 = _target_angle[3] > angle_fb_dn[3] || _target_angle[3] < angle_fb_up[3];

    //
    u8 ang_flag = af1 && af2 && af3;
    return ang_flag;
}
#endif
//ADT_INF_4 ref err 相位
//ADT_INF_2 ref tlk 相位
const u16 ADT_DEBUG_INF = 0;//ADT_INF_5 | ADT_INF_1; //state
const u8 adt_errpxx_dB_thr = 100;//60;
const u8 adt_err_zcv_thr = 0;//12;
const u8 pwr_inside_even_flag0_thr = 4; //pwr
const u8 angle_err_even_flag0_thr = 3;//4;  //angle_err
const u8 angle_talk_even_flag0_thr = 5; //angle_tlk
const u16 f1f2_angsum_thr = 200;
//==============================================//
//    广域点击参数配置
//==============================================//
#ifndef ADT_CLIENT_BOARD
//正常敲击时max_range:要大于2000
#if 1
const float wat_pn_gain     = 1.1;//0.22;
const float wat_bypass_gain = 6;
const float wat_anc_gain    = 1;
#else
const float wat_pn_gain     = 0.22;//1.1;//0.22;
const float wat_bypass_gain = 16;//6;
const float wat_anc_gain    = 0.22;//1;
#endif

#endif

#endif/*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/
