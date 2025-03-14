#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".app_le_auracast.data.bss")
#pragma data_seg(".app_le_auracast.data")
#pragma const_seg(".app_le_auracast.text.const")
#pragma code_seg(".app_le_auracast.text")
#endif
#include "system/includes.h"
#include "app_config.h"
#include "btstack/avctp_user.h"
#include "app_tone.h"
#include "app_main.h"
#include "linein.h"
#include "wireless_trans.h"
#include "audio_config.h"
#include "a2dp_player.h"
#include "vol_sync.h"
#include "ble_rcsp_server.h"
#include "spdif_file.h"
#include "btstack/le/auracast_sink_api.h"
#include "btstack/le/auracast_delegator_api.h"
#include "le_audio_player.h"
#include "btstack/le/att.h"
#include "btstack/le/ble_api.h"
#include "app_le_auracast.h"
#include "esco_player.h"
#include "btstack/a2dp_media_codec.h"
#include "bt_slience_detect.h"
#include "user_cfg.h"
#if TCFG_USER_TWS_ENABLE
#include "classic/tws_api.h"
#endif

#if ((TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_AURACAST_SINK_EN | LE_AUDIO_JL_AURACAST_SINK_EN)))||((TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_AURACAST_SOURCE_EN | LE_AUDIO_JL_AURACAST_SOURCE_EN)))

/**************************************************************************************************
  Macros
**************************************************************************************************/
#define LOG_TAG             "[APP_AURACAST_SINK]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#define APP_MSG_KEY_AURACAST 4
#define MAX_AURACAST_NUM 3

static OS_MUTEX mutex;
static u8 app_auracast_init_flag = 0;

static struct broadcast_hdl *broadcast_hdl = NULL;
struct auracast_audio  g_rx_audio;
static u8 add_source_state = 0;
static u8 add_source_mac[6];


BROADCAST_STATUS le_auracast_state = 0;

u8 a2dp_auracast_preempted_addr[6];

struct broadcast_rx_audio_hdl {
    void *le_audio;
    void *rx_stream;
};

typedef struct {
    u16 bis_hdl;
    void *recorder;
    struct broadcast_rx_audio_hdl rx_player;
} bis_hdl_info_t ;

struct auracast_audio {
    uint8_t bis_num;
    bis_hdl_info_t audio_hdl[MAX_NUM_BIS];
};

struct broadcast_hdl {
    struct list_head entry;
    u8 del;
    u8 big_hdl;
    u8 status;
    bis_hdl_info_t bis_hdl_info[MAX_NUM_BIS];
    u32 big_sync_delay;
    const char *role_name;
};

struct broadcast_featrue_notify {
    uint8_t feature;
    uint8_t metadata_len;
    uint8_t metadata[20];
} __attribute__((packed));

struct broadcast_base_info_notify {
    uint8_t address_type;
    uint8_t address[6];
    uint8_t adv_sid;
    uint16_t pa_interval;
} __attribute__((packed));

struct broadcast_codec_info {
    uint8_t nch;
    u32 coding_type;
    s16 frame_len;
    s16 sdu_period;
    int sample_rate;
    int bit_rate;
} __attribute__((packed));

struct broadcast_source_endpoint_notify {
    uint8_t prd_delay[3];
    uint8_t num_subgroups;
    uint8_t num_bis;
    uint8_t codec_id[5];
    uint8_t codec_spec_length;
    uint8_t codec_spec_data[0];
    uint8_t metadata_length;
    uint8_t bis_data[0];
} __attribute__((packed));

typedef struct {
    uint8_t  save_auracast_addr[NO_PAST_MAX_BASS_NUM_SOURCES][6];
    uint8_t encryp_addr[NO_PAST_MAX_BASS_NUM_SOURCES][6];
    uint8_t  broadcast_name[28];
    uint32_t  broadcast_id;
    uint8_t enc;
    struct  broadcast_featrue_notify fea_data;
    struct broadcast_base_info_notify base_data;
    struct broadcast_codec_info codec_data;
} bass_no_past_source_t;

struct auracast_adv_info {
    uint16_t length;
    uint8_t flag;
    uint8_t op;
    uint8_t sn;
    //uint8_t seq;
    uint8_t data[0];
    uint8_t crc[0];
} __attribute__((packed));

static const struct conn_update_param_t con_param = {
    .interval_min = 166,
    .interval_max = 166,
    .latency = 2,
    .timeout = 500,
};

bass_no_past_source_t no_past_broadcast_sink_notify;
static u8 no_past_broadcast_num = 0;
static u8 encry_lock = 0;
static u16 auracast_scan_time = 0;

#if TCFG_USER_TWS_ENABLE
static auracast_delegator_info_t *g_tws_auracast_delegator = NULL;
void le_auracast_delegator_scan_stop_tws_sync(void);
void le_auracast_delegator_device_modify_tws_sync(void);
#endif

void auracast_sink_event_callback(uint16_t event, uint8_t *packet, uint16_t length);
void auracast_delegator_event_callback(uint16_t event, uint8_t *packet, uint16_t length);

