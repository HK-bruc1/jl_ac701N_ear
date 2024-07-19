#ifndef _AUDIO_HW_CROSSOVER_H_
#define _AUDIO_HW_CROSSOVER_H_

#include "effects/eq_config.h"
#include "effects/audio_eq.h"

void *audio_eq_coeff_open(void *coeff, u8 nsection, u32 sample_rate, u32 ch_num, u32 in_mode, u32 out_mode);
void audio_eq_coeff_run(void *hdl, void *indata, void *outdata, u32 indata_len);
void audio_eq_coeff_update(void *_hdl, void *coeff);
void audio_eq_coeff_close(void *_hdl);

#endif

