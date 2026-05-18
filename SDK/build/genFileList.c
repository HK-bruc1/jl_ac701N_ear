#include "app_config.h"

// *INDENT-OFF*



c_SRC_FILES += \
      audio/framework/plugs/source/a2dp_file.c \
      audio/framework/plugs/source/a2dp_streamctrl.c \
      audio/framework/plugs/source/esco_file.c \
      audio/framework/plugs/source/adc_file.c \
	  audio/framework/plugs/source/multi_ch_adc_file.c \
      audio/framework/nodes/esco_tx_node.c \
      audio/framework/nodes/plc_node.c \
      audio/framework/nodes/volume_node.c \

#if TCFG_AUDIO_DEBUG_NODE_ENABLE
c_SRC_FILES += \
	  audio/framework/nodes/audio_debug_node.c

#endif


#if TCFG_NS_NODE_ENABLE || TCFG_NS_NODE_LITE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/ns_node.c
#endif

#if TCFG_CHANNEL_SWAP_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/channle_swap_node.c
#endif

#if TCFG_EFFECT_DEV0_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/effect_dev0_node.c
#endif
#if TCFG_EFFECT_DEV1_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/effect_dev1_node.c
#endif
#if TCFG_EFFECT_DEV2_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/effect_dev2_node.c
#endif
#if TCFG_EFFECT_DEV3_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/effect_dev3_node.c
#endif
#if TCFG_EFFECT_DEV4_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/effect_dev4_node.c
#endif
#if TCFG_SINK_DEV0_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/sink_dev0_node.c
#endif
#if TCFG_SINK_DEV1_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/sink_dev1_node.c
#endif
#if TCFG_SINK_DEV2_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/sink_dev2_node.c
#endif
#if TCFG_SINK_DEV3_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/sink_dev3_node.c
#endif
#if TCFG_SINK_DEV4_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/sink_dev4_node.c
#endif

#if TCFG_SOURCE_DEV0_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/plugs/source/source_dev0_file.c
#endif
#if TCFG_SOURCE_DEV1_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/plugs/source/source_dev1_file.c
#endif
#if TCFG_SOURCE_DEV2_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/plugs/source/source_dev2_file.c
#endif
#if TCFG_SOURCE_DEV3_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/plugs/source/source_dev3_file.c
#endif
#if TCFG_SOURCE_DEV4_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/plugs/source/source_dev4_file.c
#endif

#if TCFG_AGC_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/agc_node.c
#endif

#if TCFG_SURROUND_DEMO_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/surround_demo_node.c
#endif

#if TCFG_LHDC_X_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/lhdc_x_node.c
#endif

#if TCFG_AUDIO_CVP_SMS_ANS_MODE || TCFG_AUDIO_CVP_SMS_DNS_MODE
c_SRC_FILES += \
      audio/framework/nodes/cvp_sms_node.c
#endif

c_SRC_FILES += \
      audio/framework/nodes/cvp_dms_node.c

#if TCFG_AUDIO_CVP_3MIC_MODE
c_SRC_FILES += \
      audio/framework/nodes/cvp_3mic_node.c
#endif

#if TCFG_AUDIO_CVP_DEVELOP_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/cvp_develop_node.c
#endif

#if TCFG_UART_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/uart_node.c
#endif

#if TCFG_AUDIO_CVP_V3_MODE
c_SRC_FILES += \
      audio/framework/nodes/cvp_v3_node.c
#endif

#if TCFG_AUDIO_CVP_SMS_VF_MODE
c_SRC_FILES += \
      audio/framework/nodes/cvp_sms_vf_node.c
#endif

#if TCFG_DNS_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/dns_node.c
#endif

#if TCFG_AI_TX_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/ai_tx_node.c
#endif

#if TCFG_AI_RX_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/plugs/source/ai_rx_file.c
#endif

#if TCFG_DATA_EXPORT_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/data_export_node.c
#endif

#if TCFG_LOCAL_TWS_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/local_tws_source_node.c \
      audio/framework/plugs/source/local_tws_file.c \
	  audio/interface/player/local_tws_player.c
#endif

#if EXPORT_PLATFORM_AUDIO_PDM_ENABLE
#if TCFG_PDM_NODE_ENABLE
c_SRC_FILES += \
    audio/framework/plugs/source/pdm_mic_file.c
#endif
#endif

#if EXPORT_PLATFORM_AUDIO_SPDIF_ENABLE
#if TCFG_SPDIF_ENABLE
c_SRC_FILES += \
	  audio/framework/plugs/source/spdif_file.c
#endif
#endif


// Audio Test Tool Kit
#if TCFG_AUDIO_MIC_DUT_ENABLE
c_SRC_FILES += \
	  audio/test_tools/mic_dut_process.c
#endif

#if AUDIO_ENC_MPT_SELF_ENABLE
c_SRC_FILES += \
	  audio/test_tools/audio_enc_mpt_self.c \
	  audio/test_tools/audio_enc_mpt_cvp_ctr.c
#endif

#if TCFG_AEC_TOOL_ONLINE_ENABLE
c_SRC_FILES += \
	  audio/test_tools/cvp_tool.c
#endif

#if TCFG_AUDIO_DUT_ENABLE
c_SRC_FILES += \
	  audio/test_tools/audio_dut_control.c \
	  audio/test_tools/audio_dut_control_old.c
#endif

// Audio Common
c_SRC_FILES += \
	  audio/common/audio_node_config.c \
	  audio/common/audio_dvol.c \
	  audio/common/audio_general.c \
	  audio/common/audio_build_needed.c \
	  audio/common/online_debug/aud_data_export.c \
	  audio/common/online_debug/audio_online_debug.c \
	  audio/common/online_debug/audio_capture.c \
	  audio/common/audio_plc.c \
	  audio/common/audio_noise_gate.c \
	  audio/common/audio_ns.c \
	  audio/common/audio_utils.c \
	  audio/common/audio_export_demo.c \
	  audio/common/amplitude_statistic.c \
	  audio/common/frame_length_adaptive.c \
	  audio/common/bt_audio_energy_detection.c \
	  audio/common/audio_event_handler.c \
	  audio/common/debug/audio_debug.c \
	  audio/common/power/mic_power_manager.c \
	  audio/common/audio_volume_mixer.c \
	  audio/common/audio_effect_verify.c \
	  audio/common/pcm_data/sine_pcm.c \

#if AUDIO_IIS_LRCLK_CAPTURE_EN
c_SRC_FILES += \
	  audio/common/audio_iis_lrclk_capture.c
#endif

#if TCFG_AUDIO_MIC_DUT_ENABLE
c_SRC_FILES += \
	  audio/common/online_debug/aud_mic_dut.c
#endif

#if (TCFG_AUDIO_DATA_EXPORT_DEFINE == AUDIO_DATA_EXPORT_VIA_UART)
c_SRC_FILES += \
	  audio/common/uartPcmSender.c
#endif

#if 0
c_SRC_FILES += \
	  audio/common/demo/codec_demo/audio_enc_file_demo.c \
	  audio/common/demo/codec_demo/audio_file_dec_demo.c \
	  audio/common/demo/codec_demo/audio_frame_codec_demo.c \
	  audio/common/demo/codec_demo/audio_msbc_hw_codec_demo.c \
	  audio/common/demo/codec_demo/audio_msbc_sw_codec_demo.c \
	  audio/common/demo/codec_demo/audio_sbc_codec_demo.c \
	  audio/common/demo/codec_demo/audio_speex_codec_demo.c \
	  audio/common/demo/codec_demo/audio_opus_codec_demo.c \
	  audio/common/demo/audio_alink_demo.c \
	  audio/common/demo/audio_eq_demo.c \

#endif

#if EXPORT_PLATFORM_AUDIO_HW_VAD_ENABLE
#if 0
c_SRC_FILES += \
	  audio/common/demo/audio_vad_demo.c

#endif
#endif

#if EXPORT_PLATFORM_HW_MATH_VERSION == 2
c_SRC_FILES += \
	  audio/common/demo/hw_math_v2_demo.c
#endif


#if EXPORT_PLATFORM_AUDIO_WM8978_ENABLE
#if 0
c_SRC_FILES += \
	  audio/common/wm8978/wm8978.c \
	  audio/common/wm8978/iic.c
#endif
#endif


// Audio Player
c_SRC_FILES += \
	  audio/interface/player/tone_player.c \
	  audio/interface/player/ring_player.c \
	  audio/interface/player/a2dp_player.c \
	  audio/interface/player/esco_player.c \
	  audio/interface/player/key_tone_player.c \
	  audio/interface/player/dev_flow_player.c \
	  audio/interface/player/adda_loop_player.c \
	  audio/interface/player/linein_player.c \
	  audio/interface/player/ai_rx_player.c \