u8 auracast_suspend_edr_a2dp()
{
    u8 bt_addr[6];
    if (a2dp_player_get_btaddr(bt_addr)) {
#if TCFG_A2DP_PREEMPTED_ENABLE
        memcpy(a2dp_auracast_preempted_addr, bt_addr, 6);
        a2dp_player_close(bt_addr);
        a2dp_media_mute(bt_addr);
        void *device = btstack_get_conn_device(bt_addr);
        if (device) {
            btstack_device_control(device, USER_CTRL_AVCTP_OPID_PAUSE);
        }
#else
        // 不抢播, 暂停auracast的播歌
        le_auracast_stop();
        return -1;
#endif
    }
    return 0;
}

void auracast_resume_edr_a2dp()
{
    if (a2dp_media_is_mute(a2dp_auracast_preempted_addr)) {
        bt_stop_a2dp_slience_detect(a2dp_auracast_preempted_addr);
        a2dp_media_unmute(a2dp_auracast_preempted_addr);
        a2dp_media_close(a2dp_auracast_preempted_addr);
        if (!a2dp_player_is_playing(a2dp_auracast_preempted_addr)) {
            void *device = btstack_get_conn_device(a2dp_auracast_preempted_addr);
            if (device) {
                btstack_device_control(device, USER_CTRL_AVCTP_OPID_PLAY);
            }
        }
        memset(a2dp_auracast_preempted_addr, 0xff, 6);
    }

}


static bool match_name(char *target_name, char *source_name, size_t target_len)
{
    if (strlen(target_name) == 0 || strlen(source_name) == 0) {
        return FALSE;
    }

    if (0 == memcmp(target_name, source_name, target_len)) {
        return TRUE;
    }

    return FALSE;
}

static inline void app_auracast_mutex_pend(OS_MUTEX *mutex, u32 line)
{
    if (!app_auracast_init_flag) {
        log_error("%s err, mutex uninit", __FUNCTION__);
        return;
    }
    int os_ret;
    os_ret = os_mutex_pend(mutex, 0);
    if (os_ret != OS_NO_ERR) {
        log_error("%s err, os_ret:0x%x", __FUNCTION__, os_ret);
        ASSERT(os_ret != OS_ERR_PEND_ISR, "line:%d err, os_ret:0x%x", line, os_ret);
    }
}

static inline void app_auracast_mutex_post(OS_MUTEX *mutex, u32 line)
{
    if (!app_auracast_init_flag) {
        log_error("%s err, mutex uninit", __FUNCTION__);
        return;
    }
    int os_ret;
    os_ret = os_mutex_post(mutex);
    if (os_ret != OS_NO_ERR) {
        log_error("%s err, os_ret:0x%x", __FUNCTION__, os_ret);
        ASSERT(os_ret != OS_ERR_PEND_ISR, "line:%d err, os_ret:0x%x", line, os_ret);
    }
}

static void le_auracast_sync_start(uint8_t *packet, uint16_t length)
{
    ASSERT(packet, "packet is NULL");
    app_auracast_mutex_pend(&mutex, __LINE__);
    auracast_sink_source_info_t _config = {0};
    memcpy(&_config, packet, sizeof(auracast_sink_source_info_t));
    auracast_sink_source_info_t *config = (auracast_sink_source_info_t *)&_config;//(auracast_sink_source_info_t *)packet;
    printf("sync create\n");

    put_buf(add_source_mac, 6);
    put_buf(config->source_mac_addr, 6);
    u8 status;
    if (add_source_state == DELEGATOR_SYNCHRONIZED_TO_PA && !encry_lock) {
        status = memcmp(config->source_mac_addr, add_source_mac, 6);
        if (!status) {
            auracast_sink_big_sync_create(config);
        }
    } else {
        /* for (u8 i = 0; i < NO_PAST_MAX_BASS_NUM_SOURCES; i++) { */
        /*     status = memcmp(no_past_broadcast_sink_notify.save_auracast_addr[i], config->source_mac_addr, 6); */
        /*     if (!status) { */
        /*         return; */
        /*     } */
        /* } */
        no_past_broadcast_num += 1;
        if (no_past_broadcast_num >= NO_PAST_MAX_BASS_NUM_SOURCES) {
            no_past_broadcast_num = 0;
        }
        memset(no_past_broadcast_sink_notify.broadcast_name, 0, sizeof(no_past_broadcast_sink_notify.broadcast_name));
        memcpy(no_past_broadcast_sink_notify.broadcast_name, config->broadcast_name, strlen((void *)config->broadcast_name));
        no_past_broadcast_sink_notify.broadcast_id = config->broadcast_id;
        no_past_broadcast_sink_notify.base_data.address_type = config->Address_Type;
        memcpy(no_past_broadcast_sink_notify.base_data.address, config->source_mac_addr, 6);
        memcpy(no_past_broadcast_sink_notify.save_auracast_addr[no_past_broadcast_num], config->source_mac_addr, 6);
        no_past_broadcast_sink_notify.base_data.adv_sid = config->Advertising_SID;
        no_past_broadcast_sink_notify.fea_data.feature = config->feature;
        printf("Advertising_SID[%d]Address_Type[%d]ADDR:\n", config->Advertising_SID, config->Address_Type);
        put_buf(config->source_mac_addr, 6);
        printf("auracast name:%s\n", config->broadcast_name);
        auracast_sink_big_sync_create(config);
    }
    app_auracast_mutex_post(&mutex, __LINE__);
}

static u8 make_auracast_ltv_data(u8 *buf, u8 data_type, u8 *data, u8 data_len)
{
    buf[0] = data_len + 1;
    buf[1] = data_type;
    memcpy(buf + 2, data, data_len);
    return data_len + 2;
}

