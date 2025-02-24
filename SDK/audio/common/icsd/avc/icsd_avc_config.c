#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".icsd_adt.data.bss")
#pragma data_seg(".icsd_adt.data")
#pragma const_seg(".icsd_adt.text.const")
#pragma code_seg(".icsd_adt.text")
#endif

#include "app_config.h"

#if TCFG_AUDIO_ANC_ENABLE

#include "audio_config.h"
#include "icsd_avc.h"

struct avc_function *AVC_FUNC;
int (*avc_printf)(const char *format, ...) = _avc_printf;

void avc_config_init(__avc_config *_avc_config)
{
}

void avc_function_init()
{
    AVC_FUNC->avc_config_init = avc_config_init;
    AVC_FUNC->app_audio_get_volume = app_audio_get_volume;
}

#endif
