#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".icsd_aeq.data.bss")
#pragma data_seg(".icsd_aeq.data")
#pragma const_seg(".icsd_aeq.text.const")
#pragma code_seg(".icsd_aeq.text")
#endif

#include "app_config.h"
#include "audio_config_def.h"

#if (TCFG_AUDIO_ADAPTIVE_EQ_ENABLE && TCFG_AUDIO_ANC_ENABLE && \
	 TCFG_AUDIO_ANC_EXT_VERSION == ANC_EXT_V2) && \
	(defined TCFG_EQ_ENABLE)

#include "icsd_aeq_app.h"
#include "icsd_aeq_config.h"
#include "audio_anc.h"
#include "jlstream.h"
#include "node_uuid.h"
#include "clock_manager/clock_manager.h"
#include "audio_config.h"
#include "volume_node.h"
#include "audio_anc_debug_tool.h"

#if TCFG_USER_TWS_ENABLE
#include "bt_tws.h"
#endif/*TCFG_USER_TWS_ENABLE*/

#if TCFG_AUDIO_FIT_DET_ENABLE
#include "icsd_dot_app.h"
#endif

#if TCFG_AUDIO_ANC_EAR_ADAPTIVE_EN
#include "icsd_anc_v2_interactive.h"
#endif

#if 1
#define aeq_printf printf
#else
#define aeq_printf (...)
#endif

struct audio_aeq_t {
    u8 eff_mode;							//AEQ效果模式
    enum audio_adaptive_fre_sel fre_sel;	//AEQ数据来源
    volatile u8 state;						//状态
    s16 now_volume;							//当前EQ参数的音量
    void *lib_alloc_ptr;					//AEQ申请的内存指针
    float *sz_ref;							//金机参考SZ
    float *sz_dut_cmp;						//产测补偿SZ
    struct list_head head;
    spinlock_t lock;
    struct audio_afq_output *fre_out;		//实时SZ输出句柄
    struct eq_default_seg_tab *eq_ref;		//参考EQ
    void (*result_cb)(int msg);			//AEQ训练结束回调
};

struct audio_aeq_bulk {
    struct list_head entry;
    s16 volume;
    struct eq_default_seg_tab *eq_output;
};

/*
	佩戴松紧度阈值thr
	1、thr > DOT_NORM_THR 				  	判定:紧
	2、DOT_NORM_THR >= thr > DOT_LOOSE_THR 	判定:正常
	3、DOT_LOOSE_THR  >= thr				判定:松
*/
#define DOT_NORM_THR		0.0f
#define DOT_LOOSE_THR		-6.0f

/*
   AEQ 根据音量分档表，音量从小到大排列；
   一般建议分3档以下，档位越多，AEQ时间越长，需要空间更大
*/
u8 aeq_volume_grade_list[] = {
    5,	//0-5
    10,	//6-10
    16,	//11-16
};

u8 aeq_volume_grade_maxdB_table[3][3] = {
    {
        //佩戴-紧
        22,//0-5
        12,//6-10
        3, //11-16
    },
    {
        //佩戴-正常
        20,
        14,
        2,
    },
    {
        //佩戴-松
        15,
        10,
        1,
    },
};

anc_packet_data_t *adaptive_eq_data = NULL;

static struct eq_seg_info test_eq_tab_normal[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,    0, 0.7f},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,    0, 0.7f},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,   0, 0.7f},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,   0, 0.7f},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   0, 0.7f},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  0, 0.7f},
    {6, EQ_IIR_TYPE_LOW_PASS, 2000,  0, 0.7f},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,  0, 0.7f},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,  0, 0.7f},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000, 0, 0.7f},
};

static struct audio_aeq_t *aeq_hdl = NULL;
struct eq_default_seg_tab default_seg_tab = {
    .global_gain = 0.0f,
    .seg_num = ARRAY_SIZE(test_eq_tab_normal),
    .seg = test_eq_tab_normal,
};

static void audio_adaptive_eq_afq_output_hdl(struct audio_afq_output *p);
static struct eq_default_seg_tab *audio_adaptive_eq_cur_list_query(s16 volume);
static float audio_adaptive_eq_vol_gain_get(s16 volume);
static struct eq_default_seg_tab *audio_icsd_eq_default_tab_read();
static int audio_icsd_eq_eff_update(struct eq_default_seg_tab *eq_tab);

