#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".recp_translator.data.bss")
#pragma data_seg(".recp_translator.data")
#pragma const_seg(".recp_translator.text.const")
#pragma code_seg(".recp_translator.text")
#endif
#include "system/includes.h"
#include "app_config.h"
#include "app_msg.h"
#include "btstack_rcsp_user.h"
#include "rcsp.h"
#include "rcsp_config.h"
#include "rcsp/JL_rcsp_protocol.h"
#include "app_protocol_common.h"
#include "rcsp_translator.h"
#include "circular_buf.h"
#include "ai_rx_player.h"
#include "a2dp_player.h"
#include "esco_player.h"
#include "esco_recoder.h"
#include "sniff.h"
#include "classic/tws_api.h"
#include "volume_node.h"
#include "reference_time.h"
#include "asm/charge.h"
#include "ai_voice_recoder.h"

#define RCSP_DEBUG_EN
#ifdef  RCSP_DEBUG_EN
#define rcsp_putchar(x)                	putchar(x)
#define rcsp_printf                    	printf
#define rcsp_put_buf(x,len)				put_buf(x,len)
#else
#define rcsp_putchar(...)
#define rcsp_printf(...)
#define rcsp_put_buf(...)
#endif

#if RCSP_MODE && RCSP_ADV_TRANSLATOR

#define AUDIO_CBUF_SIZE(frame_size)         ((frame_size) * 4)
#define AUDIO_CH_MAX                        2
#define MIC_REC_METHOD_NEW                  1  //填1时录音翻译用文件内send_ch替换ai_mic_rec_start管理发数缓存，支持2路发送
#define TRANSLATION_RECV_CHANNEL_ENABLE     0  //填1时翻译播报用文件内recv_ch接口，否则可以用a2dp等方式播放翻译音频
#define TRANSLATION_CALL_CONTROL_BY_DEVICE  0  //录音翻译中通话（来电），需关闭翻译，由耳机控制还是APP控制，1表示耳机控制
#define A2DP_TRANSLATION_RECV_ENABLE        0  //a2dp翻译后是否接收翻译语音数据
#define RECV_SPEED_DEBUG                    0  //数据接收读写buf速度debug
#define ENC_TO_DEC_TEST                     0  //编码数据送去解码测试

#define TWS_FUNC_ID_TRANSLATOR              TWS_FUNC_ID('T', 'S', 'L', 'T')

struct rx_ch_t {
    struct list_head head;
    u32 recv_size;
    u32 max_size;
    void *cb_priv;
    void (*recv_decode_resume)(void *cb_priv, u8 source);
    u8 ready;
    u8 full;
    u8 source;
    OS_MUTEX mutex;
    u32 start_time;
    u32 timestamp; //us
    u8 ready_ts;
    u8 sync_play;
    u8 underrun;
    u16 timer;
    u16 frame_size;
    u16 frame_dms;
    u32 coding_type;
    u32 sample_rate;
    u32 bit_rate;
    u8 channel_mode;
};

struct tx_ch_t {
    u8 *cbuf_buf;
    u32 buf_size;
    u32 offset;
    u16 timer_id;
    u8 source;
    OS_MUTEX mutex;
};

struct todo_list {
    struct list_head entry;
    u8 cmd;
    u8 *param;
    u32 param_len;
};

//翻译功能总信息
struct translator_meta_t {
    u8 state;
    u8 ch_state;
    u16 ble_con_handle;
    u8 ble_remote_addr[6];
    u8 spp_remote_addr[6];
    u16 volume_timer_id;
    struct translator_mode_info mode_info;
    struct rx_ch_t rx_ch[AUDIO_CH_MAX];
    struct tx_ch_t tx_ch[AUDIO_CH_MAX];
    struct list_head todo_list_head;
    u16 detect_timer_id;
    u8 record_state;
};
static struct translator_meta_t tlr_hdl;

enum {
    TWS_TRANS_CMD_SET_MODE,
    TWS_TRANS_CMD_SYNC_REMOTE_ADDR,
    TWS_TRANS_CMD_MANUAL_REC_START,
    TWS_TRANS_CMD_MANUAL_REC_STOP,
    TWS_TRANS_CMD_INFORM_MODE,
    TWS_TRANS_CMD_RECV_AUDIO_SYNC,
    TWS_TRANS_CMD_SET_VOLUME,
    TWS_TRANS_CMD_SYNC_PLAY,
    TWS_TRANS_CMD_MEDIA_SUSPEND,
    TWS_TRANS_CMD_MEDIA_RESUME,
    TWS_TRANS_CMD_SYNC_REC_STA
};

//note: 状态用于回复APP，如有更新需要跟APP一同更新
enum {
    TRANS_SET_MODE_STATUS_SUCC = 0,
    TRANS_SET_MODE_STATUS_IN_MODE,
    TRANS_SET_MODE_STATUS_INVALID_PARAM,
    TRANS_SET_MODE_STATUS_IN_CALL,
    TRANS_SET_MODE_STATUS_IN_A2DP,
    TRANS_SET_MODE_STATUS_BUSY,
    TRANS_SET_MODE_STATUS_FAIL
};

static u16 g_bt_volume = 8;

void bt_sniff_ready_clean(void);
static int _translator_op_send_audio_data(u8 *buf, u32 len, u8 source, u8 count);
static int _translator_op_inform_cache_free_size(u8 source, u32 free_size);
static int translator_tws_send_cmd(u8 cmd, u8 OpCode, u8 *param, u32 len);
static void translator_tws_sync_play(u8 source);
static void translator_tws_recv_ch_suspend(u8 source);
static void translator_tws_recv_ch_resume(u8 source);
extern u32 bt_audio_conn_clock_time(void *addr);

static u8 source_to_ch_remap(u8 source)
{
    u8 ch;
    switch (source) {
    case RCSP_TRANSLATOR_AUDIO_SOURCE_DEV_MIC:
    case RCSP_TRANSLATOR_AUDIO_SOURCE_PHONE_MIC:
    case RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_UPSTREAM:
    case RCSP_TRANSLATOR_AUDIO_SOURCE_MSBC:
        ch = 0;
        break;
    case RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_DOWNSTREAM:
        ch = 1;
        break;
    default:
        ch = 0;
    }
    return ch;
}

static u8 *translator_get_remote_bt_addr()
{
    u8 *remote_bt_addr = NULL;
    u8 zero_addr[6] = {0};

    if ((tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED)) {
        if (tws_api_get_role() == TWS_ROLE_SLAVE) {
            if (tlr_hdl.ble_con_handle) {
                remote_bt_addr = tlr_hdl.ble_remote_addr;
            } else {
                remote_bt_addr = tlr_hdl.spp_remote_addr;
            }
            return remote_bt_addr;
        }
    }
    if (tlr_hdl.ble_con_handle) {
        if (!memcmp(tlr_hdl.ble_remote_addr, zero_addr, 6)) {
            if (tlr_hdl.ble_con_handle >= 0x50) {
                remote_bt_addr = rcsp_get_ble_hdl_remote_mac_addr(tlr_hdl.ble_con_handle);
            } else {
                remote_bt_addr = bt_get_current_remote_addr();
            }
            if (remote_bt_addr) {
                memcpy(tlr_hdl.ble_remote_addr, remote_bt_addr, 6);
            }
        } else {
            remote_bt_addr = tlr_hdl.ble_remote_addr;
        }
    } else {
        remote_bt_addr = tlr_hdl.spp_remote_addr;
    }
    return remote_bt_addr;
}

static void translator_sync_remote_bt_addr()
{
    u8 bt_addr_sync[14];

    if (tws_api_get_role() == TWS_ROLE_SLAVE) {
        return;
    }
    translator_get_remote_bt_addr();
    memcpy(&bt_addr_sync[0], tlr_hdl.spp_remote_addr, 6);
    memcpy(&bt_addr_sync[6], &tlr_hdl.ble_con_handle, 2);
    memcpy(&bt_addr_sync[8], tlr_hdl.ble_remote_addr, 6);
    translator_tws_send_cmd(TWS_TRANS_CMD_SYNC_REMOTE_ADDR, 0, bt_addr_sync, 14);
}

