#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".audio_cvp_v3.data.bss")
#pragma data_seg(".audio_cvp_v3.data")
#pragma const_seg(".audio_cvp_v3.text.const")
#pragma code_seg(".audio_cvp_v3.text")
#endif
/*
 ****************************************************************
 *							AUDIO (Sigle DualMic TMS System)
 * File  : audio_cvp_v3.c
 * By    :
 * Notes : 第三代CVP算法
 *
 ****************************************************************
 */
#include "audio_cvp.h"
#include "system/includes.h"
#include "media/includes.h"
#include "effects/eq_config.h"
#include "effects/audio_pitch.h"
#include "circular_buf.h"
#include "audio_cvp_online.h"
#include "cvp_node.h"
#if TCFG_USER_TWS_ENABLE
#include "bt_tws.h"
#endif/*TCFG_USER_TWS_ENABLE*/
#include "overlay_code.h"
#include "audio_dc_offset_remove.h"
#include "audio_cvp_def.h"
#include "effects/audio_gain_process.h"
#include "amplitude_statistic.h"
#include "app_main.h"
#include "lib_h/jlsp_v3_ns.h"
#include "fs/sdfile.h"
#include "online_db_deal.h"
#include "spp_user.h"
#if TCFG_AUDIO_DUT_ENABLE
//#include "audio_dut_control.h"
#endif/*TCFG_AUDIO_DUT_ENABLE*/
#if ((defined TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN) && TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN && \
        TCFG_AUDIO_ANC_ENABLE && TCFG_AUDIO_ANC_WIND_NOISE_DET_ENABLE)
#include "icsd_adt_app.h"
#endif

#if !defined(TCFG_CVP_DEVELOP_ENABLE) || (TCFG_CVP_DEVELOP_ENABLE == 0)
#if TCFG_AUDIO_CVP_V3_MODE
#define LOG_TAG_CONST       AEC_USER
#define LOG_TAG             "[AEC_USER]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"
#include "audio_cvp_debug.c"
#include "adc_file.h"
#include "cvp_v3_config.c"

//*********************************************************************************//
//                          CVP Common Configs                                     //
//*********************************************************************************//
const u8 CONST_JLSP_FF_COMPEN = 1;
const u8 CONST_ACTSEQ_EN = 0;
//代码量控制
const int CVP_V3_ALGO_ENABLEBIT = TCFG_CVP_ALGO_TYPE;

/*CVP_TOGGLE:CVP模块(包括AEC、NLP、NS等)总开关，Disable则数据完全不经过处理，释放资源*/
#define CVP_TOGGLE				1

/*响度指示器*/
#define CVP_LOUDNESS_TRACE_ENABLE		0	//跟踪获取当前mic输入幅值

/* 通过蓝牙spp发送风噪信息
 * 需要同时打开USER_SUPPORT_PROFILE_SPP和APP_ONLINE_DEBUG*/
#define WIND_DETECT_INFO_SPP_DEBUG_ENABLE  0
#if WIND_DETECT_INFO_SPP_DEBUG_ENABLE && (!APP_ONLINE_DEBUG || !TCFG_BT_SUPPORT_SPP)
#error "蓝牙spp发数功能需要打开TCFG_BT_SUPPORT_SPP 和 APP_ONLINE_DEBUG"
#endif

/*数据输出开头丢掉的数据包数*/
#define AEC_OUT_DUMP_PACKET		15
/*数据输出开头丢掉的数据包数*/
#define AEC_IN_DUMP_PACKET		1

/*使能即可跟踪通话过程的内存情况*/
#define CVP_MEM_TRACE_ENABLE	0
//**************************CVP Common Configs End************************************//

extern void aec_code_movable_load(void);
extern void aec_code_movable_unload(void);

__attribute__((weak))u32 usb_mic_is_running()
{
    return 0;
}

struct cvp_hdl {
    volatile u8 start;					//CVP模块状态
    u8 inbuf_clear_cnt;					//CVP输入数据丢掉
    u8 output_fade_in;					//CVP输出淡入使能
    u8 output_fade_in_gain;				//CVP输出淡入增益
    u8 EnableBit;						//CVP使能模块
    u8 input_clear;						//清0输入数据标志
    u16 dump_packet;					//前面如果有杂音，丢掉几包
    struct cvp_attr attr;				//v3 cvp aec模块参数属性
    struct audio_cvp_pre_param_t pre;	//预处理配置
    int algo_type;
#if TCFG_SUPPORT_MIC_CAPLESS
    void *dcc_hdl;
#endif
#if WIND_DETECT_INFO_SPP_DEBUG_ENABLE
    struct spp_operation_t *spp_opt;    //蓝牙spp发送句柄
    u8 spp_cnt;                         //发数间隔
    int wd_flag;                        //风噪状态
    float wd_val;                       //风噪强度(dB)
    char spp_tmpbuf[25];                //打印缓存buf
#endif
};
struct cvp_hdl *cvp_v3 = NULL;

static u8 global_output_way = 0;

void audio_cvp_set_output_way(u8 en)
{
    global_output_way = en;
}

int audio_cvp_probe_param_update(struct audio_cvp_pre_param_t *cfg)
{
    if (cvp_v3) {
        cvp_v3->pre = *cfg;
    }
    return 0;
}

