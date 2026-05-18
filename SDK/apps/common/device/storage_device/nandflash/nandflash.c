#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".nandflash.data.bss")
#pragma data_seg(".nandflash.data")
#pragma const_seg(".nandflash.text.const")
#pragma code_seg(".nandflash.text")
#endif
#include "nandflash.h"
#include "nandflash_test.h"
#include "app_config.h"
#include "clock.h"
#include "asm/wdt.h"

#if TCFG_NANDFLASH_DEV_ENABLE

#undef LOG_TAG_CONST
#define LOG_TAG     "[nandFLASH]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
#include "debug.h"

#define NAND_FLASH_TIMEOUT			1000000
#define MAX_NANDFLASH_PART_NUM      4

#define XT26G01C                    0x0b11
#define XT26G02C                    0x2c24
#define XCSP4AAWH                   0x9cb1
#define F35SQA002G                  0xcd72
#define F35SQA001G                  0xcd71
#define F35SQA512M                  0xcd70
#define ZB35Q01C                    0x5e41
#define GD5F4GM7UE                  0xc894


struct nandflash_data nand_flash = {0};
static u8 spi_data_width = SPI_MODE_BIDIR_1BIT;

struct nandflash_partition {
    const char *name;
    u32 start_addr;
    u32 size;
    struct device device;
};

static struct nandflash_partition nor_part[MAX_NANDFLASH_PART_NUM];

struct nandflash_info {
    u32 flash_id;
    int spi_num;
    int spi_err;
    u8 spi_cs_io;
    u8 spi_r_width;
    u8 part_num;
    u8 open_cnt;
    struct nandflash_partition *const part_list;
    OS_MUTEX mutex;
    u32 max_end_addr;
};

static struct nandflash_info _nandflash = {
    .spi_num = (int) - 1,
    .part_list = nor_part,
};

int nand_flash_read(u32 addr, u8 *buf, u32 len);
int nand_flash_erase(u32 addr);


#define spi_cs_init() \
    do { \
        gpio_set_mode(IO_PORT_SPILT(_nandflash.spi_cs_io), PORT_OUTPUT_HIGH); \
    } while (0)

#define spi_cs_uninit() \
    do { \
        gpio_set_mode(IO_PORT_SPILT(_nandflash.spi_cs_io), PORT_HIGHZ); \
    } while (0)
#define spi_cs_h()                  gpio_write(_nandflash.spi_cs_io, 1)
#define spi_cs_l()                  gpio_write(_nandflash.spi_cs_io, 0)
#define spi_read_byte()             spi_recv_byte(_nandflash.spi_num, &_nandflash.spi_err)
#define spi_write_byte(x)           spi_send_byte(_nandflash.spi_num, x)
#define spi_dma_read(x, y)          spi_dma_recv(_nandflash.spi_num, x, y)
#define spi_dma_write(x, y)         spi_dma_send(_nandflash.spi_num, x, y)
#define spi_set_width(x)            spi_set_bit_mode(_nandflash.spi_num, x)

static struct nandflash_partition *nandflash_find_part(const char *name)
{
    struct nandflash_partition *part = NULL;
    u32 idx;
    for (idx = 0; idx < MAX_NANDFLASH_PART_NUM; idx++) {
        part = &_nandflash.part_list[idx];
        if (part->name == NULL) {
            continue;
        }
        if (!strcmp(part->name, name)) {
            return part;
        }
    }
    return NULL;
}

static struct nandflash_partition *nandflash_new_part(const char *name, u32 addr, u32 size)
{
    struct nandflash_partition *part;
    u32 idx;
    for (idx = 0; idx < MAX_NANDFLASH_PART_NUM; idx++) {
        part = &_nandflash.part_list[idx];
        if (part->name == NULL) {
            break;
        }
    }
    if (part->name != NULL) {
        log_error("create nandflash part fail\n");
        return NULL;
    }
    memset(part, 0, sizeof(*part));
    part->name = name;
    part->start_addr = addr;
    part->size = size;
    if (part->start_addr + part->size > _nandflash.max_end_addr) {
        _nandflash.max_end_addr = part->start_addr + part->size;
    }
    _nandflash.part_num++;
    return part;
}

static void nandflash_delete_part(const char *name)
{
    struct nandflash_partition *part;
    u32 idx;
    for (idx = 0; idx < MAX_NANDFLASH_PART_NUM; idx++) {
        part = &_nandflash.part_list[idx];
        if (part->name == NULL) {
            continue;
        }
        if (!strcmp(part->name, name)) {
            part->name = NULL;
            _nandflash.part_num--;
        }
    }
}

static int nandflash_verify_part(struct nandflash_partition *p)
{
    struct nandflash_partition *part = NULL;
    u32 idx;
    for (idx = 0; idx < MAX_NANDFLASH_PART_NUM; idx++) {
        part = &_nandflash.part_list[idx];
        if (part->name == NULL) {
            continue;
        }
        if ((p->start_addr >= part->start_addr) && (p->start_addr < part->start_addr + part->size)) {
            if (strcmp(p->name, part->name) != 0) {
                return -1;
            }
        }
    }
    return 0;
}

u16 nand_flash_read_id()
{
    u16 id;
    os_time_dly(1);
    spi_cs_l();
    spi_write_byte(GD_READ_ID);
    spi_write_byte(0x0);
    id = spi_read_byte();
    id <<= 8;
    id |= spi_read_byte();
    spi_cs_h();
    return id;
}

static void nand_write_enable()
{
    spi_cs_l();
    spi_write_byte(GD_WRITE_ENABLE);
    spi_cs_h();
}

static void nand_write_disable()
{
    spi_cs_l();
    spi_write_byte(GD_WRITE_DISABLE);
    spi_cs_h();
}

