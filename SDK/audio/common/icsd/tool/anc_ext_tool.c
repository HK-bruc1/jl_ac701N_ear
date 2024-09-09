
/***********************************************
			ANC EXT 工具交互相关流程
1、耳道自适应

***********************************************/

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".anc_ext.data.bss")
#pragma data_seg(".anc_ext.data")
#pragma const_seg(".anc_ext.text.const")
#pragma code_seg(".anc_ext.text")
#endif
#include "app_config.h"
#if TCFG_AUDIO_ANC_ENABLE
#include "audio_anc_common.h"
#endif
#if TCFG_AUDIO_ANC_EXT_TOOL_ENABLE
#include "anc_ext_tool.h"
#include "anctool.h"
#include "audio_anc.h"
#include "audio_anc_debug_tool.h"
#include "icsd_anc_v2_app.h"
#if TCFG_ANC_BOX_ENABLE
#include "asm/chargestore.h"
#endif

#if 1
#define anc_ext_log	printf
#else
#define anc_ext_log(...)
#endif/*log_en*/

//解析数据打印使能
#define ANC_EXT_CFG_LOG_EN		0

//耳道自适应产测数据最大长度
#define ANC_EXT_EAR_ADAPTIVE_DUT_MAX_LEN	60 * 2 * 2 * 4	//(pz[60]+sz[60]) * channel * sizeof(float)

/*
   ANC_EXT 耳道自适应工具 在线连接 权限说明
   1、支持工具自适应数据读取,会占用3-4K ram
   2、TWS区分左右参数
   3、进入后不支持产测，关机自动退出开发者模式
*/

//命令错误码
enum {
    ERR_NO = 0,
    ERR_NO_ANC_MODE = 1,    // 不支持非ANC模式打开
    ERR_EAR_FAIL = 2,   	// 耳机返回失败
};

//ANC_EXT命令
enum {
    CMD_GET_VERSION     		 = 0x01, //获取版本号
    CMD_TIME_DOMAIN_SHOW_CH_SET	 = 0x02, //时域频响SHOW CH设置
    CMD_BYPASS_TEST              = 0x03, //Bypass_test 模式切换
    CMD_EAR_ADAPTIVE_RUN         = 0x04, //自适应模式切换
    CMD_DEBUG_TOOL				 = 0x05, //Debug 开关
    CMD_ADAPTIVE_TYPE_GET		 = 0x06, //获取自适应类型
    CMD_ADAPTIVE_MODE_GET		 = 0x07, //获取自适应训练模式Debug 开关
    CMD_FUNCTION_ENABLEBIT_GET	 = 0x08, //获取小机已打开的功能(同时表示工具在线标志)
    CMD_EAR_ADAPTIVE_DUT_SAVE	 = 0x09, //耳道自适应产测数据保存VM
    CMD_EAR_ADAPTIVE_SIGN_TRIM	 = 0x0A, //耳道自适应符号校准
    CMD_EAR_ADAPTIVE_DUT_EN_SET	 = 0x0B, //耳道自适应产测使能
    CMD_EAR_ADAPTIVE_DUT_CLEAR	 = 0x0C, //耳道自适应产测数据清0
};

#define EAR_ADAPTIVE_STR_OFFSET(x)		offsetof(struct anc_ext_ear_adaptive_param, x)

