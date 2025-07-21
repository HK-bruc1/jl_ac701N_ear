/*********************************************************************************************
 *   Filename        : connected.c

 *   Description     :

 *   Author          : Weixin Liang

 *   Email           : liangweixin@zh-jieli.com

 *   Last modifiled  : 2022-12-15 14:26

 *   Copyright:(c)JIELI  2011-2022  @ , All Rights Reserved.
 *********************************************************************************************/
#include "app_config.h"
#include "system/includes.h"
#include "btstack/avctp_user.h"
#include "app_msg.h"
#include "cig.h"
#include "le_connected.h"
#include "wireless_trans_manager.h"
#include "wireless_trans.h"
#include "clock_manager/clock_manager.h"
#include "le_audio_stream.h"
#include "le_audio_player.h"
#include "le_audio_recorder.h"
#include "btstack/le/ble_api.h"

#if ((TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_UNICAST_SINK_EN | LE_AUDIO_JL_UNICAST_SINK_EN)))

#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_MONO_L) || (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_MONO_R)
#define UNICAST_SINK_CIS_NUMS               (1)			// 单声道
#else
#define UNICAST_SINK_CIS_NUMS               (2)			// 立体声
#endif

/**************************************************************************************************
  Macros
 **************************************************************************************************/
#define LOG_TAG             "[LE_CONNECTED]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

/*! \brief CIS丢包修复 */
#define CIS_AUDIO_PLC_ENABLE    1

#define connected_get_cis_tick_time(cis_txsync)  wireless_trans_get_last_tx_clk((connected_role & CONNECTED_ROLE_PERIP) == CONNECTED_ROLE_PERIP ? "cig_perip" : "cig_central", (void *)(cis_txsync))

/**************************************************************************************************
  Data Types
 **************************************************************************************************/

/*! \brief 广播状态枚举 */
enum {
    CONNECTED_STATUS_STOP,      /*!< 广播停止 */
    CONNECTED_STATUS_STOPPING,  /*!< 广播正在停止 */
    CONNECTED_STATUS_OPEN,      /*!< 广播开启 */
    CONNECTED_STATUS_START,     /*!< 广播启动 */
};

typedef struct {
    u32 isoIntervalUs;
    u16 cis_hdl;								// cis连接句柄
    u16 acl_hdl;								// acl连接句柄
    u16 Max_PDU_P_To_C;
    u16 Max_PDU_C_To_P;
    u8 flush_timeout;
    u8 BN_C_To_P;								// 一个iso interval里面有多少个包
    u8 BN_P_To_C;								// 一个iso interval里面有多少个包
} cis_hdl_info_t;

/*! \brief 广播结构体 */
struct connected_hdl {
    struct list_head entry; /*!< cig链表项，用于多cig管理 */
    cis_hdl_info_t cis_hdl_info[CIG_MAX_CIS_NUMS];	// cis句柄
    struct connected_rx_audio_hdl rx_player;		// 解码器
    u32 cig_sync_delay;
    const char *role_name;							// 同步名称
    void *recorder;									// 编码器
    u8 del;											// 是否已删除
    u8 cig_hdl;										// cig句柄
    u8 rx_cis_num;									// rx cis的数量
};

struct acl_recv_hdl {
    struct list_head entry;
    void (*recv_handle)(u16 conn_handle, const void *const buf, size_t length, void *priv);
};
static u16 multi_cis_rx_temp_buf_len = 0;
static u8 *multi_cis_rx_buf = NULL;
static u16 multi_cis_data_offect = {0};
static bool multi_cis_plc_flag = false;
static bool rcsp_update_flag  = false;

/**************************************************************************************************
  Static Prototypes
 **************************************************************************************************/
static int connected_dec_data_receive_handler(void *_hdl, void *data, int len);
static void connected_iso_callback(const void *const buf, size_t length, void *priv);
static void connected_perip_event_callback(const CIG_EVENT event, void *priv);
static void connected_free_cig_hdl(u8 cig_hdl);
extern void set_aclMaxPduPToC_test(uint8_t aclMaxPduPToC);

/**************************************************************************************************
  Global Variables
 **************************************************************************************************/
static DEFINE_SPINLOCK(connected_lock);
static OS_MUTEX connected_mutex;
static u8 connected_role;   /*!< 记录当前广播为接收端还是发送端 */
static u8 connected_init_flag;  /*!< 广播初始化标志 */
static u8 g_cig_hdl;        /*!< 用于cig_hdl获取 */
static u8 connected_num;    /*!< 记录当前开启了多少个cig广播 */
static u8 *transmit_buf;    /*!< 用于发送端发数 */
static struct list_head connected_list_head = LIST_HEAD_INIT(connected_list_head);	// cig链表
static struct list_head acl_data_recv_list_head = LIST_HEAD_INIT(acl_data_recv_list_head);
static struct le_audio_mode_ops *le_audio_switch_ops = NULL; /*!< 广播音频和本地音频切换回调接口指针 */
#if (TCFG_LE_AUDIO_APP_CONFIG & LE_AUDIO_JL_UNICAST_SINK_EN)
u8 cig_peripheral_support_lea_profile  = 0;			// 是否支持公有的cis协议
u8 cig_peripheral_support_dongle = 1;
#elif (TCFG_LE_AUDIO_APP_CONFIG & LE_AUDIO_UNICAST_SINK_EN )
u8 cig_peripheral_support_lea_profile  = 1;			// 是否支持公有的cis协议
u8 cig_peripheral_support_dongle = 0;
#endif
const cig_callback_t cig_perip_cb = {
    .receive_packet_cb      = connected_iso_callback,
    .event_cb               = connected_perip_event_callback,
};

#if CIS_AUDIO_PLC_ENABLE
//丢包修复补包 解码读到 这两个byte 才做丢包处理
static unsigned char errpacket[2] = {
    0x02, 0x00
};
#endif /*CIS_AUDIO_PLC_ENABLE*/

/**************************************************************************************************
  Function Declarations
 **************************************************************************************************/
static inline void connected_mutex_pend(OS_MUTEX *mutex, u32 line)
{
    int os_ret;
    os_ret = os_mutex_pend(mutex, 0);
    if (os_ret != OS_NO_ERR) {
        log_error("%s err, os_ret:0x%x", __FUNCTION__, os_ret);
        ASSERT(os_ret != OS_ERR_PEND_ISR, "line:%d err, os_ret:0x%x", line, os_ret);
    }
}

