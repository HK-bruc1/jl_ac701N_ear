
#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".audio_tdm.data.bss")
#pragma data_seg(".audio_tdm.data")
#pragma const_seg(".audio_tdm.text.const")
#pragma code_seg(".audio_tdm.text")
#endif
#include "audio_tdm.h"
#include "tdm_file.h"
#include "includes.h"
#include "audio_config.h"
#include "media/includes.h"

#define TDM_DEBUG_ENABLE 		1
#if TDM_DEBUG_ENABLE
#define tdm_debug 			y_printf
#else
#define tdm_debug(...)
#endif /* #if TDM_DEBUG_ENABLE */

#define TDM_LOG_ENABLE          0
#if TDM_LOG_ENABLE
#define LOG_TAG     "[TDM]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#include "debug.h"
#else
#define log_info(fmt, ...)
#define log_error(fmt, ...)
#define log_debug(fmt, ...)
#endif

static TDM_PARM	*p_tdm_parm;
#define __this	(p_tdm_parm)
#define TDM_TX_OPEN_ALWAYS (1) //tdm发送硬件不关一直打开
#if TCFG_MULTI_CH_TDM_NODE_ENABLE
#define TDM_SOURCE_CH_NUM  (8) //源数据有多少通道
#else
#define TDM_SOURCE_CH_NUM  (1) //源数据有多少通道
#endif
#define TDM_TX_CBUF_LEN 	((TDM_SAMPLE_RATE / 1000 +1) * TDM_SOURCE_CH_NUM * (TDM_32_BIT_EN ? 4 : 2)*50)  //cbuf缓冲50ms
/* TDM TX中断在线检测 */
#define TDM_ONLINE_DET_EN   1
#define TDM_ONLINE_DET_TIME 20

extern void audio_tdm_update_frame(void *hdl, u32 frames);
static void handle_tdm_rx(void *priv, void *buf, u32 len);
static void handle_tdm_tx(void *priv, void *buf, u32 len);

//tmd数据直接写到DAC的demo
#define AUDIO_TDM_DEMO_EN  0
#if AUDIO_TDM_DEMO_EN
extern struct audio_dac_hdl dac_hdl;
static u16 tx_s_cnt = 0;
const s16 sin_48k[48] = {
    0, 2139, 4240, 6270, 8192, 9974, 11585, 12998,
    14189, 15137, 15826, 16244, 16384, 16244, 15826, 15137,
    14189, 12998, 11585, 9974, 8192, 6270, 4240, 2139,
    0, -2139, -4240, -6270, -8192, -9974, -11585, -12998,
    -14189, -15137, -15826, -16244, -16384, -16244, -15826, -15137,
    -14189, -12998, -11585, -9974, -8192, -6270, -4240, -2139
};
static short const tsin_16k[16] = {
    0x0000, 0x30fd, 0x5a83, 0x7641, 0x7fff, 0x7642, 0x5a82, 0x30fc, 0x0000, 0xcf04, 0xa57d, 0x89be, 0x8000, 0x89be, 0xa57e, 0xcf05,
};
static int get_sine_16k_data(u16 *s_cnt, s16 *data, u16 points, u8 ch)
{
    while (points--) {
        if (*s_cnt >= 16) {
            *s_cnt = 0;
        }
        *data++ = tsin_16k[*s_cnt];
        if (ch == 2) {
            *data++ = tsin_16k[*s_cnt];
        }
        (*s_cnt)++;
    }
    return 0;
}
//变采样
const u8 SRC_TYPE = 1;
#include "Resample_api.h"
static RS_STUCT_API *sw_src_api = NULL;
static unsigned int *sw_src_buf = NULL;
#include "asm/audio_src.h"
static struct audio_src_handle *ref_hw_src = NULL;;
static u16 ref_hw_src_len = 0;
int sw_src_init(u8 nch, u16 insample, u16 outsample)
{
    if (1) {
        if (1) {
            sw_src_api = get_rs16_context();
            g_printf("sw_src_api:0x%x\n", sw_src_api);
            ASSERT(sw_src_api);
            u32 sw_src_need_buf = sw_src_api->need_buf();
            g_printf("sw_src_buf:%d\n", sw_src_need_buf);
            sw_src_buf = malloc(sw_src_need_buf);
            ASSERT(sw_src_buf);
            RS_PARA_STRUCT rs_para_obj;
            rs_para_obj.nch = nch;

            rs_para_obj.new_insample = insample;//48000;
            rs_para_obj.new_outsample = outsample;//16000;
            rs_para_obj.dataTypeobj.IndataBit = 0;
            rs_para_obj.dataTypeobj.OutdataBit = 0;
            rs_para_obj.dataTypeobj.IndataInc = (nch == 1) ? 1 : 2;
            rs_para_obj.dataTypeobj.OutdataInc = (nch == 1) ? 1 : 2;
            rs_para_obj.dataTypeobj.Qval = 15;
            printf("sw src,ch = %d, in = %d,out = %d\n", rs_para_obj.nch, rs_para_obj.new_insample, rs_para_obj.new_outsample);
            sw_src_api->open(sw_src_buf, &rs_para_obj);
        }
    }
    return 0;
}
int sw_src_run(s16 *indata, s16 *outdata, u16 len)
{
    ASSERT(indata);
    ASSERT(outdata);
    int outlen = len;
    if (1) {
        if (sw_src_api && sw_src_buf) {
            if (TDM_32_BIT_EN) {
                outlen = sw_src_api->run(sw_src_buf, indata, len >> 2, outdata);
                outlen = outlen << 2;
            } else {
                outlen = sw_src_api->run(sw_src_buf, indata, len >> 1, outdata);
                outlen = outlen << 1;
            }
        } else if (SRC_TYPE == 2) {
            if (ref_hw_src) {
                ref_hw_src_len = 0;;
                outlen = audio_src_resample_write(ref_hw_src, indata, len);
                outlen = ref_hw_src_len;
            }
        }
        return outlen;
    }
}
static void sw_src_exit(void)
{
    if (SRC_TYPE == 1) {
        if (sw_src_buf) {
            free(sw_src_buf);
            sw_src_buf = NULL;
            sw_src_api = NULL;
        }
    } else if (SRC_TYPE == 2) {
        printf("[HW]ref_src_exit\n");
        if (ref_hw_src) {
            audio_hw_src_close(ref_hw_src);
            free(ref_hw_src);
            ref_hw_src = NULL;
        }
    }
}

