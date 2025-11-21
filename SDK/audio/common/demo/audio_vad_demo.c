/*
 ****************************************************************************
 *							Audio VAd Demo
 *
 *Description	: Audio VAD使用范例
 *Notes			: (1)本demo为开发测试范例，请不要修改该demo， 如有需求，请自行
 *				  复制再修改
 *				  (2)mic工作模式说明
 *				A.单端隔直(cap_mode)
 *				  可以选择mic供电方式：外部供电还是内部供电(mic_bias_inside = 1)
 *				B.单端省电容(capless_mode)
 *				C.差分模式(cap_diff_mode)
 ****************************************************************************
 */
#include "cpu/includes.h"
#include "media/includes.h"
#include "system/includes.h"
#include "app_main.h"
#include "audio_config.h"
#include "audio_demo.h"
#include "audio_adc.h"
#include "nn_vad.h"
#include "vad_mic.h"
#include "clock_manager/clock_manager.h"

#if 1
#define VAD_DEMO_LOG	printf
#else
#define VAD_DEMO_LOG(...)
#endif

extern struct audio_adc_hdl adc_hdl;
extern u8 audio_adc_file_get_mic_mode(u8 mic_index);


#define VAD_DEMO_CH_NUM        	1	/*支持的最大采样通道(max = 1)*/
#define VAD_DEMO_BUF_NUM        2	/*采样buf数*/
#define VAD_DEMO_IRQ_POINTS     160	/*采样中断点数*/
#define VAD_DEMO_BUFS_SIZE      (VAD_DEMO_CH_NUM * VAD_DEMO_BUF_NUM * VAD_DEMO_IRQ_POINTS)

/*
 * 选择 VAD（语音识别检测）时，需要配置以下宏：
 * 1. CONFIG_VAD_PLATFORM_SUPPORT_EN = 1   // 启用平台支持的 VAD 功能
 * 2. TCFG_VAD_LOWPOWER_CLOCK = VAD_CLOCK_USE_PMU_STD12M  // 配置 VAD 低功耗时钟，使用 PMU 标准 12MHz 时钟
 * 3. 注释audio_setup.c下audio_smart_voice_detect_init函数
 */

#define VAD_PLATFORM			1 //1: nn_vad  2:vad

/*数据导出DEBUG宏*/
#if 0
#define VAD_DEMO_DEBUG_DATA		1
#else
#define VAD_DEMO_DEBUG_DATA		0
#endif


#if (VAD_PLATFORM == 2)
static struct vad_mic_platform_data vad_mic_data = {
    .mic_data = {
        /*mic类型：隔直/差分/省电容*/
        .mic_mode = AUDIO_MIC_CAP_DIFF_MODE,
        .mic_ldo_isel = 2,
        .mic_ldo_vsel = 5,
        .mic_ldo2PAD_en = 1,
        .mic_bias_en = 0,
        .mic_bias_res = 0,
        /*是否使用mic_bias*/
        .mic_bias_inside = 0,
        /* ADC偏置电阻配置*/
        .adc_rbs = 3,
        /* ADC输入电阻配置*/
        .adc_rin = 3,
        /* .adc_test = 1, */
    },
    .power_data = {
        /*VADLDO电压档位：0~7*/
        .ldo_vs = 3,
        /*VADLDO误差运放电流档位：0~3*/
#if TCFG_VAD_LOWPOWER_CLOCK == VAD_CLOCK_USE_PMU_STD12M
        .ldo_is = 1,
#else
        .ldo_is = 2,
#endif
        .clock = TCFG_VAD_LOWPOWER_CLOCK, /*VAD时钟选项*/
        .acm_select = 8,
    },
};
#endif


#if VAD_DEMO_DEBUG_DATA
extern int aec_uart_open(u8 nch, u16 single_size);
extern int aec_uart_fill(u8 ch, void *buf, u16 size);
extern void aec_uart_write(void);
extern int aec_uart_close(void);
#endif

#define VAD_DEMO_TASK_NAME      "vad_demo"

struct vad_demo {
    struct audio_adc_output_hdl adc_output;
    struct adc_mic_ch mic_ch;
    s16 *adc_buf;
    void *vad;
    OS_SEM sem;
    cbuffer_t cbuf;
    u8 c_buf[VAD_DEMO_BUFS_SIZE << 1];
    s16 runbuf[VAD_DEMO_IRQ_POINTS << 1];
};
static struct vad_demo *__this = NULL;