//保留现场及功能互斥
static void audio_adaptive_eq_mutex_suspend(void)
{
    if (aeq_hdl) {
    }
}

//恢复现场及功能互斥
static void audio_adaptive_eq_mutex_resume(void)
{
    if (aeq_hdl) {
    }
}

static int audio_adaptive_eq_permit(enum audio_adaptive_fre_sel fre_sel)
{
    if (aeq_hdl->state != ADAPTIVE_EQ_STATE_CLOSE) {
        return 1;
    }
    return 0;
}

int audio_adaptive_eq_init(void)
{
    aeq_hdl = zalloc(sizeof(struct audio_aeq_t));
    spin_lock_init(&aeq_hdl->lock);
    INIT_LIST_HEAD(&aeq_hdl->head);
    ASSERT(aeq_hdl);
    return 0;
}

int audio_adaptive_eq_open(enum audio_adaptive_fre_sel fre_sel, void (*result_cb)(int msg))
{
    if (audio_adaptive_eq_permit(fre_sel)) {
        aeq_printf("adaptive_eq_permit, open fail\n");
        return 1;
    }

    aeq_printf("===================adaptive_eq_init:from %d===================\n", fre_sel);
    mem_stats();

    //1.保留现场及功能互斥
    audio_adaptive_eq_mutex_suspend();

    clock_alloc("ANC_AEQ", 160 * 1000000L);

    aeq_hdl->state = ADAPTIVE_EQ_STATE_OPEN;
    aeq_hdl->fre_sel = fre_sel;
    aeq_hdl->result_cb = result_cb;

    //2.准备算法输入参数
    if (aeq_hdl->eq_ref && (aeq_hdl->eq_ref != &default_seg_tab)) {
        free(aeq_hdl->eq_ref->seg);
        free(aeq_hdl->eq_ref);
    }
#if ADAPTIVE_EQ_TARGET_DEFAULT_CFG_READ
    aeq_hdl->eq_ref = audio_icsd_eq_default_tab_read();
#endif
    if (!aeq_hdl->eq_ref) {
        aeq_printf("AEQ read default param fail!\n");
        aeq_hdl->eq_ref = &default_seg_tab;
    }
    /* aeq_hdl->eq_ref = &default_seg_tab; */

    // EQ参数修改为默认
    audio_icsd_eq_eff_update(aeq_hdl->eq_ref);
    //输入算法前，dB转线性值
    aeq_hdl->eq_ref->global_gain = eq_db2mag(aeq_hdl->eq_ref->global_gain);

    //3. SZ获取
    audio_afq_common_add_output_handler("ANC_AEQ", fre_sel, audio_adaptive_eq_afq_output_hdl);

    //cppcheck-suppress memleak
    return 0;
}

int audio_adaptive_eq_close()
{
    aeq_printf("%s\n", __func__);
    if (aeq_hdl) {
        if (aeq_hdl->state == ADAPTIVE_EQ_STATE_OPEN) {
            if (strcmp(os_current_task(), "afq_common") == 0) {
                //aeq close在AEQ线程执行会造成死锁，需改为在APP任务执行
                aeq_printf("aeq close post to app_core\n");
                int msg[2];
                msg[0] = (int)audio_adaptive_eq_close;
                msg[1] = 1;
                int ret = os_taskq_post_type("app_core", Q_CALLBACK, 2, msg);
                if (ret) {
                    aeq_printf("aeq taskq_post err\n");
                }
                return 0;
            }
            //删除频响来源回调，若来源结束，则关闭来源
            audio_afq_common_del_output_handler("ANC_AEQ");

            aeq_printf("%s, ok\n", __func__);
            //恢复ANC相关状态
            audio_adaptive_eq_mutex_resume();

            if (aeq_hdl->eq_ref && (aeq_hdl->eq_ref != &default_seg_tab)) {
                if (aeq_hdl->eq_ref->seg) {
                    free(aeq_hdl->eq_ref->seg);
                }
                free(aeq_hdl->eq_ref);
                aeq_hdl->eq_ref = NULL;
            }
            clock_free("ANC_AEQ");

            //在线更新EQ效果
            aeq_hdl->eff_mode = AEQ_EFF_MODE_ADAPTIVE;
            s16 volume = app_audio_get_volume(APP_AUDIO_STATE_MUSIC);
            aeq_hdl->now_volume = volume;
            audio_icsd_eq_eff_update(audio_adaptive_eq_cur_list_query(volume));

            aeq_hdl->state = ADAPTIVE_EQ_STATE_CLOSE;

            if (aeq_hdl->result_cb) {
                aeq_hdl->result_cb(0);
            }
        }
    }
    return 0;
}