//ANC_EXT 耳道自适应 参数ID/存储目标地址对照表
static const struct __anc_ext_subfile_id_table ear_adaptive_id_table[] = {
    //BASE 界面参数 文件ID FILE_ID_ANC_EXT_EAR_ADAPTIVE_BASE
    { 0, ANC_EXT_EAR_ADAPTIVE_BASE_CFG_ID, 			EAR_ADAPTIVE_STR_OFFSET(base_cfg)},

    /*------------------------------头戴式 左声道 / TWS LR-----------------------------*/
    //FF->滤波器设置界面 文件ID FILE_ID_ANC_EXT_EAR_ADAPTIVE_IIR_FF
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_GENERAL_ID, 		EAR_ADAPTIVE_STR_OFFSET(ff_iir_general)},

    { 0, ANC_EXT_EAR_ADAPTIVE_FF_HIGH_GAINS_ID, 	EAR_ADAPTIVE_STR_OFFSET(ff_iir_high_gains)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_MEDIUM_GAINS_ID, 	EAR_ADAPTIVE_STR_OFFSET(ff_iir_medium_gains)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_LOW_GAINS_ID, 		EAR_ADAPTIVE_STR_OFFSET(ff_iir_low_gains)},

    { 0, ANC_EXT_EAR_ADAPTIVE_FF_HIGH_IIR_ID, 		EAR_ADAPTIVE_STR_OFFSET(ff_iir_high)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_MEDIUM_IIR_ID, 	EAR_ADAPTIVE_STR_OFFSET(ff_iir_medium)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_LOW_IIR_ID, 		EAR_ADAPTIVE_STR_OFFSET(ff_iir_low)},

    //FF->权重和性能设置界面 文件ID FILE_ID_ANC_EXT_EAR_ADAPTIVE_FF_WEIGHT
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_WEIGHT_PARAM_ID, 	EAR_ADAPTIVE_STR_OFFSET(ff_weight_param)},

    { 0, ANC_EXT_EAR_ADAPTIVE_FF_HIGH_WEIGHT_ID, 	EAR_ADAPTIVE_STR_OFFSET(ff_weight_high)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_MEDIUM_WEIGHT_ID, 	EAR_ADAPTIVE_STR_OFFSET(ff_weight_medium)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_LOW_WEIGHT_ID, 	EAR_ADAPTIVE_STR_OFFSET(ff_weight_low)},

    { 0, ANC_EXT_EAR_ADAPTIVE_FF_HIGH_MSE_ID, 		EAR_ADAPTIVE_STR_OFFSET(ff_mse_high)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_MEDIUM_MSE_ID, 	EAR_ADAPTIVE_STR_OFFSET(ff_mse_medium)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_LOW_MSE_ID, 		EAR_ADAPTIVE_STR_OFFSET(ff_mse_low)},

    //FF->TARGE挡位设置界面 文件ID FILE_ID_ANC_EXT_EAR_ADAPTIVE_FF_TARGET
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_TARGET_PARAM_ID, 	EAR_ADAPTIVE_STR_OFFSET(ff_target_param)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_TARGET_SV_ID, 		EAR_ADAPTIVE_STR_OFFSET(ff_target_sv)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_TARGET_CMP_ID, 	EAR_ADAPTIVE_STR_OFFSET(ff_target_cmp)},

    //FF->耳道记忆设置界面 文件ID FILE_ID_ANC_EXT_EAR_ADAPTIVE_FF_RECORDER
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_RECORDER_PARAM_ID,	EAR_ADAPTIVE_STR_OFFSET(ff_ear_mem_param)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_RECORDER_PZ_ID, 	EAR_ADAPTIVE_STR_OFFSET(ff_ear_mem_pz)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_RECORDER_SZ_ID, 	EAR_ADAPTIVE_STR_OFFSET(ff_ear_mem_sz)},

    /*------------------------------------头戴式 右声道------------------------------------*/
    //FF->滤波器设置界面 文件ID FILE_ID_ANC_EXT_EAR_ADAPTIVE_IIR_FF
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_R_GENERAL_ID, 		EAR_ADAPTIVE_STR_OFFSET(rff_iir_general)},

    { 0, ANC_EXT_EAR_ADAPTIVE_FF_R_HIGH_GAINS_ID, 	EAR_ADAPTIVE_STR_OFFSET(rff_iir_high_gains)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_R_MEDIUM_GAINS_ID, 	EAR_ADAPTIVE_STR_OFFSET(rff_iir_medium_gains)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_R_LOW_GAINS_ID, 		EAR_ADAPTIVE_STR_OFFSET(rff_iir_low_gains)},

    { 0, ANC_EXT_EAR_ADAPTIVE_FF_R_HIGH_IIR_ID, 		EAR_ADAPTIVE_STR_OFFSET(rff_iir_high)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_R_MEDIUM_IIR_ID, 	EAR_ADAPTIVE_STR_OFFSET(rff_iir_medium)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_R_LOW_IIR_ID, 		EAR_ADAPTIVE_STR_OFFSET(rff_iir_low)},

    //FF->权重和性能设置界面 文件ID FILE_ID_ANC_EXT_EAR_ADAPTIVE_FF_WEIGHT
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_R_WEIGHT_PARAM_ID, 	EAR_ADAPTIVE_STR_OFFSET(rff_weight_param)},

    { 0, ANC_EXT_EAR_ADAPTIVE_FF_R_HIGH_WEIGHT_ID, 	EAR_ADAPTIVE_STR_OFFSET(rff_weight_high)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_R_MEDIUM_WEIGHT_ID, 	EAR_ADAPTIVE_STR_OFFSET(rff_weight_medium)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_R_LOW_WEIGHT_ID, 	EAR_ADAPTIVE_STR_OFFSET(rff_weight_low)},

    { 0, ANC_EXT_EAR_ADAPTIVE_FF_R_HIGH_MSE_ID, 		EAR_ADAPTIVE_STR_OFFSET(rff_mse_high)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_R_MEDIUM_MSE_ID, 	EAR_ADAPTIVE_STR_OFFSET(rff_mse_medium)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_R_LOW_MSE_ID, 		EAR_ADAPTIVE_STR_OFFSET(rff_mse_low)},

    //FF->TARGE挡位设置界面 文件ID FILE_ID_ANC_EXT_EAR_ADAPTIVE_FF_TARGET
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_R_TARGET_PARAM_ID, 	EAR_ADAPTIVE_STR_OFFSET(rff_target_param)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_R_TARGET_SV_ID, 		EAR_ADAPTIVE_STR_OFFSET(rff_target_sv)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_R_TARGET_CMP_ID, 	EAR_ADAPTIVE_STR_OFFSET(rff_target_cmp)},

    //FF->耳道记忆设置界面 文件ID FILE_ID_ANC_EXT_EAR_ADAPTIVE_FF_RECORDER
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_R_RECORDER_PARAM_ID,	EAR_ADAPTIVE_STR_OFFSET(rff_ear_mem_param)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_R_RECORDER_PZ_ID, 	EAR_ADAPTIVE_STR_OFFSET(rff_ear_mem_pz)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_R_RECORDER_SZ_ID, 	EAR_ADAPTIVE_STR_OFFSET(rff_ear_mem_sz)},

    //FF->耳道自适应产测数据
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_DUT_PZ_CMP_ID,	EAR_ADAPTIVE_STR_OFFSET(ff_dut_pz_cmp)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_DUT_SZ_CMP_ID,	EAR_ADAPTIVE_STR_OFFSET(ff_dut_sz_cmp)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_R_DUT_PZ_CMP_ID,	EAR_ADAPTIVE_STR_OFFSET(rff_dut_pz_cmp)},
    { 0, ANC_EXT_EAR_ADAPTIVE_FF_R_DUT_SZ_CMP_ID,	EAR_ADAPTIVE_STR_OFFSET(rff_dut_sz_cmp)},
};


#if ANC_EXT_CFG_LOG_EN

static void anc_file_ear_adaptive_base_cfg_printf(int id, void *buf, int len);
static void anc_file_ear_adaptive_iir_general_printf(int id, void *buf, int len);
static void anc_file_ear_adaptive_iir_gains_printf(int id, void *buf, int len);
static void anc_file_ear_adaptive_iir_printf(int id, void *buf, int len);
static void anc_file_ear_adaptive_weight_param_printf(int id, void *buf, int len);
static void anc_file_ear_adaptive_weight_printf(int id, void *buf, int len);
static void anc_file_ear_adaptive_target_param_printf(int id, void *buf, int len);
static void anc_file_ear_adaptive_target_printf(int id, void *buf, int len);
static void anc_file_ear_adaptive_mem_param_printf(int id, void *buf, int len);
static void anc_file_ear_adaptive_mem_data_printf(int id, void *buf, int len);
static void anc_file_ear_adaptive_dut_cmp_printf(int id, void *buf, int len);

struct __anc_ext_printf {
    void (*p)(int id, void *buf, int len);
};