static void nand_set_features(u8 addr, u8 dat)
{
    spi_cs_l();
    spi_write_byte(GD_SET_FEATURES); //命令头
    spi_write_byte(addr);             //发送protection寄存器地址
    spi_write_byte(dat);             //需要设置的数据
    spi_cs_h();
}

static u8 nand_get_features(u8 addr)
{
    u8 temp;
    spi_cs_l();
    spi_write_byte(GD_GET_FEATURES); //命令头
    spi_write_byte(addr);             //发送protection寄存器地址
    temp = spi_read_byte();
    spi_cs_h();
    return temp;
}
void nand_flash_set_quad(u8 en)
{
    u8 cfg_reg = nand_get_features(GD_FEATURES);
    if ((!!(cfg_reg & BIT(0))) == en) {
        log_debug("nandflash_set_quad succ\n");
        return;
    }
    if (en) {
        cfg_reg |= BIT(0);
    } else {
        cfg_reg &= ~ BIT(0);
    }
    nand_set_features(GD_FEATURES, cfg_reg);
}

enum {
    NANDFLASH_OIP_FAIL = 1,
    NANDFLASH_E_FAIL,
    NANDFLASH_P_FAIL,
    NANDFLASH_ECC_FAIL,
    NANDFLASH_BAD_BLOCK,

};
static u8 nand_flash_wait_ok(u32 timeout)
{
    udelay(1);
    u8 sta;
    do {
        sta = nand_get_features(GD_GET_STATUS);
        if ((sta & NAND_STATUS_OIP) == 0) {
            sta = 0;
            break;
        }
//It will also be set if the user attempts to program an invalid
//address or a locked or a protected region, including the OTP area.
        if (sta & NAND_STATUS_P_FAIL) { //ing
            log_error("nand_flash prom error![status:0x%x,timeout:%d]\r\n", sta, timeout);
            sta = NANDFLASH_P_FAIL;
            goto _exit;
        }
//It will also be set if the user attempts to erase a locked region.
        if (sta & NAND_STATUS_E_FAIL) { //erase fail
            log_error("nand_flash erase error![status:0x%x,timeout:%d]\r\n", sta, timeout);
            sta = NANDFLASH_E_FAIL;
            goto _exit;
        }
        if ((sta & nand_flash.ecc_mask) == nand_flash.ecc_err) {
            log_error("nand_flash block should be marked as a bad block![status:0x%x,timeout:%d]\r\n", sta, timeout);
            sta = NANDFLASH_BAD_BLOCK;//坏块
            break;
        } else if (sta & nand_flash.ecc_mask) {
            log_error("nand_flash block data should be refreshed![status:0x%x,timeout:%d]\r\n", sta, timeout);
            sta = NANDFLASH_ECC_FAIL;//ecc错误
            /* break; */
        }

        udelay(1);
    } while (--timeout);
    if (timeout == 0) {
        log_error("nand_flash_wait_ok timeout!\r\n");
        sta = NANDFLASH_OIP_FAIL;
    }
_exit:
    return sta;
}


void nand_flash_reset(void)
{
    os_time_dly(2);
    spi_cs_l();
    spi_write_byte(0xff); //命令头
    spi_cs_h();
    os_time_dly(2);
// 4Gb:must wait Tpor,2Gb:no wait
    os_time_dly(2);
// wait ok
    if (nand_flash_wait_ok(NAND_FLASH_TIMEOUT)) {
        log_error("nand flash reset error");
    }
}

static u32 block_page_get(u32 addr, u32 *cache_addr)
{
    u32  page = 0;
    u32 block = 0, bp_mix = 0;

    //<地址超1页范围
    if (addr >= nand_flash.page_size) {
        if (addr >= nand_flash.block_size) {
            while (addr >= nand_flash.block_size) {
                block++;
                addr -= nand_flash.block_size;
            }
            goto _page_count;
        } else {
            goto _page_count;
        }
    }
    //<地址不超1页范围
    else {
        goto _end_count;
    }

_page_count:
    while (addr >= nand_flash.page_size) {
        page++;
        addr -= nand_flash.page_size;
    }

_end_count:
    *cache_addr = addr;
    bp_mix = (block << 6) | page;
    /* log_info("addr change:block:%d,page:%d,cache:%d", block, page, addr); */
    return bp_mix;
}


int nand_flash_erase(u32 address)
{
    u32 bp_mix = 0;
    u32 cache_addr;
    bp_mix = block_page_get(address, &cache_addr);
    nand_write_enable();

    spi_cs_l();
    spi_write_byte(GD_BLOCK_ERASE);
    spi_write_byte(bp_mix >> 16); //擦除地址:block+page addr
    spi_write_byte(bp_mix >> 8);//
    spi_write_byte(bp_mix);
    spi_cs_h();
    /* nand_write_disable(); */

    u8 sta = nand_flash_wait_ok(NAND_FLASH_TIMEOUT);
    if (sta) {
        log_error("nand flash erase error status reg %x", sta);
    }
    return sta;
}

static void nand_flash_erase_all()
{
    nand_set_features(0xA0, 0x00);
    u16 block_num = nand_flash.block_number;
    for (int i = 0; i < block_num; i++) {
        nand_flash_erase(nand_flash.block_size * i);
        wdt_clear();
    }
    log_info("nandflash erase all!!!");
}

//return:0:ok, >0:err, <0:bad block(-1表示0,-2:1,,,)
static int nand_page_read_to_cache(u32 block_page_addr) //PageRead
{
    u8 reg = 0;
    int err_b_addr;

    spi_cs_l();
    spi_write_byte(GD_PAGE_READ_CACHE);         //PageRead to cache
    spi_write_byte(block_page_addr >> 16);  //send the Page/Block Add
    spi_write_byte((u8)((block_page_addr) >> 8));
    spi_write_byte((u8)block_page_addr);
    spi_cs_h();
    reg = nand_flash_wait_ok(NAND_FLASH_TIMEOUT);
    if (reg) {
        log_error("[addr:block:%d,page:%d]nand flash read to cache error", block_page_addr >> 6, block_page_addr & 0x3f);
        if (reg == NANDFLASH_BAD_BLOCK) {
            /* log_error("nand_flash block should be marked as a bad block!\r\n"); */
            err_b_addr = block_page_addr >> 6;
            err_b_addr = -err_b_addr - 1;
            return err_b_addr;
        }
    }
    return reg;
}