static int get_txsine_48k_data_32(u16 *s_cnt, s32 *data, u16 points, u8 ch)
{

    while (points--) {
        if (*s_cnt >= 48) {
            *s_cnt = 0;
        }
        *data++ = 0;//(sin_48k[*s_cnt] << 16) / 4;
        if (ch == 2) {
            *data++ = sin_48k[*s_cnt] << 16;
        }
        if (ch == 3) {
            *data++ = sin_48k[*s_cnt] << 16;
            *data++ = sin_48k[*s_cnt] << 16;
        }

        if (ch == 8) {
            *data++ = sin_48k[*s_cnt] << 16 / 4;
            *data++ = sin_48k[*s_cnt] << 16 / 4;
            *data++ = sin_48k[*s_cnt] << 16 / 4;
            *data++ = sin_48k[*s_cnt] << 16 / 4;
            *data++ = sin_48k[*s_cnt] << 16 / 4;
            *data++ = sin_48k[*s_cnt] << 16 / 4;
            *data++ = sin_48k[*s_cnt] << 16 / 4;
        }
        (*s_cnt)++;
    }
    return 0;
}
void handle_tdm_rx_demo(void *priv, void *buf, u32 len)
{
    s32 *data_s32 = buf;
    s16 *data_s16 = buf;
    if (TDM_32_BIT_EN) {
        for (u16 i = 0; i < len / TDM_SLOT_NUM / 4; i++) {
            data_s32[i] = data_s32[TDM_SLOT_NUM * i] / 2 ;
        }
        u32 len0 = len / TDM_SLOT_NUM;
        u32 rlen = 0;
        len0 = sw_src_run((s16 *)data_s32, (s16 *) data_s32, len0);
        while (len0) {
            rlen = audio_dac_write(&dac_hdl, data_s32, len0);
            if (rlen != len0) {
                printf("re:%d,%d\n", rlen, len0);
            }
            len0 -= rlen;
            data_s32 += rlen / 4  ;
        }
    } else {
        int unread_frames = JL_AUDIO->DAC_LEN - JL_AUDIO->DAC_SWN - 1;
        for (u16 i = 0; i < len / TDM_SLOT_NUM / 2; i++) {
            data_s16[i] = data_s16[TDM_SLOT_NUM * i] / 6 + data_s16[TDM_SLOT_NUM * i + 1] / 6;
        }
        u32 len0 = len / TDM_SLOT_NUM;
        u32 rlen = 0;
        len0 = sw_src_run(data_s16, data_s16, len0);
        while (len0) {
            rlen = audio_dac_write(&dac_hdl, data_s16, len0);
            if (rlen != len0) {
                printf("RE:%d,%d,%d\n", rlen, len0, unread_frames);
            }
            len0 -= rlen;
            data_s16 += rlen / 2 ;
        }
    }
}