// Audio Recoder
c_SRC_FILES += \
	  audio/interface/recoder/esco_recoder.c \
	  audio/interface/recoder/ai_voice_recoder.c \
	  audio/interface/recoder/dev_flow_recoder.c \

#if EXPORT_LE_AUDIO_SUPPORT
#if LE_AUDIO_STREAM_ENABLE
c_SRC_FILES += \
	audio/interface/player/le_audio_player.c \
	audio/framework/nodes/le_audio_source.c \
	audio/framework/plugs/source/le_audio_file.c \
	audio/le_audio/le_audio_stream.c
#endif
#if ((TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_UNICAST_SINK_EN | LE_AUDIO_JL_UNICAST_SINK_EN)))
c_SRC_FILES += \
	  audio/interface/recoder/le_audio_recorder.c
#endif
#endif

#if EXPORT_PLATFORM_AUDIO_MIDI_ENABLE
#if MIDI_FILE_DEC_ENABLE
c_SRC_FILES += \
	audio/midi/audio_dec_midi_file.c
#endif

#if MIDI_CTRL_DEC_ENABLE
c_SRC_FILES += \
	audio/midi/audio_dec_midi_ctrl.c
#endif
#endif


// Audio Effects
c_SRC_FILES += \
      audio/effect/eq_config.c \
	  audio/effect/spk_eq.c \
	  audio/effect/audio_voice_changer_api.c \
	  audio/effect/esco_ul_voice_changer.c \
	  audio/effect/bass_treble.c \
	  audio/effect/audio_dc_offset_remove.c \
	  audio/effect/effects_adj.c \
	  audio/effect/effects_dev.c \
	  audio/effect/effects_default_param.c \
	  audio/effect/node_param_update.c \
	  audio/effect/scene_update.c \

// ICSD
#if EXPORT_PLATFORM_ICSD_ENABLE
#if TCFG_AUDIO_ANC_ENABLE
c_SRC_FILES += \
	audio/common/icsd/adt/icsd_adt.c \
	audio/common/icsd/adt/icsd_adt_app.c \
	audio/common/icsd/adt/icsd_adt_config.c \
	audio/common/icsd/adt/icsd_adt_alg.c \
	audio/common/icsd/adt/icsd_adt_demo.c \
	audio/common/icsd/anc/icsd_anc_app.c \
	audio/common/icsd/anc/icsd_anc_board.c \
	audio/common/icsd/anc/icsd_anc_data.c \
	audio/common/icsd/anc/icsd_anc_interactive.c \
	audio/common/icsd/common/icsd_common.c \
	audio/common/icsd/dot/icsd_dot_app.c \
	audio/common/icsd/dot/icsd_dot.c \
	audio/common/icsd/common_v2/icsd_common_v2.c \
	audio/common/icsd/common_v2/icsd_common_v2_app.c \
	audio/common/icsd/anc_v2/icsd_anc_v2.c \
	audio/common/icsd/anc_v2/icsd_anc_v2_app.c \
	audio/common/icsd/anc_v2/icsd_anc_v2_interactive.c \
	audio/common/icsd/config/icsd_anc_v2_config.c \
	audio/common/icsd/rt_anc/rt_anc.c \
	audio/common/icsd/rt_anc/rt_anc_app.c \
	audio/common/icsd/rt_anc/rt_anc_config.c \
	audio/common/icsd/tool/anc_ext_tool.c \
	audio/common/icsd/tool/anc_ext_tool_file.c \
	audio/common/icsd/aeq/icsd_aeq_app.c \
	audio/common/icsd/aeq/icsd_aeq.c \
	audio/common/icsd/aeq/icsd_aeq_config.c \
	audio/common/icsd/afq/icsd_afq_app.c \
	audio/common/icsd/afq/icsd_afq.c \
	audio/common/icsd/afq/icsd_afq_config.c \
	audio/common/icsd/cmp/icsd_cmp_app.c \
	audio/common/icsd/config/icsd_cmp_config.c \
	audio/common/icsd/demo/icsd_demo.c \
	audio/common/icsd/ein/icsd_ein_config.c \
	audio/common/icsd/vdt/icsd_vdt_config.c \
	audio/common/icsd/vdt/icsd_vdt_app.c \
	audio/common/icsd/wat/icsd_wat_config.c \
	audio/common/icsd/wat/icsd_wat_app.c \
	audio/common/icsd/wind/icsd_wind_config.c \
	audio/common/icsd/wind/icsd_wind_app.c \
	audio/common/icsd/wind/cvp_wind_app.c \
	audio/common/icsd/avc/icsd_avc_config.c \
	audio/common/icsd/avc/icsd_avc_app.c \
	audio/common/icsd/env_noise_det/icsd_env_noise_det_app.c \
	audio/common/icsd/adjdcc/icsd_adjdcc_config.c \
	audio/common/icsd/howl/icsd_howl_config.c \
	audio/common/icsd/howl/icsd_howl_app.c \

#endif
#endif

// Audio ANC
#if EXPORT_PLATFORM_ANC_ENABLE
#if TCFG_AUDIO_ANC_ENABLE
c_SRC_FILES += \
	  audio/anc/audio_anc_fade_ctr.c \
	  audio/anc/audio_anc_common_plug.c \
	  audio/anc/audio_anc_debug_tool.c \
	  audio/anc/audio_anc_mult_scene.c \
	  audio/anc/audio_anc_common.c \

#endif
#endif

// Smart Voice
#if EXPORT_PLATFORM_AUDIO_SMART_VOICE_ENABLE
#if TCFG_SMART_VOICE_ENABLE
c_SRC_FILES += \
	audio/smart_voice/smart_voice_core.c \
	audio/smart_voice/smart_voice_config.c \
	audio/smart_voice/jl_kws_platform.c \
	audio/smart_voice/user_asr.c \
	audio/smart_voice/voice_mic_data.c \
	audio/smart_voice/kws_event.c \
	audio/smart_voice/nn_vad.c \
	audio/smart_voice/nn_vad_bin.c
#endif
#endif

// KWS(yes/no)
#if TCFG_KWS_VOICE_RECOGNITION_ENABLE
c_SRC_FILES += \
	audio/jl_kws/jl_kws_main.c \
	audio/jl_kws/jl_kws_audio.c \
	audio/jl_kws/jl_kws_algo.c \
	audio/jl_kws/jl_kws_event.c \

#endif


#if EXPORT_PLATFORM_AUDIO_SPATIAL_EFFECT_ENABLE
#if TCFG_SPATIAL_AUDIO_ENABLE
c_SRC_FILES += \
	audio/framework/nodes/spatial_effects_node.c \
	audio/common/online_debug/aud_spatial_effect_dut.c \

c_SRC_FILES += \
	audio/effect/spatial_effect/spatial_effect.c \
	audio/effect/spatial_effect/spatial_effect_imu.c \
	audio/effect/spatial_effect/spatial_effect_tws.c \
	audio/effect/spatial_effect/spatial_imu_trim.c \
	audio/effect/spatial_effect/spatial_effects_process.c \

#endif
#endif

#if EXPORT_SOMATOSENSORY_ENABLE
c_SRC_FILES += \
	audio/effect/somatosensory/audio_somatosensory.c \

#endif

// Clear Voice Process(AEC/NLP/NS/AGC...)
c_SRC_FILES += \
	  audio/CVP/audio_aec.c \
	  audio/CVP/audio_cvp.c \
	  audio/CVP/audio_cvp_sms_vf.c \
	  audio/CVP/audio_cvp_dms.c \
	  audio/CVP/audio_cvp_3mic.c \
	  audio/CVP/audio_cvp_v3.c \
	  audio/CVP/audio_cvp_online.c \
	  audio/CVP/audio_cvp_demo.c \
	  audio/CVP/audio_cvp_develop.c \
	  audio/CVP/audio_cvp_sync.c \
	  audio/CVP/audio_cvp_ais_3mic.c \
	  audio/CVP/audio_cvp_ref_task.c \
	  audio/CVP/audio_cvp_config.c \

#if TCFG_USER_TWS_ENABLE
c_SRC_FILES += \
	  audio/interface/player/tws_tone_player.c
#endif

// ALINK BUILD
#if EXPORT_PLATFORM_AUDIO_ALINK_ENABLE
#if TCFG_IIS_NODE_ENABLE
c_SRC_FILES += \
	  audio/framework/nodes/iis_node.c \
	  audio/framework/plugs/source/iis_file.c
