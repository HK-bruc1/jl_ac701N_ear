#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".icsd_cmp.data.bss")
#pragma data_seg(".icsd_cmp.data")
#pragma const_seg(".icsd_cmp.text.const")
#pragma code_seg(".icsd_cmp.text")
#endif

#include "app_config.h"
#include "audio_config_def.h"
#if (TCFG_AUDIO_ANC_EAR_ADAPTIVE_EN && \
	 TCFG_AUDIO_ANC_ENABLE && \
	 TCFG_AUDIO_ANC_EAR_ADAPTIVE_VERSION == ANC_EXT_V2)

#include "audio_anc.h"

#if ANC_EAR_ADAPTIVE_CMP_EN

#include "icsd_cmp.h"
#include "icsd_cmp_app.h"
#include "icsd_common_v2_app.h"

struct icsd_cmp_hdl {
    void *lib_alloc_ptr;
    _cmp_output *l_output;
    _cmp_output *r_output;
};

struct icsd_cmp_hdl *cmp_hdl = NULL;

static int audio_anc_ear_adaptive_cmp_close(void);

void icsd_cmp_force_exit()
{
    if (cmp_hdl) {
        DeAlorithm_disable();
    }
}

void audio_anc_ear_adaptive_cmp_run(__afq_output *p)
{
    printf("==============icsd cmp run===============\n");
    u8 l_state = 0, r_state = 0;
    s8 sz_sign = anc_ext_ear_adaptive_sz_calr_sign_get();
    printf("sz_sign %d\n", sz_sign);
    if (p->sz_l) {
        /* u32 last = jiffies_usec(); */
        if (sz_sign == -1) {
            for (int j = 0; j < p->sz_l->len; j++) {
                p->sz_l->out[j] = 0 -  p->sz_l->out[j];
            }
        }
        cmp_hdl->l_output = icsd_cmp_run(p->sz_l->out);
        l_state = cmp_hdl->l_output->state;
        //CMP增益符号与SZ符号 负相关
        cmp_hdl->l_output->fgq[0] = (abs(cmp_hdl->l_output->fgq[0])) * (sz_sign) * (-1);
        /* g_printf("cmp use time %d \n",(int)(jiffies_usec() - last)); */
#if 0
        float *fgq = cmp_hdl->l_output->fgq;
        g_printf("Global %d/10000\n", (int)(fgq[0] * 10000));
        for (int i = 0; i < ANC_ADAPTIVE_CMP_ORDER; i++) {
            g_printf("SEG[%d]:Type %d, FGQ/10000:%d %d %d\n", i, icsd_cmp_type[i], \
                     (int)(fgq[i * 3 + 1] * 10000), (int)(fgq[i * 3 + 2] * 10000), \
                     (int)(fgq[i * 3 + 3] * 10000));
        }
#endif
    }
    if (p->sz_r) {
        if (sz_sign == -1) {
            for (int j = 0; j < p->sz_r->len; j++) {
                p->sz_r->out[j] = 0 -  p->sz_r->out[j];
            }
        }
        cmp_hdl->r_output = icsd_cmp_run(p->sz_r->out);
        r_state = cmp_hdl->r_output->state;
        cmp_hdl->r_output->fgq[0] = (abs(cmp_hdl->r_output->fgq[0])) * (sz_sign) * (-1);
    }
    if (l_state || r_state) {
        printf("CMP OUTPUT ERR!! STATE : L %d, R %d\n", l_state, r_state);
    }
}

int audio_anc_ear_adaptive_cmp_open(void)
{
    if (cmp_hdl) {
        return -1;
    }
    cmp_hdl = zalloc(sizeof(struct icsd_cmp_hdl));
    enum audio_adaptive_fre_sel fre_sel = AUDIO_ADAPTIVE_FRE_SEL_ANC;

    struct icsd_cmp_libfmt libfmt;
    struct icsd_cmp_infmt  fmt;
    icsd_cmp_get_libfmt(&libfmt);
    printf("cmp RAM SIZE:%d %d %d\n", libfmt.lib_alloc_size, cmp_IIR_NUM_FIX, cmp_IIR_NUM_FLEX);
    cmp_hdl->lib_alloc_ptr = fmt.alloc_ptr = zalloc(libfmt.lib_alloc_size);
    icsd_cmp_set_infmt(&fmt);
    return 0;
}

int audio_anc_ear_adaptive_cmp_close(void)
{
    if (cmp_hdl) {
        free(cmp_hdl->lib_alloc_ptr);
        free(cmp_hdl);
        cmp_hdl = NULL;
    }
    return 0;
}

struct icsd_cmp_hdl *audio_anc_ear_adaptive_cmp_hdl_get(void)
{
    return cmp_hdl;
}

float *audio_anc_ear_adaptive_cmp_output_get(enum ANC_EAR_ADAPTIVE_CMP_CH ch)
{
    if (cmp_hdl) {
        switch (ch) {
        case ANC_EAR_ADAPTIVE_CMP_CH_L:
            if (cmp_hdl->l_output) {
                return cmp_hdl->l_output->fgq;
            }
            break;
        case ANC_EAR_ADAPTIVE_CMP_CH_R:
            if (cmp_hdl->r_output) {
                return cmp_hdl->r_output->fgq;
            }
            break;
        }
    }
    return NULL;
}

u8 audio_anc_ear_adaptive_cmp_result_get(void)
{
    u8 result = 0;
    if (cmp_hdl) {
#if ANC_CONFIG_LFB_EN
        if (!cmp_hdl->l_output->state) {
            result |= ANC_ADAPTIVE_RESULT_LCMP;
        }
#endif
#if ANC_CONFIG_RFB_EN
        if (!cmp_hdl->r_output->state) {
            result |= ANC_ADAPTIVE_RESULT_RCMP;
        }
#endif
    }
    return result;
}

#endif
#endif


