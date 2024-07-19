#ifndef DEVICE_IRKEY_H
#define DEVICE_IRKEY_H

#include "typedef.h"


#ifndef CONFIG_IRKEY_MAX_NUM
#define CONFIG_IRKEY_MAX_NUM  35
#endif

struct ff00_2_keynum {
    u8 source_value;
    u8 key_value;
};

struct irkey_platform_data {
    u8 enable;
    u8 port;
    u8 num;
    struct ff00_2_keynum IRff00_2_keynum[CONFIG_IRKEY_MAX_NUM];
};

u8 ir_get_key_value(void);
int irkey_init(void);

int get_irkey_io();
#endif

