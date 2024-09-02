/*************************************************************************************************/
/*!
*  \file      le_audio_recorder.c
*
*  \brief
*
*  Copyright (c) 2011-2023 ZhuHai Jieli Technology Co.,Ltd.
*
*/
/*************************************************************************************************/
#include "jlstream.h"
#include "le_audio_recorder.h"
#include "sdk_config.h"
#include "le_audio_stream.h"
#include "audio_config_def.h"
#include "audio_config.h"
#include "media/sync/audio_syncts.h"
#include "asm/dac.h"
#include "audio_cvp.h"

struct le_audio_mic_recorder {
    void *stream;
};

static struct le_audio_mic_recorder *g_mic_recorder = NULL;

static void mic_recorder_callback(void *private_data, int event)
{
    printf("le audio mic recorder callback : %d\n", event);
}

int le_audio_mic_recorder_open(void *params, void *le_audio, int latency)
{
    int err = 0;
    struct le_audio_stream_params *lea_params = params;
    struct le_audio_stream_format *le_audio_fmt = &lea_params->fmt;
    u16 source_uuid;
    u16 uuid = jlstream_event_notify(STREAM_EVENT_GET_PIPELINE_UUID, (int)"mic_le_audio");
    struct stream_enc_fmt fmt = {
        .channel = le_audio_fmt->nch,
        .bit_rate = le_audio_fmt->bit_rate,
        .sample_rate = le_audio_fmt->sample_rate,
        .frame_dms = le_audio_fmt->frame_dms,
        .coding_type = le_audio_fmt->coding_type,
    };
    if (!g_mic_recorder) {
        g_mic_recorder = zalloc(sizeof(struct le_audio_mic_recorder));
        if (!g_mic_recorder) {
            return -ENOMEM;
        }
    }
    g_mic_recorder->stream = jlstream_pipeline_parse(uuid, NODE_UUID_ADC);
    source_uuid = NODE_UUID_ADC;

    if (!g_mic_recorder->stream) {
        err = -ENOMEM;
        goto __exit0;
    }

    jlstream_set_callback(g_mic_recorder->stream, NULL, mic_recorder_callback);
    if (lea_params->service_type == LEA_SERVICE_CALL) {
        //printf("LEA Recoder:LEA_CALL\n");
        jlstream_set_scene(g_mic_recorder->stream, STREAM_SCENE_LEA_CALL);
        jlstream_node_ioctl(g_mic_recorder->stream, NODE_UUID_SOURCE, NODE_IOC_SET_PRIV_FMT, 256);
    } else {
        //printf("LEA Recoder:MIC\n");
        jlstream_set_scene(g_mic_recorder->stream, STREAM_SCENE_MIC);
        jlstream_node_ioctl(g_mic_recorder->stream, NODE_UUID_SOURCE, NODE_IOC_SET_PRIV_FMT, 160);
    }
    if (latency == 0) {
        latency = 100000;
    }

    jlstream_node_ioctl(g_mic_recorder->stream, NODE_UUID_CAPTURE_SYNC, NODE_IOC_SET_PARAM, latency);
    jlstream_node_ioctl(g_mic_recorder->stream, NODE_UUID_LE_AUDIO_SOURCE, NODE_IOC_SET_BTADDR, (int)le_audio);
    //设置中断点数
    /* jlstream_node_ioctl(g_mic_recorder->stream, NODE_UUID_SOURCE, NODE_IOC_SET_PRIV_FMT, AUDIO_ADC_IRQ_POINTS); */
    jlstream_node_ioctl(g_mic_recorder->stream, NODE_UUID_VOCAL_TRACK_SYNTHESIS, NODE_IOC_SET_PRIV_FMT, AUDIO_ADC_IRQ_POINTS);//四声道时，指定声道合并单个声道的点数

    u16 node_uuid = get_cvp_node_uuid();
    if (node_uuid) {
        u32 ref_sr = audio_dac_get_sample_rate(&dac_hdl);
        jlstream_node_ioctl(g_mic_recorder->stream, node_uuid, NODE_IOC_SET_FMT, (int)ref_sr);
        err = jlstream_node_ioctl(g_mic_recorder->stream, node_uuid, NODE_IOC_SET_PRIV_FMT, source_uuid);
        //根据回音消除的类型，将配置传递到对应的节点
        if (err && (err != -ENOENT)) {	//兼容没有cvp节点的情况
            goto __exit1;
        }
    }

    err = jlstream_ioctl(g_mic_recorder->stream, NODE_IOC_SET_ENC_FMT, (int)&fmt);
    if (err == 0) {
        err = jlstream_start(g_mic_recorder->stream);
    }
    if (err) {
        goto __exit1;
    }

    printf("le_audio mic recorder open success  \n");
    return 0;

__exit1:
    jlstream_release(g_mic_recorder->stream);
__exit0:
    free(g_mic_recorder);
    g_mic_recorder = NULL;
    return err;
}

void le_audio_mic_recorder_close(void)
{
    struct le_audio_mic_recorder *mic_recorder = g_mic_recorder;

    printf("le_audio_mic_recorder_close\n");

    if (!mic_recorder) {
        return;
    }
    if (mic_recorder->stream) {
        jlstream_stop(mic_recorder->stream, 0);
        jlstream_release(mic_recorder->stream);
    }
    free(mic_recorder);
    g_mic_recorder = NULL;
    jlstream_event_notify(STREAM_EVENT_CLOSE_PLAYER, (int)"mic_le_audio");
}

