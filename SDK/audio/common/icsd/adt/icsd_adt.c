#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".icsd_adt.data.bss")
#pragma data_seg(".icsd_adt.data")
#pragma const_seg(".icsd_adt.text.const")
#pragma code_seg(".icsd_adt.text")
#endif
#include "app_config.h"
#if ((defined TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN) && TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN && \
	 TCFG_AUDIO_ANC_ENABLE)
#include "system/task.h"
#include "icsd_adt.h"
#include "icsd_adt_app.h"
#include "classic/tws_api.h"
#include "tone_player.h"
#include "icsd_anc_user.h"
#include "asm/audio_adc.h"
#include "audio_anc.h"

#define ICSD_ANC_TASK_NAME  "icsd_anc"
#define ICSD_ADT_TASK_NAME  "icsd_adt"
#define ICSD_SRC_TASK_NAME  "icsd_src"

int wind_adc_sr = 8000;//

int (*adt_printf)(const char *format, ...);
int (*wat_printf)(const char *format, ...);
int (*ein_printf)(const char *format, ...);
int (*wind_printf)(const char *format, ...);
int printf_off(const char *format, ...)
{
    return 0;
}

#define ADT_DATA_DEBUG  0

void icsd_envnl_output(int result)
{
    printf("icsd_envnl_output:%d\n", result);
}

static u8 icsd_wind_lvl = 0;
void icsd_wind_output(u8 wind_lvl)
{
    wind_printf("%s:%d", wind_lvl);
    icsd_wind_lvl = wind_lvl;
    if (ICSD_WIND_EN && ICSD_WIND_MODE) {
        audio_acoustic_detector_output_hdl(0, icsd_wind_lvl, 0);
    }
}

void icsd_adt_run_output(__adt_result *adt_result)
{
    u8 voice_state = adt_result->voice_state;
    u8 wind_lvl    = icsd_wind_lvl;
    u8 wat_result  = adt_result->wat_result;
    u8 ein_state   = adt_result->ein_state;
    adt_printf("%s:%d, %d, %d, %d", voice_state, wind_lvl, wat_result, ein_state);
#if ADT_DATA_DEBUG
    adt_result->wind_lvl = icsd_wind_lvl;
    extern void icsd_adt_output_demo(__adt_result * adt_result);
    icsd_adt_output_demo(adt_result);
#else
    audio_acoustic_detector_output_hdl(voice_state, wind_lvl, wat_result);
#endif
}

#define AUDIO_ADC_CH0   BIT(0)
#define AUDIO_ADC_CH1   BIT(1)
#define AUDIO_ADC_CH2   BIT(2)
#define AUDIO_ADC_CH3   BIT(3)

void icsd_adt_mic_ch_analog_open(u32 ch)
{
    audio_adc_mic_mana_t *mic_param = audio_anc_mic_param_get();
    u8 ch_tmp = ch;
    u8 ch_num;
    //获取MIC序号;
    for (ch_num = 0; ch_num < AUDIO_ADC_MAX_NUM; ch_num++) {
        ch_tmp >>= 1;
        if (ch_tmp == 0) {
            break;
        }
    }
    //不需要打开ADC数字部分，走 ANC 开MIC流程
    audio_adc_mic_open(NULL, ch, &adc_hdl, &mic_param[ch_num].mic_p);
}

void icsd_adt_mic_ch_analog_close(u32 ch)
{
    audio_adc_mic_ch_analog_close(&adc_hdl, ch);
}

u8 talk_mic_analog_close = 0;
u8 audio_adt_talk_mic_analog_close()
{
    if (talk_mic_analog_close == 0) {
        printf("-----------audio_adt_talk_mic_analog_close\n");
        u8 talk_mic_ch = icsd_get_talk_mic_ch();
        icsd_adt_mic_ch_analog_close(talk_mic_ch);
        talk_mic_analog_close = 1;
        return 1;
    }
    return 0;
}

u8 audio_adt_talk_mic_analog_open()
{
    if (talk_mic_analog_close) {
        printf("-----------audio_adt_talk_mic_analog_open\n");
        u8 talk_mic_ch = icsd_get_talk_mic_ch();
        icsd_adt_mic_ch_analog_open(talk_mic_ch);
        talk_mic_analog_close = 0;
        return 1;
    }
    return 0;
}

