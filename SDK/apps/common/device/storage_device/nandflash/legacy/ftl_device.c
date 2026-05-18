// ⚠️ 此文件仅用于 2KB page NAND + 闭源 ftl.a 的适配场景。
// 当前工程使用 nandflash_ftl.c（开源 FTL，支持 4KB page）替代此文件。
// 此文件已从 genFileList.c 中移除编译，不再参与构建。
// 如需切换回闭源 FTL：
//   1. 在 genFileList.c 中编译此文件
//   2. 在 Makefile.mk 中链接 ftl.a
//   3. 从 git 历史恢复 nandflash_get_ftl_info() 到 nandflash.c（已从此分支移除）
// 若确认不再需要，可直接删除此文件和本 legacy/ 目录。
#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ftl_device.data.bss")
#pragma data_seg(".ftl_device.data")
#pragma const_seg(".ftl_device.text.const")
#pragma code_seg(".ftl_device.text")
#endif
#include "device/ioctl_cmds.h"
#include "device/device.h"
#include "nandflash.h"
#include "ftl_api.h"
#include "app_config.h"
#include "asm/wdt.h"

#if TCFG_NANDFLASH_DEV_ENABLE

#undef LOG_TAG_CONST
#define LOG_TAG     "[FTL_DEV]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
#include "debug.h"

static u32 g_capacity = 0;
static struct device ftl_device;

static u32 get_first_one(u32 n)
{
    u32 pos = 0;
    for (pos = 0; pos < 32; pos++) {
        if (n & BIT(pos)) {
            return pos;
        }
    }
    return 0xff;
}

static enum ftl_error_t ftl_port_page_read(u16 block, u8 page, u16 offset, u8 *buf, int len)
{
    int ret = nand_flash_read_page(block, page, offset, buf, len);
    if (ret == 0) {
        return FTL_ERR_NO;
    }
    if (ret == 1) {
        return FTL_ERR_1BIT_ECC;
    }
    return FTL_ERR_READ;
}

static enum ftl_error_t ftl_port_page_write(u16 block, u8 page, u8 *buf, int len)
{
    int ret = nand_flash_write_page(block, page, buf, len);
    if (ret == 0) {
        return FTL_ERR_NO;
    }
    if (ret == 1) {
        return FTL_ERR_1BIT_ECC;
    }
    return FTL_ERR_WRITE;
}

static enum ftl_error_t ftl_port_erase_block(u32 addr)
{
    int ret = nand_flash_erase(addr);
    if (ret == 0) {
        return FTL_ERR_NO;
    }
    return FTL_ERR_WRITE;
}

static int ftl_dev_init(const struct dev_node *node, void *arg)
{
    return 0;
}

static int ftl_dev_open(const char *name, struct device **device, void *arg)
{
    int ret = 0;
    dev_open("nand_flash", NULL);

    struct ftl_nand_flash flash;
    nandflash_get_ftl_info(&flash);
    flash.page_size_shift = get_first_one(flash.page_size);
    flash.block_size_shift = get_first_one(flash.page_size * flash.page_num);
    flash.page_read = ftl_port_page_read;
    flash.page_write = ftl_port_page_write;
    flash.erase_block = ftl_port_erase_block;

    g_capacity = flash.logic_block_num << (flash.block_size_shift - 9);

    struct ftl_config config = {
        .page_buf_num = 1,
        .delayed_write_msec = 0,
    };
    ret = ftl_init(&flash, &config);
    if (ret) {
        log_error("ftl_init failed: %d", ret);
        return ret;
    }

    // check if FTL needs first-time format
    {
        u8 marker[4];
        u32 marker_addr = (g_capacity - 1) * 512;
        enum ftl_error_t err;
        if (ftl_read_bytes(marker_addr, marker, 4, &err) < 0
            || marker[0] != 'F' || marker[1] != 'T'
            || marker[2] != 'L' || marker[3] != '!') {
            log_info("ftl first format start");
            wdt_disable();
            ret = ftl_format();
            if (ret) {
                log_error("ftl_format failed: %d", ret);
            } else {
                ftl_uninit();
                ret = ftl_init(&flash, &config);
                if (ret == 0) {
                    u8 magic[4] = {'F', 'T', 'L', '!'};
                    ftl_write_bytes(marker_addr, magic, 4, &err);
                }
            }
            wdt_enable();
            log_info("ftl first format ret: %d", ret);
        }
    }

    *device = &ftl_device;
    return ret;
}

static int ftl_dev_close(struct device *device)
{
    ftl_uninit();
    return 0;
}

static int ftl_dev_read(struct device *device, void *buf, u32 len, u32 offset)
{
    enum ftl_error_t error;
    int rlen = ftl_read_bytes(offset * 512, buf, len * 512, &error);
    if (rlen < 0) {
        return 0;
    }
    return rlen / 512;
}

static int ftl_dev_write(struct device *device, void *buf, u32 len, u32 offset)
{
    enum ftl_error_t error;
    int wlen = ftl_write_bytes(offset * 512, buf, len * 512, &error);
    if (wlen < 0) {
        return 0;
    }
    return wlen / 512;
}

static bool ftl_dev_online(const struct dev_node *node)
{
    return 1;
}

static int ftl_dev_ioctl(struct device *device, u32 cmd, u32 arg)
{
    int reg = 0;

    switch (cmd) {
    case IOCTL_GET_STATUS:
        *(u32 *)arg = 1;
        break;
    case IOCTL_GET_ID:
        *((u32 *)arg) = nandflash_get_flash_id();
        break;
    case IOCTL_GET_BLOCK_SIZE:
        *(u32 *)arg = 512;//usb fat
        break;
    case IOCTL_ERASE_BLOCK:
        break;
    case IOCTL_GET_CAPACITY:
        *(u32 *)arg = g_capacity;
        break;
    case IOCTL_FLUSH:
        break;
    case IOCTL_CMD_RESUME:
        break;
    case IOCTL_CMD_SUSPEND:
        break;
    case IOCTL_CHECK_WRITE_PROTECT:
        *(u32 *)arg = 0;
        break;
    default:
        reg = -EINVAL;
        break;
    }
    return reg;
}

const struct device_operations ftl_dev_ops = {
    .init   = ftl_dev_init,
    .online = ftl_dev_online,
    .open   = ftl_dev_open,
    .read   = ftl_dev_read,
    .write  = ftl_dev_write,
    .ioctl  = ftl_dev_ioctl,
    .close  = ftl_dev_close,
};

#endif