//查询aeq是否活动中
u8 audio_adaptive_eq_is_running(void)
{
    if (aeq_hdl) {
        return aeq_hdl->state;
    }
    return ADAPTIVE_EQ_STATE_CLOSE;
}

//立即更新EQ参数
static int audio_icsd_eq_eff_update(struct eq_default_seg_tab *eq_tab)
{
    if (!eq_tab) {
        return 1;
    }

    aeq_printf("%s\n", __func__);
    if (cpu_in_irq()) {
        aeq_printf("AEQ_eff update post app_core\n");
        int msg[3];
        msg[0] = (int)audio_icsd_eq_eff_update;
        msg[1] = 1;
        msg[2] = (int)eq_tab;
        int ret = os_taskq_post_type("app_core", Q_CALLBACK, 3, msg);
        if (ret) {
            aeq_printf("aeq taskq_post err\n");
        }
        return 0;
    }
#if 0
    aeq_printf("global_gain %d/100\n", (int)(eq_tab->global_gain * 100.f));
    aeq_printf("seg_num %d\n", eq_tab->seg_num);
    for (int i = 0; i < eq_tab->seg_num; i++) {
        aeq_printf("index %d\n", eq_tab->seg[i].index);
        aeq_printf("iir_type %d\n", eq_tab->seg[i].iir_type);
        aeq_printf("freq %d\n", eq_tab->seg[i].freq);
        aeq_printf("gain %d/100\n", (int)(eq_tab->seg[i].gain * 100.f));
        aeq_printf("q %d/100\n", (int)(eq_tab->seg[i].q * 100.f));
    }
#endif
    /*
     *更新用户自定义总增益
     * */
    struct eq_adj eff = {0};
    char *node_name = ADAPTIVE_EQ_TARGET_NODE_NAME;//节点名称（节点内的第一参数，用户自定义,长度小于等于15byte）
    eff.param.global_gain =  eq_tab->global_gain;       //设置总增益 -5dB
    eff.fade_parm.fade_time = 10;      //ms，淡入timer执行的周期
    eff.fade_parm.fade_step = 0.1f;    //淡入步进

    eff.type = EQ_GLOBAL_GAIN_CMD;      //参数类型：总增益
    int ret = jlstream_set_node_param(NODE_UUID_EQ, node_name, &eff, sizeof(eff));
    if (!ret) {
        puts("error\n");
    }
    /*
     *更新系数表段数(如系数表段数发生改变，需要用以下处理，更新系数表段数)
     * */
    eff.param.seg_num = eq_tab->seg_num;           //系数表段数
    eff.type = EQ_SEG_NUM_CMD;         //参数类型：系数表段数
    ret = jlstream_set_node_param(NODE_UUID_EQ, node_name, &eff, sizeof(eff));
    if (!ret) {
        puts("error\n");
    }
    /*
     *更新用户自定义的系数表
     * */
    eff.type = EQ_SEG_CMD;             //参数类型：滤波器参数
    for (int j = 0; j < eq_tab->seg_num; j++) {
        memcpy(&eff.param.seg, &(eq_tab->seg[j]), sizeof(struct eq_seg_info));//滤波器参数
        jlstream_set_node_param(NODE_UUID_EQ, node_name, &eff, sizeof(eff));
    }
    return 0;
}

/*
   读取可视化工具stream.bin的配置;
   如复用EQ节点，若用户用EQ节点，并自定义参数，需要在更新参数的时候通知AEQ算法
 */
