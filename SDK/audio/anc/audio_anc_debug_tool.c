
/****************************************************
				audio_anc_debug_tool.c
*	ANC debug 工具，用SPP代替有线串口打印关键debug数据

*****************************************************/

#include "app_config.h"
#include "audio_anc_debug_tool.h"
#include "btstack/avctp_user.h"
#include "classic/hci_lmp.h"
#include "generic/circular_buf.h"
#include "system/task.h"
#include "timer.h"
#include "asm/crc16.h"

#if TCFG_ANC_TOOL_DEBUG_ONLINE && TCFG_AUDIO_ANC_ENABLE

#define ANC_DEBUG_TOOL_BY_TIMER			1  //定时器输出
#define ANC_DEBUG_TOOL_HEAD_MARK0		0xAA
#define ANC_DEBUG_TOOL_HEAD_MARK1		0x56

#define ANC_DEBUG_SEND_MAX_SIZE 		1024	//数据包最大缓存

#define ANC_DEBUG_TOOL_SEND_TIME 		50		//定时发送间隔		（最短40，太小可能会导致发不过来，与手机端有关）
#define ANC_DEBUG_SEND_SIZE 			350		//每次发送最大长度	（最长550，与手机端有关）


//数据包头信息
struct anc_debug_tool_packt_t {
    u8 mark0;
    u8 mark1;
    u16 id;
    u16 len;
    u8 data[0];
    /* u16 crc; */
};
#define ANC_DEBUG_TOOL_HEAD_SIZE 	sizeof(struct anc_debug_tool_packt_t) + 2 + 1	//head+crc+cmd

struct anc_debug_tool_t {
    u8 state;
    volatile u8 send_busy;
    volatile u8 send_id;
    OS_MUTEX mutex;

#if ANC_DEBUG_TOOL_BY_TIMER
    u32 send_len;
    cbuffer_t cbuf;
    u8 send_buf[ANC_DEBUG_SEND_MAX_SIZE + ANC_DEBUG_TOOL_HEAD_SIZE];
    u16 spp_timer;
#endif
};

enum anc_debug_tool_cmd_t {
    ANC_DEBUG_CMD_SINGLE_PACKET = 0x1,		//单包模式
    ANC_DEBUG_CMD_UNPACK_PACKET,			//拆包模式
};


static struct anc_debug_tool_t *debug_hdl = NULL;

static void audio_anc_debug_spp_send(u8 *buf, int len)
{
    int tmp_len = 512;
    int err = 0;
    //大于512循环发送
    while (len > tmp_len) {
        do {
            err = bt_cmd_prepare(USER_CTRL_SPP_SEND_DATA, tmp_len, buf);
        } while (err);
        buf += tmp_len;
        len -= tmp_len;
    }
    do {
        err = bt_cmd_prepare(USER_CTRL_SPP_SEND_DATA, len, buf);
    } while (err);
}

static void audio_anc_debug_send_timer(void *p)
{
    u8 *ptr;
    if (debug_hdl->state == ANC_DEBUG_STA_OPEN) {
        debug_hdl->send_busy = 1;

        ptr = cbuf_read_alloc(&debug_hdl->cbuf, &debug_hdl->send_len);
        if (debug_hdl->send_len) {
            if (debug_hdl->send_len > ANC_DEBUG_SEND_SIZE) {
                debug_hdl->send_len = ANC_DEBUG_SEND_SIZE;
            }
            //写不进，可以尝试加大MAX_SUPPORT_WAITINT_PKT
            bt_cmd_prepare(USER_CTRL_SPP_SEND_DATA, debug_hdl->send_len, ptr);
        }
        cbuf_read_updata(&debug_hdl->cbuf, debug_hdl->send_len);
        debug_hdl->send_len = 0;
        debug_hdl->send_busy = 0;
    }
}

