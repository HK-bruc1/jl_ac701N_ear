/*************************************************************************************************/
/*!
*  \file      le_audio_player.h
*
*  \brief
*
*  Copyright (c) 2011-2023 ZhuHai Jieli Technology Co.,Ltd.
*
*/
/*************************************************************************************************/
#ifndef _LE_AUDIO_PLAYER_H_
#define _LE_AUDIO_PLAYER_H_

#include "le_audio_stream.h"

int le_audio_player_open(u8 *conn, struct le_audio_stream_params *lea_param);

void le_audio_player_close(u8 *btaddr);

bool le_audio_player_is_playing(void);

int le_audio_set_dvol(u8 le_audio_num, u8 vol);

s16 le_audio_get_dvol(u8 le_audio_num);

void le_audio_dvol_up(u8 le_audio_num);

void le_audio_dvol_down(u8 le_audio_num);

#endif