static void le_auracast_big_info(uint8_t *packet, uint16_t length)
{
    ASSERT(packet, "packet is NULL");
    auracast_sink_source_info_t _config = {0};
    memcpy(&_config, packet, sizeof(auracast_sink_source_info_t));
    auracast_sink_source_info_t *config = (auracast_sink_source_info_t *)&_config;//(auracast_sink_source_info_t *)packet;
    if (add_source_state == DELEGATOR_SYNCHRONIZED_TO_PA) {
        auracast_sink_big_create();
    } else {
        no_past_broadcast_sink_notify.enc = config->enc;
        printf("%s\n", no_past_broadcast_sink_notify.broadcast_name);
        printf("%d\n", no_past_broadcast_sink_notify.base_data.address_type);
        put_buf(no_past_broadcast_sink_notify.base_data.address, 6);
        printf("%d\n", no_past_broadcast_sink_notify.base_data.adv_sid);
        printf("%d\n", no_past_broadcast_sink_notify.broadcast_id);
        printf("%d\n", no_past_broadcast_sink_notify.enc);
        auracast_delegator_notify_t notify;
        notify.att_send_len = 100;
        if (auracast_delegator_event_notify(DELEGATOR_ATT_CHECK_NOTIFY, (void *)&notify, sizeof(auracast_delegator_notify_t))) {
            u8 build_notify_data[200];
            u8 offset = 0;
            struct auracast_adv_info *info = (struct auracast_adv_info *)build_notify_data;
            info->flag = 1;
            info->op = 3;
            info->sn = 2;
            offset += 5;
            offset += make_auracast_ltv_data(&build_notify_data[offset], 0x1, no_past_broadcast_sink_notify.broadcast_name, strlen((void *)no_past_broadcast_sink_notify.broadcast_name));
            offset += make_auracast_ltv_data(&build_notify_data[offset], 0x2, (u8 *)&no_past_broadcast_sink_notify.broadcast_id, 3);
            struct broadcast_featrue_notify data;
            if (no_past_broadcast_sink_notify.enc) {
                data.feature = 0x7;
                memcpy(no_past_broadcast_sink_notify.encryp_addr[no_past_broadcast_num], no_past_broadcast_sink_notify.base_data.address, 6);
            } else {
                data.feature = 0x6;
            }
            data.metadata_len = 0;
            offset += make_auracast_ltv_data(&build_notify_data[offset], 0x3, (u8 *)&data, 2);
#if 1
            u8 ccc[100];
            struct broadcast_source_endpoint_notify *codec_data = (struct broadcast_source_endpoint_notify *)ccc;
            u8 codec_offset = 0;
            codec_data->prd_delay[0] = 0;
            codec_data->prd_delay[1] = 0;
            codec_data->prd_delay[2] = 0;
            codec_data->num_subgroups = 1;
            codec_data->num_bis = 1;
            u8 codec_id[5] = {0x6, 0x0, 0x0, 0x0, 0x0};
            memcpy(codec_data->codec_id, codec_id, 5);

            codec_offset += 11;
            u8 save_offset = codec_offset;
            u8 frequency = 0x8;
            codec_offset += make_auracast_ltv_data(&ccc[codec_offset], 0x1, &frequency, 1);
            u8 frame_duration = 0x1;
            codec_offset += make_auracast_ltv_data(&ccc[codec_offset], 0x2, &frame_duration, 1);
            u16 octets_frame = 0x0064;
            codec_offset += make_auracast_ltv_data(&ccc[codec_offset], 0x4, (u8 *)&octets_frame, 2);

            codec_data->codec_spec_length = codec_offset - save_offset;

            u8 meta_len = 0;
            ccc[codec_offset] = meta_len;
            codec_offset += 1;
            u8 bis_index = 1;
            ccc[codec_offset] = bis_index;
            codec_offset += 1;
            u8 bis_codec_len = 6;
            ccc[codec_offset] = bis_codec_len;
            codec_offset += 1;

            u8 bis_codec[4] = {0x1, 0x0, 0x0, 0x0};
            codec_offset += make_auracast_ltv_data(&ccc[codec_offset], 0x3, bis_codec, sizeof(bis_codec));

            put_buf(ccc, codec_offset);

            offset += make_auracast_ltv_data(&build_notify_data[offset], 0x4, ccc, codec_offset);
#else
            biginfo_notify_data[biginfo_size - 1] = 0;
            offset += make_auracast_ltv_data(&build_notify_data[offset], 0x4, biginfo_notify_data, biginfo_size);

            if (biginfo_notify_data) {
                free(biginfo_notify_data);
            }
#endif
            no_past_broadcast_sink_notify.base_data.pa_interval = 0xffff;
            offset += make_auracast_ltv_data(&build_notify_data[offset], 0x5, (u8 *)&no_past_broadcast_sink_notify.base_data, sizeof(struct broadcast_base_info_notify));
            info->length = offset;
            u16 crc = CRC16(build_notify_data, offset);
            memcpy(&build_notify_data[offset], &crc, 2);
            offset += 2;
            put_buf(build_notify_data, offset);

            for (u8 i = 0; i < 2; i++) {
                auracast_delegator_notify_t notify;
                notify.big_len = offset;
                notify.big_data = build_notify_data;
                auracast_delegator_event_notify(DELEGATOR_ATT_SEND_NOTIFY, (void *)&notify, sizeof(auracast_delegator_notify_t));
                mdelay(100);
            }
        }
        auracast_sink_set_scan_filter(1, no_past_broadcast_num, config->source_mac_addr);
        auracast_sink_rescan();
    }
}