void handle_tdm_tx_demo(void *priv, void *buf, u32 len)
{
    putchar('i');
    s16 *data_buf = (s16 *)buf;
    /* get_sine_16k_data(&tx_s_cnt, mmbuf, 512 / 4, 1); */
    for (u16 i = 0; i < len / 3 / 2; i ++) {
        data_buf[3 * i]  = i | i << 8;
        data_buf[3 * i + 1]  = i | i << 8;
        data_buf[3 * i + 2]  = i | i << 8;
    }
    /* put_buf((u8 *)buf, len); */

}
#endif

static TDM_PARM	TDM_RX_CFG = {
    .mclk_io = TCFG_TDM_RX_PORT_MCLK,
    .bclk_io = TCFG_TDM_RX_PORT_BCLK,
    .wclk_io = TCFG_TDM_RX_PORT_WCLK,
    .data_io = TCFG_TDM_RX_PORT_DATA,
    .role = TCFG_TDM_RX_ROLE,
    /*.role = TDM_ROLE_SLAVE,*/
    .delay_mode = TDM_WORK_MODE,
    .update_sample_mode = TDM_BCLK_FALL_UPDATE_RAISE_SAMPLE,
    .data_width = TDM_SLOT_MODE_16BIT,
    .sample_rate = TDM_SAMPLE_RATE,
    .wclk_duty = 1, //位宽乘以slot num 再除以2 就是50 %占空比
    .slot_num = TDM_SLOT_NUM,
#if AUDIO_TDM_DEMO_EN
    .isr_cb = handle_tdm_rx_demo,
#else
    .isr_cb = handle_tdm_rx,
#endif
};

static TDM_PARM	TDM_TX_CFG = {
    .mclk_io = TCFG_TDM_RX_PORT_MCLK,
    .bclk_io = TCFG_TDM_RX_PORT_BCLK,
    .wclk_io = TCFG_TDM_RX_PORT_WCLK,
    .data_io = TCFG_TDM_RX_PORT_DATA,
    .role = TCFG_TDM_TX_ROLE,
    .delay_mode = TDM_WORK_MODE,
    .update_sample_mode = TDM_BCLK_FALL_UPDATE_RAISE_SAMPLE,
    .data_width =  TDM_SLOT_MODE_16BIT,
    .sample_rate = TDM_SAMPLE_RATE,
    .wclk_duty = 1,
    .slot_num = TDM_SLOT_NUM,
#if AUDIO_TDM_DEMO_EN
    .isr_cb = handle_tdm_tx_demo,
#else
    .isr_cb = handle_tdm_tx,
#endif
};

void *audio_tdm_get_hdl()
{
    return __this;
}

static void tdm_io_out_init(u8 gpio)
{
    gpio_set_mode(IO_PORT_SPILT(gpio), PORT_OUTPUT_HIGH);
}

static void tdm_io_in_init(u8 gpio)
{
    gpio_set_mode(IO_PORT_SPILT(gpio), PORT_INPUT_FLOATING);
}