/*
*********************************************************************
*                  Audio CVP Process_Probe
* Description: AEC模块数据前处理回调
* Arguments  : data 数据地址
*			   len	数据长度
* Return	 : 0 成功 其他 失败
* Note(s)    : 在源数据经过AEC模块前，可以增加自定义处理
*********************************************************************
*/
static LOUDNESS_M_STRUCT mic_loudness;
static int audio_cvp_probe(short *talk_mic, short *talk_ref_mic, short *talk_fb_mic, short *ref, u16 len)
{
#if TCFG_AUDIO_MIC_ARRAY_TRIM_ENABLE
    //表示使用主副麦差值计算，且仅减小增益
    if (app_var.audio_mic_array_trim_en) {
        if (app_var.audio_mic_array_diff_cmp != 1.0f) {
            if (app_var.audio_mic_array_diff_cmp > 1.0f) {
                float mic0_gain = 1.0 / app_var.audio_mic_array_diff_cmp;
                GainProcess_16Bit(talk_mic, talk_mic, mic0_gain, 1, 1, 1, len >> 1);
            } else {
                float mic1_gain = app_var.audio_mic_array_diff_cmp;
                GainProcess_16Bit(talk_ref_mic, talk_ref_mic, mic1_gain, 1, 1, 1, len >> 1);
            }
        } else {       //表示使用每个MIC与金机曲线的差值
            GainProcess_16Bit(talk_mic, talk_mic, app_var.audio_mic_cmp.talk, 1, 1, 1, len >> 1);
            GainProcess_16Bit(talk_ref_mic, talk_ref_mic, app_var.audio_mic_cmp.ff, 1, 1, 1, len >> 1);
        }
    }
#endif

#if TCFG_SUPPORT_MIC_CAPLESS
    if (cvp_v3->dcc_hdl) {
        audio_dc_offset_remove_run(cvp_v3->dcc_hdl, (void *)talk_mic, len);
    }
#endif

#if CVP_LOUDNESS_TRACE_ENABLE
    loudness_meter_short(&mic_loudness, talk_mic, len >> 1);
#endif/*CVP_LOUDNESS_TRACE_ENABLE*/

    if (cvp_v3->inbuf_clear_cnt) {
        cvp_v3->inbuf_clear_cnt--;
        if ((cvp_v3->algo_type & CVP_ALGO_1MIC) || (cvp_v3->algo_type & CVP_ALGO_1MIC_AECNLP)) {
            memset(talk_mic, 0, len);
        } else if (cvp_v3->algo_type & CVP_TYPE_2MIC) {
            memset(talk_mic, 0, len);
            memset(talk_ref_mic, 0, len);
        } else if (cvp_v3->algo_type & CVP_ALGO_3MIC) {
            memset(talk_mic, 0, len);
            memset(talk_ref_mic, 0, len);
            memset(talk_fb_mic, 0, len);
        }
    }
    return 0;
}

/*
*********************************************************************
*                  Audio CVP Process_Post
* Description: CVP模块数据后处理回调
* Arguments  : data 数据地址
*			   len	数据长度
* Return	 : 0 成功 其他 失败
* Note(s)    : 在数据处理完毕，可以增加自定义后处理
*********************************************************************
*/
static int audio_cvp_post(s16 *data, u16 len)
{
    //后处理获取风噪信息
#if ((defined TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN) && TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN && \
        TCFG_AUDIO_ANC_ENABLE && TCFG_AUDIO_ANC_WIND_NOISE_DET_ENABLE)
    if (get_cvp_icsd_wind_lvl_det_state()) {
        int wd_flag = 0;
        float wind_lvl = 0.f;
        audio_cvp_v3_get_wind_detect_info(&wd_flag, &wind_lvl)
        audio_cvp_wind_lvl_output_handle(wind_lvl);
    }
#endif

#if WIND_DETECT_INFO_SPP_DEBUG_ENABLE
#if TCFG_USER_TWS_ENABLE
    if ((tws_api_get_role() == TWS_ROLE_MASTER))
#endif
    {
        cvp_v3->spp_cnt ++;
        if ((cvp_v3->attr.EnableBit & WNC_EN) && (cvp_v3->spp_cnt > 20) && cvp_v3->spp_opt && cvp_v3->spp_opt->send_data) {
            cvp_v3->spp_cnt = 0;
            memset(cvp_v3->spp_tmpbuf, 0x20, sizeof(cvp_v3->spp_tmpbuf));
            jlsp_cvp_v3_get_wind_detect_info(&cvp_v3->wd_flag, &cvp_v3->wd_val);
            sprintf(cvp_v3->spp_tmpbuf, "flag:%d, val:%d", cvp_v3->wd_flag, (int)cvp_v3->wd_val);
            cvp_v3->spp_opt->send_data(NULL, cvp_v3->spp_tmpbuf, sizeof(cvp_v3->spp_tmpbuf));
            printf("wd_flag:%d, wd_val:%d", cvp_v3->wd_flag, (int)cvp_v3->wd_val);
        }
    }
#endif
    return 0;
}


