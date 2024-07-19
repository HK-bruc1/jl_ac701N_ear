#ifndef _LINEIN_FILE_H_
#define _LINEIN_FILE_H_

#include "generic/typedef.h"
#include "media/includes.h"
#include "app_config.h"

struct linein_platform_cfg {
    u8 linein_mode;
    u8 linein_gain;
    u8 linein_pre_gain;   // 0:0dB   1:6dB
    u8 linein_ain_sel;    // 0/1/2
    u8 linein_dcc;        // DCC level
};

struct linein_file_hdl {
    void *source_node;
    u8 start;
    u8 dump_cnt;
    u8 ch_num;
    u8 mute_en;
    u16 sample_rate;
    u16 irq_points;
    struct adc_linein_ch linein_ch;
    struct audio_adc_output_hdl adc_output;
    s16 *adc_buf;
    u8 adc_seq;
    u16 output_fade_in_gain;
    u8 output_fade_in;
};

void audio_linein_file_init();
int adc_file_linein_open(struct adc_linein_ch *linein, int ch);


#endif // #ifndef _ADC_FILE_H_