//03h/0bh/3bh/6bh/bbh/ebh:addr_bit12:sel plane
static void nand_read_from_cache(u8 *buf, u32 cache_addr, u32 len) //ReadCache
{
    /* cache_addr |= 0x4000; */
    spi_cs_l();//SPI_CS(0);
    spi_write_byte(GD_READ_FROM_CACHE);     //PageRead to cache
    spi_write_byte((u8)((cache_addr) >> 8));
    spi_write_byte((u8)cache_addr);
    spi_write_byte(0xFF);  //send 1byte dummy clk
    spi_dma_read(buf, len);
    spi_cs_h();//SPI_CS(1);
}
//3bh/6bh/bbh/ebh:addr_bit12:sel plane
static void nand_read_from_cache_x2(u8 *buf, u32 cache_addr, u32 len) //ReadCache
{
    /* cache_addr |= 0x4000; */
    spi_cs_l();//SPI_CS(0);
    spi_write_byte(GD_READ_FROM_CACHE_X2);     //PageRead to cache
    spi_write_byte((u8)((cache_addr) >> 8));
    spi_write_byte((u8)cache_addr);
    spi_write_byte(0xff);
    spi_set_width(SPI_MODE_UNIDIR_2BIT);
    spi_dma_read(buf, len);
    spi_set_width(spi_data_width);
    spi_cs_h();//SPI_CS(1);
}
//03h/0bh/3bh/6bh/bbh/ebh:addr_bit12:sel plane
static void nand_read_from_cache_dual_io(u8 *buf, u32 cache_addr, u32 len) //ReadCache
{
    /* cache_addr |= 0x4000; */
    spi_cs_l();//SPI_CS(0);
    spi_write_byte(GD_READ_FROM_CACHE_DUAL_IO);     //PageRead to cache

    spi_set_width(SPI_MODE_UNIDIR_2BIT);
    spi_write_byte((u8)((cache_addr) >> 8));
    spi_write_byte((u8)cache_addr);
    spi_write_byte(0xff);
    spi_dma_read(buf, len);
    spi_set_width(spi_data_width);

    spi_cs_h();//SPI_CS(1);
}
//3bh/6bh/bbh/ebh:addr_bit12:sel plane
static void nand_read_from_cache_x4(u8 *buf, u32 cache_addr, u32 len) //ReadCache
{
    if (nand_flash.quad_mode_qe) {
        nand_flash_set_quad(1);
    }
    /* cache_addr |= 0x4000; */
    spi_cs_l();//SPI_CS(0);
    spi_write_byte(GD_READ_FROM_CACHE_X4);     //PageRead to cache
    spi_write_byte((u8)((cache_addr) >> 8));
    spi_write_byte((u8)cache_addr);
    spi_write_byte(0xff);
    spi_set_width(SPI_MODE_UNIDIR_4BIT);
    spi_dma_read(buf, len);
    spi_set_width(spi_data_width);

    spi_cs_h();//SPI_CS(1);
    if (nand_flash.quad_mode_qe) {
        nand_flash_set_quad(0);
    }
}

//03h/0bh/3bh/6bh/bbh/ebh:addr_bit12:sel plane
static void nand_read_from_cache_quad_io(u8 *buf, u32 cache_addr, u32 len) //ReadCache
{
    if (nand_flash.quad_mode_qe) {
        nand_flash_set_quad(1);
    }
    u8 temp[4];
    temp[0] = (u8)((cache_addr) >> 8);
    temp[1] = (u8)cache_addr;
    temp[2] = 0xff;
    temp[3] = 0xff;
    /* cache_addr |= 0x4000; */
    spi_cs_l();//SPI_CS(0);
    spi_write_byte(GD_READ_FROM_CACHE_QUAD_IO);     //PageRead to cache

    spi_set_width(SPI_MODE_UNIDIR_4BIT);
    spi_dma_write(temp, 2 + nand_flash.quad_mode_dummy_num);
    spi_dma_read(buf, len);
    spi_set_width(spi_data_width);

    spi_cs_h();//SPI_CS(1);
    if (nand_flash.quad_mode_qe) {
        nand_flash_set_quad(0);
    }
}
//return:0:ok, >0:err, <0:bad block(-1表示0,-2:1,,,)
static int nand_read(u32 addr, u32 len, u8 *buf)
{
    int reg = 0;
    u32 bp_mix = 0;
    u32 cache_addr;
    bp_mix = block_page_get(addr, &cache_addr);
    cache_addr &= 0x0fff;
    if (nand_flash.plane_select == 1) { //XT26G02C(2Gb)
        if (bp_mix & BIT(6)) {
            cache_addr |= BIT(12);
        }
    }
    reg = nand_page_read_to_cache(bp_mix);
    if (reg < 0) {
        return reg;
    }
    //时间间隔需小于trd
    if (_nandflash.spi_r_width == SPI_MODE_UNIDIR_2BIT) {
        /* nand_read_from_cache_dual_io(buf, cache_addr, len); */
        nand_read_from_cache_x2(buf, cache_addr, len);
    } else if (_nandflash.spi_r_width == SPI_MODE_UNIDIR_4BIT) {
        /* nand_read_from_cache_quad_io(buf, cache_addr, len); */
        nand_read_from_cache_x4(buf, cache_addr, len);
    } else {
        nand_read_from_cache(buf, cache_addr, len);
    }
    return reg;
}