#endif

#if TCFG_MULTI_CH_IIS_NODE_ENABLE
c_SRC_FILES += \
	  audio/framework/nodes/multi_ch_iis_node.c \
	  audio/framework/plugs/source/multi_ch_iis_file.c
#endif

#endif

// SPDIF BUILD
#if EXPORT_PLATFORM_AUDIO_SPDIF_ENABLE
#if TCFG_SPDIF_MASTER_NODE_ENABLE
c_SRC_FILES += \
	  audio/framework/nodes/spdif_node.c
#endif
#endif

// SPDIF BUILD
#if EXPORT_PLATFORM_AUDIO_FM_ENABLE
c_SRC_FILES += \
	  audio/interface/player/fm_player.c \
	  audio/framework/plugs/source/fm_file.c \

#endif

#if EXPORT_PLATFORM_AUDIO_TDM_ENABLE
c_SRC_FILES += \
	  audio/interface/player/tdm_player.c \
	  audio/framework/plugs/source/tdm_file.c \
	  audio/framework/nodes/tdm_node.c \

#endif

#if TCFG_APP_MUSIC_EN
c_SRC_FILES += \
      audio/interface/player/file_player.c \

#endif

#if TCFG_APP_RECORD_EN || TCFG_MIX_RECORD_ENABLE
c_SRC_FILES += \
      audio/interface/recoder/file_recorder.c \

#endif

c_SRC_FILES += \
	  audio/framework/plugs/source/linein_file.c \

#if TCFG_USB_SLAVE_AUDIO_SPK_ENABLE
c_SRC_FILES += \
	  audio/framework/plugs/source/pc_spk_file.c \
	  audio/interface/player/pc_spk_player.c \

#endif

#if TCFG_USB_SLAVE_AUDIO_MIC_ENABLE
c_SRC_FILES += \
	  audio/framework/nodes/pc_mic_node.c \
	  audio/interface/recoder/pc_mic_recoder.c \

#endif

#if TCFG_PITCH_SPEED_NODE_ENABLE
c_SRC_FILES += \
	  audio/effect/audio_pitch_speed_api.c \

#endif



// Audio Effects params update demo
#if EXPORT_PLATFORM_AUDIO_EFFECT_DEMO_ENABLE
#if 0
c_SRC_FILES += \
      audio/effect/demo/autotune_demo.c \
      audio/effect/demo/bass_treble_demo.c \
      audio/effect/demo/chorus_demo.c \
      audio/effect/demo/crossover_demo.c \
      audio/effect/demo/drc_advance_demo.c \
      audio/effect/demo/drc_demo.c \
      audio/effect/demo/dynamic_eq_demo.c \
      audio/effect/demo/echo_demo.c \
      audio/effect/demo/energy_detect_demo.c \
      audio/effect/demo/eq_demo.c \
      audio/effect/demo/frequency_shift_howling_demo.c \
      audio/effect/demo/gain_demo.c \
      audio/effect/demo/harmonic_exciter_demo.c \
      audio/effect/demo/multiband_drc_demo.c \
      audio/effect/demo/noisegate_demo.c \
      audio/effect/demo/notch_howling_demo.c \
      audio/effect/demo/pitch_speed_demo.c \
      audio/effect/demo/reverb_advance_demo.c \
      audio/effect/demo/reverb_demo.c \
      audio/effect/demo/spectrum_demo.c \
      audio/effect/demo/stereo_widener_demo.c \
      audio/effect/demo/surround_demo.c \
      audio/effect/demo/virtual_bass_demo.c \
      audio/effect/demo/voice_changer_demo.c \
      audio/effect/demo/lhdc_x_demo.c \

#endif
#endif

c_SRC_FILES += \
	audio/cpu/common.c \




// *INDENT-OFF*

#if EXPORT_LE_AUDIO_SUPPORT

#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_AURACAST_SOURCE_EN | LE_AUDIO_JL_AURACAST_SOURCE_EN | LE_AUDIO_AURACAST_SINK_EN | LE_AUDIO_JL_AURACAST_SINK_EN))
c_SRC_FILES += \
	apps/common/le_audio/big.c
#endif

#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_UNICAST_SOURCE_EN | LE_AUDIO_JL_UNICAST_SOURCE_EN | LE_AUDIO_UNICAST_SINK_EN | LE_AUDIO_JL_UNICAST_SINK_EN))
c_SRC_FILES += \
	apps/common/le_audio/cig.c
#endif

#if TCFG_LE_AUDIO_APP_CONFIG
c_SRC_FILES += \
	apps/common/le_audio/wireless_trans_manager.c
#endif

#endif


// *INDENT-OFF*

c_SRC_FILES += \
	apps/common/config/bt_profile_config.c \


//------
#if TCFG_CFG_TOOL_ENABLE
c_SRC_FILES += \
	apps/common/config/new_cfg_tool.c \
	apps/common/config/cfg_tool_statistics_functions/cfg_tool_statistics.c
#if (TCFG_COMM_TYPE == TCFG_USB_COMM)
c_SRC_FILES += \
	apps/common/config/cfg_tool_cdc.c
#endif
#endif



#if (TCFG_ONLINE_ENABLE || TCFG_CFG_TOOL_ENABLE)
c_SRC_FILES += \
	apps/common/config/app_config.c

#if (TCFG_COMM_TYPE == TCFG_UART_COMM)
c_SRC_FILES += \
	apps/common/config/ci_transport_uart.c
#endif
#endif



#if TCFG_BT_NAME_SEL_BY_AD_ENABLE
c_SRC_FILES += \
	apps/common/config/bt_name_parse.c
#endif


#if 0
c_SRC_FILES += \
	apps/common/fat_nor/cfg_private.c

#endif


#if TCFG_DEBUG_DLOG_ENABLE
c_SRC_FILES += \
	apps/common/debug/dlog_config.c \
    apps/common/debug/dlog_output_config.c
#endif



#if TCFG_UPDATE_ENABLE
c_SRC_FILES += \
	apps/common/update/update.c \
	apps/common/update/testbox_update.c \
	apps/common/update/testbox_uart_update.c
#endif

#if (OTA_TWS_SAME_TIME_ENABLE && !OTA_TWS_SAME_TIME_NEW)
c_SRC_FILES += \
	apps/common/update/update_tws.c
#endif

#if (OTA_TWS_SAME_TIME_ENABLE && OTA_TWS_SAME_TIME_NEW && !OTA_TWS_SAME_TIME_NEW_LESS)
c_SRC_FILES += \
	apps/common/update/update_tws_new.c
#endif

#if (OTA_TWS_SAME_TIME_ENABLE && OTA_TWS_SAME_TIME_NEW && OTA_TWS_SAME_TIME_NEW_LESS)
c_SRC_FILES += \
	apps/common/update/update_tws_new_less.c
#endif

#ifdef CONFIG_UPDATE_MUTIL_CPU_UART
#if CONFIG_UPDATE_MUTIL_CPU_UART
c_SRC_FILES += \
	apps/common/update/uart_update_driver.c \
	apps/common/update/update_interactive_uart.c
#endif

#if (TCFG_UPDATE_UART_IO_EN) && (!TCFG_UPDATE_UART_ROLE)
c_SRC_FILES += \
	apps/common/update/uart_update.c
#endif

#if (TCFG_UPDATE_UART_IO_EN) && (TCFG_UPDATE_UART_ROLE)
c_SRC_FILES += \
	apps/common/update/uart_update_master.c
#endif
#endif

#if defined(TCFG_NORFLASH_DEV_ENABLE) && TCFG_NORFLASH_DEV_ENABLE
c_SRC_FILES += \
    apps/common/device/storage_device/norflash/norflash.c
#endif


// *INDENT-OFF*

// 注意：仅在BR27和BR28的soundbox工程开始才使用这个UI结构
//
// 其它工程和CPU使用原来的UI代码结构，后续逐渐再转到这里
//
// 防止出现兼容性问题导致客户SDK无法维护
//

// $(info make project ---> $(CPU) $(APP_CASE))


#if TCFG_PWMLED_ENABLE
c_SRC_FILES += \
	apps/common/ui/pwm_led/led_ui_api.c \
	apps/common/ui/pwm_led/led_ui_tws_sync.c
#endif




#if defined CONFIG_SOUNDBOX_CASE || defined CONFIG_DONGLE_CASE
#if (TCFG_UI_ENABLE && (CONFIG_UI_STYLE == STYLE_JL_LED7))
c_SRC_FILES += \
	apps/common/ui/led7/led7_ui_api.c
#endif


