#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".audio_anc_common.data.bss")
#pragma data_seg(".audio_anc_common.data")
#pragma const_seg(".audio_anc_common.text.const")
#pragma code_seg(".audio_anc_common.text")
#endif

/*
 ****************************************************************
 *							AUDIO ANC COMMON
 * File  : audio_anc_comon_plug.c
 * By    :
 * Notes : 存放ANC共用流程
 *
 ****************************************************************
 */

#include "app_config.h"

#if TCFG_AUDIO_ANC_ENABLE
#include "audio_anc.h"
#include "audio_anc_common_plug.h"
#include "fs/resfile.h"

#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
#include "icsd_adt.h"
#include "icsd_adt_app.h"
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/

#if 1
#define anc_log	printf
#else
#define anc_log (...)
#endif


struct anc_common_t {
    u8 production_mode;
};

static struct anc_common_t common_hdl;
#define __this  (&common_hdl)

//ANC进入产测模式
int audio_anc_production_enter(void)
{
    if (__this->production_mode == 0) {
        anc_log("ANC in Production mode enter\n");
        __this->production_mode = 1;
        //挂起产测互斥功能

#if ANC_EAR_ADAPTIVE_EN
        //耳道自适应将ANC参数修改为默认参数
        if (audio_anc_coeff_mode_get()) {
            audio_anc_coeff_adaptive_set(0, 0);
        }
#endif

#if ANC_ADAPTIVE_EN
        //关闭场景自适应功能
        audio_anc_power_adaptive_suspend();
#endif

#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
        //关闭风噪检测、智能免摘、广域点击
        if (audio_icsd_adt_is_running()) {
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

#if ANC_MUSIC_DYNAMIC_GAIN_EN
        //关闭ANC防破音
        audio_anc_music_dynamic_gain_suspend();
#endif
        //关闭啸叫抑制、DRC、自适应DCC等功能；
    }
    return 0;
}

//ANC退出产测模式
int audio_anc_production_exit(void)
{
    if (__this->production_mode == 1) {
        anc_log("ANC in Production mode exit\n");
        __this->production_mode = 0;
        //恢复产测互斥功能
    }
    return 0;
}

//ANC配置文件 使用RES文件区 覆盖ANC_IF预留区的功能
#if ANC_COEFF_OTA_UPDATE_ENABLE

#define ANC_EXT_HEAD_LEN		20
#define CFG_ANC_CFG_GAINS_FILE		FLASH_RES_PATH"anc_gains.bin"
#define CFG_ANC_CFG_COEFF_FILE		FLASH_RES_PATH"anc_coeff.bin"

#define GROUP_TYPE_ANC_CFG_GAIN		0x9881
#define GROUP_TYPE_ANC_CFG_COEFF	0x98C1

//anc_ext.bin 文件数据头
struct anc_cfg_rsfile_head {
    u32 total_len;	//4 后面所有数据加起来长度
    u16 group_crc;	//6 group_type开始做的CRC，更新数据后需要对应更新CRC
    u16 group_type;	//8
    u16 group_len;  //10 header开始到末尾的长度
    char header[10];//20
};

//获取目标文件和长度
u8 *anc_config_rsfile_get(u8 id, u32 *file_len)
{
    void *fp = NULL;
    int ret = 0;
    u8 *file;
    struct anc_cfg_rsfile_head *f_head;
    u16 target_type;
    if (id == ANC_DB_COEFF) {
        fp = resfile_open(CFG_ANC_CFG_COEFF_FILE);
        target_type = GROUP_TYPE_ANC_CFG_COEFF;
    } else {
        fp = resfile_open(CFG_ANC_CFG_GAINS_FILE);
        target_type = GROUP_TYPE_ANC_CFG_GAIN;
    }

    if (!fp) {
        anc_log("[anc_cfg.bin, id = %d], read_faild\n", id);
        return NULL;
    }
    struct resfile_attrs attr;
    resfile_get_attrs(fp, &attr);
    /* anc_log("faddr:%x,fsize:%d\n", attr.sclust, attr.fsize); */
    if (attr.sclust % 4 != 0) {
        /* ASSERT(0, "ERR!!ANC EXT NO ALIGN 4 BYTE\n"); */
        anc_log("ERR!!ANC CFG NO ALIGN 4 BYTE\n");
        ret = 1;
        goto __exit;
    }
    file = (u8 *)attr.sclust;
    /* put_buf(file, attr.fsize); */

    f_head = (struct anc_cfg_rsfile_head *)file;

    u32 check_total_len = f_head->total_len;
    if ((check_total_len == 0xFFFFFFFF) || \
        (check_total_len == 0) || \
        (f_head->group_type != target_type) || \
        (f_head->group_len == 10)) {
        anc_log("f_head head err,total_len:%u,group_type:%x,group_len:%u\n", check_total_len, f_head->group_type, f_head->group_len);
        ret = 1;
        goto __exit;
    }

    /* anc_log("total_len:0x%x=%u ", f_head->total_len, f_head->total_len); */
    /* anc_log("group_crc:0x%x ", f_head->group_crc); */
    /* anc_log("group_type:0x%x ", f_head->group_type); */
    /* anc_log("group_len:0x%x=%d ", f_head->group_len, f_head->group_len); */
    /* anc_log("tag:%s\n", f_head->header); */

    *file_len = attr.fsize - ANC_EXT_HEAD_LEN;

__exit:
    if (ret) {
        anc_log("anc_ext file get failed\n");
        return NULL;
    }
    anc_log("f_head get succ\n");
    resfile_close(fp);
    return (file + ANC_EXT_HEAD_LEN);
}

/*
    对比 资源文件 与 ANC_IF区域 的ANC配置文件，若差异，以资源文件覆盖ANC区域
    查询耗时：2.3ms;  查询并覆盖耗时：41.8ms
*/
static int anc_config_rsfile_read(void *p)
{
    u32 rs_gain_len, rs_coeff_len;
    int ret = 0;
    u8 *rs_gain = NULL;
    u8 *rs_coeff = NULL;
    u16 rs_crc, db_crc;
    u8 change_flag = 0;
    if (!p) {
        return -1;
    }
    audio_anc_t *param = (audio_anc_t *)p;

    u32 last_jiff = jiffies_usec();
    /*对比ANC增益系数*/
    rs_gain = anc_config_rsfile_get(ANC_DB_GAIN, &rs_gain_len);
    if (rs_gain) {
        rs_crc = CRC16(rs_gain, rs_gain_len);
        anc_gain_t *db_gain = (anc_gain_t *)anc_db_get(ANC_DB_GAIN, &param->gains_size);
        if (db_gain) {
            db_crc = CRC16(db_gain, param->gains_size);
            if (rs_crc != db_crc) {
                change_flag = 1;
                // anc_log("-----------anc db gains------------");
                // put_buf(db_gain, param->gains_size);
                // anc_log("-----------anc rsfile gains------------");
                // put_buf(rs_gain, rs_gain_len);
                anc_log("anc rsfile gains diff, len %d->%d\n", param->gains_size, rs_gain_len);
            }
        }
    } else {
        anc_log("anc rsfile gains read empty!\n");
    }

    /*对比ANC滤波器系数*/
    rs_coeff = anc_config_rsfile_get(ANC_DB_COEFF, &rs_coeff_len);
    if (rs_coeff) {
        rs_crc = CRC16(rs_coeff, rs_coeff_len);
        anc_coeff_t *db_coeff = (anc_coeff_t *)anc_db_get(ANC_DB_COEFF, &param->coeff_size);
        if (db_coeff) {
            db_crc = CRC16(db_coeff, param->coeff_size);
            if (rs_crc != db_crc) {
                change_flag = 1;
                // anc_log("-----------anc db coeff------------");
                // put_buf(db_coeff, param->coeff_size);
                // anc_log("-----------anc rsfile coeff------------");
                // put_buf(rs_coeff, rs_coeff_len);
                anc_log("anc rsfile coeff diff, len %d->%d\n", param->coeff_size, rs_coeff_len);
            }
        }
    } else {
        anc_log("anc rsfile coeff read empty!\n");
    }

    if (change_flag) {
        param->write_coeff_size = rs_coeff_len;
        ret = anc_db_put(param, (anc_gain_t *)rs_gain, (anc_coeff_t *)rs_coeff);
        if (!ret) {
            anc_log("anc rsfile change succ\n");
        } else {
            anc_log("anc rsfile change fail\n");
        }
    }

    anc_log("anc rsfile check time : %d\n", jiffies_usec2offset(last_jiff, jiffies_usec()));

    return ret;
}
#endif



#endif