//各个参数的打印信息，需严格按照ear_adaptive_id_table的顺序排序
const struct __anc_ext_printf anc_ext_printf[] = {
    { anc_file_ear_adaptive_base_cfg_printf },

    /*--------头戴式 左声道 / TWS LR---------*/
    { anc_file_ear_adaptive_iir_general_printf },
    { anc_file_ear_adaptive_iir_gains_printf },
    { anc_file_ear_adaptive_iir_gains_printf },
    { anc_file_ear_adaptive_iir_gains_printf },
    { anc_file_ear_adaptive_iir_printf },
    { anc_file_ear_adaptive_iir_printf },
    { anc_file_ear_adaptive_iir_printf },
    { anc_file_ear_adaptive_weight_param_printf },
    { anc_file_ear_adaptive_weight_printf },
    { anc_file_ear_adaptive_weight_printf },
    { anc_file_ear_adaptive_weight_printf },
    { anc_file_ear_adaptive_weight_printf },
    { anc_file_ear_adaptive_weight_printf },
    { anc_file_ear_adaptive_weight_printf },
    { anc_file_ear_adaptive_target_param_printf },
    { anc_file_ear_adaptive_target_printf },
    { anc_file_ear_adaptive_target_printf },
    { anc_file_ear_adaptive_mem_param_printf },
    { anc_file_ear_adaptive_mem_data_printf },
    { anc_file_ear_adaptive_mem_data_printf },

    /*------------头戴式 右声道--------------*/
    { anc_file_ear_adaptive_iir_general_printf },
    { anc_file_ear_adaptive_iir_gains_printf },
    { anc_file_ear_adaptive_iir_gains_printf },
    { anc_file_ear_adaptive_iir_gains_printf },
    { anc_file_ear_adaptive_iir_printf },
    { anc_file_ear_adaptive_iir_printf },
    { anc_file_ear_adaptive_iir_printf },
    { anc_file_ear_adaptive_weight_param_printf },
    { anc_file_ear_adaptive_weight_printf },
    { anc_file_ear_adaptive_weight_printf },
    { anc_file_ear_adaptive_weight_printf },
    { anc_file_ear_adaptive_weight_printf },
    { anc_file_ear_adaptive_weight_printf },
    { anc_file_ear_adaptive_weight_printf },
    { anc_file_ear_adaptive_target_param_printf },
    { anc_file_ear_adaptive_target_printf },
    { anc_file_ear_adaptive_target_printf },
    { anc_file_ear_adaptive_mem_param_printf },
    { anc_file_ear_adaptive_mem_data_printf },
    { anc_file_ear_adaptive_mem_data_printf },

    //FF->耳道自适应产测数据
    { anc_file_ear_adaptive_dut_cmp_printf },
    { anc_file_ear_adaptive_dut_cmp_printf },
    { anc_file_ear_adaptive_dut_cmp_printf },
    { anc_file_ear_adaptive_dut_cmp_printf },

};

#endif

static int anc_cfg_analysis_ear_adaptive(u8 *file_data, int file_len, u8 alloc_flag);
static u8 *anc_ext_tool_data_alloc(u32 id, u8 *data, int len);
static struct anc_ext_alloc_bulk_t *anc_ext_tool_data_alloc_query(u32 id);

static struct __anc_ext_tool_hdl *tool_hdl = NULL;

//ANC_EXT 初始化
void anc_ext_tool_init(void)
{
    int ret;
    tool_hdl = zalloc(sizeof(struct __anc_ext_tool_hdl));
    INIT_LIST_HEAD(&tool_hdl->alloc_list);
    //解析anc_ext.bin
    if (anc_ext_rsfile_read()) {
        anc_ext_log("ERR! anc_ext.bin read error\n");
    }
    //获取耳道自适应产测数据文件
    u8 *tmp_dut_buf = malloc(ANC_EXT_EAR_ADAPTIVE_DUT_MAX_LEN);
    ret = syscfg_read(CFG_ANC_ADAPTIVE_DUT_ID, tmp_dut_buf, ANC_EXT_EAR_ADAPTIVE_DUT_MAX_LEN);
    if (ret > 0) {
        //解析申请buffer使用
        anc_ext_log("ANC EAR ADAPTIVE DUT DATA SUSS!, ret %d\n", ret);
        anc_ext_subfile_analysis_each(FILE_ID_ANC_EXT_EAR_ADAPTIVE_DUT_CMP, tmp_dut_buf, ret, 1);
    } else {
        anc_ext_log("ANC EAR ADAPTIVE DUT DATA EMPTY!\n");
        //填充0数据,避免产测读数异常
        struct anc_ext_subfile_head dut_tmp;
        dut_tmp.file_id = FILE_ID_ANC_EXT_EAR_ADAPTIVE_DUT_CMP;
        dut_tmp.file_len = sizeof(struct anc_ext_subfile_head);
        dut_tmp.version = 0;
        dut_tmp.id_cnt = 0;
        anc_ext_subfile_analysis_each(FILE_ID_ANC_EXT_EAR_ADAPTIVE_DUT_CMP, (u8 *)&dut_tmp, dut_tmp.file_len, 1);
    }
    /* ret = syscfg_read(CFG_ANC_ADAPTIVE_DUT_EN_ID, &tool_hdl->ear_adaptive.dut_cmp_en, 1); */

    /* if (ret <= 0) { */
    /* anc_ext_log("ANC EAR ADAPTIVE DUT EN EMPTY!\n"); */
    /* } */
    free(tmp_dut_buf);
}

/*
   ANC_EXT 数据发送
	param : cmd2pc 命令对象
			buf 发送buf
			len 发送长度
*/
static void anc_ext_tool_send_buf(u8 cmd2pc, u8 *buf, int len)
{
    if (tool_hdl->uart_sel == ANC_EXT_UART_SEL_SPP) {
        if (len > 512 - 4) {
            printf("ERROR anc_ext_tool send_buf len = %d overflow\n", len);
            return;
        }
        u8 *cmd = malloc(len + 4);
        //透传命令标识
        cmd[0] = 0xFD;
        cmd[1] = 0x90;
        //ANC_EXT命令标识
        cmd[2] = 0xB0;
        cmd[3] = cmd2pc;
        memcpy(cmd + 4, buf, len);
        anctool_api_write(cmd, len + 4);
        free(cmd);
    }
#if TCFG_ANC_BOX_ENABLE
    else {
        if (len > 32 - 3) {
            printf("ERROR anc_ext_tool send_buf len = %d overflow\n", len);
            return;
        }
        u8 *cmd = malloc(len + 3);
        //透传命令标识
        cmd[0] = 0x90;
        //ANC_EXT命令标识
        cmd[1] = 0xB0;
        cmd[2] = cmd2pc;
        memcpy(cmd + 3, buf, len);
        chargestore_api_write(cmd, len + 3);
        free(cmd);
    }
#endif
}

