#ifndef __AURACAST_SINK_API_H__
#define __AURACAST_SINK_API_H__

#include "typedef.h"

// max config
#define MAX_NUM_BIS 1
#define NO_PAST_MAX_BASS_NUM_SOURCES 5

typedef struct {
    u8 max_bis_num;
} auracast_sink_user_config_t;

enum {
    FRAME_DURATION_7_5,
    FRAME_DURATION_10,
};

typedef struct {
    uint8_t source_mac_addr[6];
    uint8_t broadcast_name[28];
    uint8_t Address_Type;
    uint8_t Advertising_SID;
    uint8_t feature;
    uint16_t con_handle;
    uint32_t broadcast_id;
    s16 sdu_period;
    s16 frame_len;
    uint8_t enc;
    int sample_rate;
    int bit_rate;
    uint8_t frame_duration;
    uint8_t BIG_Handle;
    uint8_t Num_BIS;
    uint16_t Connection_Handle[2];
} auracast_sink_source_info_t;

typedef enum {
    SINK_SCAN_STATE_IDLE = 0x00,
    SINK_SCAN_STATE_EXT_SCANNING,
    SINK_SCAN_STATE_EXT_REPORTED,
    SINK_SCAN_STATE_PADV_SCANNING,
    SINK_SCAN_STATE_PADV_REPORTED,
} auracast_sink_scan_state_t;

typedef enum {
    SINK_BIG_STATE_IDLE = 0x00,
    SINK_BIG_STATE_EXT_SCANNING,
    SINK_BIG_STATE_EXT_REPORTED,
    SINK_BIG_STATE_PADV_TRY_SYNC,
    SINK_BIG_STATE_PADV_SYNC,
    SINK_BIG_STATE_BIG_TRY_SYNC,
    SINK_BIG_STATE_BIG_SYNC,
} auracast_sink_big_state_t;

extern auracast_sink_scan_state_t auracast_sink_scan_state_get(void);
extern auracast_sink_big_state_t auracast_sink_big_state_get(void);
extern auracast_sink_source_info_t *auracast_sink_listening_source_info_get(void);

enum {
    AURACAST_SINK_SOURCE_INFO_REPORT_EVENT,
    AURACAST_SINK_BLE_CONNECT_EVENT,
    AURACAST_SINK_BIG_SYNC_CREATE_EVENT,
    AURACAST_SINK_BIG_SYNC_TERMINATE_EVENT,
    AURACAST_SINK_BIG_SYNC_LOST_EVENT,
    AURACAST_SINK_PERIODIC_ADVERTISING_SYNC_LOST_EVENT,
    AURACAST_SINK_BIG_INFO_REPORT_EVENT,
    AURACAST_SINK_ISO_RX_CALLBACK_EVENT,
    AURACAST_SINK_BIG_SYNC_FAIL_EVENT,
    AURACAST_SINK_EXT_SCAN_COMPLETE_EVENT,
};

typedef void (*auracast_sink_event_callback_t)(uint16_t event, uint8_t *packet, uint16_t length);

extern int auracast_sink_init(void);
extern int auracast_sink_uninit(void);
extern int auracast_sink_scan_start(void);
extern int auracast_sink_scan_stop(void);
extern int auracast_sink_big_sync_create(auracast_sink_source_info_t *param);
extern void auracast_sink_big_create(void);
extern void auracast_sink_rescan(void);
extern int auracast_sink_big_sync_terminate(void);
extern void auracast_sink_set_audio_state(u8 state);
extern void auracast_sink_set_broadcast_code(u8 *key);
extern void auracast_sink_set_source_filter(u8 state, u8 *mac);
extern void auracast_sink_set_scan_filter(u8 state, u8 filter_num, u8 *mac);
extern void auracast_sink_event_callback_register(auracast_sink_event_callback_t callback);

#endif /* __AURACAST_SINK_API_H__ */
