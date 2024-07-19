#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".icsd_dot.data.bss")
#pragma data_seg(".icsd_dot.data")
#pragma const_seg(".icsd_dot.text.const")
#pragma code_seg(".icsd_dot.text")
#endif

//DOT degree of tightness

#include "app_config.h"
#if ((defined TCFG_AUDIO_FIT_DET_ENABLE) && TCFG_AUDIO_FIT_DET_ENABLE && \
	 TCFG_AUDIO_ANC_ENABLE)

#include "icsd_dot_app.h"
#include "audio_anc.h"
#include "dac_node.h"
#include "clock_manager/clock_manager.h"
#include "jlstream.h"
#include "app_tone.h"

#if TCFG_USER_TWS_ENABLE
#include "bt_tws.h"
#include "tws_tone_player.h"
#endif/*TCFG_USER_TWS_ENABLE*/

#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
#include "icsd_adt.h"
#include "icsd_adt_app.h"
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/

#if 1
#define dot_printf printf
#else
#define dot_printf (...)
#endif

struct audio_dot_t {

    struct icsd_dot_libfmt libfmt;	//获取算法库需要的参数
    struct icsd_dot_infmt infmt;	//输入算法库的参数

    void *lib_alloc_ptr;			//dot申请的内存指针
    void (*result_cb)(u8 result);	//贴合度结果回调

    volatile u8 state;				//状态
    volatile u8 tone_state;			//提示音状态
    u8 last_anc_mode;				//记录ANC上一个模式
    u8 last_alogm;					//ANC采样率
    u8 dac_check_flag;				//DAC静音校验标志
#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
    u8 icsd_adt_state;
#endif
};

static struct audio_dot_t *dot_hdl = NULL;

extern struct audio_dac_hdl dac_hdl;

static void audio_icsd_dot_dac_check_slience_cb(void *buf, int len)
{
    if (dot_hdl) {
        if (dot_hdl->dac_check_flag) {
            s16 *check_buf = (s16 *)buf;
            for (int i = 0; i < len / 2; i++) {
                if (check_buf[i]) {
                    dot_hdl->dac_check_flag = 0;
                    //触发
                    dot_printf("%s ok\n", __func__);
                    icsd_dot_tone_start(i, dac_hdl.sample_rate);
                    dac_node_write_callback_del("ANC_DOT");
                    break;
                }
            }
        }
    }
}

static int audio_dot_tone_play_cb(void *priv, enum stream_event event)
{
    /* if (event != STREAM_EVENT_STOP) { */
    /*     return 0; */
    /* } */
    /*  */
    dot_printf("%s: %d\n", __func__, event);
    if ((event != STREAM_EVENT_START) && (event != STREAM_EVENT_STOP) && (event != STREAM_EVENT_INIT)) {
        /*没有提示音，或者提示音打断*/
        if (dot_hdl) {
            dot_hdl->state = ICSD_DOT_STATE_CLOSE;
            dot_hdl->tone_state = ICSD_DOT_STATE_TONE_STOP;
            audio_icsd_dot_close();
        }
    }

    //提示音播放结束
    if (event == STREAM_EVENT_STOP) {
#if ICSD_DOT_EN_MODE_NEXT_SW
        if (dot_hdl) {
            dot_hdl->tone_state = ICSD_DOT_STATE_TONE_STOP;
            audio_icsd_dot_close();
        }
#endif
    }
    return 0;
}

int audio_icsd_dot_end(int result_thr)
{
    printf("audio_icsd_dot_end\n");
    if (dot_hdl) {
        if (dot_hdl->result_cb) {
            dot_hdl->result_cb(result_thr);
        }
        dot_hdl->state = ICSD_DOT_STATE_CLOSE;
        audio_icsd_dot_close();
    }
    return 0;
}

static void tws_dot_tone_callback(int priv, enum stream_event event)
{
    r_printf("tws_dot_tone_callback: %d\n",  event);
    audio_dot_tone_play_cb(NULL, event);
}

#define TWS_DOT_TONE_STUB_FUNC_UUID   0xD14A6159
REGISTER_TWS_TONE_CALLBACK(tws_anc_tone_stub) = {
    .func_uuid  = TWS_DOT_TONE_STUB_FUNC_UUID,
    .callback   = tws_dot_tone_callback,
};

//提示音播放
static int audio_icsd_dot_tone_play(void)
{
    int ret = 0;
    dot_printf("%s\n", __func__);

#if ICSD_DOT_EN_MODE_NEXT_SW
    dot_hdl->tone_state = ICSD_DOT_STATE_TONE_START;
#endif

#if TCFG_USER_TWS_ENABLE
    if (get_tws_sibling_connect_state()) {
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            ret = tws_play_tone_file_callback(get_tone_files()->fit_det_on, 400, TWS_DOT_TONE_STUB_FUNC_UUID);
        }
    } else {
        ret = tws_play_tone_file_callback(get_tone_files()->fit_det_on, 400, TWS_DOT_TONE_STUB_FUNC_UUID);
    }
#else
    ret = play_tone_file_callback(get_tone_files()->fit_det_on, NULL, anc_tone_play_cb);
#endif
    return ret;
}

//保留现场及功能互斥
static void audio_icsd_dot_mutex_suspend(void)
{
    if (dot_hdl) {
#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
        /*进入贴合度前关闭adt*/
        dot_hdl->icsd_adt_state = audio_icsd_adt_is_running();
        if (dot_hdl->icsd_adt_state) {
            audio_icsd_adt_close(0, 1);
        }

        printf("======================= %d", audio_icsd_adt_is_running());
        int cnt;
        //adt关闭时间较短，预留100ms
        for (cnt = 0; cnt < 10; cnt++) {
            if (!audio_icsd_adt_is_running()) {
                break;
            }
            os_time_dly(1);  //  等待ADT 关闭
        }
        if (cnt == 10) {
            printf("Err:dot_suspend adt wait timeout\n");
        }
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/

        dot_hdl->last_alogm = get_anc_gains_alogm();
        dot_hdl->last_anc_mode = anc_mode_get();
    }
}

