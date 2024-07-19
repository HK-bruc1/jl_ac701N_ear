#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".icsd_common.data.bss")
#pragma data_seg(".icsd_common.data")
#pragma const_seg(".icsd_common.text.const")
#pragma code_seg(".icsd_common.text")
#endif

#include "app_config.h"
#include "audio_config_def.h"

#if (TCFG_AUDIO_ANC_ENABLE && \
	 TCFG_AUDIO_ANC_EXT_VERSION == ANC_EXT_V2)

#include "icsd_common_v2.h"

#include "audio_anc.h"
#if ANC_CHIP_VERSION == ANC_VERSION_BR28
const u8 ICSD_ANC_CPU = ICSD_BR28;
#else /*ANC_CHIP_VERSION == ANC_VERSION_BR50*/
const u8 ICSD_ANC_CPU = ICSD_BR50;
#endif

float icsd_anc_pow10(float n)
{
    float pow10n = exp_float((float)n / icsd_log10_anc(exp_float((float)1.0)));
    return pow10n;
}

double icsd_sqrt_anc(double x)
{
    double tmp = x;
    if (tmp == 0) {
        tmp = 0.000001;//
    }
    return sqrt(tmp);
}
double icsd_log10_anc(double x)
{
    double tmp = x;
    if (tmp == 0) {
        tmp = 0.0000001;
    }
    return log10_float(tmp);
}
void icsd_anc_fft(int *in, int *out)
{
    u32 fft_config;
    fft_config = hw_fft_config(1024, 10, 1, 0, 1);
    hw_fft_run(fft_config, in, out);
}
void icsd_anc_fft256(int *in, int *out)
{
    u32 fft_config;
    fft_config = hw_fft_config(256, 8, 1, 0, 1);
    hw_fft_run(fft_config, in, out);
}
void icsd_anc_fft128(int *in_cur, int *out)
{
    u32 fft_config;
    fft_config = hw_fft_config(128, 7, 1, 0, 1);
    hw_fft_run(fft_config, in_cur, out);
}
#endif/*TCFG_AUDIO_ANC_ENABLE*/
