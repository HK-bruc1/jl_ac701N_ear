
#include "mic_power_manager.h"
#include "gpio_config.h"
#include "asm/audio_adc.h"
#include "adc_file.h"

/*
 *MIC电源管理
 *设置mic vdd对应port的状态
 */
void audio_mic_pwr_ctl(audio_mic_pwr_t state)
{
    int i;
    struct adc_file_cfg *cfg = audio_adc_file_get_cfg();
    switch (state) {
    case MIC_PWR_OFF:
        if (audio_adc_is_active()) {
            printf("MIC_IO_PWR AUDIO_ADC is useing !\n");
            return;
        }
        printf("MIC_IO_PWR close\n");
    case MIC_PWR_INIT:
        /*mic供电IO配置：输出0*/
        for (i = 0; i < AUDIO_ADC_MIC_MAX_NUM; i++) {
            if (cfg->mic_en_map & BIT(i)) {
                if ((cfg->param[i].mic_bias_sel == 0) && (cfg->param[i].power_io != 0)) {
                    u32 gpio = uuid2gpio(cfg->param[i].power_io);
                    gpio_set_mode(IO_PORT_SPILT(gpio), PORT_OUTPUT_LOW);
                }
            }
        }

        /*
        //测试发现 少数MIC在开机的时候，需要将MIC_IN拉低，否则工作异常
        gpio_set_mode(IO_PORT_SPILT(gpio), PORT_OUTPUT_LOW);	//MIC0
        gpio_set_mode(IO_PORT_SPILT(gpio), PORT_OUTPUT_LOW);	//MIC1
        gpio_set_mode(IO_PORT_SPILT(gpio), PORT_OUTPUT_LOW);	//MIC2
        gpio_set_mode(IO_PORT_SPILT(gpio), PORT_OUTPUT_LOW);	//MIC3
         */
        break;

    case MIC_PWR_ON:
        /*mic供电IO配置：输出1*/
        for (i = 0; i < AUDIO_ADC_MIC_MAX_NUM; i++) {
            if (cfg->mic_en_map & BIT(i)) {
                if ((cfg->param[i].mic_bias_sel == 0) && (cfg->param[i].power_io != 0)) {
                    u32 gpio = uuid2gpio(cfg->param[i].power_io);
                    gpio_set_mode(IO_PORT_SPILT(gpio), PORT_OUTPUT_HIGH);
                }
            }
        }
        break;

    case MIC_PWR_DOWN:
        break;
    }
}