static void tdm_master_io_init(void)
{
    //MASTER模式输出时钟, 数据输入
    if (__this->mclk_io) {
        tdm_io_out_init(__this->mclk_io);
        gpio_set_fun_output_port(__this->mclk_io, FO_TDM_M_MCK, 1, 1);
    }
    if (__this->bclk_io) {
        tdm_io_out_init(__this->bclk_io);
        gpio_set_fun_output_port(__this->bclk_io, FO_TDM_M_BCK, 1, 1);
    }
    if (__this->wclk_io) {
        tdm_io_out_init(__this->wclk_io);
        gpio_set_fun_output_port(__this->wclk_io, FO_TDM_M_WCK, 1, 1);
    }
    if (__this->data_io) {
        tdm_io_in_init(__this->data_io);
        gpio_set_fun_input_port(__this->data_io, PFI_TDM_M_DA);
    }
}

static void tdm_slave_io_init(void)
{
    //SLAVE模式输入时钟, 数据输出
    if (__this->bclk_io) {
        tdm_io_in_init(__this->bclk_io);
        gpio_set_fun_input_port(__this->bclk_io, PFI_TDM_S_BCK);
    }
    if (__this->wclk_io) {
        tdm_io_in_init(__this->wclk_io);
        gpio_set_fun_input_port(__this->wclk_io, PFI_TDM_S_WCK);
    }
    if (__this->data_io) {
        tdm_io_out_init(__this->data_io);
        gpio_set_fun_output_port(__this->data_io, FO_TDM_S_DA, 1, 1);
    }
}
extern u32 clock_tdm_sel(u8 module, u32 frequency);
static void tdm_sr_config(TDM_SR sr)
{
    u32 mclk_fre = 0;
    u32 bclk_fre = 0;
    switch (sr) {
    case TDM_SR_8000:
    case TDM_SR_12000:
    case TDM_SR_16000:
    case TDM_SR_24000:
    case TDM_SR_32000:
    case TDM_SR_48000:
        mclk_fre = 12288000UL; //320M div 26
        break;
    case TDM_SR_11025:
    case TDM_SR_22050:
    case TDM_SR_44100:
        mclk_fre = 11289600UL; //192M div 17
        break;
    default:
        ASSERT(0, "tdm no suport %d sr", sr);
        break;
    }

    clock_tdm_sel(0, mclk_fre);
    /* clock_tdm_sel("tdm", mclk_fre); */

    //=============================================//
    //         WCLK周期计算(基于bclk个数)          //
    //=============================================//
    u16 bit_width = 8;
    switch (__this->data_width) {
    case TDM_SLOT_MODE_8BIT:
        bit_width = 8;
        break;
    case TDM_SLOT_MODE_16BIT:
        bit_width = 16;
        break;
    case TDM_SLOT_MODE_20BIT:
        bit_width = 20;
        break;
    case TDM_SLOT_MODE_24BIT:
        bit_width = 24;
        break;
    case TDM_SLOT_MODE_32BIT:
        bit_width = 32;
        break;
    }

    u16 bclk_per_frame = __this->slot_num * bit_width;

    if (__this->delay_mode) {
        /*delay模式时，丢弃第0个slot，用后面几个slot*/
        TDM_WCLK_PRD((__this->slot_num + 1) * bit_width);
    } else {
        TDM_WCLK_PRD(__this->slot_num * bit_width);
    }

    //=============================================//
    //                 BCLK计算                    //
    //=============================================//
    if (__this->delay_mode == TDM_DELAY_ONE_BCLK_MODE) {
        bclk_per_frame--;
    }
    bclk_fre = bclk_per_frame * sr;
    TDM_BCLK_DIV(mclk_fre / bclk_fre);
    tdm_debug("%d %d %d %d\n", mclk_fre, bclk_fre, bclk_per_frame, sr);

    JL_TDM->SLOT_EN = 0;
    for (u8 i = 0; i < (__this->slot_num); i++) {
        JL_TDM->SLOT_EN |= BIT(i);
    }
    if (__this->delay_mode) {
        /*delay模式时，丢弃第0个slot，用后面几个slot*/
        JL_TDM->SLOT_EN <<= 1;
    }

    //=============================================//
    //         WCLK高电平宽度(基于bclk个数)          //
    //=============================================//
    TDM_WCLK_DUTY(__this->wclk_duty);
}