static int translator_recv_ch_start(u8 source)
{
    int ret = 0;
    u8 ch;
    struct rx_ch_t *channel = NULL;
    u32 fsize = 40;
    u32 size;
    u8 *remote_bt_addr = NULL;
    struct ai_rx_player_param param = {0};

    ch = source_to_ch_remap(source);
    os_mutex_pend(&tlr_hdl.rx_ch[ch].mutex, 0);
    if (tlr_hdl.ch_state & BIT(ch)) {
        os_mutex_post(&tlr_hdl.rx_ch[ch].mutex);
        return 0;
    }
    rcsp_printf("rcsp start decode in: source %x, ch %d\n", source, ch);
    channel = &tlr_hdl.rx_ch[ch];
    if (tlr_hdl.mode_info.encode == RCSP_TRANSLATOR_ENCODE_TYPE_OPUS) {
        param.coding_type = AUDIO_CODING_OPUS;
        param.channel_mode =  AUDIO_CH_MIX;
        param.sample_rate = 16000;
        channel->frame_size = 40; //一帧编码输出40个byte;
        channel->frame_dms = 200; //20ms一帧
        fsize = 40 * 5;
    } else if (tlr_hdl.mode_info.encode == RCSP_TRANSLATOR_ENCODE_TYPE_PCM) {
        param.coding_type = AUDIO_CODING_PCM;
        fsize = 320 * 2;  //320采样点 16bit 1ch
    } else if (tlr_hdl.mode_info.encode == RCSP_TRANSLATOR_ENCODE_TYPE_SPEEX) {
        //TODO
        param.coding_type = AUDIO_CODING_SPEEX;
        fsize = 200;
    } else if (tlr_hdl.mode_info.encode == RCSP_TRANSLATOR_ENCODE_TYPE_MSBC) {
        //TODO
        param.coding_type = AUDIO_CODING_MSBC;
        fsize = 200;
    } else if (tlr_hdl.mode_info.encode == RCSP_TRANSLATOR_ENCODE_TYPE_JLA_V2) {
        param.coding_type = AUDIO_CODING_JLA_V2;
        param.sample_rate = 16000;
        param.frame_dms = 200;  //20ms一帧
        param.channel_mode = AUDIO_CH_MIX;
        param.bit_rate = 16000;
        int frame_size = param.frame_dms * param.bit_rate / 8 / 10000 + 2;
        channel->frame_size = frame_size; //一帧编码输出的大小;
        channel->frame_dms = param.frame_dms;
        channel->bit_rate =  param.bit_rate;
        fsize = frame_size * 5;
    } else {
        ret = -1;
        goto __err_exit;
    }

    channel->coding_type =  param.coding_type;
    channel->sample_rate =  param.sample_rate;
    channel->channel_mode =  param.channel_mode;

    size = AUDIO_CBUF_SIZE(fsize);
    INIT_LIST_HEAD(&channel->head);
    channel->recv_size = 0;
    channel->max_size = size;
    channel->ready = 0;
    channel->underrun = 0;
    channel->full = 0;
    channel->source = source;
#if TCFG_AI_RX_NODE_ENABLE
    remote_bt_addr = translator_get_remote_bt_addr();
    if (remote_bt_addr == NULL) {
        ret = -1;
        goto __err_exit;
    }
    switch (source) {
    case RCSP_TRANSLATOR_AUDIO_SOURCE_MSBC:
        param.type = AI_SERVICE_MEDIA;
        break;
    case RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_UPSTREAM:
        param.type = AI_SERVICE_CALL_UPSTREAM;
        break;
    case RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_DOWNSTREAM:
        param.type = AI_SERVICE_CALL_DOWNSTREAM;
        break;
    default:
        param.type = AI_SERVICE_VOICE;
        break;
    }

    ret = ai_rx_player_open(remote_bt_addr, source, &param);
    if (ret < 0) {
        goto __err_exit;
    }
#endif
    rcsp_printf("rcsp start decode out: source %x, ch %d\n", source, ch);
    tlr_hdl.ch_state |= BIT(ch);
    os_mutex_post(&tlr_hdl.rx_ch[ch].mutex);

    return 0;

__err_exit:
    rcsp_printf("rcsp start decode fail: source %x, ch %d\n", source, ch);
    os_mutex_post(&tlr_hdl.rx_ch[ch].mutex);
    return ret;
}

static void translator_recv_ch_stop(u8 source)
{
    struct rx_ch_t *channel;
    u8 ch;
    struct list_head *p, *n;
    struct translator_recv_audio_frame *frame;

    ch = source_to_ch_remap(source);
    os_mutex_pend(&tlr_hdl.rx_ch[ch].mutex, 0);
    if (!(tlr_hdl.ch_state & BIT(ch))) {
        os_mutex_post(&tlr_hdl.rx_ch[ch].mutex);
        return;
    }
    rcsp_printf("rcsp stop decode: source %x, ch %d\n", source, ch);
    channel = &tlr_hdl.rx_ch[ch];
#if TCFG_AI_RX_NODE_ENABLE
    ai_rx_player_close(source);
#endif
    list_for_each_safe(p, n, &channel->head) {
        frame = list_entry(p, struct translator_recv_audio_frame, entry);
        list_del(&frame->entry);
        frame->size = 0;
        frame->timestamp = 0;
        free(frame);
    }
    channel->recv_size = 0;
    tlr_hdl.ch_state &= ~BIT(ch);
    os_mutex_post(&tlr_hdl.rx_ch[ch].mutex);
}

static u32 translator_recv_ch_get_timestamp(u8 source)
{
    struct rx_ch_t *channel;
    u8 ch;
    u8 *remote_bt_addr = NULL;
    u32 timestamp = 0;

    ch = source_to_ch_remap(source);
    if (tlr_hdl.ch_state & BIT(ch)) {
        remote_bt_addr = translator_get_remote_bt_addr();
        if (remote_bt_addr == NULL) {
            return 0;
        }
        channel = &tlr_hdl.rx_ch[ch];
        if (OS_NO_ERR != os_mutex_pend(&channel->mutex, 10)) {
            return 0;
        }
        if (!channel->ready) {
            if (!channel->ready_ts) {
                channel->start_time = bt_audio_conn_clock_time(remote_bt_addr);//获取当前时间,
                channel->timestamp = channel->start_time * 625; //转换成us
                channel->ready_ts = 1;
                /* y_printf("-ready : start_ts: %u,\n",channel->timestamp); */
            }

        } else {
            channel->ready_ts = 0;
        }
        if (!channel->start_time) {
            channel->start_time = bt_audio_conn_clock_time(remote_bt_addr);//获取当前时间,
            channel->timestamp = channel->start_time * 625;
            /* y_printf("---start_ts: %u,\n",channel->timestamp); */
        }
        /* printf("--- %u,\n",channel->timestamp); */
        timestamp = channel->timestamp & 0xffffffff;
        os_mutex_post(&channel->mutex);
    }
    return timestamp;
}

static void translator_recv_ch_update_timestamp(u8 source, u32 input_size)
{
    struct rx_ch_t *channel;
    u8 ch;
    u8 frame_num = 0;
    u32 frame_time = 0;
    /*解析帧长时间*/
    ch = source_to_ch_remap(source);
    if (tlr_hdl.ch_state & BIT(ch)) {
        channel = &tlr_hdl.rx_ch[ch];
        if (OS_NO_ERR != os_mutex_pend(&channel->mutex, 10)) {
            return;
        }
        if (tlr_hdl.mode_info.encode == RCSP_TRANSLATOR_ENCODE_TYPE_OPUS) {
            //opus一帧数据40个byte,20ms,如有改动，这里需要同步修改
            frame_num = input_size / 40;
            frame_time = frame_num * 20 * 1000 ;  //us
        } else if (tlr_hdl.mode_info.encode == RCSP_TRANSLATOR_ENCODE_TYPE_JLA_V2) {
            //JLA_V2一帧数据40个byte + 2个byte 的头,20ms,如有改动，这里需要同步修改
            /* frame_num = input_size / (40 + 2); */
            frame_num = input_size / channel->frame_size;
            frame_time = frame_num * channel->frame_dms * 1000 / 10;  //us
        } else {
            /*添加其他格式 */
        }
        channel->timestamp += frame_time;
        os_mutex_post(&channel->mutex);
    }
}

static int translator_recv_ch_put_frame(u8 source, u8 *buf, u32 len, u32 timestamp)
{
    struct rx_ch_t *channel;
    u8 ch;
    int ret = 0;
    struct translator_recv_audio_frame *frame = NULL;

    if (!tlr_hdl.state) {
        return 0;
    }
    ch = source_to_ch_remap(source);
    if (OS_NO_ERR != os_mutex_pend(&tlr_hdl.rx_ch[ch].mutex, 10)) {
        return 0;
    }
    if (!(tlr_hdl.ch_state & BIT(ch))) {
        ret = -EFAULT;
        goto __err_exit;
    }
    channel = &tlr_hdl.rx_ch[ch];
    if (channel->underrun) {
        if ((channel->underrun & 0x02) == 0) {
            if ((tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED)) {
                if (tws_api_get_role() == TWS_ROLE_MASTER) {
                    translator_tws_send_cmd(TWS_TRANS_CMD_MEDIA_RESUME, 0, &channel->source, sizeof(u8));
                }
            } else {
                int msg[3];
                msg[0] = (int)translator_tws_recv_ch_resume;
                msg[1] = 1;
                msg[2] = channel->source;
                os_taskq_post_type("app_core", Q_CALLBACK, 3, msg);
            }
            channel->underrun |= 0x02;
        }
    }
    if (channel->recv_size + len > channel->max_size) {
        rcsp_putchar('T');
        ret = -ENOMEM;
        goto __err_exit;
    }
    frame = zalloc(sizeof(*frame) + len);
    if (frame == NULL) {
        ret = -ENOMEM;
        goto __err_exit;
    }
    frame->buf = (u8 *)(frame + 1);
    frame->size = len;
    frame->timestamp = timestamp;
    frame->source = source;
    memcpy(frame->buf, buf, frame->size);
    list_add_tail(&frame->entry, &channel->head);
    channel->recv_size += len;

    if ((tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED)) {
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            if (tlr_hdl.mode_info.mode != RCSP_TRANSLATOR_MODE_CALL_TRANSLATION) {
                translator_tws_send_cmd(TWS_TRANS_CMD_RECV_AUDIO_SYNC, 0, (u8 *)frame, sizeof(*frame) + len);
            }
        }
    }

    if (!channel->ready) {
        if (channel->recv_size >= channel->max_size * 2 / 4) {
            channel->ready = 1;
            if ((tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED)) {
                //if (tws_api_get_role() == TWS_ROLE_MASTER) {
                //    channel->sync_play = 0;
                translator_tws_send_cmd(TWS_TRANS_CMD_SYNC_PLAY, 0, &channel->source, sizeof(u8));
                //}
            } else {
                int msg[3];
                msg[0] = (int)translator_tws_sync_play;
                msg[1] = 1;
                msg[2] = channel->source;
                os_taskq_post_type("app_core", Q_CALLBACK, 3, msg);
            }
        }
    }

#if RECV_SPEED_DEBUG
    g_printf("put[%d]: %d %d\n", ch, frame->size, channel->recv_size);
#endif
    os_mutex_post(&tlr_hdl.rx_ch[ch].mutex);
    return ret;

__err_exit:
    if (frame) {
        free(frame);
    }
    os_mutex_post(&tlr_hdl.rx_ch[ch].mutex);
    return ret;
}

static void translator_recv_audio_underrun_signal(void *arg)
{
    struct rx_ch_t *channel = (struct rx_ch_t *)arg;

    local_irq_disable();
    if (channel->recv_decode_resume) {
        channel->recv_decode_resume(channel->cb_priv, channel->source);
    }
    channel->timer = 0;
    local_irq_enable();
}


