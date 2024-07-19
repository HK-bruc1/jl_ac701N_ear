#include "audio_anc.h"
#include "app_tone.h"
#include "classic/tws_api.h"

#if TCFG_AUDIO_ANC_ENABLE && ANC_REAL_TIME_ADAPTIVE_ENABLE
#include "rt_anc.h"

struct rt_anc_function	RT_ANC_FUNC;
void *rt_anc_ram_addr = 0;
u8 rtanc_open_flag;

int (*rt_printf)(const char *format, ...) = _rt_printf;
int rt_printf_off(const char *format, ...)
{
    return 0;
}

void icsd_rtanc_send();


#define TWS_FUNC_ID_RTANC_M2S    TWS_FUNC_ID('I', 'C', 'M', 'S')
REGISTER_TWS_FUNC_STUB(icsd_rtanc_m2s) = {
    .func_id = TWS_FUNC_ID_RTANC_M2S,
    .func    = icsd_rtanc_m2s_cb,
};

#define TWS_FUNC_ID_RTANC_S2M    TWS_FUNC_ID('I', 'C', 'S', 'M')
REGISTER_TWS_FUNC_STUB(icsd_rtanc_s2m) = {
    .func_id = TWS_FUNC_ID_RTANC_S2M,
    .func    = icsd_rtanc_s2m_cb,
};

void icsd_rtanc_tws_m2s(u8 cmd)
{
    printf("icsd_rtanc_tws_m2s\n");
    struct rt_anc_tws_packet packet;
    rt_anc_master_packet(&packet, cmd);
    int ret = tws_api_send_data_to_sibling(packet.data, packet.len, TWS_FUNC_ID_RTANC_M2S);
}

void icsd_rtanc_tws_s2m(u8 cmd)
{
    /*
    s8 data[16];
    s_tig = tig;
    data[0] = s_tig;
    int ret = tws_api_send_data_to_sibling(data, 16, TWS_FUNC_ID_RTANC_S2M);
    */
    struct rt_anc_tws_packet packet;
    rt_anc_slave_packet(&packet, cmd);
    int ret = tws_api_send_data_to_sibling(packet.data, packet.len, TWS_FUNC_ID_RTANC_S2M);
}

void icsd_rtanc_send()
{
    u32 tws_state = tws_api_get_tws_state();
    u32 role = tws_api_get_role();
    if (tws_state & TWS_STA_SIBLING_CONNECTED) {
        if (role == 0) { //master
            //extern void tws_tx_unsniff_req(void);
            //tws_tx_unsniff_req();
            icsd_rtanc_tws_m2s(M_CMD_TEST);
        } else {
            icsd_rtanc_tws_s2m(S_CMD_TEST);
        }
    }

}

void icsd_rtanc_master_send(u8 cmd)
{
    u32 tws_state = tws_api_get_tws_state();
    u32 role = tws_api_get_role();
    if (tws_state & TWS_STA_SIBLING_CONNECTED) {
        if (role == 0) { //master
            icsd_rtanc_tws_m2s(cmd);
        }
    }
}

void icsd_rtanc_slave_send(u8 cmd)
{
    u32 tws_state = tws_api_get_tws_state();
    u32 role = tws_api_get_role();
    if (tws_state & TWS_STA_SIBLING_CONNECTED) {
        if (role == 0) { //master
        } else {
            icsd_rtanc_tws_s2m(cmd);
        }
    }
}

