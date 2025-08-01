#include "audio_dac.h"
#include "audio_adc.h"
#include "media_config.h"
#include "power/power_manage.h"
#include "audio_volume_mixer.h"
#include "app_config.h"
#include "audio_demo/audio_demo.h"


static u8 audio_dac_idle_query()
{
    return audio_dac_is_idle();
}

static enum LOW_POWER_LEVEL audio_dac_level_query(void)
{
    /*根据dac的状态选择sleep等级*/
    if (config_audio_dac_power_off_lite) {
        /*anc打开，进入轻量级低功耗*/
        return LOW_POWER_MODE_LIGHT_SLEEP;
    } else {
        /*进入最优低功耗*/
        return LOW_POWER_MODE_SLEEP;
    }
}

REGISTER_LP_TARGET(audio_dac_lp_target) = {
    .name    = "audio_dac",
    .level   = audio_dac_level_query,
    .is_idle = audio_dac_idle_query,
};

void audio_fast_mode_test()
{
    printf("audio_fast_mode_test\n");
    audio_dac_set_volume(&dac_hdl, app_audio_get_volume(APP_AUDIO_CURRENT_STATE));
#if TCFG_DAC_NODE_ENABLE
    audio_dac_start(&dac_hdl);
    audio_adc_mic_demo_open(AUDIO_ADC_MIC_CH, 10, 16000, 1);
#endif

}

void dac_power_on(void)
{
#if TCFG_DAC_NODE_ENABLE
    audio_dac_open(&dac_hdl);
#endif
}

void dac_power_off(void)
{
#if TCFG_DAC_NODE_ENABLE
    audio_dac_close(&dac_hdl);
#endif
}

