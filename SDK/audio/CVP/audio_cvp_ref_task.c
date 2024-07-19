#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".audio_cvp_ref_task.data.bss")
#pragma data_seg(".audio_cvp_ref_task.data")
#pragma const_seg(".audio_cvp_ref_task.text.const")
#pragma code_seg(".audio_cvp_ref_task.text")
#endif
/*
 ****************************************************************
 *							AUDIO CVP REF SRC
 * File  : audio_cvp_ref_src.c
 * By    :
 * Notes : CVP回音消除的外部参考数变采样
 *
 ****************************************************************
 */
#include "audio_cvp.h"
#include "system/includes.h"
#include "media/includes.h"
#include "circular_buf.h"
#include "Resample_api.h"
#include "media/audio_general.h"

#define CVP_REF_SRC_TASK_NAME "CVP_RefTask"

typedef struct {
    RS_STUCT_API *sw_src_api;
    u32 *sw_src_buf;
    u8 busy;
    u8 ref_busy;
    u32 input_rate;
    u32 output_rate;
    s16 *ref_buf;
    int ref_buf_size;
    u8 iport_channel_mode;	//保存输入节点的声道数
    u8 buf_cnt;						//循环输入buffer位置
    u8 buf_bulk;    //循环buf数
    u8 data_width; //数据位宽，0：16bit，1：24bit
} cvp_ref_src_t;
cvp_ref_src_t *cvp_ref_src = NULL;

extern void pcm_dual_to_single(void *out, void *in, u16 len);
extern void pcm_dual_to_single_32bit(void *out, void *in, u16 len);
extern void audio_convert_data_32bit_to_16bit_round(s32 *in, s16 *out, u32 npoint);

void audio_cvp_ref_src_run(s16 *data, int len);
static void audio_cvp_ref_src_task(void *p)
{
    int msg[16];
    while (1) {
        os_taskq_pend("taskq", msg, ARRAY_SIZE(msg));
        if (cvp_ref_src) {
            cvp_ref_src->busy = 1;
            s16 *data = (s16 *)msg[1];
            int len = msg[2];

            audio_cvp_ref_start(1);
            audio_cvp_ref_src_run(data, len);
            cvp_ref_src->busy = 0;
        }
    }
}

/*
*********************************************************************
*                  Audio CVP Ref Src
* Description: 打开外部参考数据变采样
* Arguments  : input_rate	输入采样率
*			   output_rate	输出采样率
*			   iport_channel_mode	数据输入通道类型
* Return	 : None.
* Note(s)    : 声卡设备是DAC，默认不用外部提供参考数据
*********************************************************************
*/
void audio_cvp_ref_src_open(u32 input_rate, u32 output_rate, u8 iport_channel_mode)
{
    if (cvp_ref_src) {
        printf("cvp_ref_src alreadly open !!!");
        return;
    }
    cvp_ref_src_t *hdl = zalloc(sizeof(cvp_ref_src_t));
    if (hdl == NULL) {
        printf("cvp_ref_src malloc fail !!!");
        return;
    }

    printf("insr: %d, outsr: %d, in_ch: %d", input_rate, output_rate, iport_channel_mode);
    hdl->input_rate = input_rate;
    hdl->output_rate = output_rate;
    hdl->iport_channel_mode = iport_channel_mode;
    hdl->buf_cnt = 0;
    hdl->buf_bulk = 1;
    hdl->data_width = audio_general_out_dev_bit_width();

    /*输入输出采样率不一样时，新建任务做变采样*/
    if (hdl->output_rate != hdl->input_rate) {
        int err = task_create(audio_cvp_ref_src_task, hdl, CVP_REF_SRC_TASK_NAME);
        if (err != OS_NO_ERR) {
            printf("task create error!");
            free(hdl);
            hdl = NULL;
            return;
        }

        hdl->buf_bulk = 3;

        hdl->sw_src_api = get_rs16_context();
        printf("sw_src_api:0x%x\n", (int)(hdl->sw_src_api));
        ASSERT(hdl->sw_src_api);
        int sw_src_need_buf = hdl->sw_src_api->need_buf();
        printf("sw_src_buf:%d\n", sw_src_need_buf);
        hdl->sw_src_buf = zalloc(sw_src_need_buf);
        ASSERT(hdl->sw_src_buf, "sw_src_buf zalloc fail");
        RS_PARA_STRUCT rs_para_obj;
        rs_para_obj.nch = 1;
        rs_para_obj.new_insample = hdl->output_rate;
        rs_para_obj.new_outsample = hdl->input_rate;
        rs_para_obj.dataTypeobj.IndataBit = 0;
        rs_para_obj.dataTypeobj.OutdataBit = 0;
        rs_para_obj.dataTypeobj.IndataInc = 1;
        rs_para_obj.dataTypeobj.OutdataInc = 1;
        rs_para_obj.dataTypeobj.Qval = 15;

        printf("sw src,ch = %d, in = %d,out = %d\n", rs_para_obj.nch, rs_para_obj.new_insample, rs_para_obj.new_outsample);
        hdl->sw_src_api->open(hdl->sw_src_buf, &rs_para_obj);
    }
    /*申请缓存*/
    hdl->ref_buf_size = 512;
    hdl->ref_buf = zalloc(hdl->ref_buf_size * hdl->buf_bulk);
    cvp_ref_src = hdl;
}

