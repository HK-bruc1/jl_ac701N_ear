# 需要编译的 .c 文件
c_SRC_FILES := \
	apps/common/app_mode_manager/app_mode_manager.c \
	apps/common/bt_common/bt_fre_offset_storage.c \
	apps/common/cJSON/cJSON.c \
	apps/common/clock_manager/clock_manager.c \
	apps/common/debug/debug.c \
	apps/common/debug/debug_lite.c \
	apps/common/debug/debug_uart_config.c \
	apps/common/dev_manager/dev_manager.c \
	apps/common/device/storage_device/norflash/norflash_sfc.c \
	apps/common/fat_nor/virfat_flash.c \
	apps/common/temp_trim/dtemp_pll_trim.c \
	apps/common/third_party_profile/common/3th_profile_api.c \
	apps/common/third_party_profile/multi_protocol_main.c \
	apps/earphone/app_main.c \
	apps/earphone/audio/jlstream_event_handler.c \
	apps/earphone/audio/scene_switch.c \
	apps/earphone/audio/tone_table.c \
	apps/earphone/audio/vol_sync.c \
	apps/earphone/battery/battery_level.c \
	apps/earphone/battery/charge.c \
	apps/earphone/battery/charge_clibration.c \
	apps/earphone/battery/charge_device_handle.c \
	apps/earphone/battery/charge_store.c \
	apps/earphone/ble/ble_adv.c \
	apps/earphone/ble/bt_ble.c \
	apps/earphone/board/br28/board_config.c \
	apps/earphone/board/instruction_forbidden.c \
	apps/earphone/board/sdk_board_config.c \
	apps/earphone/demo/att_over_edr_demo.c \
	apps/earphone/demo/debug_record_demo.c \
	apps/earphone/demo/pbg_demo.c \
	apps/earphone/demo/read_sn_demo.c \
	apps/earphone/demo/trans_data_demo.c \
	apps/earphone/device_config.c \
	apps/earphone/log_config/app_config.c \
	apps/earphone/log_config/lib_btctrler_config.c \
	apps/earphone/log_config/lib_btstack_config.c \
	apps/earphone/log_config/lib_driver_config.c \
	apps/earphone/log_config/lib_media_config.c \
	apps/earphone/log_config/lib_net_config.c \
	apps/earphone/log_config/lib_system_config.c \
	apps/earphone/log_config/lib_update_config.c \
	apps/earphone/message/adapter/app_msg.c \
	apps/earphone/message/adapter/audio_player.c \
	apps/earphone/message/adapter/battery.c \
	apps/earphone/message/adapter/btstack.c \
	apps/earphone/message/adapter/driver.c \
	apps/earphone/message/adapter/eartouch.c \
	apps/earphone/message/adapter/key.c \
	apps/earphone/message/adapter/tws.c \
	apps/earphone/mode/bt/a2dp_play.c \
	apps/earphone/mode/bt/bt_background.c \
	apps/earphone/mode/bt/bt_call_kws_handler.c \
	apps/earphone/mode/bt/bt_event_func.c \
	apps/earphone/mode/bt/bt_key_func.c \
	apps/earphone/mode/bt/bt_key_msg_table.c \
	apps/earphone/mode/bt/bt_net_event.c \
	apps/earphone/mode/bt/bt_slience_detect.c \
	apps/earphone/mode/bt/bt_tws.c \
	apps/earphone/mode/bt/dual_a2dp_play.c \
	apps/earphone/mode/bt/dual_conn.c \
	apps/earphone/mode/bt/dual_phone_call.c \
	apps/earphone/mode/bt/earphone.c \
	apps/earphone/mode/bt/eartch_event_deal.c \
	apps/earphone/mode/bt/kws_voice_event_deal.c \
	apps/earphone/mode/bt/low_latency.c \
	apps/earphone/mode/bt/phone_call.c \
	apps/earphone/mode/bt/poweroff.c \
	apps/earphone/mode/bt/sniff.c \
	apps/earphone/mode/bt/tone.c \
	apps/earphone/mode/bt/tws_dual_conn.c \
	apps/earphone/mode/bt/tws_dual_share.c \
	apps/earphone/mode/bt/tws_phone_call.c \
	apps/earphone/mode/bt/tws_poweroff.c \
	apps/earphone/mode/common/app_default_msg_handler.c \
	apps/earphone/mode/idle/idle.c \
	apps/earphone/mode/idle/idle_key_msg_table.c \
	apps/earphone/mode/key_tone.c \
	apps/earphone/mode/mic_effect/mic_effect.c \
	apps/earphone/mode/power_on/power_on.c \
	apps/earphone/test/rssi.c \
	apps/earphone/test/tws_role_switch.c \
	apps/earphone/third_part/app_protocol_deal.c \
	apps/earphone/tools/app_ancbox.c \
	apps/earphone/tools/app_anctool.c \
	apps/earphone/tools/app_key_dut.c \
	apps/earphone/tools/app_testbox.c \
	apps/earphone/ui/led/led_config.c \
	apps/earphone/ui/led/led_ui_msg_handler.c \
	apps/earphone/user_cfg.c \