static int translator_recv_audio_frame_underrun_detect(u8 source)
{
    u8 *remote_bt_addr = translator_get_remote_bt_addr();
    if (remote_bt_addr == NULL) {
        return -1;
    }
    u8 ch = source_to_ch_remap(source);
    struct rx_ch_t *channel = &tlr_hdl.rx_ch[ch];
    int underrun_time = 30;
    u32 reference_time = bt_audio_conn_clock_time(remote_bt_addr) * 625;
    u32 next_timestamp =  translator_recv_ch_get_timestamp(source);
    int distance_time = next_timestamp - reference_time;
    if (distance_time > 67108863L || distance_time < -67108863L) {
        if (next_timestamp > reference_time) {
            distance_time = next_timestamp - 0xffffffff - reference_time;
        } else {
            distance_time = 0xffffffff - reference_time + next_timestamp;
        }
    }
    if (distance_time <= underrun_time) {
        return true;
    }
    //local_irq_disable();
    if (channel->timer) {
        sys_hi_timeout_del(channel->timer);
        channel->timer = 0;
    }
    channel->timer = sys_hi_timeout_add(channel, translator_recv_audio_underrun_signal, (distance_time - underrun_time));
    //local_irq_enable();

    return 0;
}

struct translator_recv_audio_frame *JL_rcsp_translator_recv_ch_get_frame(u8 source)
{
    struct rx_ch_t *channel;
    u8 ch;
    struct translator_recv_audio_frame *frame = NULL;

    if (!tlr_hdl.state) {
        return NULL;
    }
    ch = source_to_ch_remap(source);
    if (OS_NO_ERR != os_mutex_pend(&tlr_hdl.rx_ch[ch].mutex, 10)) {
        return NULL;
    }
    if (!(tlr_hdl.ch_state & BIT(ch))) {
        goto __exit;
    }
    channel = &tlr_hdl.rx_ch[ch];
    if (channel->ready && channel->sync_play) {
        if (!list_empty(&channel->head)) {
            frame = list_first_entry(&channel->head, struct translator_recv_audio_frame, entry);
        } else {
            if (tws_api_get_role() == TWS_ROLE_MASTER) {
                if (translator_recv_audio_frame_underrun_detect(source)) {
                    putchar('X');
                    channel->ready = 0;
                    channel->sync_play = 0;
                    if ((tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED)) {
                        translator_tws_send_cmd(TWS_TRANS_CMD_MEDIA_SUSPEND, 0, &channel->source, sizeof(u8));
                    } else {
                        int msg[3];
                        msg[0] = (int)translator_tws_recv_ch_suspend;
                        msg[1] = 1;
                        msg[2] = channel->source;
                        os_taskq_post_type("app_core", Q_CALLBACK, 3, msg);
                    }
                }
            } else {
                channel->ready = 0;
                channel->sync_play = 0;
            }
        }
    }
__exit:
    os_mutex_post(&tlr_hdl.rx_ch[ch].mutex);
    return frame;
}

int JL_rcsp_translator_recv_ch_free_frame(u8 source, struct translator_recv_audio_frame *frame)
{
    struct rx_ch_t *channel;
    u8 ch;
    int ret = 0;

    if (!tlr_hdl.state) {
        return 0;
    }
    if (frame == NULL) {
        return -EFAULT;
    }
    ch = source_to_ch_remap(source);
    if (OS_NO_ERR != os_mutex_pend(&tlr_hdl.rx_ch[ch].mutex, 10)) {
        return 0;
    }
    if (!(tlr_hdl.ch_state & BIT(ch))) {
        ret = 0;
        goto __exit;
    }
    channel = &tlr_hdl.rx_ch[ch];
    channel->recv_size -= frame->size;
#if RECV_SPEED_DEBUG
    g_printf("get[%d]: %d %d\n", ch, frame->size, channel->recv_size);
#endif
    if (channel->full) {
        if (channel->recv_size <= channel->max_size * 3 / 4) {
            //消耗一定量缓存之后通知APP恢复发数
            u32 free_size = channel->max_size - channel->recv_size;
            _translator_op_inform_cache_free_size(frame->source, free_size);
            channel->full = 0;
        }
    }
    list_del(&frame->entry);
    free(frame);
__exit:
    os_mutex_post(&tlr_hdl.rx_ch[ch].mutex);
    return ret;
}

int JL_rcsp_translator_set_play_volume(u16 volume)
{
    struct volume_cfg vcfg;

    if ((tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED)) {
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            translator_tws_send_cmd(TWS_TRANS_CMD_SET_VOLUME, 0, (u8 *)&volume, 2);
        }
    }

    rcsp_printf("translator set volume to %d\n", volume);
    g_bt_volume = volume;
    vcfg.bypass = VOLUME_NODE_CMD_SET_VOL;
    vcfg.cur_vol = volume;
    return jlstream_set_node_param(NODE_UUID_VOLUME_CTRLER, "Vol_AIVoice", &vcfg, sizeof(vcfg));
}

void JL_rcsp_translator_update_play_volume(u8 source)
{
    switch (tlr_hdl.mode_info.mode) {
    case RCSP_TRANSLATOR_MODE_RECORD_TRANSLATION:
    case RCSP_TRANSLATOR_MODE_FACE_TO_FACE_TRANSLATION:
        JL_rcsp_translator_set_play_volume(g_bt_volume);
        break;
    }
}

void JL_rcsp_translator_set_decode_resume_handler(u8 source, void *priv, void (*func)(void *priv, u8 source))
{
    u8 ch = source_to_ch_remap(source);
    struct rx_ch_t *channel = &tlr_hdl.rx_ch[ch];

    channel->cb_priv = priv;
    channel->recv_decode_resume = func;
}

static int translator_send_ch_send(u8 source, u8 *buf, u32 len)
{
    int ret = 0;
    u8 ch;
    struct tx_ch_t *channel = NULL;
    u32 left_len = 0;

    if (!tlr_hdl.state) {
        return 0;
    }
    ch = source_to_ch_remap(source);
    if (OS_NO_ERR != os_mutex_pend(&tlr_hdl.tx_ch[ch].mutex, 10)) {
        return -1;
    }
    if (tlr_hdl.ch_state & BIT(4 + ch)) {
        channel = &tlr_hdl.tx_ch[ch];
        if (len > channel->buf_size - channel->offset) {
            left_len = len - (channel->buf_size - channel->offset);
            len -= left_len;
            if (left_len > channel->buf_size) {
                r_printf("trans send overflow!!! source %x, ch %d\n", source, ch);
                ret = -1;
                goto __exit;
            }
        }
        memcpy(channel->cbuf_buf + channel->offset, buf, len);
        channel->offset += len;
        if (channel->offset == channel->buf_size) {
            channel->offset = 0;
            //rcsp_printf("send: len: %d source: %d ch: %d\n", channel->buf_size, source, ch);
            ret = _translator_op_send_audio_data(channel->cbuf_buf, channel->buf_size, source, 0);
            if (ret) {
                ret = -1;
            }
        }
        if (left_len) {
            memcpy(channel->cbuf_buf, buf + len, left_len);
            channel->offset = left_len;
        }
    }
__exit:
    os_mutex_post(&tlr_hdl.tx_ch[ch].mutex);
    return ret;
}

static int translator_send_ch_for_dev_mic(u8 *buf, u32 len)
{
    return translator_send_ch_send(RCSP_TRANSLATOR_AUDIO_SOURCE_DEV_MIC, buf, len);
}

static int translator_send_ch_for_esco_upstream(u8 *buf, u32 len)
{
    return translator_send_ch_send(RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_UPSTREAM, buf, len);
}

static int translator_send_ch_for_esco_downstream(u8 *buf, u32 len)
{
    return translator_send_ch_send(RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_DOWNSTREAM, buf, len);
}

static int translator_send_ch_for_a2dp(u8 *buf, u32 len)
{
    return translator_send_ch_send(RCSP_TRANSLATOR_AUDIO_SOURCE_MSBC, buf, len);
}

static int translator_send_ch_for_phone_mic(u8 *buf, u32 len)
{
    return translator_send_ch_send(RCSP_TRANSLATOR_AUDIO_SOURCE_PHONE_MIC, buf, len);
}

static int translator_send_ch_try_to_set_func(struct tx_ch_t *channel)
{
    int done = 0;
    switch (tlr_hdl.mode_info.mode) {
    case RCSP_TRANSLATOR_MODE_RECORD:
    case RCSP_TRANSLATOR_MODE_RECORD_TRANSLATION:
        //目前还是用mic_rec_pram_init()方式注册回调，如果改用send_ch方式，注意替代
        //ai_mic_rec_start() --> translator_send_ch_start() + ai_voice_recoder_open()
        //ai_mic_rec_close() --> translator_send_ch_stop() + ai_voice_recoder_close()
        //ai_voice_recoder_set_ai_tx_node_func(translator_send_ch_for_dev_mic);
        done = 1;
        break;
    case RCSP_TRANSLATOR_MODE_A2DP_TRANSLATION:
        if (a2dp_player_runing()) {
            a2dp_player_set_ai_tx_node_func(translator_send_ch_for_a2dp);
            done = 1;
        }
        break;
    case RCSP_TRANSLATOR_MODE_CALL_TRANSLATION:
        switch (channel->source) {
        case RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_UPSTREAM:
            if (esco_recoder_running()) {
                esco_recoder_set_ai_tx_node_func(translator_send_ch_for_esco_upstream);
                done = 1;
            }
            break;
        case RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_DOWNSTREAM:
            if (esco_player_runing()) {
                esco_player_set_ai_tx_node_func(translator_send_ch_for_esco_downstream);
                done = 1;
            }
            break;
        }
        break;
    case RCSP_TRANSLATOR_MODE_FACE_TO_FACE_TRANSLATION:
        //TODO
        done = 1;
        break;
    default:
        done = 1;
    }
    return done;
}

static void translator_send_ch_delay_set_func(void *priv)
{
    struct tx_ch_t *channel = priv;
    int done = 0;
    done = translator_send_ch_try_to_set_func(channel);
    if (done && channel->timer_id) {
        sys_timer_del(channel->timer_id);
        channel->timer_id = 0;
    }
}