static void le_auracast_att_init(uint8_t *packet, uint16_t length)
{
    auracast_sink_source_info_t *config = (auracast_sink_source_info_t *)packet;
    ASSERT(config, "config is NULL");
    printf("att init\n");
    auracast_delegator_notify_t notify;
    notify.con_handle = config->con_handle;
    auracast_delegator_event_notify(DELEGATOR_ATT_PROFILE_START_NOTIFY, (void *)&notify, sizeof(auracast_delegator_notify_t));
    ble_user_cmd_prepare(BLE_CMD_REQ_CONN_PARAM_UPDATE, 2, config->con_handle, &con_param);
}

static void app_auracast_init(void)
{
    if (app_auracast_init_flag) {
        return;
    }

    int os_ret = os_mutex_create(&mutex);
    if (os_ret != OS_NO_ERR) {
        log_error("%s %d err, os_ret:0x%x", __FUNCTION__, __LINE__, os_ret);
        ASSERT(0);
    }

    app_auracast_init_flag = 1;
}

static void le_auracast_audio_open(uint8_t *packet, uint16_t length)
{
    auracast_sink_source_info_t *config = (auracast_sink_source_info_t *)packet;
    ASSERT(config, "config is NULL");
    printf("audio open\n");
    //audio open
    if (config->Num_BIS > MAX_NUM_BIS) {
        g_rx_audio.bis_num = MAX_NUM_BIS;
    } else {
        g_rx_audio.bis_num = config->Num_BIS;
    }
    for (u8 i = 0; i < g_rx_audio.bis_num; i++) {
        g_rx_audio.audio_hdl[i].bis_hdl = config->Connection_Handle[i];
    }
    struct le_audio_stream_params params;
    params.fmt.nch = LE_AUDIO_CODEC_CHANNEL;
    params.fmt.coding_type = LE_AUDIO_CODEC_TYPE;
    params.fmt.frame_dms = LE_AUDIO_CODEC_FRAME_LEN;
    params.fmt.sdu_period = config->sdu_period;
    params.fmt.isoIntervalUs = config->sdu_period;
    params.fmt.sample_rate = config->sample_rate;
    params.fmt.dec_ch_mode = LEA_DEC_OUTPUT_CHANNEL;
    params.fmt.bit_rate = config->bit_rate;
    broadcast_hdl = (struct broadcast_hdl *)malloc(sizeof(struct broadcast_hdl));
    ASSERT(broadcast_hdl, "broadcast_hdl is NULL");
    broadcast_hdl->role_name = "big_rx";
    /* broadcast_hdl->status = BROADCAST_STATUS_START; */
    broadcast_hdl->big_hdl = config->BIG_Handle;
    int err;

    for (u8 i = 0; i < g_rx_audio.bis_num; i++) {
        broadcast_hdl->bis_hdl_info[i].bis_hdl = g_rx_audio.audio_hdl[i].bis_hdl;
        params.conn =  broadcast_hdl->bis_hdl_info[i].bis_hdl; //使用当前路的bis handle
        g_rx_audio.audio_hdl[i].rx_player.le_audio = le_audio_stream_create(params.conn, &params.fmt);// params.reference_time);
        g_rx_audio.audio_hdl[i].rx_player.rx_stream = le_audio_stream_rx_open(g_rx_audio.audio_hdl[i].rx_player.le_audio, params.fmt.coding_type);
        err = le_audio_player_open(g_rx_audio.audio_hdl[i].rx_player.le_audio, &params);
        if (err != 0) {
            ASSERT(0, "player open fail");
        }
    }
    le_auracast_state = BROADCAST_STATUS_START;
}

static void le_auracast_source_info_report(uint8_t *packet, uint16_t length)
{
    auracast_sink_source_info_t *config = (auracast_sink_source_info_t *)packet;
    ASSERT(config, "config is NULL");
    printf("sample_rate[%d]sdu_period[%d]bit_rate[%d]\n", config->sample_rate, config->sdu_period, config->bit_rate);
}

static void le_auracast_sync_terminate(void)
{
    printf("sync terminate\n");
    auracast_sink_big_sync_terminate();
}

static void le_auracast_iso_rx_callback(uint8_t *packet, uint16_t size)
{
    hci_iso_hdr_t hdr = {0};
    ll_iso_unpack_hdr(packet, &hdr);
    if ((hdr.pb_flag == 0b10) && (hdr.iso_sdu_length == 0)) {
        if (hdr.packet_status_flag == 0b00) {
            putchar('m');
            return;
            /* log_error("SDU empty"); */
        } else {
            putchar('s');
            return;
            /* log_error("SDU lost"); */
        }
    }
    if (((hdr.pb_flag == 0b10) || (hdr.pb_flag == 0b00)) && (hdr.packet_status_flag == 0b01)) {
        putchar('p');
        return;
        //log_error("SDU invalid, len=%d", hdr.iso_sdu_length);
    }

    for (u8 i = 0; i < g_rx_audio.bis_num; i++) {
        //printf("[%d][%d]",hdr.handle,g_rx_audio.audio_hdl[i].bis_hdl);
        //if (g_rx_audio.audio_hdl[i].bis_hdl == hdr.handle) {
        le_audio_stream_rx_frame(g_rx_audio.audio_hdl[i].rx_player.rx_stream, (void *)hdr.iso_sdu, hdr.iso_sdu_length, hdr.time_stamp);
        //}
    }
}

