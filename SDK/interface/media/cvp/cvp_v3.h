#ifndef _CVP_V3_H_
#define _CVP_V3_H_

#include "generic/typedef.h"
#include "cvp_common.h"
#include "lib_h/jlsp_v3_ns.h"

//BIT[0:3]
#define CVP_ALGO_1MIC   				BIT(0)
#define CVP_ALGO_1MIC_AECNLP 			BIT(1) // AEC_NLP
//BIT[4:15]
#define CVP_ALGO_2MIC_BF   				BIT(4)
#define CVP_ALGO_2MIC_HYBRID   			BIT(5)
#define CVP_ALGO_2MIC_AWN   			BIT(6)
#define CVP_ALGO_2MIC_FLEXIBLE   		BIT(7)
//BIT[16:23]
#define CVP_ALGO_3MIC   				BIT(16)

#define CVP_TYPE_2MIC					(CVP_ALGO_2MIC_BF | CVP_ALGO_2MIC_HYBRID | CVP_ALGO_2MIC_AWN |CVP_ALGO_2MIC_FLEXIBLE)

#define CVP_V3_1MIC_ALGO_ENABLE			(TCFG_CVP_ALGO_TYPE & 0x00000F)
#define CVP_V3_2MIC_ALGO_ENABLE			(TCFG_CVP_ALGO_TYPE & 0x00FFF0)
#define CVP_V3_3MIC_ALGO_ENABLE			(TCFG_CVP_ALGO_TYPE & 0xFF0000)

/* Beamforming 版本控制*/
#define JLSP_BF_V100					0xA1
#define JLSP_BF_V200					0xA2

typedef struct {
    u8 ver;							//Ver:01
    u8 mic_again;					//MIC增益,default:3(0~14)
    u8 fb_mic_again;				//FB MIC增益,default:3(0~14)
    u8 dac_again;					//DAC增益,default:22(0~31)
    u8 enable_module;       		//使能模块
    u8 ul_eq_en;        			//上行EQ使能,default:enable(disable(0), enable(1))
    /*
    float mic0PreGain;
    float mic1PreGain;
    float mic2PreGain;
    float mic3PreGain;
    u8 mic0En;
    u8 mic1En;
    u8 mic2En;
    u8 mic3En;
    */
    /*aec*/
    int aec_process_maxfrequency;	//default:8000,range[3000:8000]
    int aec_process_minfrequency;	//default:0,range[0:1000]
    /*nlp*/
    int nlp_process_maxfrequency;	//default:8000,range[3000:8000]
    int nlp_process_minfrequency;	//default:0,range[0:1000]
    float overdrive;				//default:1,range[0:30]
    /*enc*/
    int enc_process_maxfreq;		//default:8000,range[3000:8000]
    int enc_process_minfreq;		//default:0,range[0:1000]
    int sir_maxfreq;				//default:3000,range[1000:8000]
    float mic_distance;				//default:0.015,range[0.035:0.015]
    float target_signal_degradation;//default:1,range[0:1]
    float enc_aggressfactor;		//default:4.f,range[0:4]
    float enc_minsuppress;			//default:0.09f,range[0:0.1]
    /*dns*/
    float aggressfactor;			//default:1.25,range[1:2]
    float minsuppress;				//default:0.04,range[0.01:0.1]
    /*drc*/
    float kneethresholdDb;  		//default:-6.0f
    float noisegatethresholdDb; 	//default:-50.f
    float makeupGain;				//default:14.0f
    /*wnc*/
    float windProbHighTh;
    float windProbLowTh;
    float windEngDbTh;
    /*MFDT Parameters*/
    float detect_time;           	// // 检测时间s，影响状态切换的速度
    float detect_eng_diff_thr;   	// 0~-90 dB 两个mic能量差异持续大于此阈值超过检测时间则会检测为故障
    float detect_eng_lowerbound; 	// 0~-90 dB 当处于故障状态时，正常的mic能量大于此阈值才会检测能量差异，避免安静环境下误判切回正常状态
    int MalfuncDet_MaxFrequency;	// 检测信号的最大频率成分
    int MalfuncDet_MinFrequency;	// 检测信号的最小频率成分
    int OnlyDetect;					// 0 -> 故障切换到单mic模式， 1-> 只检测不切换
    /*fusion 不支持在线调试 */
    int fusionFreq;
    float MagTh1;
    float MagTh2;
    /*流程参数配置*/
    float preGainDb;				//ADC前级增益
    float CompenDb;					//流程补偿增益
    /*fb*/
    //float trifbCompenDb;			//三麦fb补偿增益
    /*回采*/
    u16 adc_ref_en;    				/*adc回采参考数据使能*/
    /*debug*/
    u8 output_sel;/*dms output选择: 0:Default, 1:Master Raw, 2:Slave Raw, 3:Fbmic Raw*/
} _GNU_PACKED_ CVP_CONFIG;

