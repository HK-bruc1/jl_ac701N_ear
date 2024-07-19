#ifndef CONFIG_APP_MUSIC_H
#define CONFIG_APP_MUSIC_H

#include " key_event_deal.h"

extern const u8 music_key_table[][KEY_EVENT_MAX];

void app_music_exit();
int music_app_check(void);

#endif