static void le_auracast_audio_close(void)
{
    printf("audio close\n");
    for (u8 i = 0; i < g_rx_audio.bis_num; i++) {
        if (g_rx_audio.audio_hdl[i].rx_player.le_audio && g_rx_audio.audio_hdl[i].rx_player.rx_stream) {
            le_audio_player_close(g_rx_audio.audio_hdl[i].rx_player.le_audio);
            le_audio_stream_rx_close(g_rx_audio.audio_hdl[i].rx_player.rx_stream);
            le_audio_stream_free(g_rx_audio.audio_hdl[i].rx_player.le_audio);
            g_rx_audio.audio_hdl[i].rx_player.le_audio = NULL;
            g_rx_audio.audio_hdl[i].rx_player.rx_stream = NULL;
        }
    }
    if (broadcast_hdl) {
        free(broadcast_hdl);
        broadcast_hdl = NULL;
    }
}

void le_auracast_stop(void)
{
    g_printf("le_auracast_stop\n");
    auracast_sink_set_audio_state(0);
    le_auracast_sync_terminate();
    le_auracast_audio_close();
    if (auracast_scan_time) {
        sys_timer_del(auracast_scan_time);
        auracast_scan_time = 0;
    }
#if TCFG_USER_TWS_ENABLE
    if (g_tws_auracast_delegator) {
        free(g_tws_auracast_delegator);
        g_tws_auracast_delegator = NULL;
    }
#endif
    auracast_sink_stop_scan();
    /* auracast_sink_uninit(); */
}

void auracast_scan_switch_priority(void *_sw)
{
    int sw = (int)_sw;
    auracast_scan_time = 0;
    u8 bt_addr[6];
    int timeout = 150;
    u8 a2dp_play = 0;
    if (a2dp_player_get_btaddr(bt_addr)) {
        a2dp_play = 1;
    }
    //r_printf("auracast_scan_switch=%d,%d\n", sw, timeout);

    //edr classic acl priority 30-11=19
    if (sw) {
        /* putchar('S'); */
        ble_op_ext_scan_set_priority(12);//30-12=18
    } else {
        /* putchar('s'); */
        ble_op_ext_scan_set_priority(8);//30-8=22
        timeout = a2dp_play ? 500 : 600;
    }
    sw = !sw;
    auracast_scan_time = sys_timeout_add((void *)sw, auracast_scan_switch_priority, timeout);
}

