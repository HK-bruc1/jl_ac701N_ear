#include "jlstream.h"
#include "media/audio_base.h"
#include "effects/effects_adj.h"
#include "app_config.h"
#include "effect_dev_node.h"

#if TCFG_EFFECT_DEV0_NODE_ENABLE

#define LOG_TAG_CONST EFFECTS
#define LOG_TAG     "[EFFECT_DEV0-NODE]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma   code_seg(".effect_dev0.text")
#pragma   data_seg(".effect_dev0.data")
#pragma  const_seg(".effect_dev0.text.const")
#endif

/* 音效算法处理帧长
 * 0   : 等长输入输出，输入数据算法需要全部处理完
 * 非0 : 按照帧长输入数据到算法处理接口
 */
#define EFFECT_DEV0_FRAME_POINTS  (256)

struct effect_dev0_node_hdl {
    char name[16];
    void *effect_dev0;
    s16 *remain_buf;
    struct stream_frame *pack_frame;
    struct user_effect_tool_param cfg;//工具界面参数
    u32 sample_rate;
    u16 remain_len; //记录算法消耗的剩余的数据长度
    u16 frame_len;
    u8 ch_num;
    u8 bit_width;
};

/* 自定义算法，初始化
 * hdl->sample_rate:采样率
 * hdl->ch_num:通道数，单声道 1，立体声 2, 四声道 4
 * hdl->bit_width:位宽 0，16bit  1，32bit
 **/
static void audio_effect_dev0_init(struct effect_dev0_node_hdl *hdl)
{
    //do something
}

/* 自定义算法，运行
 * hdl->sample_rate:采样率
 * hdl->ch_num:通道数，单声道 1，立体声 2, 四声道 4
 * hdl->bit_width:位宽 0，16bit  1，32bit
 * *data:输入输出数据同地址
 * data_len :输入数据长度,byte
 * */
static void audio_effect_dev0_run(struct effect_dev0_node_hdl *hdl, s16 *data, u32 data_len)
{
    //do something
    /* printf("effect dev0 do something here\n"); */
}
/* 自定义算法，关闭
 **/
static void audio_effect_dev0_exit()
{
    //do something

}


/*节点输出回调处理，可处理数据或post信号量*/
static void effect_dev0_handle_frame(struct stream_iport *iport, struct stream_note *note)
{
    struct stream_frame *frame;
    struct effect_dev0_node_hdl *hdl = (struct effect_dev0_node_hdl *)iport->node->private_data;
    struct stream_node *node = iport->node;

    while (1) {
        frame = jlstream_pull_frame(iport, note);		//从iport读取数据
        if (!frame) {
            break;
        }
#if EFFECT_DEV0_FRAME_POINTS
        /*算法出来一帧的数据长度，byte*/
        hdl->remain_len += frame->len;  //记录目前还有多少数据
        u16 tmp_remain = 0;             //记录上一次剩余多少
        u8 pack_frame_num = hdl->remain_len / hdl->frame_len;//每次数据需要跑多少帧
        u16 pack_frame_len = pack_frame_num * hdl->frame_len;       //记录本次需要跑多少数据
        u16 offset = 0;

        if (pack_frame_num) {
            if (!hdl->pack_frame) {
                /*申请资源存储输出输出*/
                hdl->pack_frame = jlstream_get_frame(node->oport, pack_frame_len);
                if (!hdl->pack_frame) {
                    return;
                }
            }
            tmp_remain = hdl->remain_len - frame->len;//上一次剩余的数据大小
            /*拷贝上一次剩余的数据*/
            memcpy(hdl->pack_frame->data, hdl->remain_buf, tmp_remain);
            /*拷贝本次数据*/
            memcpy((void *)((int)hdl->pack_frame->data + tmp_remain), frame->data, pack_frame_len - tmp_remain);
            while (pack_frame_num--) {
                audio_effect_dev0_run(hdl, (s16 *)((int)hdl->pack_frame->data + offset), hdl->frame_len);
                offset += hdl->frame_len;
            }
            hdl->remain_len -= pack_frame_len;//剩余数据长度
            hdl->pack_frame->len = pack_frame_len;
            jlstream_push_frame(node->oport, hdl->pack_frame);	//将数据推到oport
            hdl->pack_frame = NULL;

            /*保存剩余不够一帧的数据*/
            memcpy(hdl->remain_buf, (void *)((int)frame->data + frame->len - hdl->remain_len), hdl->remain_len);
            jlstream_free_frame(frame);	//释放iport资源
        } else {
            /*当前数据不够跑一帧算法时*/
            memcpy((void *)((int)hdl->remain_buf + hdl->remain_len - frame->len), frame->data, frame->len);
            jlstream_free_frame(frame);	//释放iport资源
        }
#else
        audio_effect_dev0_run(hdl, (s16 *)frame->data, frame->len);

        if (node->oport) {
            jlstream_push_frame(node->oport, frame);	//将数据推到oport
        } else {
            jlstream_free_frame(frame);	//释放iport资源
        }
#endif
    }
}