#if (TCFG_UI_ENABLE && (TCFG_SPI_LCD_ENABLE || TCFG_LCD_OLED_ENABLE) && (!TCFG_SIMPLE_LCD_ENABLE))
c_SRC_FILES += \
    apps/common/ui/lcd/lcd_ui_api.c
#endif

#if EXPORT_DOT_UI_ENABLE
#if TCFG_UI_ENABLE

#if TCFG_SPI_LCD_ENABLE
// SPI屏的驱动代码
#if TCFG_LCD_SPI_SH8601A_ENABLE
c_SRC_FILES += \
	apps/common/ui/lcd_drive/lcd_spi/lcd_spi_sh8601a_454x454.c
#endif

#if TCFG_LCD_SPI_ST7789V_ENABLE
c_SRC_FILES += \
	apps/common/ui/lcd_drive/lcd_spi/lcd_spi_st7789v_240x240.c
#endif

#if TCFG_OLED_SPI_SSD1306_ENABLE
c_SRC_FILES += \
	apps/common/ui/lcd_drive/lcd_spi/oled_spi_ssd1306_128x64.c
#endif
#endif


#if (TCFG_LCD_OLED_ENABLE || TCFG_SPI_LCD_ENABLE)
c_SRC_FILES += \
    apps/common/ui/interface/ui_platform.c \
    apps/common/ui/interface/ui_pushScreen_manager.c \
    apps/common/ui/interface/ui_resources_manager.c \
    apps/common/ui/interface/ui_synthesis_manager.c \
    apps/common/ui/interface/ui_synthesis_oled.c \
    apps/common/ui/interface/watch_bgp.c
#endif

#endif
#endif // EXPORT_DOT_UI_ENABLE
#endif // CONFIG_SOUNDBOX_CASE



// *INDENT-OFF*

#if TCFG_AI_PLAYER_ENABLE || TCFG_AI_RECORDER_ENABLE || TCFG_AI_TRANSLATOR_ENABLE
c_SRC_FILES += \
    apps/common/ai_audio/ai_audio_common.c \

#endif

#if TCFG_AI_PLAYER_ENABLE
c_SRC_FILES += \
    apps/common/ai_audio/ai_player.c \

#endif

#if TCFG_AI_RECORDER_ENABLE
c_SRC_FILES += \
    apps/common/ai_audio/ai_recorder.c \

#endif

#if TCFG_AI_TRANSLATOR_ENABLE
c_SRC_FILES += \
    apps/common/ai_audio/ai_translator.c \

#endif


// *INDENT-OFF*

#if THIRD_PARTY_PROTOCOLS_SEL || TCFG_LE_AUDIO_APP_CONFIG
c_SRC_FILES += \
	apps/common/third_party_profile/multi_protocol_common.c \
	apps/common/third_party_profile/multi_protocol_event.c \

#endif


#if (THIRD_PARTY_PROTOCOLS_SEL & (TME_EN | GMA_EN))
c_SRC_FILES += \
    apps/common/third_party_profile/interface/app_protocol_api.c \
    apps/common/third_party_profile/interface/app_protocol_common.c
#endif

#if (THIRD_PARTY_PROTOCOLS_SEL & GMA_EN)
c_SRC_FILES += \
	apps/common/third_party_profile/interface/app_protocol_gma.c \

#endif

#if APP_PROTOCOL_MMA_CODE
c_SRC_FILES += \
	apps/common/third_party_profile/interface/app_protocol_mma.c
#endif

#if 0
c_SRC_FILES += \
    apps/common/third_party_profile/interface/app_protocol_ota.c
#endif

#if (THIRD_PARTY_PROTOCOLS_SEL & TME_EN)
c_SRC_FILES += \
	apps/common/third_party_profile/interface/app_protocol_tme.c
#endif

#if (THIRD_PARTY_PROTOCOLS_SEL & CUSTOM_DEMO_EN)
c_SRC_FILES += \
    apps/common/third_party_profile/custom_protocol_demo/custom_protocol.c
#endif

#if (THIRD_PARTY_PROTOCOLS_SEL & ANCS_AMS_MODE_EN)
c_SRC_FILES += \
    apps/common/third_party_profile/app_ble_ancs_ams/app_ble_ancs_ams.c
#endif

#if (THIRD_PARTY_PROTOCOLS_SEL & MULTI_CLIENT_EN)
c_SRC_FILES += \
    apps/common/third_party_profile/multi_ble_client/ble_multi_client.c
#endif

#if (THIRD_PARTY_PROTOCOLS_SEL & SWIFT_PAIR_EN)
c_SRC_FILES += \
    apps/common/third_party_profile/swift_pair/swift_pair_protocol.c
#endif


#if (THIRD_PARTY_PROTOCOLS_SEL & GFPS_EN)
c_SRC_FILES += \
    apps/common/third_party_profile/gfps_protocol/gfps_protocol.c
#endif


#if (THIRD_PARTY_PROTOCOLS_SEL & MMA_EN)
c_SRC_FILES += \
    apps/common/third_party_profile/mma_protocol/mma_config.c \
    apps/common/third_party_profile/mma_protocol/mma_platform_api.c \
    apps/common/third_party_profile/mma_protocol/mma_protocol.c \

#endif

#if EXPORT_TENCENT_LL_ENABLE
#if (THIRD_PARTY_PROTOCOLS_SEL & LL_SYNC_EN)
c_SRC_FILES += \
    apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_import.c \
    apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_llsync_data.c \
    apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_llsync_device.c \
    apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_llsync_event.c \
    apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_llsync_ota.c \
    apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_service.c \
    apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_utils_base64.c \
    apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_utils_crc.c \
    apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_utils_hmac.c \
    apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_utils_log.c \
    apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_utils_md5.c \
    apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_utils_sha1.c \
    apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_template.c \
    apps/common/third_party_profile/Tencent_LL/tencent_ll_demo/ll_demo.c \
    apps/common/third_party_profile/Tencent_LL/tencent_ll_demo/ll_task.c \
    apps/common/third_party_profile/Tencent_LL/tencent_ll_demo/ll_sync_event_handler.c \

#if 1
c_SRC_FILES += \
	apps/common/third_party_profile/Tencent_LL/tencent_ll_demo/jieli_iot_functions/ble_iot_utils.c \
	apps/common/third_party_profile/Tencent_LL/tencent_ll_demo/jieli_iot_functions/ble_iot_msg_manager.c \
	apps/common/third_party_profile/Tencent_LL/tencent_ll_demo/jieli_iot_functions/ble_iot_anc_manager.c \
	apps/common/third_party_profile/Tencent_LL/tencent_ll_demo/jieli_iot_functions/ble_iot_power_manager.c \
	apps/common/third_party_profile/Tencent_LL/tencent_ll_demo/jieli_iot_functions/ble_iot_tws_manager.c \

#endif


#endif
#endif