int nand_flash_read_page(u16 block, u8 page, u16 offset, u8 *buf, u16 len)
{
    os_mutex_pend(&_nandflash.mutex, 0);

    int reg = 0;
    u32 bp_mix = 0;
    u32 cache_addr = offset;
    bp_mix = (block << 6) | page;

    cache_addr &= 0x0fff;
    if (nand_flash.plane_select == 1) { //XT26G02C(2Gb)
        if (bp_mix & BIT(6)) {
            cache_addr |= BIT(12);
        }
    }
    reg = nand_page_read_to_cache(bp_mix);
    if (reg < 0) {
        os_mutex_post(&_nandflash.mutex);
        return reg;
    }
    //时间间隔需小于trd
    if (_nandflash.spi_r_width == SPI_MODE_UNIDIR_2BIT) {
        /* nand_read_from_cache_dual_io(buf, cache_addr, len); */
        nand_read_from_cache_x2(buf, cache_addr, len);
    } else if (_nandflash.spi_r_width == SPI_MODE_UNIDIR_4BIT) {
        /* nand_read_from_cache_quad_io(buf, cache_addr, len); */
        nand_read_from_cache_x4(buf, cache_addr, len);
    } else {
        nand_read_from_cache(buf, cache_addr, len);
    }
    os_mutex_post(&_nandflash.mutex);
    return reg;
}

//简化接口
//return:0:ok, >0:err, <0:bad block(-1表示0,-2:1,,,)
int nand_flash_read(u32 offset, u8 *buf,  u32 len)
{
    int reg = 0;
    int _len = len;
    u8 *_buf = (u8 *)buf;
    os_mutex_pend(&_nandflash.mutex, 0);
    u32 first_page_len = nand_flash.page_size - (offset % nand_flash.page_size);
    first_page_len = _len > first_page_len ? first_page_len : _len;
    reg = nand_read(offset, first_page_len, _buf);
    if (reg) {
        log_error("read nandflash addr:%d fail!", offset);
        goto __read_exit;
    }
    _len -= first_page_len;
    _buf += first_page_len;
    offset += first_page_len;
    while (_len) {
        u32 cnt = _len > nand_flash.page_size ? nand_flash.page_size : _len;
        reg = nand_read(offset, cnt, _buf);
        if (reg) {
            log_error("read nandflash addr:%d fail!", offset);
            goto __read_exit;
        }
        _len -= cnt;
        offset += cnt;
        _buf += cnt;
    }
__read_exit:
    os_mutex_post(&_nandflash.mutex);
    return reg;
}


//write:
static void nand_program_load(u8 *buf, u32 cache_addr, u16 len)
{
    spi_cs_l();//SPI_CS(0);
    spi_write_byte(GD_PROGRAM_LOAD);         //PageLoad to cache ,change the data
    spi_write_byte((u8)((cache_addr) >> 8));
    spi_write_byte((u8)cache_addr);
    spi_dma_write(buf, len); //将数据放到总线上
    spi_cs_h();//SPI_CS(1);
}
//02h/32h/84h/34h:addr_bit12:sel plane
static void nand_program_load_x4(u8 *buf, u32 cache_addr, u16 len)
{
    if (nand_flash.quad_mode_qe) {
        nand_flash_set_quad(1);
    }
    spi_cs_l();//SPI_CS(0);
    spi_write_byte(GD_PROGRAM_LOAD_X4);         //PageLoad to cache ,change the data
    spi_write_byte((u8)((cache_addr) >> 8));
    spi_write_byte((u8)cache_addr);

    spi_set_width(SPI_MODE_UNIDIR_4BIT);
    spi_dma_write(buf, len); //将数据放到总线上
    spi_set_width(spi_data_width);

    spi_cs_h();//SPI_CS(1);
    if (nand_flash.quad_mode_qe) {
        nand_flash_set_quad(0);
    }
}

static int nand_program_excute(u32 block_page_addr)
{
    int reg;
    spi_cs_l();
    spi_write_byte(GD_PROGRAM_EXECUTE);
    spi_write_byte(block_page_addr >> 16);  //send the Page/Block Add
    spi_write_byte((u8)((block_page_addr) >> 8));
    spi_write_byte((u8)block_page_addr);
    spi_cs_h();
    reg = nand_flash_wait_ok(NAND_FLASH_TIMEOUT);
    if (reg) {
        log_error("nand flash program error");
    }
    return reg;
}


static int nand_write(u32 addr, u16 len, u8 *buf)
{
    int reg;
    u32 bp_mix = 0;
    u32 cache_addr;

    bp_mix = block_page_get(addr, &cache_addr);
    cache_addr &= 0x0fff;
    if (nand_flash.plane_select == 1) { //XT26G02C(2Gb)
        if (bp_mix & BIT(6)) {
            cache_addr |= BIT(12);
        }
    }
    if (nand_flash.write_enable_position == 1) {
        nand_write_enable();
    }

    if (_nandflash.spi_r_width == SPI_MODE_UNIDIR_4BIT) {
        nand_program_load_x4(buf, cache_addr, len);
    } else {
        nand_program_load(buf, cache_addr, len);
    }
    if (nand_flash.write_enable_position == 0) { //XT26G01C(1Gb)
        nand_write_enable();
    }
    reg = nand_program_excute(bp_mix);
    return reg;
}/**/

