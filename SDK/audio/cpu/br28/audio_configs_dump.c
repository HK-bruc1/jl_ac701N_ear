#include "generic/typedef.h"
#include "audio_dac.h"
#include "audio_adc.h"

/*音频模块寄存器跟踪*/
void audio_adda_dump(void) //打印所有的dac,adc寄存器
{
    printf("JL_WL_AUD CON0:%x", JL_WL_AUD->CON0);
    printf("AUD_CON:%x", JL_AUDIO->AUD_CON);
    printf("DAC_CON:%x", JL_AUDIO->DAC_CON);
    printf("ADC_CON:%x", JL_AUDIO->ADC_CON);
    printf("ADC_CON1:%x", JL_AUDIO->ADC_CON1);
    printf("DAC_VL0:%x", JL_AUDIO->DAC_VL0);
    printf("DAC_TM0:%x", JL_AUDIO->DAC_TM0);
    printf("DAA_CON 0:%x 	1:%x,	2:%x,	3:%x    7:%x\n", JL_ADDA->DAA_CON0, JL_ADDA->DAA_CON1, JL_ADDA->DAA_CON2, JL_ADDA->DAA_CON3, JL_ADDA->DAA_CON7);
    printf("ADA_CON 0:%x	1:%x	2:%x	3:%x	4:%x	5:%x	6:%x	7:%x	8:%x\n", JL_ADDA->ADA_CON0, JL_ADDA->ADA_CON1, JL_ADDA->ADA_CON2, JL_ADDA->ADA_CON3, JL_ADDA->ADA_CON4, JL_ADDA->ADA_CON5, JL_ADDA->ADA_CON6, JL_ADDA->ADA_CON7, JL_ADDA->ADA_CON8);
}

/*音频模块配置跟踪*/
void audio_config_dump()
{
    u8 dac_bit_width = ((JL_AUDIO->DAC_CON & BIT(20)) ? 24 : 16);
    u8 adc_bit_width = ((JL_AUDIO->ADC_CON & BIT(20)) ? 24 : 16);
    int dac_dgain_max = 16384;
    int dac_again_max = 7;
    int mic_gain_max = 19;
    u8 dac_dcc = (JL_AUDIO->DAC_CON >> 12) & 0xF;
    u8 mic0_dcc = (JL_AUDIO->ADC_CON >> 12) & 0xF;
    u8 mic1_dcc = JL_AUDIO->ADC_CON1 & 0xF;
    u8 mic2_dcc = (JL_AUDIO->ADC_CON1 >> 4) & 0xF;
    u8 mic3_dcc = (JL_AUDIO->ADC_CON1 >> 8) & 0xF;

    u8 dac_again_l = JL_ADDA->DAA_CON1 & 0xF;
    u8 dac_again_r = (JL_ADDA->DAA_CON1 >> 4) & 0xF;
    u32 dac_dgain_l = JL_AUDIO->DAC_VL0 & 0xFFFF;
    u32 dac_dgain_r = (JL_AUDIO->DAC_VL0 >> 16) & 0xFFFF;
    u8 mic0_0_6 = JL_ADDA->ADA_CON4 & 0x1;
    u8 mic1_0_6 = JL_ADDA->ADA_CON5 & 0x1;
    u8 mic2_0_6 = JL_ADDA->ADA_CON6 & 0x1;
    u8 mic3_0_6 = JL_ADDA->ADA_CON7 & 0x1;
    u8 mic0_gain = JL_ADDA->ADA_CON8 & 0x1F;
    u8 mic1_gain = (JL_ADDA->ADA_CON8 >> 5) & 0x1F;
    u8 mic2_gain = (JL_ADDA->ADA_CON8 >> 10) & 0x1F;
    u8 mic3_gain = (JL_ADDA->ADA_CON8 >> 15) & 0x1F;
    int dac_sr = audio_dac_get_sample_rate_base_reg();
    int adc_sr = audio_adc_mic_get_sample_rate();


    printf("[ADC]BitWidth:%d,DCC:%d,%d,%d,%d,SR:%d\n", adc_bit_width, mic0_dcc, mic1_dcc, mic2_dcc, mic3_dcc, adc_sr);
    printf("[ADC]Gain(Max:%d):%d,%d,%d,%d,6dB_Boost:%d,%d,%d,%d,\n", mic_gain_max, mic0_gain, mic1_gain, mic2_gain, mic3_gain, \
           mic0_0_6, mic1_0_6, mic2_0_6, mic3_0_6);

    printf("[DAC]BitWidth:%d,DCC:%d,SR:%d\n", dac_bit_width, dac_dcc, dac_sr);
    printf("[DAC]AGain(Max:%d):%d,%d,DGain(Max:%d):%d,%d\n", dac_again_max, dac_again_l, dac_again_r, \
           dac_dgain_max, dac_dgain_l, dac_dgain_r);

    //short anc_gain = 0;//JL_ANC->CON5 & 0xFFFF;
    //printf("[ANC]Gain:%d\n",anc_gain);

}
