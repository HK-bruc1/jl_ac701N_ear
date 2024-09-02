/**
* 注意点：
* 0.此文件变化，在工具端会自动同步修改到工具配置中
* 1.功能块通过【---------xxx------】和 【#endif // xxx 】，是工具识别的关键位置，请勿随意改动
* 2.目前工具暂不支持非文件已有的C语言语法，此文件应使用文件内已有的语法增加业务所需的代码，避免产生不必要的bug
* 3.修改该文件出现工具同步异常或者导出异常时，请先检查文件内语法是否正常
**/

#ifndef JLSTREAM_NODE_CFG_H
#define JLSTREAM_NODE_CFG_H

// ------------提示音宏定义------------
#define TCFG_TONE_EN_ENABLE 1
#define TCFG_TONE_ZH_ENABLE 0
#define TCFG_DEC_AAC_ENABLE 0 // AAC
#define TCFG_DEC_F2A_ENABLE 0 // F2A
#define TCFG_DEC_MP3_ENABLE 0 // MP3
#define TCFG_DEC_MSBC_ENABLE 0 // MSBC
#define TCFG_DEC_MTY_ENABLE 0 // MTY
#define TCFG_DEC_SBC_ENABLE 0 // SBC
#define TCFG_DEC_SIN_ENABLE 1 // SIN
#define TCFG_DEC_WAV_ENABLE 0 // WAV
#define TCFG_DEC_WTG_ENABLE 1 // WTG
#define TCFG_DEC_WTS_ENABLE 0 // WTS
// ------------提示音宏定义------------

