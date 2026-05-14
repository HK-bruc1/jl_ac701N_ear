





















c_SRC_FILES += audio/framework/plugs/source/a2dp_file.c audio/framework/plugs/source/a2dp_streamctrl.c audio/framework/plugs/source/esco_file.c audio/framework/plugs/source/adc_file.c audio/framework/plugs/source/multi_ch_adc_file.c audio/framework/nodes/esco_tx_node.c audio/framework/nodes/plc_node.c audio/framework/nodes/volume_node.c
c_SRC_FILES += audio/framework/nodes/cvp_sms_node.c



c_SRC_FILES += audio/framework/nodes/cvp_dms_node.c
c_SRC_FILES += audio/common/audio_node_config.c audio/common/audio_dvol.c audio/common/audio_general.c audio/common/audio_build_needed.c audio/common/online_debug/aud_data_export.c audio/common/online_debug/audio_online_debug.c audio/common/online_debug/audio_capture.c audio/common/audio_plc.c audio/common/audio_noise_gate.c audio/common/audio_ns.c audio/common/audio_utils.c audio/common/audio_export_demo.c audio/common/amplitude_statistic.c audio/common/frame_length_adaptive.c audio/common/bt_audio_energy_detection.c audio/common/audio_event_handler.c audio/common/debug/audio_debug.c audio/common/power/mic_power_manager.c audio/common/audio_volume_mixer.c audio/common/audio_effect_verify.c audio/common/pcm_data/sine_pcm.c
c_SRC_FILES += audio/common/demo/hw_math_v2_demo.c
c_SRC_FILES += audio/interface/player/tone_player.c audio/interface/player/ring_player.c audio/interface/player/a2dp_player.c audio/interface/player/esco_player.c audio/interface/player/key_tone_player.c audio/interface/player/dev_flow_player.c audio/interface/player/adda_loop_player.c audio/interface/player/linein_player.c audio/interface/player/ai_rx_player.c
c_SRC_FILES += audio/interface/recoder/esco_recoder.c audio/interface/recoder/ai_voice_recoder.c audio/interface/recoder/dev_flow_recoder.c
c_SRC_FILES += audio/effect/eq_config.c audio/effect/spk_eq.c audio/effect/audio_voice_changer_api.c audio/effect/esco_ul_voice_changer.c audio/effect/bass_treble.c audio/effect/audio_dc_offset_remove.c audio/effect/effects_adj.c audio/effect/effects_dev.c audio/effect/effects_default_param.c audio/effect/node_param_update.c audio/effect/scene_update.c
c_SRC_FILES += audio/effect/somatosensory/audio_somatosensory.c





c_SRC_FILES += audio/CVP/audio_aec.c audio/CVP/audio_cvp.c audio/CVP/audio_cvp_sms_vf.c audio/CVP/audio_cvp_dms.c audio/CVP/audio_cvp_3mic.c audio/CVP/audio_cvp_v3.c audio/CVP/audio_cvp_online.c audio/CVP/audio_cvp_demo.c audio/CVP/audio_cvp_develop.c audio/CVP/audio_cvp_sync.c audio/CVP/audio_cvp_ais_3mic.c audio/CVP/audio_cvp_ref_task.c audio/CVP/audio_cvp_config.c
c_SRC_FILES += audio/interface/player/tws_tone_player.c
c_SRC_FILES += audio/framework/plugs/source/linein_file.c
c_SRC_FILES += audio/cpu/common.c
c_SRC_FILES += apps/common/config/bt_profile_config.c
c_SRC_FILES += apps/common/update/update.c apps/common/update/testbox_update.c apps/common/update/testbox_uart_update.c
c_SRC_FILES += apps/common/device/key/key_driver.c



c_SRC_FILES += apps/common/device/key/iokey.c
c_SRC_FILES += apps/common/device/usb/device/usb_pll_trim.c
c_SRC_FILES += apps/common/device/usb/device/msd_upgrade.c
c_SRC_FILES += cpu/components/iic_soft.c cpu/components/iic_api.c cpu/components/ir_encoder.c cpu/components/ir_decoder.c cpu/components/rdec_soft.c
c_SRC_FILES += cpu/config/gpio_file_parse.c cpu/config/lib_power_config.c
c_SRC_FILES += audio/cpu/br28/audio_setup.c audio/cpu/br28/audio_mic_capless.c audio/cpu/br28/audio_dai/audio_pdm.c audio/cpu/br28/audio_dai/audio_tdm.c audio/cpu/br28/audio_config.c audio/cpu/br28/audio_configs_dump.c
c_SRC_FILES += audio/cpu/br28/audio_accelerator/hw_fft.c
c_SRC_FILES += audio/cpu/br28/audio_demo/audio_adc_demo.c



c_SRC_FILES += audio/cpu/br28/audio_demo/audio_dac_demo.c audio/cpu/br28/audio_demo/audio_fft_demo.c audio/cpu/br28/audio_demo/audio_pdm_demo.c
c_SRC_FILES += cpu/br28/setup.c cpu/br28/overlay_code.c cpu/br28/private_iis.c
c_SRC_FILES += cpu/br28/charge/charge.c cpu/br28/charge/charge_config.c





c_SRC_FILES += cpu/br28/charge/chargestore.c cpu/br28/charge/chargestore_config.c
c_SRC_FILES += cpu/br28/periph/rdec.c cpu/br28/periph/mcpwm.c
c_SRC_FILES += cpu/br28/power/key_wakeup.c


c_SRC_FILES += cpu/br28/power/power_app.c cpu/br28/power/power_config.c cpu/br28/power/power_port.c
