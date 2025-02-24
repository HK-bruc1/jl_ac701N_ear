#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".icsd_adt.data.bss")
#pragma data_seg(".icsd_adt.data")
#pragma const_seg(".icsd_adt.text.const")
#pragma code_seg(".icsd_adt.text")
#endif

#include "app_config.h"
#if ((defined TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN) && TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN && \
	 TCFG_AUDIO_ANC_ENABLE)
#include "icsd_wind.h"
#include "icsd_adt.h"

//====================风噪检测配置=====================
const u8 ICSD_WIND_PHONE_TYPE  = SDK_WIND_PHONE_TYPE;
const u8 ICSD_WIND_MIC_TYPE    = SDK_WIND_MIC_TYPE;
const u8 ICSD_WIND_ALG_BT_INF  = 1;
const u8 ICSD_WIND_DATA_BT_INF = 0;

struct wind_function *WIND_FUNC;
int (*win_printf)(const char *format, ...) = _win_printf;

void wind_config_init(__wind_config *_wind_config)
{
    __wind_config *wind_config = _wind_config;
    wind_config->msc_lp_thr    = 2.4;
    wind_config->msc_mp_thr    = 2.4;
    wind_config->corr_thr      = 0.4;//tws 不需要
    wind_config->cepst_1p_thr  = 20;
    wind_config->ref_pwr_thr   = 30;
    wind_config->wind_iir_alpha = 16;
    wind_config->wind_lvl_scale = 2;
    wind_config->icsd_wind_num_thr2 = 3;
    wind_config->icsd_wind_num_thr1 = 1;
}

void wind_function_init()
{
    WIND_FUNC->wind_config_init = wind_config_init;
    WIND_FUNC->HanningWin_pwr_float = icsd_HanningWin_pwr_float;
    WIND_FUNC->HanningWin_pwr_s1 = icsd_HanningWin_pwr_s1;
    WIND_FUNC->FFT_radix256 = icsd_FFT_radix256;
    WIND_FUNC->FFT_radix128 = icsd_FFT_radix128;
    WIND_FUNC->complex_mul  = icsd_complex_mul_v2;
    WIND_FUNC->complex_div  = icsd_complex_div_v2;
    WIND_FUNC->pxydivpxxpyy = icsd_pxydivpxxpyy;
    WIND_FUNC->log10_float  = icsd_log10_anc;
    WIND_FUNC->cal_score    = icsd_cal_score;
    WIND_FUNC->icsd_adt_tws_msync = icsd_adt_tws_msync;
    WIND_FUNC->icsd_adt_tws_ssync = icsd_adt_tws_ssync;
    WIND_FUNC->icsd_wind_run_part2_cmd = icsd_wind_run_part2_cmd;
    WIND_FUNC->icsd_adt_wind_part1_rx = icsd_adt_wind_part1_rx;
}

void icsd_wind_demo()
{
    static int *alloc_addr = NULL;
    struct icsd_win_libfmt libfmt;
    struct icsd_win_infmt  fmt;
    icsd_wind_get_libfmt(&libfmt);
    win_printf("WIND RAM SIZE:%d\n", libfmt.lib_alloc_size);
    if (alloc_addr == NULL) {
        alloc_addr = zalloc(libfmt.lib_alloc_size);
    }
    fmt.alloc_ptr = alloc_addr;
    icsd_wind_set_infmt(&fmt);
}

#endif/*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/