static int translator_send_ch_start(u8 source)
{
    int ret = 0;
    u8 ch;
    struct tx_ch_t *channel = NULL;
    u32 fsize = 40;
    u32 size;

    if (tws_api_get_role() == TWS_ROLE_SLAVE) {
        return 0;
    }
    ch = source_to_ch_remap(source);
    os_mutex_pend(&tlr_hdl.tx_ch[ch].mutex, 0);
    if (tlr_hdl.ch_state & BIT(4 + ch)) {
        os_mutex_post(&tlr_hdl.tx_ch[ch].mutex);
        return 0;
    }
    rcsp_printf("rcsp start encode in: source %x, ch %d\n", source, ch);
    channel = &tlr_hdl.tx_ch[ch];
    if (tlr_hdl.mode_info.encode == RCSP_TRANSLATOR_ENCODE_TYPE_OPUS) {
        fsize = 40;
    } else if (tlr_hdl.mode_info.encode == RCSP_TRANSLATOR_ENCODE_TYPE_PCM) {
        fsize = 320 * 2;  //320采样点 16bit 1ch
    } else if (tlr_hdl.mode_info.encode == RCSP_TRANSLATOR_ENCODE_TYPE_SPEEX) {
        //TODO
        fsize = 200;
    } else if (tlr_hdl.mode_info.encode == RCSP_TRANSLATOR_ENCODE_TYPE_MSBC) {
        //TODO
        fsize = 200;
    } else if (tlr_hdl.mode_info.encode == RCSP_TRANSLATOR_ENCODE_TYPE_JLA_V2) {
        fsize = 40 + 2;
    } else {
        ret = -1;
        goto __err_exit;
    }
    size = fsize * 5;
    channel->cbuf_buf = malloc(size);
    if (channel->cbuf_buf == NULL) {
        ret = -1;
        goto __err_exit;
    }
    channel->buf_size = size;
    channel->offset = 0;
    channel->source = source;
    //设置AI_TX节点发送回调函数，如果a2dp / esco未打开不能设进去回调函数，
    //就设定时器定时查询直到设入成功
    if (0 == translator_send_ch_try_to_set_func(channel)) {
        channel->timer_id = sys_timer_add(channel, translator_send_ch_delay_set_func, 4);
    }
    rcsp_printf("rcsp start encode out: source %x, ch %d\n", source, ch);
    tlr_hdl.ch_state |= BIT(4 + ch);
    os_mutex_post(&tlr_hdl.tx_ch[ch].mutex);

    return 0;

__err_exit:
    rcsp_printf("rcsp start encode fail: source %x, ch %d\n", source, ch);
    if (channel) {
        if (channel->cbuf_buf) {
            free(channel->cbuf_buf);
            channel->cbuf_buf = NULL;
        }
    }
    os_mutex_post(&tlr_hdl.tx_ch[ch].mutex);
    return ret;
}

static void translator_send_ch_stop(u8 source)
{
    struct tx_ch_t *channel;
    u8 ch;

    if (tws_api_get_role() == TWS_ROLE_SLAVE) {
        return;
    }
    ch = source_to_ch_remap(source);
    os_mutex_pend(&tlr_hdl.tx_ch[ch].mutex, 0);
    if (!(tlr_hdl.ch_state & BIT(4 + ch))) {
        os_mutex_post(&tlr_hdl.tx_ch[ch].mutex);
        return;
    }
    rcsp_printf("rcsp stop encode: source %x, ch %d\n", source, ch);
    channel = &tlr_hdl.tx_ch[ch];
    if (channel->cbuf_buf) {
        free(channel->cbuf_buf);
        channel->cbuf_buf = NULL;
    }
    if (channel->timer_id) {
        sys_timer_del(channel->timer_id);
        channel->timer_id = 0;
    }
    tlr_hdl.ch_state &= ~BIT(4 + ch);
    os_mutex_post(&tlr_hdl.tx_ch[ch].mutex);
}

u32 JL_rcsp_translator_get_status()
{
    return tlr_hdl.state;
}

void JL_rcsp_translator_get_mode_info(struct translator_mode_info *minfo)
{
    memcpy(minfo, &tlr_hdl.mode_info, sizeof(*minfo));
}

int JL_rcsp_translator_manual_record_start()
{
    int ret = -1;
    if ((tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) && tws_api_get_role() == TWS_ROLE_SLAVE) {
        translator_tws_send_cmd(TWS_TRANS_CMD_MANUAL_REC_START, 0, NULL, 0);
        return 0;
    }
    if (tlr_hdl.state) {
#if MIC_REC_METHOD_NEW
        ret = translator_send_ch_start(RCSP_TRANSLATOR_AUDIO_SOURCE_DEV_MIC);
        if (ret) {
            goto __err_exit;
        }
        //TODO ai_voice_recoder_open()需要在后续版本修改支持多路
        ret = ai_voice_recoder_open(AUDIO_CODING_OPUS, 0);
        if (ret) {
            goto __err_exit;
        }
        ai_voice_recoder_set_ai_tx_node_func(translator_send_ch_for_dev_mic);
#else
        ret = ai_mic_rec_start();
        if (ret) {
            goto __err_exit;
        }
#endif
        tlr_hdl.record_state = 1;
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            translator_tws_send_cmd(TWS_TRANS_CMD_SYNC_REC_STA, 0, &tlr_hdl.record_state, 1);
        }
    }

__err_exit:
    return ret;
}

int JL_rcsp_translator_manual_record_stop()
{
    int ret = 0;
    if ((tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) && tws_api_get_role() == TWS_ROLE_SLAVE) {
        translator_tws_send_cmd(TWS_TRANS_CMD_MANUAL_REC_STOP, 0, NULL, 0);
        return 0;
    }
#if MIC_REC_METHOD_NEW
    ai_voice_recoder_close();
    translator_send_ch_stop(RCSP_TRANSLATOR_AUDIO_SOURCE_DEV_MIC);
#else
    if (ai_mic_is_busy()) {
        ret = ai_mic_rec_close();
    }
#endif
    tlr_hdl.record_state = 0;
    if (tws_api_get_role() == TWS_ROLE_MASTER) {
        translator_tws_send_cmd(TWS_TRANS_CMD_SYNC_REC_STA, 0, &tlr_hdl.record_state, 1);
    }
    return ret;
}

int JL_rcsp_translator_whether_play_by_ai_rx()
{
    return !!TRANSLATION_RECV_CHANNEL_ENABLE;
}

static int translator_todo_list_init()
{
    INIT_LIST_HEAD(&tlr_hdl.todo_list_head);
    return 0;
}

static int translator_todo_list_add(u8 cmd, u8 *param, u32 param_len)
{
    struct todo_list *todo;

    if (tlr_hdl.todo_list_head.next == NULL) {
        return -EFAULT;
    }
    todo = zalloc(sizeof(*todo) + param_len);
    if (!todo) {
        return -ENOMEM;
    }
    todo->cmd = cmd;
    todo->param = (u8 *)(todo + 1);
    todo->param_len = param_len;
    memcpy(todo->param, param, param_len);
    list_add_tail(&todo->entry, &tlr_hdl.todo_list_head);
    return 0;
}

static struct todo_list *translator_todo_list_find(u8 cmd)
{
    struct todo_list *p;

    if (tlr_hdl.todo_list_head.next == NULL) {
        return NULL;
    }
    list_for_each_entry(p, &tlr_hdl.todo_list_head, entry) {
        if (p->cmd == cmd) {
            return p;
        }
    }
    return NULL;
}

static int translator_todo_list_del(struct todo_list *todo)
{
    if (todo == NULL) {
        return -EFAULT;
    }
    list_del(&todo->entry);
    free(todo);
    return 0;
}

static int translator_todo_list_clear()
{
    struct todo_list *p, *n;

    if (tlr_hdl.todo_list_head.next == NULL) {
        return -EFAULT;
    }
    list_for_each_entry_safe(p, n, &tlr_hdl.todo_list_head, entry) {
        list_del(&p->entry);
        free(p);
    }
    return 0;
}

static void translator_set_mode_delay(void *arg)
{
    struct todo_list *todo;
    if (0 == a2dp_player_runing()) {
        //TODO  mute住a2dp
        sys_timer_del(tlr_hdl.detect_timer_id);
        tlr_hdl.detect_timer_id = 0;
    }
}

static void translator_stop_a2dp_player()
{
    bt_cmd_prepare(USER_CTRL_AVCTP_OPID_PAUSE, 0, NULL);
    if (!tlr_hdl.detect_timer_id) {
        tlr_hdl.detect_timer_id = sys_timer_add(NULL, translator_set_mode_delay, 50);
    }
}

static int _translator_op_get_info(u8 *buf, u32 len)
{
    struct translator_mode_info *minfo = &tlr_hdl.mode_info;
    buf[0] = minfo->mode;
    buf[1] = minfo->encode;
    buf[2] = minfo->ch;
    buf[3] = (minfo->sr >> 24) & 0xff;
    buf[4] = (minfo->sr >> 16) & 0xff;
    buf[5] = (minfo->sr >> 8) & 0xff;
    buf[6] = (minfo->sr >> 0) & 0xff;
    rcsp_printf("get translator info:\nmode: %d, encode: %d, ch: %d, sr: %d\n",
                minfo->mode,
                minfo->encode,
                minfo->ch,
                minfo->sr);
    return 0;
}