static struct eq_default_seg_tab *audio_icsd_eq_default_tab_read()
{
    /*
     *解析配置文件内效果配置
     * */
    struct eq_default_seg_tab *eq_ref = NULL;
    struct cfg_info info  = {0};             //节点配置相关信息（参数存储的目标地址、配置项大小）
    char mode_index       = 0;               //模式序号（当前节点无多参数组，mode_index是0）
    char *node_name       = ADAPTIVE_EQ_TARGET_NODE_NAME;       //节点名称（节点内的第一参数，用户自定义,长度小于等于15byte）
    char cfg_index        = 0;               //目标配置项序号（当前节点无多参数组，cfg_index是0）
    int ret = jlstream_read_form_node_info_base(mode_index, node_name, cfg_index, &info);
    if (!ret) {
        struct eq_tool *tab = zalloc(info.size);
        if (!jlstream_read_form_cfg_data(&info, tab)) {
            printf("[AEQ]user eq cfg parm read err\n");
            free(tab);
            return NULL;
        }
#if 0
        aeq_printf("global_gain %d/100\n", (int)(tab->global_gain * 100.f));
        aeq_printf("is_bypass %d\n", tab->is_bypass);
        aeq_printf("seg_num %d\n", tab->seg_num);
        for (int i = 0; i < tab->seg_num; i++) {
            aeq_printf("index %d\n", tab->seg[i].index);
            aeq_printf("iir_type %d\n", tab->seg[i].iir_type);
            aeq_printf("freq %d\n", tab->seg[i].freq);
            aeq_printf("gain %d/100\n", (int)(tab->seg[i].gain * 100.f));
            aeq_printf("q %d/100\n", (int)(tab->seg[i].q * 100.f));
        }
#endif
        //读取EQ格式转换
        eq_ref = malloc(sizeof(struct eq_default_seg_tab));
        int seg_size = sizeof(struct eq_seg_info) * tab->seg_num;
        eq_ref->seg = malloc(seg_size);
        eq_ref->global_gain = tab->global_gain;
        eq_ref->seg_num = tab->seg_num;
        memcpy(eq_ref->seg, tab->seg, seg_size);

        free(tab);
    } else {
        printf("[AEQ]user eq cfg parm read err ret %d\n", ret);
    }

    return eq_ref;
}

//读取自适应AEQ结果 - 用于播歌更新效果
struct eq_default_seg_tab *audio_icsd_adaptive_eq_read(void)
{
    struct eq_default_seg_tab *eq_tab;
    if (aeq_hdl) {	//自适应模式且没有训练中，才允许读取
        if (aeq_hdl->eff_mode == AEQ_EFF_MODE_ADAPTIVE && \
            (!audio_adaptive_eq_is_running())) {
            s16 volume = app_audio_get_volume(APP_AUDIO_STATE_MUSIC);
            eq_tab = audio_adaptive_eq_cur_list_query(volume);
            aeq_printf("%s\n", __func__);
#if 0
            aeq_printf("global_gain %d/100\n", (int)(eq_tab->global_gain * 100.f));
            aeq_printf("seg_num %d\n", eq_tab->seg_num);
            for (int i = 0; i < eq_tab->seg_num; i++) {
                aeq_printf("index %d\n", eq_tab->seg[i].index);
                aeq_printf("iir_type %d\n", eq_tab->seg[i].iir_type);
                aeq_printf("freq %d\n", eq_tab->seg[i].freq);
                aeq_printf("gain %d/100\n", (int)(eq_tab->seg[i].gain * 100.f));
                aeq_printf("q %d/100\n", (int)(eq_tab->seg[i].q * 100.f));
            }
#endif
            return eq_tab;
        }
    }
    return NULL;	//表示使用默认参数
}