/*
   ANC_EXT 命令执行结果回复
	param : cmd2pc 命令对象
			ret 执行结果(TRUE/FALSE)
			err_num 错误代码
*/
static void anc_ext_tool_cmd_ack(u8 cmd2pc, u8 ret, u8 err_num)
{
    if (tool_hdl->uart_sel == ANC_EXT_UART_SEL_SPP) {
        u8 cmd[5];
        //透传命令标识
        cmd[0] = 0xFD;
        cmd[1] = 0x90;
        //ANC_EXT命令标识
        cmd[2] = 0xB0;
        if (ret == TRUE) {
            cmd[3] = cmd2pc;
            anctool_api_write(cmd, 4);
        } else {
            cmd[3] = 0xFE;
            cmd[4] = err_num;
            anctool_api_write(cmd, 5);
        }
    }
#if TCFG_ANC_BOX_ENABLE
    else {
        u8 cmd[3];
        //透传命令标识
        cmd[0] = 0x90;
        //ANC_EXT命令标识
        cmd[1] = 0xB0;
        if (ret == TRUE) {
            cmd[2] = cmd2pc;
        } else {
            cmd[2] = 0xFE;
        }
        chargestore_api_write(cmd, 3);
    }
#endif
}

//ANC_EXT CMD事件处理
void anc_ext_tool_cmd_deal(u8 *data, u16 len, enum ANC_EXT_UART_SEL uart_sel)
{
    u8 cmd = data[0];
    int ret;
    tool_hdl->uart_sel = uart_sel;
    switch (cmd) {
    case CMD_GET_VERSION:
        anc_ext_log("CMD_GET_VERSION\n");
        break;
    case CMD_TIME_DOMAIN_SHOW_CH_SET:
        anc_ext_log("CMD_TIME_DOMAIN_SHOW_CH_SET, %d\n", data[1]);
        tool_hdl->ear_adaptive.time_domain_show_en = data[1];
        if (data[1]) {
            tool_hdl->ear_adaptive.time_domain_len = sizeof(__icsd_time_data);
            tool_hdl->ear_adaptive.time_domain_buf = malloc(tool_hdl->ear_adaptive.time_domain_len);
            if (!tool_hdl->ear_adaptive.time_domain_buf) {
                anc_ext_tool_cmd_ack(cmd, FALSE, ERR_NO);
                break;
            }
        } else {
            free(tool_hdl->ear_adaptive.time_domain_buf);
            tool_hdl->ear_adaptive.time_domain_buf = NULL;
        }
        anc_ext_tool_cmd_ack(cmd, TRUE, ERR_NO);
        break;

    case CMD_BYPASS_TEST:
        anc_ext_log("CMD_BYPASS_TEST, mode %d\n", data[1]);
        if (anc_mode_get() == ANC_ON && (anc_ext_ear_adaptive_param_check() == 0)) {
            if (data[1]) {
                //初始化参数依赖自适应流程
                struct anc_ext_ear_adaptive_param *cfg = anc_ext_ear_adaptive_cfg_get();
                anc_ear_adaptive_train_set_bypass(ANC_ADAPTIVE_FF_ORDER, \
                                                  cfg->base_cfg->bypass_vol, cfg->base_cfg->calr_sign[2]);
            } else {
                anc_ear_adaptive_train_set_bypass_off();
            }
            anc_ext_tool_cmd_ack(cmd, TRUE, ERR_NO);
        } else {
            anc_ext_tool_cmd_ack(cmd, FALSE, ERR_NO_ANC_MODE);
        }
        break;

    case CMD_EAR_ADAPTIVE_RUN:
        anc_ext_log("CMD_EAR_ADAPTIVE_RUN, mode %d\n", data[1]);
        tool_hdl->ear_adaptive.train_mode = data[1];
        ret = audio_anc_mode_ear_adaptive(1);
        if (ret) {
            anc_ext_tool_cmd_ack(cmd, FALSE, ERR_EAR_FAIL);
        } else {
            tool_hdl->tool_ear_adaptive_en = 1;
        }
        break;
    case CMD_DEBUG_TOOL:
        anc_ext_log("CMD_DEBUG_TOOL, mode %d\n", data[1]);
        if (data[1]) {
            audio_anc_debug_tool_open();
        } else {
            audio_anc_debug_tool_close();
        }
        anc_ext_tool_cmd_ack(cmd, TRUE, ERR_NO);
        break;
    case CMD_ADAPTIVE_TYPE_GET:
        anc_ext_log("CMD_ADAPTIVE_TYPE_GET\n");
        u8 type = (ICSD_EP_TYPE_V2 == ICSD_HEADSET) ? ANC_ADAPTIVE_TYPE_HEADSET : ANC_ADAPTIVE_TYPE_TWS;
        anc_ext_tool_send_buf(cmd, &type, 1);
        break;
    case CMD_ADAPTIVE_MODE_GET:
        anc_ext_log("CMD_ADAPTIVE_MODE_GET\n");
        u8 mode = ANC_TRAIN_MODE_TONE_BYPASS_PNC;
        anc_ext_tool_send_buf(cmd, &mode, 1);
        break;
    case CMD_FUNCTION_ENABLEBIT_GET:
        anc_ext_log("CMD_FUNCTION_ENABLEBIT_GET\n");
        tool_hdl->tool_online = 1;
        u16 en = ANC_EXT_FUNC_EN_ADAPTIVE;
        audio_anc_param_map_init();
        anc_ext_tool_send_buf(cmd, (u8 *)&en, 2);
        break;
    case CMD_EAR_ADAPTIVE_DUT_SAVE:
        anc_ext_log("CMD_EAR_ADAPTIVE_DUT_SAVE\n");
        struct anc_ext_alloc_bulk_t *bulk = anc_ext_tool_data_alloc_query(FILE_ID_ANC_EXT_EAR_ADAPTIVE_DUT_CMP);
        if (bulk) {
            ret = syscfg_write(CFG_ANC_ADAPTIVE_DUT_ID, bulk->alloc_addr, bulk->alloc_len);
            if (ret <= 0) {
                anc_ext_log("ANC_EAR_ADAPTIVE DUT write to vm err, ret %d\n", ret);
                anc_ext_tool_cmd_ack(cmd, FALSE, ERR_NO);
                break;
            }
        }
        /* ret = syscfg_write(CFG_ANC_ADAPTIVE_DUT_EN_ID, &tool_hdl->ear_adaptive.dut_cmp_en, 1); */
        /* if (ret <= 0) { */
        /* anc_ext_log("ANC_EAR_ADAPTIVE DUT_EN write to vm err, ret %d\n", ret); */
        /* anc_ext_tool_cmd_ack(cmd, FALSE, ERR_NO); */
        /* break; */
        /* } */
        anc_ext_tool_cmd_ack(cmd, TRUE, ERR_NO);
        break;
    case CMD_EAR_ADAPTIVE_DUT_EN_SET:
        anc_ext_log("CMD_EAR_ADAPTIVE_DUT_EN_SET %d \n", data[1]);
        tool_hdl->ear_adaptive.dut_cmp_en = data[1];
        break;
    case CMD_EAR_ADAPTIVE_DUT_CLEAR:
        anc_ext_log("CMD_EAR_ADAPTIVE_DUT_CLEAR\n");
        if (anc_ear_adaptive_busy_get()) {
            anc_ext_tool_cmd_ack(cmd, FALSE, ERR_EAR_FAIL);
            break;
        }
        //填充0数据,避免产测读数异常
        struct anc_ext_subfile_head dut_tmp;
        dut_tmp.file_id = FILE_ID_ANC_EXT_EAR_ADAPTIVE_DUT_CMP;
        dut_tmp.file_len = sizeof(struct anc_ext_subfile_head);
        dut_tmp.version = 0;
        dut_tmp.id_cnt = 0;
        anc_ext_subfile_analysis_each(FILE_ID_ANC_EXT_EAR_ADAPTIVE_DUT_CMP, (u8 *)&dut_tmp, dut_tmp.file_len, 1);
        tool_hdl->ear_adaptive.ff_dut_pz_cmp = NULL;
        tool_hdl->ear_adaptive.ff_dut_sz_cmp = NULL;
        tool_hdl->ear_adaptive.rff_dut_pz_cmp = NULL;
        tool_hdl->ear_adaptive.rff_dut_sz_cmp = NULL;

        ret = syscfg_write(CFG_ANC_ADAPTIVE_DUT_ID, (u8 *)&dut_tmp, dut_tmp.file_len);
        if (ret <= 0) {
            anc_ext_log("ANC_EAR_ADAPTIVE DUT write to vm err, ret %d\n", ret);
            anc_ext_tool_cmd_ack(cmd, FALSE, ERR_NO);
            break;
        }
        anc_ext_tool_cmd_ack(cmd, TRUE, ERR_NO);
        break;
    case CMD_EAR_ADAPTIVE_SIGN_TRIM:
        anc_ext_log("CMD_EAR_ADAPTIVE_SIGN_TRIM\n");
        tool_hdl->ear_adaptive.train_mode = EAR_ADAPTIVE_MODE_SIGN_TRIM;
        ret = audio_anc_mode_ear_adaptive(1);
        if (ret) {
            anc_ext_tool_cmd_ack(cmd, FALSE, ERR_EAR_FAIL);
        }
        //符号校准完毕后回复命令
        break;
    default:
        anc_ext_tool_cmd_ack(cmd, FALSE, ERR_NO);
        break;
    };
}