/*
*********************************************************************
*                  AUDIO VAD OUTPUT
* Description: vad数据输出回调
* Arguments  : pric 私有参数
*			   data	mic数据
*			   len	数据长度
*********************************************************************
*/

static int vad_output(void *priv, s16 *data, int len)
{
    /* printf("> %d %d %d %d\n", data[0], data[1], data[2], data[3]); */
#if VAD_DEMO_DEBUG_DATA
    aec_uart_fill(0, data, VAD_DEMO_IRQ_POINTS << 1);
    aec_uart_write();
#endif

    if (__this && __this->vad) {

    }
    return 0;
}



static int vad_task_data_write(void *data, int len)
{
    int wlen = 0;
#if (VAD_PLATFORM == 1)
    wlen = cbuf_write(&__this->cbuf, data, len);
    if (wlen == len) {
        os_sem_post(&__this->sem);
    } else {
        printf("vad cbuf write  err %d, %d", wlen, len);
    }
#else
    vad_output(__this, data, len);
#endif
    return wlen;
}

static void vad_demo_task(void *priv)
{
    int rlen;
    int flag;
    while (1) {
        os_sem_pend(&__this->sem, 0);
        rlen = cbuf_read(&__this->cbuf, __this->runbuf, VAD_DEMO_IRQ_POINTS << 1);
        if (rlen == (VAD_DEMO_IRQ_POINTS << 1)) {
            putchar('w');
#if (VAD_DEMO_DEBUG_DATA && (VAD_PLATFORM == 2))
            aec_uart_fill(0, __this->runbuf, VAD_DEMO_IRQ_POINTS << 1);
            aec_uart_write();
#endif
            flag = audio_nn_vad_data_handler(__this->vad, __this->runbuf, VAD_DEMO_IRQ_POINTS << 1);
            if (flag) {
                printf("nn_vad!!!\n");
            }
        } else {
            printf("vad demo cbuf read fail");
        }
    }
}

/* void timer_no_response_callback(const char *task_name, void *func, u32 msec, void *timer, u32 curr_msec)
{
    return;
} */


/*
*********************************************************************
*                  AUDIO MIC OUTPUT
* Description: mic采样数据回调
* Arguments  : pric 私有参数
*			   data	mic数据
*			   len	数据长度
*********************************************************************
*/
s16 *ptr16 = NULL;
s32 *ptr32 = NULL;
s16 *temp16 = NULL;
s32 *temp32 = NULL;
static void adc_output(void *priv, s16 *data, int len)
{
    /* printf("> %d %d %d %d\n", data[0], data[1], data[2], data[3]); */
    if (__this && __this->vad) {
        vad_task_data_write(data, len);
    }
}

/*
*********************************************************************
*                  AUDIO ADC MIC OPEN
* Description: 打开mic通道
* Arguments  : mic_idx 		mic通道
*			   gain			mic增益
*			   sr			mic采样率
* Return	 : None.
* Note(s)    : (1)打开一个mic通道示例：
*				audio_adc_open(AUDIO_ADC_MIC_0,10,16000,1);
*				或者
*				audio_adc_open(AUDIO_ADC_MIC_1,10,16000,1);
*			   (2)打开两个mic通道示例：
*				audio_adc_open(AUDIO_ADC_MIC_1|AUDIO_ADC_MIC_0,10,16000,1);
*********************************************************************
*/
void audio_adc_open(u8 mic_idx, u8 gain, int sr)
{

    __this->adc_buf = zalloc(VAD_DEMO_BUFS_SIZE << 2);
    ASSERT(__this->adc_buf);

    //step0:打开mic通道，并设置增益
    //step1:设置mic通道采样率,LPADC会根据采样率分频，需要在mic_open前配置采样率
    audio_adc_mic_set_sample_rate(&__this->mic_ch, sr);
#if 1   // use config file
    extern void audio_adc_file_init(void);
    extern int adc_file_mic_open(struct adc_mic_ch * mic, int ch);
    audio_adc_file_init();

    for (int i = 0; i < AUDIO_ADC_MAX_NUM; i++) {
        if (mic_idx & AUDIO_ADC_MIC(i)) {
            adc_file_mic_open(&__this->mic_ch, AUDIO_ADC_MIC(i));
        }
    }
#else
    struct mic_open_param mic_param = {0};
    mic_param.mic_mode      = AUDIO_MIC_CAP_MODE;
    mic_param.mic_ain_sel   = AUDIO_MIC0_CH0;
    mic_param.mic_bias_sel  = AUDIO_MIC_BIAS_CH0;
    mic_param.mic_bias_rsel = 4;
    mic_param.mic_dcc       = 8;
    audio_adc_mic_open(&__this->mic_ch, AUDIO_ADC_MIC_0, &adc_hdl, &mic_param);
    audio_adc_mic_set_gain(&__this->mic_ch, AUDIO_ADC_MIC_0, gain);
    audio_adc_mic_gain_boost(AUDIO_ADC_MIC_0, 1);
#endif
    //step2:设置mic采样buf
    audio_adc_mic_set_buffs(&__this->mic_ch, __this->adc_buf, VAD_DEMO_IRQ_POINTS * 2, VAD_DEMO_BUF_NUM);
    //step3:设置mic采样输出回调函数
    __this->adc_output.handler = adc_output;
    audio_adc_add_output_handler(&adc_hdl, &__this->adc_output);
    //step4:启动mic通道采样
    audio_adc_mic_start(&__this->mic_ch);

    VAD_DEMO_LOG("audio_adc start succ\n");

    /* while (1) {
        os_time_dly(500);
    } */
}

