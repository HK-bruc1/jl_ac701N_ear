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
#include "asm/rtc.h"
#include "linein_dev.h"
#include "usb/host/usb_storage.h"
#include "asm/spi_hw.h"
#include "spi.h"

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

/************************** rtc ****************************/
#if TCFG_APP_RTC_EN
//初始一下当前时间
const struct sys_time def_sys_time = {
    .year = 2020,
    .month = 1,
    .day = 1,
    .hour = 0,
    .min = 0,
    .sec = 0,
};

//初始一下目标时间，即闹钟时间
const struct sys_time def_alarm = {
    .year = 2050,
    .month = 1,
    .day = 1,
    .hour = 0,
    .min = 0,
    .sec = 0,
};

/* extern void alarm_isr_user_cbfun(u8 index); */
RTC_DEV_PLATFORM_DATA_BEGIN(rtc_data)
.default_sys_time = &def_sys_time,
 .default_alarm = &def_alarm,

  .clk_sel = CLK_SEL_32K,
   /* #if defined(CONFIG_CPU_BR27) */
   /* .rtc_sel = HW_RTC, */
   /* #endif */
   //闹钟中断的回调函数,用户自行定义
   .cbfun = NULL,
    /* .cbfun = alarm_isr_user_cbfun, */
    RTC_DEV_PLATFORM_DATA_END()

#endif

#if (TCFG_HW_SPI1_ENABLE || TCFG_HW_SPI2_ENABLE)
const struct spi_platform_data spix_p_data[HW_SPI_MAX_NUM] = {
    {
        //spi0
    },
    {
        //spi1
        .port = {
            TCFG_HW_SPI1_PORT_CLK, //clk any io
            TCFG_HW_SPI1_PORT_DO, //do any io
            TCFG_HW_SPI1_PORT_DI, //di any io
#ifdef  TCFG_HW_SPI1_PORT_D2
            TCFG_HW_SPI1_PORT_D2, //d2 any io
#endif
#ifdef  TCFG_HW_SPI1_PORT_D3
            TCFG_HW_SPI1_PORT_D3, //d3 any io
#endif
            0xff, //cs any io(主机不操作cs)
        },
        .role = TCFG_HW_SPI1_ROLE,//SPI_ROLE_MASTER,
        .clk  = TCFG_HW_SPI1_BAUD,
        .mode = TCFG_HW_SPI1_MODE,//SPI_MODE_BIDIR_1BIT,//SPI_MODE_UNIDIR_2BIT,
        .bit_mode = SPI_FIRST_BIT_MSB,
        .cpol = 0,//clk level in idle state:0:low,  1:high
        .cpha = 0,//sampling edge:0:first,  1:second
        .ie_en = 0, //ie enbale:0:disable,  1:enable
        .irq_priority = 3,
        .spi_isr_callback = NULL,  //spi isr callback
    },
#if SUPPORT_SPI2
    {
        //spi2
        .port = {
            TCFG_HW_SPI2_PORT_CLK, //clk any io
            TCFG_HW_SPI2_PORT_DO, //do any io
            TCFG_HW_SPI2_PORT_DI, //di any io
#ifdef  TCFG_HW_SPI2_PORT_D2
            TCFG_HW_SPI2_PORT_D2, //d2 any io
#endif
#ifdef  TCFG_HW_SPI2_PORT_D3
            TCFG_HW_SPI2_PORT_D3, //d3 any io
#endif
            0xff, //cs any io(主机不操作cs)
        },
        .role = TCFG_HW_SPI2_ROLE,//SPI_ROLE_MASTER,
        .clk  = TCFG_HW_SPI2_BAUD,
        .mode = TCFG_HW_SPI2_MODE,//SPI_MODE_BIDIR_1BIT,//SPI_MODE_UNIDIR_2BIT,
        .bit_mode = SPI_FIRST_BIT_MSB,
        .cpol = 0,//clk level in idle state:0:low,  1:high
        .cpha = 0,//sampling edge:0:first,  1:second
        .ie_en = 0, //ie enbale:0:disable,  1:enable
        .irq_priority = 3,
        .spi_isr_callback = NULL,  //spi isr callback
    },
#endif
};
#endif


#if TCFG_NANDFLASH_DEV_ENABLE
#include "nandflash.h"
NANDFLASH_DEV_PLATFORM_DATA_BEGIN(nandflash_dev_data) = {
    .spi_hw_num     = TCFG_FLASH_DEV_SPI_HW_NUM,
    .spi_cs_port    = TCFG_FLASH_DEV_SPI_CS_PORT,
    .spi_read_width = TCFG_FLASH_DEV_FLASH_READ_WIDTH,//flash读数据的线宽
    .start_addr     = 0,
    .size           = 128 * 1024 * 1024,
#if (TCFG_FLASH_DEV_SPI_HW_NUM == 1)
    .spi_pdata      = &spix_p_data[1],
#elif (TCFG_FLASH_DEV_SPI_HW_NUM == 2)
    .spi_pdata      = &spix_p_data[2],
#endif
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

#if TCFG_APP_RTC_EN
    { "rtc",   &rtc_dev_ops, (void *) &rtc_data},
#endif

#if TCFG_NANDFLASH_DEV_ENABLE
    {"nand_flash",   &nandflash_dev_ops, (void *) &nandflash_dev_data},
    {"nandflash_ftl",   &ftl_dev_ops, NULL },
#endif

};
