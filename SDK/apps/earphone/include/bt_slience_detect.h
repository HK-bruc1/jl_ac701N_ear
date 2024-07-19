#ifndef BT_SLIENCE_DETECT_H
#define BT_SLIENCE_DETECT_H

#include "generic/typedef.h"

enum {
    BT_SLIENCE_NO_DETECTING,
    BT_SLIENCE_NO_ENERGY,
    BT_SLIENCE_HAVE_ENERGY,
};
enum {
    SLIENCE_MODE_DROP_PACKET = 1,
    SLIENCE_MODE_A2DP_TIME_ENERGY,

};
#define SLIENCT_TIMER_CHECK  13//*80MS
void bt_start_esco_backup_a2dp_slience_detect(u8 *bt_addr, int mode, u8 a2dp_open_media_file);

void bt_start_a2dp_slience_detect(u8 *bt_addr, int ingore_packet_num);

void bt_stop_a2dp_slience_detect(u8 *bt_addr);

int bt_slience_detect_get_result(u8 *bt_addr);

void bt_reset_a2dp_slience_detect();

bool bt_a2dp_slience_detecting();

int bt_slience_get_detect_addr(u8 *bt_addr);

extern void bt_action_clr_drop_packet_flag(void *device, u8 *bt_addr);
extern void bt_action_clr_energy_flag(void *device, u8 *bt_addr);
extern void bt_action_a2dp_play(void *device, u8 *bt_addr);
extern int bt_action_is_drop_flag(void *device, u8 *bt_addr);
extern int bt_action_is_energy_flag(void *device, u8 *bt_addr);

#endif
