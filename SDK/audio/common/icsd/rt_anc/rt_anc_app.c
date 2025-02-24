#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".rt_anc.data.bss")
#pragma data_seg(".rt_anc.data")
#pragma const_seg(".rt_anc.text.const")
#pragma code_seg(".rt_anc.text")
#endif

#include "app_config.h"
#include "audio_config_def.h"

#if TCFG_AUDIO_ANC_ENABLE && TCFG_AUDIO_ANC_REAL_TIME_ADAPTIVE_ENABLE
#include "audio_anc.h"
#include "rt_anc.h"
#include "rt_anc_app.h"
#include "clock_manager/clock_manager.h"
#include "icsd_anc_user.h"

#include "icsd_adt.h"
#include "icsd_adt_app.h"

#if TCFG_AUDIO_FREQUENCY_GET_ENABLE
#include "icsd_afq_app.h"
#endif

#if ANC_EAR_ADAPTIVE_CMP_EN
#include "icsd_cmp_app.h"
#endif

struct audio_rt_anc_hdl {
    u8 seq;
    u8 state;
    u8 app_func_en;
    int suspend_cnt;
    audio_anc_t *param;
    float *debug_param;
};

static struct audio_rt_anc_hdl	*hdl = NULL;

#if AUDIO_RT_ANC_PARAM_BY_TOOL_DEBUG
//修改默认值
float rtanc_debug_param[10] = {
    -2.0f,  // MSE update thr
    200.0f,  // 50Hz jitter thr
    2.0f,  // dither thr
    2.0f,  // target update thr
    7.0f,  // pz self flt dB thr < -10

    7.0f,  // pz self flt dB thr > -10
    -2.0f,  // pz self flt num thr
    1.0f,  // speak flag
    0.0f,  // vod print
    1.0f,
};

void audio_rtanc_debug_param_set(u8 cmd, float param)
{
    u8 max_num = ARRAY_SIZE(rtanc_debug_param);
    if (cmd > max_num) {
        printf("Err : rtanc debug param set fail, size limit %d > %d\n", \
               cmd, max_num);
        return;
    }
    rtanc_debug_param[cmd] = param;
    printf("cmd %d, %d/100\n", cmd, (int)(rtanc_debug_param[cmd] * 100.0));
}
#endif

void audio_anc_real_time_adaptive_init(audio_anc_t *param)
{
    hdl = zalloc(sizeof(struct audio_rt_anc_hdl));
    hdl->param = param;

#if AUDIO_RT_ANC_PARAM_BY_TOOL_DEBUG
    hdl->debug_param = rtanc_debug_param;
#endif
}

/* Real Time ANC 自适应启动限制 */
int audio_rtanc_permit(void)
{
    if (anc_ext_ear_adaptive_param_check()) { //没有自适应参数
        return 1;
    }
    if (hdl->param->mode != ANC_ON) {	//非ANC模式
        return 2;
    }
    if (audio_anc_real_time_adaptive_busy_get()) { //重入保护
        return 3;
    }
    if (anc_mode_switch_lock_get()) { //其他切模式过程不支持
        return 4;
    }
#if TCFG_AUDIO_FREQUENCY_GET_ENABLE
    if (audio_icsd_afq_is_running()) {	//AFQ 运行过程中不支持
        return 5;
    }
#endif
    if (hdl->param->lff_yorder > RT_ANC_MAX_ORDER || \
        hdl->param->lfb_yorder > RT_ANC_MAX_ORDER || \
        hdl->param->lcmp_yorder > RT_ANC_MAX_ORDER || \
        hdl->param->rff_yorder > RT_ANC_MAX_ORDER || \
        hdl->param->rfb_yorder > RT_ANC_MAX_ORDER || \
        hdl->param->rcmp_yorder > RT_ANC_MAX_ORDER) {
        return 6;						//最大滤波器个数限制
    }
    return 0;
}