//耳道自适应执行结果回调函数
void anc_ext_tool_ear_adaptive_end_cb(u8 result)
{
    if (tool_hdl->tool_ear_adaptive_en) {
        anc_ext_tool_send_buf(CMD_EAR_ADAPTIVE_RUN, &result, 1);
        tool_hdl->tool_ear_adaptive_en = 0;
    }
    tool_hdl->ear_adaptive.train_mode = EAR_ADAPTIVE_MODE_NORAML;
}

//耳道自适应-符号校准 执行结果回调函数
void anc_ext_tool_ear_adaptive_sign_trim_end_cb(u8 *buf, int len)
{
    struct anc_ext_ear_adaptive_param *p = &tool_hdl->ear_adaptive;
    //符号校准数据上传工具
    anc_ext_tool_send_buf(CMD_EAR_ADAPTIVE_SIGN_TRIM, buf, len);
    //覆盖当前自适应参数符号对应指针
    /* p->base_sign = (struct __anc_ext_ear_adaptive_base_sign *)anc_ext_tool_data_alloc(\ */
    /* ANC_EXT_EAR_ADAPTIVE_BASE_SIGN_ID, buf, sizeof(struct __anc_ext_ear_adaptive_base_sign)); */
    /* p->base_sign->sign_trim_en = 1; */
    /* memcpy(&p->base_sign->sz_calr_sign, buf, len); */
    p->train_mode = EAR_ADAPTIVE_MODE_NORAML;
}

//参数临时缓存申请
static u8 *anc_ext_tool_data_alloc(u32 id, u8 *data, int len)
{
    struct anc_ext_alloc_bulk_t *bulk;
    //更新ID对应地址
    list_for_each_entry(bulk, &tool_hdl->alloc_list, entry) {
        if (id == bulk->alloc_id) {
            free(bulk->alloc_addr);
            goto __exit;
        }
    }
    //新增ID 链表
    bulk = zalloc(sizeof(struct anc_ext_alloc_bulk_t));
    bulk->alloc_id = id;
    list_add_tail(&bulk->entry, &tool_hdl->alloc_list);

__exit:
    bulk->alloc_addr = malloc(len);
    bulk->alloc_len = len;
    if (!bulk->alloc_addr) {
        return NULL;
    }
    memcpy(bulk->alloc_addr, data, len);
    return bulk->alloc_addr;
}


//参数临时缓存查询
static struct anc_ext_alloc_bulk_t *anc_ext_tool_data_alloc_query(u32 id)
{
    struct anc_ext_alloc_bulk_t *bulk;
    list_for_each_entry(bulk, &tool_hdl->alloc_list, entry) {
        if (id == bulk->alloc_id) {
            return bulk;
        }
    }
    return NULL;
}

//参数临时缓存释放
static void anc_ext_tool_data_alloc_free(void)
{
    struct anc_ext_alloc_bulk_t *bulk;
    struct anc_ext_alloc_bulk_t *temp;
    //更新文件ID对应地址
    list_for_each_entry_safe(bulk, temp, &tool_hdl->alloc_list, entry) {
        free(bulk->alloc_addr);
        list_del(&bulk->entry);
        free(bulk);
    }
}