void audio_anc_debug_tool_open(void)
{
    if (debug_hdl) {
        printf("ERR:ANC_DEBUG OPEN NOW!");
        return;
    }
    debug_hdl = zalloc(sizeof(struct anc_debug_tool_t));
    debug_hdl->state = ANC_DEBUG_STA_OPEN;
    debug_hdl->send_id = 1;
    os_mutex_create(&debug_hdl->mutex);
#if ANC_DEBUG_TOOL_BY_TIMER
    cbuf_init(&debug_hdl->cbuf, debug_hdl->send_buf, sizeof(debug_hdl->send_buf));
    debug_hdl->spp_timer = sys_timer_add(NULL, audio_anc_debug_send_timer, ANC_DEBUG_TOOL_SEND_TIME);
#endif
}

void audio_anc_debug_tool_close(void)
{
    if (debug_hdl) {
        debug_hdl->state = ANC_DEBUG_STA_CLOSE;
        while (debug_hdl->send_busy) {
            os_time_dly(1);
        }
#if ANC_DEBUG_TOOL_BY_TIMER
        sys_timer_del(debug_hdl->spp_timer);
#endif
        free(debug_hdl);
        debug_hdl = NULL;
    }
}

int audio_anc_debug_send_data_packet(u8 cmd, u8 *buf, int len)
{
    os_mutex_pend(&debug_hdl->mutex, 0);
    int err = 0;
    u16 calc_crc;
    int packet_len = len + ANC_DEBUG_TOOL_HEAD_SIZE;
    struct anc_debug_tool_packt_t *packet = malloc(packet_len);	//len+ head + crc

    if (debug_hdl->send_id < 255) {
        debug_hdl->send_id++;
    } else {
        debug_hdl->send_id = 1;
    }

    packet->mark0 = ANC_DEBUG_TOOL_HEAD_MARK0;
    packet->mark1 = ANC_DEBUG_TOOL_HEAD_MARK1;
    packet->id = debug_hdl->send_id;
    packet->len = len + 1;
    packet->data[0] = cmd;
    memcpy(packet->data + 1, buf, len);
    calc_crc = CRC16(packet, packet_len - 2);
    memcpy(packet->data + 1 + len, (u8 *)&calc_crc, 2);

    if (debug_hdl) {
#if ANC_DEBUG_TOOL_BY_TIMER
        //异步发送
        if (cbuf_is_write_able(&debug_hdl->cbuf, len)) {
            cbuf_write(&debug_hdl->cbuf, packet, packet_len);
        } else {
            putchar('N');
            free(packet);
            return 0;
        }
#else
        //阻塞式发送
        audio_anc_debug_spp_send(packet, len);
#endif
    }
    os_mutex_post(&debug_hdl->mutex);
    free(packet);
    return len;
}

int audio_anc_debug_send_data(u8 *buf, int len)
{
    if (debug_hdl) {
        if (len <= ANC_DEBUG_SEND_MAX_SIZE) {
            //单包模式
            return audio_anc_debug_send_data_packet(ANC_DEBUG_CMD_SINGLE_PACKET, buf, len);
        } else {
            //拆包模式
            printf("ERR:ANC_DEBUG buff size full!");
            return 0;
        }
    }
    return 0;
}


u8 audio_anc_debug_busy_get(void)
{
    if (debug_hdl) {
        return debug_hdl->send_busy;
        /* return bt_cmd_prepare(USER_CTRL_SPP_SEND_DATA, 0, NULL); */
    }
    return 0;
}

#if 0
void audio_anc_debug_test(void)
{
    printf("audio_anc_debug_test\n");
    /* audio_anc_debug_tool_open(); */

    /* char s[50]; */
    /* for (int j = 0; j < 10; j++) { */
    /* sprintf(s, "test %d\n", j); */
    /* audio_anc_debug_send_data(s, strlen(s)); */
    /* } */
    /* return; */
    int err = 0;

    u8 *buf = malloc(510);
    for (int i = 0; i < 510; i += 51) {
        memset(buf + i, ((i / 51) << 4) | (i / 51), 51);
        do {
            err = audio_anc_debug_send_data(buf + i, 51);
            break;
            if (!err) {
                printf("a");
                os_time_dly(1);
            }
        } while (!err);
    }
    free(buf);
}
#endif

#endif/*TCFG_ANC_TOOL_DEBUG_ONLINE*/