static inline void connected_mutex_post(OS_MUTEX *mutex, u32 line)
{
    int os_ret;
    os_ret = os_mutex_post(mutex);
    if (os_ret != OS_NO_ERR) {
        log_error("%s err, os_ret:0x%x", __FUNCTION__, os_ret);
        ASSERT(os_ret != OS_ERR_PEND_ISR, "line:%d err, os_ret:0x%x", line, os_ret);
    }
}

/**
 * @brief cig事件发送到用户线程
 *
 * @param event 事件类型
 * @param value 事件消息
 * @param len 	事件消息长度
 */
int cig_event_to_user(int event, void *value, u32 len)
{
    int *evt = zalloc(sizeof(int) + len);
    ASSERT(evt);
    evt[0] = event;
    memcpy(&evt[1], value, len);
    app_send_message_from(MSG_FROM_CIG, sizeof(int) + len, (int *)evt);
    free(evt);
    return 0;
}

/**
 *	@brief 获取cig句柄
 *
 *	@param id cig句柄id
 *	@param head cig链表头
 */
static u16 get_available_cig_hdl(u8 id, struct list_head *head)
{
    struct connected_hdl *p;
    u8 hdl = id;
    if ((hdl == 0) || (hdl > 0xEF)) {
        hdl = 1;
        g_cig_hdl = 1;
    }

    connected_mutex_pend(&connected_mutex, __LINE__);
__again:
    list_for_each_entry(p, head, entry) {
        if (hdl == p->cig_hdl) {
            hdl++;
            goto __again;
        }
    }

    if (hdl > 0xEF) {
        hdl = 0;
    }

    if (hdl == 0) {
        hdl++;
        goto __again;
    }

    g_cig_hdl = hdl;
    connected_mutex_post(&connected_mutex, __LINE__);
    return hdl;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 初始化CIG所需的参数及流程
 *
 */
/* ----------------------------------------------------------------------------*/
void connected_init(void)
{
    log_info("--func=%s", __FUNCTION__);
    int ret;

    if (connected_init_flag) {
        return;
    }

    int os_ret = os_mutex_create(&connected_mutex);
    if (os_ret != OS_NO_ERR) {
        log_error("%s %d err, os_ret:0x%x", __FUNCTION__, __LINE__, os_ret);
        ASSERT(0);
    }

    connected_init_flag = 1;

    //初始化cis接收参数及注册回调
    ret = wireless_trans_init("cig_perip", NULL);
    if (ret != 0) {
        log_error("wireless_trans_uninit fail:0x%x\n", ret);
        connected_init_flag = 0;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 复位CIG所需的参数及流程
 *
 */
/* ----------------------------------------------------------------------------*/
void connected_uninit(void)
{
    log_info("--func=%s", __FUNCTION__);
    int ret;

    if (!connected_init_flag) {
        return;
    }

    int os_ret = os_mutex_del(&connected_mutex, OS_DEL_NO_PEND);
    if (os_ret != OS_NO_ERR) {
        log_error("%s %d err, os_ret:0x%x", __FUNCTION__, __LINE__, os_ret);
    }

    connected_init_flag = 0;

    ret = wireless_trans_uninit("cig_perip", NULL);
    if (ret != 0) {
        log_error("wireless_trans_uninit fail:0x%x\n", ret);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief CIG从机连接成功处理事件
 *
 * @param priv:连接成功附带的句柄参数
 *
 * @return 是否执行成功 -- 0为成功，其他为失败
 */
/* ----------------------------------------------------------------------------*/
int connected_perip_connect_deal(void *priv)
{
    u8 i;
    u8 index = 0;
    u8 find = 0;
    struct connected_hdl *connected_hdl = 0;
    cig_hdl_t *hdl = (cig_hdl_t *)priv;
    struct le_audio_stream_params params = {0};


    r_printf("connected_perip_connect_deal");
    log_info("cig_hdl:%d, acl_hdl:%d, cis_hdl:%d, Max_PDU_C_To_P:%d, Max_PDU_P_To_C:%d, BN_C_To_P:%d, BN_P_To_C:%d\n", hdl->cig_hdl, hdl->acl_hdl, hdl->cis_hdl, hdl->Max_PDU_C_To_P, hdl->Max_PDU_P_To_C, hdl->BN_C_To_P, hdl->BN_P_To_C);

    //真正连上设备后，清除BIT(7)，使外部跑转发流程
    connected_role &= ~BIT(7);

    connected_mutex_pend(&connected_mutex, __LINE__);
    list_for_each_entry(connected_hdl, &connected_list_head, entry) {
        if (connected_hdl->cig_hdl == hdl->cig_hdl) {
            find = 1;
            break;
        }
    }
    connected_mutex_post(&connected_mutex, __LINE__);

    if (!find) {
        connected_hdl = (struct connected_hdl *)zalloc(sizeof(struct connected_hdl));
        ASSERT(connected_hdl, "connected_hdl is NULL");
        connected_hdl->role_name = "cig_perip";
        connected_hdl->cig_hdl = hdl->cig_hdl;
        if (!connected_hdl->cig_sync_delay) {
            connected_hdl->cig_sync_delay = hdl->cig_sync_delay;
        }
    }

    for (i = 0; i < CIG_MAX_CIS_NUMS; i++) {
        if (!connected_hdl->cis_hdl_info[i].cis_hdl) {
            index = i;
            break;
        }
    }

    if (hdl->Max_PDU_C_To_P) {
        connected_hdl->rx_cis_num += 1;
    }
    connected_hdl->cis_hdl_info[index].acl_hdl = hdl->acl_hdl;
    connected_hdl->cis_hdl_info[index].cis_hdl = hdl->cis_hdl;
    connected_hdl->cis_hdl_info[index].flush_timeout = hdl->flush_timeout_C_to_P;
    connected_hdl->cis_hdl_info[index].isoIntervalUs = hdl->isoIntervalUs;
    connected_hdl->cis_hdl_info[index].BN_C_To_P = hdl->BN_C_To_P;
    connected_hdl->cis_hdl_info[index].BN_P_To_C = hdl->BN_P_To_C;
    connected_hdl->cis_hdl_info[index].Max_PDU_P_To_C = hdl->Max_PDU_P_To_C;
    connected_hdl->cis_hdl_info[index].Max_PDU_C_To_P = hdl->Max_PDU_C_To_P;


    log_info("rx_cis_num:%d, hdl->flush_timeout:%d, hdl->isoIntervalUs:%d\n", connected_hdl->rx_cis_num, hdl->flush_timeout_C_to_P, hdl->isoIntervalUs);


    if (get_le_audio_jl_dongle_device_type()) {
        get_decoder_params_fmt(&(params.fmt));
    } else {
        params.fmt.nch = UNICAST_SINK_CIS_NUMS;
        params.fmt.bit_rate = get_cig_audio_coding_bit_rate() * UNICAST_SINK_CIS_NUMS;
        params.fmt.frame_dms = get_cig_audio_coding_frame_duration();
        params.fmt.sdu_period = get_cig_sdu_period_us();
        params.fmt.sample_rate = get_cig_audio_coding_sample_rate();
    }
    params.fmt.coding_type = LE_AUDIO_CODEC_TYPE;
    params.fmt.dec_ch_mode = LEA_DEC_OUTPUT_CHANNEL;
    params.fmt.flush_timeout = hdl->flush_timeout_C_to_P;
    params.fmt.isoIntervalUs = hdl->isoIntervalUs;

    params.latency = 50 * 1000;//tx延时暂时先设置 50ms
    params.conn = hdl->cis_hdl;//使用当前路的cis_hdl
    if (get_le_audio_jl_dongle_device_type()) {
        params.service_type = LEA_SERVICE_MEDIA;
    } else {
        params.service_type = (hdl->Max_PDU_P_To_C) ? LEA_SERVICE_CALL : LEA_SERVICE_MEDIA;
    }

    g_printf("nch:%d, coding_type:0x%x, dec_ch_mode:%d, conn:%d, dms:%d, sdu_period:%d, br:%d\n",
             params.fmt.nch, params.fmt.coding_type, params.fmt.dec_ch_mode, params.conn, params.fmt.frame_dms, params.fmt.sdu_period, params.fmt.bit_rate);

    if (connected_hdl->rx_cis_num > 1) {
        if (multi_cis_rx_buf) {
            free(multi_cis_rx_buf);
            multi_cis_rx_buf = NULL;
        }
        if (!multi_cis_rx_buf) {
            multi_cis_rx_temp_buf_len = params.fmt.bit_rate * params.fmt.frame_dms / 8 / 1000 / 10;
            multi_cis_rx_buf = zalloc(multi_cis_rx_temp_buf_len);
            printf("multi_cis_rx_temp_buf_len:%d, BN_C_To_P:%d, 0x%x\n", multi_cis_rx_temp_buf_len, hdl->BN_C_To_P, (unsigned int)multi_cis_rx_buf);
            ASSERT(multi_cis_rx_buf, "multi_cis_rx_buf is NULL");
        }
    }

    if (!le_audio_player_is_playing()) {

        le_audio_switch_ops = get_connected_audio_sw_ops();

        //关闭本地音频播放
        if (le_audio_switch_ops && le_audio_switch_ops->local_audio_close) {
            le_audio_switch_ops->local_audio_close();
        }

        //TODO:开启播放器播放远端传来的音频
        if (le_audio_switch_ops && le_audio_switch_ops->rx_le_audio_open) {
            le_audio_switch_ops->rx_le_audio_open(&connected_hdl->rx_player, &params);
        }
    }

    if (!le_audio_mic_recorder_is_running()) {
        //phone call
        if (!get_le_audio_jl_dongle_device_type() && hdl->Max_PDU_P_To_C) {
            //发数中间临时缓存buffer
            if (transmit_buf == NULL) {
                transmit_buf = zalloc(get_cig_transmit_data_len() * hdl->BN_P_To_C);    //by haibin--0624
                ASSERT(transmit_buf, "transmit_buf is NULL");
            }
            params.service_type = LEA_SERVICE_CALL;
            if (!connected_hdl->recorder) {
#if TCFG_AUDIO_BIT_WIDTH
                params.latency = 80 * 1000;//tx延时暂时先设置 80ms
#else
                params.latency = 80 * 1000;//tx延时暂时先设置 50ms：三星手机单mic+ANS设置50ms会有上行无声的问题，这里改成80ms
#endif
                if (le_audio_switch_ops && le_audio_switch_ops->tx_le_audio_open) {
                    connected_hdl->recorder = le_audio_switch_ops->tx_le_audio_open(&params);
                }
            }

        }
    }


    connected_mutex_pend(&connected_mutex, __LINE__);
    if (!find) {
        spin_lock(&connected_lock);
        list_add_tail(&connected_hdl->entry, &connected_list_head);
        spin_unlock(&connected_lock);
    }
    connected_mutex_post(&connected_mutex, __LINE__);

    return 0;
}

#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_UNICAST_SINK_EN))
/**
 * @brief 私有cis命令开启或者关闭解码器
 *
 * @param en 开启/关闭解码器
 * @param acl_hdl acl链路句柄
 */
void connected_perip_connect_recoder(u8 en, u16 acl_hdl)
{
    int i;
    struct connected_hdl *p;
    u8 _bn_p_to_c = 0;
    le_audio_switch_ops = get_connected_audio_sw_ops();
    void *recorder = NULL;
    if (en) {
        r_printf("connected_perip_connect_recoder open\n");
        struct le_audio_stream_params params = {0};
        get_encoder_params_fmt(&params.fmt);
        params.latency = 50 * 1000;//tx延时暂时先设置 50ms
        params.fmt.coding_type = LE_AUDIO_CODEC_TYPE;
        params.fmt.dec_ch_mode = LEA_DEC_OUTPUT_CHANNEL;
        params.service_type = LEA_SERVICE_CALL;
        connected_mutex_pend(&connected_mutex, __LINE__);
        spin_lock(&connected_lock);
        list_for_each_entry(p, &connected_list_head, entry) {
            for (i = 0; i < CIG_MAX_CIS_NUMS; i++) {
                if (p->cis_hdl_info[i].acl_hdl == acl_hdl) {
                    params.fmt.flush_timeout = p->cis_hdl_info[i].flush_timeout;
                    params.fmt.isoIntervalUs = p->cis_hdl_info[i].isoIntervalUs;
                    _bn_p_to_c = p->cis_hdl_info[i].BN_P_To_C;
                }
            }
        }
        spin_unlock(&connected_lock);
        connected_mutex_post(&connected_mutex, __LINE__);
        if ((transmit_buf == NULL) && _bn_p_to_c) {
            transmit_buf = zalloc(get_cig_transmit_data_len() * _bn_p_to_c);    //by haibin--0624
        }
        ASSERT(transmit_buf, "transmit_buf is NULL, BN_P_To_C:%d\n", _bn_p_to_c);
        if (le_audio_switch_ops && le_audio_switch_ops->tx_le_audio_open) {
            recorder = le_audio_switch_ops->tx_le_audio_open(&params);
        }
    } else {
        r_printf("connected_perip_connect_recoder close\n");
    }
    connected_mutex_pend(&connected_mutex, __LINE__);
    spin_lock(&connected_lock);
    list_for_each_entry(p, &connected_list_head, entry) {
        for (i = 0; i < CIG_MAX_CIS_NUMS; i++) {
            if (p->cis_hdl_info[i].acl_hdl == acl_hdl) {
                if (en) {
                    if (!p->recorder) {
                        p->recorder = recorder;
                    }
                } else {
                    if (le_audio_switch_ops && le_audio_switch_ops->tx_le_audio_close) {
                        if (p->recorder) {
                            spin_unlock(&connected_lock);
                            le_audio_switch_ops->tx_le_audio_close(p->recorder);
                            spin_lock(&connected_lock);
                            p->recorder = NULL;

                        }
                    }
                }
            }
        }
    }
    spin_unlock(&connected_lock);
    connected_mutex_post(&connected_mutex, __LINE__);
}
#endif

/* --------------------------------------------------------------------------*/
/**
 * @brief CIG从机连接断开成功处理接口口
 *
 * @param priv:断链成功附带的句柄参数
 *
 * @return 是否执行成功 -- 0为成功，其他为失败
 */
/* ----------------------------------------------------------------------------*/
int connected_perip_disconnect_deal(void *priv)
{
    u8 i, index;
    u8 cis_connected_num = 0;
    int player_status = 0;
    struct connected_hdl *p, *connected_hdl;
    cig_hdl_t *hdl = (cig_hdl_t *)priv;
    void *recorder = 0;
    struct connected_rx_audio_hdl player = {0};

    player.le_audio = 0;
    player.rx_stream = 0;

    log_info("connected_perip_disconnect_deal");
    log_info("%s, cig_hdl:%d", __FUNCTION__, hdl->cig_hdl);

    //TODO:获取播放状态
    if (le_audio_switch_ops && le_audio_switch_ops->play_status) {
        player_status = le_audio_switch_ops->play_status();
    }

    connected_mutex_pend(&connected_mutex, __LINE__);
    spin_lock(&connected_lock);
    list_for_each_entry(p, &connected_list_head, entry) {
        /* log_info("%s, cig_hdl:%x  %x", __FUNCTION__, hdl->cig_hdl, p->cig_hdl); */
        if (p && (p->cig_hdl == hdl->cig_hdl)) {
            for (i = 0; i < CIG_MAX_CIS_NUMS; i++) {
                /* log_info("%s, cis_hdl:%x  %x", __FUNCTION__, p->cis_hdl_info[i].cis_hdl,  hdl->cis_hdl); */
                p->cis_hdl_info[i].cis_hdl = 0xff;
                if (p->recorder) {
                    recorder = p->recorder;
                    p->recorder = NULL;
                }

                if (p->rx_player.le_audio) {
                    player.le_audio = p->rx_player.le_audio;
                    p->rx_player.le_audio = NULL;
                }

                if (p->rx_player.rx_stream) {
                    player.rx_stream = p->rx_player.rx_stream;
                    p->rx_player.rx_stream = NULL;
                }
                index = i;
            }
            p->rx_cis_num = 0;

            spin_unlock(&connected_lock);

            if (recorder) {
                //TODO:关闭recorder
                if (le_audio_switch_ops && le_audio_switch_ops->tx_le_audio_close) {
                    le_audio_switch_ops->tx_le_audio_close(recorder);
                }
            }
            if (player.le_audio && player.rx_stream) {
                //TODO:关闭player
                if (le_audio_switch_ops && le_audio_switch_ops->rx_le_audio_close) {
                    le_audio_switch_ops->rx_le_audio_close(&player);
                }
                if (multi_cis_rx_buf) {
                    free(multi_cis_rx_buf);
                    multi_cis_rx_buf = 0;
                }
            }

            spin_lock(&connected_lock);

            memset(&p->cis_hdl_info[index], 0, sizeof(cis_hdl_info_t));

            connected_hdl = p;
            //break;
        }
    }

    spin_unlock(&connected_lock);
    connected_mutex_post(&connected_mutex, __LINE__);


    if (!cis_connected_num) {

        connected_role |= BIT(7);   //断开连接后，或上BIT(7)，防止外部流程判断错误

        connected_hdl->cig_sync_delay = 0;

        //recorder 关闭之后发数中间临时缓存buffer释放
        if (transmit_buf) {
            free(transmit_buf);
            transmit_buf = NULL;
        }

        connected_free_cig_hdl(hdl->cig_hdl);

        if (player_status == LOCAL_AUDIO_PLAYER_STATUS_PLAY) {
            //TODO:打开本地音频
            if (le_audio_switch_ops && le_audio_switch_ops->local_audio_open) {
                le_audio_switch_ops->local_audio_open();
            }
        }
    }

    return 0;
}

/**
 *	@brief 蓝牙取数回调接口
 *
 *	@param cig_hdl cig句柄
 */
static int connected_tx_align_data_handler(u8 cig_hdl)
{
    struct connected_hdl *connected_hdl = 0;
    cis_hdl_info_t *cis_hdl_info;
    u32 timestamp = 0;
    cis_txsync_t txsync;
    int rlen = 0, i, j, k;
    int err;
    u8 capture_send_update = 0;
    u8 packet_num;
    u16 single_ch_trans_data_len;
    void *L_buffer, *R_buffer;
    void *last_recorder;
    cig_stream_param_t param = {0};

    spin_lock(&connected_lock);
    list_for_each_entry(connected_hdl, &connected_list_head, entry) {
        if (connected_hdl->cig_hdl != cig_hdl) {
            continue;
        }

        if (connected_hdl->del) {
            continue;
        }

        if (!connected_hdl->recorder) {
            continue;
        }

        for (i = 0; i < CIG_MAX_CIS_NUMS; i++) {
            cis_hdl_info = &connected_hdl->cis_hdl_info[i];
            /* printf("c:%d, i:%d, bn:%d\n", cis_hdl_info->cis_hdl, i, cis_hdl_info->BN_P_To_C, cis_hdl_info->Max_PDU_P_To_C); */
            if (cis_hdl_info->cis_hdl && cis_hdl_info->BN_P_To_C && cis_hdl_info->Max_PDU_P_To_C) {

                if (connected_hdl->recorder && (connected_hdl->recorder != last_recorder)) {
                    last_recorder = connected_hdl->recorder;
                    txsync.cis_hdl = cis_hdl_info->cis_hdl;
                    connected_get_cis_tick_time(&txsync);
                    timestamp = (txsync.tx_ts + connected_hdl->cig_sync_delay +
                                 get_cig_mtl_time() * 1000L + get_cig_sdu_period_us()) & 0xfffffff;
                }

                if (timestamp) {
                    rlen = le_audio_stream_tx_data_handler(connected_hdl->recorder, transmit_buf, get_cig_transmit_data_len() * cis_hdl_info->BN_P_To_C, timestamp);
                }
                if (!rlen) {
                    putchar('^');
                    memset(transmit_buf, 0, get_cig_transmit_data_len() * cis_hdl_info->BN_P_To_C);
                    /* printf("c:%d, n\n", cis_hdl_info->cis_hdl); */
                }
                int offset = 0;
                for (k = 0; k < cis_hdl_info->BN_P_To_C; k++) {
                    param.cis_hdl = cis_hdl_info->cis_hdl;
                    err = wireless_trans_transmit((connected_role & CONNECTED_ROLE_PERIP) == CONNECTED_ROLE_PERIP ? "cig_perip" : "cig_central", transmit_buf + offset, get_cig_transmit_data_len(), &param);
                    if (err != 0) {
                        log_error("wireless_trans_transmit fail\n");
                    }
                    offset += get_cig_transmit_data_len();
                }
            }
        }
    }
    spin_unlock(&connected_lock);

    return 0;
}

void le_audio_phone_connect_profile_begin(uint16_t con_handle)
{
    r_printf("le_audio_phone_connect_profile_begin");
    cig_event_to_user(CIG_EVENT_PHONE_CONNECT, (void *)&con_handle, 2);
}

void le_audio_phone_connect_profile_disconnect(uint16_t con_handle)
{
    r_printf("le_audio_phone_connect_profile_disconnect");
    cig_event_to_user(CIG_EVENT_PHONE_DISCONNECT, (void *)&con_handle, 2);
}

/**
 *	cis事件回调
 */
static void connected_perip_event_callback(const CIG_EVENT event, void *priv)
{
    /* log_info("--func=%s, %d", __FUNCTION__, event); */

    /* g_printf("connected_perip_event_callback=%x\n", event); */
    switch (event) {
    //cis接收端连接成功后回调事件
    case CIG_EVENT_CIS_CONNECT:
        log_info("%s, CIG_EVENT_CIS_CONNECT\n", __FUNCTION__);
        cig_event_to_user(CIG_EVENT_PERIP_CONNECT, priv, sizeof(cig_hdl_t));
        break;

    //cis接收端断开成功后回调事件
    case CIG_EVENT_CIS_DISCONNECT:
        log_info("%s, CIG_EVENT_CIS_DISCONNECT\n", __FUNCTION__);
        cig_event_to_user(CIG_EVENT_PERIP_DISCONNECT, priv, sizeof(cig_hdl_t));
        break;

    case CIG_EVENT_ACL_CONNECT:
        log_info("%s, CIG_EVENT_ACL_CONNECT\n", __FUNCTION__);
        cig_event_to_user(event, priv, sizeof(cis_acl_info_t));
        break;

    case CIG_EVENT_ACL_DISCONNECT:
        log_info("%s, CIG_EVENT_ACL_DISCONNECT\n", __FUNCTION__);
        cig_event_to_user(event, priv, sizeof(cis_acl_info_t));
        break;

    //蓝牙取数发射回调事件
    case CIG_EVENT_TRANSMITTER_ALIGN:
        /* WARNING:该事件为中断函数回调, 不要添加过多打印 */
        u8 cig_hdl = *((u8 *)priv);
        connected_tx_align_data_handler(cig_hdl);
        break;

    default:
        break;
    }
}

int connected_iso_recv_handle_register(void *priv, void (*recv_handle)(u16 conn_handle, const void *const buf, size_t length, void *priv))
{
    struct acl_recv_hdl *item = NULL;
    if (recv_handle) {
        item = zalloc(sizeof(struct acl_recv_hdl));
        if (NULL == item) {
            printf("err: %s alloc fail\n", __func__);
            return -1;
        }
        item->recv_handle = recv_handle;
        list_add_tail(&item->entry, &acl_data_recv_list_head);
    }

    rcsp_update_flag = 1;
    return 0;
}

static void connected_iso_recv_handle(u16 conn_handle, const void *const buf, size_t length, void *priv)
{
    struct acl_recv_hdl *item = NULL;

    if (rcsp_update_flag) {
        //set rcsp acl param
        printf("Set rcsp acl param:0x%x", conn_handle);
        set_aclMaxPduPToC_test(255);
        ble_op_set_rxmaxbuf(conn_handle, 255);
        rcsp_update_flag = 0;
    }

    list_for_each_entry(item, &acl_data_recv_list_head, entry) {
        item->recv_handle(conn_handle, buf, length, priv);
    }
}

/*
 * CIS通道收到数据回连到上层处理
 * */
static void connected_iso_callback(const void *const buf, size_t length, void *priv)
{
    u8 i = 0;
    static u8 j = 0;
    s8 index = -1;
    bool plc_flag = 0;
    cig_stream_param_t *param;
    struct connected_hdl *hdl;

    param = (cig_stream_param_t *)priv;

    if (param->acl_hdl) {
        //收取同步数据
        connected_iso_recv_handle(param->acl_hdl, buf, length, priv);
        log_error("%s, %d\n", __FUNCTION__, __LINE__);
        return;
    }

    //为了兼容配置了间隔20ms，连续两包CIS的时候timestamp一样，临时处理使用
    /* log_info("<<- cis Data Out <<- TS:%d,%d", param->ts, (int)length); */
    spin_lock(&connected_lock);
    list_for_each_entry(hdl, &connected_list_head, entry) {
        if (hdl->del) {
            log_error("%s, %d\n", __FUNCTION__, __LINE__);
            continue;
        }
        if ((!hdl->rx_player.le_audio) || (!hdl->rx_player.rx_stream)) {
            log_error("%s, %d\n", __FUNCTION__, __LINE__);
            continue;
        }
#if CIS_AUDIO_PLC_ENABLE
        if (length == 0) {
            plc_flag = 1;
            /* log_error("SDU empty"); */
            putchar('z');
        }
#endif//CIS_AUDIO_PLC_ENABLE

        for (i = 0; i < hdl->rx_cis_num; i++) {
            if (hdl->cis_hdl_info[i].cis_hdl == param->cis_hdl) {
                index = i;
                break;
            }
        }

        if (index == -1) {
            continue;
        }

        j++;
        if (j >= hdl->cis_hdl_info[i].BN_C_To_P) {
            j = 0;
        }

        /* printf("[%d][%d][%d][%d]\n", i, j, param->cis_hdl, (int)length); */
        if (plc_flag || multi_cis_plc_flag) {
            if (multi_cis_rx_buf) {
                multi_cis_plc_flag = 1;
                for (i = 0; i < hdl->rx_cis_num; i++) {
                    if (hdl->cis_hdl_info[i].cis_hdl == param->cis_hdl) {
                        break;
                    }
                }
                /* printf("[%d][%d][%d][0x%x]\n", i, param->cis_hdl, (int)length, (unsigned int)multi_cis_rx_buf); */
                if (i == (hdl->rx_cis_num - 1)) {
                    memcpy(multi_cis_rx_buf, errpacket, 2);
                    /* putchar('r'); */
                    le_audio_stream_rx_frame(hdl->rx_player.rx_stream, (void *)multi_cis_rx_buf, 2, param->ts);
                    multi_cis_data_offect = 0;
                    multi_cis_plc_flag = 0;
                }
            } else {
                /* putchar('r'); */

                /* printf("r:%d, %d\n", (int)length, (int)param->ts); */
                int frame_num = 1;
                if (get_le_audio_jl_dongle_device_type()) {
                    frame_num = hdl->cis_hdl_info[i].Max_PDU_C_To_P / get_lc3_decoder_info_octets_per_frame();
                }
                u32 timestamp = 0;
                for (int k = 0; k < frame_num; k++) {
                    le_audio_stream_rx_frame(hdl->rx_player.rx_stream, (void *)errpacket, 2, param->ts + timestamp);
                    timestamp += get_cig_audio_coding_frame_duration() * 100;
                }
                /* r_printf("r=%d,%d\n",get_lc3_decoder_info_octets_per_frame(),frame_num ); */
            }
        } else {
            if (multi_cis_rx_buf) {
                /* printf("[%d][%d][%d][%d][%d][0x%x]\n", i, param->cis_hdl, multi_cis_data_offect, (int)length, multi_cis_rx_temp_buf_len, (unsigned int)multi_cis_rx_buf); */
                memcpy(multi_cis_rx_buf + multi_cis_data_offect, buf, length);
                multi_cis_data_offect += length;
                ASSERT(multi_cis_data_offect <= multi_cis_rx_temp_buf_len);
                if (multi_cis_data_offect >= multi_cis_rx_temp_buf_len) {
                    /* put_buf((void *)multi_cis_rx_buf, multi_cis_rx_temp_buf_len); */
                    /* putchar('R'); */
                    /* printf("R:%d\n", param->ts); */
                    le_audio_stream_rx_frame(hdl->rx_player.rx_stream, (void *)multi_cis_rx_buf, multi_cis_rx_temp_buf_len, param->ts);
                    multi_cis_data_offect = 0;
                }
            } else {
                /* putchar('R'); */
                /* printf("R:%d, %d\n", (int)length, (int)param->ts); */
                le_audio_stream_rx_frame(hdl->rx_player.rx_stream, (void *)buf, length, param->ts);
            }
            /* extern uint32_t bb_le_clk_get_time_us(void); */
            /* u32 local = bb_le_clk_get_time_us(); */
            /* printf("[%d, %d, %d]\n", local, param->ts, local - param->ts); */
        }
    }
    spin_unlock(&connected_lock);
}

/* --------------------------------------------------------------------------*/
/**
 * @brief CIG从机启动接口
 *
 * @param params:CIG从机启动所需参数
 *
 * @return 分配的cig_hdl
 */
/* ----------------------------------------------------------------------------*/
int connected_perip_open(cig_parameter_t *params)
{
    u8 i, j;
    int ret;

    connected_init();

    if (!connected_init_flag) {
        return -2;
    }

    if (connected_num >= CIG_MAX_NUMS) {
        log_error("connected_num overflow");
        return -1;
    }

    if ((connected_role & CONNECTED_ROLE_CENTRAL) == CONNECTED_ROLE_CENTRAL) {
        log_error("connected_role err");
        return -1;
    }

    g_printf("--func=%s", __FUNCTION__);
    u8 available_cig_hdl = 0xFF;

    set_cig_hdl(CONNECTED_ROLE_PERIP, available_cig_hdl);
    ret = wireless_trans_open("cig_perip", (void *)params);
    if (ret != 0) {
        log_error("wireless_trans_open fail:0x%x\n", ret);
        if (connected_num == 0) {
            connected_role = CONNECTED_ROLE_UNKNOW;
        }
        return -1;
    }
    /*
        if (transmit_buf) {
            free(transmit_buf);
        }
        transmit_buf = zalloc(get_cig_transmit_data_len());
        ASSERT(transmit_buf, "transmit_buf is NULL");
    */
    connected_role = CONNECTED_ROLE_PERIP | BIT(7);	//或上BIT(7)，防止外部流程判断错误

    connected_num++;

    //TODO:修改时钟
    clock_alloc("le_connected", 12 * 1000000UL);

    return available_cig_hdl;
}
int cis_audio_player_resume(u8 cig_hdl, u8 cig_phone_call_play)
{
    int ret = 0;
    u8 i;
    u8 find = 0;
    struct connected_hdl *connected_hdl = 0;
    struct le_audio_stream_params params = {0};

    connected_mutex_pend(&connected_mutex, __LINE__);
    list_for_each_entry(connected_hdl, &connected_list_head, entry) {
        if (connected_hdl->cig_hdl == cig_hdl) {
            find = 1;
            break;
        }
    }
    connected_mutex_post(&connected_mutex, __LINE__);

    if (!find) {
        return 0;
    }

    le_audio_switch_ops = get_connected_audio_sw_ops();

    for (i = 0; i < CIG_MAX_CIS_NUMS; i++) {
        if (connected_hdl->cis_hdl_info[i].cis_hdl) {
            if (get_le_audio_jl_dongle_device_type()) {
                get_decoder_params_fmt(&(params.fmt));
            } else {
                params.fmt.nch = get_cig_audio_coding_nch();
                params.fmt.bit_rate = get_cig_audio_coding_bit_rate();
                params.fmt.frame_dms = get_cig_audio_coding_frame_duration();
                params.fmt.sdu_period = get_cig_sdu_period_us();
                params.fmt.sample_rate = get_cig_audio_coding_sample_rate();
            }
            params.fmt.coding_type = LE_AUDIO_CODEC_TYPE;
            params.fmt.dec_ch_mode = LEA_DEC_OUTPUT_CHANNEL;
            params.fmt.flush_timeout = connected_hdl->cis_hdl_info[i].flush_timeout;
            params.fmt.isoIntervalUs = connected_hdl->cis_hdl_info[i].isoIntervalUs;
            /* params.reference_time = connected_audio_reference_time; */
            params.latency = 50 * 1000;//tx延时暂时先设置 50ms
            params.conn = connected_hdl->cis_hdl_info[i].cis_hdl;

            if (get_le_audio_jl_dongle_device_type()) {
                params.service_type = LEA_SERVICE_MEDIA;
            } else {
                params.service_type = (connected_hdl->cis_hdl_info[i].Max_PDU_P_To_C) ? LEA_SERVICE_CALL : LEA_SERVICE_MEDIA;
            }

            //TODO:开启播放器播放远端传来的音频
            if (le_audio_switch_ops && le_audio_switch_ops->rx_le_audio_open) {
                connected_mutex_pend(&connected_mutex, __LINE__);
                le_audio_switch_ops->rx_le_audio_open(&connected_hdl->rx_player, &params);

                if (cig_phone_call_play) {
                    connected_perip_connect_recoder(1, connected_hdl->cis_hdl_info[i].acl_hdl);
                }
                connected_mutex_post(&connected_mutex, __LINE__);
            }
            ret = 1;
        }
    }
    return ret;
}

int cis_audio_player_close(u8 cig_hdl)
{
    struct connected_hdl *p;
    struct connected_rx_audio_hdl player;
    int ret = 0;

    void *recorder = 0;
    player.le_audio = 0;
    player.rx_stream = 0;
    connected_mutex_pend(&connected_mutex, __LINE__);
    spin_lock(&connected_lock);
    list_for_each_entry(p, &connected_list_head, entry) {
        if (p->cig_hdl == cig_hdl) {
            for (int i = 0; i < CIG_MAX_CIS_NUMS; i++) {
                if (p->recorder) {
                    recorder = p->recorder;
                    p->recorder = NULL;
                }
                if (p->rx_player.le_audio) {
                    player.le_audio = p->rx_player.le_audio;
                    p->rx_player.le_audio = NULL;
                }

                if (p->rx_player.rx_stream) {
                    player.rx_stream = p->rx_player.rx_stream;
                    p->rx_player.rx_stream = NULL;
                }

                spin_unlock(&connected_lock);
                if (recorder) {
                    //TODO:关闭recorder
                    if (le_audio_switch_ops && le_audio_switch_ops->tx_le_audio_close) {
                        le_audio_switch_ops->tx_le_audio_close(recorder);
                        recorder = NULL;
                    }
                }

                if (player.le_audio && player.rx_stream) {
                    if (le_audio_switch_ops && le_audio_switch_ops->rx_le_audio_close) {
                        le_audio_switch_ops->rx_le_audio_close(&player);
                        player.le_audio = NULL;
                        player.rx_stream = NULL;
                        ret = 1;
                    }
                }

                spin_lock(&connected_lock);
            }
        }
    }
    spin_unlock(&connected_lock);
    connected_mutex_post(&connected_mutex, __LINE__);
    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 关闭对应cig_hdl的CIG连接
 *
 * @param cig_hdl:需要关闭的CIG连接的cig_hdl
 */
/* ----------------------------------------------------------------------------*/
static void connected_free_cig_hdl(u8 cig_hdl)
{
    //释放链表
    struct connected_hdl *p, *n;
    void *recorder = 0;
    struct connected_rx_audio_hdl player;

    player.le_audio = 0;
    player.rx_stream = 0;
    connected_mutex_pend(&connected_mutex, __LINE__);
    spin_lock(&connected_lock);
    list_for_each_entry_safe(p, n, &connected_list_head, entry) {
        if (p->cig_hdl != cig_hdl) {
            continue;
        }

        list_del(&p->entry);

        for (int i = 0; i < CIG_MAX_CIS_NUMS; i++) {
            p->cis_hdl_info[i].cis_hdl = 0;
            if (p->recorder) {
                recorder = p->recorder;
                p->recorder = NULL;
            }

            if (p->rx_player.le_audio) {
                player.le_audio = p->rx_player.le_audio;
                p->rx_player.le_audio = NULL;
            }

            if (p->rx_player.rx_stream) {
                player.rx_stream = p->rx_player.rx_stream;
                p->rx_player.rx_stream = NULL;
            }

            spin_unlock(&connected_lock);

            if (recorder) {
                //TODO:关闭recorder
                if (le_audio_switch_ops && le_audio_switch_ops->tx_le_audio_close) {
                    le_audio_switch_ops->tx_le_audio_close(recorder);
                    recorder = NULL;
                }
            }
            if (player.le_audio && player.rx_stream) {
                //TODO:关闭player
                if (le_audio_switch_ops && le_audio_switch_ops->rx_le_audio_close) {
                    le_audio_switch_ops->rx_le_audio_close(&player);
                    player.le_audio = NULL;
                    player.rx_stream = NULL;
                }
            }

            spin_lock(&connected_lock);
        }

        free(p);
    }
    spin_unlock(&connected_lock);
    connected_mutex_post(&connected_mutex, __LINE__);
}

/**
 * @brief 关闭对应cig_hdl的CIG连接
 *
 * @param cig_hdl:需要关闭的CIG连接的cig_hdl
 */
void connected_close(u8 cig_hdl)
{
    int player_status = 0;
    int ret;

    if (!connected_init_flag) {
        return;
    }

    log_info("--func=%s", __FUNCTION__);

    //TODO:获取播放状态
    if (le_audio_switch_ops && le_audio_switch_ops->play_status) {
        player_status = le_audio_switch_ops->play_status();
    }

    struct connected_hdl *hdl;
    connected_mutex_pend(&connected_mutex, __LINE__);
    spin_lock(&connected_lock);
    list_for_each_entry(hdl, &connected_list_head, entry) {
        if (hdl->cig_hdl != cig_hdl) {
            continue;
        }
        hdl->del = 1;
    }
    spin_unlock(&connected_lock);
    connected_mutex_post(&connected_mutex, __LINE__);

    //关闭CIG
    if ((connected_role & CONNECTED_ROLE_PERIP) == CONNECTED_ROLE_PERIP) {
        ret = wireless_trans_close("cig_perip", &cig_hdl);
        if (ret != 0) {
            log_error("wireless_trans_close fail:0x%x\n", ret);
        }
    } else if ((connected_role & CONNECTED_ROLE_CENTRAL) == CONNECTED_ROLE_CENTRAL) {
        ret = wireless_trans_close("cig_central", &cig_hdl);
        if (ret != 0) {
            log_error("wireless_trans_close fail:0x%x\n", ret);
        }
    }

    reset_cig_params();

    connected_free_cig_hdl(cig_hdl);

    connected_num--;
    if (connected_num == 0) {
        connected_uninit();
        connected_role = CONNECTED_ROLE_UNKNOW;
        clock_free("le_connected");
    }

    if (player_status == LOCAL_AUDIO_PLAYER_STATUS_PLAY) {
        //TODO:打开本地音频
        if (le_audio_switch_ops && le_audio_switch_ops->local_audio_open) {
            le_audio_switch_ops->local_audio_open();
        }
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 获取当前CIG是主机还是从机
 *
 * @return CIG角色
 */
/* ----------------------------------------------------------------------------*/
u8 get_connected_role(void)
{
    return connected_role;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief acl数据发送接口
 *
 * @param acl_hdl:acl数据通道句柄
 * @param data:需要发送的数据
 * @param length:数据长度
 *
 * @return 实际发送出去的数据长度
 */
/* ----------------------------------------------------------------------------*/
int connected_send_acl_data(u16 acl_hdl, void *data, size_t length)
{
    int err = -1;
    cig_stream_param_t param = {0};
    param.acl_hdl = acl_hdl;

    if ((connected_role & CONNECTED_ROLE_CENTRAL) == CONNECTED_ROLE_CENTRAL) {
        err = wireless_trans_transmit("cig_central", data, length, &param);
    } else if ((connected_role & CONNECTED_ROLE_PERIP) == CONNECTED_ROLE_PERIP) {
        err = wireless_trans_transmit("cig_perip", data, length, &param);
    }
    if (err != 0) {
        log_error("acl wireless_trans_transmit fail:0x%x\n", err);
        err = 0;
    } else {
        err = length;
    }
    return err;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 供外部手动初始化recorder模块
 *
 * @param cis_hdl:recorder模块所对应的cis_hdl
 */
/* ----------------------------------------------------------------------------*/
void cis_audio_recorder_reset(u16 cis_hdl)
{
    u8 i;
    struct connected_hdl *p;
    void *recorder = 0;

    connected_mutex_pend(&connected_mutex, __LINE__);
    list_for_each_entry(p, &connected_list_head, entry) {
        for (i = 0; i < CIG_MAX_CIS_NUMS; i++) {
            if (p->cis_hdl_info[i].cis_hdl == cis_hdl) {
                spin_lock(&connected_lock);
                if (p->recorder) {
                    recorder = p->recorder;
                    p->recorder = NULL;
                }
                spin_unlock(&connected_lock);

                //关闭旧的recorder
                if (recorder) {

                }

                //重新打开新的recorder
                if (!p->recorder) {

                }
                spin_lock(&connected_lock);
                p->recorder = recorder;
                spin_unlock(&connected_lock);
            }
        }
    }
    connected_mutex_post(&connected_mutex, __LINE__);
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 供外部手动关闭recorder模块
 *
 * @param cis_hdl:recorder模块所对应的cis_hdl
 */
/* ----------------------------------------------------------------------------*/
void cis_audio_recorder_close(u16 cis_hdl)
{
    u8 i;
    struct connected_hdl *p;
    void *recorder = 0;

    connected_mutex_pend(&connected_mutex, __LINE__);
    list_for_each_entry(p, &connected_list_head, entry) {
        for (i = 0; i < CIG_MAX_CIS_NUMS; i++) {
            if (p->cis_hdl_info[i].cis_hdl == cis_hdl) {
                spin_lock(&connected_lock);
                if (p->recorder) {
                    recorder = p->recorder;
                    p->recorder = NULL;
                }
                spin_unlock(&connected_lock);

                //关闭旧的recorder
                if (recorder) {

                }
            }
        }
    }
    connected_mutex_post(&connected_mutex, __LINE__);
}

#endif

