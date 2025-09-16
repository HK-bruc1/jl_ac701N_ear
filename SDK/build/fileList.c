#include "app_config.h"




objs += \
      $(ROOT)/audio/framework/plugs/source/a2dp_file.o \
      $(ROOT)/audio/framework/plugs/source/a2dp_streamctrl.o \
      $(ROOT)/audio/framework/plugs/source/esco_file.o \
      $(ROOT)/audio/framework/plugs/source/adc_file.o \
	  $(ROOT)/audio/framework/plugs/source/multi_ch_adc_file.o \
      $(ROOT)/audio/framework/nodes/esco_tx_node.o \
      $(ROOT)/audio/framework/nodes/plc_node.o \
      $(ROOT)/audio/framework/nodes/volume_node.o \


objs += \
      $(ROOT)/audio/framework/nodes/ns_node.o

objs += \
      $(ROOT)/audio/framework/nodes/channle_swap_node.o

objs += \
      $(ROOT)/audio/framework/nodes/effect_dev0_node.o
objs += \
      $(ROOT)/audio/framework/nodes/effect_dev1_node.o
objs += \
      $(ROOT)/audio/framework/nodes/effect_dev2_node.o
objs += \
      $(ROOT)/audio/framework/nodes/effect_dev3_node.o
objs += \
      $(ROOT)/audio/framework/nodes/effect_dev4_node.o
objs += \
      $(ROOT)/audio/framework/nodes/sink_dev0_node.o
objs += \
      $(ROOT)/audio/framework/nodes/sink_dev1_node.o
objs += \
      $(ROOT)/audio/framework/nodes/sink_dev2_node.o
objs += \
      $(ROOT)/audio/framework/nodes/sink_dev3_node.o
objs += \
      $(ROOT)/audio/framework/nodes/sink_dev4_node.o

objs += \
      $(ROOT)/audio/framework/plugs/source/source_dev0_file.o
objs += \
      $(ROOT)/audio/framework/plugs/source/source_dev1_file.o
objs += \
      $(ROOT)/audio/framework/plugs/source/source_dev2_file.o
objs += \
      $(ROOT)/audio/framework/plugs/source/source_dev3_file.o
objs += \
      $(ROOT)/audio/framework/plugs/source/source_dev4_file.o

objs += \
      $(ROOT)/audio/framework/nodes/agc_node.o

objs += \
      $(ROOT)/audio/framework/nodes/surround_demo_node.o

objs += \
      $(ROOT)/audio/framework/nodes/lhdc_x_node.o

objs += \
      $(ROOT)/audio/framework/nodes/cvp_sms_node.o

objs += \
      $(ROOT)/audio/framework/nodes/cvp_dms_node.o

objs += \
      $(ROOT)/audio/framework/nodes/cvp_3mic_node.o

objs += \
      $(ROOT)/audio/framework/nodes/cvp_develop_node.o

objs += \
      $(ROOT)/audio/framework/nodes/uart_node.o

objs += \
      $(ROOT)/audio/framework/nodes/cvp_v3_node.o

objs += \
      $(ROOT)/audio/framework/nodes/dns_node.o

objs += \
      $(ROOT)/audio/framework/nodes/ai_tx_node.o

objs += \
      $(ROOT)/audio/framework/plugs/source/ai_rx_file.o

objs += \
      $(ROOT)/audio/framework/nodes/data_export_node.o

objs += \
      $(ROOT)/audio/framework/nodes/local_tws_source_node.o \
      $(ROOT)/audio/framework/plugs/source/local_tws_file.o \
	  $(ROOT)/audio/interface/player/local_tws_player.o

#if EXPORT_PLATFORM_AUDIO_PDM_ENABLE
objs += \
    $(ROOT)/audio/framework/plugs/source/pdm_mic_file.o
#endif

#if EXPORT_PLATFORM_AUDIO_SPDIF_ENABLE
objs += \
	  $(ROOT)/audio/framework/plugs/source/spdif_file.o
#endif


objs += \
	  $(ROOT)/audio/test_tools/mic_dut_process.o

objs += \
	  $(ROOT)/audio/test_tools/audio_enc_mpt_self.o \
	  $(ROOT)/audio/test_tools/audio_enc_mpt_cvp_ctr.o

objs += \
	  $(ROOT)/audio/test_tools/cvp_tool.o

objs += \
	  $(ROOT)/audio/test_tools/audio_dut_control.o \
	  $(ROOT)/audio/test_tools/audio_dut_control_old.o

objs += \
	  $(ROOT)/audio/common/audio_node_config.o \
	  $(ROOT)/audio/common/audio_dvol.o \
	  $(ROOT)/audio/common/audio_general.o \
	  $(ROOT)/audio/common/audio_build_needed.o \
	  $(ROOT)/audio/common/online_debug/aud_data_export.o \
	  $(ROOT)/audio/common/online_debug/audio_online_debug.o \
	  $(ROOT)/audio/common/online_debug/audio_capture.o \
	  $(ROOT)/audio/common/audio_plc.o \
	  $(ROOT)/audio/common/audio_noise_gate.o \
	  $(ROOT)/audio/common/audio_ns.o \
	  $(ROOT)/audio/common/audio_utils.o \
	  $(ROOT)/audio/common/audio_export_demo.o \
	  $(ROOT)/audio/common/amplitude_statistic.o \
	  $(ROOT)/audio/common/frame_length_adaptive.o \
	  $(ROOT)/audio/common/bt_audio_energy_detection.o \
	  $(ROOT)/audio/common/audio_event_handler.o \
	  $(ROOT)/audio/common/debug/audio_debug.o \
	  $(ROOT)/audio/common/power/mic_power_manager.o \
	  $(ROOT)/audio/common/audio_volume_mixer.o \
	  $(ROOT)/audio/common/audio_effect_verify.o \
	  $(ROOT)/audio/common/pcm_data/sine_pcm.o \

objs += \
	  $(ROOT)/audio/common/audio_iis_lrclk_capture.o

objs += \
	  $(ROOT)/audio/common/online_debug/aud_mic_dut.o

objs += \
	  $(ROOT)/audio/common/uartPcmSender.o