#if (THIRD_PARTY_PROTOCOLS_SEL & TUYA_DEMO_EN)
c_SRC_FILES += \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/apps/tuyaos_demo_ble_peripheral/src/app_dp_parser.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/apps/tuyaos_demo_ble_peripheral/src/app_key.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/apps/tuyaos_demo_ble_peripheral/src/app_led.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/apps/tuyaos_demo_ble_peripheral/src/app_product_test.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/apps/tuyaos_demo_ble_peripheral/src/tuya_ble_protocol_callback.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/apps/tuyaos_demo_ble_peripheral/src/tuya_sdk_callback.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/apps/tuyaos_demo_ble_peripheral/src/tuya_app_func.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/apps/tuyaos_demo_ble_peripheral/src/tuya_sys_event.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/apps/tuyaos_demo_ble_peripheral/src/tuya_ota.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/app_tuya_driver/src/tal_adc.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/app_tuya_driver/src/tal_gpio.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/app_tuya_driver/src/tal_i2c.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/app_tuya_driver/src/tal_pwm.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/app_tuya_driver/src/tal_spi.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_app_passthrough/src/tal_ble_app_passthrough.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_attach_ota/src/tuya_ble_attach_ota.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_attach_ota/src/tuya_ble_attach_ota_port.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_beacon/src/tal_ble_beacon.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_bulkdata/src/tuya_ble_bulkdata.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_bulkdata/src/tuya_ble_bulkdata_demo.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_file/src/tal_ble_file.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_iot_channel/src/tuya_ble_iot_channel.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_ota/src/tuya_ble_ota.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_product_test/src/tal_gpio_test.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_product_test/src/tuya_ble_product_test.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_product_test/src/tuya_ble_product_test_over_air.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_protocol/src/tuya_ble_api.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_protocol/src/tuya_ble_data_handler.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_protocol/src/tuya_ble_event.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_protocol/src/tuya_ble_event_handler.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_protocol/src/tuya_ble_feature_accessory.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_protocol/src/tuya_ble_gatt_send_queue.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_protocol/src/tuya_ble_main.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_protocol/src/tuya_ble_mem.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_protocol/src/tuya_ble_mutli_tsf_protocol.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_protocol/src/tuya_ble_port.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_protocol/src/tuya_ble_queue.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_protocol/src/tuya_ble_storage.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_scene/src/tuya_ble_scene.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_uart_common/src/tuya_ble_uart_common.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_ble_weather/src/tuya_ble_weather.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_bluetooth/src/tal_bluetooth.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_driver/src/tal_flash.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_driver/src/tal_rtc.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_driver/src/tal_uart.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_driver/src/tal_watchdog.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_feature_ext_module/src/tal_ble_x_demo.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_feature_ext_module/src/tal_feature_ext_module.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_key/src/tal_key.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_local_timer/src/tal_local_timer.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_local_timer/src/tal_local_timer_storage.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_oled/src/tal_oled.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_repeater/src/tal_repeater.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_sdk_test/src/tal_sdk_test.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_system/src/tal_log.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_system/src/tal_memory.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_system/src/tal_ota.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_system/src/tal_sleep.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_system/src/tal_sw_timer.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_system/src/tal_system.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_utc/src/tal_utc.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/components/tal_util/src/tal_util.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/vendor/jl_ble/tuyaos/bluetooth/tkl_bluetooth.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/vendor/jl_ble/tuyaos/drivers/tkl_adc.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/vendor/jl_ble/tuyaos/drivers/tkl_flash.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/vendor/jl_ble/tuyaos/drivers/tkl_gpio.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/vendor/jl_ble/tuyaos/drivers/tkl_i2c.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/vendor/jl_ble/tuyaos/drivers/tkl_pwm.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/vendor/jl_ble/tuyaos/drivers/tkl_rtc.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/vendor/jl_ble/tuyaos/drivers/tkl_spi.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/vendor/jl_ble/tuyaos/drivers/tkl_uart.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/vendor/jl_ble/tuyaos/drivers/tkl_watchdog.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/vendor/jl_ble/tuyaos/system/tkl_memory.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/vendor/jl_ble/tuyaos/system/tkl_ota.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/vendor/jl_ble/tuyaos/system/tkl_sleep.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/vendor/jl_ble/tuyaos/system/tkl_system.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/vendor/jl_ble/tuyaos/system/tkl_thread.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/vendor/jl_ble/tuyaos/system/tkl_queue.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/vendor/jl_ble/tuyaos/system/tkl_wakeup.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/vendor/jl_ble/tuyaos/utilities/tuya_list.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/vendor/jl_ble/tuyaos/utilities/tuya_mem_heap.c \
    apps/common/third_party_profile/tuyaos_protocol/TuyaOS/vendor/jl_ble/tuyaos/utilities/tuya_queue.c \

#endif


#if (THIRD_PARTY_PROTOCOLS_SEL & ANCS_CLIENT_EN)
c_SRC_FILES += \
    apps/common/third_party_profile/ancs_client_demo/le_ancs_client.c
#endif


#if (BT_MIC_EN)
c_SRC_FILES += \
	apps/common/third_party_profile/common/mic_rec.c
#endif


#if RCSP_ADV_EN || RCSP_BTMATE_EN || (RCSP_MODE || SMART_BOX_EN) || APP_PROTOCOL_READ_CFG_EN
c_SRC_FILES += \
	apps/common/third_party_profile/common/custom_cfg.c
#endif

#if (THIRD_PARTY_PROTOCOLS_SEL & REALME_EN)
c_SRC_FILES += \
    apps/common/third_party_profile/realme_protocol/realme_config.c \
    apps/common/third_party_profile/realme_protocol/realme_platform_api.c \
    apps/common/third_party_profile/realme_protocol/realme_protocol.c \

#endif

#if (THIRD_PARTY_PROTOCOLS_SEL & RCSP_MODE_EN)
c_SRC_FILES += \
    apps/common/third_party_profile/jieli/rcsp/adv/ble_rcsp_adv.c \
	apps/common/third_party_profile/jieli/rcsp/ble_rcsp_client.c \
	apps/common/third_party_profile/jieli/rcsp/ble_rcsp_multi_common.c \
	apps/common/third_party_profile/jieli/rcsp/ble_rcsp_server.c \

#if RCSP_BLE_MASTER
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/client/rcsp_c_cmd_recieve.c \
	apps/common/third_party_profile/jieli/rcsp/client/rcsp_c_cmd_recieve_no_response.c \
	apps/common/third_party_profile/jieli/rcsp/client/rcsp_c_cmd_response.c

#if RCSP_UPDATE_EN
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/client/rcsp_m_update/rcsp_update_master.c
#endif
#endif


c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp.c \
	apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp_bt_manage.c \
	apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp_config.c \
	apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp_event.c \
	apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp_manage.c \
	apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp_task.c \

#if RCSP_FILE_OPT
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/browser/rcsp_browser.c
#endif

c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_bt_func.c \

#if (TCFG_APP_FM_EN)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_fm_func.c
#endif

#if TCFG_APP_LINEIN_EN && !SOUNDCARD_ENABLE
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_linein_func.c
#endif

#if (TCFG_APP_MUSIC_EN)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_music_func.c
#endif

#if (TCFG_APP_PC_EN && TCFG_USB_SLAVE_AUDIO_SPK_ENABLE)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_pc_func.c
#endif

#if (TCFG_APP_RTC_EN && RCSP_APP_RTC_EN)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_rtc_func.c
#endif


#if (TCFG_APP_SPDIF_EN)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_spdif_func.c
#endif

c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/rcsp_device_feature.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/rcsp_device_status.c \

#if (JL_RCSP_EXTRA_FLASH_OPT)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/external_flash/rcsp_extra_flash_cmd.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/external_flash/rcsp_extra_flash_opt.c
#endif


#if TCFG_DEV_MANAGER_ENABLE
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/dev_format.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/file_bluk_trans_prepare.c
#endif

#if (TCFG_DEV_MANAGER_ENABLE && JL_RCSP_SIMPLE_TRANSFER)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/file_simple_transfer.c
#endif

#if (TCFG_DEV_MANAGER_ENABLE && RCSP_FILE_OPT)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/file_delete.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/file_trans_back.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/file_transfer.c \
    apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/file_transfer_sync.c
#endif

c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/rcsp_setting_opt.c \



#if TCFG_USER_TWS_ENABLE
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/rcsp_setting_sync.c
#endif

#if TCFG_RCSP_DUAL_CONN_ENABLE
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_1t2_setting.c
#endif

#if (RCSP_ADV_ANC_VOICE && RCSP_ADV_ADAPTIVE_NOISE_REDUCTION)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_adaptive_noise_reduction.c
#endif

#if RCSP_ADV_AI_NO_PICK
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_ai_no_pick.c
#endif

#if RCSP_ADV_ANC_VOICE
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_anc_voice.c
#endif

#if RCSP_ADV_EN
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/rcsp_adv_bluetooth.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_time_stamp_setting.c
#endif

#if (RCSP_ADV_EN && RCSP_ADV_KEY_SET_ENABLE)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_key_setting.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_anc_voice_key.c
#endif

#if (RCSP_ADV_EN && RCSP_ADV_NAME_SET_ENABLE)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_bt_name_setting.c
#endif
#if (RCSP_ADV_EN && RCSP_ADV_ASSISTED_HEARING)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_hearing_aid_setting.c
#endif


#if (RCSP_ADV_EN && RCSP_ADV_LED_SET_ENABLE)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_led_setting.c
#endif

#if (RCSP_ADV_EN &&  RCSP_ADV_MIC_SET_ENABLE)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_mic_setting.c
#endif

#if (RCSP_ADV_ANC_VOICE && RCSP_ADV_SCENE_NOISE_REDUCTION)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_scene_noise_reduction.c
#endif


#if (RCSP_ADV_ANC_VOICE && RCSP_ADV_WIND_NOISE_DETECTION)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_voice_enhancement_mode.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_wind_noise_detection.c
#endif


#if (RCSP_ADV_EN && RCSP_ADV_WORK_SET_ENABLE)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_work_setting.c
#endif

#if (RCSP_ADV_COLOR_LED_SET_ENABLE)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_color_led_setting.c
#endif