struct cvp_attr {
    u8 EnableBit;
    u8 ul_eq_en;
    u8 wn_en;
    u8 output_way;   		/*输出方式配置0:dac  1:fm_tx */
    u8 fm_tx_start;  		/*fm发射同步标志*/
    u8 FB_EnableBit;
    u8 packet_dump;
    u8 output_sel;			/*dms output选择*/
    u8 aptfilt_only;
    u8 bandwidth;
    u8 mic_bit_width; 		/*麦克风数据位宽*/
    u8 ref_bit_width; 		/*参考数据位宽*/
    u16 ref_channel;    	/*参考数据声道数*/
    u16 adc_ref_en;    		/*adc回采参考数据使能*/
    u16 hw_delay_offset;	/*dac hardware delay offset*/
    u16 wn_gain;			/*white_noise gain*/
    u32 mic_sr;				/*麦克风数据采样率*/
    u32 ref_sr;				/*参考数据采样率*/
    u32 algo_type;
    /*流程配置*/
    float CompenDb;					//流程补偿增益
    JLSP_params_v3_cfg cvp_cfg;
    /*data handle*/
    int (*cvp_advanced_options)(void *aec,
                                void *nlp,
                                void *ns,
                                void *enc,
                                void *agc,
                                void *wn,
                                void *mfdt);
    int (*cvp_probe)(short *talk_mic, short *ff_mic, short *fb_mic, short *ref, u16 len);
    int (*cvp_post)(s16 *dat, u16 len);
    int (*cvp_update)(u8 EnableBit);
    int (*cvp_output)(s16 *dat, u16 len);
};

int cvp_init(struct cvp_attr *attr);
int cvp_exit();
int cvp_talk_mic_push(void *dat, u16 len);
int cvp_ff_mic_push(void *dat, u16 len);
int cvp_fb_mic_push(void *dat, u16 len);
int cvp_spk_data_push(void *data0, void *data1, u16 len);
int cvp_cfg_update(CVP_CONFIG *cfg);
int cvp_reboot(u8 enablebit);
u8 get_cvp_rebooting();
void cvp_toggle(u8 toggle);
void audio_cvp_lock();
void audio_cvp_unlock();
int cvp_read_ref_data(void);

/*
 * 获取风噪检测信息：
 * wd_flag - 风噪标志（0: 无风噪，1: 存在风噪）
 * wd_val  - 风噪强度值（dB）
 */
int jlsp_cvp_v3_get_wind_detect_info(int *wd_flag, float *wd_val);
/*
 * 获取当前语音带宽模式：
 * 0: 窄带（Narrowband）
 * 1: 宽带（Wideband）
 */

int jlsp_cvp_v3_get_bandwidth_info(int is_wb_state);

/*
 * 麦克风状态定义：
 * 0: 麦克风故障，自动切换为单麦降噪模式
 * 1: 正常工作（Talk 模式）
 * 2: 前馈麦克风工作（FF 模式）
 */
int jlsp_cvp_v3_get_mic_state_info(int mic_state);

#endif/*_CVP_V3_H_*/