int icsd_adt_get_mic_sr()
{
    wind_adc_sr = 8000;
    return wind_adc_sr;
}

int icsd_audio_dac_read(s16 points_offset, void *data, int len, u8 read_channel)
{
    return audio_dac_read_anc(points_offset, data, len, read_channel);
}

float adt_pow10(float n)
{
    float pow10n = exp_float((float)n / adt_log10_anc(exp_float((float)1.0)));
    return pow10n;
}

u32 icsd_adt_get_role()
{
    return tws_api_get_role();
}

u32 icsd_adt_get_tws_state()
{
    return tws_api_get_tws_state();
}

#define TWS_FUNC_ID_SDADT_M2S    TWS_FUNC_ID('A', 'D', 'T', 'M')
REGISTER_TWS_FUNC_STUB(icsd_adt_m2s) = {
    .func_id = TWS_FUNC_ID_SDADT_M2S,
    .func    = icsd_adt_m2s_cb,
};
#define TWS_FUNC_ID_SDADT_S2M    TWS_FUNC_ID('A', 'D', 'T', 'S')
REGISTER_TWS_FUNC_STUB(icsd_adt_s2m) = {
    .func_id = TWS_FUNC_ID_SDADT_S2M,
    .func    = icsd_adt_s2m_cb,
};

void icsd_adt_tws_m2s(u8 cmd)
{
    local_irq_disable();
    s8 data[8];
    icsd_adt_m2s_packet(data, cmd);
    int ret = tws_api_send_data_to_sibling(data, 8, TWS_FUNC_ID_SDADT_M2S);
    local_irq_enable();
}

void icsd_adt_tws_s2m(u8 cmd)
{
    local_irq_disable();
    s8 data[8];
    icsd_adt_s2m_packet(data, cmd);
    int ret = tws_api_send_data_to_sibling(data, 8, TWS_FUNC_ID_SDADT_S2M);
    local_irq_enable();
}

void icsd_adt_anc_dma_on(u8 out_sel, int *buf, int len)
{
    anc_dma_on_double(out_sel, buf, len);
}

static u8 icsd_adt_dma_done_flag = 0;
void icsd_adt_dma_done()
{
    if (icsd_adt_dma_done_flag) {
        icsd_acoustic_detector_ancdma_done();
    }
}

void set_icsd_adt_dma_done_flag(u8 flag)
{
    icsd_adt_dma_done_flag = flag;
}

void icsd_adt_hw_suspend()
{
    anc_core_dma_ie(0);
    anc_core_dma_stop();
}

void icsd_adt_hw_resume()
{
    audio_acoustic_detector_updata();
}

void icsd_adt_FFT_radix256(int *in_cur, int *out)
{
    u32 fft_config;
    fft_config = hw_fft_config(256, 8, 1, 0, 1);
    hw_fft_run(fft_config, in_cur, out);
}

void icsd_adt_FFT_radix128(int *in_cur, int *out)
{
    u32 fft_config;
    fft_config = hw_fft_config(128, 7, 1, 0, 1);
    hw_fft_run(fft_config, in_cur, out);
}

void icsd_adt_FFT_radix64(int *in_cur, int *out)
{
    u32 fft_config;
    fft_config = hw_fft_config(64, 6, 1, 0, 1);
    hw_fft_run(fft_config, in_cur, out);
}

/*********************** icsd adt task api ***********************/
static u8 icsd_anc_task = 0;
void icsd_post_anctask_msg(u8 cmd)
{
    icsd_anctask_cmd_check(cmd);
    int err = os_taskq_post_msg(ICSD_ANC_TASK_NAME, 2, ANC_MSG_ADT, cmd);
    if (err != OS_NO_ERR) {
        printf("%s err %d", __func__, err);
    }
}

void icsd_anc_process_task(void *priv)
{
    int res = 0;
    int msg[30];
    while (1) {
        res = os_taskq_pend(NULL, msg, ARRAY_SIZE(msg));
        if (res == OS_TASKQ) {
            switch (msg[1]) {
            case ANC_MSG_ADT:
                icsd_adt_anctask_handle((u8)msg[2]);
                break;
            }
        } else {
            printf("res:%d,%d", res, msg[1]);
        }
    }

}