static int _translator_op_set_mode(u8 *buf, u32 len)
{
    struct translator_mode_info *minfo = &tlr_hdl.mode_info;
    u8 last_mode;
    int ret = 0;
    u8 source_tx[2] = {0xff, 0xff};
    u8 source_rx[2] = {0xff, 0xff};

    if (minfo->mode == buf[0]) {
        return -TRANS_SET_MODE_STATUS_IN_MODE;
    }
    last_mode = minfo->mode;
    minfo->mode = buf[0];
    minfo->encode = buf[1];
    minfo->ch = buf[2];
    minfo->sr = READ_BIG_U32(buf + 3);
    rcsp_printf("set translator mode:\nmode: %d, encode: %d, ch: %d, sr: %d\n",
                minfo->mode,
                minfo->encode,
                minfo->ch,
                minfo->sr);
    if (minfo->mode == RCSP_TRANSLATOR_MODE_RECORD) {
        if (a2dp_player_runing()) {
            rcsp_printf("a2dp is running\n");
            translator_stop_a2dp_player();
        } else if (esco_player_runing()) {
            rcsp_printf("esco is running\n");
#if TRANSLATION_CALL_CONTROL_BY_DEVICE
            ret = -TRANS_SET_MODE_STATUS_IN_CALL;
            goto __err_exit;
#endif
        }
        tlr_hdl.state = 1;
    } else if (minfo->mode == RCSP_TRANSLATOR_MODE_RECORD_TRANSLATION) {
        if (a2dp_player_runing()) {
            rcsp_printf("a2dp is running\n");
            translator_stop_a2dp_player();
        } else if (esco_player_runing()) {
            rcsp_printf("esco is running\n");
#if TRANSLATION_CALL_CONTROL_BY_DEVICE
            ret = -TRANS_SET_MODE_STATUS_IN_CALL;
            goto __err_exit;
#endif
        }
        if (tlr_hdl.state == 0) {
#if TRANSLATION_RECV_CHANNEL_ENABLE
            ret = translator_recv_ch_start(RCSP_TRANSLATOR_AUDIO_SOURCE_DEV_MIC);
            if (ret) {
                ret = -TRANS_SET_MODE_STATUS_FAIL;
                goto __err_exit;
            }
            source_rx[0] = RCSP_TRANSLATOR_AUDIO_SOURCE_DEV_MIC;
#endif
            tlr_hdl.state = 1;
        }
    } else if (minfo->mode == RCSP_TRANSLATOR_MODE_CALL_TRANSLATION) {
        if (a2dp_player_runing()) {
            rcsp_printf("a2dp is running\n");
            translator_stop_a2dp_player();
        }
        if (tlr_hdl.state == 0) {
            ret = translator_send_ch_start(RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_UPSTREAM);
            if (ret) {
                ret = -TRANS_SET_MODE_STATUS_FAIL;
                goto __err_exit;
            }
            source_tx[0] = RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_UPSTREAM;
            ret = translator_recv_ch_start(RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_UPSTREAM);
            if (ret) {
                ret = -TRANS_SET_MODE_STATUS_FAIL;
                goto __err_exit;
            }
            source_rx[0] = RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_UPSTREAM;
            ret = translator_send_ch_start(RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_DOWNSTREAM);
            if (ret) {
                ret = -TRANS_SET_MODE_STATUS_FAIL;
                goto __err_exit;
            }
            source_tx[1] = RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_DOWNSTREAM;
            ret = translator_recv_ch_start(RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_DOWNSTREAM);
            if (ret) {
                ret = -TRANS_SET_MODE_STATUS_FAIL;
                goto __err_exit;
            }
            source_rx[1] = RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_DOWNSTREAM;
            tlr_hdl.state = 1;
        }
    } else if (minfo->mode == RCSP_TRANSLATOR_MODE_A2DP_TRANSLATION) {
        if (esco_player_runing()) {
            rcsp_printf("esco is running\n");
#if TRANSLATION_CALL_CONTROL_BY_DEVICE
            ret = -TRANS_SET_MODE_STATUS_IN_CALL;
            goto __err_exit;
#endif
        }
        if (tlr_hdl.state == 0) {
            ret = translator_send_ch_start(RCSP_TRANSLATOR_AUDIO_SOURCE_MSBC);
            if (ret) {
                ret = -TRANS_SET_MODE_STATUS_FAIL;
                goto __err_exit;
            }
            source_tx[0] = RCSP_TRANSLATOR_AUDIO_SOURCE_MSBC;
#if TRANSLATION_RECV_CHANNEL_ENABLE
#if A2DP_TRANSLATION_RECV_ENABLE
            ret = translator_recv_ch_start(RCSP_TRANSLATOR_AUDIO_SOURCE_MSBC);
            if (ret) {
                ret = -TRANS_SET_MODE_STATUS_FAIL;
                goto __err_exit;
            }
            source_rx[0] = RCSP_TRANSLATOR_AUDIO_SOURCE_MSBC;
#endif
#endif
            tlr_hdl.state = 1;
        }
    } else if (minfo->mode == RCSP_TRANSLATOR_MODE_FACE_TO_FACE_TRANSLATION) {
        if (a2dp_player_runing()) {
            rcsp_printf("a2dp is running\n");
            translator_stop_a2dp_player();
        } else if (esco_player_runing()) {
            rcsp_printf("esco is running\n");
#if TRANSLATION_CALL_CONTROL_BY_DEVICE
            ret = -TRANS_SET_MODE_STATUS_IN_CALL;
            goto __err_exit;
#endif
        }
        if (tlr_hdl.state == 0) {
            //ret = ai_mic_rec_start();
            //if (ret) {
            //    goto __err_exit;
            //}
#if TRANSLATION_RECV_CHANNEL_ENABLE
            ret = translator_recv_ch_start(RCSP_TRANSLATOR_AUDIO_SOURCE_PHONE_MIC);
            if (ret) {
                ret = -TRANS_SET_MODE_STATUS_FAIL;
                goto __err_exit;
            }
            source_rx[0] = RCSP_TRANSLATOR_AUDIO_SOURCE_PHONE_MIC;
#endif
            tlr_hdl.state = 1;
        }
    } else if (minfo->mode == RCSP_TRANSLATOR_MODE_IDLE) {
        ret = 0;
        if (last_mode == RCSP_TRANSLATOR_MODE_RECORD) {
            goto __err_exit;
        } else if (last_mode == RCSP_TRANSLATOR_MODE_RECORD_TRANSLATION) {
#if TRANSLATION_RECV_CHANNEL_ENABLE
            source_rx[0] = RCSP_TRANSLATOR_AUDIO_SOURCE_DEV_MIC;
#endif
            goto __err_exit;
        } else if (last_mode == RCSP_TRANSLATOR_MODE_CALL_TRANSLATION) {
            if (tws_api_get_role() == TWS_ROLE_MASTER) {
                source_tx[0] = RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_UPSTREAM;
                source_rx[0] = RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_UPSTREAM;
            }
            source_tx[1] = RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_DOWNSTREAM;
            source_rx[1] = RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_DOWNSTREAM;
            goto __err_exit;
        } else if (last_mode == RCSP_TRANSLATOR_MODE_A2DP_TRANSLATION) {
            source_tx[0] = RCSP_TRANSLATOR_AUDIO_SOURCE_MSBC;
#if TRANSLATION_RECV_CHANNEL_ENABLE
#if A2DP_TRANSLATION_RECV_ENABLE
            source_rx[0] = RCSP_TRANSLATOR_AUDIO_SOURCE_MSBC;
#endif
#endif
            goto __err_exit;
        } else if (last_mode == RCSP_TRANSLATOR_MODE_FACE_TO_FACE_TRANSLATION) {
#if TRANSLATION_RECV_CHANNEL_ENABLE
            source_rx[0] = RCSP_TRANSLATOR_AUDIO_SOURCE_PHONE_MIC;
#endif
            goto __err_exit;
        }
    }
    bt_sniff_disable();
    if ((tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED)) {
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            translator_sync_remote_bt_addr();
            if (minfo->mode != RCSP_TRANSLATOR_MODE_CALL_TRANSLATION) {
                translator_tws_send_cmd(TWS_TRANS_CMD_SET_MODE, 0, buf, len);
            }
        }
    }
    return 0;

__err_exit:
    bt_sniff_enable();
    if (source_tx[1] != 0xff) {
        translator_send_ch_stop(source_tx[1]);
    }
    if (source_tx[0] != 0xff) {
        translator_send_ch_stop(source_tx[0]);
    }
    if (source_rx[1] != 0xff) {
        translator_recv_ch_stop(source_rx[1]);
    }
    if (source_rx[0] != 0xff) {
        translator_recv_ch_stop(source_rx[0]);
    }
#if MIC_REC_METHOD_NEW == 0
    if (ai_mic_is_busy()) {
        ai_mic_rec_close();
    }
#endif
    tlr_hdl.state = 0;
    if (ret != 0) {
        minfo->mode = RCSP_TRANSLATOR_MODE_IDLE;
    } else {
        //退出模式通知tws从机
        if ((tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED)) {
            translator_sync_remote_bt_addr();
            if (tws_api_get_role() == TWS_ROLE_MASTER) {
                translator_tws_send_cmd(TWS_TRANS_CMD_SET_MODE, 0, buf, len);
            }
        }
    }
    return ret;
}

static int _translator_op_record_mode_start(u8 update_state)
{
    struct translator_mode_info *minfo = &tlr_hdl.mode_info;
    int ret = -1;

    if (minfo->mode == RCSP_TRANSLATOR_MODE_RECORD ||
        minfo->mode == RCSP_TRANSLATOR_MODE_RECORD_TRANSLATION ||
        minfo->mode == RCSP_TRANSLATOR_MODE_FACE_TO_FACE_TRANSLATION) {
        rcsp_printf("rcsp start mic record\n");
#if MIC_REC_METHOD_NEW
        ret = translator_send_ch_start(RCSP_TRANSLATOR_AUDIO_SOURCE_DEV_MIC);
        if (ret) {
            goto __err_exit;
        }
        //TODO ai_voice_recoder_open()需要在后续版本修改支持多路
        ret = ai_voice_recoder_open(AUDIO_CODING_OPUS, 0);
        if (ret) {
            goto __err_exit;
        }
        ai_voice_recoder_set_ai_tx_node_func(translator_send_ch_for_dev_mic);
#else
        ret = ai_mic_rec_start();
        if (ret) {
            goto __err_exit;
        }
#endif
        if (update_state) {
            tlr_hdl.record_state = 1;
            if (tws_api_get_role() == TWS_ROLE_MASTER) {
                translator_tws_send_cmd(TWS_TRANS_CMD_SYNC_REC_STA, 0, &tlr_hdl.record_state, 1);
            }
        }
    }
    return ret;

__err_exit:
#if MIC_REC_METHOD_NEW
    ai_voice_recoder_close();
    translator_send_ch_stop(RCSP_TRANSLATOR_AUDIO_SOURCE_DEV_MIC);
#else
    if (ai_mic_is_busy()) {
        ai_mic_rec_close();
    }
#endif
    return ret;
}

