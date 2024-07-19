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
    audio_config_trace_setup(3000);
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