objs += \
	  $(ROOT)/audio/common/demo/codec_demo/audio_enc_file_demo.o \
	  $(ROOT)/audio/common/demo/codec_demo/audio_file_dec_demo.o \
	  $(ROOT)/audio/common/demo/codec_demo/audio_frame_codec_demo.o \
	  $(ROOT)/audio/common/demo/codec_demo/audio_msbc_hw_codec_demo.o \
	  $(ROOT)/audio/common/demo/codec_demo/audio_msbc_sw_codec_demo.o \
	  $(ROOT)/audio/common/demo/codec_demo/audio_sbc_codec_demo.o \
	  $(ROOT)/audio/common/demo/codec_demo/audio_speex_codec_demo.o \
	  $(ROOT)/audio/common/demo/codec_demo/audio_opus_codec_demo.o \
	  $(ROOT)/audio/common/demo/audio_alink_demo.o \
	  $(ROOT)/audio/common/demo/audio_eq_demo.o \


#if EXPORT_PLATFORM_AUDIO_HW_VAD_ENABLE
objs += \
	  $(ROOT)/audio/common/demo/audio_vad_demo.o

#endif

#if EXPORT_PLATFORM_HW_MATH_VERSION == 2
objs += \
	  $(ROOT)/audio/common/demo/hw_math_v2_demo.o
#endif


#if EXPORT_PLATFORM_AUDIO_WM8978_ENABLE
objs += \
	  $(ROOT)/audio/common/wm8978/wm8978.o \
	  $(ROOT)/audio/common/wm8978/iic.o
#endif


objs += \
	  $(ROOT)/audio/interface/player/tone_player.o \
	  $(ROOT)/audio/interface/player/ring_player.o \
	  $(ROOT)/audio/interface/player/a2dp_player.o \
	  $(ROOT)/audio/interface/player/esco_player.o \
	  $(ROOT)/audio/interface/player/key_tone_player.o \
	  $(ROOT)/audio/interface/player/dev_flow_player.o \
	  $(ROOT)/audio/interface/player/adda_loop_player.o \
	  $(ROOT)/audio/interface/player/linein_player.o \
	  $(ROOT)/audio/interface/player/ai_rx_player.o \

objs += \
	  $(ROOT)/audio/interface/recoder/esco_recoder.o \
	  $(ROOT)/audio/interface/recoder/ai_voice_recoder.o \
	  $(ROOT)/audio/interface/recoder/dev_flow_recoder.o \

#if EXPORT_LE_AUDIO_SUPPORT
objs += \
	$(ROOT)/audio/interface/player/le_audio_player.o \
	$(ROOT)/audio/framework/nodes/le_audio_source.o \
	$(ROOT)/audio/framework/plugs/source/le_audio_file.o \
	$(ROOT)/audio/le_audio/le_audio_stream.o
objs += \
	  $(ROOT)/audio/interface/recoder/le_audio_recorder.o
#endif

#if EXPORT_PLATFORM_AUDIO_MIDI_ENABLE
objs += \
	$(ROOT)/audio/midi/audio_dec_midi_file.o

objs += \
	$(ROOT)/audio/midi/audio_dec_midi_ctrl.o
#endif


objs += \
      $(ROOT)/audio/effect/eq_config.o \
	  $(ROOT)/audio/effect/spk_eq.o \
	  $(ROOT)/audio/effect/audio_voice_changer_api.o \
	  $(ROOT)/audio/effect/esco_ul_voice_changer.o \
	  $(ROOT)/audio/effect/bass_treble.o \
	  $(ROOT)/audio/effect/audio_dc_offset_remove.o \
	  $(ROOT)/audio/effect/effects_adj.o \
	  $(ROOT)/audio/effect/effects_dev.o \
	  $(ROOT)/audio/effect/effects_default_param.o \
	  $(ROOT)/audio/effect/node_param_update.o \
	  $(ROOT)/audio/effect/scene_update.o \

#if EXPORT_PLATFORM_ICSD_ENABLE
objs += \
	$(ROOT)/audio/common/icsd/adt/icsd_adt.o \
	$(ROOT)/audio/common/icsd/adt/icsd_adt_app.o \
	$(ROOT)/audio/common/icsd/adt/icsd_adt_config.o \
	$(ROOT)/audio/common/icsd/adt/icsd_adt_alg.o \
	$(ROOT)/audio/common/icsd/adt/icsd_adt_demo.o \
	$(ROOT)/audio/common/icsd/anc/icsd_anc_app.o \
	$(ROOT)/audio/common/icsd/anc/icsd_anc_board.o \
	$(ROOT)/audio/common/icsd/anc/icsd_anc_data.o \
	$(ROOT)/audio/common/icsd/anc/icsd_anc_interactive.o \
	$(ROOT)/audio/common/icsd/common/icsd_common.o \
	$(ROOT)/audio/common/icsd/dot/icsd_dot_app.o \
	$(ROOT)/audio/common/icsd/dot/icsd_dot.o \
	$(ROOT)/audio/common/icsd/common_v2/icsd_common_v2.o \
	$(ROOT)/audio/common/icsd/common_v2/icsd_common_v2_app.o \
	$(ROOT)/audio/common/icsd/anc_v2/icsd_anc_v2.o \
	$(ROOT)/audio/common/icsd/anc_v2/icsd_anc_v2_app.o \
	$(ROOT)/audio/common/icsd/anc_v2/icsd_anc_v2_interactive.o \
	$(ROOT)/audio/common/icsd/config/icsd_anc_v2_config.o \
	$(ROOT)/audio/common/icsd/rt_anc/rt_anc.o \
	$(ROOT)/audio/common/icsd/rt_anc/rt_anc_app.o \
	$(ROOT)/audio/common/icsd/rt_anc/rt_anc_config.o \
	$(ROOT)/audio/common/icsd/tool/anc_ext_tool.o \
	$(ROOT)/audio/common/icsd/tool/anc_ext_tool_file.o \
	$(ROOT)/audio/common/icsd/aeq/icsd_aeq_app.o \
	$(ROOT)/audio/common/icsd/aeq/icsd_aeq.o \
	$(ROOT)/audio/common/icsd/aeq/icsd_aeq_config.o \
	$(ROOT)/audio/common/icsd/afq/icsd_afq_app.o \
	$(ROOT)/audio/common/icsd/afq/icsd_afq.o \
	$(ROOT)/audio/common/icsd/afq/icsd_afq_config.o \
	$(ROOT)/audio/common/icsd/cmp/icsd_cmp_app.o \
	$(ROOT)/audio/common/icsd/config/icsd_cmp_config.o \
	$(ROOT)/audio/common/icsd/demo/icsd_demo.o \
	$(ROOT)/audio/common/icsd/ein/icsd_ein_config.o \
	$(ROOT)/audio/common/icsd/vdt/icsd_vdt_config.o \
	$(ROOT)/audio/common/icsd/vdt/icsd_vdt_app.o \
	$(ROOT)/audio/common/icsd/wat/icsd_wat_config.o \
	$(ROOT)/audio/common/icsd/wat/icsd_wat_app.o \
	$(ROOT)/audio/common/icsd/wind/icsd_wind_config.o \
	$(ROOT)/audio/common/icsd/wind/icsd_wind_app.o \
	$(ROOT)/audio/common/icsd/wind/cvp_wind_app.o \
	$(ROOT)/audio/common/icsd/avc/icsd_avc_config.o \
	$(ROOT)/audio/common/icsd/avc/icsd_avc_app.o \
	$(ROOT)/audio/common/icsd/env_noise_det/icsd_env_noise_det_app.o \
	$(ROOT)/audio/common/icsd/adjdcc/icsd_adjdcc_config.o \
	$(ROOT)/audio/common/icsd/howl/icsd_howl_config.o \
	$(ROOT)/audio/common/icsd/howl/icsd_howl_app.o \