/*节点预处理-在ioctl之前*/
static int effect_dev0_adapter_bind(struct stream_node *node, u16 uuid)
{
    struct effect_dev0_node_hdl *hdl = (struct effect_dev0_node_hdl *)node->private_data;
    memset(hdl, 0, sizeof(*hdl));
    return 0;
}

/*打开改节点输入接口*/
static void effect_dev0_ioc_open_iport(struct stream_iport *iport)
{
    iport->handle_frame = effect_dev0_handle_frame;				//注册输出回调
}

/*节点参数协商*/
static int effect_dev0_ioc_negotiate(struct stream_iport *iport)
{
    /* struct stream_oport *oport = iport->node->oport;
    struct stream_fmt *in_fmt = &iport->prev->fmt;*/

    return 0;
}

/*节点start函数*/
static void effect_dev0_ioc_start(struct effect_dev0_node_hdl *hdl)
{
    struct stream_fmt *fmt = &hdl_node(hdl)->oport->fmt;
    /* struct jlstream *stream = jlstream_for_node(hdl_node(hdl)); */


    hdl->sample_rate = fmt->sample_rate;
    hdl->ch_num = AUDIO_CH_NUM(fmt->channel_mode);

    /*
     *获取配置文件内的参数,及名字
     * */
    int len = jlstream_read_node_data_new(hdl_node(hdl)->uuid, hdl_node(hdl)->subid, (void *)&hdl->cfg, hdl->name);
    if (!len) {
        log_error("%s, read node data err\n", __FUNCTION__);
        return;
    }

    /*
     *获取在线调试的临时参数
     * */
    if (config_audio_cfg_online_enable) {
        if (jlstream_read_effects_online_param(hdl_node(hdl)->uuid, hdl->name, &hdl->cfg, sizeof(hdl->cfg))) {
            log_debug("get effect dev0 online param\n");
        }
    }
    printf("effect dev0 name : %s \n", hdl->name);
    for (int i = 0 ; i < 8; i++) {
        printf("cfg.int_param[%d] %d\n", i, hdl->cfg.int_param[i]);
    }
    for (int i = 0 ; i < 8; i++) {
        printf("cfg.float_param[%d] %d.%02d\n", i, (int)hdl->cfg.float_param[i], debug_digital(hdl->cfg.float_param[i]));
    }

    hdl->bit_width = hdl_node(hdl)->iport->prev->fmt.bit_wide;
    printf("effect_dev0_ioc_start, sr: %d, ch: %d, bitw: %d", hdl->sample_rate, hdl->ch_num, hdl->bit_width);
#if EFFECT_DEV0_FRAME_POINTS
    hdl->frame_len = EFFECT_DEV0_FRAME_POINTS * hdl->ch_num * (2 << hdl->bit_width);
    hdl->remain_buf = zalloc(hdl->frame_len);
#endif
    audio_effect_dev0_init(hdl);
}