int nand_flash_write_page(u16 block, u8 page, u8 *buf, u16 len)
{
    os_mutex_pend(&_nandflash.mutex, 0);
    int reg;
    u32 bp_mix = 0;
    u32 cache_addr;

    bp_mix = (block << 6) | page;
    cache_addr = 0;
    if (nand_flash.plane_select == 1) { //XT26G02C(2Gb)
        if (bp_mix & BIT(6)) {
            cache_addr |= BIT(12);
        }
    }
    if (nand_flash.write_enable_position == 1) {
        nand_write_enable();
    }

    if (_nandflash.spi_r_width == SPI_MODE_UNIDIR_4BIT) {
        nand_program_load_x4(buf, cache_addr, len);
    } else {
        nand_program_load(buf, cache_addr, len);
    }
    if (nand_flash.write_enable_position == 0) { //XT26G01C(1Gb)
        nand_write_enable();
    }
    reg = nand_program_excute(bp_mix);
    os_mutex_post(&_nandflash.mutex);
    return reg;
}


//写前需确保该block(128k)已擦除
int nand_flash_write(u32 offset, u8 *buf,  u32 len)
{
    int reg;
    int _len = len;
    u8 *_buf = (u8 *)buf;
    os_mutex_pend(&_nandflash.mutex, 0);
    u32 first_page_len = nand_flash.page_size - (offset % nand_flash.page_size);
    first_page_len = _len > first_page_len ? first_page_len : _len;
    reg = nand_write(offset, first_page_len, _buf);
    if (reg) {
        goto __exit;
    }
    _len -= first_page_len;
    _buf += first_page_len;
    offset += first_page_len;
    while (_len) {
        u32 cnt = _len > nand_flash.page_size ? nand_flash.page_size : _len;
        reg = nand_write(offset, cnt, _buf);
        if (reg) {
            goto __exit;
        }
        _len -= cnt;
        offset += cnt;
        _buf += cnt;
    }
__exit:
    os_mutex_post(&_nandflash.mutex);
    return reg;
}

//1Gb 2Gb program 时序不同
// If the random data is not sequential, then another PROGRAM LOAD RANDOM DATA x1 (84h) command must be issued with a new column address.*******************
//02h/32h/84h/34h:addr_bit12:sel plane
static void nand_program_load_random_data(u8 *buf, u16 cache_addr, u16 len)
{
    spi_cs_l();//SPI_CS(0);
    spi_write_byte(GD_PROGRAM_LOAD_RANDOM_DATA);
    spi_write_byte((u8)((cache_addr) >> 8));
    spi_write_byte((u8)cache_addr);
    if (buf && (len > 0)) {

        spi_dma_write(buf, len); //将数据放到总线上
    }
    spi_cs_h();//SPI_CS(1);
}
//02h/32h/84h/34h:addr_bit12:sel plane
static void nand_program_load_random_data_x4(u8 *buf, u16 cache_addr, u16 len)
{
    if (nand_flash.quad_mode_qe) {
        nand_flash_set_quad(1);
    }
    spi_cs_l();//SPI_CS(0);
    spi_write_byte(GD_PROGRAM_LOAD_RANDOM_DATA_X4);
    spi_write_byte((u8)((cache_addr) >> 8));
    spi_write_byte((u8)cache_addr);
    if (buf && (len > 0)) {
        spi_set_width(SPI_MODE_UNIDIR_4BIT);
        spi_dma_write(buf, len); //将数据放到总线上
        spi_set_width(spi_data_width);
    }
    spi_cs_h();//SPI_CS(1);
    if (nand_flash.quad_mode_qe) {
        nand_flash_set_quad(0);
    }
}

/*
 * 块间或块内页数据移动, 可替换offset位置len长的数据
 * page_src_addr :要移动的页
 * page_dest_addr:移动目标页(需已擦除)
 * offset :需要替换的数据起始地址(<nand_flash.page_size)
 * len    :需替换数据长度. 无则=0
 * buf    :存储的替换的数据.无则NULL
 * return :nandflash status reg(0:ok, >0:err) ;or bad block(<0:bad block num(-1表示0块,-2:1,,,))
 * */
int nand_page_internal_data_move(u32 page_src_addr, u32 page_dest_addr, u16 offset, u16 len, u8 *buf)
{
    u32 bp_mix_src = 0, bp_mix_dst = 0;
    u32 cache_addr_src, cache_addr_dst;
    int bad_block = 0;

    bp_mix_src = block_page_get(page_src_addr, &cache_addr_src);
    bp_mix_dst = block_page_get(page_dest_addr, &cache_addr_dst);

    //<1、PAGE READ to cache
    bad_block = nand_page_read_to_cache(bp_mix_src);
    if (bad_block < 0) {
        return bad_block;
    }

    //<2、PROGRAM LOAD RANDOM DATA
    cache_addr_dst &= 0x0fff;
    if (nand_flash.plane_select == 1) { //XT26G02C(2Gb)
        if (bp_mix_dst & BIT(6)) {
            cache_addr_dst |= BIT(12);
        }
    }
    if (nand_flash.write_enable_position == 1) {
        nand_write_enable();
    }
    if (_nandflash.spi_r_width == SPI_MODE_UNIDIR_4BIT) {
        nand_program_load_random_data_x4(buf, cache_addr_dst + offset, len);
    } else {
        nand_program_load_random_data(buf, cache_addr_dst + offset, len);
    }
// If the random data is not sequential, then another PROGRAM LOAD RANDOM DATA x1 (84h) command must be issued with a new column address.*******************
    //<3、WRITE ENABLE
    //<4、PROGRAM EXECUTE
    if (nand_flash.write_enable_position == 0) { //XT26G01C(1Gb)
        nand_write_enable();
    }
    nand_program_excute(bp_mix_dst);
    //<5、GET FEATURE
    return nand_get_features(GD_GET_STATUS);
}

/*
 * 同一plane中块与块数据移动, 可替换块中page_num页的offset位置len长的数据
 * block_src_addr :要移动的块
 * block_dest_addr:移动目标块(需已擦除)
 * page_num:需修改数据的页号(<=63)
 * offset :需要替换的数据起始地址(<nand_flash.page_size)
 * len    :需替换数据长度. 无则=0
 * buf    :存储的替换的数据.无则NULL
 * return :nandflash status reg(0:ok, >0:err) ;or bad block(<0:bad block num(-1表示0块,-2:1,,,))
 * */
