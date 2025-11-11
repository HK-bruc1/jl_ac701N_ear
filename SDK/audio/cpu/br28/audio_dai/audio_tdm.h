#ifndef _AUDIO_TDM_H_
#define _AUDIO_TDM_H_

#include "generic/typedef.h"
#include "circular_buf.h"
#include "audio_general.h"
//================================================================//
//                该文件描述TDM模块配置和接口                     //
//================================================================//
//----------------------------------------------------------------//
/*
   TDM 硬件连接TOPO
    ______                 ______
   |      |  mclk(可选)   |      |
   |      |-------------> |      |
   |      |    bclk       |      |
   |      |-------------> |      |
   |      |    wclk       |      |
   |      |-------------> |      |
   |      |    data       |      |
   |      |<------------- |      |
   |______|               |______|
  BR28(master)          Sensor(slave)

 */
#define TDM_SAMPLE_RATE TDM_SR_16000
#define TDM_IRQ_POINTS (256)
#define TDM_SLOT_NUM 4
//位宽配置
//要和数据流一致
#define TDM_32_BIT_EN (audio_general_out_dev_bit_width())
#define TDM_DMA_LEN (TDM_IRQ_POINTS * 2 * TDM_SLOT_NUM *(TDM_32_BIT_EN ? 4 : 2))

#define TDM_CON_RESET() 				do {JL_TDM->CON0 = 0; JL_TDM->SLOT_EN = 0; JL_TDM->WCLK_DUTY = 0; JL_TDM->WCLK_PRD = 0; JL_TDM->DMA_BASE_ADR = 0; JL_TDM->DMA_BUF_LEN = 0;} while(0)

#define TDM_PND0_CLR()					(JL_TDM->CON0 |= BIT(28))
#define TDM_IS_PNDING0()				(JL_TDM->CON0 & BIT(30))
#define TDM_PND1_CLR()					(JL_TDM->CON0 |= BIT(29))
#define TDM_IS_PNDING1()				(JL_TDM->CON0 & BIT(31))

#define TDM_RX_KICK_START() 			(JL_TDM->CON0 |= BIT(27)) //输出时钟, 启动数据接收
#define TDM_STATUS_RST() 				(JL_TDM->CON0 |= BIT(26)) //输出时钟, 启动数据接收
#define TDM_RX_TIMING_OPT(x)   			SFR(JL_TDM->CON0, 24, 1, x) //
#define TDM_TX_DLY_OPT(x)   			SFR(JL_TDM->CON0, 23, 1, x) //
#define TDM_RX_DLY_OPT(x)   			SFR(JL_TDM->CON0, 22, 1, x) //
#define TDM_RX_RISING_OPT(x)   			SFR(JL_TDM->CON0, 21, 1, x) //
#define TDM_TX_TIMING_OPT(x)   			SFR(JL_TDM->CON0, 20, 1, x) //
#define TDM_BCLK_MODE(x)   				SFR(JL_TDM->CON0, 19, 1, x) //主机模式下, 1：写RX_KST后, BCLK和WCLK同时输出; 0: BCLK在配置好时钟后输出，WCLK在写RX_KST后输出;

#define TDM_IE(x)			   			SFR(JL_TDM->CON0, 18, 1, x)

#define TDM_DLY_MODE_ENABLE(x) 			SFR(JL_TDM->CON0, 17, 1, x) //是否延迟一个bclk接收数据
#define TDM_DATA_CLK_MODE_SEL(x) 		SFR(JL_TDM->CON0, 15, 2, x) //采样(收)边沿选择：1: 下降沿采样; 0: 上升沿采样

#define TDM_WCLK_INV(x) 				SFR(JL_TDM->CON0, 14, 1, x) //WCLK取反, 1: 取反(IIS); 0: 不取反(left/right)
#define TDM_SLOT_MODE_SEL(x) 			SFR(JL_TDM->CON0, 11, 3, x) //每个slot data位数
#define TDM_HOST_MODE_SEL(x)			SFR(JL_TDM->CON0, 10, 1, x) //1: 主机模式, 0: 从机模式

