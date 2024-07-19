#ifndef _ICSD_RT_ANC_LIB_H
#define _ICSD_RT_ANC_LIB_H

#include "icsd_common_v2.h"
#include "math.h"
#include "asm/math_fast_function.h"

#if 1
#define _rt_printf printf               //打开智能免摘库打印信息
#else
extern int rt_printf_off(const char *format, ...);
#define _rt_printf rt_printf_off
#endif
extern int (*rt_printf)(const char *format, ...);


#define RT_DMA_BELONG_TO_SZL    		1
#define RT_DMA_BELONG_TO_SZR    		2
#define RT_DMA_BELONG_TO_PZL    		3
#define RT_DMA_BELONG_TO_PZR    		4
#define RT_DMA_BELONG_TO_TONEL  		5
#define RT_DMA_BELONG_TO_BYPASSL 		6
#define RT_DMA_BELONG_TO_TONER  		7
#define RT_DMA_BELONG_TO_BYPASSR 	    8

#define RT_ANC_TEST_DATA			    0
#define RT_ANC_DSF8_DATA_DEBUG	    	0
#define RT_DSF8_DATA_DEBUG_TYPE			RT_DMA_BELONG_TO_PZL

#define RT_ANC_USER_TRAIN_DMA_LEN 		16384
#define RT_ANC_DMA_DOUBLE_LEN       	(512*4) //(512*8)
#define RT_ANC_DOUBLE_TRAIN_LEN         512
#define RT_ANC_DMA_DOUBLE_CNT       	32

struct rt_anc_function {
    u8(*anc_dma_done_ppflag)();
    void (*anc_dma_on)(u8 out_sel, int *buf, int len);
    void (*anc_dma_on_double)(u8 out_sel, int *buf, int len);
    void (*anc_core_dma_ie)(u8 en);
    void (*anc_core_dma_stop)(void);
};
extern struct rt_anc_function	RT_ANC_FUNC;

enum {
    RT_ANC_CMD_FORCED_EXIT = 0,
    RT_ANC_CMD_GET_DMA_DATA,
    RT_ANC_CMD_EXIT,
    RT_ANC_CMD_UPDATA,
    RT_ANC_CMD_DMA_ON,
    RT_ANC_CMD_SUSPEND,
    /*
    ANC_CMD_TONE_MODE_START,
    ANC_CMD_TONEL_START,
    ANC_CMD_TONER_START,
    ANC_CMD_PZL_START,
    ANC_CMD_PZR_START,
    ANC_CMD_TONE_DACON,
    ANC_CMD_TRAIN_AFTER_TONE,
    ANC_CMD_TARGET_END,
    ANC_CMD_TARGET_HANDLE,
    ANC_CMD_SYNCINF2_FUNCTION,
    ANC_CMD_HALF_ANC_RUN,
    ANC_CMD_MASTER_ANCON_SYNC,
    ANC_CMD_HALF_ANC_ON_PRE,
    ANC_CMD_HALF_TIMEOUT,
    */
};

struct rt_anc_infmt {
    void *alloc_ptr;     //外部申请的ram地址
    u16	tone_delay;      //提示音延长多长时间开始获取sz数据
    u8  type;            //RT_ANC_TWS / RT_ANC_HEADSET
    void *param;
};

struct rt_anc_libfmt {
    //输出参数
    int lib_alloc_size;  //算法ram需求大小
    u8  alogm;           //anc采样率
    //输入参数
    u8  type;            //RT_ANC_TWS / RT_ANC_HEADSET
};

typedef struct {
    u8	ff_yorder;
    u8  fb_yorder;
    float ffgain;
    float fbgain;
    double ff_coeff[50];//max
    double fb_coeff[50];//max
    void *param;
} __rt_anc_param;

struct rt_anc_tws_packet {
    s8  *data;
    u16 len;
};

enum {
    M_CMD_TEST = 0,
    S_CMD_TEST,
};

typedef struct {
    u8  cmd;
    u8  id;
    u8  data;
} __rtanc_sync_cmd;


extern const u8 rt_anc_dma_ch_num;

//RT ANC FUNCTION
// extern void anc_dma_on(u8 out_sel, int *buf, int len);
// extern void anc_dma_on_double(u8 out_sel, int *buf, int len);
// extern u8 anc_dma_done_ppflag();
// extern void anc_core_dma_ie(u8 en);
// extern void anc_core_dma_stop(void);
//RT_ANC调用
extern int os_taskq_post_msg(const char *name, int argc, ...);
//SDK 调用
extern void rt_anc_get_libfmt(struct rt_anc_libfmt *libfmt);
extern void rt_anc_set_infmt(struct rt_anc_infmt *fmt);
extern void rt_anc_dma_done();
extern void rt_anc_anctask_cmd_handle(void *param, u8 cmd);
extern void rt_anc_exit();
//库调用
extern void rt_anc_post_anctask_cmd(u8 cmd);
extern void rt_anc_get_param(void *_param, void *rt_param_l, void *rt_param_r);
extern void rt_anc_updata_param(void *rt_param_l, void *rt_param_r);

extern void rt_anc_fade_out();

extern void rt_anc_fade_to(u16 gain);
extern void rt_anc_fade_to2ch(u16 ffgain, u16 fbgain);

extern void rt_anc_post_rtanctask_cmd(u8 cmd);
extern void rt_anc_master_packet(struct rt_anc_tws_packet *packet, u8 cmd);
extern void rt_anc_slave_packet(struct rt_anc_tws_packet *packet, u8 cmd);

extern void master_func(u8 m_tig, u8 s_tig);
extern void slaver_func(u8 m_tig, u8 s_tig);
extern void icsd_rtanc_m2s_cb(void *_data, u16 len, bool rx);
extern void icsd_rtanc_s2m_cb(void *_data, u16 len, bool rx);
extern void icsd_rtanc_send();
extern void icsd_rtanc_master_send(u8 cmd);
extern void icsd_rtanc_slave_send(u8 cmd);

extern void rt_anc_dma_2ch_on(u8 out_sel, int *buf, int len);
extern void rt_anc_dma_4ch_on(u8 out_sel_ch01, u8 out_sel_ch23, int *buf, int len);
extern void rt_anc_dsf8_data_debug(u8 belong, s16 *dsf_out_ch0, s16 *dsf_out_ch1, s16 *dsf_out_ch2, s16 *dsf_out_ch3);


#endif
