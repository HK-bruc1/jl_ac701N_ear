
const JLSP_mic_v3_cfg_t mic_init_cfg = {
    .mic0En = 1,
    .mic1En = 1,
    .mic2En = 1,
    .mic3En = 1,
    .mic0PreGain = 1.0f,
    .mic1PreGain = 1.0f,
    .mic2PreGain = 1.0f,
    .mic3PreGain = 1.0f,
    .micEnNum = 4
};

//出现单个麦掩蔽参数
const JLSP_micsel_v3_cfg_t micsel_init_cfg = {
    .detectTime = 1.0f,
    .detectEngDiffTh = 20.0f,
    .detectEngLowerBound = -55.0f,
    .detMaxFrequency = 8000,
    .detMinFrequency = 200,
    .onlyDetect = 0
};

//beamforming 参数配置
const JLSP_bf_v3_cfg_t bf_init_cfg = {
    .encProcessMaxFrequency = 8000,
    .encProcessMinFrequency = 0,
    .micDistance = 0.015f,
    .sirMaxFreq = 3000,
    .targetSignalDegradation = 1.0f,
    .aggressFactor = 1.0f,
    .minSuppress = 0.1f,

    .steer_vec1 = NULL,
    .steer_vec2 = NULL,
    .ds_steer_vec = NULL,

    .bfDualAdaptiveFilter = 0,
    .bfDrvTypeHyper = 0,
    .bfMainLobeMic = 1,
    .bfSideLobeMic = 0,
    /*可配用宏*/
    //V1 + post_en = 1<==>1代bf    V2 + post_en<==> 3代算法且无需配置ENCMIC补偿值
    .supressFactor = 0.6f,
#if (CVP_BF_VERSION == JLSP_BF_V100)
    .type = BF_TYPE_V1,
    .bfPost_en = 1
#else
    .type = BF_TYPE_V2,
    .bfPost_en = 0
#endif
};


const JLSP_fusion_cfg_t fusion_init_cfg = {
    .fusionFreq = 2000,
    .dBTh1 = 1,
    .dBTh2 = 105,
    .type = FB_FUSION_TYPE_V5,
};

const JLSP_aec_v3_cfg_t aec_cfg_default = {
    .aecProcessMaxFrequency = 8000,
    .aecProcessMinFrequency = 0,
    .muf = 0.05f
};

const JLSP_nlp_v3_cfg_t nlp_cfg_default = {
    .nlpProcessMaxFrequency = 8000,
    .nlpProcessMinFrequency = 0,
    .preEnhance = 0,
    .overDrive = 5.0f
};

JLSP_aec_v3_cfg_t aec_fb_cfg_default = {
    .aecProcessMaxFrequency = 2000,
    .aecProcessMinFrequency = 0,
    .muf = 0.05f
};

const JLSP_nlp_v3_cfg_t nlp_fb_cfg_default = {
    .nlpProcessMaxFrequency = 2000,
    .nlpProcessMinFrequency = 0,
    .preEnhance = 1,
    .overDrive = 5.0f
};

const JLSP_dns_v3_cfg_t dns_init_cfg = {
    .aggressFactor = 1.0f,
    .minSuppress = 0.001f,
    .initNoiseLvl = 2.2e4f
};

const JLSP_wind_v3_cfg_t wn_init_cfg = {
    .windProbHighTh = 0.55f,
    .windProbLowTh = 0.15f,
    .windCountEnterTh = 20,
    .windCountExitTh = 20,
    .windEngDbTh = 95.0f
};

const JLSP_drc_v3_cfg_t drc_init_cfg = {
    .compressAttackTime = 0.0003f,
    .compressReleaseTime = 0.1f,
    .ngAttackTime = 0.1f,
    .ngReleaseTime = 0.01f,
    .kneeWidth = 2.0f,
    .compressSlope = 1.0f / 40.0f,
    .kneeThresholdDb = -6.0f,
    .limitMagDb = 0.0f,
    .noiseGateThresholdDb = -50.0f,
    .ngStepOutDb = -1.2f,
    .makeUpGain = 14.0f,
    .samplerate = 16000
};

const JLSP_single_aecnlp_v3_cfg_t aecnlp_init_cfg = {
    .enableBit = 0x03,
    .samplerate = 16000,
    .preGainDb = 10.f,
};

const JLSP_hybrid_v3_cfg_t hybrid_init_cfg = {
    .enableBit = 0x3c,
    .enableBitEx = 0x00,

    .hybridFbTransferFunc = NULL,
    .hybridWbEqVec = NULL,
    .hybridNbEqVec = NULL,
    .hybridFbCompenDb = 0.0f,

    .samplerate = 16000,
    .hybridPreGainDb  = 18.0f,
    .hybridProcessMaxFrequency = 8000,
    .hybridProcessMinFrequency = 0,
    .hybridCompenDb = 10.0f,

    .aggressFactor = 1.0f,
    .minSupress = 0.001f,

    .externalEnableBit = 0x00,
};

const JLSP_single_v3_cfg_t single_init_cfg = {

    .enableBit = 0x14,
    .singleWbEq = NULL,
    .singleNbEq = NULL,
    .samplerate = 16000,
    .spe_att_en = 0,	// 1 for enable, else for disable.
    .post_pro_en = 0,		// 1 for enable, else for disable.
    .processMaxFrequency = 8000,
    .processMinFrequency = 0,
    .preGainDb = 0.0f,
    .singleCompenDb = 0.0f,
    .aggressFactor = 1.0f,
    .minSupress = 0.001f,
    .externalEnableBit = 0x00,
};

const JLSP_dual_bf_v3_cfg_t dual_bf_init_cfg = {

    .enableBit = 0x3f,

    .dualPhaseCompenVec = NULL,
    .dualWbEqVec = NULL,
    .dualNbEqVec = NULL,

    .samplerate = 16000,
    .dualPreGainDb = 18.0f,
    .dualProcessMaxFrequency = 8000,
    .dualProcessMinFrequency = 0,
    .dualCompenDb = 10.0f, //18.0f, //bf增益补偿


    .spe_att_en = 0,	// 1 for enable, else for disable.
    .post_pro_en = 0,		// 1 for enable, else for disable.
    .noise_est_en = 0,		// 1 for enable, else for disable.

    .aggressFactor = 1.0f,
    .minSupress = 0.001f,

    .externalEnableBit = 0x00,
};

const JLSP_tri_v3_cfg_t tri_init_cfg = {

    .enableBit = 0x3f,
    .enableBitEx = 0x03,

    .triFbTransferFuncOn = NULL,  //fb -> main传递函数(anc on)
    .triFbTransferFuncOff = NULL, //fb -> main传递函数(anc off)
    .triPhaseCompenVec = NULL,
    .triWbEqVec = NULL,
    .triNbEqVec = NULL,
    .triFbCompenDb = 0.0f,	//fb增益补偿
    //.Tri_TransferMode = 0,
    .spe_att_en = 0,	// 1 for enable, else for disable.
    .post_pro_en = 0,		// 1 for enable, else for disable.
    .noise_est_en = 0,		// 1 for enable, else for disable.

    .samplerate = 16000,
    .triPreGainDb = 18.0f,
    .triProcessMaxFrequency = 8000,
    .triProcessMinFrequency = 0,
    .triCompenDb = 10.0f, //18.0f, //bf增益补偿

    .aggressFactor = 1.0f,
    .minSupress = 0.001f,

    .externalEnableBit = 0x00,

};