static void audio_adaptive_eq_data_packet(_aeq_output *aeq_output, float *sz, float *fgq, int cnt)
{
    int len = AEQ_FLEN / 2;
    int out_seg_num =  AEQ_IIR_NUM_FLEX + AEQ_IIR_NUM_FIX;
    int eq_dat_len =  sizeof(anc_fr_t) * out_seg_num + 4;
    u8 tmp_type[out_seg_num];
    u8 *leq_dat = zalloc(eq_dat_len);
    for (int i = 0; i < out_seg_num; i++) {
        tmp_type[i] = aeq_type[i];
    }

    /* for (int i = 0; i < 120; i++) { */
    /* printf("target:%d\n", (int)aeq_output->target[i]); */
    /* } */
    /* for (int i = 0; i < 60; i++) { */
    /* printf("freq:%d\n", (int)aeq_output->h_freq[i]); */
    /* } */
    audio_anc_fr_format(leq_dat, fgq, out_seg_num, tmp_type);
    if (cnt == 0) {
        adaptive_eq_data = anc_data_catch(adaptive_eq_data, NULL, 0, 0, 1);
#if TCFG_USER_TWS_ENABLE
        if (bt_tws_get_local_channel() == 'R') {
            r_printf("ANC ear-adaptive send data, ch:R\n");
            adaptive_eq_data = anc_data_catch(adaptive_eq_data, (u8 *)aeq_output->h_freq, len * 4, ANC_R_ADAP_FRE, 0);
            adaptive_eq_data = anc_data_catch(adaptive_eq_data, (u8 *)sz, len * 8, ANC_R_ADAP_SZPZ, 0);
            adaptive_eq_data = anc_data_catch(adaptive_eq_data, (u8 *)aeq_output->target, len * 8, ANC_R_ADAP_TARGET, 0);
            adaptive_eq_data = anc_data_catch(adaptive_eq_data, (u8 *)leq_dat, eq_dat_len, ANC_R_FF_IIR, 0);  //R_ff
        } else
#endif
        {
            r_printf("ANC ear-adaptive send data, ch:L\n");
            adaptive_eq_data = anc_data_catch(adaptive_eq_data, (u8 *)aeq_output->h_freq, len * 4, ANC_L_ADAP_FRE, 0);
            adaptive_eq_data = anc_data_catch(adaptive_eq_data, (u8 *)sz, len * 8, ANC_L_ADAP_SZPZ, 0);
            adaptive_eq_data = anc_data_catch(adaptive_eq_data, (u8 *)aeq_output->target, len * 8, ANC_L_ADAP_TARGET, 0);
            adaptive_eq_data = anc_data_catch(adaptive_eq_data, (u8 *)leq_dat, eq_dat_len, ANC_L_FF_IIR, 0);  //R_ff
        }
    } else if (cnt == 1) {
#if TCFG_USER_TWS_ENABLE
        if (bt_tws_get_local_channel() == 'R') {
            r_printf("ANC ear-adaptive send data, ch:R\n");
            adaptive_eq_data = anc_data_catch(adaptive_eq_data, (u8 *)leq_dat, eq_dat_len, ANC_R_FB_IIR, 0);  //R_ff
        } else
#endif
        {
            r_printf("ANC ear-adaptive send data, ch:L\n");
            adaptive_eq_data = anc_data_catch(adaptive_eq_data, (u8 *)leq_dat, eq_dat_len, ANC_L_FB_IIR, 0);  //R_ff
        }
    } else if (cnt == 2) {
#if TCFG_USER_TWS_ENABLE
        if (bt_tws_get_local_channel() == 'R') {
            r_printf("ANC ear-adaptive send data, ch:R\n");
            adaptive_eq_data = anc_data_catch(adaptive_eq_data, (u8 *)leq_dat, eq_dat_len, ANC_R_CMP_IIR, 0);  //R_ff
        } else
#endif
        {
            r_printf("ANC ear-adaptive send data, ch:L\n");
            adaptive_eq_data = anc_data_catch(adaptive_eq_data, (u8 *)leq_dat, eq_dat_len, ANC_L_CMP_IIR, 0);  //R_ff
        }
    }

    free(leq_dat);
}

static int audio_adaptive_eq_cur_list_add(s16 volume, struct eq_default_seg_tab *eq_output)
{
    struct audio_aeq_bulk *bulk = zalloc(sizeof(struct audio_aeq_bulk));
    struct audio_aeq_bulk *tmp;
    spin_lock(&aeq_hdl->lock);
    list_for_each_entry(tmp, &aeq_hdl->head, entry) {
        if (tmp->volume == volume) {
            free(bulk);
            return -1;
        }
    }
    bulk->volume = volume;
    bulk->eq_output = eq_output;
    list_add_tail(&bulk->entry, &aeq_hdl->head);
    spin_unlock(&aeq_hdl->lock);
    return 0;
}

static int audio_adaptive_eq_cur_list_del(void)
{
    struct audio_aeq_bulk *bulk;
    struct audio_aeq_bulk *temp;
    spin_lock(&aeq_hdl->lock);
    list_for_each_entry_safe(bulk, temp, &aeq_hdl->head, entry) {
        free(bulk->eq_output->seg);
        free(bulk->eq_output);
        __list_del_entry(&(bulk->entry));
        free(bulk);
    }
    spin_unlock(&aeq_hdl->lock);
    return 0;
}