# 需要编译的 .S 文件
S_SRC_FILES := \
	apps/earphone/sdk_version.z.S \


# 需要编译的 .s 文件
s_SRC_FILES :=


# 需要编译的 .cpp 文件
cpp_SRC_FILES :=


# 需要编译的 .cc 文件
cc_SRC_FILES :=


# 需要编译的 .cxx 文件
cxx_SRC_FILES :=


include build/fileList.mk

# 链接参数
LFLAGS := \
	--plugin-opt=-pi32v2-always-use-itblock=false \
	--plugin-opt=-enable-ipra=true \
	--plugin-opt=-pi32v2-merge-max-offset=4096 \
	--plugin-opt=-pi32v2-enable-simd=true \
	--plugin-opt=mcpu=r3 \
	--plugin-opt=-global-merge-on-const \
	--plugin-opt=-inline-threshold=5 \
	--plugin-opt=-inline-max-allocated-size=32 \
	--plugin-opt=-inline-normal-into-special-section=true \
	--plugin-opt=-dont-used-symbol-list=malloc,free,sprintf,printf,puts,putchar \
	--plugin-opt=save-temps \
	--plugin-opt=-pi32v2-enable-rep-memop \
	--plugin-opt=-warn-stack-size=256 \
	--sort-common \
	--plugin-opt=-used-symbol-file=apps/earphone/sdk_used_list.used \
	--plugin-opt=-enable-movable-region=true \
	--plugin-opt=-movable-region-section-prefix=.movable.slot. \
	--plugin-opt=-movable-region-stub-section-prefix=.movable.stub. \
	--plugin-opt=-movable-region-prefix=.movable.region. \
	--plugin-opt=-movable-region-stub-prefix=__movable_stub_ \
	--plugin-opt=-movable-region-stub-swi-number=-2 \
	--plugin-opt=-movable-region-map-file-list=apps/earphone/movable/funcname.txt \
	--plugin-opt=-movable-region-section-map-file-list=apps/earphone/movable/section.txt \
	--plugin-opt=-movable-region-exclude-func-file-list=apps/earphone/movable/exclude.txt \
	--plugin-opt=-pi32v2-large-program=true \
	--plugin-opt=-floating-point-may-trap=true \
	--gc-sections \
	--plugin-opt=-pi32v2-csync-before-rti=true \
	--start-group \
	cpu/br28/liba/cpu.a \
	cpu/br28/liba/system.a \
	cpu/br28/liba/btstack.a \
	cpu/br28/liba/bt_hash_enc.a \
	cpu/br28/liba/rcsp_stack.a \
	cpu/br28/liba/ximalaya_lib.a \
	cpu/br28/liba/google_fps.a \
	cpu/br28/liba/fmna_stack.a \
	cpu/br28/liba/lib_fmy_auth_br28.a \
	cpu/br28/liba/swift_pair.a \
	cpu/br28/liba/libdma-protocol.a \
	cpu/br28/liba/libdma-device-ota.a \
	cpu/br28/liba/btctrler.a \
	cpu/br28/liba/aec.a \
	cpu/br28/liba/media.a \
	cpu/br28/liba/libepmotion.a \
	cpu/br28/liba/libAptFilt_pi32v2_OnChip.a \
	cpu/br28/liba/libEchoSuppress_pi32v2_OnChip.a \
	cpu/br28/liba/libNoiseSuppress_pi32v2_OnChip.a \
	cpu/br28/liba/libSplittingFilter_pi32v2_OnChip.a \
	cpu/br28/liba/libDelayEstimate_pi32v2_OnChip.a \
	cpu/br28/liba/libSMS_TDE_pi32v2_OnChip.a \
	cpu/br28/liba/libSMS_TDE_Stereo_pi32v2_OnChip.a \
	cpu/br28/liba/libDualMicSystem_pi32v2_OnChip.a \
	cpu/br28/liba/libDualMicSystem_flexible_pi32v2_OnChip.a \
	cpu/br28/liba/libAdaptiveEchoSuppress_pi32v2_OnChip.a \
	cpu/br28/liba/libOpcore_maskrom_pi32v2_OnChip.a \
	cpu/br28/liba/lib_advaudio_plc.a \
	cpu/br28/liba/lib_esco_audio_plc.a \
	cpu/br28/liba/noisegate.a \
	cpu/br28/liba/lib_resample_cal.a \
	cpu/br28/liba/lib_resample_fast_cal.a \
	cpu/br28/liba/sbc_eng_lib.a \
	cpu/br28/liba/lib_sbc_msbc.a \
	cpu/br28/liba/lib_mp3_dec.a \
	cpu/br28/liba/lib_f2a_dec.a \
	cpu/br28/liba/lib_ogg_opus_dec.a \
	cpu/br28/liba/opus_enc_lib.a \
	cpu/br28/liba/aac_enc_lib.a \
	cpu/br28/liba/libLHDCv4DecodeLib.a \
	cpu/br28/liba/libLHDCv5DecodeLib.a \
	cpu/br28/liba/lib_ldac_dec.a \
	cpu/br28/liba/lib_ldac_enc.a \
	cpu/br28/liba/mp3_decstream_lib.a \
	cpu/br28/liba/lib_mty_dec.a \
	cpu/br28/liba/lib_wav_dec.a \
	cpu/br28/liba/lib_flac_dec.a \
	cpu/br28/liba/lib_m4a_dec.a \
	cpu/br28/liba/lib_ape_dec.a \
	cpu/br28/liba/lib_wma_dec.a \
	cpu/br28/liba/agreement.a \
	cpu/br28/liba/libjlsp.a \
	cpu/br28/liba/libjlsp_kws.a \
	cpu/br28/liba/libkwscommon.a \
	cpu/br28/liba/JL_Phone_Call.a \
	cpu/br28/liba/crossover_coff.a \
	cpu/br28/liba/drc.a \
	cpu/br28/liba/lib_drc_advance.a \
	cpu/br28/liba/lib_drc_detect.a \
	cpu/br28/liba/lib_voiceChanger.a \
	cpu/br28/liba/VirtualBass.a \
	cpu/br28/liba/lib_virtual_bass_classic.a \
	cpu/br28/liba/lib_virtual_bass_pro.a \
	cpu/br28/liba/lib_point360_td.a \
	cpu/br28/liba/SensorCalib.a \
	cpu/br28/liba/SpatialAudio.a \
	cpu/br28/liba/spatial_imp.a \
	cpu/br28/liba/SomatosensoryControl.a \
	cpu/br28/liba/DynamicEQ.a \
	cpu/br28/liba/DynamicEQPro.a \
	cpu/br28/liba/lib_icsd_adt.a \
	cpu/br28/liba/lib_icsd_common.a \
	cpu/br28/liba/lib_icsd_dot.a \
	cpu/br28/liba/lib_icsd_anc_v2.a \
	cpu/br28/liba/lib_icsd_common_v2.a \
	cpu/br28/liba/lib_icsd_aeq.a \
	cpu/br28/liba/lib_icsd_afq.a \
	cpu/br28/liba/lib_icsd_cmp.a \
	cpu/br28/liba/lib_icsd_wind.a \
	cpu/br28/liba/lib_icsd_wat.a \
	cpu/br28/liba/lib_icsd_vdt.a \
	cpu/br28/liba/lib_icsd_ein.a \
	cpu/br28/liba/lib_icsd_avc.a \
	cpu/br28/liba/lib_icsd_howl.a \
	cpu/br28/liba/libjlsp_kws_far_keyword.a \
	cpu/br28/liba/libjlsp_kws_india_english.a \
	cpu/br28/liba/3DSpatial.a \
	cpu/br28/liba/lib_plateReverb_adv.a \
	cpu/br28/liba/lib_reverb_cal.a \
	cpu/br28/liba/lib_AutoWah.a \
	cpu/br28/liba/lib_Chorus.a \
	cpu/br28/liba/lib_FrequencyShift.a \
	cpu/br28/liba/howling.a \
	cpu/br28/liba/lib_HarmonicExciter.a \
	cpu/br28/liba/libllns.a \
	cpu/br28/liba/libDualMicSystem_Hybrid.a \
	cpu/br28/liba/libDualMicSystem_Awn.a \
	cpu/br28/liba/lib_ThreeD_effect.a \
	cpu/br28/liba/lib_SurroundEffect.a \
	cpu/br28/liba/lib_StereoWidener.a \
	cpu/br28/liba/lib_PcmDelay.a \
	cpu/br28/liba/crossover_coff.a  \
	cpu/br28/liba/lib_limiter.a \
	cpu/br28/liba/lib_iir_filter.a \
	cpu/br28/liba/spatial_adv.a \
	cpu/br28/liba/le_audio_profile.a \
	cpu/br28/liba/lib_jla_codec.a \
	cpu/br28/liba/lib_lc3_codec.a \
	cpu/br28/liba/lib_jl_codec_common.a \
	cpu/br28/liba/lib_jla_v2_codec.a \
	cpu/br28/liba/lib_noisegate_pro.a \
	cpu/br28/liba/lib_eq_design_coeff.a \
	cpu/br28/liba/libSAS_pi32v2_OnChip.a \
	cpu/br28/liba/lib_phaser.a \
	cpu/br28/liba/lib_flanger.a \
	cpu/br28/liba/lib_chorus_adv.a \
	cpu/br28/liba/lib_pingpong_echo.a \
	cpu/br28/liba/libStereoSpatialWider_pi32v2_OnChip.a \
	cpu/br28/liba/libdistortion.a \
	cpu/br28/liba/lib_frequency_compressor.a \
	cpu/br28/liba/lib_gain_mix.a \
	cpu/br28/liba/liblhdc_x_edge.a \
	cpu/br28/liba/libFFT_pi32v2_OnChip.a \
	cpu/br28/liba/lib_wtg_dec.a \
	cpu/br28/liba/bfilterfun_lib.a \
	cpu/br28/liba/lib_wtgv2_rom_dec.a \
	cpu/br28/liba/lib_bt_aac_dec.a \
	cpu/br28/liba/crypto_toolbox_Osize.a \
	cpu/br28/liba/lib_dns.a \
	apps/common/third_party_profile/tuya_protocol/sdk/lib/libtuya_lib.a \
	cpu/br28/liba/update.a \
	cpu/br28/liba/cbuf.a \
	cpu/br28/liba/lbuf.a \
	cpu/br28/liba/printf.a \
	cpu/br28/liba/device.a \
	cpu/br28/liba/fs.a \
	cpu/br28/liba/ascii.a \
	cpu/br28/liba/cfg_tool.a \
	cpu/br28/liba/vm.a \
	cpu/br28/liba/ftl.a \
	cpu/br28/liba/debug_record.a \
	cpu/br28/liba/lzma_dec.a \
	cpu/br28/liba/chargestore.a \
	cpu/br28/liba/printf.a  \
	cpu/br28/liba/cbuf.a  \
	cpu/br28/liba/vm.a  \
	cpu/br28/liba/fs.a  \
	cpu/br28/liba/cfg_tool.a  \
	--end-group \
	-Tcpu/br28/sdk.ld \
	-M=cpu/br28/tools/sdk.map \
	--plugin-opt=mcpu=r3 \
	--plugin-opt=-mattr=+fprev1 \