//========================================================================================================//
/*
NOTE:
	1) WCLK_DUTY: WCLK高电平长度是所配置值个bclk周期长度;
	2) WCLK_PRD: WCLK周期是所配置值个bclk周期;

 */

/*
-----------------------------------------------------------------------------------------------------------
tdm输出时钟计算:
1.外接tdm传感器, 已知:
	1) 传输频率(采样率) SR: 48000, 44100 ...
	2) 数据位宽 DATA_WIDE: 8/16/20/24/32bit
	3) slot个数
2.计算实例: 外接支持iis协议的dsp芯片, 传输协议:
	1)采样率SR 16000
	2)nodelay
	3)数据位宽16bit
	4)x y z 轴 < -- > 3slot

采样率确定:
	1)Fwclk = SR = 16000Hz
位宽确定:
	2)Fbclk = Fwclk x DATA_WIDE x slot_num = 16000 x 16 x 3 = 768000Hz
采样率48k系列tdm外部输出源选择tdm_clk:
	3)根据音频领域规范:
	a) 48k系列, Fmclk需要得到12.288MHz, 320M div 26
	b) 44.1k系列, Fmclk需要得到11.2896MHz, 192M div 17
	4)tdm_clk --> mclk不分频, 确定tdm_clk前置分频为 13, 得到tdm_clk = Fmclk 约= 12.288MHz
	5)确定mclk->bclk分频, divn = Fmclk / Fbclk = 12.288MHz / 768000Hz = 16;
	6)实际得到Fblck = 768000Hz, Fwclk =  Fblck / DATA_WIDE / slot_num = 16025Hz, 由于存在小数分频导致误差

3.WCLK 的周期和占空比确定
	1) WCLK_DUTY: WCLK高电平长度是所配置值个bclk周期长度, WCLK_DUTY根据协议要求配置, 典型应用IIS要求配置为50%占空比, WCLK_DUTY = WCLK_PRD / 2;
	2) WCLK_PRD: WCLK周期是所配置值个bclk周期, WCLK_PRD = DATA_WIDE x slot_num;

 */
//========================================================================================================//
#define TDM_MCLK_DIVn_SEL(x)			SFR(JL_TDM->CON0, 9, 1, x) //1: tdm_clk div 2, 0: tdm_clk
#define TDM_BCLK_DIV(x)					SFR(JL_TDM->CON0, 1, 8, ((x) - 1)) //0~xxx: (1 ~ xxx + 1) div
#define TDM_MODULE_EN(x) 				SFR(JL_TDM->CON0, 0, 1, x) //TDM模块使能

#define TDM_WCLK_PRD(x)					(JL_TDM->WCLK_PRD = ((x) - 1)) //PRD个BCLK
#define TDM_WCLK_DUTY(x)				(JL_TDM->WCLK_DUTY = x) //DUTY个BCLK

#define TDM_DMA_BUF_LEN_SET(x)			(JL_TDM->DMA_BUF_LEN = x)
#define TDM_DMA_BUF_LEN_GET()			(JL_TDM->DMA_BUF_LEN) //该寄存器不可读，读出为deaddead

typedef enum {
    TDM_ROLE_SLAVE,  //从机发送数据
    TDM_ROLE_MASTER, //主机接收数据
} TDM_ROLE;

//bclk时钟延时选择
typedef enum {
    TDM_NO_DELAY_MODE,  	 //不延迟一个bclk, 类似左对齐模式
    TDM_DELAY_ONE_BCLK_MODE, //延迟一个bclk, 类似iis模式
} TDM_DELAY_MODE;

//时钟采样边沿选择
typedef enum {
    TDM_BCLK_RAISE_UPDATE_RAISE_SAMPLE, //上升沿更新数据, 上升沿采样数据
    TDM_BCLK_FALL_UPDATE_RAISE_SAMPLE, //下降沿更新数据, 上升沿采样数据
    TDM_BCLK_RAISE_UPDATE_FALL_SAMPLE, //上升沿更新数据, 下降沿采样数据
    TDM_BCLK_FALL_UPDATE_FALL_SAMPLE, //下降沿更新数据, 下降沿采样数据
} TDM_DATA_CLK_MODE;


