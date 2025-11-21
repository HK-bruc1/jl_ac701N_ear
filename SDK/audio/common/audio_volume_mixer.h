#ifndef _AUD_VOLUME_MIXER_H_
#define _AUD_VOLUME_MIXER_H_

#include "generic/typedef.h"

struct volume_mixer {
    u16(*hw_dvol_max)(void);
};

void audio_volume_mixer_init(struct volume_mixer *param);

/*
*********************************************************************
*                  Audio Volume Get
* Description: 音量获取
* Arguments  : state	要获取音量值的音量状态
* Return	 : 返回指定状态对应得音量值
* Note(s)    : None.
*********************************************************************
*/
s16 app_audio_get_volume(u8 state);

#endif