/*
*********************************************************************
*                  AUDIO VAD OPEN
* Description: 打开vad模块
* Arguments  : None.
* Return     : None.
* Note(s)    : None.
*********************************************************************
*/

void audio_vad_demo_open(void)
{
    __this = zalloc(sizeof(struct vad_demo));
    ASSERT(__this);
    audio_adc_open(AUDIO_ADC_MIC_0, 10, 16000);
#if (VAD_PLATFORM == 1)
    __this->vad = audio_nn_vad_open();
#else
    /*获取mic0的配置, vad只能是mic0*/
    vad_mic_data.mic_data.mic_mode = audio_adc_file_get_mic_mode(0);
    vad_mic_data.mic_data.mic_bias_inside = (audio_adc_file_get_mic_mode(0) == AUDIO_MIC_CAP_DIFF_MODE) ? 0 : 1;
    lp_vad_mic_data_init((struct vad_mic_platform_data *)&vad_mic_data);
    __this->vad = lp_vad_mic_open((void *)__this, vad_output);
#endif
    ASSERT(__this->vad);
    os_sem_create(&__this->sem, 0);
    cbuf_init(&__this->cbuf, __this->c_buf, sizeof(__this->c_buf));
    task_create(vad_demo_task, __this, VAD_DEMO_TASK_NAME);

#if VAD_DEMO_DEBUG_DATA
    aec_uart_open(1, 320);
#endif
    VAD_DEMO_LOG("audio_vad open succ\n");

    while (1) {
        os_time_dly(500);
    }


}

/*
*********************************************************************
*                  AUDIO ADC MIC CLOSE
* Description: 关闭mic采样模块
* Arguments  : None.
* Return     : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_adc_close(void)
{
    VAD_DEMO_LOG("audio_adc_close\n");
    if (__this) {
        audio_adc_del_output_handler(&adc_hdl, &__this->adc_output);
        audio_adc_mic_close(&__this->mic_ch);
        /*audio_adc_mic_close()里面自动释放adcbuf，不需要外面释放*/
        /* free(__this->adc_buf); */
    }
}

/*
*********************************************************************
*                  AUDIO VAD CLOSE
* Description: 关闭vad模块
* Arguments  : None.
* Return     : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_vad_close(void)
{
    VAD_DEMO_LOG("audio_vad_close\n");
    if (__this) {
        audio_adc_close();
        if (__this->vad) {
#if (VAD_PLATFORM == 1)
            audio_nn_vad_close(__this->vad);
            __this->vad = NULL;
#else
            lp_vad_mic_close(__this->vad);
            __this->vad = NULL;
#endif
        }
        task_kill(VAD_DEMO_TASK_NAME);
        free(__this);
        __this = NULL;
        free(temp16);
        free(temp32);
    }
}


#if AUDIO_DEMO_LP_REG_ENABLE
static u8 vad_demo_idle_query()
{
    return (__this) ? 0 : 1;
}

REGISTER_LP_TARGET(vad_demo_lp_target) = {
    .name = "vad_demo",
    .is_idle = vad_demo_idle_query,
};

#endif/*AUDIO_DEMO_LP_REG_ENABLE*/

