#include "audio_debug.h"
#include "system/timer.h"
#include "audio_config.h"
#include "system/task.h"
#include "jlstream.h"
#include "classic/hci_lmp.h"
#include "media_memory.h"


#define AUD_CFG_DUMP_ENABLE				1	//音频配置跟踪使能
#define AUD_REG_DUMP_ENABLE				0	//音频寄存器跟踪使能
#define AUD_CACHE_INFO_DUMP_ENABLE		0 	//cache信息跟踪使能
#define AUD_TASK_INFO_DUMP_ENABLE		0	//任务运行信息跟踪使能
#define AUD_JLSTREAM_MEM_DUMP_ENABLE	0	//jlstream内存跟踪
#define AUD_BT_INFO_DUMP_ENABLE			0	//蓝牙音频流跟踪
#define SYS_MEM_INFO_DUMP_ENABLE		0	//系统内存信息跟踪
#define AUD_MEM_INFO_DUMP_ENABLE		0	//音频模块内存申请信息跟踪

const int CONFIG_MEDIA_MEM_DEBUG = AUD_MEM_INFO_DUMP_ENABLE;

static void audio_config_trace(void *priv)
{
    printf(">>Audio_Config_Trace:\n");
#if AUD_CFG_DUMP_ENABLE
    audio_config_dump();
#endif

#if AUD_REG_DUMP_ENABLE
    audio_adda_dump();
#endif

#if AUD_CACHE_INFO_DUMP_ENABLE
    extern void CacheReport(void);
    CacheReport();
#endif

#if AUD_TASK_INFO_DUMP_ENABLE
    task_info_output(0);
#endif

#if AUD_JLSTREAM_MEM_DUMP_ENABLE
    stream_mem_unfree_dump();
#endif

#if AUD_BT_INFO_DUMP_ENABLE
    printf("ESCO Tx Packet Num:%d", lmp_private_get_esco_tx_packet_num());
#endif

#if SYS_MEM_INFO_DUMP_ENABLE
    mem_stats();
#endif

#if AUD_MEM_INFO_DUMP_ENABLE
    media_mem_unfree_dump();
#endif
}

void audio_config_trace_setup(int interval)
{
    sys_timer_add(NULL, audio_config_trace, interval);
}

#define DAC_POP_UP_NOISE_DEBUG_DISABLE	0
#define DAC_POWER_ON_POP_DBG			1
#define DAC_MUTE_POP_DBG				2
#define DAC_POP_UP_NOISE_DEBUG_CFG		DAC_POP_UP_NOISE_DEBUG_DISABLE
static void dac_power_on_pop_debug(void *priv)
{

}

static void dac_mute_pop_debug(void *priv)
{

}

void audio_dac_pop_up_noise_debug(void)
{
#if (DAC_POP_UP_NOISE_DEBUG_CFG ==  DAC_POWER_ON_POP_DBG)
    sys_timer_add(NULL, dac_power_on_pop_debug, 3000);
#elif (DAC_POP_UP_NOISE_DEBUG_CFG ==  DAC_MUTE_POP_DBG)
    sys_timer_add(NULL, dac_mute_pop_debug, 3000);
#endif
}