void icsd_anc_task_create()
{
    if (icsd_anc_task == 0) {
        task_create(icsd_anc_process_task, NULL, ICSD_ANC_TASK_NAME);
        icsd_anc_task = 1;
    }
}

void icsd_anc_task_kill()
{
    if (icsd_anc_task) {
        task_kill(ICSD_ANC_TASK_NAME);
        icsd_anc_task = 0;
    }
}

/*********************** icsd adt task api ***********************/
static u8 adt_task = 0;
void icsd_post_adttask_msg(u8 cmd)
{
    int err = os_taskq_post_msg(ICSD_ADT_TASK_NAME, 1, cmd);
    if (err != OS_NO_ERR) {
        printf("%s err %d", __func__, err);
    }
}

void icsd_adt_task(void *priv)
{
    int res = 0;
    int msg[30];
    while (1) {
        res = os_taskq_pend(NULL, msg, ARRAY_SIZE(msg));
        if (res == OS_TASKQ) {
            icsd_adt_task_handler(msg[1]);
        }
    }
}

void icsd_adt_task_create()
{
    if (adt_task == 0) {
        task_create(icsd_adt_task, NULL, ICSD_ADT_TASK_NAME);
        adt_task = 1;
    }
}

void icsd_adt_task_kill()
{
    if (adt_task) {
        task_kill(ICSD_ADT_TASK_NAME);
        adt_task = 0;
    }
}

/*********************** icsd src task api ***********************/
static u8 src_task = 0;
void icsd_post_srctask_msg(u8 cmd)
{
    icsd_srctask_cmd_check(cmd);
    int err = os_taskq_post_msg(ICSD_SRC_TASK_NAME, 1, cmd);
    if (err != OS_NO_ERR) {
        printf("%s err %d", __func__, err);
    }
}

//ANC TASK 发送
//SRC_TASK_SUSPEND,
//ICSD_ADT_SUSPEND,
//ICSD_ADT_RESUME,
void icsd_post_srctask_msg_wait(u8 cmd)
{
    int err = os_taskq_post_msg("icsd_src", 1, cmd);
    if (err != OS_NO_ERR) {
        printf("%s err %d", __func__, err);
    }
}

void icsd_src_task(void *priv)
{
    int res = 0;
    int msg[30];
    while (1) {
        res = os_taskq_pend(NULL, msg, ARRAY_SIZE(msg));
        if (res == OS_TASKQ) {
            icsd_src_task_handler(msg[1]);
        }
    }
}

void icsd_src_task_create()
{
    if (src_task == 0) {
        task_create(icsd_src_task, NULL, ICSD_SRC_TASK_NAME);
        src_task = 1;
    }
}

void icsd_src_task_kill()
{
    if (src_task) {
        task_kill(ICSD_SRC_TASK_NAME);
        src_task = 0;
    }
}

void icsd_printf_debug_start()
{
    adt_printf = _adt_printf;
    wat_printf = _wat_printf;
    ein_printf = _ein_printf;
    wind_printf = _wind_printf;
    printf("icsd_printf_debug_start====================================\n");
}

void icsd_task_create()
{
    icsd_anc_task_create();
    icsd_adt_task_create();
    icsd_src_task_create();
    icsd_printf_debug_start();
}

void icsd_task_kill()
{
    icsd_anc_task_kill();
    icsd_adt_task_kill();
    icsd_src_task_kill();
}

void icsd_adt_init_pre()
{
    icsd_debug_adt_en();
    if ((ICSD_WIND_EN && (ICSD_WIND_MODE)) || ICSD_ENVNL_EN) {
        if (ICSD_WIND_3MIC) {
            ADT_PATH_CONFIG |= ADT_PATH_3M_EN;
        } else {
            ADT_PATH_CONFIG &= ~ADT_PATH_3M_EN;
        }
    }
}