int nand_block_internal_data_move(u32 block_src_addr, u32 block_dest_addr, u8 page_num, u16 offset, u16 len, u8 *buf)
{

    u32 bp_mix_src = 0, bp_mix_dst = 0;
    u32 cache_addr_src, cache_addr_dst;
    u8 page_cnt = 0;
    int bad_block;

    bp_mix_src = block_page_get(block_src_addr, &cache_addr_src);
    bp_mix_src &= ~0x3f;
    bp_mix_dst = block_page_get(block_dest_addr, &cache_addr_dst);
    bp_mix_dst &= ~0x3f;
    cache_addr_dst &= 0x0fff;
    if (nand_flash.plane_select == 1) { //XT26G02C(2Gb)
        if (bp_mix_dst & BIT(6)) {
            cache_addr_dst |= BIT(12);
        }
    }
    u32 wnum = (len + offset) / nand_flash.page_size;
    u32 wlen;
    if (((len + offset) % nand_flash.page_size) || (0 == (len + offset))) {
        wnum += 1;
    }

    for (page_cnt = 0; page_cnt < 64; page_cnt++) {
        //<1、PAGE READ to cache
        bad_block = nand_page_read_to_cache(bp_mix_src + page_cnt);
        if (bad_block < 0) {
            return bad_block;
        }
        //<2、PROGRAM LOAD RANDOM DATA
        if (nand_flash.write_enable_position == 1) { //XT26G02C(2Gb)
            nand_write_enable();
        }
        if ((page_cnt >= page_num) && (page_cnt < (page_num + wnum))) {
            wlen = len;
            if ((wlen + offset) > nand_flash.page_size) {
                len -= (nand_flash.page_size - offset);
                wlen = (nand_flash.page_size - offset);
            }

            if (_nandflash.spi_r_width == SPI_MODE_UNIDIR_4BIT) {
                nand_program_load_random_data_x4(buf, cache_addr_dst + offset, wlen);
            } else {
                nand_program_load_random_data(buf, cache_addr_dst + offset, wlen);
            }
            if (buf) {
                buf += wlen;
                offset = 0;
            }
        } else {
            /* putchar('+'); */
            if (_nandflash.spi_r_width == SPI_MODE_UNIDIR_4BIT) {
                nand_program_load_random_data_x4(NULL, cache_addr_dst, 0);
            } else {
                nand_program_load_random_data(NULL, cache_addr_dst, 0);
            }
        }
        // If the random data is not sequential, then another PROGRAM LOAD RANDOM DATA x1 (84h) command must be issued with a new column address.*******************
        //<3、WRITE ENABLE
        //<4、PROGRAM EXECUTE
        if (nand_flash.write_enable_position == 0) { //XT26G01C(1Gb)
            nand_write_enable();
        }
        nand_program_excute(bp_mix_dst + page_cnt);
    }
    //<5、GET FEATURE
    return nand_get_features(GD_GET_STATUS);

}

static int _nandflash_open(void *arg);

int _nandflash_init(const char *name, struct nandflash_dev_platform_data *pdata)
{
    log_info("nandflash_init ! %x %x", pdata->spi_cs_port, pdata->spi_read_width);
    if (_nandflash.spi_num == (int) - 1) {
        _nandflash.spi_num = pdata->spi_hw_num;
        _nandflash.spi_cs_io = pdata->spi_cs_port;
        _nandflash.spi_r_width = pdata->spi_read_width;
        _nandflash.flash_id = 0;
        os_mutex_create(&_nandflash.mutex);
        _nandflash.max_end_addr = 0;
        _nandflash.part_num = 0;
        spi_data_width = pdata->spi_pdata->mode;
        gpio_set_drive_strength(IO_PORT_SPILT(pdata->spi_pdata->port[0]), PORT_DRIVE_STRENGT_64p0mA);
    }
    ASSERT(_nandflash.spi_num == pdata->spi_hw_num);
    ASSERT(_nandflash.spi_cs_io == pdata->spi_cs_port);
    ASSERT(_nandflash.spi_r_width == pdata->spi_read_width);
    struct nandflash_partition *part;
    part = nandflash_find_part(name);
    if (!part) {
        part = nandflash_new_part(name, pdata->start_addr, pdata->size);
        ASSERT(part, "not enough nandflash partition memory in array\n");
        ASSERT(nandflash_verify_part(part) == 0, "nandflash partition %s overlaps\n", name);
        log_info("nandflash new partition %s\n", part->name);
    } else {
        return -1;
    }
    // Open HW early so SPI init, chip ID probe, and geometry detection
    // happen at partition-registration time. Without this, the FTL layer's
    // first dev_open would fail because nand_flash.page_size/block_size
    // are still zero and IOCTL_GET_PAGE_SIZE returns invalid geometry.
    _nandflash_open(NULL);
    return 0;
}

static void clock_critical_enter()
{

}
static void clock_critical_exit()
{
    if (!(_nandflash.flash_id == 0 || _nandflash.flash_id == 0xffff)) {
        spi_set_baud(_nandflash.spi_num, spi_get_baud(_nandflash.spi_num));
    }
}
CLOCK_CRITICAL_HANDLE_REG(spi_nandflash, clock_critical_enter, clock_critical_exit);

