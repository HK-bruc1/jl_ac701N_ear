#ifndef TWS_A2DP_PLAY_H
#define TWS_A2DP_PLAY_H

extern u8 a2dp_avrcp_play_cmd_addr[6]; // 收到avrcp播放命令的蓝牙设备地址

void tws_a2dp_slience_detect(u8 *bt_addr, bool tx_do_action);

#endif