___interrupt
static void tdm_isr(void)
{
    spin_lock(&__this->lock);
    s8 *buf_addr = NULL;
    u32 len = __this->dma_len / 2;

    buf_addr = (s8 *)(__this->dma_buf);

    if (TDM_IS_PNDING0()) {
        TDM_PND0_CLR();
    } else if (TDM_IS_PNDING1()) {
        TDM_PND1_CLR();
        buf_addr += len;
    } else {
        spin_unlock(&__this->lock);
        return;
    }
    if (!__this) {
        spin_unlock(&__this->lock);
        return;
    }
    if (__this->isr_cb) {
        __this->irq_flag = 1;
        __this->isr_cb(__this->private_data, buf_addr, len);
    }
    spin_unlock(&__this->lock);
}

static void tdm_info_dump(void)
{
    tdm_debug("JL_TDM->CON0 = 0x%x\n", JL_TDM->CON0);
    tdm_debug("JL_TDM->WCLK_DUTY = 0x%x\n", JL_TDM->WCLK_DUTY);
    tdm_debug("JL_TDM->WCLK_PRD = 0x%x\n", JL_TDM->WCLK_PRD);
    tdm_debug("JL_TDM->DMA_BASE_ADR = 0x%x\n", JL_TDM->DMA_BASE_ADR);
    tdm_debug("JL_TDM->DMA_BUF_LEN = 0x%x\n", JL_TDM->DMA_BUF_LEN);
    tdm_debug("JL_TDM->SLOT_EN = 0x%x\n", JL_TDM->SLOT_EN);
    tdm_debug("JL_CLOCK->CLK_CON3 = 0x%x\n", JL_CLOCK->CLK_CON3);
}

void tdm_rx_cfg_dump(const TDM_PARM *cfg)
{
    tdm_debug("TDM_RX_CFG configuration:\n");
    tdm_debug("mclk_io: %d\n", cfg->mclk_io);
    tdm_debug("bclk_io: %d\n", cfg->bclk_io);
    tdm_debug("wclk_io: %d\n", cfg->wclk_io);
    tdm_debug("data_io: %d\n", cfg->data_io);
    tdm_debug("role: %d\n", cfg->role);
    tdm_debug("delay_mode: %d\n", cfg->delay_mode);
    tdm_debug("update_sample_mode: %d\n", cfg->update_sample_mode);
    tdm_debug("data_width: %d\n", cfg->data_width);
    tdm_debug("sample_rate: %d\n", cfg->sample_rate);
    tdm_debug("wclk_duty: %d\n", cfg->wclk_duty);
    tdm_debug("slot_num: %d\n", cfg->slot_num);

#if AUDIO_TDM_DEMO_EN
    tdm_debug("isr_cb: handle_tdm_rx_demo\n");
#else
    tdm_debug("isr_cb: handle_tdm_rx\n");
#endif
}

int tdm_init(TDM_PARM *parm)
{
    TDM_CON_RESET();

    if (parm == NULL) {
        return -1;
    }

    __this = parm;

    //=============================================//
    //                 TDM模式配置                 //
    //=============================================//
    TDM_HOST_MODE_SEL(__this->role);
    if (__this->role == TDM_ROLE_MASTER) {
        tdm_master_io_init();
        /*模式采用上升沿采样时置1 ，优化模块时序, 建议置0使用 */
        TDM_RX_RISING_OPT(0);
    } else {
        tdm_slave_io_init();
        /*TX 模式时置 1 ，优化模块时序*/
        TDM_TX_TIMING_OPT(1);
    }

    //=============================================//
    //                 TDM时钟配置                 //
    //=============================================//
    TDM_DLY_MODE_ENABLE(__this->delay_mode);
    if (__this->delay_mode) {
        if (__this->role == TDM_ROLE_MASTER) {
            /*RX delay 模式时置1 ，优化模块时序*/
            TDM_RX_DLY_OPT(1);
        } else {
            /*TX delay模式时置1，优化模块时序*/
            TDM_TX_DLY_OPT(1);
        }
    }
    /*BLCK更新边沿和采样边沿选择*/
    TDM_DATA_CLK_MODE_SEL(__this->update_sample_mode);

    //=============================================//
    //BCLK与WCLK同时输出配置(该配置暂时默认配置为0)//
    //=============================================//
    if (__this->role == TDM_ROLE_MASTER) {
#if 1
        /*0: 写RX_KST后 ，BCLK和WCLK同时对齐输出*/
        TDM_BCLK_MODE(0);
        /*RX模式下 ，当 BCLK_MODE (bit19) = 0 时置1 ，优化模块时序*/
        TDM_RX_TIMING_OPT(1);
#else
        /*1 ：配置好时钟后BCLK直接输出，而WCLK需要写RX_KST再输出出*/
        TDM_BCLK_MODE(1);
        TDM_RX_TIMING_OPT(0);
#endif
    }

    //=============================================//
    //                TDM 采样率配置               //
    //=============================================//
    tdm_sr_config(__this->sample_rate);

    //=============================================//
    //                 TDM DMA配置                 //
    //=============================================//
    TDM_SLOT_MODE_SEL(__this->data_width);
    if (!__this->dma_buf) {
        __this->dma_buf = dma_malloc(__this->dma_len);
        memset(__this->dma_buf, 0, __this->dma_len);
    }
    JL_TDM->DMA_BASE_ADR = (u32)__this->dma_buf;

    TDM_DMA_BUF_LEN_SET(__this->dma_len / 2);
    //=============================================//
    //                 TDM 中断配置                //
    //=============================================//
    TDM_PND0_CLR();
    TDM_PND1_CLR();
    request_irq(IRQ_TDM_IDX, 1, tdm_isr, 0);

    return 0;
}

