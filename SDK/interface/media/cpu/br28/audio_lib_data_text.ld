

        media_data_code_start = .;

        /*使用动态加载时*/
#ifdef CONFIG_CODE_MOVABLE_ENABLE
#if AUDIO_ENCODER_AT_RAM
		*(.msbc_enc)
		*(.cvsd_codec)
#endif
#if AUD_AAC_DEC_AT_RAM
		aac_dec_code_begin = .;
		*(.bt_aac_dec_code)
		*(.bt_aac_dec_sparse_code)
		aac_dec_code_end = .;
		aac_dec_code_size  = aac_dec_code_end - aac_dec_code_begin ;

		. = ALIGN(4);
		bt_aac_dec_const_begin = .;
		*(.bt_aac_dec_const)
		*(.bt_aac_dec_sparse_const)
		bt_aac_dec_const_end = .;
		bt_aac_dec_const_size = bt_aac_dec_const_end -  bt_aac_dec_const_begin ;

		. = ALIGN(4);
		*(.aac_mem)
		*(.aac_ctrl_mem)
#endif
#endif /*CONFIG_CODE_MOVABLE_ENABLE*/ 

        . = ALIGN(4);
        *(.pSOUND360TD_cal_sparse_code)
        *(.pSOUND360TD_cal_code)
        *(.pSOUND360TD_cal_const)
        *(.spatial_mem)
        *(.TDSpatial.text)
        *(.TDSpatial.text.cache.L2)
        *(.sbc_eng_code)
#if (TCFG_CODE_RUN_RAM_FM_MODE)
        *(.pcm_code)
        *(.pcm_const)
		*(.audio_dec_pcm_code)
		*(.audio_dec_pcm_const)
		*(.stream_code_nor_run)			// 数据流普通输出处理
		*(.mixer_code_nor_run)			// mixer数据流直通输出
		*(.audio_codec_code)			// 解码任务流程
		*(.audio_codec_code_read_file)	// 解码读文件
		*(.audio_src_code_nor_run)		// audio_src运行处理
		*(.src_base_code_nor_run)		// src_base运行处理
		*(.audio_dac_code_nor_run)		// audio_dac普通输出运行处理
		*(.audio_cfifo_code_nor_run)	// audio_cfifo普通运行处理
		*(.audio_track_code)
		*(.audio_track_const)
#endif /*(TCFG_CODE_RUN_RAM_FM_MODE)*/

#if AUDIO_EFFECTS_SURROUND_AT_RAM 
		*(.audio_surround_code)
		*(.audio_surround_const)
		*(.sur_cal_common_code)
		*(.sur_TYPE0_cal_code)
	    *(.sur_TYPE1_cal_code)
		*(.sur_TYPE0_cal_const)
	    *(.sur_TYPE1_cal_const)
		/* *(.sur_LRDRIFT2_cal_code) */
		/* *(.sur_LRDRIFT2_cal_const) */
		/* *(.sur_ROTATE_cal_code) */
		/* *(.sur_ROTATE_cal_const) */
        /* *(.sur_LRDRIFT_cal_code) */
        /* *(.sur_LRDRIFT_cal_const) */
#endif
        . = ALIGN(4);
#if TCFG_CODE_RUN_RAM_LC3_CODE
        *(.lc3_dec.text.const)
        *(.lc3_dec.text)
        *(.lc3_enc.text.const)
        *(.lc3_enc.text)
        *(.lc3_dec_const)
        *(.lc3_dec_code)
        *(.lc3_codec_c_const)
        *(.lc3_codec_c_code)
        *(.lc3_codec_ari_c_const)
        *(.lc3_codec_ari_c_code)
        *(.lc3_codec_d_code)
        *(.lc3_codec_e_code)
        *(.lc3_codec_c_sparse_code)
        *(.lc3_codec_t8k_code)
        *(.lc3_codec_t16k_code)
        *(.lc3_codec_t24k_code)
        *(.lc3_codec_t32k_code)
        *(.lc3_codec_t441k_code)
        . = ALIGN(4);
#endif
#if TCFG_CODE_RUN_RAM_JLA_CODE
		*(.jla_dec.text.const)
	    *(.jla_dec.text)
	    *(.jla_enc.text.const)
	    *(.jla_enc.text)
	    *(.jla_dec_const)
	    *(.jla_dec_code)
		*(.jla_codec_c_const)
		*(.jla_codec_c_code)
		*(.jla_codec_ari_c_const)
		*(.jla_codec_ari_c_code)
		*(.jla_codec_d_code)
		*(.jla_codec_e_code)
		*(.jla_codec_c_sparse_code)
		*(.jla_codec_t8k_code)
		*(.jla_codec_t16k_code)
		*(.jla_codec_t24k_code)
		*(.jla_codec_t32k_code)
	    *(.jla_codec_t441k_code)
		. = ALIGN(4);
#endif

#if AFx_VBASS_AT_RAM
		*(.vbss.text.cache.L2.code)	
#endif

#if AFx_REVERB_AT_RAM
		*(.reverb_cal.text.cache.L2.run)
	    *(.plateReverb_adv.text.cache.L2.run)
#endif

#if AFx_ECHO_AT_RAM

#endif

#if AFx_VOICECHANGER_AT_RAM
	    *(.voiceChanger.text.cache.L2.run)
		*(.voiceChanger.text.const)
#endif

#if AFx_ADVAUDIO_PLC_AT_RAM
 		*(.advaudio_plc.text.cache.L2.run)
#endif

#if AFx_DRC_AT_RAM
		*(.drc.text.cache.L2.code)
		*(.crossover.text.cache.L2.code)
		*(.BandMerge.text.cache.L2.code)
#endif

#if AFx_HARMONIC_EXCITER_AT_RAM
		*(.HarmonicExciter.text.cache.L2.run)
	    *(.HarmonicExciter.text.const)
#endif

#if AFx_DYN_EQ_AT_RAM 				
		*(.dynamiceq.text.cache.L2.code)
#endif

#if AFx_NOTCH_HOWLING_AT_RAM
		*(.notchhowling.text.cache.L2.code) 
 		*(.notchhowling.text.cache.L2.const)
#endif

#if AFx_FREQ_SHIFT_AT_RAM
		*(.FrequencyShift.text.cache.L2.run)
		*(.FrequencyShift.text.const)
#endif

#if AFx_NOISEGATE_AT_RAM
		*(.noisegate.text.cache.L2.code)
#endif

 		*(.mixer.text.cache.L2)
		*(.dac.text.cache.L2)
		*(.convert.text.cache.L2.code)
		*(.VocalRemove.text.cache.L2.code)

#if AUD_AAC_DEC_AT_RAM
 		*(.bt_aac_dec.text.cache.L2.run)
 		*(.bt_aac_dec.text.cache.L2.const)
#endif

        *(.ldac_dec.text)
	    *(.ldac_dec.text.const)
        *(.ldac.text)
	    *(.ldac.text.const)

        . = ALIGN(4);
        media_data_code_end = .;



