/*********************************************************************************************
    *   Filename        : lib_driver_config.c

    *   Description     : Optimized Code & RAM (编译优化配置)

    *   Author          : Bingquan

    *   Email           : caibingquan@zh-jieli.com

    *   Last modifiled  : 2019-03-18 14:58

    *   Copyright:(c)JIELI  2011-2019  @ , All Rights Reserved.
*********************************************************************************************/
#include "app_config.h"
#include "system/includes.h"

#if TCFG_SD0_SD1_USE_THE_SAME_HW
const int sd0_sd1_use_the_same_hw = 1;
#else
const int sd0_sd1_use_the_same_hw = 0;
#endif

#if TCFG_KEEP_CARD_AT_ACTIVE_STATUS
const int keep_card_at_active_status = 1;
#else
const int keep_card_at_active_status = 0;
#endif

#if TCFG_SDX_CAN_OPERATE_MMC_CARD
const int sdx_can_operate_mmc_card = TCFG_SDX_CAN_OPERATE_MMC_CARD;
#else
const int sdx_can_operate_mmc_card = 0;
#endif

#if TCFG_POWER_MODE_QUIET_ENABLE
const int config_dcdc_mode = 1;
#else
const int config_dcdc_mode = 0;
#endif

#if(TCFG_CLOCK_SYS_SRC == SYS_CLOCK_INPUT_PLL_RCL) //系统时钟源选择
const int  clock_sys_src_use_lrc_hw = 1; //当使用lrc时timer.c需要特殊设置
#else
const int  clock_sys_src_use_lrc_hw = 0;
#endif

#ifdef TCFG_VAD_LOWPOWER_CLOCK
const char config_vad_lowpower_clock = TCFG_VAD_LOWPOWER_CLOCK;
#else
const char config_vad_lowpower_clock = 0;
#endif

#if defined(CONFIG_CPU_BR28) || (TCFG_CHARGESTORE_PORT == IO_PORT_DP)
const int config_otg_slave_detect_method_2 = 1;
#else
const int config_otg_slave_detect_method_2 = 0;
#endif

/* 是否开启把vm配置项暂存到ram的功能 */
/* 具体使用方法和功能特性参考《项目帮助文档》的“11.4. 配置项管理 -VM配置项暂存RAM功能描述” */
const char vm_ram_storage_enable = FALSE;

const char vm_ram_storage_in_irq_enable = TRUE;

/* 设置vm的ram内存缓存区总大小限制，0为不限制，非0为限制大小（单位/byte）。 */
const int  vm_ram_storage_limit_data_total_size  = 16 * 1024;

const int config_rtc_enable = 0;
/**
 * @brief Log (Verbose/Info/Debug/Warn/Error)
 */
/*-----------------------------------------------------------*/
const char log_tag_const_v_CLOCK  = CONFIG_DEBUG_LIB(FALSE) ;
const char log_tag_const_i_CLOCK  = CONFIG_DEBUG_LIB(FALSE) ;
const char log_tag_const_d_CLOCK  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_CLOCK  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_CLOCK  = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_LP_TIMER  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_LP_TIMER  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_LP_TIMER  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_LP_TIMER  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_LP_TIMER  = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_LRC  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_LRC  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_LRC  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_LRC  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_LRC  = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_RCH AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_RCH AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_RCH AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_RCH AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_RCH AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_P33_MISC  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_P33_MISC  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_P33_MISC  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_P33_MISC  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_P33_MISC  = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_P33  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_P33  = CONFIG_DEBUG_LIB(1);
const char log_tag_const_d_P33  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_P33  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_P33  = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_PMU  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_PMU  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_d_PMU  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_PMU  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_PMU  = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_RTC  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_RTC  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_d_RTC  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_RTC  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_RTC  = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_WKUP  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_WKUP  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_d_WKUP  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_WKUP  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_WKUP  = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_SDFILE  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_SDFILE  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_SDFILE  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_SDFILE  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_SDFILE  = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_CHARGE  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_CHARGE  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_d_CHARGE  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_CHARGE  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_CHARGE  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_c_CHARGE  = CONFIG_DEBUG_LIB(FALSE);

const char log_tag_const_v_DEBUG  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_DEBUG  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_DEBUG  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_DEBUG  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_DEBUG  = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_PWM_LED  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_PWM_LED  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_PWM_LED  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_PWM_LED  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_PWM_LED  = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_KEY  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_KEY  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_KEY  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_KEY  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_KEY  = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_TMR  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_TMR  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_TMR  = CONFIG_DEBUG_LIB(0);
const char log_tag_const_w_TMR  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_TMR  = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_VM  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_VM  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_d_VM  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_VM  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_VM  = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_TRIM_VDD  = CONFIG_DEBUG_LIB(FALSE) ;
const char log_tag_const_i_TRIM_VDD  = CONFIG_DEBUG_LIB(1);
const char log_tag_const_d_TRIM_VDD  = CONFIG_DEBUG_LIB(1);
const char log_tag_const_w_TRIM_VDD  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_TRIM_VDD  = CONFIG_DEBUG_LIB(TRUE);

//audio dac
const char log_tag_const_v_SYS_DAC  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_SYS_DAC  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_SYS_DAC  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_SYS_DAC  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_SYS_DAC  = CONFIG_DEBUG_LIB(FALSE);


const char log_tag_const_v_APP_EDET  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_APP_EDET  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_APP_EDET  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_APP_EDET  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_APP_EDET  = CONFIG_DEBUG_LIB(FALSE);


const char log_tag_const_v_FM  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_FM  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_FM  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_FM  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_FM  = CONFIG_DEBUG_LIB(FALSE);

const char log_tag_const_v_CORE  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_CORE  = CONFIG_DEBUG_LIB(1);
const char log_tag_const_d_CORE  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_CORE  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_CORE  = CONFIG_DEBUG_LIB(FALSE);

const char log_tag_const_v_CACHE  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_CACHE  = CONFIG_DEBUG_LIB(1);
const char log_tag_const_d_CACHE  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_CACHE  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_CACHE  = CONFIG_DEBUG_LIB(FALSE);

const char log_tag_const_v_LP_KEY  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_LP_KEY  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_d_LP_KEY  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_LP_KEY  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_LP_KEY  = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_SD  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_SD  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_d_SD  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_SD  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_SD  = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_EAR_DETECT  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_EAR_DETECT  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_d_EAR_DETECT  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_EAR_DETECT  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_EAR_DETECT  = CONFIG_DEBUG_LIB(FALSE);

const char log_tag_const_v_TDM  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_TDM  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_d_TDM  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_TDM  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_TDM  = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_USB  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_USB  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_USB  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_USB  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_USB  = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_UART  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_UART  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_UART  = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_UART  = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_UART  = CONFIG_DEBUG_LIB(TRUE);