static int _translator_op_record_mode_stop(u8 update_state)
{
    struct translator_mode_info *minfo = &tlr_hdl.mode_info;
    int ret = -1;

    if (minfo->mode == RCSP_TRANSLATOR_MODE_RECORD ||
        minfo->mode == RCSP_TRANSLATOR_MODE_RECORD_TRANSLATION ||
        minfo->mode == RCSP_TRANSLATOR_MODE_FACE_TO_FACE_TRANSLATION ||
        minfo->mode == RCSP_TRANSLATOR_MODE_IDLE) {
        rcsp_printf("rcsp stop mic record\n");
#if MIC_REC_METHOD_NEW
        ai_voice_recoder_close();
        translator_send_ch_stop(RCSP_TRANSLATOR_AUDIO_SOURCE_DEV_MIC);
#else
        ai_mic_rec_close();
#endif
        if (update_state) {
            tlr_hdl.record_state = 0;
            if (tws_api_get_role() == TWS_ROLE_MASTER) {
                translator_tws_send_cmd(TWS_TRANS_CMD_SYNC_REC_STA, 0, &tlr_hdl.record_state, 1);
            }
        }
        ret = 0;
    }
    return ret;
}

int JL_rcsp_translator_op_inform_mode_info(struct translator_mode_info *info)
{
    int ret;
    u16 len;
    u8 buf[16] = {0};

    if ((tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) && tws_api_get_role() == TWS_ROLE_SLAVE) {
        translator_tws_send_cmd(TWS_TRANS_CMD_INFORM_MODE, 0, (u8 *)info, sizeof(*info));
        return 0;
    }
    rcsp_printf("rcsp inform info:\nmode: %d, encode: %d, ch: %d, sr: %d\n",
                info->mode,
                info->encode,
                info->ch,
                info->sr);
    len = sizeof(struct translator_mode_info);
    buf[0] = RCSP_TRANSLATOR_OP_INFORM_MODE_INFO;
    buf[1] = len;
    buf[2] = info->mode;
    buf[3] = info->encode;
    buf[4] = info->ch;
    buf[5] = (info->sr >> 24) & 0xff;
    buf[6] = (info->sr >> 16) & 0xff;
    buf[7] = (info->sr >> 8) & 0xff;
    buf[8] = (info->sr >> 0) & 0xff;
    len += 2;
    ret = JL_CMD_send(JL_OPCODE_TRANSLATOR, buf, len, JL_NOT_NEED_RESPOND, 0, NULL);
    return ret;
}

static int _translator_op_get_cache_free_size(u8 source, u32 *free_size)
{
    int ret = 0;
    u8 ch;
    struct rx_ch_t *channel = NULL;

    if (!tlr_hdl.state) {
        return -EFAULT;
    }
    ch = source_to_ch_remap(source);
    //os_mutex_pend(&tlr_hdl.rx_ch[ch].mutex, 0);
    if (!(tlr_hdl.ch_state & BIT(ch))) {
        //os_mutex_post(&tlr_hdl.rx_ch[ch].mutex);
        *free_size = 0;
        return -ENOMEM;
    }
    channel = &tlr_hdl.rx_ch[ch];
    *free_size = channel->max_size - channel->recv_size;
#if RECV_SPEED_DEBUG
    g_printf("free_size[%d]: %d\n", ch, *free_size);
#endif
    if (*free_size == 0) {
        //缓存已满，APP挂起发数，等待设备通知APP解除挂起
        channel->full = 1;
    }
    //os_mutex_post(&tlr_hdl.rx_ch[ch].mutex);
    return ret;
}

static int _translator_op_inform_cache_free_size(u8 source, u32 free_size)
{
    int ret = 0;
    u8 ch;
    u8 buf[16] = {0};

    ch = source_to_ch_remap(source);
#if RECV_SPEED_DEBUG
    g_printf("wakeup[%d]: %d\n", ch, free_size);
#endif
    buf[0] = RCSP_TRANSLATOR_OP_INFORM_CACHE_FREE_SIZE;
    buf[1] = 5;
    buf[2] = source;
    free_size = READ_BIG_U32(&free_size);
    memcpy(buf + 3, &free_size, 4);
    ret = JL_CMD_send(JL_OPCODE_TRANSLATOR, buf, 7, JL_NOT_NEED_RESPOND, 0, NULL);
    return ret;
}

static int _translator_op_receive_audio_data(u8 *buf, u32 len)
{
    struct translator_mode_info *minfo = &tlr_hdl.mode_info;
    u16 ret = 0;
    u32 audio_len;
    u8 *audio_buf;
    struct translator_op_03_audio_format op03_fmt = {0};
    u16 crc;
    u32 timestamp;

    if (tlr_hdl.state == 0) {
        return 0;
    }
    memcpy(&op03_fmt, buf, sizeof(struct translator_op_03_audio_format));
    //TODO 注意有没有非对齐问题，crc的offset是3。目前校验pass
    op03_fmt.crc = READ_BIG_U16(&op03_fmt.crc);
    op03_fmt.len = READ_BIG_U16(&op03_fmt.len);
    if (len != sizeof(struct translator_op_03_audio_format) + op03_fmt.len) {
        rcsp_printf("trans recv len not match\n");
        return -1;
    }
    crc = CRC16(buf + sizeof(struct translator_op_03_audio_format), op03_fmt.len);
    if (op03_fmt.crc == crc) {
        //rcsp_printf("opCode 0x34, op 0x03 rx crc ok\n");
        audio_len = op03_fmt.len;
        audio_buf = buf + sizeof(struct translator_op_03_audio_format);
        //rcsp_put_buf(audio_buf, audio_len);
#if (ENC_TO_DEC_TEST == 0)
        timestamp = translator_recv_ch_get_timestamp(op03_fmt.source);
        translator_recv_ch_put_frame(op03_fmt.source, audio_buf, audio_len, timestamp);
        translator_recv_ch_update_timestamp(op03_fmt.source, audio_len);
#endif
    } else {
        rcsp_printf("opCode 0x34, op 0x03 rx crc error, %x - %x\n", op03_fmt.crc, crc);
        ret = -1;
    }
    return ret;
}

static int _translator_op_send_audio_data(u8 *buf, u32 len, u8 source, u8 count)
{
    struct translator_mode_info *minfo = &tlr_hdl.mode_info;
    int ret = -1;
    u32 param_len;
    u8 *param;
    u32 ch;
    struct translator_op_03_audio_format op03_fmt = {0};

    if (tlr_hdl.state == 0) {
        return -1;
    }
    switch (minfo->mode) {
    case RCSP_TRANSLATOR_MODE_RECORD:
    case RCSP_TRANSLATOR_MODE_RECORD_TRANSLATION:
        ret = JL_DATA_send(JL_OPCODE_DATA, 0x04, buf, len, JL_NOT_NEED_RESPOND, 0, NULL);
        break;
    case RCSP_TRANSLATOR_MODE_CALL_TRANSLATION:
    case RCSP_TRANSLATOR_MODE_A2DP_TRANSLATION:
    case RCSP_TRANSLATOR_MODE_FACE_TO_FACE_TRANSLATION:
        if (minfo->mode == RCSP_TRANSLATOR_MODE_CALL_TRANSLATION) {
            op03_fmt.source = RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_UPSTREAM;
        } else if (minfo->mode == RCSP_TRANSLATOR_MODE_A2DP_TRANSLATION) {
            op03_fmt.source = RCSP_TRANSLATOR_AUDIO_SOURCE_MSBC;
        } else if (minfo->mode == RCSP_TRANSLATOR_MODE_FACE_TO_FACE_TRANSLATION) {
            op03_fmt.source = RCSP_TRANSLATOR_AUDIO_SOURCE_DEV_MIC;
        }
        if (source != 0xff) {
            op03_fmt.source = source;
        }
        //TODO 确认后期会不会换编码格式
        op03_fmt.encode = minfo->encode;
        //TODO
        op03_fmt.count = count;
        op03_fmt.crc = CRC16(buf, len);
        op03_fmt.crc = READ_BIG_U16(&op03_fmt.crc);
        op03_fmt.len = READ_BIG_U16(&len);
        param_len = 2 + sizeof(struct translator_op_03_audio_format) + len;
        param = zalloc(param_len);
        if (!param) {
            return -1;
        }
        param[0] = RCSP_TRANSLATOR_OP_SEND_AUDIO_DATA;
        //TODO 有超长度的风险
        param[1] = sizeof(struct translator_op_03_audio_format) + len;
        memcpy(param + 2, &op03_fmt, sizeof(struct translator_op_03_audio_format));
        memcpy(param + 2 + sizeof(struct translator_op_03_audio_format), buf, len);
        ret = JL_CMD_send(JL_OPCODE_TRANSLATOR, param, param_len, JL_NOT_NEED_RESPOND, 0, NULL);
        free(param);
        break;
    case RCSP_TRANSLATOR_MODE_IDLE:
        break;
    }
#if ENC_TO_DEC_TEST  //编码数据送去解码测试
    u32 timestamp = translator_recv_ch_get_timestamp(op03_fmt.source);
    translator_recv_ch_put_frame(op03_fmt.source, buf, len, timestamp);
    translator_recv_ch_update_timestamp(op03_fmt.source, len);
#endif
    return ret;
}

static u16 translator_send_mic_data(u8 *buf, u16 len)
{
    if (_translator_op_send_audio_data(buf, len, 0xff, 0)) {
        return 1;
    }
    return 0;
}