#ifndef ADT_CLIENT_BOARD
const u8 ADT_EIN_VER  = ADT_EIN_TWS_V0;
void icsd_adt_ein_parm_HT03(__icsd_adt_parm *_ADT_PARM)
{
    _ADT_PARM->anc_in_pz_sum = -80;
    _ADT_PARM->anc_out_pz_sum = 0;
    _ADT_PARM->anc_in_sz_mean = 1;
    _ADT_PARM->anc_out_sz_mean = -12;
    _ADT_PARM->anc_in_fb_sum = 60;
    _ADT_PARM->anc_out_fb_sum = 30;

    _ADT_PARM->pnc_in_pz_sum = -80;
    _ADT_PARM->pnc_out_pz_sum = 0;
    _ADT_PARM->pnc_in_sz_mean = 1;
    _ADT_PARM->pnc_out_sz_mean = -12;
    _ADT_PARM->pnc_in_fb_sum = 60;
    _ADT_PARM->pnc_out_fb_sum = 30;

    _ADT_PARM->trans_in_pz_sum = 2;
    _ADT_PARM->trans_out_pz_sum = 26;
    _ADT_PARM->trans_in_sz_mean = 2;
    _ADT_PARM->trans_out_sz_mean = -7;
    _ADT_PARM->trans_in_fb_sum = 20;
    _ADT_PARM->trans_out_fb_sum = -20;
}

void icsd_adt_voice_board_data()
{
    __icsd_adt_parm *ADT_PARM;
    ADT_PARM = zalloc(sizeof(__icsd_adt_parm));
    extern const float HT03_ADT_FLOAT_DATA[];
    extern const float HT03_sz_out_data[28];
    extern const float HT03_pz_out_data[28];
    ADT_PARM->adt_float_data = (void *)HT03_ADT_FLOAT_DATA;
    ADT_PARM->sz_inptr = (float *)HT03_sz_out_data;
    ADT_PARM->pz_inptr = (float *)HT03_pz_out_data;
    icsd_adt_ein_parm_HT03(ADT_PARM);
    icsd_adt_parm_set(ADT_PARM);
    free(ADT_PARM);
}
#else
#include "icsd_adt_client_board.c"
#endif/*ADC_CLIENT_BOARD*/
//=======================================================================================================

#if ADT_DATA_DEBUG
void icsd_play_num0()
{
    icsd_adt_tone_play(ICSD_ADT_TONE_NUM0);
}
void icsd_play_num1()
{
    icsd_adt_tone_play(ICSD_ADT_TONE_NUM1);
}
void icsd_play_num2()
{
    icsd_adt_tone_play(ICSD_ADT_TONE_NUM2);
}
void icsd_play_num3()
{
    icsd_adt_tone_play(ICSD_ADT_TONE_NUM3);
}
void icsd_play_normal()
{
    icsd_adt_tone_play(ICSD_ADT_TONE_NORMAL);
}


extern u8 cepst_on;
void icsd_adt_output_demo(__adt_result *adt_result)
{
#if ADT_EAR_IN_EN
    static u8 hold = 0;
    static u8 hold_flg = 0;
    static int hold_time = 0;
    if (adt_result->ein_state == 0) {
        hold = 1;
        hold_flg = 1;
        hold_time = 0;
        printf("------------------------------------------------------------------ein state:0\n");
    } else {
        if (hold_flg) {
            hold_flg = 0;
            hold_time = jiffies;
            printf("set hold time:%d\n", hold_time);
        }
        if ((hold_time > 0) && (hold)) {
            if (jiffies > (hold_time + 100)) {
                printf("hold time out:%d\n", jiffies);
                hold = 0;
            }
        }
        printf("--------------------------------ein state:1\n");
    }
    if (hold) {
        //return;
    }
#endif

    if (adt_result->wind_lvl) {
        printf("------wind lvl:%d\n", adt_result->wind_lvl);
    }
    if (adt_result->voice_state) {
        icsd_play_normal();
    }
    switch (adt_result->wat_result) {
    case 2:
        icsd_play_num2();
        break;
    case 3:
        icsd_play_num3();
        break;
    default:
        break;
    }
}
#endif /*ADT_DATA_DEBUG*/


double adt_log10_anc(double x)
{
    double tmp = x;
    if (tmp == 0) {
        tmp = 0.0000001;
    }
    return log10_float(tmp);
}

float adt_anc_pow10(float n)
{
    float pow10n = exp_float((float)n / adt_log10_anc(exp_float((float)1.0)));
    return pow10n;
}

void adt_icsd_anc_fft256(int *in, int *out)
{
    u32 fft_config;
    fft_config = hw_fft_config(256, 8, 1, 0, 1);
    hw_fft_run(fft_config, in, out);
}



#endif /*(defined TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN) && TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/

