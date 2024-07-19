#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".device_config.data.bss")
#pragma data_seg(".device_config.data")
#pragma const_seg(".device_config.text.const")
#pragma code_seg(".device_config.text")
#endif
#include "system/includes.h"
#include "app_main.h"
#include "app_config.h"
#include "asm/sdmmc.h"
#include "linein_dev.h"

#if TCFG_SD0_ENABLE
SD0_PLATFORM_DATA_BEGIN(sd0_data) = {
    .port = {
        TCFG_SD0_PORT_CMD,
        TCFG_SD0_PORT_CLK,
        TCFG_SD0_PORT_DA0,
        TCFG_SD0_PORT_DA1,
        TCFG_SD0_PORT_DA2,
        TCFG_SD0_PORT_DA3,
    },
    .data_width             = TCFG_SD0_DAT_MODE,
    .speed                  = TCFG_SD0_CLK,
    .detect_mode            = TCFG_SD0_DET_MODE,
    .priority				= 3,

#if (TCFG_SD0_DET_MODE == SD_IO_DECT)
    .detect_io              = TCFG_SD0_DET_IO,
    .detect_io_level        = TCFG_SD0_DET_IO_LEVEL,
    .detect_func            = sdmmc_0_io_detect,
#elif (TCFG_SD0_DET_MODE == SD_CLK_DECT)
    .detect_io_level        = TCFG_SD0_DET_IO_LEVEL,
    .detect_func            = sdmmc_0_clk_detect,
#else
    .detect_func            = sdmmc_cmd_detect,
#endif

#if (TCFG_SD0_POWER_SEL == SD_PWR_SDPG)
    .power                  = sd_set_power,
#else
    .power                  = NULL,
#endif

    SD0_PLATFORM_DATA_END()
};
#endif

/************************** linein KEY ****************************/
#if TCFG_APP_LINEIN_EN
struct linein_dev_data linein_data = {
    .enable = TCFG_APP_LINEIN_EN,
    .port   = NO_CONFIG_PORT,
    .up     = 1,
    .down   = 0,
    .ad_channel = NO_CONFIG_PORT,
    .ad_vol = 0,
};
#endif

REGISTER_DEVICES(device_table) = {
#if TCFG_SD0_ENABLE
    { "sd0", 	&sd_dev_ops,	(void *) &sd0_data},
#endif
#if TCFG_APP_LINEIN_EN
    { "linein",  &linein_dev_ops, (void *) &linein_data},
#endif
#if TCFG_UDISK_ENABLE
    { "udisk0",   &mass_storage_ops, NULL},
#endif
};