int JL_rcsp_translator_functions(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len, u16 ble_con_handle, u8 *spp_remote_addr)
{
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    u8 op;
    u32 param_len;
    u8 *param;
    u32 resp_len;
    u8 *resp_buf;
    u32 free_size;
    int err;
    int ret = 0;
    u8 ch;
    int mutex_ret = 0;

#define check_resp_alloc(resp_buf) \
    if (resp_buf == NULL) { \
        /*ASSERT(resp_buf, "malloc fail\n");*/ \
        ret = JL_PRO_STATUS_FAIL; \
        goto __err_exit; \
    }

    if (rcspModel == NULL) {
        return JL_PRO_STATUS_FAIL;
    }

    tlr_hdl.ble_con_handle = ble_con_handle;
    if (spp_remote_addr) {
        memcpy(tlr_hdl.spp_remote_addr, spp_remote_addr, 6);
    }

    switch (OpCode) {
    case JL_OPCODE_TRANSLATOR:
        //printf("%s() %d\n", __func__, __LINE__);
        op = data[0];
        param_len = data[1];
        param = data + 2;
        switch (op) {
        case RCSP_TRANSLATOR_OP_GET_MODE_INFO:
            //printf("%s() %d\n", __func__, __LINE__);
            resp_len = 2 + sizeof(struct translator_mode_info);
            resp_buf = zalloc(resp_len);
            check_resp_alloc(resp_buf);
            resp_buf[0] = op;
            resp_buf[1] = resp_len - 2;
            _translator_op_get_info(resp_buf + 2, resp_len - 2);
            ret = JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, resp_buf, resp_len, ble_con_handle, spp_remote_addr);
            free(resp_buf);
            break;
        case RCSP_TRANSLATOR_OP_SET_MODE:
            //printf("%s() %d\n", __func__, __LINE__);
            resp_len = 3;
            resp_buf = zalloc(resp_len);
            check_resp_alloc(resp_buf);
            resp_buf[0] = op;
            resp_buf[1] = 1;
            err = _translator_op_set_mode(param, param_len);
            if (!err) {
                resp_buf[2] = 0;
            } else {
                resp_buf[2] = -err;
            }
            ret = JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, resp_buf, resp_len, ble_con_handle, spp_remote_addr);
            free(resp_buf);
            break;
        case RCSP_TRANSLATOR_OP_INFORM_MODE_INFO:
            break;
        case RCSP_TRANSLATOR_OP_SEND_AUDIO_DATA:
            //no response
            //printf("%s() %d\n", __func__, __LINE__);
            err = _translator_op_receive_audio_data(param, param_len);
            if (err == 0) {
                ret = JL_PRO_STATUS_SUCCESS;
            } else {
                ret = JL_PRO_STATUS_FAIL;
                goto __exit;
            }
            break;
        case RCSP_TRANSLATOR_OP_GET_CACHE_FREE_SIZE:
            resp_len = 7;
            resp_buf = zalloc(resp_len);
            check_resp_alloc(resp_buf);
            ch = source_to_ch_remap(param[0]);
            mutex_ret = os_mutex_pend(&tlr_hdl.rx_ch[ch].mutex, 10);
            err = _translator_op_get_cache_free_size(param[0], &free_size);
            if (err < 0) {
                free_size = 0;
            }
            resp_buf[0] = RCSP_TRANSLATOR_OP_GET_CACHE_FREE_SIZE;
            resp_buf[1] = 5;
            resp_buf[2] = param[0];
            free_size = READ_BIG_U32(&free_size);
            memcpy(resp_buf + 3, &free_size, 4);
            ret = JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, resp_buf, resp_len, ble_con_handle, spp_remote_addr);
            if (mutex_ret == OS_NO_ERR) {
                os_mutex_post(&tlr_hdl.rx_ch[ch].mutex);
            }
            free(resp_buf);
            break;
        default:
            ret = JL_PRO_STATUS_UNKOWN_CMD;
            goto __exit;
        }
        break;
    case 0x04:  //start record
        param_len = len;
        param = data;
        rcsp_printf("cmd 0x04, encode %x, sr type %x, judge way %x\n", param[0], param[1], param[2]);
        err = _translator_op_record_mode_start(1);
        if (err < 0) {
            ret = JL_PRO_STATUS_FAIL;
            goto __err_exit;
        }
        ret = JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, NULL, 0, ble_con_handle, spp_remote_addr);
        break;
    case 0x05:  //stop record
        param_len = len;
        param = data;
        rcsp_printf("cmd 0x05, stop reason %x\n", param[0]);
        err = _translator_op_record_mode_stop(1);
        if (err < 0) {
            ret = JL_PRO_STATUS_FAIL;
            goto __err_exit;
        }
        ret = JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, NULL, 0, ble_con_handle, spp_remote_addr);
        break;
    default:
        ret = JL_PRO_STATUS_UNKOWN_CMD;
        goto __exit;
    }

__exit:
    return ret;

__err_exit:
    JL_CMD_response_send(OpCode, ret, OpCode_SN, NULL, 0, ble_con_handle, spp_remote_addr);
    return ret;
}

static int translator_charge_event_handler(int *msg)
{
    int ret = false;
    switch (msg[0]) {
    case CHARGE_EVENT_LDO5V_IN:
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            _translator_op_record_mode_stop(0);
        }
        break;
    case CHARGE_EVENT_LDO5V_OFF:
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            if (tlr_hdl.record_state) {
                _translator_op_record_mode_start(0);
            }
        }
        break;
    }
    return ret;
}

APP_MSG_HANDLER(translator_charge_event) = {
    .owner = 0xff,
    .from = MSG_FROM_BATTERY,
    .handler = translator_charge_event_handler,
};

static void translator_bt_event_set_volume(void *arg)
{
    JL_rcsp_translator_set_play_volume(g_bt_volume);
    tlr_hdl.volume_timer_id = 0;
}

static int translator_bt_event_handler(int *event)
{
    struct bt_event *bt = (struct bt_event *)event;
    struct translator_mode_info minfo;

    switch (bt->event) {
    case BT_STATUS_AVRCP_VOL_CHANGE:
        if (tws_api_get_role() == TWS_ROLE_SLAVE) {
            break;
        }
        g_bt_volume = (bt->value + 1) * 16 / 127;
        if (g_bt_volume > 16) {
            g_bt_volume = 16;
        }
        if (tlr_hdl.volume_timer_id) {
            sys_timer_re_run(tlr_hdl.volume_timer_id);
        } else {
            tlr_hdl.volume_timer_id = sys_timeout_add(NULL, translator_bt_event_set_volume, 100);
        }
        break;
    case BT_STATUS_SCO_CONNECTION_REQ:
#if TRANSLATION_CALL_CONTROL_BY_DEVICE
        //来电关闭录音（听译、音视频，面对面）翻译模式
        switch (tlr_hdl.mode_info.mode) {
        case RCSP_TRANSLATOR_MODE_RECORD:
        case RCSP_TRANSLATOR_MODE_RECORD_TRANSLATION:
        case RCSP_TRANSLATOR_MODE_A2DP_TRANSLATION:
        case RCSP_TRANSLATOR_MODE_FACE_TO_FACE_TRANSLATION:
            memcpy(&minfo, &tlr_hdl.mode_info, sizeof(minfo));
            minfo.mode = RCSP_TRANSLATOR_MODE_IDLE;
            JL_rcsp_translator_op_inform_mode_info(&minfo);
            minfo.sr = READ_BIG_U32(&minfo.sr);
            _translator_op_set_mode((u8 *)&minfo, sizeof(minfo));
            break;
        }
#endif
        break;
    case BT_STATUS_SCO_DISCON:
#if TRANSLATION_CALL_CONTROL_BY_DEVICE
        //挂断电话关闭通话录音模式
        switch (tlr_hdl.mode_info.mode) {
        case RCSP_TRANSLATOR_MODE_CALL_TRANSLATION:
            memcpy(&minfo, &tlr_hdl.mode_info, sizeof(minfo));
            minfo.mode = RCSP_TRANSLATOR_MODE_IDLE;
            JL_rcsp_translator_op_inform_mode_info(&minfo);
            minfo.sr = READ_BIG_U32(&minfo.sr);
            _translator_op_set_mode((u8 *)&minfo, sizeof(minfo));
            break;
        }
#endif
        break;
    }
    return 0;
}

APP_MSG_HANDLER(translator_bt_event) = {
    .owner = 0xff,
    .from = MSG_FROM_BT_STACK,
    .handler = translator_bt_event_handler,
};

static void translator_tws_sync_play(u8 source)
{
    u8 ch;
    struct rx_ch_t *channel;

    ch = source_to_ch_remap(source);
    channel = &tlr_hdl.rx_ch[ch];
    if (OS_NO_ERR != os_mutex_pend(&channel->mutex, 10)) {
        return;
    }
    if (tlr_hdl.ch_state & BIT(ch)) {
        channel->sync_play = 1;
        if (channel->recv_decode_resume) {
            channel->recv_decode_resume(channel->cb_priv, source);
        }
    }
    os_mutex_post(&channel->mutex);
}

static void translator_tws_recv_ch_suspend(u8 source)
{
    u8 ch;
    struct rx_ch_t *channel;
    struct list_head *p, *n;
    struct translator_recv_audio_frame *frame;
    u8 frame_source = 0;
    ch = source_to_ch_remap(source);
    channel = &tlr_hdl.rx_ch[ch];
    if (OS_NO_ERR != os_mutex_pend(&channel->mutex, 10)) {
        return;
    }
    if (tlr_hdl.ch_state & BIT(ch)) {
#if TCFG_AI_RX_NODE_ENABLE
        ai_rx_player_close(source);
#endif
        list_for_each_safe(p, n, &channel->head) {
            frame = list_entry(p, struct translator_recv_audio_frame, entry);
            frame_source = frame->source;
            list_del(&frame->entry);
            free(frame);
        }
        channel->recv_size = 0;
        channel->ready = 0;
        channel->sync_play = 0;
        channel->underrun = 1;

        //会有出现tws发送suspend到响应suspend期间，APP将缓存一次性填满情况，
        //然后APP挂起等待设备消耗数据，通知APP。这里清过缓存，如果是满的需要
        //通知APP发数
        if (channel->full) {
            if ((tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED)) {
                if (tws_api_get_role() == TWS_ROLE_MASTER) {
                    _translator_op_inform_cache_free_size(frame_source, channel->max_size);
                }
            } else {
                _translator_op_inform_cache_free_size(frame_source, channel->max_size);
            }
            channel->full = 0;
        }
    }
    os_mutex_post(&channel->mutex);
}