#if (TCFG_EQ_ENABLE && RCSP_ADV_EQ_SET_ENABLE)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_eq_setting.c
#endif

#if ( RCSP_ADV_HIGH_LOW_SET && TCFG_BASS_TREBLE_NODE_ENABLE)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_high_low_vol_setting.c
#endif

#if (TCFG_EQ_ENABLE && MIC_EFFECT_EQ_EN && RCSP_ADV_KARAOKE_EQ_SET_ENABLE && RCSP_ADV_KARAOKE_SET_ENABLE)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_karaoke_eq_setting.c
#endif

#if (SOUNDCARD_ENABLE && RCSP_ADV_KARAOKE_SET_ENABLE)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_karaoke_setting.c
#endif

#if (RCSP_DRC_VAL_SETTING && TCFG_LIMITER_NODE_ENABLE && RCSP_ADV_EQ_SET_ENABLE)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_misc_drc_setting.c
#endif

#if (RCSP_REVERBERATION_SETTING && TCFG_MIC_EFFECT_ENABLE && RCSP_ADV_EQ_SET_ENABLE)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_misc_reverbration_setting.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_misc_setting.c
#endif

#if (RCSP_ADV_MUSIC_INFO_ENABLE)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_music_info_setting.c
#endif

#if (RCSP_ADV_EQ_SET_ENABLE)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_vol_setting.c
#endif



c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_switch_device.c \

#if (RCSP_UPDATE_EN && !RCSP_BLE_MASTER)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_update/rcsp_ch_loader_download.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_update/rcsp_update.c
#endif

#if ((RCSP_MODE == RCSP_MODE_SOUNDBOX) && OTA_TWS_SAME_TIME_ENABLE)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_update/rcsp_update_tws.c
#endif


#if (JL_RCSP_SENSORS_DATA_OPT && JL_RCSP_NFC_DATA_OPT)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/nfc_data_opt.c
#endif

#if (JL_RCSP_SENSORS_DATA_OPT)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sensor_log_notify.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sensors_data_opt.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_func.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_info_opt/sport_info_bt_disconn.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_info_opt/sport_info_continuous_heart_rate.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_info_opt/sport_info_exercise_heart_rate.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_info_opt/sport_info_fall_detection.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_info_opt/sport_info_personal_info.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_info_opt/sport_info_pressure_detection.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_info_opt/sport_info_raise_wrist.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_info_opt/sport_info_sedentary.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_info_opt/sport_info_sensor_opt.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_info_opt/sport_info_sleep_detection.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_opt/sport_data_air_pressure.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_opt/sport_data_altitude.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_opt/sport_data_blood_oxygen.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_opt/sport_data_exercise_recovery_time.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_opt/sport_data_exercise_steps.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_opt/sport_data_heart_rate.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_opt/sport_data_max_oxygen_uptake.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_opt/sport_data_pressure_detection.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_opt/sport_data_sports_information.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_opt/sport_data_training_load.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_info_opt.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_info_sync.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_info_vm.c
#endif

#if RCSP_ADV_TRANSLATOR
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/functions/translator/rcsp_translator.c
#endif

c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/rcsp_cmd_recieve.c \
	apps/common/third_party_profile/jieli/rcsp/server/rcsp_cmd_recieve_no_respone.c \
	apps/common/third_party_profile/jieli/rcsp/server/rcsp_cmd_respone.c \
	apps/common/third_party_profile/jieli/rcsp/server/rcsp_cmd_user.c \
	apps/common/third_party_profile/jieli/rcsp/server/rcsp_command.c \
	apps/common/third_party_profile/jieli/rcsp/server/rcsp_data_recieve.c \
	apps/common/third_party_profile/jieli/rcsp/server/rcsp_data_recieve_no_respone.c \
	apps/common/third_party_profile/jieli/rcsp/server/rcsp_data_respone.c \

c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/rcsp_interface.c \
	apps/common/third_party_profile/jieli/rcsp/rcsp_simplified/rcsp_interface_simplified.c \
    apps/common/third_party_profile/jieli/rcsp/server/functions/mass_data/mass_data.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/pub_mutual_set_cmd/pub_mutual_set_cmd_opt.c \

#endif


#if TCFG_THIRD_PARTY_PROTOCOLS_ENABLE && (THIRD_PARTY_PROTOCOLS_SEL & TRANS_DATA_EN)
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/trans_data_demo/le_trans_data.c \
	apps/common/third_party_profile/jieli/trans_data_demo/spp_trans_data.c
#endif


#if APP_ONLINE_DEBUG
c_SRC_FILES += \
	apps/common/third_party_profile/jieli/online_db/spp_online_db.c \
	apps/common/third_party_profile/jieli/online_db/online_db_deal.c
#endif

#if (THIRD_PARTY_PROTOCOLS_SEL & AURACAST_APP_EN)
c_SRC_FILES += \
    apps/common/third_party_profile/auracast_app/auracast_app_protocol.c \
    apps/common/third_party_profile/auracast_app/auracast_app_ble.c \
    apps/common/third_party_profile/auracast_app/auracast_app_gatt_over_edr.c \
    apps/common/third_party_profile/auracast_app/auracast_app_source_api.c \
    apps/common/third_party_profile/auracast_app/auracast_app_sink_api.c \

#endif

#if EXPORT_FNMA_ENABLE
#if (THIRD_PARTY_PROTOCOLS_SEL & FMNA_EN)
c_SRC_FILES += \
    apps/common/third_party_profile/bt_fmy/ble_fmy.c \
    apps/common/third_party_profile/bt_fmy/ble_fmy_fmna.c \
    apps/common/third_party_profile/bt_fmy/ble_fmy_ota.c \
    apps/common/third_party_profile/bt_fmy/ble_fmy_modet.c \

#endif
#endif

#if (THIRD_PARTY_PROTOCOLS_SEL & DMA_EN)
c_SRC_FILES += \
    apps/common/third_party_profile/dma_protocol/dma_use_lib/dma_le_port.c \
    apps/common/third_party_profile/dma_protocol/dma_use_lib/dma_setting.c \
    apps/common/third_party_profile/dma_protocol/dma_use_lib/dma_spp_port.c \
    apps/common/third_party_profile/dma_protocol/dma_use_lib/platform_template.c \
    apps/common/third_party_profile/dma_protocol/dma_protocol.c \

#endif

#if (THIRD_PARTY_PROTOCOLS_SEL & XIMALAYA_EN)
c_SRC_FILES += \
    apps/common/third_party_profile/ximalaya_protocol/xmly_protocol.c \
    apps/common/third_party_profile/ximalaya_protocol/xmly_platform_api.c \
    apps/common/third_party_profile/ximalaya_protocol/xmly_config.c \
    apps/common/third_party_profile/ximalaya_protocol/xmly_ble.c \

#endif


#if THIRD_PARTY_PROTOCOLS_SEL & JL_SBOX_EN
c_SRC_FILES += \
    apps/common/third_party_profile/jieli/jl_earbox/sbox_core_config.c \
    apps/common/third_party_profile/jieli/jl_earbox/sbox_protocol.c \
    apps/common/third_party_profile/jieli/jl_earbox/sbox_user_app.c \
    apps/common/third_party_profile/jieli/jl_earbox/sbox_uart_app.c \
    apps/common/third_party_profile/jieli/jl_earbox/sbox_eq_switch.c \
    apps/common/third_party_profile/jieli/jl_earbox/user_video_ctr.c \
    apps/common/third_party_profile/jieli/jl_earbox/edr_hid_user.c \
    apps/common/third_party_profile/jieli/jl_earbox/sbox_connect_emitter.c \

//apps/common/third_party_profile/jieli/jl_earbox/sbox_key_setting.c \

#endif

// *INDENT-OFF*

#ifdef CONFIG_SOUNDBOX_CASE
#if TCFG_USB_APPLE_DOCK_EN
c_SRC_FILES += \
    apps/common/iap/iAP_chip.c \
    apps/common/iap/iAP_des.c \
    apps/common/iap/iAP_device.c \
    apps/common/iap/iAP_iic.c \

#endif
#endif



// *INDENT-OFF*


#if TCFG_EAR_DETECT_ENABLE
c_SRC_FILES += \
	apps/common/device/in_ear_detect/in_ear_detect.c \
	apps/common/device/in_ear_detect/in_ear_manage.c
#endif



#ifdef CONFIG_EARPHONE_CASE
#if TCFG_OUTSIDE_EARTCH_ENABLE
c_SRC_FILES += \
	apps/common/device/eartouch/eartouch_manage.c \
	apps/common/device/eartouch/eartouch_iic_interface.c