#endif

#if EXPORT_PLATFORM_ANC_ENABLE
objs += \
	  $(ROOT)/audio/anc/audio_anc_fade_ctr.o \
	  $(ROOT)/audio/anc/audio_anc_common_plug.o \
	  $(ROOT)/audio/anc/audio_anc_debug_tool.o \
	  $(ROOT)/audio/anc/audio_anc_mult_scene.o \
	  $(ROOT)/audio/anc/audio_anc_common.o \

#endif

#if EXPORT_PLATFORM_AUDIO_SMART_VOICE_ENABLE
objs += \
	$(ROOT)/audio/smart_voice/smart_voice_core.o \
	$(ROOT)/audio/smart_voice/smart_voice_config.o \
	$(ROOT)/audio/smart_voice/jl_kws_platform.o \
	$(ROOT)/audio/smart_voice/user_asr.o \
	$(ROOT)/audio/smart_voice/voice_mic_data.o \
	$(ROOT)/audio/smart_voice/kws_event.o \
	$(ROOT)/audio/smart_voice/nn_vad.o \
	$(ROOT)/audio/smart_voice/nn_vad_bin.o
#endif

objs += \
	$(ROOT)/audio/jl_kws/jl_kws_main.o \
	$(ROOT)/audio/jl_kws/jl_kws_audio.o \
	$(ROOT)/audio/jl_kws/jl_kws_algo.o \
	$(ROOT)/audio/jl_kws/jl_kws_event.o \



#if EXPORT_PLATFORM_AUDIO_SPATIAL_EFFECT_ENABLE
objs += \
	$(ROOT)/audio/framework/nodes/spatial_effects_node.o \
	$(ROOT)/audio/common/online_debug/aud_spatial_effect_dut.o \

objs += \
	$(ROOT)/audio/effect/spatial_effect/spatial_effect.o \
	$(ROOT)/audio/effect/spatial_effect/spatial_effect_imu.o \
	$(ROOT)/audio/effect/spatial_effect/spatial_effect_tws.o \
	$(ROOT)/audio/effect/spatial_effect/spatial_imu_trim.o \
	$(ROOT)/audio/effect/spatial_effect/spatial_effects_process.o \

#endif

#if EXPORT_SOMATOSENSORY_ENABLE
objs += \
	$(ROOT)/audio/effect/somatosensory/audio_somatosensory.o \

#endif

objs += \
	  $(ROOT)/audio/CVP/audio_aec.o \
	  $(ROOT)/audio/CVP/audio_cvp.o \
	  $(ROOT)/audio/CVP/audio_cvp_dms.o \
	  $(ROOT)/audio/CVP/audio_cvp_3mic.o \
	  $(ROOT)/audio/CVP/audio_cvp_v3.o \
	  $(ROOT)/audio/CVP/audio_cvp_online.o \
	  $(ROOT)/audio/CVP/audio_cvp_demo.o \
	  $(ROOT)/audio/CVP/audio_cvp_develop.o \
	  $(ROOT)/audio/CVP/audio_cvp_sync.o \
	  $(ROOT)/audio/CVP/audio_cvp_ais_3mic.o \
	  $(ROOT)/audio/CVP/audio_cvp_ref_task.o \
	  $(ROOT)/audio/CVP/audio_cvp_config.o \

objs += \
	  $(ROOT)/audio/interface/player/tws_tone_player.o

#if EXPORT_PLATFORM_AUDIO_ALINK_ENABLE
objs += \
	  $(ROOT)/audio/framework/nodes/iis_node.o \
	  $(ROOT)/audio/framework/plugs/source/iis_file.o

objs += \
	  $(ROOT)/audio/framework/nodes/multi_ch_iis_node.o \
	  $(ROOT)/audio/framework/plugs/source/multi_ch_iis_file.o

#endif

#if EXPORT_PLATFORM_AUDIO_SPDIF_ENABLE
objs += \
	  $(ROOT)/audio/framework/nodes/spdif_node.o
#endif

#if EXPORT_PLATFORM_AUDIO_FM_ENABLE
objs += \
	  $(ROOT)/audio/interface/player/fm_player.o \
	  $(ROOT)/audio/framework/plugs/source/fm_file.o \

#endif

#if EXPORT_PLATFORM_AUDIO_TDM_ENABLE
objs += \
	  $(ROOT)/audio/interface/player/tdm_player.o \
	  $(ROOT)/audio/framework/plugs/source/tdm_file.o \
	  $(ROOT)/audio/framework/nodes/tdm_node.o \

#endif

objs += \
      $(ROOT)/audio/interface/player/file_player.o \


objs += \
      $(ROOT)/audio/interface/recoder/file_recorder.o \