/*跟踪系统内存使用情况:physics memory size xxxx bytes*/
static void sys_memory_trace(void)
{
    static int cnt = 0;
    if (cnt++ > 200) {
        cnt = 0;
        mem_stats();
    }
}
/*跟踪系统风噪信息->宽窄带信息->麦克风状态信息*/
/*
void wind_band_trace(void *priv)
{
    int is_wb_state = 0;
    int mic_state = 0;
    int wd_flag = 0;
    float wd_val = 0.f;
    mic_state = audio_cvp_v3_get_mic_state_info(is_wb_state);
    is_wb_state =  audio_cvp_v3_get_bandwidth_info(mic_state);
    audio_cvp_v3_get_wind_detect_info(&wd_flag, &wd_val);
    printf("is_wb_state %d mic_state %d wd_flag %d\n", is_wb_state, mic_state, wd_flag);
    printf("cvp_v3 wd_val(dB):\n");
    put_float(wd_val);
}
*/

int audio_aec_sync_buffer_set(s16 *data, int len)
{
    return cvp_v3_node_output_handle(data, len);
}
/*
 *********************************************************************
 *                  Audio CVP Output Handle
 * Description: AEC模块数据输出回调
 * Arguments  : data 输出数据地址
 *			   len	输出数据长度
 * Return	 : 数据输出消耗长度
 * Note(s)    : None.
 *********************************************************************
 */
static int audio_cvp_output(s16 *data, u16 len)
{
#if (((defined TCFG_KWS_VOICE_RECOGNITION_ENABLE) && TCFG_KWS_VOICE_RECOGNITION_ENABLE) || \
     ((defined TCFG_CALL_KWS_SWITCH_ENABLE) && TCFG_CALL_KWS_SWITCH_ENABLE))
    //Voice Recognition get mic data here
    extern void kws_aec_data_output(void *priv, s16 * data, int len);
    kws_aec_data_output(NULL, data, len);

#endif/*TCFG_KWS_VOICE_RECOGNITION_ENABLE*/

#if CVP_MEM_TRACE_ENABLE
    sys_memory_trace();
#endif/*CVP_MEM_TRACE_ENABLE*/

    if (cvp_v3->dump_packet) {
        cvp_v3->dump_packet--;
        memset(data, 0, len);
    } else  {
        if (cvp_v3->output_fade_in) {
            s32 tmp_data;
            //printf("fade:%d\n",cvp_v3->output_fade_in_gain);
            for (int i = 0; i < len / 2; i++) {
                tmp_data = data[i];
                data[i] = tmp_data * cvp_v3->output_fade_in_gain >> 7;
            }
            cvp_v3->output_fade_in_gain += 12;
            if (cvp_v3->output_fade_in_gain >= 128) {
                cvp_v3->output_fade_in = 0;
            }
        }
    }
    return cvp_v3_node_output_handle(data, len);
}
/*
  目前,宽窄带EQ曲线和ANCON ANCOFF曲线需要使用Hybrid工具导出bin文件
  其中目标曲线选择参考线,其余曲线目标曲线选择EQ曲线
  */
#define AUDIO_DMS_WB_COEFF_FILE 			(FLASH_RES_PATH"wb.bin")     			//宽带EQ
#define AUDIO_DMS_NB_COEFF_FILE 			(FLASH_RES_PATH"nb.bin")	 			//窄带EQ
#define AUDIO_DMS_PHASECOMPEN_COEFF_FILE 	(FLASH_RES_PATH"dms_phase.bin") 		//双麦相位
#define AUDIO_ANC_ON_MIC_PARAM_FILE 		(FLASH_RES_PATH"fb_ancon.bin")  		//fb->talk (anc on)
#define AUDIO_ANC_OFF_MIC_PARAM_FILE 		(FLASH_RES_PATH"fb_ancoff.bin")  		//fb->talk (anc on)
#define AUDIO_TRI_PHASECOMPEN_COEFF_FILE	(FLASH_RES_PATH"tri_phase.bin")  		//三麦相位

enum cvp_coeff_type {
    WB_EQ,
    NB_EQ,
    FB_ANC_ON,
    FB_ANC_OFF,
    /* DMS_PHASE,
    TMD_PHASE, */
};
char *cvp_coeff_name[] = {"WB_EQ", "NB_EQ", "FB_ANC_ON", "FB_ANC_OFF"};
float *cvp_coeff_file_parse(enum cvp_coeff_type type)
{
    if (cvp_v3 == NULL) {
        return NULL;
    }
    int coeff_offset;
    RESFILE *fp = NULL;
    printf("cvp_coeff_file_parse:%s", cvp_coeff_name[type]);
    switch (type) {
    case WB_EQ:
        fp = resfile_open(AUDIO_DMS_WB_COEFF_FILE);
        coeff_offset = 259;
        break;
    case NB_EQ:
        fp = resfile_open(AUDIO_DMS_NB_COEFF_FILE);
        coeff_offset = 259;
        break;
    case FB_ANC_ON:
        fp = resfile_open(AUDIO_ANC_ON_MIC_PARAM_FILE);
        coeff_offset = 2;
        break;
    case FB_ANC_OFF:
        fp = resfile_open(AUDIO_ANC_OFF_MIC_PARAM_FILE);
        coeff_offset = 2;
        break;
    }
    if (!fp) {
        printf("[error]open cvp coeff file fail");
        return NULL;
    }

    float *tmp_coeff_file = NULL;
    u32 tmp_coeff_len = resfile_get_len(fp);
    printf("cvp coeff file len:%d", tmp_coeff_len);
    if (tmp_coeff_len) {
        tmp_coeff_file = zalloc(tmp_coeff_len);
        if (tmp_coeff_file == NULL) {
            resfile_close(fp);
            return NULL;
        }
    }
    /* resfile_seek(fp, ptr, RESFILE_SEEK_SET); */
    int rlen = resfile_read(fp, tmp_coeff_file, tmp_coeff_len);
    if (rlen != tmp_coeff_len) {
        printf("[err]read cvp coeff file fail,coeff_len:%d,rlen:%d", tmp_coeff_len, rlen);
        if (tmp_coeff_file) {
            free(tmp_coeff_file);
        }
    }
    resfile_close(fp);
    //3mic fb->talk bin文件 偏移 259 float points dB转幅值
    float *coeff_file_offset = (float *)tmp_coeff_file + coeff_offset;
    int coeff_len = 257;
    float *coeff_file = (float *)zalloc(coeff_len * sizeof(float));
    for (int i = 0; i < coeff_len; i++) {
        coeff_file[i] = eq_db2mag(coeff_file_offset[i]);
    }
    free(tmp_coeff_file);
    return coeff_file;
}