//恢复现场及功能互斥
static void audio_icsd_dot_mutex_resume(void)
{
    if (dot_hdl) {
        //恢复ANC相关状态
        set_anc_gains_alogm(dot_hdl->last_alogm);			//设置ANC采样率
        anc_mode_switch(dot_hdl->last_anc_mode, 0);

        //按ANC最低采样率的淡出时间预留1.5s, 常用配置150ms以内
        int cnt;
        for (cnt = 0; cnt < 150; cnt++) {
            if (!anc_mode_switch_lock_get()) {
                break;
            }
            os_time_dly(1);  //  等待ANC模式切换完毕
        }
        if (cnt == 150) {
            printf("Err:dot_resume switch_lock wait timeout\n");
        }

#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
        if (dot_hdl->icsd_adt_state) {
            audio_icsd_adt_open(0);
        }
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/
    }
}

int audio_icsd_dot_permit(void)
{
    if (dot_hdl) {
        return 1;
    }
    //ANC训练过程不允许打开
    if (anc_train_open_query()) {
        return 1;
    }
#if TCFG_AUDIO_ANC_EAR_ADAPTIVE_EN
    //耳道自适应过程不允许打开
    if (anc_ear_adaptive_busy_get()) {
        return 1;
    }
#endif
    return 0;
}

int audio_icsd_dot_open(u8 tone_en, void (*result_cb)(u8 result))
{
    if (audio_icsd_dot_permit()) {
        dot_printf("icsd_dot_permit, open fail\n");
        return 1;
    }

    dot_printf("===================icsd_dot_init===================\n");
    icsd_dot_version();

    dot_hdl = zalloc(sizeof(struct audio_dot_t));

    dot_hdl->state = ICSD_DOT_STATE_CLOSE;
    dot_hdl->result_cb = result_cb;

    //1.保留现场及功能互斥
    audio_icsd_dot_mutex_suspend();

    icsd_dot_get_libfmt(&dot_hdl->libfmt);
    dot_printf("DOT RAM SIZE:%d\n", dot_hdl->libfmt.lib_alloc_size);
    dot_hdl->lib_alloc_ptr = zalloc(dot_hdl->libfmt.lib_alloc_size);
    if (!dot_hdl->lib_alloc_ptr) {
        return -1;
    }
    dot_hdl->infmt.alloc_ptr = dot_hdl->lib_alloc_ptr;
    dot_hdl->infmt.tone_delay = 700;
    dot_hdl->infmt.dot_dma_on = anc_dma_on;
    icsd_dot_set_infmt(&dot_hdl->infmt);
    //2.初始化算法
    icsd_dot_parm_init();
    icsd_dot_data_debug_en(0);
    icsd_dot_line_debug_en(0);
    clock_alloc("ANC_DOT", 60 * 1000000L);

    //3.ANC初始化
    set_anc_gains_alogm(dot_hdl->libfmt.alogm);			//设置ANC采样率
    anc_mode_switch(ANC_EXT, 0);	//进入ANC扩展模式
    dot_hdl->state = ICSD_DOT_STATE_OPEN;

    //按ANC最低采样率的淡出时间预留1.5s, 常用配置150ms以内
    int cnt;
    for (cnt = 0; cnt < 150; cnt++) {
        if (!anc_mode_switch_lock_get()) {
            break;
        }
        os_time_dly(1);  //  等待ANC模式切换完毕
    }
    if (cnt == 150) {
        printf("Err:dot_open switch_lock wait timeout\n");
    }
    //4.播放提示音
    if (tone_en) {
        audio_icsd_dot_tone_play();
    }

    dac_node_write_callback_add("ANC_DOT", 0XFF, audio_icsd_dot_dac_check_slience_cb);	//注册DAC回调接口-静音检测
    dot_hdl->dac_check_flag = 1;

    //cppcheck-suppress memleak
    return 0;
}

int audio_icsd_dot_close()
{
    dot_printf("%s\n", __func__);
    if (dot_hdl) {
        if (dot_hdl->state || dot_hdl->tone_state) {
            return 1;
        }
        if (strcmp(os_current_task(), "anc") == 0) {
            //dot close在ANC线程执行会造成死锁，需改为在APP任务执行
            dot_printf("dot close post to app_core\n");
            int msg[2];
            msg[0] = (int)audio_icsd_dot_close;
            msg[1] = 1;
            int ret = os_taskq_post_type("app_core", Q_CALLBACK, 2, msg);
            if (ret) {
                dot_printf("dot taskq_post err\n");
            }
            return 0;
        }
        dot_printf("%s, ok\n", __func__);
        //恢复ANC相关状态
        audio_icsd_dot_mutex_resume();

        clock_free("ANC_DOT");
        free(dot_hdl->lib_alloc_ptr);
        free(dot_hdl);
        dot_hdl = NULL;
    }
    return 0;
}

//查询DOT是否活动中
u8 audio_icsd_dot_is_running(void)
{
    if (dot_hdl) {
        return (dot_hdl->state | dot_hdl->tone_state);
    }
    return ICSD_DOT_STATE_CLOSE;
}

void audio_icsd_dot_anc_dma_done(void)
{
    if (dot_hdl && dot_hdl->state) {
        icsd_dot_anc_dma_done();
    }
}


#endif/*TCFG_AUDIO_FIT_DET_ENABLE*/