static struct eq_default_seg_tab *audio_adaptive_eq_cur_list_query(s16 volume)
{
    struct audio_aeq_bulk *bulk;

#if	ADAPTIVE_EQ_VOLUME_GRADE_EN
    int vol_list_num = sizeof(aeq_volume_grade_list);

    for (u8 i = 0; i < vol_list_num; i++) {
        if (volume <= aeq_volume_grade_list[i]) {
            volume = aeq_volume_grade_list[i];
            break;
        }
    }
#else
    volume = 0;
#endif

    spin_lock(&aeq_hdl->lock);
    list_for_each_entry(bulk, &aeq_hdl->head, entry) {
        if (bulk->volume == volume) {
            spin_unlock(&aeq_hdl->lock);
            return bulk->eq_output;
        }
    }
    spin_unlock(&aeq_hdl->lock);
    //读不到则返回默认参数
    return audio_icsd_eq_default_tab_read();
}

static int audio_adaptive_eq_start(void)
{
    //初始化算法，申请资源
    struct icsd_aeq_libfmt libfmt;	//获取算法库需要的参数
    struct icsd_aeq_infmt infmt;	//输入算法库的参数

    icsd_aeq_get_libfmt(&libfmt);
    aeq_printf("AEQ RAM SIZE:%d\n", libfmt.lib_alloc_size);
    aeq_hdl->lib_alloc_ptr = zalloc(libfmt.lib_alloc_size);
    if (!aeq_hdl->lib_alloc_ptr) {
        return -1;
    }
    infmt.alloc_ptr = aeq_hdl->lib_alloc_ptr;
    infmt.seg_num = AEG_SEG_NUM;
    icsd_aeq_set_infmt(&infmt);
    return 0;

}

static int audio_adaptive_eq_stop(void)
{
    free(aeq_hdl->lib_alloc_ptr);
    aeq_hdl->lib_alloc_ptr = NULL;
    return 0;
}

static struct eq_default_seg_tab *audio_adaptive_eq_run(float maxgain_dB, int cnt)
{
    struct eq_default_seg_tab *eq_cur_tmp;
    float *sz_cur = aeq_hdl->fre_out->sz_l.out;

    aeq_hdl->sz_ref = (float *)malloc(AEQ_FLEN * sizeof(float));
    memcpy((u8 *)aeq_hdl->sz_ref, sz_ref_test, AEQ_FLEN * sizeof(float));
    aeq_hdl->sz_dut_cmp = (float *)malloc(AEQ_FLEN * sizeof(float));

    //低频平稳滤波
    for (int i = 0; i < 10; i += 2) {
        aeq_hdl->sz_ref[i] = aeq_hdl->sz_ref[10] * 0.9 ; // i
        aeq_hdl->sz_ref[i + 1] = aeq_hdl->sz_ref[11] * 0.9; //

        sz_cur[i] = (sz_cur[12] + sz_cur[10]) / 2;
        sz_cur[i + 1] = (sz_cur[13] + sz_cur[11]) / 2;
    }

    for (int i = 0; i < AEQ_FLEN; i += 2) {
        //测试代码-目前产测未接入
        aeq_hdl->sz_dut_cmp[i] = 1.0;
        aeq_hdl->sz_dut_cmp[i + 1] = 0;
        /* aeq_hdl->sz_ref[i] = 1.0; */
        /* aeq_hdl->sz_ref[i + 1] = 0; */
        /* sz_in[i  ] = 1.0; */
        /* sz_in[i + 1] = 0; */
    }
    /* put_buf((u8 *)sz_cur, 120 * 4); */

    int output_seg_num = AEQ_IIR_NUM_FLEX + AEQ_IIR_NUM_FIX;
    float *lib_eq_cur = zalloc(((3 * 10) + 1) * sizeof(float));
    //需要足够运行时间
    _aeq_output *aeq_output = icsd_aeq_run(aeq_hdl->sz_ref, sz_cur, \
                                           (void *)aeq_hdl->eq_ref, aeq_hdl->sz_dut_cmp, maxgain_dB, lib_eq_cur);
    if (aeq_output->state) {
        aeq_printf("AEQ OUTPUT ERR!\n");
    }

#if ADPTIVE_EQ_TOOL_BETA_ENABLE
    audio_adaptive_eq_data_packet(aeq_output, sz_cur, lib_eq_cur, cnt);
#endif

