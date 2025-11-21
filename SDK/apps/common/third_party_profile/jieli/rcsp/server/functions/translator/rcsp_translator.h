#ifndef __RCSP_TRANSLATOR__
#define __RCSP_TRANSLATOR__

#include "typedef.h"
#include "generic/list.h"

//op
#define RCSP_TRANSLATOR_OP_GET_MODE_INFO                        0x00
#define RCSP_TRANSLATOR_OP_SET_MODE                             0x01
#define RCSP_TRANSLATOR_OP_INFORM_MODE_INFO                     0x02
#define RCSP_TRANSLATOR_OP_SEND_AUDIO_DATA                      0x03
#define RCSP_TRANSLATOR_OP_GET_CACHE_FREE_SIZE                  0x04
#define RCSP_TRANSLATOR_OP_INFORM_CACHE_FREE_SIZE               0x05

//mode info
#define RCSP_TRANSLATOR_MODE_IDLE                               0x00
#define RCSP_TRANSLATOR_MODE_RECORD                             0x01
#define RCSP_TRANSLATOR_MODE_RECORD_TRANSLATION                 0x02
#define RCSP_TRANSLATOR_MODE_CALL_TRANSLATION                   0x03
#define RCSP_TRANSLATOR_MODE_A2DP_TRANSLATION                   0x04
#define RCSP_TRANSLATOR_MODE_FACE_TO_FACE_TRANSLATION           0x05

//encode type
#define RCSP_TRANSLATOR_ENCODE_TYPE_PCM                         0x00
#define RCSP_TRANSLATOR_ENCODE_TYPE_SPEEX                       0x01
#define RCSP_TRANSLATOR_ENCODE_TYPE_OPUS                        0x02
#define RCSP_TRANSLATOR_ENCODE_TYPE_MSBC                        0x03
#define RCSP_TRANSLATOR_ENCODE_TYPE_JLA_V2                      0x04

//audio source
#define RCSP_TRANSLATOR_AUDIO_SOURCE_FILE                       0x00
#define RCSP_TRANSLATOR_AUDIO_SOURCE_DEV_MIC                    0x01
#define RCSP_TRANSLATOR_AUDIO_SOURCE_PHONE_MIC                  0x02
#define RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_UPSTREAM              0x03
#define RCSP_TRANSLATOR_AUDIO_SOURCE_ESCO_DOWNSTREAM            0x04
#define RCSP_TRANSLATOR_AUDIO_SOURCE_MSBC                       0x05
#pragma pack(1)
//mode info
struct translator_mode_info {
    u8 mode;
    u8 encode;  //编码类型
    u8 ch;  //声道
    u32 sr;  //采样率
};

//opCode 0x34 op 03音频数据格式
struct translator_op_03_audio_format {
    u8 source;  //来源
    u8 encode;  //编码类型
    u8 count;  //计数，倒序，计数到0为结束包
    u16 crc;  //音频数据的crc
    u16 len;  //音频数据长度
    //u8 buf[0];
};
#pragma pack()

struct translator_recv_audio_frame {
    struct list_head entry;
    u8 source;
    u8 *buf;
    u32 size;
    u32 timestamp;
};


void JL_rcsp_translator_init();
void JL_rcsp_translator_deinit();
int JL_rcsp_translator_op_inform_mode_info(struct translator_mode_info *info);
int JL_rcsp_translator_functions(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len, u16 ble_con_handle, u8 *spp_remote_addr);
struct translator_recv_audio_frame *JL_rcsp_translator_recv_ch_get_frame(u8 source);
int JL_rcsp_translator_recv_ch_free_frame(u8 source, struct translator_recv_audio_frame *frame);

void JL_rcsp_translator_set_decode_resume_handler(u8 source, void *priv, void (*func)(void *priv, u8 source));
u32 JL_rcsp_translator_get_status();
void JL_rcsp_translator_get_mode_info(struct translator_mode_info *minfo);
int JL_rcsp_translator_manual_record_start();
int JL_rcsp_translator_manual_record_stop();
int JL_rcsp_translator_set_play_volume(u16 volume);
void JL_rcsp_translator_update_play_volume(u8 source);
int JL_rcsp_translator_whether_play_by_ai_rx();

#endif
