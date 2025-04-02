#ifndef OPUS_CODEC_API_H
#define OPUS_CODEC_API_H

#include "audio_encode_common_api.h"
#include "audio_decode_common_api.h"


/******************* ENC *******************/

typedef struct _OPUS_EN_FILE_IO_ {
    void *priv;
    u16(*input_data)(void *priv, s16 *buf, u16 len);  //short
    void (*output_data)(void *priv, u8 *buf, u16 len); //bytes
} OPUS_EN_FILE_IO;


typedef struct __OPUS_ENC_OPS {
    u32(*need_buf)(u16 samplerate);       //samplerate=16k   ignore
    u32(*open)(u8 *ptr, OPUS_EN_FILE_IO *audioIO, u8 quality, u16 sample_rate);
    //1. quality:bitrate     0:16kbps    1:32kbps    2:64kbps
    //   quality: MSB_2:(bit7_bit6)     format_mode    //0:百度_无头.                   1:酷狗_eng+range.
    //   quality:LMSB_2:(bit5_bit4)     low_complexity //0:高复杂度,高质量.兼容之前库.  1:低复杂度,低质量.
    //2. sample_rate         sample_rate=16k         ignore
    u32(*run)(u8 *ptr);
} OPUS_ENC_OPS;

extern OPUS_ENC_OPS *get_opus_enc_ops(void);



/******************** DEC **********************/

typedef struct __OPUSLIB_DEC_OPS {
    u32(*need_buf)(int  bitwidth);
    u32(*open)(u32 *ptr, int br_index, int  bitwidth);         //0,1,2
    u32(*run)(u32 *ptr, char *frame_data, short *frame_out);
} opuslib_dec_ops;

typedef struct  _BR_CONTEXT_ {
    int br_index;
} BR_CONTEXT;


// opus (silkm模式)解码设置
#define SET_DEC_SR           0x91

// ogg_opus 解码设置
#define	SET_OPUS_RAWDTF      0x90 //设置OPUS 为raw 数据. 带8字节packet头(4字节大端包长+4字节range校验值)
#define	SET_OPUS_CBR_PKTLEN  0x91 //设置OPUS 为raw 数据 + CBR_OPUS 包长,可能有多帧共用TOC. 返回0设置成功.

typedef struct _AUDIO_OPUS_PKTLEN {
    u32 opus_pkt_len;    //SET_OPUS_CBR_PKTLEN
} AUDIO_OPUS_PKTLEN;


extern opuslib_dec_ops *getopuslibdec_ops();



#define OPUS_SR_8000_OUT_POINTS			(160)
#define OPUS_SR_16000_OUT_POINTS		(320)


#endif


