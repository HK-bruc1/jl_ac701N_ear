#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".icsd_dot.data.bss")
#pragma data_seg(".icsd_dot.data")
#pragma const_seg(".icsd_dot.text.const")
#pragma code_seg(".icsd_dot.text")
#endif

//DOT degree of tightness

#include "app_config.h"
#include "audio_config_def.h"

#if (TCFG_AUDIO_FIT_DET_ENABLE && TCFG_AUDIO_ANC_ENABLE && \
	 TCFG_AUDIO_ANC_EXT_VERSION == ANC_EXT_V2)

#include "icsd_dot_app.h"
#include "audio_anc.h"
#include "clock_manager/clock_manager.h"

#if 1
#define dot_log printf
#else
#define dot_log (...)
#endif

struct audio_dot_t {

    volatile u8 state;				//状态
    int result;						//贴合度结果
    void *lib_alloc_ptr;			//dot申请的内存指针
    void (*result_cb)(int result);	//贴合度结果回调

    struct audio_afq_output *fre_out;		//实时SZ输出句柄
};

static void audio_icsd_dot_output_hdl(struct audio_afq_output *p);

static struct audio_dot_t *dot_hdl = NULL;

//保留现场及功能互斥
static void audio_icsd_dot_mutex_suspend(void)
{
    if (dot_hdl) {
    }
}

//恢复现场及功能互斥
static void audio_icsd_dot_mutex_resume(void)
{
    if (dot_hdl) {
    }
}

int audio_icsd_dot_permit(void)
{
    if (dot_hdl) {	//禁止重入
        return 1;
    }
    return 0;
}

int audio_icsd_dot_open(enum audio_adaptive_fre_sel fre_sel, void (*result_cb)(int result))
{
    struct icsd_dot_libfmt libfmt;
    struct icsd_dot_infmt  infmt;

    if (audio_icsd_dot_permit()) {
        dot_log("icsd_dot_permit, open fail\n");
        return 1;
    }

    dot_log("===================icsd_dot_init===================\n");

    dot_hdl = zalloc(sizeof(struct audio_dot_t));
    dot_hdl->result_cb = result_cb;

    //1.保留现场及功能互斥
    audio_icsd_dot_mutex_suspend();

    //2.初始化算法

    clock_alloc("ANC_DOT", 60 * 1000000L);

    //3.ANC初始化
    dot_hdl->state = ICSD_DOT_STATE_OPEN;

    //3. SZ获取
    audio_afq_common_add_output_handler("ANC_DOT", fre_sel, audio_icsd_dot_output_hdl);

    //cppcheck-suppress memleak
    return 0;
}

int audio_icsd_dot_close()
{
    dot_log("%s\n", __func__);
    if (dot_hdl) {
        if (dot_hdl->state == ICSD_DOT_STATE_OPEN) {
            if (strcmp(os_current_task(), "afq_common") == 0) {
                //dot close在ANC线程执行会造成死锁，需改为在APP任务执行
                dot_log("dot close post to app_core\n");
                int msg[2];
                msg[0] = (int)audio_icsd_dot_close;
                msg[1] = 1;
                int ret = os_taskq_post_type("app_core", Q_CALLBACK, 2, msg);
                if (ret) {
                    dot_log("dot taskq_post err\n");
                }
                return 0;
            }
            //删除频响来源回调，若来源结束，则关闭来源
            audio_afq_common_del_output_handler("ANC_DOT");

            dot_log("%s, ok\n", __func__);
            //恢复ANC相关状态
            audio_icsd_dot_mutex_resume();

            if (dot_hdl->result_cb) {
                dot_hdl->result_cb(dot_hdl->result);
            }

            clock_free("ANC_DOT");
            free(dot_hdl);
            dot_hdl = NULL;
        }
    }
    return 0;
}

//查询DOT是否活动中
u8 audio_icsd_dot_is_running(void)
{
    if (dot_hdl) {
        return dot_hdl->state;
    }
    return ICSD_DOT_STATE_CLOSE;
}

static void audio_icsd_dot_output_hdl(struct audio_afq_output *p)
{
    dot_hdl->fre_out = p;

    struct icsd_dot_libfmt libfmt;
    struct icsd_dot_infmt  infmt;
    icsd_dot_get_libfmt(&libfmt);
    infmt.alloc_ptr = zalloc(libfmt.lib_alloc_size);
    if (!infmt.alloc_ptr) {
        return;
    }
    icsd_dot_set_infmt(&infmt);
    _dot_output *output = icsd_dot_run(p->sz_l.out, p->sz_l.msc);

    if (output->state) {
        printf("DOT OUTPUT ERR~~~~\n");
    } else {
        dot_log("========================================= \n");
        dot_log("                     = %d/100 \n", (int)(output->dot_db * 100));
        dot_log("========================================= \n");

        float norm_thr = DOT_PARM->norm_thr;
        float loose_thr = DOT_PARM->loose_thr;
        if (output->dot_db > norm_thr) {
            dot_log(" dot: tight ");
            dot_hdl->result = AUDIO_FIT_DET_RESULT_TIGHT;
        } else if (output->dot_db > loose_thr) {
            dot_log(" dot: norm ");
            dot_hdl->result = AUDIO_FIT_DET_RESULT_NORMAL;
        } else { // < loose_thr
            dot_log(" dot: loose ");
            dot_hdl->result = AUDIO_FIT_DET_RESULT_LOOSE;
        }
    }
    free(infmt.alloc_ptr);

    audio_icsd_dot_close();
}

//贴合度轻量模块
float audio_icsd_dot_light_open(struct audio_afq_output *p)
{
    float dot_db;
    struct icsd_dot_libfmt libfmt;
    struct icsd_dot_infmt  infmt;
    icsd_dot_get_libfmt(&libfmt);
    infmt.alloc_ptr = zalloc(libfmt.lib_alloc_size);
    if (!infmt.alloc_ptr) {
        return -1;
    }
    icsd_dot_set_infmt(&infmt);
    _dot_output *output = icsd_dot_run(p->sz_l.out, p->sz_l.msc);
    dot_db = output->dot_db;
    free(infmt.alloc_ptr);
    return dot_db;
}

#endif/*TCFG_AUDIO_FIT_DET_ENABLE*/