int tdm_start(void)
{
    TDM_MODULE_EN(1);
    TDM_IE(1);

    TDM_RX_KICK_START();
    tdm_debug("tdm_start\n");

    /* TDM_STATUS_RST(); */
    tdm_info_dump();
    return 0;
}

void tdm_close(void)
{
    TDM_MODULE_EN(0);
    TDM_IE(0);
    if (__this->dma_buf) {
        dma_free(__this->dma_buf);
        __this->dma_buf = NULL;
    }
    __this->tx_start = 0;
    __this->private_data = NULL;
    tdm_debug("tdm_uninit\n");
}

int tdm_get_tx_ch_points()
{
    if (__this && __this->tdm_initd) {
        return cbuf_get_data_len(&__this->tdm_cbuf) / (TDM_32_BIT_EN ? 4 : 2) ;
    }
    return 0;
}

__attribute__((always_inline))
u32 tdm_timestamp()
{
    if (__this) {
        return 	__this->timestamp;
    }
    return 0;
}

/* static u32 last_time = 0; */
#define timestamp_threshold (100)
__attribute__((always_inline))
void tdm_cal_timestamp()
{
    if (__this) {
        u32 time = audio_jiffies_usec();
        /* printf("t:%d,%d,%d,%d\n",__LINE__,time,last_time,time-last_time); */
        /* last_time = time; */
        if (__this->timestamp_cnt  <= timestamp_threshold) {
            if (__this->timestamp) {
                __this->timestamp_gap  += (time - __this->timestamp);
            }
            if (__this->timestamp_cnt  == timestamp_threshold) {
                __this->timestamp_gap  /= timestamp_threshold;
            }
            __this->timestamp_cnt ++;

            __this->timestamp  = time;
        } else if (__this->timestamp_cnt  > timestamp_threshold) {
            __this->timestamp  += __this->timestamp_gap ;
            int diff_us = __this->timestamp  - time;
            if (__builtin_abs(diff_us) > (1000000 / __this->sample_rate)) {
                if (__builtin_abs(diff_us) > 10 * (1000000 / __this->sample_rate)) {
                    if (diff_us < 0) {
                        __this->timestamp  += 10;
                    }
                    if (diff_us > 0) {
                        __this->timestamp  -= 10;
                    }

                } else {
                    if (diff_us < 0) {
                        __this->timestamp  += 1;
                    }
                    if (diff_us > 0) {
                        __this->timestamp  -= 1;
                    }
                }
                /* printf("diff %d, %d %d,%d\n", diff_us, __this->timestamp ,time,__this->timestamp_gap); */
            }
        }
    }

}

