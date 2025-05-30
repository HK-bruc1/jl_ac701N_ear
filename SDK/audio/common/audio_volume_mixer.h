#ifndef _AUD_VOLUME_MIXER_H_
#define _AUD_VOLUME_MIXER_H_

#include "generic/typedef.h"

struct volume_mixer {
    u16(*hw_dvol_max)(void);
};

void audio_volume_mixer_init(struct volume_mixer *param);

#endif