static void translator_tws_recv_ch_resume(u8 source)
{
    u8 ch;
    struct rx_ch_t *channel;
    u8 *remote_bt_addr;
    struct ai_rx_player_param param = {0};

    ch = source_to_ch_remap(source);
    channel = &tlr_hdl.rx_ch[ch];
    if (OS_NO_ERR != os_mutex_pend(&channel->mutex, 10)) {
        return;
    }
    if (tlr_hdl.ch_state & BIT(ch)) {
        if (!channel->underrun) {
            goto __exit;
        }
        switch (source) {
        case RCSP_TRANSLATOR_AUDIO_SOURCE_MSBC:
        case RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_UPSTREAM:
        case RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_DOWNSTREAM:
            //注意检查打开的通道不要出错
            ASSERT(channel->source == source);
            break;
        }
        remote_bt_addr = translator_get_remote_bt_addr();
        if (remote_bt_addr == NULL) {
            goto __exit;
        }
#if TCFG_AI_RX_NODE_ENABLE
        switch (source) {
        case RCSP_TRANSLATOR_AUDIO_SOURCE_MSBC:
            param.type = AI_SERVICE_MEDIA;
            break;
        case RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_UPSTREAM:
            param.type = AI_SERVICE_CALL_UPSTREAM;
            break;
        case RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_DOWNSTREAM:
            param.type = AI_SERVICE_CALL_DOWNSTREAM;
            break;
        default:
            param.type = AI_SERVICE_VOICE;
            break;
        }
        param.coding_type = channel->coding_type;
        param.sample_rate = channel->sample_rate;
        param.bit_rate = channel->bit_rate;
        param.channel_mode = channel->channel_mode;
        param.frame_dms = channel->frame_dms;

        ai_rx_player_open(remote_bt_addr, source, &param);
#endif
        channel->underrun = 0;
    }
__exit:
    os_mutex_post(&channel->mutex);
}

static int translator_tws_send_cmd(u8 cmd, u8 OpCode, u8 *param, u32 len)
{
    u8 *buf;
    int ret = 0;

    buf = malloc(len + 2);
    if (!buf) {
        return -ENOMEM;
    }
    buf[0] = cmd;
    buf[1] = OpCode;
    if (param) {
        memcpy(buf + 2, param, len);
    }
    ret = tws_api_send_data_to_sibling(buf, len + 2, TWS_FUNC_ID_TRANSLATOR);
    free(buf);
    return ret;
}

static void translator_tws_msg_handler(u8 *_data, u32 len)
{
    int err = 0;
    u8 cmd;
    u32 param_len;
    u8 *param;
    u8 OpCode;
    struct translator_recv_audio_frame frame;
    u8 source;

    cmd = _data[0];
    OpCode = _data[1];
    switch (cmd) {
    case TWS_TRANS_CMD_SET_MODE:
        param_len = len - 2;
        param = _data + 2;
        if (tws_api_get_role() == TWS_ROLE_SLAVE) {
            //需要同步的模式跟当前不一致才设入
            if (tlr_hdl.mode_info.mode != param[0]) {
                _translator_op_set_mode(param, param_len);
            }
        }
        break;
    case TWS_TRANS_CMD_SYNC_REMOTE_ADDR:
        param = _data + 2;
        if (tws_api_get_role() == TWS_ROLE_SLAVE) {
            memcpy(tlr_hdl.spp_remote_addr, param, 6);
            memcpy(&tlr_hdl.ble_con_handle, param + 6, 2);
            memcpy(tlr_hdl.ble_remote_addr, param + 8, 6);
        }
        break;
    case TWS_TRANS_CMD_MANUAL_REC_START:
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            JL_rcsp_translator_manual_record_start();
        }
        break;
    case TWS_TRANS_CMD_MANUAL_REC_STOP:
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            JL_rcsp_translator_manual_record_stop();
        }
        break;
    case TWS_TRANS_CMD_INFORM_MODE:
        param = _data + 2;
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            JL_rcsp_translator_op_inform_mode_info((struct translator_mode_info *)param);
        }
        break;
    case TWS_TRANS_CMD_RECV_AUDIO_SYNC:
        memcpy(&frame, _data + 2, sizeof(frame));
        param = _data + 2 + sizeof(frame);
        param_len = frame.size;
        if (tws_api_get_role() == TWS_ROLE_SLAVE) {
            translator_recv_ch_put_frame(frame.source, param, param_len, frame.timestamp);
        }
        break;
    case TWS_TRANS_CMD_SET_VOLUME:
        if (tws_api_get_role() == TWS_ROLE_SLAVE) {
            memcpy(&g_bt_volume, _data + 2, 2);
            JL_rcsp_translator_set_play_volume(g_bt_volume);
        }
        break;
    case TWS_TRANS_CMD_SYNC_PLAY:
        g_printf("TWS_TRANS_CMD_SYNC_PLAY\n");
        source = _data[2];
        translator_tws_sync_play(source);
        break;
    case TWS_TRANS_CMD_MEDIA_SUSPEND:
        g_printf("TWS_TRANS_CMD_MEDIA_SUSPEND\n");
        source = _data[2];
        translator_tws_recv_ch_suspend(source);
        break;
    case TWS_TRANS_CMD_MEDIA_RESUME:
        g_printf("TWS_TRANS_CMD_MEDIA_RESUME\n");
        source = _data[2];
        translator_tws_recv_ch_resume(source);
        break;
    case TWS_TRANS_CMD_SYNC_REC_STA:
        if (tws_api_get_role() == TWS_ROLE_SLAVE) {
            tlr_hdl.record_state = _data[2];
        }
        break;
    }

    free(_data);
}

static void translator_tws_msg_from_sibling(void *_data, u16 len, bool rx)
{
    u8 *rx_data;
    int msg[4];
    if (!rx) {
        //是否需要限制只有rx才能收到消息
        //return;
    }
    rx_data = malloc(len);
    if (!rx_data) {
        return;
    }
    memcpy(rx_data, _data, len);
    msg[0] = (int)translator_tws_msg_handler;
    msg[1] = 2;
    msg[2] = (int)rx_data;
    msg[3] = len;
    int err = os_taskq_post_type("app_core", Q_CALLBACK, 4, msg);
    if (err) {
        printf("tws rx post fail\n");
    }
}

REGISTER_TWS_FUNC_STUB(translator_tws_sibling_stub) = {
    .func_id = TWS_FUNC_ID_TRANSLATOR,
    .func = translator_tws_msg_from_sibling,
};

#if TCFG_USER_TWS_ENABLE
static int translator_tws_event_handler(int *_msg)
{
    struct tws_event *evt = (struct tws_event *)_msg;
    int role = evt->args[0];
    int reason = evt->args[2];
    struct translator_mode_info minfo = {0};
    u8 mode;

    switch (evt->event) {
    case TWS_EVENT_CONNECTED:
        if (role == TWS_ROLE_MASTER) {
            translator_tws_send_cmd(TWS_TRANS_CMD_SET_VOLUME, 0, (u8 *)&g_bt_volume, 2);
            //连接上tws的时候将主机的状态同步给从机
            memcpy(&minfo, &tlr_hdl.mode_info, sizeof(minfo));
            minfo.sr = READ_BIG_U32(&minfo.sr);
#if 1
            mode = minfo.mode;
            minfo.mode = RCSP_TRANSLATOR_MODE_IDLE;
            _translator_op_set_mode((u8 *)&minfo, sizeof(minfo));
            if (mode != RCSP_TRANSLATOR_MODE_IDLE) {
                minfo.mode = mode;
                _translator_op_set_mode((u8 *)&minfo, sizeof(minfo));
                if (tlr_hdl.record_state) {
                    _translator_op_record_mode_start(1);
                }
            }
#else
            translator_tws_send_cmd(TWS_TRANS_CMD_SET_MODE, 0, (u8 *)&minfo, sizeof(struct translator_mode_info));
#endif
        } else {
            JL_rcsp_translator_init();
        }
        break;
    case TWS_EVENT_CONNECTION_DETACH:
        if (role == TWS_ROLE_MASTER) {

        } else {
            JL_rcsp_translator_deinit();
        }
        break;
    case TWS_EVENT_ROLE_SWITCH:
        if (role == TWS_ROLE_MASTER) {
            memcpy(&minfo, &tlr_hdl.mode_info, sizeof(minfo));
            memset(tlr_hdl.ble_remote_addr, 0, 6);
            if (minfo.mode != RCSP_TRANSLATOR_MODE_IDLE) {
                mode = minfo.mode;
                minfo.sr = READ_BIG_U32(&minfo.sr);
                //主从机都先关闭
                minfo.mode = RCSP_TRANSLATOR_MODE_IDLE;
                _translator_op_set_mode((u8 *)&minfo, sizeof(minfo));
                //再次打开（因为只有主机才会打开录音，从机不打开，需要交换一下）
                minfo.mode = mode;
                _translator_op_set_mode((u8 *)&minfo, sizeof(minfo));
            }
            if (tlr_hdl.record_state) {
                _translator_op_record_mode_start(0);
            }
        } else {
            if (tlr_hdl.record_state) {
                _translator_op_record_mode_stop(0);
            }
        }
        break;
    }
    return 0;
}

APP_MSG_HANDLER(translator_tws_event) = {
    .owner = 0xff,
    .from = MSG_FROM_TWS,
    .handler = translator_tws_event_handler,
};
#endif

void JL_rcsp_translator_init()
{
    memset(&tlr_hdl, 0, sizeof(tlr_hdl));
    tlr_hdl.mode_info.encode = RCSP_TRANSLATOR_ENCODE_TYPE_OPUS;
    tlr_hdl.mode_info.ch = 1;
    tlr_hdl.mode_info.sr = 16000;
    for (int i = 0; i < AUDIO_CH_MAX; i++) {
        os_mutex_create(&tlr_hdl.rx_ch[i].mutex);
        os_mutex_create(&tlr_hdl.tx_ch[i].mutex);
    }
    translator_todo_list_init();
#if MIC_REC_METHOD_NEW == 0
    mic_rec_pram_init(AUDIO_CODING_OPUS, 0, translator_send_mic_data, 4, 1024);
#endif
}

void JL_rcsp_translator_deinit()
{
    struct translator_mode_info minfo;
    translator_todo_list_clear();
    _translator_op_get_info((u8 *)&minfo, sizeof(minfo));
    if (minfo.mode == RCSP_TRANSLATOR_MODE_RECORD ||
        minfo.mode == RCSP_TRANSLATOR_MODE_RECORD_TRANSLATION) {
        _translator_op_record_mode_stop(0);
    }
    minfo.mode = RCSP_TRANSLATOR_MODE_IDLE;
    JL_rcsp_translator_set_decode_resume_handler(RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_UPSTREAM, NULL, NULL);
    JL_rcsp_translator_set_decode_resume_handler(RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_DOWNSTREAM, NULL, NULL);
    _translator_op_set_mode((u8 *)&minfo, sizeof(minfo));
}

#endif