objs += \
	  $(ROOT)/audio/framework/plugs/source/linein_file.o \

objs += \
	  $(ROOT)/audio/framework/plugs/source/pc_spk_file.o \
	  $(ROOT)/audio/interface/player/pc_spk_player.o \


objs += \
	  $(ROOT)/audio/framework/nodes/pc_mic_node.o \
	  $(ROOT)/audio/interface/recoder/pc_mic_recoder.o \


objs += \
	  $(ROOT)/audio/effect/audio_pitch_speed_api.o \




#if EXPORT_PLATFORM_AUDIO_EFFECT_DEMO_ENABLE
objs += \
      $(ROOT)/audio/effect/demo/autotune_demo.o \
      $(ROOT)/audio/effect/demo/bass_treble_demo.o \
      $(ROOT)/audio/effect/demo/chorus_demo.o \
      $(ROOT)/audio/effect/demo/crossover_demo.o \
      $(ROOT)/audio/effect/demo/drc_advance_demo.o \
      $(ROOT)/audio/effect/demo/drc_demo.o \
      $(ROOT)/audio/effect/demo/dynamic_eq_demo.o \
      $(ROOT)/audio/effect/demo/echo_demo.o \
      $(ROOT)/audio/effect/demo/energy_detect_demo.o \
      $(ROOT)/audio/effect/demo/eq_demo.o \
      $(ROOT)/audio/effect/demo/frequency_shift_howling_demo.o \
      $(ROOT)/audio/effect/demo/gain_demo.o \
      $(ROOT)/audio/effect/demo/harmonic_exciter_demo.o \
      $(ROOT)/audio/effect/demo/multiband_drc_demo.o \
      $(ROOT)/audio/effect/demo/noisegate_demo.o \
      $(ROOT)/audio/effect/demo/notch_howling_demo.o \
      $(ROOT)/audio/effect/demo/pitch_speed_demo.o \
      $(ROOT)/audio/effect/demo/reverb_advance_demo.o \
      $(ROOT)/audio/effect/demo/reverb_demo.o \
      $(ROOT)/audio/effect/demo/spectrum_demo.o \
      $(ROOT)/audio/effect/demo/stereo_widener_demo.o \
      $(ROOT)/audio/effect/demo/surround_demo.o \
      $(ROOT)/audio/effect/demo/virtual_bass_demo.o \
      $(ROOT)/audio/effect/demo/voice_changer_demo.o \
      $(ROOT)/audio/effect/demo/lhdc_x_demo.o \

#endif

objs += \
	$(ROOT)/audio/cpu/common.o \





#if EXPORT_LE_AUDIO_SUPPORT

objs += \
	$(ROOT)/apps/common/le_audio/big.o

objs += \
	$(ROOT)/apps/common/le_audio/cig.o

objs += \
	$(ROOT)/apps/common/le_audio/wireless_trans_manager.o

#endif



objs += \
	$(ROOT)/apps/common/config/bt_profile_config.o \


objs += \
	$(ROOT)/apps/common/config/new_cfg_tool.o \
	$(ROOT)/apps/common/config/cfg_tool_statistics_functions/cfg_tool_statistics.o
objs += \
	$(ROOT)/apps/common/config/cfg_tool_cdc.o



objs += \
	$(ROOT)/apps/common/config/app_config.o

objs += \
	$(ROOT)/apps/common/config/ci_transport_uart.o



objs += \
	$(ROOT)/apps/common/config/bt_name_parse.o


objs += \
	$(ROOT)/apps/common/fat_nor/cfg_private.o



objs += \
	$(ROOT)/apps/common/debug/dlog_config.o \
    $(ROOT)/apps/common/debug/dlog_output_config.o



objs += \
	$(ROOT)/apps/common/update/update.o \
	$(ROOT)/apps/common/update/testbox_update.o \
	$(ROOT)/apps/common/update/testbox_uart_update.o

objs += \
	$(ROOT)/apps/common/update/update_tws.o

objs += \
	$(ROOT)/apps/common/update/update_tws_new.o

objs += \
	$(ROOT)/apps/common/update/update_tws_new_less.o

#ifdef CONFIG_UPDATE_MUTIL_CPU_UART
objs += \
	$(ROOT)/apps/common/update/uart_update_driver.o \
	$(ROOT)/apps/common/update/update_interactive_uart.o

objs += \
	$(ROOT)/apps/common/update/uart_update.o

objs += \
	$(ROOT)/apps/common/update/uart_update_master.o
#endif

objs += \
    $(ROOT)/apps/common/device/storage_device/norflash/norflash.o






objs += \
	$(ROOT)/apps/common/ui/pwm_led/led_ui_api.o \
	$(ROOT)/apps/common/ui/pwm_led/led_ui_tws_sync.o




#if defined CONFIG_SOUNDBOX_CASE || defined CONFIG_DONGLE_CASE
objs += \
	$(ROOT)/apps/common/ui/led7/led7_ui_api.o


objs += \
    $(ROOT)/apps/common/ui/lcd/lcd_ui_api.o

#if EXPORT_DOT_UI_ENABLE

objs += \
	$(ROOT)/apps/common/ui/lcd_drive/lcd_spi/lcd_spi_sh8601a_454x454.o

objs += \
	$(ROOT)/apps/common/ui/lcd_drive/lcd_spi/lcd_spi_st7789v_240x240.o

objs += \
	$(ROOT)/apps/common/ui/lcd_drive/lcd_spi/oled_spi_ssd1306_128x64.o


objs += \
    $(ROOT)/apps/common/ui/interface/ui_platform.o \
    $(ROOT)/apps/common/ui/interface/ui_pushScreen_manager.o \
    $(ROOT)/apps/common/ui/interface/ui_resources_manager.o \
    $(ROOT)/apps/common/ui/interface/ui_synthesis_manager.o \
    $(ROOT)/apps/common/ui/interface/ui_synthesis_oled.o \
    $(ROOT)/apps/common/ui/interface/watch_bgp.o

#endif // EXPORT_DOT_UI_ENABLE
#endif // CONFIG_SOUNDBOX_CASE




objs += \
	$(ROOT)/apps/common/third_party_profile/multi_protocol_common.o \
	$(ROOT)/apps/common/third_party_profile/multi_protocol_event.o \