/*
*********************************************************************
*                  Audio CVP Parameters
* Description: CVP模块配置参数
* Arguments  : p	参数指针
* Return	 : None.
* Note(s)    : 读取配置文件成功，则使用配置文件的参数配置，否则使用默
*			   认参数配置
*********************************************************************
*/
__CVP_BANK_CODE
static void audio_cvp_param_init(struct cvp_attr *p, u16 node_uuid)
{
    JLSP_params_v3_cfg *cvp_cfg = &p->cvp_cfg;
    cvp_cfg->mic_cfg        	= mic_init_cfg;
    cvp_cfg->aec1_cfg       	= aec_cfg_default;
    cvp_cfg->aec2_cfg       	= aec_cfg_default;
    cvp_cfg->aec3_cfg       	= aec_fb_cfg_default;
    cvp_cfg->nlp1_cfg       	= nlp_cfg_default;
    cvp_cfg->nlp2_cfg       	= nlp_cfg_default;
    cvp_cfg->nlp3_cfg       	= nlp_fb_cfg_default;
    cvp_cfg->wind_cfg       	= wn_init_cfg;
    cvp_cfg->bf_cfg         	= bf_init_cfg;
    cvp_cfg->fusion_cfg     	= fusion_init_cfg;
    cvp_cfg->drc_cfg        	= drc_init_cfg;
    cvp_cfg->micSel_cfg     	= micsel_init_cfg;
    cvp_cfg->single_cfg     	= single_init_cfg;
    cvp_cfg->dual_cfg       	= dual_bf_init_cfg;
    cvp_cfg->tri_cfg        	= tri_init_cfg;
    cvp_cfg->hybrid_cfg 	  	= hybrid_init_cfg;
    cvp_cfg->single_aecnlp_cfg 	= aecnlp_init_cfg;

    //读取工具配置参数+预处理参数
    CVP_CONFIG cfg;
    int ret = cvp_v3_node_param_cfg_read(&cfg, 0, node_uuid);
#if TCFG_AEC_TOOL_ONLINE_ENABLE
    //APP在线调试，APP参数覆盖工具配置参数(不覆盖预处理参数)
    ret = aec_cfg_online_update_fill(&cfg, sizeof(CVP_CONFIG));
#endif/*TCFG_AEC_TOOL_ONLINE_ENABLE*/
    cvp_v3->algo_type = cvp_get_algo_type();
    log_info("CVP-V3 ALGO_TYPE = %d\n", cvp_v3->algo_type);
    if (ret == sizeof(CVP_CONFIG)) {
        log_info("CVP_V3 read cfg ok\n");
        p->EnableBit = cfg.enable_module;
        p->ul_eq_en = cfg.ul_eq_en;
        p->output_sel = cfg.output_sel;
        p->adc_ref_en = cfg.adc_ref_en;
        //aecnlp流程无流程补偿
        if (!(cvp_v3->algo_type & CVP_ALGO_1MIC_AECNLP)) {
            p->CompenDb = cfg.CompenDb;
        }
        // aec
        cvp_cfg->aec1_cfg.aecProcessMaxFrequency = cfg.aec_process_maxfrequency;
        cvp_cfg->aec1_cfg.aecProcessMinFrequency = cfg.aec_process_minfrequency;
        cvp_cfg->aec2_cfg.aecProcessMaxFrequency = cfg.aec_process_maxfrequency;
        cvp_cfg->aec2_cfg.aecProcessMinFrequency = cfg.aec_process_minfrequency;
        //nlp
        cvp_cfg->nlp1_cfg.nlpProcessMaxFrequency = cfg.nlp_process_maxfrequency;
        cvp_cfg->nlp1_cfg.nlpProcessMinFrequency = cfg.nlp_process_minfrequency;
        cvp_cfg->nlp1_cfg.overDrive = cfg.overdrive;
        cvp_cfg->nlp2_cfg.nlpProcessMaxFrequency = cfg.nlp_process_maxfrequency;
        cvp_cfg->nlp2_cfg.nlpProcessMinFrequency = cfg.nlp_process_minfrequency;
        cvp_cfg->nlp2_cfg.overDrive = cfg.overdrive;
        if (!(cvp_v3->algo_type & CVP_ALGO_1MIC_AECNLP)) {
            //dns
            cvp_cfg->single_cfg.aggressFactor = cfg.aggressfactor;
            cvp_cfg->single_cfg.minSupress = cfg.minsuppress;
            cvp_cfg->dual_cfg.aggressFactor = cfg.aggressfactor;
            cvp_cfg->dual_cfg.minSupress = cfg.minsuppress;
            cvp_cfg->tri_cfg.aggressFactor = cfg.aggressfactor;
            cvp_cfg->tri_cfg.minSupress = cfg.minsuppress;
            // drc
            cvp_cfg->drc_cfg.noiseGateThresholdDb = cfg.noisegatethresholdDb;
            cvp_cfg->drc_cfg.makeUpGain = cfg.makeupGain;
            cvp_cfg->drc_cfg.kneeThresholdDb = cfg. kneethresholdDb;
        }
#if (TCFG_CVP_ALGO_TYPE > 0xF)//2mic/3mic
        if ((cvp_v3->algo_type & CVP_TYPE_2MIC) || (cvp_v3->algo_type & CVP_ALGO_3MIC)) {
            //enc
            cvp_cfg->bf_cfg.encProcessMaxFrequency = cfg.enc_process_maxfreq;
            cvp_cfg->bf_cfg.encProcessMinFrequency = cfg.enc_process_minfreq;
            cvp_cfg->bf_cfg.micDistance = cfg.mic_distance;
            cvp_cfg->bf_cfg.sirMaxFreq = cfg.sir_maxfreq;
            cvp_cfg->bf_cfg.targetSignalDegradation = cfg.target_signal_degradation;
            cvp_cfg->bf_cfg.aggressfactor = cfg.enc_aggressfactor;
            cvp_cfg->bf_cfg.minsuppress = cfg.enc_minsuppress;
        }
        //双麦三麦有wnc mfdt
        if ((cvp_v3->algo_type & CVP_TYPE_2MIC) || (cvp_v3->algo_type & CVP_ALGO_3MIC)) {
            // wnc
            cvp_cfg->wind_cfg.windProbHighTh = cfg.windProbHighTh;
            cvp_cfg->wind_cfg.windProbLowTh = cfg.windProbLowTh;
            cvp_cfg->wind_cfg.windEngDbTh = cfg.windEngDbTh;
            //mfdt
            cvp_cfg->micSel_cfg.detectTime = cfg.detect_time;            // in second
            /*0~-90 dB 两个mic能量差异持续大于此阈值超过检测时间则会检测为故障*/
            cvp_cfg->micSel_cfg.detectEngDiffTh = cfg.detect_eng_diff_thr;     //  dB
            /*0~-90 dB 当处于故障状态时，正常的mic能量大于此阈值才会检测能量差异，避免安静环境下误判切回正常状态*/
            cvp_cfg->micSel_cfg.detectEngLowerBound = cfg.detect_eng_lowerbound; // 0~-90 dB start detect when mic energy lower than this
            cvp_cfg->micSel_cfg.detMaxFrequency = cfg.MalfuncDet_MaxFrequency;  //检测频率上限
            cvp_cfg->micSel_cfg.detMinFrequency = cfg.MalfuncDet_MinFrequency;   //检测频率下限
            cvp_cfg->micSel_cfg.OnlyDetect = cfg.OnlyDetect;// 0 -> 故障切换到单mic模式， 1-> 只检测不切换
        }
#endif
        //flow
        cvp_cfg->single_cfg.preGainDb = cfg.preGainDb;
        cvp_cfg->dual_cfg.dualPreGainDb = cfg.preGainDb;
        cvp_cfg->tri_cfg.triPreGainDb = cfg.preGainDb;
    } else {
        log_error("CVP-V3 read cfg error,use default param\n");
        p->EnableBit = AEC_EN | NLP_EN; //读取cfg配置文件失败，默认使能AEC和NLP避免选择当前模式时传EnableBit错误
        p->ul_eq_en = 1;
        p->output_sel = DMS_OUTPUT_SEL_DEFAULT;
        if (!(cvp_v3->algo_type & CVP_ALGO_1MIC_AECNLP)) {
            p->CompenDb = 0.f;
        }
    }

    if (!(cvp_v3->algo_type & CVP_ALGO_1MIC_AECNLP)) {
        cvp_cfg->single_cfg.singleWbEq  = cvp_coeff_file_parse(WB_EQ);
        cvp_cfg->single_cfg.singleNbEq  = cvp_coeff_file_parse(NB_EQ);
        cvp_cfg->dual_cfg.dualWbEqVec   = cvp_cfg->single_cfg.singleWbEq;
        cvp_cfg->dual_cfg.dualNbEqVec   = cvp_cfg->single_cfg.singleNbEq;
        cvp_cfg->tri_cfg.triWbEqVec  	= cvp_cfg->single_cfg.singleWbEq;
        cvp_cfg->tri_cfg.triNbEqVec  	= cvp_cfg->single_cfg.singleNbEq;
    }
    //三麦phase参数
    if (cvp_v3->algo_type & CVP_ALGO_3MIC) {
        cvp_cfg->tri_cfg.triFbTransferFuncOn  = cvp_coeff_file_parse(FB_ANC_ON);
        cvp_cfg->tri_cfg.triFbTransferFuncOff = cvp_coeff_file_parse(FB_ANC_OFF);
        cvp_cfg->nlp1_cfg.preEnhance = 1;
        cvp_cfg->nlp2_cfg.preEnhance = 1;
    }

    p->algo_type = cvp_v3->algo_type;
    log_info("CVP_V3:AEC[%d] NLP[%d] NS[%d] ENC[%d] DRC[%d] MFDT[%d] WNC[%d]", !!(p->EnableBit & AEC_EN), !!(p->EnableBit & NLP_EN), !!(p->EnableBit & ANS_EN), !!(p->EnableBit & ENC_EN), !!(p->EnableBit & AGC_EN), !!(p->EnableBit & MFDT_EN), !!(p->EnableBit & WNC_EN));

#if TCFG_AUDIO_MIC_ARRAY_TRIM_ENABLE
    app_var.enc_degradation = p->target_signal_degradation;
#endif
}