//ANC_EXT SUBFILE文件解析遍历
int anc_ext_subfile_analysis_each(u32 file_id, u8 *data, int len, u8 alloc_flag)
{
    int ret = 0;

    anc_ext_log("Analysis - FILE ID 0x%x\n", file_id);
    switch (file_id) {
    //anc_ext.bin 文件ID解析
    case FILE_ID_ANC_EXT_EAR_ADAPTIVE_BASE:
    case FILE_ID_ANC_EXT_EAR_ADAPTIVE_FF_IIR:
    case FILE_ID_ANC_EXT_EAR_ADAPTIVE_FF_WEIGHT:
    case FILE_ID_ANC_EXT_EAR_ADAPTIVE_FF_TARGET:
    case FILE_ID_ANC_EXT_EAR_ADAPTIVE_FF_RECORDER:
        ret = anc_cfg_analysis_ear_adaptive(data, len, alloc_flag);
        /* put_buf(data, len); */
        break;
    case FILE_ID_ANC_EXT_EAR_ADAPTIVE_DUT_CMP:
        if (alloc_flag) {
            u8 *dut_buf = anc_ext_tool_data_alloc(FILE_ID_ANC_EXT_EAR_ADAPTIVE_DUT_CMP, data, len);
            ret = anc_cfg_analysis_ear_adaptive(dut_buf, len, 0);
        }
        /* put_buf(data, len); */
        break;
    case FILE_ID_ANC_EXT_BIN:
        break;
    default:
        break;
    }
    return ret;
}

//SUBFILE 工具写文件文件结束
int anc_ext_tool_write_end(u32 file_id, u8 *data, int len, u8 alloc_flag)
{
    int ret = anc_ext_subfile_analysis_each(file_id, data, len, alloc_flag);
    //工具下发参数针对特殊ID做处理
    switch (file_id) {
    case FILE_ID_ANC_EXT_EAR_ADAPTIVE_DUT_CMP:
        break;
    }
    return ret;
}


//SUBFILE 工具开始读文件
int anc_ext_tool_read_file_start(u32 file_id, u8 **data, u32 *len)
{
    switch (file_id) {
    case FILE_ID_ANC_EXT_BIN:
        anc_ext_log("read FILE_ID_ANC_EXT_BIN\n");
        *data = anc_ext_rsfile_get(len);
        if (*data == NULL) {
            return 1;
        }
        break;
    case FILE_ID_ANC_EXT_EAR_ADAPTIVE_TIME_DOMAIN:
        anc_ext_log("read FILE_ID_ANC_EXT_EAR_ADAPTIVE_TIME_DOMAIN\n");
        *len = tool_hdl->ear_adaptive.time_domain_len;
        *data = tool_hdl->ear_adaptive.time_domain_buf;
        if (*data == NULL) {
            return 1;
        }
        break;
    case FILE_ID_ANC_EXT_EAR_ADAPTIVE_DUT_CMP:
        struct anc_ext_alloc_bulk_t *bulk = anc_ext_tool_data_alloc_query(FILE_ID_ANC_EXT_EAR_ADAPTIVE_DUT_CMP);
        if (bulk) {
            *len = bulk->alloc_len;
            *data = bulk->alloc_addr;
        } else {
            return 1;
        }
        break;
    default:
        break;
    }
    return 0;
}

//SUBFILE 工具读文件结束
int anc_ext_tool_read_file_end(u32 file_id)
{
    switch (file_id) {
    case FILE_ID_ANC_EXT_EAR_ADAPTIVE_FF_IIR:
        break;
    default:
        break;
    }
    return 0;
}

/*-----------------------文件解析------------------------------*/

static void anc_file_ear_adaptive_base_cfg_printf(int id, void *buf, int len)
{
    struct __anc_ext_ear_adaptive_base_cfg *cfg = (struct __anc_ext_ear_adaptive_base_cfg *)buf;
    anc_ext_log("---ANC_EXT_EAR_ADAPTIVE_BASE_CFG_ID 0x%x---\n", id);

    anc_ext_log("sign_trim_en      :%d \n", cfg->sign_trim_en);
    anc_ext_log("sz_calr_sign      :%d \n", cfg->calr_sign[0]);
    anc_ext_log("pz_calr_sign      :%d \n", cfg->calr_sign[1]);
    anc_ext_log("bypass_calr_sign  :%d \n", cfg->calr_sign[2]);
    anc_ext_log("perf_calr_sign    :%d \n", cfg->calr_sign[3]);
    anc_ext_log("vld1              :%d \n", cfg->vld1);
    anc_ext_log("vld2              :%d \n", cfg->vld2);
    anc_ext_log("adaptive_mode     :%d \n", cfg->adaptive_mode);
    anc_ext_log("adaptive_times    :%d \n", cfg->adaptive_times);
    anc_ext_log("tonel_delay       :%d \n", cfg->tonel_delay);
    anc_ext_log("toner_delay       :%d \n", cfg->toner_delay);
    anc_ext_log("pzl_delay 	       :%d \n", cfg->pzl_delay);
    anc_ext_log("pzr_delay         :%d \n", cfg->pzr_delay);
    anc_ext_log("bypass_vol        :%d/100 \n", (int)(cfg->bypass_vol * 100));
    anc_ext_log("sz_pri_thr        :%d/100 \n", (int)(cfg->sz_pri_thr * 100));
}

static void anc_file_ear_adaptive_iir_general_printf(int id, void *buf, int len)
{
    struct __anc_ext_ear_adaptive_iir_general *cfg = (struct __anc_ext_ear_adaptive_iir_general *)buf;
    anc_ext_log("---ANC_EXT_EAR_ADAPTIVE_FF_GENERAL_ID 0x%x---\n", id);
    anc_ext_log("biquad_level_l    :%d \n", cfg->biquad_level_l);
    anc_ext_log("biquad_level_h    :%d \n", cfg->biquad_level_h);
    anc_ext_log("total_gain_freq_l :%d/100 \n", (int)(cfg->total_gain_freq_l * 100));
    anc_ext_log("total_gain_freq_h :%d/100 \n", (int)(cfg->total_gain_freq_h * 100));
    anc_ext_log("total_gain_limit  :%d/100 \n", (int)(cfg->total_gain_limit * 100));
}

