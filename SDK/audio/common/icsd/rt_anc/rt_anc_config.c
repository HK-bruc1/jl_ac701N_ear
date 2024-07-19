#include "audio_anc.h"

#if TCFG_AUDIO_ANC_ENABLE && ANC_REAL_TIME_ADAPTIVE_ENABLE
#include "rt_anc.h"

const u8 rt_anc_dma_ch_num = 4;

#endif