/*
*********************************************************************
*                  Audio CVP Open
* Description: 初始化CVP模块
* Arguments  : sr 			采样率(8000/16000)
*			   enablebit	使能模块(AEC/NLP/AGC/ANS...)
*			   out_hdl		自定义回调函数，NULL则用默认的回调
* Return	 : 0 成功 其他 失败
* Note(s)    : 该接口是对audio_aec_init的扩展，支持自定义使能模块以及
*			   数据输出回调函数
*********************************************************************
*/
__CVP_BANK_CODE
int audio_aec_open(struct audio_aec_init_param_t *init_param, s16 enablebit, int (*out_hdl)(s16 *data, u16 len))
{
    u32 mic_sr = init_param->sample_rate;
    u32 ref_sr = init_param->ref_sr;
    u8 ref_channel = init_param->ref_channel;
    struct cvp_attr *cvp_param;
    printf("audio_cvp_v3_open\n");
    mem_stats();
    cvp_v3 = zalloc(sizeof(struct cvp_hdl));
    if (cvp_v3 == NULL) {
        log_error("cvp_v3 malloc failed");
        return -ENOMEM;
    }

    overlay_load_code(OVERLAY_AEC);
    aec_code_movable_load();

    /*初始化dac read的资源*/
    audio_dac_read_init();

    cvp_v3->dump_packet = AEC_OUT_DUMP_PACKET;
    cvp_v3->inbuf_clear_cnt = AEC_IN_DUMP_PACKET;
    cvp_v3->output_fade_in = 1;
    cvp_v3->output_fade_in_gain = 0;
    cvp_param = &cvp_v3->attr;
    cvp_param->cvp_probe = audio_cvp_probe;
    cvp_param->cvp_post = audio_cvp_post;
    cvp_param->cvp_output = audio_cvp_output;
    if (ref_sr) {
        cvp_param->ref_sr  = ref_sr;
    } else {
        cvp_param->ref_sr  = usb_mic_is_running();
    }
    if (cvp_param->ref_sr == 0) {
        if (TCFG_ESCO_DL_CVSD_SR_USE_16K && (mic_sr == 8000)) {
            cvp_param->ref_sr = 16000;	//CVSD 下行为16K
        } else {
            cvp_param->ref_sr = mic_sr;
        }
    }

    if (ref_channel != 2) {
        ref_channel = 1;
    }
    cvp_param->ref_channel = ref_channel;

    audio_cvp_param_init(cvp_param, init_param->node_uuid);

    if (enablebit >= 0) {
        cvp_param->EnableBit = enablebit;
    }
    if (out_hdl) {
        cvp_param->cvp_output = out_hdl;
    }
    cvp_param->output_way = global_output_way;

    if (cvp_param->output_way == 0 && cvp_param->adc_ref_en == 0) {
        /*内部读取DAC数据做参考数据才需要做24bit转16bit*/
        extern struct dac_platform_data dac_data;
        cvp_param->ref_bit_width = dac_data.bit_width;
    } else {
        cvp_param->ref_bit_width = DATA_BIT_WIDE_16BIT;
    }

#if TCFG_AEC_SIMPLEX
    cvp_param->wn_en = 1;
    cvp_param->EnableBit = AEC_MODE_SIMPLEX;
    if (sr == 8000) {
        cvp_param->SimplexTail = cvp_param->SimplexTail / 2;
    }
#else
    cvp_param->wn_en = 0;
#endif/*TCFG_AEC_SIMPLEX*/

    cvp_param->mic_sr = mic_sr;
    if (mic_sr == 16000) { //WideBand宽带
        cvp_param->hw_delay_offset = 60;
    } else {//NarrowBand窄带
        cvp_param->hw_delay_offset = 55;
    }

    /*获取当前跑的esco type*/
    int type = lmp_private_get_esco_packet_type();
    int frame_time = (lmp_private_get_esco_packet_type() >> 8) & 0xff;
    int media_type = type & 0xff;
    if (media_type == 0) {
        cvp_param->bandwidth = CVP_NB_EN;
    } else  {
        cvp_param->bandwidth = CVP_WB_EN;
    }

#if TCFG_SUPPORT_MIC_CAPLESS
    if (audio_adc_file_get_mic_mode(0) == AUDIO_MIC_CAPLESS_MODE) {
        cvp_v3->dcc_hdl = audio_dc_offset_remove_open(mic_sr, 1);
    }
#endif

    //cvp_param_dump(cvp_param);
    cvp_v3->EnableBit = cvp_param->EnableBit;
    cvp_param->aptfilt_only = 0;
#if (((defined TCFG_KWS_VOICE_RECOGNITION_ENABLE) && TCFG_KWS_VOICE_RECOGNITION_ENABLE) || \
     ((defined TCFG_CALL_KWS_SWITCH_ENABLE) && TCFG_CALL_KWS_SWITCH_ENABLE))
    extern u8 kws_get_state(void);
    if (kws_get_state()) {
        cvp_param->EnableBit = AEC_EN;
        cvp_param->aptfilt_only = 1;
        printf("kws open,aec_enablebit=%x", cvp_param->EnableBit);
        //临时关闭aec, 对比测试
        //cvp_param->toggle = 0;
    }
