#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".agc_node.data.bss")
#pragma data_seg(".agc_node.data")
#pragma const_seg(".agc_node.text.const")
#pragma code_seg(".agc_node.text")
#endif
#include "jlstream.h"
#include "app_config.h"
#include "audio_config.h"
#include "effects/effects_adj.h"
#include "frame_length_adaptive.h"
#include "audio_agc.h"


#if 1
#define agc_log	printf
#else
#define agc_log(...)
#endif/*log_en*/

#define AGC_FRAME_POINTS   128
#define AGC_FRAME_SIZE     (AGC_FRAME_POINTS << 1)

#if TCFG_AGC_NODE_ENABLE

struct agc_cfg_t {
    float AGC_max_lvl;          //最大幅度压制，range[0 : -90] dB
    float AGC_fade_in_step;     //淡入步进，range[0.1 : 5] dB
    float AGC_fade_out_step;    //淡出步进，range[0.1 : 5] dB
    float AGC_max_gain;         //放大上限, range[-90 : 40] dB
    float AGC_min_gain;         //放大下限, range[-90 : 40] dB
    float AGC_speech_thr;       //放大阈值, range[-70 : -40] dB
} __attribute__((packed));

struct agc_node_hdl {
    u16 sr;
    void *agc;
    struct stream_frame *out_frame;
    agc_param_t parm;
    struct fixed_frame_len_handle *fixed_hdl;
};

int agc_param_cfg_read(struct stream_node *node)
{
    struct agc_cfg_t config;
    struct agc_node_hdl *hdl = (struct agc_node_hdl *)node->private_data;
    if (!hdl) {
        return -1 ;
    }
    /*
     *获取配置文件内的参数,及名字
     * */
    if (!jlstream_read_node_data_new(NODE_UUID_AGC, node->subid, (void *)&config, NULL)) {
        printf("%s, read node data err\n", __FUNCTION__);
        return -1 ;
    }

    memcpy(&hdl->parm, &config, sizeof(config));

    agc_log("AGC_max_lvl %d/10\n", (int)(hdl->parm.AGC_max_lvl * 10));
    agc_log("AGC_fade_in_step %d/10\n", (int)(hdl->parm.AGC_fade_in_step * 10));
    agc_log("AGC_fade_out_step %d/10\n", (int)(hdl->parm.AGC_fade_out_step * 10));
    agc_log("AGC_max_gain  %d/10\n", (int)(hdl->parm.AGC_max_gain * 10));
    agc_log("AGC_min_gain    %d/10\n", (int)(hdl->parm.AGC_min_gain * 10));
    agc_log("AGC_speech_thr     %d/10\n", (int)(hdl->parm.AGC_speech_thr * 10));

    return 0;
}

static int agc_node_fixed_frame_run(void *agc, u8 *in, u8 *out, int len)
{
    return audio_agc_run(agc, (s16 *)in, (s16 *)out, len);
}
/*节点输出回调处理，可处理数据或post信号量*/
static void agc_handle_frame(struct stream_iport *iport, struct stream_note *note)
{
    struct stream_node *node = iport->node;
    struct agc_node_hdl *hdl = (struct agc_node_hdl *)node->private_data;
    struct stream_frame *in_frame;
    int wlen;
    int out_frame_len;

    while (1) {
        in_frame = jlstream_pull_frame(iport, note);		//从iport读取数据
        if (!in_frame) {
            break;
        }
        out_frame_len = get_fixed_frame_len_output_len(hdl->fixed_hdl, in_frame->len);
        if (out_frame_len) {
            hdl->out_frame = jlstream_get_frame(node->oport, out_frame_len);
            if (!hdl->out_frame) {
                return;
            }
        }
        wlen = audio_fixed_frame_len_run(hdl->fixed_hdl, in_frame->data, hdl->out_frame->data, in_frame->len);

        if (wlen && hdl->out_frame) {
            hdl->out_frame->len = wlen;
            jlstream_push_frame(node->oport, hdl->out_frame);	//将数据推到oport
            hdl->out_frame = NULL;
        }
        jlstream_free_frame(in_frame);	//释放iport资源
    }
}

/*节点预处理-在ioctl之前*/
static int agc_adapter_bind(struct stream_node *node, u16 uuid)
{
    agc_param_cfg_read(node);
    return 0;
}

/*打开改节点输入接口*/
static void agc_ioc_open_iport(struct stream_iport *iport)
{
    iport->handle_frame = agc_handle_frame;				//注册输出回调
}


/*节点start函数*/
static void agc_ioc_start(struct agc_node_hdl *hdl)
{
    struct stream_fmt *fmt = &hdl_node(hdl)->oport->fmt;
    hdl->parm.AGC_samplerate = fmt->sample_rate;
    hdl->parm.AGC_frame_size = AGC_FRAME_POINTS;
    agc_log("AGC_samplerate : %d", hdl->parm.AGC_samplerate);
    agc_log("AGC_frame_points : %d", hdl->parm.AGC_frame_size);
    hdl->agc = audio_agc_open(&hdl->parm);
    hdl->fixed_hdl = audio_fixed_frame_len_init(AGC_FRAME_SIZE, agc_node_fixed_frame_run, hdl->agc);
}

/*节点stop函数*/
static void agc_ioc_stop(struct agc_node_hdl *hdl)
{
    if (hdl->agc) {
        audio_agc_close(hdl->agc);
        hdl->agc = NULL;
    }
    if (hdl->fixed_hdl) {
        audio_fixed_frame_len_exit(hdl->fixed_hdl);
        hdl->fixed_hdl = NULL;
    }
    if (hdl->out_frame) {
        jlstream_free_frame(hdl->out_frame);
        hdl->out_frame = NULL;
    }
}


/*节点ioctl函数*/
static int agc_adapter_ioctl(struct stream_iport *iport, int cmd, int arg)
{
    int ret = 0;
    struct agc_node_hdl *hdl = (struct agc_node_hdl *)iport->node->private_data;

    switch (cmd) {
    case NODE_IOC_OPEN_IPORT:
        agc_ioc_open_iport(iport);
        break;
    case NODE_IOC_START:
        agc_ioc_start(hdl);
        break;
    case NODE_IOC_SUSPEND:
    case NODE_IOC_STOP:
        agc_ioc_stop(hdl);
        break;
    }

    return ret;
}

/*节点用完释放函数*/
static void agc_adapter_release(struct stream_node *node)
{
    struct agc_node_hdl *hdl = (struct agc_node_hdl *)node->private_data;
    if (!hdl) {
        return;
    }
    free(hdl);
}

/*节点adapter 注意需要在sdk_used_list声明，否则会被优化*/
REGISTER_STREAM_NODE_ADAPTER(agc_node_adapter) = {
    .name       = "agc",
    .uuid       = NODE_UUID_AGC,
    .bind       = agc_adapter_bind,
    .ioctl      = agc_adapter_ioctl,
    .release    = agc_adapter_release,
    .hdl_size   = sizeof(struct agc_node_hdl),
};

//注册工具在线调试
REGISTER_ONLINE_ADJUST_TARGET(agc_process) = {
    .uuid = NODE_UUID_AGC,
};

#endif /*TCFG_AGC_NODE_ENABLE*/