static void anc_file_ear_adaptive_iir_gains_printf(int id, void *buf, int len)
{
    struct __anc_ext_ear_adaptive_iir_gains *cfg = (struct __anc_ext_ear_adaptive_iir_gains *)buf;
    anc_ext_log("---ANC_EXT_EAR_ADAPTIVE_FF_GAINS_ID 0x%x---\n", id);
    anc_ext_log("iir_target_gain_limit :%d/100 \n", (int)(cfg->iir_target_gain_limit * 100));
    anc_ext_log("def_total_gain        :%d/100 \n", (int)(cfg->def_total_gain * 100));
    anc_ext_log("upper_limit_gain      :%d/100 \n", (int)(cfg->upper_limit_gain * 100));
    anc_ext_log("lower_limit_gain      :%d/100 \n", (int)(cfg->lower_limit_gain * 100));
}

static void anc_file_ear_adaptive_iir_printf(int id, void *buf, int len)
{

    struct __anc_ext_ear_adaptive_iir *iir = (struct __anc_ext_ear_adaptive_iir *)buf;
    anc_ext_log("---ANC_EXT_EAR_ADAPTIVE_FF_IIR_ID 0x%x---\n", id);
    int cnt = len / sizeof(struct __anc_ext_ear_adaptive_iir);
    for (int i = 0; i < cnt; i++) {
        anc_ext_log("iir type_%d %d, fixed_en %d\n", i, iir[i].type, iir[i].fixed_en);
        anc_ext_log("iir_def      f %d/100, g %d/100, q %d/100\n", (int)(iir[i].def.fre * 100.0), \
                    (int)(iir[i].def.gain * 100.0), (int)(iir[i].def.q * 100.0));
        anc_ext_log("up_limit     f %d/100, g %d/100, q %d/100\n", (int)(iir[i].upper_limit.fre * 100.0), \
                    (int)(iir[i].upper_limit.gain * 100.0), (int)(iir[i].upper_limit.q * 100.0));
        anc_ext_log("lower_limit  f %d/100, g %d/100, q %d/100\n", (int)(iir[i].lower_limit.fre * 100.0), \
                    (int)(iir[i].lower_limit.gain * 100.0), (int)(iir[i].lower_limit.q * 100.0));
    }
}

static void anc_file_ear_adaptive_weight_param_printf(int id, void *buf, int len)
{
    struct __anc_ext_ear_adaptive_weight_param *cfg = (struct __anc_ext_ear_adaptive_weight_param *)buf;
    anc_ext_log("---ANC_EXT_EAR_ADAPTIVE_FF_WEIGHT_PARAM_ID 0x%x---\n", id);
    anc_ext_log("mse_level_l           :%d \n", cfg->mse_level_l);
    anc_ext_log("mse_level_h           :%d \n", cfg->mse_level_h);
}

static void anc_file_ear_adaptive_weight_printf(int id, void *buf, int len)
{
    struct __anc_ext_ear_adaptive_weight *cfg = (struct __anc_ext_ear_adaptive_weight *)buf;
    anc_ext_log("---ANC_EXT_EAR_ADAPTIVE_FF_WEIGHT/MSE_ID 0x%x---\n", id);
    int cnt = len / sizeof(float);
    for (int i = 0; i < cnt; i++) {
        anc_ext_log("data :%d/100 \n", (int)(cfg->data[i] * 100));
    }
}

static void anc_file_ear_adaptive_target_param_printf(int id, void *buf, int len)
{
    struct __anc_ext_ear_adaptive_target_param *cfg = (struct __anc_ext_ear_adaptive_target_param *)buf;
    anc_ext_log("---ANC_EXT_EAR_ADAPTIVE_FF_TARGET_PARAM_ID 0x%x---\n", id);
    anc_ext_log("cmp_en           :%d \n", cfg->cmp_en);
}

static void anc_file_ear_adaptive_target_printf(int id, void *buf, int len)
{
    struct __anc_ext_ear_adaptive_target_cmp *cfg = (struct __anc_ext_ear_adaptive_target_cmp *)buf;
    int cnt = len / sizeof(float);
    anc_ext_log("---ANC_EXT_EAR_ADAPTIVE_FF_TARGET_SV/CMP_ID 0x%x, len %d---\n", id, cnt);
    for (int i = 0; i < cnt; i++) {
        anc_ext_log("%d \n", (int)(cfg->data[i] * 1000));
    }
}

static void anc_file_ear_adaptive_mem_param_printf(int id, void *buf, int len)
{
    struct __anc_ext_ear_adaptive_mem_param *cfg = (struct __anc_ext_ear_adaptive_mem_param *)buf;
    anc_ext_log("---ANC_EXT_EAR_ADAPTIVE_FF_RECORDER_PARAM_ID 0x%x---\n", id);
    anc_ext_log("mem_curve_nums   :%d \n", cfg->mem_curve_nums);
    anc_ext_log("ear_recorder       :%d \n", cfg->ear_recorder);
}

static void anc_file_ear_adaptive_mem_data_printf(int id, void *buf, int len)
{
    struct __anc_ext_ear_adaptive_mem_data *cfg = (struct __anc_ext_ear_adaptive_mem_data *)buf;
    int cnt = len / sizeof(float);
    int i;
    anc_ext_log("---ANC_EXT_EAR_ADAPTIVE_FF_RECORDER_ID 0x%x, len %d---\n", id, cnt);
    for (i = 0; i < cnt; i++) {
        anc_ext_log("%d\n", (int)(cfg->data[i] * 1000));
    }
}

static void anc_file_ear_adaptive_dut_cmp_printf(int id, void *buf, int len)
{
    struct __anc_ext_ear_adaptive_dut_data *cfg = (struct __anc_ext_ear_adaptive_dut_data *)buf;
    int cnt = len / sizeof(float);
    int i;
    anc_ext_log("---ANC_EXT_EAR_ADAPTIVE_FF_DUT_CMP_ID 0x%x, len %d---\n", id, cnt);
    for (i = 0; i < cnt; i++) {
        anc_ext_log("%d\n", (int)(cfg->data[i] * 1000));
    }
}