//将滤波器等相关信息，整合成库需要的格式
void rt_anc_get_param(__rt_anc_param *rt_param_l, __rt_anc_param *rt_param_r)
{
    g_printf("%s, %d\n", __func__, __LINE__);

    int yorder_size = 5 * sizeof(double);
    audio_anc_t *param = hdl->param;

    if (rt_param_l) {
        rt_param_l->ff_yorder = param->lff_yorder;
        rt_param_l->fb_yorder = param->lfb_yorder;
        rt_param_l->cmp_yorder = param->lcmp_yorder;
        rt_param_l->ffgain = param->gains.l_ffgain;
        rt_param_l->fbgain = param->gains.l_fbgain;
        rt_param_l->cmpgain = param->gains.l_cmpgain;

        memcpy(rt_param_l->ff_coeff, param->lff_coeff, rt_param_l->ff_yorder * yorder_size);
        memcpy(rt_param_l->fb_coeff, param->lfb_coeff, rt_param_l->fb_yorder * yorder_size);
        memcpy(rt_param_l->cmp_coeff, param->lcmp_coeff, rt_param_l->cmp_yorder * yorder_size);
        rt_param_l->param = param;
    }

    if (rt_param_r) {
        rt_param_r->ff_yorder = param->lff_yorder;
        rt_param_r->fb_yorder = param->lfb_yorder;
        rt_param_r->cmp_yorder = param->lcmp_yorder;
        rt_param_r->ffgain = param->gains.l_ffgain;
        rt_param_r->fbgain = param->gains.l_fbgain;
        rt_param_r->cmpgain = param->gains.l_cmpgain;

        memcpy(rt_param_r->ff_coeff, param->lff_coeff, rt_param_r->ff_yorder * yorder_size);
        memcpy(rt_param_r->fb_coeff, param->lfb_coeff, rt_param_r->fb_yorder * yorder_size);
        memcpy(rt_param_r->cmp_coeff, param->lcmp_coeff, rt_param_r->cmp_yorder * yorder_size);
        rt_param_r->param = param;
    }

}

//设置RTANC初始参数
int audio_adt_rtanc_set_infmt(void)
{
    printf("=================audio_adt_rtanc_set_infmt==================\n");
    if (++hdl->seq == 0xff) {
        hdl->seq = 0;
    }

    struct rt_anc_infmt infmt;
    infmt.id = hdl->seq;
    infmt.param = hdl->param;
    infmt.ext_cfg = (void *)anc_ext_ear_adaptive_cfg_get();
    infmt.anc_param_l = zalloc(sizeof(__rt_anc_param));
    //gali
    infmt.anc_param_r = zalloc(sizeof(__rt_anc_param));
    rt_anc_get_param(infmt.anc_param_l, infmt.anc_param_r);

    extern void rt_anc_set_init(struct rt_anc_infmt * fmt);
    rt_anc_set_init(&infmt);

#if AUDIO_RT_ANC_PARAM_BY_TOOL_DEBUG
    for (int i = 0; i < ARRAY_SIZE(rtanc_debug_param); i++) {
        printf("cmd %d, %d/100\n", i, (int)(hdl->debug_param[i] * 100.0));
        RTANC_SD_CFG->debug[i] = hdl->debug_param[i];
    }
#endif
    if (infmt.anc_param_l) {
        free(infmt.anc_param_l);
    }
    if (infmt.anc_param_r) {
        free(infmt.anc_param_r);
    }
    return 0;
}

static void audio_rt_anc_param_updata(void *rt_param_l, void *rt_param_r)
{
    printf("audio_rt_anc_param_updata\n");

    audio_anc_t *param = hdl->param;

#if ANC_EAR_ADAPTIVE_CMP_EN
    struct anc_cmp_param_output cmp_p;
    audio_rtanc_adaptive_cmp_output_get(&cmp_p);
    param->gains.l_cmpgain = cmp_p.l_gain;
    param->lcmp_coeff = cmp_p.l_coeff;
    printf("updata gain = %d, coef = %d, ", (int)(param->gains.l_cmpgain * 100), (int)(param->lcmp_coeff[0] * 100));
#endif

    __rt_anc_param *anc_param = (__rt_anc_param *)rt_param_l;
    param->gains.l_ffgain = anc_param->ffgain;
    param->lff_coeff = anc_param->ff_coeff;
    param->gains.l_fbgain = anc_param->fbgain;

    //param->lfb_coeff = &anc_param->lfb_coeff[0];
#if ANC_CONFIG_RFB_EN
    if (rt_param_r) {
        anc_param = (__rt_anc_param *)rt_param_r;
        param->gains.r_ffgain = anc_param->ffgain;
        param->rff_coeff = anc_param->ff_coeff;
        param->gains.r_fbgain = anc_param->fbgain;

#if ANC_EAR_ADAPTIVE_CMP_EN
        param->gains.r_cmpgain = cmp_p.r_gain;
        param->rcmp_coeff = cmp_p.r_coeff;
#endif
    }
#endif

    anc_coeff_online_update(param, 1);
}

//实时自适应算法输出接口
void audio_adt_rtanc_output_handle(void *rt_param_l, void *rt_param_r)
{
    //挂起RT_ANC
    audio_anc_real_time_adaptive_suspend();

    //整理SZ数据结构
    __afq_output output = {0};// = zalloc(sizeof(__afq_output));// = &temp;
    output.sz_l = zalloc(sizeof(__afq_data));
    output.sz_l->len = 120;
    output.sz_l->out = zalloc(sizeof(float) * 120);
    icsd_rtanc_alg_get_sz(output.sz_l->out);

    //put_buf((u8 *)output.sz_l->out, 120 * 4);

    //根据SZ计算CMP，并更新FF/FB/CMP等滤波器
#if ANC_EAR_ADAPTIVE_CMP_EN
    audio_anc_ear_adaptive_cmp_run(&output);
#endif
    audio_rt_anc_param_updata(rt_param_l, rt_param_r);

    //执行挂在到AFQ 的算法，如AEQ, 内部会挂起RTANC
    audio_afq_common_output_post_msg(&output);

    free(output.sz_l->out);
    free(output.sz_l);

    //恢复RT_ANC
    audio_anc_real_time_adaptive_resume();
}