LIBPATHS := \
	-L$(SYS_LIB_DIR) \


LIBS := \
	$(SYS_LIB_DIR)/libm.a \
	$(SYS_LIB_DIR)/libc.a \
	$(SYS_LIB_DIR)/libm.a \
	$(SYS_LIB_DIR)/libcompiler-rt.a \



c_OBJS    := $(c_SRC_FILES:%.c=%.c.o)
S_OBJS    := $(S_SRC_FILES:%.S=%.S.o)
s_OBJS    := $(s_SRC_FILES:%.s=%.s.o)
cpp_OBJS  := $(cpp_SRC_FILES:%.cpp=%.cpp.o)
cxx_OBJS  := $(cxx_SRC_FILES:%.cxx=%.cxx.o)
cc_OBJS   := $(cc_SRC_FILES:%.cc=%.cc.o)

OBJS      := $(c_OBJS) $(S_OBJS) $(s_OBJS) $(cpp_OBJS) $(cxx_OBJS) $(cc_OBJS)
DEP_FILES := $(OBJS:%.o=%.d)


OBJS      := $(addprefix $(BUILD_DIR)/, $(OBJS))
DEP_FILES := $(addprefix $(BUILD_DIR)/, $(DEP_FILES))


# 表示下面的不是一个文件的名字，无论是否存在 all, clean, pre_build 这样的文件
# 还是要执行命令
# see: https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean pre_build