#if (TCFG_EARTCH_SENSOR_SEL == EARTCH_HX300X)
c_SRC_FILES += \
	apps/common/device/eartouch/hx300x/hx300x_driver.c
#endif
#endif

#if CFG_EARTCH_OUTSIDE_TOUCH_ENABLE
c_SRC_FILES += \
	apps/common/device/eartouch/outside_tch_driver.c
#endif
#endif


#if TCFG_NANDFLASH_DEV_ENABLE
c_SRC_FILES += \
	apps/common/device/storage_device/nandflash/nandflash.c \
	apps/common/device/storage_device/nandflash/nandflash_ftl.c
#endif


#ifdef CONFIG_SOUNDBOX_CASE
#if TCFG_APP_FM_EN
c_SRC_FILES += \
	apps/common/device/fm/fm_manage.c

#if TCFG_FM_INSIDE_ENABLE
c_SRC_FILES += \
	apps/common/device/fm/fm_inside/fm_inside.c
#endif

#if TCFG_FM_OUTSIDE_ENABLE

#if TCFG_FM_BK1080_ENABLE
c_SRC_FILES += \
	apps/common/device/fm/bk1080/Bk1080.c
#endif

#if TCFG_FM_QN8035_ENABLE
c_SRC_FILES += \
	apps/common/device/fm/qn8035/QN8035.c
#endif

#if TCFG_FM_RDA5807_ENABLE
c_SRC_FILES += \
	apps/common/device/fm/rda5807/RDA5807.c
#endif

#endif
#endif
#endif


// *INDENT-OFF*

#if NTC_DET_EN
c_SRC_FILES += \
	apps/common/device/sensor/ntc/ntc_det.c
#endif

#if(TCFG_IRSENSOR_ENABLE == 1)
c_SRC_FILES += \
	apps/common/device/sensor/ir_sensor/ir_manage.c

#if(TCFG_JSA1221_ENABLE  == 1)
c_SRC_FILES += \
	apps/common/device/sensor/ir_sensor/jsa1221.c
#endif
#endif

#ifdef CONFIG_EARPHONE_CASE
#if TCFG_IMUSENSOR_ENABLE
c_SRC_FILES += \
	apps/common/device/sensor/imu_sensor/imuSensor_manage.c \

#if TCFG_SH3001_ENABLE
c_SRC_FILES += \
	apps/common/device/sensor/imu_sensor/sh3001/sh3001.c
#endif

#if TCFG_MPU6887P_ENABLE
c_SRC_FILES += \
	apps/common/device/sensor/imu_sensor/mpu6887/mpu6887p.c
#endif

#if TCFG_TP_MPU9250_ENABLE
c_SRC_FILES += \
	apps/common/device/sensor/imu_sensor/mpu9250/mpu9250.c
#endif

#if TCFG_QMI8658_ENABLE
c_SRC_FILES += \
	apps/common/device/sensor/imu_sensor/qmi8658/qmi8658c.c
#endif

#if TCFG_LSM6DSL_ENABLE
c_SRC_FILES += \
	apps/common/device/sensor/imu_sensor/lsm6dsl/lsm6dsl.c
#endif

#if TCFG_ICM42670P_ENABLE
c_SRC_FILES += \
	apps/common/device/sensor/imu_sensor/icm_42670p/icm_42670p.c \
	apps/common/device/sensor/imu_sensor/icm_42670p/inv_imu_transport.c \
	apps/common/device/sensor/imu_sensor/icm_42670p/inv_imu_apex.c \
	apps/common/device/sensor/imu_sensor/icm_42670p/inv_imu_driver.c
#endif

#endif
#endif


// *INDENT-OFF*

c_SRC_FILES += \
	apps/common/device/key/key_driver.c

#if TCFG_IOKEY_ENABLE
c_SRC_FILES += \
	apps/common/device/key/iokey.c
#endif

#if TCFG_IRKEY_ENABLE
c_SRC_FILES += \
	apps/common/device/key/irkey.c
#endif

#if TCFG_ADKEY_ENABLE
c_SRC_FILES += \
	apps/common/device/key/adkey.c
#endif


#if TCFG_ADKEY_RTCVDD_ENABLE
c_SRC_FILES += \
	apps/common/device/key/adkey_rtcvdd.c
#endif

#if TCFG_TOUCH_KEY_ENABLE
c_SRC_FILES += \
	apps/common/device/key/touch_key.c
#endif

#if TCFG_CTMU_TOUCH_KEY_ENABLE
c_SRC_FILES += \
	apps/common/device/key/ctmu_touch_key.c
#endif

#if 0
c_SRC_FILES += \
	apps/common/device/key/uart_key.c
#endif

// *INDENT-OFF*

#if EXPORT_CONFIG_USB_ENABLE
//usb slave
#if TCFG_USB_SLAVE_ENABLE
c_SRC_FILES += \
	apps/common/device/usb/usb_config.c \
	apps/common/device/usb/device/descriptor.c \
	apps/common/device/usb/device/usb_device.c \
	apps/common/device/usb/device/user_setup.c \
	apps/common/device/usb/device/task_pc.c \

#endif

c_SRC_FILES += \
	apps/common/device/usb/device/usb_pll_trim.c \

// mass storage
#if TCFG_USB_SLAVE_MSD_ENABLE
c_SRC_FILES += \
	apps/common/device/usb/device/msd.c \

#endif

c_SRC_FILES += \
	apps/common/device/usb/device/msd_upgrade.c \

// mtp
#if TCFG_USB_SLAVE_MTP_ENABLE
c_SRC_FILES += \
	apps/common/device/usb/device/mtp.c \

#endif

// hid
#if TCFG_USB_SLAVE_HID_ENABLE
c_SRC_FILES += \
	apps/common/device/usb/device/hid.c \

#endif

#if TCFG_USB_CUSTOM_HID_ENABLE
c_SRC_FILES += \
	apps/common/device/usb/device/custom_hid.c \
	apps/common/device/usb/device/rcsp_hid_inter.c \

#endif


// audio
#if TCFG_USB_SLAVE_AUDIO_SPK_ENABLE || TCFG_USB_SLAVE_AUDIO_MIC_ENABLE
c_SRC_FILES += \
	apps/common/device/usb/device/uac_stream.c

#if (USB_AUDIO_VERSION == USB_AUDIO_VERSION_1_0)
c_SRC_FILES += \
	apps/common/device/usb/device/uac1.c
#endif

#if (USB_AUDIO_VERSION == USB_AUDIO_VERSION_2_0)
c_SRC_FILES += \
	apps/common/device/usb/device/uac2.c
#endif

#endif



#if TCFG_USB_SLAVE_CDC_ENABLE
c_SRC_FILES += \
    apps/common/device/usb/device/cdc.c \

#endif

// midi
#if TCFG_USB_SLAVE_MIDI_ENABLE
c_SRC_FILES += \
    apps/common/device/usb/device/midi.c \

#endif


// printer
#if TCFG_USB_SLAVE_PRINTER_ENABLE
c_SRC_FILES += \
    apps/common/device/usb/device/printer.c \

#endif

// usb host
#if TCFG_USB_HOST_ENABLE
c_SRC_FILES += \
	apps/common/device/usb/usb_host_config.c \
	apps/common/device/usb/host/usb_bulk_transfer.c \
	apps/common/device/usb/host/usb_ctrl_transfer.c \
	apps/common/device/usb/host/usb_host.c \

#endif


// udisk
#if TCFG_UDISK_ENABLE
c_SRC_FILES += \
	apps/common/device/usb/host/usb_storage.c

#endif

// gamepad
#if TCFG_ADB_ENABLE
c_SRC_FILES += \
	apps/common/device/usb/host/adb.c \

#endif


#if TCFG_AOA_ENABLE
c_SRC_FILES += \
    apps/common/device/usb/host/aoa.c \

#endif


#if TCFG_HID_HOST_ENABLE
c_SRC_FILES += \
    apps/common/device/usb/host/hid.c \

#endif

// usb host audio
#if TCFG_HOST_AUDIO_ENABLE
c_SRC_FILES += \
    apps/common/device/usb/host/uac_host.c \

#endif



#if TCFG_USB_SLAVE_ENABLE || TCFG_USB_HOST_ENABLE
c_SRC_FILES += \
    apps/common/device/usb/usb_epbuf_manager.c \
    apps/common/device/usb/usb_task.c \

#endif


// demo
#if 0
c_SRC_FILES += \
    apps/common/device/usb/demo/host/udisk_block_test.c
#endif


#endif



// *INDENT-OFF*