    //格式化算法输出
    int seg_size = sizeof(struct eq_seg_info) * output_seg_num;
    eq_cur_tmp = malloc(sizeof(struct eq_default_seg_tab));
    eq_cur_tmp->seg = malloc(seg_size);
    struct eq_seg_info *out_seg = eq_cur_tmp->seg;

    //算法输出时，线性值转dB
    eq_cur_tmp->global_gain = 20 * log10_float(lib_eq_cur[0] < 0 ? (0 - lib_eq_cur[0]) : lib_eq_cur[0]);
    eq_cur_tmp->seg_num = output_seg_num;

    /* aeq_printf("Global %d\n", (int)(lib_eq_cur[0] * 10000)); */
    for (int i = 0; i < output_seg_num; i++) {
        out_seg[i].index = i;
        out_seg[i].iir_type = aeq_type[i];
        out_seg[i].freq = lib_eq_cur[i * 3 + 1];
        out_seg[i].gain = lib_eq_cur[i * 3 + 1 + 1];
        out_seg[i].q 	= lib_eq_cur[i * 3 + 2 + 1];
        /* aeq_printf("SEG[%d]:Type %d, FGQ:%d %d %d\n", i, out_seg[i].iir_type, \ */
        /* (int)(out_seg[i].freq * 10000), (int)(out_seg[i].gain * 10000), \ */
        /* (int)(out_seg[i].q * 10000)); */
    }
    free(lib_eq_cur);

    if (aeq_hdl->sz_dut_cmp) {
        free(aeq_hdl->sz_dut_cmp);
        aeq_hdl->sz_dut_cmp = NULL;
    }
    if (aeq_hdl->sz_ref) {
        free(aeq_hdl->sz_ref);
        aeq_hdl->sz_ref = NULL;
    }
    return eq_cur_tmp;
}

static int audio_adaptive_eq_dot_run(void)
{
#if TCFG_AUDIO_FIT_DET_ENABLE
    float dot_db = audio_icsd_dot_light_open(aeq_hdl->fre_out);
    printf("========================================= \n");
    printf("                    dot db = %d/100 \n", (int)(dot_db * 100.0f));
    printf("========================================= \n");

    int send_data[2];
    send_data[0] = 0x1;	//cmd
    memcpy((u8 *)(send_data + 1), (u8 *)&dot_db, 4);
    audio_anc_debug_send_data((u8 *)send_data, 8);

    if (dot_db > DOT_NORM_THR) {
        printf(" dot: tight ");
        return AUDIO_FIT_DET_RESULT_TIGHT;
    } else if (dot_db > DOT_LOOSE_THR) {
        printf(" dot: norm ");
        return AUDIO_FIT_DET_RESULT_NORMAL;
    } else { // < loose_thr
        printf(" dot: loose ");
        return AUDIO_FIT_DET_RESULT_LOOSE;
    }
#endif
    return 0;
}

static void audio_adaptive_eq_afq_output_hdl(struct audio_afq_output *p)
{
    aeq_hdl->fre_out = p;

    struct eq_default_seg_tab *eq_output;
    float maxgain_dB;
    aeq_printf("AEQ RUN \n");

    //释放上一次AEQ存储空间
    audio_adaptive_eq_cur_list_del();

#if ADAPTIVE_EQ_VOLUME_GRADE_EN
    //根据参数需求循环跑AEQ

#if ADAPTIVE_EQ_TIGHTNESS_GRADE_EN
    int dot_lvl = audio_adaptive_eq_dot_run();
    /* if(dot_lvl == AUDIO_FIT_DET_RESULT_TIGHT) { */
    /* goto __aeq_close; */
    /* } */
#else
    int dot_lvl = 0;
#endif

    int vol_list_num = sizeof(aeq_volume_grade_list);
    for (u8 i = 0; i < vol_list_num; i++) {
        /* maxgain_dB = 0 - audio_adaptive_eq_vol_gain_get(aeq_volume_grade_list[i]); */
        maxgain_dB = aeq_volume_grade_maxdB_table[dot_lvl][i];
        r_printf("max_dB %d/10, lvl %d\n", (int)(maxgain_dB * 10), aeq_volume_grade_list[i]);
        audio_adaptive_eq_start();
        eq_output = audio_adaptive_eq_run(maxgain_dB, i);
        audio_adaptive_eq_cur_list_add(aeq_volume_grade_list[i], eq_output);
        audio_adaptive_eq_stop();
    }
#else
    audio_adaptive_eq_start();
    eq_output = audio_adaptive_eq_run(ADAPTIVE_EQ_MAXGAIN_DB, 0);
    audio_adaptive_eq_cur_list_add(0, eq_output);
    audio_adaptive_eq_stop();

#endif

__aeq_close:
    //关闭AEQ
    audio_adaptive_eq_close();

    aeq_printf("AEQ END\n");
}