#endif/*TCFG_KWS_VOICE_RECOGNITION_ENABLE*/

#if WIND_DETECT_INFO_SPP_DEBUG_ENABLE
    if (cvp_v3->attr.EnableBit & WNC_EN) {
        spp_get_operation_table(&cvp_v3->spp_opt);
    }
#endif

    printf("cvp_v3_open:%x\n", init_param->node_uuid);
#if CVP_TOGGLE
    int ret = cvp_init(cvp_param);
    ASSERT(ret == 0, "cvp_v3 open err %d!!", ret);
#endif
    cvp_v3->start = 1;
    mem_stats();
    printf("audio_cvp_v3_open succ\n");
    return 0;
}

/*
*********************************************************************
*                  Audio AEC Init
* Description: 初始化AEC模块
* Arguments  : sample_rate 采样率(8000/16000)
*              ref_sr 参考采样率
* Return	 : 0 成功 其他 失败
* Note(s)    : None.
*********************************************************************
*/
__CVP_BANK_CODE
int audio_aec_init(struct audio_aec_init_param_t *init_param)
{
    return audio_aec_open(init_param, -1, NULL);
}

/*
*********************************************************************
*                  Audio AEC Reboot
* Description: AEC模块复位接口
* Arguments  : reduce 复位/恢复标志
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_aec_reboot(u8 reduce)
{
    if (cvp_v3) {
        printf("audio_aec_dms_reboot:%x,%x,start:%d", cvp_v3->EnableBit, cvp_v3->attr.EnableBit, cvp_v3->start);
        if (cvp_v3->start) {
            if (reduce) {
                cvp_v3->attr.EnableBit = AEC_EN;
                cvp_v3->attr.aptfilt_only = 1;
                cvp_reboot(cvp_v3->attr.EnableBit);
            } else {
                if (cvp_v3->EnableBit != cvp_v3->attr.EnableBit) {
                    cvp_v3->attr.aptfilt_only = 0;
                    cvp_reboot(cvp_v3->EnableBit);
                }
            }
        }
    } else {
        printf("audio_aec close now\n");
    }
}

/*
*********************************************************************
*                  Audio AEC Output Select
* Description: 输出选择
* Arguments  : sel = DMS_OUTPUT_SEL_DEFAULT 默认输出算法处理结果
*				   = DMS_OUTPUT_SEL_MASTER	输出主mic(通话mic)的原始数据
*				   = DMS_OUTPUT_SEL_SLAVE	输出副mic(降噪mic)的原始数据
*			   agc 输出数据要不要经过agc自动增益控制模块
* Return	 : None.
* Note(s)    : 可以通过选择不同的输出，来测试mic的频响和ENC指标
*********************************************************************
*/
void audio_aec_output_sel(CVP_OUTPUT_ENUM sel, u8 agc)
{
    if (cvp_v3)	{
        printf("cvp_output_sel:%d\n", sel);
        if (agc) {
            cvp_v3->attr.EnableBit |= AGC_EN;
        } else {
            cvp_v3->attr.EnableBit &= ~AGC_EN;
        }
        cvp_v3->attr.output_sel = sel;
    }
}