//2Gb上电自动复位,1Gb未介绍
int _nandflash_open(void *arg)
{
    int reg = 0;
    os_mutex_pend(&_nandflash.mutex, 0);
    log_info("nandflash open\n");
    if (!_nandflash.open_cnt) {
        spi_cs_init();
        spi_open(_nandflash.spi_num, get_hw_spi_config(_nandflash.spi_num));
        spi_set_width(_nandflash.spi_r_width);
        {
            struct spi_platform_data *spi_pdata = get_hw_spi_config(_nandflash.spi_num);
            for (int i = 0; i < 6; i++) {
                if (spi_pdata->port[i] != (u8)-1) {
                    gpio_hw_set_drive_strength(IO_PORT_SPILT(spi_pdata->port[i]), PORT_DRIVE_STRENGT_64p0mA);
                }
            }
            if (spi_pdata->port[4] != (u8)-1) {
                gpio_set_mode(IO_PORT_SPILT(spi_pdata->port[4]), PORT_INPUT_PULLUP_10K);
            }
            if (spi_pdata->port[5] != (u8)-1) {
                gpio_set_mode(IO_PORT_SPILT(spi_pdata->port[5]), PORT_INPUT_PULLUP_10K);
            }
        }
        nand_flash_reset();
        _nandflash.flash_id = nand_flash_read_id();
        log_info("nandflash_read_id: 0x%x\n", _nandflash.flash_id);
        switch (_nandflash.flash_id) {
        case XT26G01C :
            nand_flash.ecc_mask = 0xf0;
            nand_flash.ecc_err = 0xf0;
            nand_flash.plane_select = 0;
            nand_flash.write_enable_position = 0;
            nand_flash.quad_mode_dummy_num = 1;
            nand_flash.quad_mode_qe = 1;
            nand_flash.block_number = 1024;
            nand_flash.capacity = 128 * 1024 * 1024;
            nand_flash.page_size = 2048;
            nand_flash.block_size = 128 * 1024;
            break;
        case XT26G02C :
            nand_flash.ecc_mask = 0x70;
            nand_flash.ecc_err = 0x20;
            nand_flash.plane_select = 1;
            nand_flash.write_enable_position = 1;
            nand_flash.quad_mode_dummy_num = 0;
            nand_flash.quad_mode_qe = 0;
            nand_flash.block_number = 2048;
            nand_flash.capacity = 256 * 1024 * 1024;
            nand_flash.page_size = 2048;
            nand_flash.block_size = 128 * 1024;
            break;
        case XCSP4AAWH :
            nand_flash.ecc_mask = 0x70;
            nand_flash.ecc_err = 0x20;
            nand_flash.plane_select = 0;
            nand_flash.write_enable_position = 0;
            nand_flash.quad_mode_dummy_num = 1;
            nand_flash.quad_mode_qe = 1;
            nand_flash.block_number = 4096;
            nand_flash.capacity = 512 * 1024 * 1024;
            nand_flash.page_size = 2048;
            nand_flash.block_size = 128 * 1024;
            break;
        case F35SQA512M :
            nand_flash.ecc_mask = 0x30;
            nand_flash.ecc_err = 0x20;
            nand_flash.plane_select = 0;
            nand_flash.write_enable_position = 0;
            nand_flash.quad_mode_dummy_num = 0;
            nand_flash.quad_mode_qe = 1;
            nand_flash.block_number = 512;
            nand_flash.capacity = 64 * 1024 * 1024;
            nand_flash.page_size = 2048;
            nand_flash.block_size = 128 * 1024;
            break;
        case F35SQA001G :
            nand_flash.ecc_mask = 0x30;
            nand_flash.ecc_err = 0x20;
            nand_flash.plane_select = 0;
            nand_flash.write_enable_position = 0;
            nand_flash.quad_mode_dummy_num = 0;
            nand_flash.quad_mode_qe = 1;
            nand_flash.block_number = 1024;
            nand_flash.capacity = 128 * 1024 * 1024;
            nand_flash.page_size = 2048;
            nand_flash.block_size = 128 * 1024;
            break;
        case F35SQA002G :
            nand_flash.ecc_mask = 0x30;
            nand_flash.ecc_err = 0x20;
            nand_flash.plane_select = 0;
            nand_flash.write_enable_position = 0;
            nand_flash.quad_mode_dummy_num = 0;
            nand_flash.quad_mode_qe = 1;
            nand_flash.block_number = 2048;
            nand_flash.capacity = 256 * 1024 * 1024;
            nand_flash.page_size = 2048;
            nand_flash.block_size = 128 * 1024;
            break;
        case ZB35Q01C:
            nand_flash.ecc_mask = 0x30;
            nand_flash.ecc_err = 0x20;
            nand_flash.plane_select = 0;
            nand_flash.write_enable_position = 0;
            nand_flash.quad_mode_dummy_num = 1;
            nand_flash.quad_mode_qe = 1;
            nand_flash.block_number = 1024;
            nand_flash.capacity = 128 * 1024 * 1024;
            nand_flash.page_size = 2048;
            nand_flash.block_size = 128 * 1024;
            break;
        case GD5F4GM7UE:
            // GD5F4GM7UE: 4Gb SPI NAND, 4KB page, 256KB block, 2048 blocks
            // ECC: 8-bit / 512-byte segment, status bits ECCS1=0b001100 ECCS0=0b000110
            nand_flash.ecc_mask = 0x30;
            nand_flash.ecc_err = 0x0C;
            nand_flash.plane_select = 0;
            nand_flash.write_enable_position = 0;
            nand_flash.quad_mode_dummy_num = 8;
            nand_flash.quad_mode_qe = 0;
            nand_flash.block_number = 2048;
            nand_flash.capacity = 512 * 1024 * 1024;
            nand_flash.page_size = 4096;
            nand_flash.block_size = 256 * 1024;
            break;
        }

//power-up:need set featurea[A0]=00h
        if (_nandflash.flash_id == GD5F4GM7UE) {
            // GD5F4GM7UE: ECC enable only, no write protect
            // A0=0x00: BP2=0,BP1=0,BP0=0 → no block protection
            // B0=0x11: ECC_EN=1 (bit4), QE=1 (bit0)
            // Set Features needs WEL=1 before modifying protection register
            nand_write_enable();
            nand_set_features(0xA0, 0x00);
            nand_flash_wait_ok(NAND_FLASH_TIMEOUT);
            nand_write_enable();
            nand_set_features(0xB0, 0x11);
            nand_flash_wait_ok(NAND_FLASH_TIMEOUT);
        } else {
            nand_set_features(0xA0, 0x00);
        }
        if ((_nandflash.flash_id == 0) || (_nandflash.flash_id == 0xffff)) {
            log_error("read nandflash id error !\n");
            reg = -ENODEV;
            goto __exit;
        }
        log_info("nandflash open success !\n");
    }
    if (_nandflash.flash_id == 0 || _nandflash.flash_id == 0xffff)  {
        log_error("re-open nandflash id error !\n");
        reg = -EFAULT;
        goto __exit;
    }
    _nandflash.open_cnt++;
    /* nand_flash_erase_all();// */
__exit:
    os_mutex_post(&_nandflash.mutex);
    return reg;
}