c_SRC_FILES += \
	  cpu/components/iic_soft.c \
	  cpu/components/iic_api.c \
	  cpu/components/ir_encoder.c \
	  cpu/components/ir_decoder.c \
	  cpu/components/rdec_soft.c \

#if TCFG_PWMLED_ENABLE
c_SRC_FILES += \
	  cpu/components/led_api.c \
	  cpu/components/two_io_led.c \

#endif

#if TCFG_LP_TOUCH_KEY_ENABLE
c_SRC_FILES += \
	  cpu/components/touch/lp_touch_key_tool.c \

#endif

// *INDENT-OFF*

c_SRC_FILES += \
    cpu/config/gpio_file_parse.c \
    cpu/config/lib_power_config.c \

// *INDENT-OFF*

#if 0
c_SRC_FILES += \
	  cpu/periph_demo/iic_master_demo.c \
	  cpu/periph_demo/iic_slave_demo.c \
	  cpu/periph_demo/ledc_test.c \
	  cpu/periph_demo/sd_test.c \
	  cpu/periph_demo/led_api_test.c \
	  cpu/periph_demo/pwm_led_test.c \
	  cpu/periph_demo/two_io_led_test.c \
	  cpu/periph_demo/spi_test.c \
	  cpu/periph_demo/uart_test.c \
	  cpu/periph_demo/gptimer_demo.c \
	  cpu/periph_demo/rdec_soft_demo.c \
	  cpu/periph_demo/ir_encoder_decoder_demo.c \

#endif


// *INDENT-OFF*

c_SRC_FILES += \
	  audio/cpu/br28/audio_setup.c \
	  audio/cpu/br28/audio_mic_capless.c \
	  audio/cpu/br28/audio_dai/audio_pdm.c \
	  audio/cpu/br28/audio_dai/audio_tdm.c \
	  audio/cpu/br28/audio_config.c \
	  audio/cpu/br28/audio_configs_dump.c \

#if CONFIG_ANC_ENABLE
c_SRC_FILES += \
	  audio/cpu/br28/audio_anc.c \
	  audio/cpu/br28/icsd_anc_user.c
#endif


//Audio Accelerator
c_SRC_FILES += \
	  audio/cpu/br28/audio_accelerator/hw_fft.c \

#if TCFG_SMART_VOICE_ENABLE
c_SRC_FILES += \
	audio/cpu/br28/audio_vad/vad_mic.c \
	audio/cpu/br28/audio_vad/vad_clock_trim.c
#endif

//Audio Demo Files
c_SRC_FILES += \
	  audio/cpu/br28/audio_demo/audio_adc_demo.c \

#if 1
c_SRC_FILES += \
	  audio/cpu/br28/audio_demo/audio_dac_demo.c \
	  audio/cpu/br28/audio_demo/audio_fft_demo.c \
	  audio/cpu/br28/audio_demo/audio_pdm_demo.c

#endif


// *INDENT-OFF*

c_SRC_FILES += \
	  cpu/br28/setup.c \
	  cpu/br28/overlay_code.c \
	  cpu/br28/private_iis.c \

#if 0
c_SRC_FILES += \
	  cpu/br28/lua_port_api.c
#endif

#if 1//TCFG_CHARGE_ENABLE
c_SRC_FILES += \
	  cpu/br28/charge/charge.c \
      cpu/br28/charge/charge_config.c
#endif

#if (TCFG_CHARGESTORE_ENABLE || TCFG_TEST_BOX_ENABLE || TCFG_ANC_BOX_ENABLE)
c_SRC_FILES += \
	  cpu/br28/charge/chargestore.c \
	  cpu/br28/charge/chargestore_config.c
#endif

#ifdef CONFIG_SOUNDBOX_CASE
c_SRC_FILES += \
      cpu/br28/ui_driver/led7/led7_driver.c \
      cpu/br28/ui_driver/ui_common.c
#endif

#if TCFG_TOUCH_KEY_ENABLE
c_SRC_FILES += \
	  cpu/br28/periph/touch/plcnt.c
#endif
#if TCFG_LP_TOUCH_KEY_ENABLE
c_SRC_FILES += \
	  cpu/br28/periph/touch/lp_touch_key.c \
	  cpu/br28/periph/touch/lpctmu_hw.c
#endif

c_SRC_FILES += \
	  cpu/br28/periph/rdec.c \
	  cpu/br28/periph/mcpwm.c \

#if TCFG_PWMLED_ENABLE
c_SRC_FILES += \
	  cpu/components/pwm_led_v1.c
#endif

c_SRC_FILES += \
		cpu/br28/power/key_wakeup.c \

c_SRC_FILES += \
	  cpu/br28/power/power_app.c \
	  cpu/br28/power/power_config.c \
	  cpu/br28/power/power_port.c \

#if EXPORT_LCD_DRIVER_ENABLE
#if TCFG_UI_ENABLE

#if TCFG_SPI_LCD_ENABLE
c_SRC_FILES += \
    cpu/$(CPU_DIR)/ui_driver/interface/ui_platform.c
#endif
#if (TCFG_SPI_LCD_ENABLE || TCFG_LCD_OLED_ENABLE)
c_SRC_FILES += \
    cpu/$(CPU_DIR)/ui_driver/lcd_drive/lcd_drive.c
#endif
#if TCFG_LCD_SPI_ST7789V_ENABLE
c_SRC_FILES += \
    cpu/$(CPU_DIR)/ui_driver/lcd_drive/lcd_spi/lcd_spi_st7789v_240x240.c
#endif
#if TCFG_LCD_SPI_SH8601A_ENABLE
c_SRC_FILES += \
    cpu/$(CPU_DIR)/ui_driver/lcd_drive/lcd_spi/lcd_spi_sh8601a_454x454.c
#endif
#if TCFG_LCD_SPI_GC9B71_ENABLE
c_SRC_FILES += \
    cpu/$(CPU_DIR)/ui_driver/lcd_drive/lcd_spi/lcd_spi_gc9b71_320x386.c
#endif

#endif
#endif

// *INDENT-OFF*

#if TCFG_APP_MUSIC_EN || TCFG_MIX_RECORD_ENABLE || TCFG_NANDFLASH_DEV_ENABLE
c_SRC_FILES += \
	apps/common/dev_manager/dev_reg.c \
	apps/common/dev_manager/dev_update.c \
	apps/earphone/mode/common/dev_status.c \

#endif

#if TCFG_APP_MUSIC_EN
c_SRC_FILES += \
	apps/common/music/breakpoint.c \
	apps/common/music/music_decrypt.c \
	apps/common/music/music_id3.c \
	apps/common/music/music_player.c \
	apps/common/file_operate/file_manager.c \
	apps/earphone/mode/music/music.c \
	apps/earphone/mode/music/music_key_msg_table.c \
	apps/earphone/mode/music/music_app_msg_handler.c \

#endif

#if TCFG_APP_LINEIN_EN
c_SRC_FILES += \
	apps/earphone/mode/linein/linein.c \
	apps/earphone/mode/linein/linein_dev.c \
	apps/earphone/mode/linein/linein_key_msg_table.c \

#endif

#if TCFG_APP_PC_EN
c_SRC_FILES += \
	apps/earphone/mode/pc/pc.c \
	apps/earphone/mode/pc/pc_key_msg_table.c \

#endif

#if TCFG_MIX_RECORD_ENABLE
c_SRC_FILES += \
	apps/earphone/audio/mix_record_api.c \

#endif

#if TCFG_LE_AUDIO_APP_CONFIG
c_SRC_FILES += \
	apps/earphone/mode/bt/le_audio/le_audio_common.c \

#endif

#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_UNICAST_SOURCE_EN | LE_AUDIO_JL_UNICAST_SOURCE_EN | LE_AUDIO_UNICAST_SINK_EN | LE_AUDIO_JL_UNICAST_SINK_EN))
c_SRC_FILES += \
	apps/earphone/mode/bt/le_audio/cig/app_le_connected.c \
	apps/earphone/mode/bt/le_audio/cig/le_connected.c \
	apps/earphone/mode/bt/le_audio/cig/le_connected_config.c \

#endif

#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_AURACAST_SOURCE_EN | LE_AUDIO_JL_AURACAST_SOURCE_EN | LE_AUDIO_AURACAST_SINK_EN | LE_AUDIO_JL_AURACAST_SINK_EN))
c_SRC_FILES += \
	apps/earphone/mode/bt/le_audio/big/app_le_auracast.c \
	apps/earphone/mode/bt/le_audio/big/le_broadcast.c \
	apps/earphone/mode/bt/le_audio/big/le_broadcast_config.c \

#endif