/*
*********************************************************************
*                  Audio AEC Close
* Description: 关闭AEC模块
* Arguments  : None.
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
__CVP_BANK_CODE
void audio_aec_close(void)
{
    printf("audio_aec_close:%x", (u32)cvp_v3);
    if (cvp_v3) {
        cvp_v3->start = 0;

#if CVP_TOGGLE
        //cppcheck-suppress knownConditionTrueFalse
        cvp_exit();
#if ((defined TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN) && TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN && \
        TCFG_AUDIO_ANC_ENABLE && TCFG_AUDIO_ANC_WIND_NOISE_DET_ENABLE)
        audio_cvp_icsd_wind_det_close();
#endif
#endif /*CVP_TOGGLE*/

        /*释放dac read的资源*/
        audio_dac_read_exit();

#if TCFG_SUPPORT_MIC_CAPLESS
        if (cvp_v3->dcc_hdl) {
            audio_dc_offset_remove_close(cvp_v3->dcc_hdl);
            cvp_v3->dcc_hdl = NULL;
        }
#endif

        local_irq_disable();
        free(cvp_v3);
        cvp_v3 = NULL;
        local_irq_enable();

        aec_code_movable_unload();
    }
}

/*
*********************************************************************
*                  Audio AEC Status
* Description: AEC模块当前状态
* Arguments  : None.
* Return	 : 0 关闭 其他 打开
* Note(s)    : None.
*********************************************************************
*/
u8 audio_aec_status(void)
{
    if (cvp_v3) {
        return cvp_v3->start;
    }
    return 0;
}