//MCLK div2 使能
//NOTE:  有特殊需求才使能, 默认配置为: TDM_MCLK_DIV1
//1)48k采样率系列: 默认mclk = 12.288MHz
//2)44.1k采样率系列: 默认mclk = 11.2896MHz
typedef enum {
    TDM_CLK_DIV1_MCLK,
    TDM_CLK_DIV2_MCLK,
} TDM_MCLK_MODE;

//数据位宽选择
typedef enum {
    TDM_SLOT_MODE_8BIT = 1,
    TDM_SLOT_MODE_16BIT,
    TDM_SLOT_MODE_20BIT,
    TDM_SLOT_MODE_24BIT,
    TDM_SLOT_MODE_32BIT,
} TDM_SLOT_DATA_WIDTH;

//数据采样率选择
typedef enum {
    TDM_SR_48000 = 48000,
    TDM_SR_44100 = 44100,
    TDM_SR_32000 = 32000,
    TDM_SR_24000 = 24000,
    TDM_SR_22050 = 22050,
    TDM_SR_16000 = 16000,
    TDM_SR_12000 = 12000,
    TDM_SR_11025 = 11025,
    TDM_SR_8000  = 8000,
} TDM_SR;

#define TDM_CLK_OUPUT_DISABLE 	0xFF

//================================================//
/*
   关于频率的计算:
   用户已知
   1)数据采样率(sample_rate);
   2)每帧bclk个数(bclk_per_frame);
   3)mclk是否需要div2;
   计算:
   1)Fblck = bclk_per_frame X sample_rate;
   2)Fwclk = sample_rate;

   WCLK占空比可调, 单位是blck宽度(wclk_duty);
 */
//================================================//
typedef struct _TDM_PARM {
    u8 mclk_io; 				//mclk IO输出配置: TDM_CLK_OUPUT_DISABLE不输出该时钟
    u8 bclk_io;					//bclk IO输出配置: TDM_CLK_OUPUT_DISABLE不输出该时钟
    u8 wclk_io;				    //wclk IO输出配置: TDM_CLK_OUPUT_DISABLE不输出该时钟
    u8 data_io;					//lrclk IO输出配置: TDM_CLK_OUPUT_DISABLE不输出该时钟
    TDM_ROLE role; 				//TDM主机接收(需要输出sclk和lrclk), 从机发送(不需要输出时钟)
    TDM_DELAY_MODE delay_mode; 	//是否延迟一个bclk时钟
    TDM_DATA_CLK_MODE update_sample_mode; 	    //更新边沿
    TDM_SLOT_DATA_WIDTH data_width;   //数据位宽8/16/20/16/32bit
    TDM_SR sample_rate;			//采样率
    u16 wclk_duty; 				//wclk高电平宽度是bclk周期长度的配置值, iis delay_mode 需要+1;
    // u16 bclk_per_frame;			//1帧的bclk个数, delay_mode 需要+1;
    u32 slot_num; 				//有效slot个数
    u32 dma_len; 				//buf长度 byte/one buf
    u32 *dma_buf;
    void *ch_buf;                //单个通道一次中断数据的buf
    void (*isr_cb)(void *priv, void *buf, u32 len); //发送或者接收数据回调
    void *private_data;			//音频私有数据
    u8 *tdm_buf;               //tdm数据缓存空间
    cbuffer_t tdm_cbuf;       //tdm数据缓存的cbuffer
    u8 tx_start; //发送端开始发数
    u8 tdm_initd; //tdm的cbuffer等是否已经初始化
    u8 tx_online;//发送是否正常起中断
    u8 irq_flag;
    u16 det_timer_id;
    spinlock_t lock;
    u32 timestamp;
    u32 timestamp_gap;
    u32 timestamp_cnt;
} TDM_PARM;

int tdm_init(TDM_PARM *parm);
int tdm_start(void);
void tdm_close(void);
void audio_tdm_tx_open(void *priv);
void audio_tdm_rx_open(void *priv);
void audio_tdm_close();
int audio_tdm_write(void *buf, int len);
void *audio_tdm_get_hdl();
int tdm_get_tx_ch_points();
u32 tdm_timestamp();

#endif/*_AUDIO_TDM_H_*/