objs += \
    $(ROOT)/apps/common/third_party_profile/interface/app_protocol_api.o \
    $(ROOT)/apps/common/third_party_profile/interface/app_protocol_common.o

objs += \
	$(ROOT)/apps/common/third_party_profile/interface/app_protocol_gma.o \


objs += \
	$(ROOT)/apps/common/third_party_profile/interface/app_protocol_mma.o

objs += \
    $(ROOT)/apps/common/third_party_profile/interface/app_protocol_ota.o

objs += \
	$(ROOT)/apps/common/third_party_profile/interface/app_protocol_tme.o

objs += \
    $(ROOT)/apps/common/third_party_profile/custom_protocol_demo/custom_protocol.o

objs += \
    $(ROOT)/apps/common/third_party_profile/app_ble_ancs_ams/app_ble_ancs_ams.o

objs += \
    $(ROOT)/apps/common/third_party_profile/multi_ble_client/ble_multi_client.o

objs += \
    $(ROOT)/apps/common/third_party_profile/swift_pair/swift_pair_protocol.o


objs += \
    $(ROOT)/apps/common/third_party_profile/gfps_protocol/gfps_protocol.o


objs += \
    $(ROOT)/apps/common/third_party_profile/mma_protocol/mma_config.o \
    $(ROOT)/apps/common/third_party_profile/mma_protocol/mma_platform_api.o \
    $(ROOT)/apps/common/third_party_profile/mma_protocol/mma_protocol.o \


#if EXPORT_TENCENT_LL_ENABLE
objs += \
    $(ROOT)/apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_import.o \
    $(ROOT)/apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_llsync_data.o \
    $(ROOT)/apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_llsync_device.o \
    $(ROOT)/apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_llsync_event.o \
    $(ROOT)/apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_llsync_ota.o \
    $(ROOT)/apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_service.o \
    $(ROOT)/apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_utils_base64.o \
    $(ROOT)/apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_utils_crc.o \
    $(ROOT)/apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_utils_hmac.o \
    $(ROOT)/apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_utils_log.o \
    $(ROOT)/apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_utils_md5.o \
    $(ROOT)/apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_utils_sha1.o \
    $(ROOT)/apps/common/third_party_profile/Tencent_LL/tencent_protocol/ble_qiot_template.o \
    $(ROOT)/apps/common/third_party_profile/Tencent_LL/tencent_ll_demo/ll_demo.o \
    $(ROOT)/apps/common/third_party_profile/Tencent_LL/tencent_ll_demo/ll_task.o \
    $(ROOT)/apps/common/third_party_profile/Tencent_LL/tencent_ll_demo/ll_sync_event_handler.o \

objs += \
	$(ROOT)/apps/common/third_party_profile/Tencent_LL/tencent_ll_demo/jieli_iot_functions/ble_iot_utils.o \
	$(ROOT)/apps/common/third_party_profile/Tencent_LL/tencent_ll_demo/jieli_iot_functions/ble_iot_msg_manager.o \
	$(ROOT)/apps/common/third_party_profile/Tencent_LL/tencent_ll_demo/jieli_iot_functions/ble_iot_anc_manager.o \
	$(ROOT)/apps/common/third_party_profile/Tencent_LL/tencent_ll_demo/jieli_iot_functions/ble_iot_power_manager.o \
	$(ROOT)/apps/common/third_party_profile/Tencent_LL/tencent_ll_demo/jieli_iot_functions/ble_iot_tws_manager.o \



#endif


objs += \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/app/demo/tuya_ble_app_demo.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/app/demo/tuya_ota.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_api.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_bulk_data.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_data_handler.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_event.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_event_handler.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_event_handler_weak.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_feature_weather.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_gatt_send_queue.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_heap.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_main.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_mem.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_mutli_tsf_protocol.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_queue.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_storage.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_unix_time.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/sdk/src/tuya_ble_utils.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/port/tuya_ble_port_JL.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/port/tuya_ble_port.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/port/tuya_ble_port_peripheral.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/extern_components/mbedtls/aes.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/extern_components/mbedtls/ccm.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/extern_components/mbedtls/hmac.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/extern_components/mbedtls/md5.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/extern_components/mbedtls/sha1.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/extern_components/mbedtls/sha256.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/app/product_test/tuya_ble_app_production_test.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/app/uart_common/tuya_ble_app_uart_common_handler.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/app/demo/tuya_app_func.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/tuya_protocol.o \
	$(ROOT)/apps/common/third_party_profile/tuya_protocol/tuya_event.o \



objs += \
    $(ROOT)/apps/common/third_party_profile/ancs_client_demo/le_ancs_client.o


objs += \
	$(ROOT)/apps/common/third_party_profile/common/mic_rec.o


objs += \
	$(ROOT)/apps/common/third_party_profile/common/custom_cfg.o

objs += \
    $(ROOT)/apps/common/third_party_profile/realme_protocol/realme_config.o \
    $(ROOT)/apps/common/third_party_profile/realme_protocol/realme_platform_api.o \
    $(ROOT)/apps/common/third_party_profile/realme_protocol/realme_protocol.o \


objs += \
    $(ROOT)/apps/common/third_party_profile/jieli/rcsp/adv/ble_rcsp_adv.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/ble_rcsp_client.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/ble_rcsp_multi_common.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/ble_rcsp_server.o \

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/client/rcsp_c_cmd_recieve.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/client/rcsp_c_cmd_recieve_no_response.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/client/rcsp_c_cmd_response.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/client/rcsp_m_update/rcsp_update_master.o


objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp_bt_manage.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp_config.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp_event.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp_manage.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp_task.o \

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/browser/rcsp_browser.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_bt_func.o \

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_fm_func.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_linein_func.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_music_func.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_pc_func.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_rtc_func.o


objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_spdif_func.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/rcsp_device_feature.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/rcsp_device_status.o \

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/external_flash/rcsp_extra_flash_cmd.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/external_flash/rcsp_extra_flash_opt.o


objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/dev_format.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/file_bluk_trans_prepare.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/file_simple_transfer.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/file_delete.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/file_trans_back.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/file_transfer.o \
    $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/file_transfer_sync.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/rcsp_setting_opt.o \



objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/rcsp_setting_sync.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_1t2_setting.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_adaptive_noise_reduction.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_ai_no_pick.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_anc_voice.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/rcsp_adv_bluetooth.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_time_stamp_setting.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_key_setting.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_anc_voice_key.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_bt_name_setting.o
objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_hearing_aid_setting.o


objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_led_setting.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_mic_setting.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_scene_noise_reduction.o


objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_voice_enhancement_mode.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_wind_noise_detection.o


objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_work_setting.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_color_led_setting.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_eq_setting.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_high_low_vol_setting.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_karaoke_eq_setting.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_karaoke_setting.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_misc_drc_setting.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_misc_reverbration_setting.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_misc_setting.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_music_info_setting.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_vol_setting.o



objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_switch_device.o \

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_update/rcsp_ch_loader_download.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_update/rcsp_update.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_update/rcsp_update_tws.o


objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/nfc_data_opt.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sensor_log_notify.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sensors_data_opt.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_func.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_info_opt/sport_info_bt_disconn.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_info_opt/sport_info_continuous_heart_rate.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_info_opt/sport_info_exercise_heart_rate.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_info_opt/sport_info_fall_detection.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_info_opt/sport_info_personal_info.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_info_opt/sport_info_pressure_detection.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_info_opt/sport_info_raise_wrist.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_info_opt/sport_info_sedentary.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_info_opt/sport_info_sensor_opt.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_info_opt/sport_info_sleep_detection.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_opt/sport_data_air_pressure.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_opt/sport_data_altitude.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_opt/sport_data_blood_oxygen.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_opt/sport_data_exercise_recovery_time.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_opt/sport_data_exercise_steps.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_opt/sport_data_heart_rate.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_opt/sport_data_max_oxygen_uptake.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_opt/sport_data_pressure_detection.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_opt/sport_data_sports_information.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_opt/sport_data_training_load.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_info_opt.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_info_sync.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_info_vm.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/translator/rcsp_translator.o

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/rcsp_cmd_recieve.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/rcsp_cmd_recieve_no_respone.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/rcsp_cmd_respone.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/rcsp_cmd_user.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/rcsp_command.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/rcsp_data_recieve.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/rcsp_data_recieve_no_respone.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/rcsp_data_respone.o \

objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/rcsp_interface.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/rcsp_simplified/rcsp_interface_simplified.o \
    $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/mass_data/mass_data.o \
	$(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/pub_mutual_set_cmd/pub_mutual_set_cmd_opt.o \



objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/trans_data_demo/le_trans_data.o \
	$(ROOT)/apps/common/third_party_profile/jieli/trans_data_demo/spp_trans_data.o


objs += \
	$(ROOT)/apps/common/third_party_profile/jieli/online_db/spp_online_db.o \
	$(ROOT)/apps/common/third_party_profile/jieli/online_db/online_db_deal.o

objs += \
    $(ROOT)/apps/common/third_party_profile/auracast_app/auracast_app_protocol.o \
    $(ROOT)/apps/common/third_party_profile/auracast_app/auracast_app_ble.o \
    $(ROOT)/apps/common/third_party_profile/auracast_app/auracast_app_gatt_over_edr.o \
    $(ROOT)/apps/common/third_party_profile/auracast_app/auracast_app_source_api.o \
    $(ROOT)/apps/common/third_party_profile/auracast_app/auracast_app_sink_api.o \


#if EXPORT_FNMA_ENABLE
objs += \
    $(ROOT)/apps/common/third_party_profile/bt_fmy/ble_fmy.o \
    $(ROOT)/apps/common/third_party_profile/bt_fmy/ble_fmy_fmna.o \
    $(ROOT)/apps/common/third_party_profile/bt_fmy/ble_fmy_ota.o \
    $(ROOT)/apps/common/third_party_profile/bt_fmy/ble_fmy_modet.o \

#endif

objs += \
    $(ROOT)/apps/common/third_party_profile/dma_protocol/dma_use_lib/dma_le_port.o \
    $(ROOT)/apps/common/third_party_profile/dma_protocol/dma_use_lib/dma_setting.o \
    $(ROOT)/apps/common/third_party_profile/dma_protocol/dma_use_lib/dma_spp_port.o \
    $(ROOT)/apps/common/third_party_profile/dma_protocol/dma_use_lib/platform_template.o \
    $(ROOT)/apps/common/third_party_profile/dma_protocol/dma_protocol.o \


objs += \
    $(ROOT)/apps/common/third_party_profile/ximalaya_protocol/xmly_protocol.o \
    $(ROOT)/apps/common/third_party_profile/ximalaya_protocol/xmly_platform_api.o \
    $(ROOT)/apps/common/third_party_profile/ximalaya_protocol/xmly_config.o \
    $(ROOT)/apps/common/third_party_profile/ximalaya_protocol/xmly_ble.o \





#ifdef CONFIG_SOUNDBOX_CASE
objs += \
    $(ROOT)/apps/common/iap/iAP_chip.o \
    $(ROOT)/apps/common/iap/iAP_des.o \
    $(ROOT)/apps/common/iap/iAP_device.o \
    $(ROOT)/apps/common/iap/iAP_iic.o \

#endif





objs += \
	$(ROOT)/apps/common/device/in_ear_detect/in_ear_detect.o \
	$(ROOT)/apps/common/device/in_ear_detect/in_ear_manage.o



#ifdef CONFIG_EARPHONE_CASE
objs += \
	$(ROOT)/apps/common/device/eartouch/eartouch_manage.o \
	$(ROOT)/apps/common/device/eartouch/eartouch_iic_interface.o
objs += \
	$(ROOT)/apps/common/device/eartouch/hx300x/hx300x_driver.o

objs += \
	$(ROOT)/apps/common/device/eartouch/outside_tch_driver.o
#endif


#ifdef CONFIG_SOUNDBOX_CASE
objs += \
	$(ROOT)/apps/common/device/storage_device/nandflash/nandflash.o \
	$(ROOT)/apps/common/device/storage_device/nandflash/ftl_device.o
#endif


#ifdef CONFIG_SOUNDBOX_CASE
objs += \
	$(ROOT)/apps/common/device/fm/fm_manage.o

objs += \
	$(ROOT)/apps/common/device/fm/fm_inside/fm_inside.o


objs += \
	$(ROOT)/apps/common/device/fm/bk1080/Bk1080.o

objs += \
	$(ROOT)/apps/common/device/fm/qn8035/QN8035.o

objs += \
	$(ROOT)/apps/common/device/fm/rda5807/RDA5807.o

#endif



objs += \
	$(ROOT)/apps/common/device/sensor/ntc/ntc_det.o

objs += \
	$(ROOT)/apps/common/device/sensor/ir_sensor/ir_manage.o

objs += \
	$(ROOT)/apps/common/device/sensor/ir_sensor/jsa1221.o

#ifdef CONFIG_EARPHONE_CASE
objs += \
	$(ROOT)/apps/common/device/sensor/imu_sensor/imuSensor_manage.o \

objs += \
	$(ROOT)/apps/common/device/sensor/imu_sensor/sh3001/sh3001.o

objs += \
	$(ROOT)/apps/common/device/sensor/imu_sensor/mpu6887/mpu6887p.o

objs += \
	$(ROOT)/apps/common/device/sensor/imu_sensor/mpu9250/mpu9250.o

objs += \
	$(ROOT)/apps/common/device/sensor/imu_sensor/qmi8658/qmi8658c.o

objs += \
	$(ROOT)/apps/common/device/sensor/imu_sensor/lsm6dsl/lsm6dsl.o

objs += \
	$(ROOT)/apps/common/device/sensor/imu_sensor/icm_42670p/icm_42670p.o \
	$(ROOT)/apps/common/device/sensor/imu_sensor/icm_42670p/inv_imu_transport.o \
	$(ROOT)/apps/common/device/sensor/imu_sensor/icm_42670p/inv_imu_apex.o \
	$(ROOT)/apps/common/device/sensor/imu_sensor/icm_42670p/inv_imu_driver.o

#endif



objs += \
	$(ROOT)/apps/common/device/key/key_driver.o

objs += \
	$(ROOT)/apps/common/device/key/iokey.o

objs += \
	$(ROOT)/apps/common/device/key/irkey.o

objs += \
	$(ROOT)/apps/common/device/key/adkey.o


objs += \
	$(ROOT)/apps/common/device/key/adkey_rtcvdd.o

objs += \
	$(ROOT)/apps/common/device/key/touch_key.o

objs += \
	$(ROOT)/apps/common/device/key/ctmu_touch_key.o

objs += \
	$(ROOT)/apps/common/device/key/uart_key.o


#if EXPORT_CONFIG_USB_ENABLE
objs += \
	$(ROOT)/apps/common/device/usb/usb_config.o \
	$(ROOT)/apps/common/device/usb/device/descriptor.o \
	$(ROOT)/apps/common/device/usb/device/usb_device.o \
	$(ROOT)/apps/common/device/usb/device/user_setup.o \
	$(ROOT)/apps/common/device/usb/device/task_pc.o \


objs += \
	$(ROOT)/apps/common/device/usb/device/usb_pll_trim.o \

objs += \
	$(ROOT)/apps/common/device/usb/device/msd.o \


objs += \
	$(ROOT)/apps/common/device/usb/device/msd_upgrade.o \

objs += \
	$(ROOT)/apps/common/device/usb/device/mtp.o \


objs += \
	$(ROOT)/apps/common/device/usb/device/hid.o \


objs += \
	$(ROOT)/apps/common/device/usb/device/custom_hid.o \
	$(ROOT)/apps/common/device/usb/device/rcsp_hid_inter.o \



objs += \
	$(ROOT)/apps/common/device/usb/device/uac_stream.o

objs += \
	$(ROOT)/apps/common/device/usb/device/uac1.o

objs += \
	$(ROOT)/apps/common/device/usb/device/uac2.o




objs += \
    $(ROOT)/apps/common/device/usb/device/cdc.o \


objs += \
    $(ROOT)/apps/common/device/usb/device/midi.o \



objs += \
    $(ROOT)/apps/common/device/usb/device/printer.o \


objs += \
	$(ROOT)/apps/common/device/usb/usb_host_config.o \
	$(ROOT)/apps/common/device/usb/host/usb_bulk_transfer.o \
	$(ROOT)/apps/common/device/usb/host/usb_ctrl_transfer.o \
	$(ROOT)/apps/common/device/usb/host/usb_host.o \



objs += \
	$(ROOT)/apps/common/device/usb/host/usb_storage.o


objs += \
	$(ROOT)/apps/common/device/usb/host/adb.o \



objs += \
    $(ROOT)/apps/common/device/usb/host/aoa.o \



objs += \
    $(ROOT)/apps/common/device/usb/host/hid.o \


objs += \
    $(ROOT)/apps/common/device/usb/host/uac_host.o \




objs += \
    $(ROOT)/apps/common/device/usb/usb_epbuf_manager.o \
    $(ROOT)/apps/common/device/usb/usb_task.o \



objs += \
    $(ROOT)/apps/common/device/usb/demo/host/udisk_block_test.o


#endif




objs += \
	  $(ROOT)/cpu/components/iic_soft.o \
	  $(ROOT)/cpu/components/iic_api.o \
	  $(ROOT)/cpu/components/ir_encoder.o \
	  $(ROOT)/cpu/components/ir_decoder.o \
	  $(ROOT)/cpu/components/rdec_soft.o \

objs += \
	  $(ROOT)/cpu/components/led_api.o \
	  $(ROOT)/cpu/components/two_io_led.o \


objs += \
	  $(ROOT)/cpu/components/touch/lp_touch_key_tool.o \



objs += \
    $(ROOT)/cpu/config/gpio_file_parse.o \
    $(ROOT)/cpu/config/lib_power_config.o \


objs += \
	  $(ROOT)/cpu/periph_demo/iic_master_demo.o \
	  $(ROOT)/cpu/periph_demo/iic_slave_demo.o \
	  $(ROOT)/cpu/periph_demo/ledc_test.o \
	  $(ROOT)/cpu/periph_demo/sd_test.o \
	  $(ROOT)/cpu/periph_demo/led_api_test.o \
	  $(ROOT)/cpu/periph_demo/pwm_led_test.o \
	  $(ROOT)/cpu/periph_demo/two_io_led_test.o \
	  $(ROOT)/cpu/periph_demo/spi_test.o \
	  $(ROOT)/cpu/periph_demo/uart_test.o \
	  $(ROOT)/cpu/periph_demo/gptimer_demo.o \
	  $(ROOT)/cpu/periph_demo/rdec_soft_demo.o \
	  $(ROOT)/cpu/periph_demo/ir_encoder_decoder_demo.o \




objs += \
	  $(ROOT)/audio/cpu/br28/audio_setup.o \
	  $(ROOT)/audio/cpu/br28/audio_mic_capless.o \
	  $(ROOT)/audio/cpu/br28/audio_dai/audio_pdm.o \
	  $(ROOT)/audio/cpu/br28/audio_config.o \
	  $(ROOT)/audio/cpu/br28/audio_configs_dump.o \

objs += \
	  $(ROOT)/audio/cpu/br28/audio_anc.o \
	  $(ROOT)/audio/cpu/br28/icsd_anc_user.o


objs += \
	  $(ROOT)/audio/cpu/br28/audio_accelerator/hw_fft.o \

objs += \
	$(ROOT)/audio/cpu/br28/audio_vad/vad_mic.o \
	$(ROOT)/audio/cpu/br28/audio_vad/vad_clock_trim.o

objs += \
	  $(ROOT)/audio/cpu/br28/audio_demo/audio_adc_demo.o \

objs += \
	  $(ROOT)/audio/cpu/br28/audio_demo/audio_dac_demo.o \
	  $(ROOT)/audio/cpu/br28/audio_demo/audio_fft_demo.o \
	  $(ROOT)/audio/cpu/br28/audio_demo/audio_pdm_demo.o



objs += \
	  $(ROOT)/cpu/br28/setup.o \
	  $(ROOT)/cpu/br28/overlay_code.o \
	  $(ROOT)/cpu/br28/private_iis.o \

objs += \
	  $(ROOT)/cpu/br28/lua_port_api.o

objs += \
	  $(ROOT)/cpu/br28/charge/charge.o \
      $(ROOT)/cpu/br28/charge/charge_config.o

objs += \
	  $(ROOT)/cpu/br28/charge/chargestore.o \
	  $(ROOT)/cpu/br28/charge/chargestore_config.o

#ifdef CONFIG_SOUNDBOX_CASE
objs += \
      $(ROOT)/cpu/br28/ui_driver/led7/led7_driver.o \
      $(ROOT)/cpu/br28/ui_driver/ui_common.o
#endif

objs += \
	  $(ROOT)/cpu/br28/periph/touch/plcnt.o
objs += \
	  $(ROOT)/cpu/br28/periph/touch/lp_touch_key.o \
	  $(ROOT)/cpu/br28/periph/touch/lpctmu_hw.o

objs += \
	  $(ROOT)/cpu/br28/periph/rdec.o \
	  $(ROOT)/cpu/br28/periph/mcpwm.o \

objs += \
	  $(ROOT)/cpu/components/pwm_led_v1.o

objs += \
		$(ROOT)/cpu/br28/power/key_wakeup.o \

objs += \
	  $(ROOT)/cpu/br28/power/power_app.o \
	  $(ROOT)/cpu/br28/power/power_config.o \
	  $(ROOT)/cpu/br28/power/power_port.o \

#if EXPORT_LCD_DRIVER_ENABLE

objs += \
    $(ROOT)/cpu/$(CPU_DIR)/ui_driver/interface/ui_platform.o
objs += \
    $(ROOT)/cpu/$(CPU_DIR)/ui_driver/lcd_drive/lcd_drive.o
objs += \
    $(ROOT)/cpu/$(CPU_DIR)/ui_driver/lcd_drive/lcd_spi/lcd_spi_st7789v_240x240.o
objs += \
    $(ROOT)/cpu/$(CPU_DIR)/ui_driver/lcd_drive/lcd_spi/lcd_spi_sh8601a_454x454.o
objs += \
    $(ROOT)/cpu/$(CPU_DIR)/ui_driver/lcd_drive/lcd_spi/lcd_spi_gc9b71_320x386.o

#endif


objs += \
	$(ROOT)/apps/common/dev_manager/dev_reg.o \
	$(ROOT)/apps/common/dev_manager/dev_update.o \
	$(ROOT)/apps/earphone/mode/common/dev_status.o \


objs += \
	$(ROOT)/apps/common/music/breakpoint.o \
	$(ROOT)/apps/common/music/music_decrypt.o \
	$(ROOT)/apps/common/music/music_id3.o \
	$(ROOT)/apps/common/music/music_player.o \
	$(ROOT)/apps/common/file_operate/file_manager.o \
	$(ROOT)/apps/earphone/mode/music/music.o \
	$(ROOT)/apps/earphone/mode/music/music_key_msg_table.o \
	$(ROOT)/apps/earphone/mode/music/music_app_msg_handler.o \


objs += \
	$(ROOT)/apps/earphone/mode/linein/linein.o \
	$(ROOT)/apps/earphone/mode/linein/linein_dev.o \
	$(ROOT)/apps/earphone/mode/linein/linein_key_msg_table.o \


objs += \
	$(ROOT)/apps/earphone/mode/pc/pc.o \
	$(ROOT)/apps/earphone/mode/pc/pc_key_msg_table.o \


objs += \
	$(ROOT)/apps/earphone/audio/mix_record_api.o \


objs += \
	$(ROOT)/apps/earphone/mode/bt/le_audio/le_audio_common.o \


objs += \
	$(ROOT)/apps/earphone/mode/bt/le_audio/cig/app_le_connected.o \
	$(ROOT)/apps/earphone/mode/bt/le_audio/cig/le_connected.o \
	$(ROOT)/apps/earphone/mode/bt/le_audio/cig/le_connected_config.o \


objs += \
	$(ROOT)/apps/earphone/mode/bt/le_audio/big/app_le_auracast.o \
	$(ROOT)/apps/earphone/mode/bt/le_audio/big/le_broadcast.o \
	$(ROOT)/apps/earphone/mode/bt/le_audio/big/le_broadcast_config.o \