/*
*********************************************************************
*                  Audio AEC Input
* Description: AEC源数据输入
* Arguments  : buf	输入源数据地址
*			   len	输入源数据长度(Byte)
* Return	 : None.
* Note(s)    : 输入一帧数据，唤醒一次运行任务处理数据，默认帧长256点
*********************************************************************
*/
void audio_aec_inbuf(s16 *buf, u16 len)
{
    if (len != 512) {
        printf("[error] aec point fault\n"); //aec一帧长度需要256 points,需修改文件(esco_recorder.c/pc_mic_recorder.c)的ADC中断点数

    }
    if (cvp_v3 && cvp_v3->start) {
        if (cvp_v3->input_clear) {
            memset(buf, 0, len);
        }
#if CVP_TOGGLE
        int ret = cvp_fill_in_data(buf, len);
        if (ret == -1) {
        } else if (ret == -2) {
            log_error("aec inbuf full\n");
        }
#else
        cvp_v3->attr.output_handle(buf, len);
#endif/*CVP_TOGGLE*/
    }
}

/*
*********************************************************************
*                  Audio AEC Input Reference
* Description: AEC源参考数据输入
* Arguments  : buf	输入源数据地址
*			   len	输入源数据长度
* Return	 : None.
* Note(s)    : 双mic ENC的参考mic数据输入,单mic的无须调用该接口
*********************************************************************
*/
void audio_aec_inbuf_ref(s16 *buf, u16 len)
{
    if (cvp_v3 && cvp_v3->start) {
        cvp_fill_in_ref_data(buf, len);
    }
}

void audio_aec_inbuf_ref_1(s16 *buf, u16 len)
{
    if (cvp_v3 && cvp_v3->start) {
        cvp_fill_in_ref_1_data(buf, len);
    }
}
/*
*********************************************************************
*                  Audio AEC Reference
* Description: AEC模块参考数据输入
* Arguments  : buf	输入参考数据地址
*			   len	输入参考数据长度
* Return	 : None.
* Note(s)    : 声卡设备是DAC，默认不用外部提供参考数据
*********************************************************************
*/
void audio_aec_refbuf(s16 *data0, s16 *data1, u16 len)
{
    if (cvp_v3 && cvp_v3->start) {
#if CVP_TOGGLE
        cvp_fill_ref_data(data0, data1, len);
#endif/*CVP_TOGGLE*/
    }
}


/*
 * 获取风噪检测信息
 * wd_flag: 0 没有风噪，1 有风噪
 * 风噪强度(dB)
 * */
int audio_cvp_v3_get_wind_detect_info(int *wd_flag, float *wd_val)
{
    if (cvp_v3 && cvp_v3->start) {
        return jlsp_cvp_v3_get_wind_detect_info(wd_flag, wd_val);
    }
    return -1;
}


/*
 * 获取宽带窄带信息
 * is_wb_state: 0 窄带 1 宽带
 * */
int audio_cvp_v3_get_bandwidth_info(int is_wb_state)
{
    if (cvp_v3 && cvp_v3->start) {
        return jlsp_cvp_v3_get_bandwidth_info(is_wb_state);
    }
    return -1;
}

/*
 * 麦克风状态定义：
 * 0: 正常使用双麦做Beamforming
 * 1: 正常工作（Talk 模式）
 * 2: 前馈麦克风工作（FF 模式）
 */

int audio_cvp_v3_get_mic_state_info(int mic_state)
{
    if (cvp_v3 && cvp_v3->start) {
        return jlsp_cvp_v3_get_mic_state_info(mic_state);
    }
    return -1;
}

/*
*********************************************************************
*                  			Audio CVP IOCTL
* Description: CVP功能配置
* Arguments  : cmd 		操作命令
*		       value 	操作数
*		       priv 	操作内存地址
* Return	 : 0 成功 其他 失败
* Note(s)    : (1)比如动态开关降噪NS模块:
*				audio_cvp_ioctl(CVP_NS_SWITCH,1,NULL);	//降噪关
*				audio_cvp_ioctl(CVP_NS_SWITCH,0,NULL);  //降噪开
*********************************************************************
*/
int audio_cvp_ioctl(int cmd, int value, void *priv)
{
    if (cvp_v3 && cvp_v3->start) {
        return cvp_ioctl(cmd, value, priv);
    } else {
        return -1;
    }

}

/*
*********************************************************************
*                  Audio CVP Toggle Set
* Description: CVP模块算法开关使能
* Arguments  : toggle 0 关闭算法 1 打开算法
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
int audio_cvp_toggle_set(u8 toggle)
{
    if (cvp_v3) {
        cvp_toggle(toggle);
        return 0;
    }
    return 1;
}

/**
 * 以下为用户层扩展接口
 */
//pbg profile use it,don't delete
void aec_input_clear_enable(u8 enable)
{
    if (cvp_v3) {
        cvp_v3->input_clear = enable;
        log_info("aec_input_clear_enable= %d\n", enable);
    }
}
#endif/*TCFG_AUDIO_CVP_V3_ENABLE == 1*/
#endif /*TCFG_CVP_DEVELOP_ENABLE*/