void audio_cvp_ref_src_run(s16 *data, int len)
{
    cvp_ref_src_t *hdl = cvp_ref_src;
    u16 ref_len = len;
    s16 *ref_data = data;
    if (hdl) {
        if (AUDIO_CH_NUM(hdl->iport_channel_mode) == 2) {
            /*双变单*/
            if (hdl->data_width) {
                pcm_dual_to_single_32bit(ref_data, ref_data, ref_len);
            } else {
                pcm_dual_to_single(ref_data, ref_data, ref_len);
            }
            ref_len >>= 1;
        }

        if (hdl->data_width) {
            audio_convert_data_32bit_to_16bit_round((s32 *)ref_data, ref_data, ref_len / 4);
            ref_len >>= 1;
        }

        /* printf("%d %d \n",input_rate,output_rate); */
        if (hdl->sw_src_api) {
            /* hdl->sw_src_api->set_sr(hdl->sw_src_buf, hdl->output_rate); */
            ref_len = hdl->sw_src_api->run(hdl->sw_src_buf, ref_data, ref_len >> 1, ref_data);
            ref_len <<= 1;
        }
        audio_aec_refbuf(ref_data, NULL, ref_len);
    }
}

void audio_cvp_ref_src_close()
{
    cvp_ref_src_t *hdl = cvp_ref_src;
    if (hdl) {
        printf("cvp_ref_src_task wait idle:%d,%d\n", hdl->busy, hdl->ref_busy);
        while (hdl->busy || (hdl->ref_busy)) {
            putchar('w');
            os_time_dly(1);
        }
        if (hdl->sw_src_api) {
            int err = task_kill(CVP_REF_SRC_TASK_NAME);
            if (err) {
                printf("kill task %s: err=%d\n", CVP_REF_SRC_TASK_NAME, err);
            }

            hdl->sw_src_api = NULL;
        }
        if (hdl->sw_src_buf) {
            free(hdl->sw_src_buf);
            hdl->sw_src_buf = NULL;
        }
        if (hdl->ref_buf) {
            free(hdl->ref_buf);
        }
        free(hdl);
        hdl = NULL;
        cvp_ref_src = NULL;
    }
}

void audio_cvp_ref_src_data_fill(s16 *data, int len)
{
    cvp_ref_src_t *hdl = cvp_ref_src;
    s16 *dat;
    u16 ref_len = len;
    if (hdl) {
        hdl->ref_busy = 1;
        /*如果需要的缓存不够重新申请*/
        if (hdl->ref_buf_size < len) {
            hdl->ref_buf_size = len;
            free(hdl->ref_buf);
            hdl->ref_buf = zalloc(hdl->ref_buf_size * hdl->buf_bulk);
        }
        dat = hdl->ref_buf + (hdl->ref_buf_size / 2 * hdl->buf_cnt);
        memcpy(dat, data, len);
        if (hdl->sw_src_api) {
            os_taskq_post_msg(CVP_REF_SRC_TASK_NAME, 2, (int)(dat), len);
            if (++hdl->buf_cnt > (hdl->buf_bulk - 1)) {
                hdl->buf_cnt = 0;
            }
        } else {
            /*输入输出采样率一样时，不用任务做变采样*/
            if (AUDIO_CH_NUM(hdl->iport_channel_mode) == 2) {
                /*双变单*/
                if (hdl->data_width) {
                    pcm_dual_to_single_32bit(dat, dat, ref_len);
                } else {
                    pcm_dual_to_single(dat, dat, ref_len);
                }
                ref_len >>= 1;
            }

            if (hdl->data_width) {
                audio_convert_data_32bit_to_16bit_round((s32 *)dat, dat, ref_len / 4);
                ref_len >>= 1;
            }
            audio_cvp_ref_start(1);
            audio_aec_refbuf(dat, NULL, ref_len);
        }
        hdl->ref_busy = 0;
    }
}