static void handle_tdm_tx(void *priv, void *buf, u32 len)
{
    int rlen = 0;
    int ch_len = len / TDM_SLOT_NUM;
    if (!__this-> tx_start && cbuf_get_data_len(&__this->tdm_cbuf) > ch_len) {
        __this->tx_start = 1;
    }

    if (!__this->tx_start) {
        memset(buf, 0, len);
        return;
    }

#if TCFG_MULTI_CH_TDM_NODE_ENABLE
    if (__this->role == TDM_ROLE_SLAVE) {
        tdm_cal_timestamp();//记录double buf tdm中断产生的时间戳
    }
    rlen = cbuf_read(&__this->tdm_cbuf, buf, len);
    /* printf("t:%d,%d,%d,%d,%d,%d,%d,%d\n",d_buf[0],d_buf[1],d_buf[2],d_buf[3],d_buf[4],d_buf[5],d_buf[6],d_buf[7]); */
    int frames = ((rlen >> (TDM_32_BIT_EN ? 2 : 1)) / TDM_SOURCE_CH_NUM);
    audio_tdm_update_frame(__this->private_data, frames);
    if (rlen < ch_len) {
        printf("TX EMPTY\n");
    }
#else
    rlen = cbuf_read(&__this->tdm_cbuf, __this->ch_buf, ch_len);
    int frames = (rlen >> (TDM_32_BIT_EN ? 2 : 1));
    audio_tdm_update_frame(__this->private_data, frames);
    if (rlen < ch_len) {
        printf("TDM TX EMPTY\n");
    }
    if (TDM_32_BIT_EN) {
        s32 *outbuf = buf;
        s32 *inbuf = __this->ch_buf;
        for (int j = 0; j < ch_len / 4; j++) {
            for (int i = 0; i < TDM_SLOT_NUM; i++) {
                *outbuf++ = inbuf[j] ;
            }
        }
    } else {
        s16 *outbuf = buf;
        s16 *inbuf = __this->ch_buf;
        for (int j = 0; j < ch_len / 2; j++) {
            for (int i = 0; i < TDM_SLOT_NUM; i++) {
                *outbuf++ = inbuf[j];
            }
        }
    }
#endif
}

static void handle_tdm_rx(void *priv, void *buf, u32 len)
{
#if TCFG_MULTI_CH_TDM_RX_NODE_ENABLE
    tdm_sample_output_handler(buf, len);
    s16 *d_buf = (s16 *)buf;
    /* printf("r:%d,%d,%d,%d,%d,%d,%d,%d\n",d_buf[0],d_buf[1],d_buf[2],d_buf[3],d_buf[4],d_buf[5],d_buf[6],d_buf[7]); */
#else
    s32 *data_s32 = buf;
    s16 *data_s16 = buf;
    if (TDM_32_BIT_EN) {
        for (u16 i = 0; i < len / TDM_SLOT_NUM / 4; i++) {
            data_s32[i] = data_s32[TDM_SLOT_NUM * i] ;  //取第0个通道的数据
        }
        u32 len0 = len / TDM_SLOT_NUM;
        tdm_sample_output_handler(buf, len0);
    } else {
        for (u16 i = 0; i < len / TDM_SLOT_NUM / 2; i++) {
            data_s16[i] = data_s16[TDM_SLOT_NUM * i] ; //取第0个通道的数据
        }
        u32 len0 = len / TDM_SLOT_NUM;
        tdm_sample_output_handler(buf, len0);
    }
#endif
}

/* 定时器检测 tdm 发送中断在线 */
static void tdm_det_timer_cb(void *priv)
{
    TDM_PARM *hdl = (TDM_PARM *)priv;
    if (hdl) {
        if (hdl->irq_flag) {
            hdl->irq_flag = 0;
            hdl->tx_online = 1;
        } else {//没有中断了
            hdl->tx_online = 0;
        }
    }
}

