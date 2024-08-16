#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".audio_general.data.bss")
#pragma data_seg(".audio_general.data")
#pragma const_seg(".audio_general.text.const")
#pragma code_seg(".audio_general.text")
#endif

#include "system/init.h"
#include "media/audio_general.h"
#include "app_config.h"
#include "media/audio_def.h"
#include "jlstream.h"
#include "uartPcmSender.h"
#include "debug/audio_debug.h"
#include "audio_config_def.h"

#if TCFG_USER_TWS_ENABLE
const int config_media_tws_en = 1;
#else
const int config_media_tws_en = 0;
#endif

/* 16bit数据流中也存在32bit位宽数据的处理 */
const int config_ch_adapter_32bit_enable = 1;
const int config_mixer_32bit_enable = 1;
const int config_jlstream_fade_32bit_enable = 1;
const int config_audio_eq_xfade_enable = 1;

#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_MONO_L)
const int config_audio_dac_channel_left_enable = 1;
const int config_audio_dac_channel_right_enable = 0;
#elif (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_MONO_R)
const int config_audio_dac_channel_left_enable = 0;
const int config_audio_dac_channel_right_enable = 1;
#else
const int config_audio_dac_channel_left_enable = 1;
const int config_audio_dac_channel_right_enable = 1;
#endif
const int config_audio_dac_power_on_mode = 0;
#ifdef TCFG_AUDIO_DAC_LIGHT_CLOSE_ENABLE
const int config_audio_dac_power_off_lite = TCFG_AUDIO_DAC_LIGHT_CLOSE_ENABLE;
#endif
#if TCFG_CFG_TOOL_ENABLE
const int config_audio_cfg_online_enable = 1;
#else
const int config_audio_cfg_online_enable = 0;
#endif


__attribute__((weak))
int get_system_stream_bit_width(void *par)
{
    return 0;
}
__attribute__((weak))
int get_media_stream_bit_width(void *par)
{
    return 0;
}
__attribute__((weak))
int get_esco_stream_bit_width(void *par)
{
    return 0;
}
__attribute__((weak))
int get_mic_eff_stream_bit_width(void *par)
{
    return 0;
}
__attribute__((weak))
int get_usb_audio_stream_bit_width(void *par)
{
    return 0;
}


static struct audio_general_params audio_general_param = {0};

struct audio_general_params *audio_general_get_param(void)
{
    return &audio_general_param;
}
/*
 *输出设备硬件位宽
 * */
int audio_general_out_dev_bit_width()
{
    if (audio_general_param.system_bit_width || audio_general_param.media_bit_width ||
        audio_general_param.esco_bit_width || audio_general_param.mic_bit_width
        || audio_general_param.usb_audio_bit_width) {
        return DATA_BIT_WIDE_24BIT;
    }
    return DATA_BIT_WIDE_16BIT;
}
/*
 *输入设备硬件位宽
 * */
int audio_general_in_dev_bit_width()
{
    if (audio_general_param.media_bit_width || audio_general_param.mic_bit_width) {
        return DATA_BIT_WIDE_24BIT;
    }
    return DATA_BIT_WIDE_16BIT;
}

int audio_general_init()
{
#if ((defined TCFG_AUDIO_DATA_EXPORT_DEFINE) && (TCFG_AUDIO_DATA_EXPORT_DEFINE == AUDIO_DATA_EXPORT_VIA_UART))
    uartSendInit();
#endif/*TCFG_AUDIO_DATA_EXPORT_DEFINE*/

#if TCFG_AUDIO_CONFIG_TRACE
    audio_config_trace_setup(TCFG_AUDIO_CONFIG_TRACE_INTERVAL);
#endif/*TCFG_AUDIO_CONFIG_TRACE*/

    struct stream_bit_width stream_par = {0};
    if (get_system_stream_bit_width(&stream_par)) {
        audio_general_param.system_bit_width = stream_par.bit_width;
    }

    if (get_media_stream_bit_width(&stream_par)) {
        audio_general_param.media_bit_width = stream_par.bit_width;
    }

    if (get_esco_stream_bit_width(&stream_par)) {
        audio_general_param.esco_bit_width = stream_par.bit_width;
    }

    if (get_mic_eff_stream_bit_width(&stream_par)) {
        audio_general_param.mic_bit_width = stream_par.bit_width;
    }

    if (get_usb_audio_stream_bit_width(&stream_par)) {
        audio_general_param.usb_audio_bit_width = stream_par.bit_width;
    }

#if defined(TCFG_AUDIO_GLOBAL_SAMPLE_RATE) &&TCFG_AUDIO_GLOBAL_SAMPLE_RATE
    audio_general_param.sample_rate = TCFG_AUDIO_GLOBAL_SAMPLE_RATE;
#endif
    return 0;
}
