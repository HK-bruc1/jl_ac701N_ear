#ifndef __NODE_PARAM_UPDATE_H_
#define __NODE_PARAM_UPDATE_H_

#include "effects/audio_gain_process.h"
#include "effects/audio_surround.h"
#include "effects/audio_bass_treble_eq.h"
#include "effects/audio_crossover.h"
#include "effects/multi_ch_mix.h"
#include "effects/eq_config.h"
#include "effects/audio_wdrc.h"
#include "effects/audio_autotune.h"
#include "effects/audio_chorus.h"
#include "effects/dynamic_eq.h"
#include "effects/audio_echo.h"
#include "effects/audio_frequency_shift_howling.h"
#include "effects/audio_noisegate.h"
#include "effects/audio_notch_howling.h"
#include "effects/audio_pitchspeed.h"
#include "effects/audio_reverb.h"
#include "effects/audio_plate_reverb.h"
#include "effects/spectrum/spectrum_fft.h"
#include "effects/audio_stereo_widener.h"
#include "effects/audio_vbass.h"
#include "effects/audio_voice_changer.h"
#include "effects/channel_adapter.h"
#include "effects/audio_harmonic_exciter.h"
#include "effects/audio_multiband_drc.h"
#include "effects/audio_virtual_surround_pro.h"
#include "effects/audio_multiband_limiter.h"
#include "effects/pcm_delay.h"

/* 左右声道按照不同比例混合参数更新 */
int stero_mix_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* 环绕声参数更新 */
int surround_effect_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* 分频器参数更新 */
int crossover_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* 多带合并参数更新 */
int band_merge_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* drc参数更新 */
int drc_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* 高低音参数更新 */
int bass_treble_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* 电音参数更新 */
int autotune_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* 合唱参数更新 */
int chorus_udpate_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* 动态eq参数更新 */
int dynamic_eq_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* 回声参数更新 */
int echo_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* 啸叫抑制-移频参数更新 */
int howling_frequency_shift_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* 啸叫抑制-陷波参数更新 */
int howling_suppress_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* 增益控制参数更新 */
int gain_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* 噪声门限参数更新 */
int noisegate_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* 混响参数更新 */
int plate_reverb_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* 混响V2参数更新 */
int reverb_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* 高阶混响参数更新 */
int reverb_advance_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* 频谱计算参数更新 */
int spectrum_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* 立体声增强参数更新 */
int stereo_widener_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* 虚拟低音参数更新 */
int virtual_bass_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* 变声参数更新 */
int voice_changer_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* 声道扩展参数更新 */
int channel_expander_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* eq参数更新 */
int eq_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* eq系数表更新*/
int eq_update_tab(u8 mode_index, char *node_name, u8 cfg_index);
/* 谐波激励参数更新 */
int harmonic_exciter_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* MDRC参数更新 */
int multiband_drc_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* DRC_ADV参数更新 */
int wdrc_advance_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* Virtual Surround Pro参数更新 */
int virtual_surround_pro_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* Limiter参数更新 */
int limiter_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/* Limiter参数更新 */
int multiband_limiter_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
/*PCM Delay 参数更新 */
int pcm_delay_update_parm(u8 mode_index, char *node_name, u8 cfg_index);
#endif