// ------------流程图宏定义------------
#define TCFG_AUDIO_BIT_WIDTH 0 // 位宽
#define TCFG_2BAND_MERGE_ENABLE 0 // 2Band Merge
#define TCFG_3BAND_MERGE_ENABLE 0 // 3Band Merge
#define TCFG_ADC_NODE_ENABLE 1 // ADC
#define TCFG_AGC_NODE_ENABLE 0 // AGC
#define TCFG_AI_TX_NODE_ENABLE 0 // AI_TX
#define TCFG_AUDIO_CVP_3MIC_MODE 0 // 3MIC通话
#define TCFG_AUDIO_CVP_DEVELOP_ENABLE 0 // 通话第三方算法
#define TCFG_AUDIO_CVP_DMS_ANS_MODE 0 // 双MIC通话
#define TCFG_AUDIO_CVP_DMS_AWN_DNS_MODE 0 // 2MIC-AWN
#define TCFG_AUDIO_CVP_DMS_DNS_MODE 0 // 双MIC+DNS
#define TCFG_AUDIO_CVP_DMS_FLEXIBLE_ANS_MODE 0 // 话务双MIC通话
#define TCFG_AUDIO_CVP_DMS_FLEXIBLE_DNS_MODE 0 // 话务双MIC+DNS
#define TCFG_AUDIO_CVP_DMS_HYBRID_DNS_MODE 0 // HYBRID双MIC+DNS
#define TCFG_AUDIO_CVP_SMS_ANS_MODE 1 // 单MIC通话
#define TCFG_AUDIO_CVP_SMS_DNS_MODE 0 // 单MIC+DNS
#define TCFG_AUTO_WAH_NODE_ENABLE 0 // Auto Wah
#define TCFG_AUTODUCK_NODE_ENABLE 0 // AutoDuck Trigger、AutoDuck
#define TCFG_AUTOMUTE_NODE_ENABLE 0 // automute
#define TCFG_AUTOTUNE_NODE_ENABLE 0 // Autotune
#define TCFG_BASS_TREBLE_NODE_ENABLE 0 // Bass Treble
#define TCFG_CHANNEL_EXPANDER_NODE_ENABLE 0 // Channel Expander
#define TCFG_CHANNEL_MERGE_NODE_ENABLE 1 // Channel Merge
#define TCFG_CHANNEL_SWAP_NODE_ENABLE 0 // 声道交换器
#define TCFG_CHORUS_NODE_ENABLE 0 // Chorus
#define TCFG_CONVERT_NODE_ENABLE 1 // Convert
#define TCFG_CROSSOVER_NODE_ENABLE 0 // 分频器 三段、分频器 两段
#define TCFG_DAC_NODE_ENABLE 1 // DAC
#define TCFG_DATA_EXPORT_NODE_ENABLE 0 // data export
#define TCFG_DNS_NODE_ENABLE 0 // DNS Noise Suppressor
#define TCFG_DYNAMIC_EQ_EXT_DETECTOR_NODE_ENABLE 0 // Dynamic EQ  Ext Detector
#define TCFG_DYNAMIC_EQ_NODE_ENABLE 0 // Dynamic EQ
#define TCFG_DYNAMIC_EQ_PRO_EXT_DETECTOR_NODE_ENABLE 0 // Dynamic EQ  Pro Ext Detector
#define TCFG_DYNAMIC_EQ_PRO_NODE_ENABLE 0 // Dynamic EQ Pro
#define TCFG_ECHO_NODE_ENABLE 0 // Echo
#define TCFG_EFFECT_DEV0_NODE_ENABLE 0 // EffectDev0
#define TCFG_EFFECT_DEV1_NODE_ENABLE 0 // EffectDev1
#define TCFG_EFFECT_DEV2_NODE_ENABLE 0 // EffectDev2
#define TCFG_EFFECT_DEV3_NODE_ENABLE 0 // EffectDev3
#define TCFG_EFFECT_DEV4_NODE_ENABLE 0 // EffectDev4
#define TCFG_ENERGY_DETECT_NODE_ENABLE 0 // Energy Detect
#define TCFG_EQ_ENABLE 1 // EQ
#define TCFG_ESCO_TX_NODE_ENABLE 1 // ESCO_TX
#define TCFG_FADE_NODE_ENABLE 0 // Fade
#define TCFG_FREQUENCY_SHIFT_HOWLING_NODE_ENABLE 0 // Frequency Shift
#define TCFG_GAIN_NODE_ENABLE 0 // Gain
#define TCFG_HARMONIC_EXCITER_NODE_ENABLE 0 // Harmonic Exciter
#define TCFG_IIS_NODE_ENABLE 0 // IIS0_RX、IIS0_TX
#define TCFG_INDICATOR_NODE_ENABL 0 // Indicator
#define TCFG_KEY_TONE_NODE_ENABLE 1 // 按键音
#define TCFG_LIMITER_NODE_ENABLE 0 // Limiter
#define TCFG_LLNS_NODE_ENABLE 0 // LLNS
#define TCFG_MIXER_NODE_ENABLE 0 // MIXER
#define TCFG_MULTI_BAND_DRC_NODE_ENABLE 0 // MDRC
#define TCFG_MULTI_BAND_LIMITER_NODE_ENABLE 0 // MB Limiter
#define TCFG_MULTI_CH_IIS_NODE_ENABLE 0 // MULTI CH IIS0 TX
#define TCFG_MULTI_CH_IIS_RX_NODE_ENABLE 0 // MULTI CH IIS0 RX
#define TCFG_NOISEGATE_NODE_ENABLE 0 // NoiseGate
#define TCFG_NOISEGATE_PRO_NODE_ENABLE 0 // NoiseGate Pro
#define TCFG_NOTCH_HOWLING_NODE_ENABLE 0 // Howling Suppress
#define TCFG_NS_NODE_ENABLE 0 // Noise Suppressor
#define TCFG_PCM_DELAY_NODE_ENABLE 0 // PCM Delay
#define TCFG_PDM_NODE_ENABLE 0 // PDM MIC
#define TCFG_PLATE_REVERB_ADVANCE_NODE_ENABLE 0 // Plate Reverb Advance
#define TCFG_PLATE_REVERB_NODE_ENABLE 0 // Plate Reverb
#define TCFG_PLC_NODE_ENABLE 1 // 丢包修复PLC
#define TCFG_REPLACE_NODE_ENABLE 0 // Replace
#define TCFG_RING_TONE_NODE_ENABLE 1 // 铃声
#define TCFG_SIGNAL_GENERATOR_NODE_ENABLE 0 // SignalGen
#define TCFG_SINK_DEV0_NODE_ENABLE 0 // Sink_Dev0
#define TCFG_SINK_DEV1_NODE_ENABLE 0 // Sink_Dev1
#define TCFG_SINK_DEV2_NODE_ENABLE 0 // Sink_Dev2
#define TCFG_SINK_DEV3_NODE_ENABLE 0 // Sink_Dev3
#define TCFG_SINK_DEV4_NODE_ENABLE 0 // Sink_Dev4
#define TCFG_SOUND_SPLITTER_NODE_ENABLE 0 // 音频分流器
#define TCFG_SOURCE_DEV0_NODE_ENABLE 0 // Source_Dev0
#define TCFG_SOURCE_DEV1_NODE_ENABLE 0 // Source_Dev1
#define TCFG_SOURCE_DEV2_NODE_ENABLE 0 // Source_Dev2
#define TCFG_SOURCE_DEV3_NODE_ENABLE 0 // Source_Dev3
#define TCFG_SOURCE_DEV4_NODE_ENABLE 0 // Source_Dev4
#define TCFG_SPATIAL_AUDIO_ENABLE 0 // 空间音效
#define TCFG_SPEAKER_EQ_NODE_ENABLE 0 // SpeakerEQ
#define TCFG_STEREO_WIDENER_NODE_ENABLE 0 // Stereo Widener
#define TCFG_STEROMIX_NODE_ENABLE 0 // SteroMix
#define TCFG_SURROUND_DEMO_NODE_ENABLE 0 // Surround Demo、环绕音demo
#define TCFG_SURROUND_NODE_ENABLE 0 // Surround Effect
#define TCFG_SWITCH_NODE_ENABLE 0 // Switch
#define TCFG_THREE_D_EFFECT_NODE_ENABLE 0 // ThreeD
#define TCFG_TONE_NODE_ENABLE 1 // 提示音
#define TCFG_UART_NODE_ENABLE 0 // 串口打印
#define TCFG_VBASS_NODE_ENABLE 0 // Virtual Bass
#define TCFG_VOCAL_REMOVER_NODE_ENABLE 0 // Vocal Remover
#define TCFG_VOCAL_TRACK_SEPARATION_NODE_ENBALE 0 // 声道拆分
#define TCFG_VOCAL_TRACK_SYNTHESIS_NODE_ENABLE 0 // 声道组合
#define TCFG_VOICE_CHANGER_ADV_NODE_ENABLE 0 // Voice Changer Adv
#define TCFG_VOICE_CHANGER_NODE_ENABLE 0 // Voice Changer
#define TCFG_WDRC_NODE_ENABLE 0 // DRC
#define EQ_SECTION_MAX 0xc // EQ_SECTION_MAX
// ------------流程图宏定义------------
#endif