static void le_auracast_delegator_scan_start()
{
    r_printf("scan start \n");
    printf("auracast_delegator_adv_enable0 %s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
    auracast_delegator_adv_enable(0);
    bt_cmd_prepare(USER_CTRL_ALL_SNIFF_EXIT, 0, NULL);
    auracast_sink_rescan();
    le_auracast_state = BROADCAST_STATUS_SCAN_START;
    /* if (auracast_scan_time == 0) { */
    /*     auracast_scan_time = sys_timeout_add((void *)1, auracast_scan_switch_priority, 150); */
    /* } */
}

static void le_auracast_delegator_scan_stop()
{
    app_auracast_mutex_pend(&mutex, __LINE__);
    r_printf("scan stop \n");
    le_auracast_stop();
    le_auracast_state = BROADCAST_STATUS_SCAN_STOP;
    no_past_broadcast_num = 0;
    for (u8 i = 0; i < NO_PAST_MAX_BASS_NUM_SOURCES; i++) {
        memset(no_past_broadcast_sink_notify.save_auracast_addr[i], 0, 6);
        auracast_sink_set_source_filter(i, no_past_broadcast_sink_notify.save_auracast_addr[i]);
    }
    auracast_sink_set_source_filter(0, 0);
    auracast_sink_set_scan_filter(0, 0, 0);
    add_source_state = 0;
    app_auracast_mutex_post(&mutex, __LINE__);
}

static void le_auracast_delegator_device_modify()
{
    app_auracast_mutex_pend(&mutex, __LINE__);
    r_printf("device modify\n");
    auracast_sink_set_audio_state(0);
    le_auracast_sync_terminate();
    le_auracast_audio_close();
    auracast_sink_stop_scan();
    app_auracast_mutex_post(&mutex, __LINE__);
}

static void le_auracast_sync_lost(uint8_t *packet, uint16_t length)
{
    r_printf("scan restart \n");
    le_auracast_sync_terminate();
    le_auracast_audio_close();
    auracast_sink_rescan();
}

static void le_auracast_big_lost(uint8_t *packet, uint16_t length)
{
    r_printf("scan restart \n");
    le_auracast_sync_terminate();
    le_auracast_audio_close();
    auracast_sink_set_scan_filter(0, 0, 0);
    auracast_sink_rescan();
}

static void le_auracast_key_add(uint8_t *packet, uint16_t length)
{
    auracast_delegator_info_t *data = (auracast_delegator_info_t *)packet;
    ASSERT(data, "data is NULL");
    auracast_sink_set_broadcast_code(data->broadcast_code);
    put_buf(data->broadcast_code, 16);
    encry_lock = 0;
    auracast_sink_rescan();
}

static void le_auracast_device_add(uint8_t *packet, uint16_t length)
{
#if TCFG_USER_TWS_ENABLE
    printf("le_auracast_device_add delegator size:%lu, packet size:%d\n", sizeof(auracast_delegator_info_t), length);
    g_tws_auracast_delegator = (auracast_delegator_info_t *)packet;
#endif
    ASSERT(packet, "packet is NULL");
    app_auracast_mutex_pend(&mutex, __LINE__);
    auracast_delegator_info_t _data = {0};
    memcpy(&_data, packet, sizeof(auracast_delegator_info_t));
    auracast_delegator_info_t *data = (auracast_delegator_info_t *)&_data;//(auracast_delegator_info_t *)packet;
    u8 encry;
    for (u8 i = 0; i < NO_PAST_MAX_BASS_NUM_SOURCES; i++) {
        encry = memcmp(no_past_broadcast_sink_notify.encryp_addr[i], data->source_addr, 6);
        if (!encry) {
            break;
        }
    }
    if (!encry) {
        printf("auracast is encryption!!\n");
        encry_lock = 1;
    } else {
        printf("auracast is not encry!!\n");
        encry_lock = 0;
        auracast_sink_rescan();
    }

    auracast_delegator_notify_t notify;
    notify.encry = encry;
    notify.bass_source_id = data->bass_source_id;
    auracast_delegator_event_notify(DELEGATOR_BASS_ADD_SOURCE_NOTIFY, (void *)&notify, sizeof(auracast_delegator_notify_t));

    add_source_state = DELEGATOR_SYNCHRONIZED_TO_PA;
    memcpy(add_source_mac, data->source_addr, 6);
    auracast_sink_set_source_filter(1, data->source_addr);
    auracast_sink_set_scan_filter(0, 0, 0);

    app_auracast_mutex_post(&mutex, __LINE__);
}

static int le_auracast_app_msg_handler(int *msg)
{
    g_printf("le_auracast_app_msg_handler:%d\n", msg[0]);
    switch (msg[0]) {
    case APP_MSG_STATUS_INIT_OK:
        log_info("APP_MSG_STATUS_INIT_OK");
        printf("--earphone auracast mode\n");
        le_auracast_state = BROADCAST_STATUS_DEFAULT;
        app_auracast_init();
        auracast_sink_init();
        auracast_sink_event_callback_register(auracast_sink_event_callback);
        auracast_delegator_user_config_t auracast_delegator_user_parm;
        auracast_delegator_user_parm.adv_edr = 1;
        auracast_delegator_user_parm.adv_interval = 40;
        memcpy(&auracast_delegator_user_parm.device_name, (u8 *)bt_get_local_name(), LOCAL_NAME_LEN);
        auracast_delegator_user_parm.device_name_len = strlen(auracast_delegator_user_parm.device_name);
        auracast_delegator_config(&auracast_delegator_user_parm);
        auracast_delegator_event_callback_register(auracast_delegator_event_callback);
        printf("auracast_delegator_adv_enable1 %s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
        auracast_delegator_adv_enable(1);
        break;
    case APP_MSG_ENTER_MODE://1
        log_info("APP_MSG_ENTER_MODE");
        break;
    case APP_MSG_BT_GET_CONNECT_ADDR://1
        log_info("APP_MSG_BT_GET_CONNECT_ADDR");
        break;
    case APP_MSG_BT_OPEN_PAGE_SCAN://1
        log_info("APP_MSG_BT_OPEN_PAGE_SCAN");
        break;
    case APP_MSG_BT_CLOSE_PAGE_SCAN://1
        log_info("APP_MSG_BT_CLOSE_PAGE_SCAN");
        break;
    case APP_MSG_BT_ENTER_SNIFF:
        break;
    case APP_MSG_BT_EXIT_SNIFF:
        break;
    /* #if TCFG_USER_TWS_ENABLE */
    /*     case APP_MSG_TWS_PAIRED://1 */
    /*         log_info("APP_MSG_TWS_PAIRED"); */
    /*         break; */
    /*     case APP_MSG_TWS_UNPAIRED://1 */
    /*         log_info("APP_MSG_TWS_UNPAIRED"); */
    /*         break; */
    /*     case APP_MSG_TWS_PAIR_SUSS://1 */
    /*         log_info("APP_MSG_TWS_PAIR_SUSS"); */
    /*     case APP_MSG_TWS_CONNECTED://1 */
    /*         log_info("APP_MSG_TWS_CONNECTED"); */
    /*         break; */
    /* #endif */
    case APP_MSG_POWER_OFF://1
        log_info("APP_MSG_POWER_OFF");
        break;
    case APP_MSG_LE_AUDIO_MODE:
        r_printf("APP_MSG_LE_AUDIO_MODE=%d\n", msg[1]);
        break;
    case APP_MSG_KEY|APP_MSG_KEY_AURACAST:
        printf("auracast_switch\n");
        le_auracast_stop();
        break;
    case DELEGATOR_DEVICE_DISCONNECT_EVENT:
#if TCFG_USER_TWS_ENABLE
        if (!(tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED)) {
            le_auracast_delegator_scan_stop();
        } else {
            le_auracast_delegator_scan_stop_tws_sync();
        }
#else
        le_auracast_delegator_scan_stop();
#endif
        break;
    default:
        break;
    }
    return 0;
}

BROADCAST_STATUS le_auracast_status_get()
{
    printf("le_auracast_state=%d\n", le_auracast_state);
    return le_auracast_state;
}

void auracast_sink_event_callback(uint16_t event, uint8_t *packet, uint16_t length)
{
    // 这里监听后会一直打印
    /* printf("auracast_sink_event_callback:%d\n", event); */
    switch (event) {
    case AURACAST_SINK_SOURCE_INFO_REPORT_EVENT:
        printf("big AURACAST_SINK_SOURCE_INFO_REPORT_EVENT\n");
        le_auracast_sync_start(packet, length);
        break;
    case AURACAST_SINK_BLE_CONNECT_EVENT:
        printf("big AURACAST_SINK_BLE_CONNECT_EVENT\n");
        le_auracast_att_init(packet, length);
        break;
    case AURACAST_SINK_BIG_SYNC_CREATE_EVENT:
        printf("big AURACAST_SINK_BIG_SYNC_CREATE_EVENT\n");
        if (esco_player_runing()) {
            r_printf("AURACAST_SINK_BIG_SYNC_CREATE_EVENT esco_player_runing\n");
            // 暂停auracast的播歌
            le_auracast_stop();
            break;
        }
        if (auracast_scan_time) {
            sys_timer_del(auracast_scan_time);
            auracast_scan_time = 0;
        }
        if (auracast_suspend_edr_a2dp() != 0) {
            break;
        }
        le_auracast_audio_open(packet, length);
        break;
    case AURACAST_SINK_BIG_SYNC_TERMINATE_EVENT:
        printf("big terminate\n");
        break;
    case AURACAST_SINK_BIG_SYNC_FAIL_EVENT:
    case AURACAST_SINK_BIG_SYNC_LOST_EVENT:
        printf("big lost or fail\n");
        app_auracast_mutex_pend(&mutex, __LINE__);
        le_auracast_big_lost(packet, length);
        app_auracast_mutex_post(&mutex, __LINE__);
        break;
    case AURACAST_SINK_PERIODIC_ADVERTISING_SYNC_LOST_EVENT:
        printf("big periodic adv sync lost\n");
        app_auracast_mutex_pend(&mutex, __LINE__);
        le_auracast_sync_lost(NULL, 0);
        app_auracast_mutex_post(&mutex, __LINE__);
        memset(no_past_broadcast_sink_notify.save_auracast_addr[no_past_broadcast_num], 0, 6);
        break;
    case AURACAST_SINK_BIG_INFO_REPORT_EVENT:
        printf("big info report\n");
        le_auracast_big_info(packet, length);
        break;
    case AURACAST_SINK_ISO_RX_CALLBACK_EVENT:
        le_auracast_iso_rx_callback(packet, length);
        break;
    case AURACAST_SINK_EXT_SCAN_COMPLETE_EVENT:
        if (auracast_scan_time == 0) {
            auracast_scan_time = sys_timeout_add((void *)1, auracast_scan_switch_priority, 150);
        }
        break;
    default:
        break;
    }
}

void auracast_delegator_event_callback(uint16_t event, uint8_t *packet, uint16_t length)
{
    g_printf("auracast_delegator_event_callback:%d\n", event);
    switch (event) {
    case DELEGATOR_SCAN_START_EVENT:
        le_auracast_delegator_scan_start();
        break;
    case DELEGATOR_SCAN_STOP_EVENT:
#if TCFG_USER_TWS_ENABLE
        le_auracast_delegator_scan_stop_tws_sync();
#else
        le_auracast_delegator_scan_stop();
#endif
        break;
    case DELEGATOR_DEVICE_ADD_EVENT:
        r_printf("device add\n");
#if TCFG_USER_TWS_ENABLE
        if (!(tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED)) {
            le_auracast_device_add(packet, length);
        } else {
            tws_api_send_data_to_sibling(packet, length, 0x23482C5B);
        }
#else
        le_auracast_device_add(packet, length);
#endif
        break;
    case DELEGATOR_DEVICE_KEY_ADD_EVENT:
        app_auracast_mutex_pend(&mutex, __LINE__);
        r_printf("device key add\n");
        le_auracast_key_add(packet, length);
        app_auracast_mutex_post(&mutex, __LINE__);
        break;
    case DELEGATOR_DEVICE_MODIFY_EVENT:
#if TCFG_USER_TWS_ENABLE
        if (!(tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED)) {
            le_auracast_delegator_device_modify();
        } else {
            le_auracast_delegator_device_modify_tws_sync();
        }
#else
        le_auracast_delegator_device_modify();
#endif
        break;
    default:
        break;
    }
}

APP_MSG_HANDLER(le_auracast_app_msg_entry) = {
    .owner      = 0xff,
    .from       = MSG_FROM_APP,
    .handler    = le_auracast_app_msg_handler,
};


#if TCFG_USER_TWS_ENABLE

static void le_auracast_scan_stop_tws_sync_in_irq(void *_data, u16 len, bool rx)
{
    int argv[2];
    argv[0] = (int)le_auracast_delegator_scan_stop;
    argv[1] = 0;
    int ret = os_taskq_post_type("app_core", Q_CALLBACK, 2, argv);
    if (ret) {
        r_printf("le_auracast taskq post err %d!\n", __LINE__);
    }
}

REGISTER_TWS_FUNC_STUB(le_auracast_scan_stop_sync) = {
    .func_id = 0x23482C5A,
    .func = le_auracast_scan_stop_tws_sync_in_irq,
};

void le_auracast_delegator_scan_stop_tws_sync(void)
{
    tws_api_send_data_to_sibling(NULL, 0, 0x23482C5A);
}

static void le_auracast_device_modify_tws_sync_in_irq(void *_data, u16 len, bool rx)
{
    int argv[2];
    argv[0] = (int)le_auracast_delegator_device_modify;
    argv[1] = 0;
    int ret = os_taskq_post_type("app_core", Q_CALLBACK, 2, argv);
    if (ret) {
        r_printf("le_auracast taskq post err %d!\n", __LINE__);
    }
}

REGISTER_TWS_FUNC_STUB(le_auracast_device_modify_sync) = {
    .func_id = 0x23482C5D,
    .func = le_auracast_device_modify_tws_sync_in_irq,
};

void le_auracast_delegator_device_modify_tws_sync(void)
{
    tws_api_send_data_to_sibling(NULL, 0, 0x23482C5D);
}

static int le_auracast_tws_msg_handler(int *msg)
{
    struct tws_event *evt = (struct tws_event *)msg;
    u8 role = evt->args[0];
    u8 phone_link_connection = evt->args[1];
    u8 reason = evt->args[2];
    switch (evt->event) {
    case TWS_EVENT_CONNECTED:
        printf("le_auracast_tws_event_handler le_auracast role:%d, %d\n", role, tws_api_get_role());
        if ((tws_api_get_role() == TWS_ROLE_MASTER) && (g_tws_auracast_delegator == NULL)) {
            printf("auracast_delegator_adv_enable1 %s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
            auracast_delegator_adv_enable(1);
        } else {
            printf("auracast_delegator_adv_enable0 %s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
            auracast_delegator_adv_enable(0);
        }
        if ((role == TWS_ROLE_MASTER) && (g_tws_auracast_delegator != NULL)) {
            // 主机已经在监听播歌了，配对后让从机也执行相同操作
            tws_api_send_data_to_sibling((uint8_t *)g_tws_auracast_delegator, sizeof(auracast_delegator_info_t), 0x23482C5B);
        }
        break;
    case TWS_EVENT_PHONE_LINK_DETACH:
        break;
    case TWS_EVENT_CONNECTION_DETACH:
        printf("le_auracast_tws_msg_handler tws detach\n");
        if (g_tws_auracast_delegator == NULL) {
            printf("auracast no play\n ");
            auracast_delegator_adv_enable(1);
        } else {
            printf("auracast is playing\n ");
        }
        break;
    case TWS_EVENT_ROLE_SWITCH:
        printf("le_auracast_tws_event_handler le_auracast role switch:%d, %d\n", role, tws_api_get_role());
        if (g_tws_auracast_delegator == NULL) {
            printf("auracast no play\n ");
            if (tws_api_get_role() == TWS_ROLE_MASTER) {
                auracast_delegator_adv_enable(1);
            } else {
                auracast_delegator_adv_enable(0);
            }
        } else {
            printf("auracast is playing\n ");
        }
        if ((role == TWS_ROLE_SLAVE) && (le_auracast_state == BROADCAST_STATUS_SCAN_START)) {
            // 新从机如果是scan状态则需要关闭自己的scan，让新主机开启scan
            tws_api_send_data_to_sibling((void *)&le_auracast_state, sizeof(uint8_t), 0x23482C5C);
            le_auracast_delegator_scan_stop();
        }
        break;
    default:
        break;
    }
    return 0;
}

APP_MSG_HANDLER(le_auracast_tws_msg_entry) = {
    .owner      = 0xff,
    .from       = MSG_FROM_TWS,
    .handler    = le_auracast_tws_msg_handler,
};

static void le_auracast_device_add_tws_sync_in_irq(void *_data, u16 len, bool rx)
{
    printf("rx:%d\n", rx);
    /* printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
    /* put_buf(_data, len); */
    u8 *rx_data = malloc(len);
    if (rx_data == NULL) {
        return;
    }
    memcpy(rx_data, _data, len);

    int argv[4];
    argv[0] = (int)le_auracast_device_add;
    argv[1] = 2;
    argv[2] = (int)rx_data;
    argv[3] = (int)len;
    int ret = os_taskq_post_type("app_core", Q_CALLBACK, 4, argv);
    if (ret) {
        r_printf("le_auracast taskq post err %d!\n", __LINE__);
        free(rx_data);
    }
}

REGISTER_TWS_FUNC_STUB(le_auracast_device_add_sync) = {
    .func_id = 0x23482C5B,
    .func = le_auracast_device_add_tws_sync_in_irq,
};

static void le_auracast_scan_state_tws_sync_in_irq(void *_data, u16 len, bool rx)
{
    u8 *u8_data = (u8 *)_data;
    BROADCAST_STATUS t_le_auracast_state = (BROADCAST_STATUS) * u8_data;
    printf("le_auracast_scan_state_tws_sync_in_irq:%d\n", t_le_auracast_state);
    if ((tws_api_get_role() == TWS_ROLE_MASTER) && (t_le_auracast_state == BROADCAST_STATUS_SCAN_START)) {
        le_auracast_delegator_scan_start();
    }
}

REGISTER_TWS_FUNC_STUB(le_auracast_scan_state_sync) = {
    .func_id = 0x23482C5C,
    .func = le_auracast_scan_state_tws_sync_in_irq,
};

#endif

#endif