# 不要使用 make 预设置的规则
# see: https://www.gnu.org/software/make/manual/html_node/Suffix-Rules.html
.SUFFIXES:

all: pre_build $(OUT_ELF)
	$(info +POST-BUILD)
	$(QUITE) $(RUN_POST_SCRIPT) sdk

pre_build:
	$(info +PRE-BUILD)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P apps/earphone/sdk_used_list.c -o apps/earphone/sdk_used_list.used
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P apps/earphone/movable/section.c -o apps/earphone/movable/section.txt
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P cpu/br28/sdk_ld.c -o cpu/br28/sdk.ld
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P cpu/br28/tools/download.c -o $(POST_SCRIPT)
	$(QUITE) $(FIXBAT) $(POST_SCRIPT)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P cpu/br28/tools/isd_config_rule.c -o cpu/br28/tools/isd_config.ini

ifeq ($(LINK_AT), 1)
$(OUT_ELF): $(OBJS)
	$(info +LINK $@)
	$(shell $(MKDIR) $(@D))
	$(file >$(OBJ_FILE), $(OBJS))
	$(QUITE) $(LD) -o $(OUT_ELF) @$(OBJ_FILE) $(LFLAGS) $(LIBPATHS) $(LIBS)
else
$(OUT_ELF): $(OBJS)
	$(info +LINK $@)
	$(shell $(MKDIR) $(@D))
	$(QUITE) $(LD) -o $(OUT_ELF) $(OBJS) $(LFLAGS) $(LIBPATHS) $(LIBS)
endif


$(BUILD_DIR)/%.c.o : %.c
	$(info +CC $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

$(BUILD_DIR)/%.S.o : %.S
	$(info +AS $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

$(BUILD_DIR)/%.s.o : %.s
	$(info +AS $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

$(BUILD_DIR)/%.cpp.o : %.cpp
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

$(BUILD_DIR)/%.cxx.o : %.cxx
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

$(BUILD_DIR)/%.cc.o : %.cc
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

-include $(DEP_FILES)