//RT ANC挂起接口
void audio_anc_real_time_adaptive_suspend(void)
{
    if (audio_anc_real_time_adaptive_busy_get()) {
        hdl->suspend_cnt++;
        if ((hdl->suspend_cnt == 1) && (hdl->state == RT_ANC_STATE_OPEN)) {
            hdl->state = RT_ANC_STATE_SUSPEND;
            icsd_adt_rtanc_suspend();
            printf("%s succ\n", __func__);
        }
    }
}

//RT ANC恢复接口
void audio_anc_real_time_adaptive_resume(void)
{
    if (audio_anc_real_time_adaptive_busy_get()) {
        if (hdl->suspend_cnt) {
            hdl->suspend_cnt--;
            if ((hdl->suspend_cnt == 0) && (hdl->state == RT_ANC_STATE_SUSPEND)) {
                hdl->state = RT_ANC_STATE_OPEN;
                icsd_adt_rtanc_resume();
                printf("%s succ\n", __func__);
            }
        }
    }
}
int audio_rtanc_adaptive_en(u8 en)
{
    int ret;
    if (en) {
        printf("=================rt_anc_open==================\n");
        ret = audio_rtanc_permit();
        if (ret) {
            printf("Err:rt_anc_open permit %d\n", ret);
            return 1;
        }
        hdl->suspend_cnt = 0;
        hdl->state = RT_ANC_STATE_OPEN;
        clock_alloc("ANC_RT_ADAPTIVE", 48 * 1000000L);
#if ANC_EAR_ADAPTIVE_CMP_EN
        audio_anc_ear_adaptive_cmp_open();
#endif
        return audio_icsd_adt_sync_open(ADT_REAL_TIME_ADAPTIVE_ANC_MODE);

    } else {
        if (audio_anc_real_time_adaptive_busy_get() == 0) {
            return 0;
        }
        printf("================rt_anc_close==================\n");
        hdl->state = RT_ANC_STATE_CLOSE;
        audio_icsd_adt_sync_close(ADT_REAL_TIME_ADAPTIVE_ANC_MODE, 0);
        clock_free("ANC_RT_ADAPTIVE");

#if ANC_EAR_ADAPTIVE_CMP_EN
        audio_anc_ear_adaptive_cmp_close();
#endif

    }
    return 0;
}

//获取用户端是否启用这个功能
u8 audio_rtanc_app_func_en_get(void)
{
    return hdl->app_func_en;
}

int audio_anc_real_time_adaptive_open(void)
{
    hdl->app_func_en = 1;
    return audio_rtanc_adaptive_en(1);
}

int audio_anc_real_time_adaptive_close(void)
{
    int ret;
    hdl->app_func_en = 0;
    ret = audio_rtanc_adaptive_en(0);
    //恢复默认ANC参数
    if (anc_mode_get() == ANC_ON) {
#if ANC_MULT_ORDER_ENABLE
        audio_anc_mult_scene_set(audio_anc_mult_scene_get());
#else
        audio_anc_db_cfg_read();
#endif
    }
    return ret;
}

static u8 RTANC_idle_query(void)
{
    if (hdl) {
        return (hdl->state) ? 0 : 1;
    }
    return 1;
}

u8 audio_anc_real_time_adaptive_busy_get(void)
{
    return (!RTANC_idle_query());
}

REGISTER_LP_TARGET(RTANC_lp_target) = {
    .name       = "RTANC",
    .is_idle    = RTANC_idle_query,
};

//测试demo
int audio_anc_real_time_adaptive_demo(void)
{
#if 1//tone
    if (hdl->state == RT_ANC_STATE_CLOSE) {
        if (audio_anc_real_time_adaptive_open()) {
            icsd_adt_tone_play(ICSD_ADT_TONE_NUM0);
        } else {
            icsd_adt_tone_play(ICSD_ADT_TONE_NUM1);
        }
    } else {
        icsd_adt_tone_play(ICSD_ADT_TONE_NUM0);
        audio_anc_real_time_adaptive_close();
    }
#else
    if (hdl->state == RT_ANC_STATE_CLOSE) {
        audio_anc_real_time_adaptive_open();
    } else {
        audio_anc_real_time_adaptive_close();
    }
#endif
    return 0;
}

#endif