int audio_adaptive_eq_eff_set(enum ADAPTIVE_EFF_MODE mode)
{
    if (aeq_hdl) {
        if (audio_adaptive_eq_is_running()) {
            return -1;
        }
        aeq_printf("AEQ EFF mode set %d\n", mode);
        struct eq_default_seg_tab *eq_tab;
        aeq_hdl->eff_mode = mode;
        switch (mode) {
        case AEQ_EFF_MODE_DEFAULT:	//默认效果
            eq_tab = audio_icsd_eq_default_tab_read();
            audio_icsd_eq_eff_update(eq_tab);
            if (eq_tab) {
                free(eq_tab->seg);
                free(eq_tab);
            }
            break;
        case AEQ_EFF_MODE_ADAPTIVE:	//自适应效果
            s16 volume = app_audio_get_volume(APP_AUDIO_STATE_MUSIC);
            audio_adaptive_eq_vol_update(volume);
            break;
        }
    }
    return 0;
}

//根据音量获取对应AEQ效果，并更新
int audio_adaptive_eq_vol_update(s16 volume)
{
    if (audio_adaptive_eq_is_running()) {
        aeq_printf("[AEQ_WARN] vol update fail, AEQ is running\n");
        return -1;
    }
    if (aeq_hdl->eff_mode == AEQ_EFF_MODE_DEFAULT) {
        aeq_printf("[AEQ] vol no need update, AEQ mode is default\n");
        return 0;
    }
    aeq_hdl->now_volume = volume;
    audio_icsd_eq_eff_update(audio_adaptive_eq_cur_list_query(volume));
    return 0;
}

static float audio_adaptive_eq_vol_gain_get(s16 volume)
{
    struct cfg_info info  = {0};             //节点配置相关信息（参数存储的目标地址、配置项大小）
    char mode_index       = 0;               //模式序号（当前节点无多参数组，mode_index是0）
    char *node_name       = ADAPTIVE_EQ_VOLUME_NODE_NAME;       //节点名称（节点内的第一参数，用户自定义,长度小于等于15byte）
    char cfg_index        = 0;               //目标配置项序号（当前节点无多参数组，cfg_index是0）
    int ret = jlstream_read_form_node_info_base(mode_index, node_name, cfg_index, &info);
    if (!ret) {
        struct volume_cfg *vol_cfg = zalloc(info.size);
        if (!jlstream_read_form_cfg_data(&info, (void *)vol_cfg)) {
            printf("[AEQ]user vol cfg parm read err\n");
            free(vol_cfg);
            return 0;
        }
        /* printf("cfg_level_max = %d\n", vol_cfg->cfg_level_max); */
        /* printf("cfg_vol_min = %d\n", vol_cfg->cfg_vol_min); */
        /* printf("cfg_vol_max = %d\n", vol_cfg->cfg_vol_max); */
        /* printf("vol_table_custom = %d\n", vol_cfg->vol_table_custom); */
        /* printf("cur_vol = %d\n", vol_cfg->cur_vol); */
        /* printf("tab_len = %d\n", vol_cfg->tab_len); */

        if (info.size > sizeof(struct volume_cfg)) { //有自定义音量表, 直接返回对应音量
            return vol_cfg->vol_table[volume];
        } else {
            u16 step = (vol_cfg->cfg_level_max - 1 > 0) ? (vol_cfg->cfg_level_max - 1) : 1;
            float step_db = (vol_cfg->cfg_vol_max - vol_cfg->cfg_vol_min) / (float)step;
            float vol_db = vol_cfg->cfg_vol_min + (volume - 1) * step_db;
            return vol_db;
        }
    } else {
        printf("[AEQ]user vol cfg parm read err ret %d\n", ret);
    }
    return 0;
}

#endif/*TCFG_AUDIO_ADAPTIVE_EQ_ENABLE*/