void audio_tdm_tx_open(void *priv)
{
    if (__this) {
#if TDM_TX_OPEN_ALWAYS
        if (__this->role == TDM_ROLE_SLAVE) {
            if (priv) {
                __this->private_data = priv;
            }
            __this->timestamp = 0;
            __this->timestamp_gap = 0;
            __this->timestamp_cnt = 0;
            return;
        }
#endif
        tdm_close();
        __this = NULL;
    }
    TDM_TX_CFG.data_width = (TDM_32_BIT_EN ? TDM_SLOT_MODE_32BIT : TDM_SLOT_MODE_16BIT);
    TDM_TX_CFG.dma_len = TDM_DMA_LEN;
    tdm_init(&TDM_TX_CFG);
    if (priv) {
        __this->private_data = priv;
    }
    if (!__this->det_timer_id) {
        __this->det_timer_id = usr_timer_add(__this, tdm_det_timer_cb, TDM_ONLINE_DET_TIME, 0);
    }
    log_info("TDM TX OPEN:buf_len:%d,irq_len:%d\n", TDM_TX_CBUF_LEN, TDM_IRQ_POINTS * (TDM_32_BIT_EN ? 4 : 2));
    __this->tdm_buf = zalloc(TDM_TX_CBUF_LEN);
    __this->ch_buf = zalloc(TDM_IRQ_POINTS * (TDM_32_BIT_EN ? 4 : 2));
    cbuf_init(&__this->tdm_cbuf, __this->tdm_buf, TDM_TX_CBUF_LEN);
    spin_lock_init(&__this->lock);
    tdm_start();
    __this->tdm_initd = 1;
}

void audio_tdm_rx_open(void *priv)
{
    if (__this) {
        tdm_close();
        __this = NULL;
    }
    if (priv) {
        __this->private_data = priv;
    }
    TDM_RX_CFG.data_width = (TDM_32_BIT_EN ? TDM_SLOT_MODE_32BIT : TDM_SLOT_MODE_16BIT);
    TDM_RX_CFG.dma_len = TDM_DMA_LEN;
    tdm_init(&TDM_RX_CFG);
    spin_lock_init(&__this->lock);
    tdm_start();
    __this->tdm_initd = 1;
}

void audio_tdm_close()
{
    if (__this) {
#if TDM_TX_OPEN_ALWAYS
        if (__this->role == TDM_ROLE_SLAVE) {
            __this->private_data = NULL;
            __this->tx_start = 0;
            cbuf_clear(&__this->tdm_cbuf);
            __this->timestamp = 0;
            __this->timestamp_gap = 0;
            __this->timestamp_cnt = 0;
            return;
        }
#endif
        spin_lock(&__this->lock);
        tdm_close();//关闭硬件
        if (__this->det_timer_id) {
            usr_timer_del(__this->det_timer_id);
            __this->det_timer_id = 0;
        }
        __this->tdm_initd = 0;
        if (__this->tdm_buf) {
            free(__this->tdm_buf);
            __this->tdm_buf = NULL;
        }
        if (__this->ch_buf) {
            free(__this->ch_buf);
            __this->ch_buf = NULL;
        }
        __this = NULL;
        spin_unlock(&__this->lock);
    }
}

int audio_tdm_write(void *buf, int len)
{
    if (__this && __this->tdm_initd) {
        if (__this->tx_online) {
            int wlen = cbuf_write(&__this->tdm_cbuf, buf, len);
            if (wlen != len) {
                printf("w:%d,%d\n", wlen, len);
            }
            return wlen;
        } else {
            printf("TDM TX NOT ONLEIN\n");
            return len;
        }
    } else {
        printf("TDM TX NOT INITED\n");
        return  len;
    }
}

#if AUDIO_TDM_DEMO_EN
//===========================================================================//
//测试代码:
//测试模型: TDM[主][收] <--> lis25ba[从][发]
//===========================================================================//
void tdm_demo(void)
{
    /*    dac_init     */
    app_audio_state_switch(APP_AUDIO_STATE_MUSIC, app_audio_volume_max_query(AppVol_BT_MUSIC), NULL);	// 音量状态设置
    audio_dac_set_volume(&dac_hdl, 30);					// dac 音量设置
    audio_dac_set_sample_rate(&dac_hdl, TDM_SAMPLE_RATE);							// 采样率设置
    audio_dac_start(&dac_hdl);											// dac 启动
    audio_dac_channel_start(NULL);
    //lis25ba 初始化
    /* extern int lis25ba_tdm_mode_init(u8 data_mode, u16 sample_rate); */
    /* lis25ba_tdm_mode_init(3, 16000); */
    tdm_debug("tdm test>>>> \n");
    //tdm 初始化
    tdm_init(&TDM_RX_CFG);
    ///变采样
    sw_src_init(1, 48000, (48000 * 624 + 625) / 625);
    tdm_start();

    while (1) {
        os_time_dly(2);
    }
}


#endif /* #if TDM_TEST_ENABLE*/
