#ifndef _NANDFLASH_H
#define _NANDFLASH_H

#include "device/device.h"
#include "ioctl_cmds.h"
#include "spi.h"
#include "printf.h"
#include "gpio.h"
#include "device_drive.h"
#include "malloc.h"

//*********************************************************************************//
//                              SPI NAND command macros                            //
//*********************************************************************************//
#define GD_WRITE_ENABLE             0x06
#define GD_WRITE_DISABLE            0x04
#define GD_BLOCK_ERASE              0xD8
#define GD_READ_ID                  0x9F
#define GD_READ_UID                 0x4B
#define GD_RESET_DEVICE             0xFF
/* feature register operations */
#define GD_GET_FEATURES             0x0F
#define GD_SET_FEATURES             0x1F
#define GD_FEATURES_PROTECT         0xA0
#define GD_FEATURES                 0xB0
#define GD_GET_STATUS               0xC0
#define GD_DRIVE_STRENGTH           0xD0
/* program operations */
#define GD_PROGRAM_LOAD             0x02
#define GD_PROGRAM_EXECUTE          0x10
#define GD_PROGRAM_LOAD_RANDOM_DATA         0x84
#define GD_PROGRAM_LOAD_X4                  0x32
#define GD_PROGRAM_LOAD_RANDOM_DATA_X4      0x34
#define GD_PROGRAM_LOAD_RANDOM_DATA_QUAD_IO 0x72
/* read operations */
#define GD_PAGE_READ_CACHE          0x13
#define GD_READ_FROM_CACHE          0x03
#define GD_READ_PAGE_CACHE_RANDOM   0x30
#define GD_READ_PAGE_CACHE_LAST     0x3f
#define GD_READ_FROM_CACHE_X2       0x3B
#define GD_READ_FROM_CACHE_DUAL_IO  0xBB
#define GD_READ_FROM_CACHE_X4       0x6B
#define GD_READ_FROM_CACHE_QUAD_IO  0xEB
/* protect */
#define GD_PERMANENT_BLOCK_LOCK_PROTECTION  0x2c

//*********************************************************************************//
//                    Feature / Status register bit definitions                    //
//*********************************************************************************//
#define NAND_OTP_PRT                BIT(7)
#define NAND_OTP_EN                 BIT(6)
#define NAND_ECC_EN                 BIT(4)
#define NAND_X4_EN                  BIT(0)

#define NAND_STATUS_OIP             BIT(0)
#define NAND_STATUS_WEL             BIT(1)
#define NAND_STATUS_E_FAIL          BIT(2)
#define NAND_STATUS_P_FAIL          BIT(3)
#define NAND_STATUS_ECCS            (0xf0)

typedef enum {
    NAND_SUCCESS = 0,
    NAND_ECC_CORRECTED = 1,
    NAND_ERROR_ECC = 2,
    NAND_ERROR_P_FAIL = 3,
    NAND_ERROR_E_FAIL = 4,
    NAND_ERROR_TIME_OUT = 5,
    NAND_ERROR_ADDR = 6,
    NAND_ERROR_EINVAL = 22, /* Invalid argument */
} NAND_Result;

struct nandflash_dev_platform_data {
    int spi_hw_num;         // SPI1 or SPI2 only
    u32 spi_cs_port;        // CS pin
    u32 spi_read_width;     // flash read data width
    const struct spi_platform_data *spi_pdata;
    u32 start_addr;         // partition start address
    u32 size;               // partition size; ignored when single partition
};

#define NANDFLASH_DEV_PLATFORM_DATA_BEGIN(data) \
    const struct nandflash_dev_platform_data data

#define NANDFLASH_DEV_PLATFORM_DATA_END()  \
};

struct nandflash_data {
    u8 ecc_mask;
    u8 ecc_err;
    u8 plane_select;
    u8 write_enable_position;
    u8 quad_mode_dummy_num: 4;
    u8 quad_mode_qe: 1;
    u16 block_number;
    u32 capacity;
    u16 page_size;
    u32 block_size;
};

extern struct nandflash_data nand_flash;

int nand_page_internal_data_move(u32 page_src_addr, u32 page_dest_addr, u16 offset, u16 len, u8 *buf);
extern const struct device_operations nandflash_dev_ops;
extern const struct device_operations ftl_dev_ops;

// NAND bare driver API (matches nandflash.c implementation signatures)
int nand_flash_write_page(u16 block, u8 page, u8 *buf, u16 len);
int nand_flash_read_page(u16 block, u8 page, u16 offset, u8 *buf, u16 len);
int nand_flash_erase(u32 addr);

u16 nandflash_get_flash_id(void);

#endif