void icsd_rt_anc_on(void *param)
{
    return;
    printf("icsd_rt_anc_on-0:%d-----------------------\n", (int)param);
    static u8 rt_open = 0;
    if (rt_open == 0) {
        extern void rt_anc_open(void *param);
        rt_anc_open(param);
        rt_open = 1;
    }

    rtanc_open_flag = 1;
    //play_tone_file(get_tone_files()->num[2]);
    //
    printf("Raymond key>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    extern void rt_anc_pzl_dma_on();
    rt_anc_pzl_dma_on();

}


void rt_anc_function_init()
{
    RT_ANC_FUNC.anc_dma_on = anc_dma_on;
    RT_ANC_FUNC.anc_dma_on_double = anc_dma_on_double;
    RT_ANC_FUNC.anc_dma_done_ppflag = anc_dma_done_ppflag;
    RT_ANC_FUNC.anc_core_dma_ie = anc_dma_ie;
    RT_ANC_FUNC.anc_core_dma_stop = anc_dma_stop;
}

void rt_anc_post_anctask_cmd(u8 cmd)
{
    os_taskq_post_msg("anc", 2, ANC_MSG_RT_ANC_CMD, cmd);
}

void rt_anc_post_rtanctask_cmd(u8 cmd)
{
    os_taskq_post_msg("rt_anc", 1, cmd);
}

void rt_anc_open(void *param)
{
    printf("=================rt_anc_open==================\n");
    rt_anc_function_init();
    struct rt_anc_libfmt libfmt;
    rt_anc_get_libfmt(&libfmt);
    if (rt_anc_ram_addr == 0) {
        rt_anc_ram_addr = zalloc(libfmt.lib_alloc_size);
    }
    struct rt_anc_infmt infmt;
    infmt.alloc_ptr = rt_anc_ram_addr;
    infmt.param = param;
    rt_anc_set_infmt(&infmt);
}

void rt_anc_close()
{
    printf("================rt_anc_close==================\n");
    rt_anc_exit();
    if (rt_anc_ram_addr) {
        free(rt_anc_ram_addr);
        rt_anc_ram_addr = 0;
    }
}

void rt_anc_get_param(void *_param, void *rt_param_l, void *rt_param_r)
{
    audio_anc_t *param = (audio_anc_t *)_param;
    __rt_anc_param *anc_param = (__rt_anc_param *)rt_param_l;
    anc_param->ff_yorder = param->lff_yorder;
    anc_param->fb_yorder = param->lfb_yorder;
    anc_param->ffgain = param->gains.l_ffgain;
    anc_param->fbgain = param->gains.l_fbgain;
    for (int i = 0; i < anc_param->ff_yorder * 5; i++) {
        anc_param->ff_coeff[i] = param->lff_coeff[i];
    }
    for (int i = 0; i < anc_param->fb_yorder * 5; i++) {
        anc_param->fb_coeff[i] = param->lfb_coeff[i];
    }
    anc_param->param = _param;

    if (rt_param_r) {
        anc_param = (__rt_anc_param *)rt_param_r;
        anc_param->ff_yorder = param->rff_yorder;
        anc_param->fb_yorder = param->rfb_yorder;
        anc_param->ffgain = param->gains.r_ffgain;
        anc_param->fbgain = param->gains.r_fbgain;
        for (int i = 0; i < anc_param->ff_yorder * 5; i++) {
            anc_param->ff_coeff[i] = param->rff_coeff[i];
        }
        for (int i = 0; i < anc_param->fb_yorder * 5; i++) {
            anc_param->fb_coeff[i] = param->rfb_coeff[i];
        }
        anc_param->param = _param;

    }

}

void rt_anc_updata_param(void *rt_param_l, void *rt_param_r)
{
    __rt_anc_param *anc_param = (__rt_anc_param *)rt_param_l;
    audio_anc_t *param = (audio_anc_t *)anc_param->param;
    param->gains.l_ffgain = anc_param->ffgain;
    param->lff_coeff = &anc_param->ff_coeff[0];
    param->gains.l_fbgain = anc_param->fbgain;
    //param->lfb_coeff = &anc_param->lfb_coeff[0];
    if (rt_param_r) {
        anc_param = (__rt_anc_param *)rt_param_r;
        audio_anc_t *param = (audio_anc_t *)anc_param->param;
        param->gains.r_ffgain = anc_param->ffgain;
        param->rff_coeff = &anc_param->ff_coeff[0];
        param->gains.r_fbgain = anc_param->fbgain;
    }
    anc_coeff_online_ff_update(param, 1);
}

void rt_anc_dma_2ch_on(u8 out_sel, int *buf, int len)
{
    printf("dma 2ch on:%d %x %d\n", out_sel, (int)buf, len);
    anc_dma_on_double(out_sel, buf, len);
}

void rt_anc_dma_4ch_on(u8 out_sel_ch01, u8 out_sel_ch23, int *buf, int len)
{
    printf("dma 4ch on:%d %d %x %d\n", out_sel_ch01, out_sel_ch23, (int)buf, len);
    anc_dma_on_double_4ch(out_sel_ch01, out_sel_ch23, buf, len);
}


#if RT_ANC_DSF8_DATA_DEBUG
s16 DSF8_DEBUG_H[RT_ANC_DMA_DOUBLE_LEN / 8 * RT_ANC_DMA_DOUBLE_CNT];
s16 DSF8_DEBUG_L[RT_ANC_DMA_DOUBLE_LEN / 8 * RT_ANC_DMA_DOUBLE_CNT];
s16 DSF8_R_DEBUG_H[RT_ANC_DMA_DOUBLE_LEN / 8 * RT_ANC_DMA_DOUBLE_CNT];
s16 DSF8_R_DEBUG_L[RT_ANC_DMA_DOUBLE_LEN / 8 * RT_ANC_DMA_DOUBLE_CNT];
static int dsf8_wptr = 0;
void rt_anc_dsf8_data_2ch(s16 *dsf_out_h, s16 *dsf_out_l)
{
    printf("rt anc 2ch dsf8 save\n");
    s16 *wptr_h = &DSF8_DEBUG_H[dsf8_wptr];
    s16 *wptr_l = &DSF8_DEBUG_L[dsf8_wptr];
    memcpy(wptr_h, dsf_out_h, RT_ANC_DMA_DOUBLE_LEN / 8 * 2);
    memcpy(wptr_l, dsf_out_l, RT_ANC_DMA_DOUBLE_LEN / 8 * 2);
    dsf8_wptr += RT_ANC_DMA_DOUBLE_LEN / 8;
    if (dsf8_wptr >= (RT_ANC_DMA_DOUBLE_LEN / 8 * RT_ANC_DMA_DOUBLE_CNT)) {
        local_irq_disable();
        for (int i = 0; i < RT_ANC_DMA_DOUBLE_LEN / 8 * RT_ANC_DMA_DOUBLE_CNT; i++) {
            printf("DSF8:H/L:%d                            %d\n", DSF8_DEBUG_H[i], DSF8_DEBUG_L[i]);
        }
    }
}
void rt_anc_dsf8_data_4ch(s16 *dsf_out_h, s16 *dsf_out_l, s16 *dsf_out_h_r, s16 *dsf_out_l_r)
{
    printf("rt anc 4ch dsf8 save\n");
    s16 *wptr_h = &DSF8_DEBUG_H[dsf8_wptr];
    s16 *wptr_l = &DSF8_DEBUG_L[dsf8_wptr];
    s16 *wptr_h_r = &DSF8_R_DEBUG_H[dsf8_wptr];
    s16 *wptr_l_r = &DSF8_R_DEBUG_L[dsf8_wptr];
    memcpy(wptr_h, dsf_out_h, RT_ANC_DMA_DOUBLE_LEN / 8 * 2);
    memcpy(wptr_l, dsf_out_l, RT_ANC_DMA_DOUBLE_LEN / 8 * 2);
    memcpy(wptr_h_r, dsf_out_h_r, RT_ANC_DMA_DOUBLE_LEN / 8 * 2);
    memcpy(wptr_l_r, dsf_out_l_r, RT_ANC_DMA_DOUBLE_LEN / 8 * 2);
    dsf8_wptr += RT_ANC_DMA_DOUBLE_LEN / 8;
    if (dsf8_wptr >= (RT_ANC_DMA_DOUBLE_LEN / 8 * RT_ANC_DMA_DOUBLE_CNT)) {
        local_irq_disable();
        for (int i = 0; i < RT_ANC_DMA_DOUBLE_LEN / 8 * RT_ANC_DMA_DOUBLE_CNT; i++) {
            printf("4CH DSF8:%d                      %d                       %d                     %d\n", DSF8_DEBUG_H[i], DSF8_DEBUG_L[i], DSF8_R_DEBUG_H[i], DSF8_R_DEBUG_L[i]);
        }
    }
}
#endif

void rt_anc_dsf8_data_debug(u8 belong, s16 *dsf_out_ch0, s16 *dsf_out_ch1, s16 *dsf_out_ch2, s16 *dsf_out_ch3)
{
#if RT_ANC_DSF8_DATA_DEBUG
    if (belong == RT_DSF8_DATA_DEBUG_TYPE) {
        extern const u8 rt_anc_dma_ch_num;
        if (rt_anc_dma_ch_num == 2) {
            rt_anc_dsf8_data_2ch(dsf_out_ch0, dsf_out_ch1);
        } else {
            rt_anc_dsf8_data_4ch(dsf_out_ch0, dsf_out_ch1, dsf_out_ch2, dsf_out_ch3);
        }
    }
#endif
}

#endif