/*节点stop函数*/
static void effect_dev0_ioc_stop(struct effect_dev0_node_hdl *hdl)
{
    audio_effect_dev0_exit();
#if EFFECT_DEV0_FRAME_POINTS
    hdl->remain_len = 0;
    if (hdl->remain_buf) {
        free(hdl->remain_buf);
        hdl->remain_buf = NULL;
    }
    if (hdl->pack_frame) {
        jlstream_free_frame(hdl->pack_frame);
        hdl->pack_frame = NULL;
    }
#endif
}

static int effect_dev0_ioc_update_parm(struct effect_dev0_node_hdl *hdl, int parm)
{
    int ret = false;
    return ret;
}
static int get_effect_dev0_ioc_parm(struct effect_dev0_node_hdl *hdl, int parm)
{
    int ret = 0;
    return ret;
}

static int effect_ioc_update_parm(struct effect_dev0_node_hdl *hdl, int parm)
{
    int ret = false;
    struct user_effect_tool_param *cfg = (struct user_effect_tool_param *)parm;
    if (hdl) {
        memcpy(&hdl->cfg, cfg, sizeof(struct user_effect_tool_param));
        //打印在线调音发送下来的参数
        printf("effect dev0 name : %s \n", hdl->name);
        for (int i = 0 ; i < 8; i++) {
            printf("cfg->int_param[%d] %d\n", i, cfg->int_param[i]);
        }
        for (int i = 0 ; i < 8; i++) {
            printf("cfg->float_param[%d] %d.%02d\n", i, (int)cfg->float_param[i], debug_digital(cfg->float_param[i]));
        }

        ret = true;
    }

    return ret;
}


/*节点ioctl函数*/
static int effect_dev0_adapter_ioctl(struct stream_iport *iport, int cmd, int arg)
{
    int ret = 0;
    struct effect_dev0_node_hdl *hdl = (struct effect_dev0_node_hdl *)iport->node->private_data;

    switch (cmd) {
    case NODE_IOC_OPEN_IPORT:
        effect_dev0_ioc_open_iport(iport);
        break;
    case NODE_IOC_OPEN_OPORT:
        break;
    case NODE_IOC_CLOSE_IPORT:
        break;
    case NODE_IOC_SET_SCENE:
        break;
    case NODE_IOC_NEGOTIATE:
        *(int *)arg |= effect_dev0_ioc_negotiate(iport);
        break;
    case NODE_IOC_START:
        effect_dev0_ioc_start(hdl);
        break;
    case NODE_IOC_SUSPEND:
    case NODE_IOC_STOP:
        effect_dev0_ioc_stop(hdl);
        break;
    case NODE_IOC_NAME_MATCH:
        if (!strcmp((const char *)arg, hdl->name)) {
            ret = 1;
        }
        break;

    case NODE_IOC_SET_PARAM:
        ret = effect_ioc_update_parm(hdl, arg);
        break;
    }

    return ret;
}

/*节点用完释放函数*/
static void effect_dev0_adapter_release(struct stream_node *node)
{
}

/*节点adapter 注意需要在sdk_used_list声明，否则会被优化*/
REGISTER_STREAM_NODE_ADAPTER(effect_dev0_node_adapter) = {
    .name       = "effect_dev0",
    .uuid       = NODE_UUID_EFFECT_DEV0,
    .bind       = effect_dev0_adapter_bind,
    .ioctl      = effect_dev0_adapter_ioctl,
    .release    = effect_dev0_adapter_release,
    .hdl_size   = sizeof(struct effect_dev0_node_hdl),
};

REGISTER_ONLINE_ADJUST_TARGET(effect_dev0) = {
    .uuid = NODE_UUID_EFFECT_DEV0,
};

#endif