int _nandflash_close(void)
{
    os_mutex_pend(&_nandflash.mutex, 0);
    log_info("nandflash close\n");
    if (_nandflash.open_cnt) {
        _nandflash.open_cnt--;
    }
    if (!_nandflash.open_cnt) {
        spi_deinit(_nandflash.spi_num);
        spi_cs_uninit();

        log_info("nandflash close done\n");
    }
    os_mutex_post(&_nandflash.mutex);
    return 0;
}


int _nandflash_ioctl(u32 cmd, u32 arg, u32 unit, void *_part)
{
    int reg = 0;
    struct nandflash_partition *part = _part;
    os_mutex_pend(&_nandflash.mutex, 0);
    switch (cmd) {
    case IOCTL_GET_STATUS:
        *(u32 *)arg = 1;
        /* *(u32 *)arg = nand_get_features(GD_GET_STATUS); */
        break;
    case IOCTL_GET_ID:
        *((u32 *)arg) = _nandflash.flash_id;
        break;
    case IOCTL_GET_BLOCK_SIZE:
        *(u32 *)arg = 512;//usb fat
        /* *(u32 *)arg = nand_flash.block_size; */
        break;
    case IOCTL_ERASE_BLOCK:
        if ((arg  + part->start_addr) % nand_flash.block_size == 0) {
            reg = nand_flash_erase(arg  + part->start_addr);
        }
        break;
    case IOCTL_GET_CAPACITY:
        *(u32 *)arg = nand_flash.capacity;
        break;
    case IOCTL_GET_PAGE_SIZE:
        *(u32 *)arg = nand_flash.page_size;
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
    os_mutex_post(&_nandflash.mutex);
    return reg;
}


/*************************************************************************************
 *                                  挂钩 device_api
 ************************************************************************************/

static int nandflash_dev_init(const struct dev_node *node, void *arg)
{
    struct nandflash_dev_platform_data *pdata = arg;
    int ret = _nandflash_init(node->name, pdata);
#if TCFG_NAND_TEST_ENABLE
    nand_test_run_raw();
#endif
    return ret;
}

static int nandflash_dev_open(const char *name, struct device **device, void *arg)
{
    struct nandflash_partition *part;
    part = nandflash_find_part(name);
    if (!part) {
        log_error("no nandflash partition is found\n");
        return -ENODEV;
    }
    *device = &part->device;
    (*device)->private_data = part;
    if (atomic_read(&part->device.ref)) {
        return 0;
    }
    return _nandflash_open(arg);
}
static int nandflash_dev_close(struct device *device)
{
    return _nandflash_close();
}

//return: 0:err, =len:ok, <0:block err
static int nandflash_dev_read(struct device *device, void *buf, u32 len, u32 offset)
{
    int reg;
    offset = offset * 512;
    len = len * 512;

    struct nandflash_partition *part;
    part = (struct nandflash_partition *)device->private_data;
    if (!part) {
        log_error("nandflash partition invalid\n");
        return -EFAULT;
    }
    offset += part->start_addr;
    reg = nand_flash_read(offset, buf, len);
    if (reg) {
        log_error("nandflash read error, reg=%d", reg);
        len = 0;
        if (reg < 0) {
            return reg;
        }
    }
    len = len / 512;
    return len;
}
//写前需确保该block(128k)已擦除
static int nandflash_dev_write(struct device *device, void *buf, u32 len, u32 offset)
{

    int reg = 0;
    offset = offset * 512;
    len = len * 512;
    struct nandflash_partition *part = device->private_data;
    if (!part) {
        log_error("nandflash partition invalid\n");
        return -EFAULT;
    }
    offset += part->start_addr;
    reg = nand_flash_write(offset, buf, len);
    if (reg) {
        log_error("nandflash write error, reg=%d", reg);
        len = 0;
    }
    len = len / 512;
    return len;
}
static bool nandflash_dev_online(const struct dev_node *node)
{
    return 1;
}
static int nandflash_dev_ioctl(struct device *device, u32 cmd, u32 arg)
{
    struct nandflash_partition *part = device->private_data;
    if (!part) {
        log_error("nandflash partition invalid\n");
        return -EFAULT;
    }
    return _nandflash_ioctl(cmd, arg, 512,  part);
}

u16 nandflash_get_flash_id(void)
{
    return _nandflash.flash_id;
}

const struct device_operations nandflash_dev_ops = {
    .init   = nandflash_dev_init,
    .online = nandflash_dev_online,
    .open   = nandflash_dev_open,
    .read   = nandflash_dev_read,
    .write  = nandflash_dev_write,//写前需确保该block(128k)已擦除
    .ioctl  = nandflash_dev_ioctl,
    .close  = nandflash_dev_close,
};


#endif