/*
   ANCEXT 耳道自适应SUBFILE内参数ID解析
param: file_data, file_len
*/
static int anc_cfg_analysis_ear_adaptive(u8 *file_data, int file_len, u8 alloc_flag)
{
    int i, j;
    u8 *temp_dat;
    struct anc_ext_subfile_head *file = (struct anc_ext_subfile_head *)file_data;
    struct anc_ext_ear_adaptive_param *p = &tool_hdl->ear_adaptive;
    const struct __anc_ext_subfile_id_table *table = ear_adaptive_id_table;
    int table_cnt = ARRAY_SIZE(ear_adaptive_id_table);

    if (file) {
        anc_ext_log("file->cnt %d\n", file->id_cnt);
        if (!file->id_cnt) {
            return 1;
        }
        struct anc_ext_id_head *id_head = (struct anc_ext_id_head *)file->data;
        temp_dat = file->data + (file->id_cnt * sizeof(struct anc_ext_id_head));
        for (i = 0; i < file->id_cnt; i++) {
            anc_ext_log("id:0X%x, offset:%d,len:%d\n", id_head->id, id_head->offset, id_head->len);
            for (j = 0; j < table_cnt; j++) {
                if (id_head->id == table[j].id) {
                    if (id_head->len) {
                        u32 dat_p = (u32)(temp_dat + id_head->offset);
                        if (alloc_flag) {	//工具在线调试申请临时空间
                            u8 *temp = anc_ext_tool_data_alloc(id_head->id, temp_dat + id_head->offset, id_head->len);
                            if (!temp) {
                                anc_ext_log("Analysis tool_alloc err!! data_id %d\n", id_head->id);
                                return -1;
                            }
                            dat_p = (u32)temp;
                        }
                        //拷贝目标地址至对应参数结构体指针
                        memcpy(((u8 *)p) + table[j].offset, (u8 *)(&dat_p), sizeof(u32));
#if ANC_EXT_CFG_LOG_EN
                        /* put_buf((u8 *)dat_p, id_head->len); */
                        anc_ext_printf[j].p(id_head->id, (void *)dat_p, id_head->len);
#endif
                    }
                }
            }
            id_head++;
        }
        return 0;
    }
    return 1;
}

//ANC_EXT 耳道自适应参数检查
u8 anc_ext_ear_adaptive_param_check(void)
{
    struct anc_ext_ear_adaptive_param *cfg = &tool_hdl->ear_adaptive;
    for (u32 i = (u32)(&cfg->base_cfg); i <= (u32)(&cfg->ff_target_param); i += 4) {
        if ((*(u32 *)i) == 0) {
            anc_ext_log("ERR:ANC_EXT cfg no enough!, offset=%d\n", i);
            return 1;
        }
    }
    if (cfg->ff_target_param->cmp_curve_num && \
        (!cfg->ff_target_sv || !cfg->ff_target_cmp)) {
        anc_ext_log("ERR:ANC_EXT target cfg no enough!, sv %p, cmp %p\n", \
                    cfg->ff_target_sv, cfg->ff_target_cmp);
        return 1;
    }
    if (cfg->ff_ear_mem_param) {
        if (cfg->ff_ear_mem_param->ear_recorder && (!cfg->ff_ear_mem_pz || !cfg->ff_ear_mem_sz)) {
            anc_ext_log("ERR:ANC_EXT ear_recoder cfg no enough! pz %p, sz %p\n", \
                        cfg->ff_ear_mem_pz, cfg->ff_ear_mem_sz);
            return 1;
        }
    } else {
        anc_ext_log("ERR:ANC_EXT ear_recoder cfg no enough! param %p\n", cfg->ff_ear_mem_param);
        return 1;
    }
    if (ICSD_EP_TYPE_V2 == ICSD_HEADSET) {
        for (u32 i = (u32)(&cfg->rff_iir_high_gains); i <= (u32)(&cfg->rff_target_param); i += 4) {
            if ((*(u32 *)i) == 0) {
                anc_ext_log("ERR:ANC_EXT R cfg no enough!, offset=%d\n", i);
                return 1;
            }
        }
        if (cfg->rff_target_param->cmp_curve_num && \
            (!cfg->rff_target_sv || !cfg->rff_target_cmp)) {
            anc_ext_log("ERR:ANC_EXT R target cfg no enough!, sv %p, cmp %p\n", \
                        cfg->rff_target_sv, cfg->rff_target_cmp);
            return 1;
        }
        if (cfg->rff_ear_mem_param) {
            if (cfg->rff_ear_mem_param->ear_recorder && (!cfg->rff_ear_mem_pz || !cfg->rff_ear_mem_sz)) {
                anc_ext_log("ERR:ANC_EXT R ear_recoder cfg no enough! pz %p, sz %p\n", \
                            cfg->rff_ear_mem_pz, cfg->rff_ear_mem_sz);
                return 1;
            }
        } else {
            anc_ext_log("ERR:ANC_EXT R ear_recoder cfg no enough! param %p\n", cfg->rff_ear_mem_param);
            return 1;
        }
    }
    return 0;
}

//ANC_EXT 获取耳道自适应的配置参数
struct anc_ext_ear_adaptive_param *anc_ext_ear_adaptive_cfg_get(void)
{
    if (tool_hdl) {
        return &tool_hdl->ear_adaptive;
    }
    return NULL;
}

//ANC_EXT 查询工具是否在线
u8 anc_ext_tool_online_get(void)
{
    if (tool_hdl) {
        return tool_hdl->tool_online;
    }
    return 0;
}

//ANC_EXT 工具断开连接
void anc_ext_tool_disconnect(void)
{
    if (tool_hdl) {
        /* tool_hdl->tool_online = 0; */
    }
}

//获取耳道自适应训练模式
enum ANC_EAR_ADPTIVE_MODE anc_ext_ear_adaptive_train_mode_get(void)
{
    if (tool_hdl) {
        return tool_hdl->ear_adaptive.train_mode;
    }
    return EAR_ADAPTIVE_MODE_NORAML;
}

u8 anc_ext_ear_adaptive_result_from(void)
{
    if (tool_hdl) {
        if (tool_hdl->ear_adaptive.base_cfg) {
            return tool_hdl->ear_adaptive.base_cfg->adaptive_mode;
        }
    }
    return EAR_ADAPTIVE_FAIL_RESULT_FROM_ADAPTIVE;
}

s8 anc_ext_ear_adaptive_sz_calr_sign_get(void)
{
    if (tool_hdl) {
        if (tool_hdl->ear_adaptive.base_cfg) {
            return tool_hdl->ear_adaptive.base_cfg->calr_sign[0];
        }
    }
    return 0;
}


#endif
