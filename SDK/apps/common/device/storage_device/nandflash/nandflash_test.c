#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".nandflash.test.data.bss")
#pragma data_seg(".nandflash.test.data")
#pragma const_seg(".nandflash.test.text.const")
#pragma code_seg(".nandflash.test.text")
#endif

#include "nandflash_test.h"

#if TCFG_NAND_TEST_ENABLE

#include "nandflash.h"
#include "nandflash_ftl.h"
#include "fs.h"
#include "system/includes.h"

#undef LOG_TAG_CONST
#define LOG_TAG     "[NAND_TEST]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
#include "debug.h"

// ---- extern internal globals from nandflash.c / nandflash_ftl.c ----
extern int nand_flash_read(u32 offset, u8 *buf, u32 len); // non-static, declared only in .c
extern FTL_HDL ftl_hdl;
extern int _ftl_read(void *buf, u32 logic_sec_addr, u32 len);
extern int ftl_get_block_info(u32 location, BLOCK_INFO *dir, int flag);

// ---- helpers ----
static u8 _test_buf[4096] __attribute__((aligned(4)));

static void _print_pass(const char *name)
{
    printf("  [PASS] %s\n", name);
}

static void _print_fail(const char *name, int err)
{
    printf("  [FAIL] %s (err=%d)\n", name, err);
}

static void _print_skip(const char *name, const char *reason)
{
    printf("  [SKIP] %s — %s\n", name, reason);
}

// ==================== RAW Driver Test ====================
#if NAND_TEST_RAW
static int _test_raw_driver(void)
{
    int res;
    u16 chip_id;
    const u8 pattern[] = {0xA5, 0x5A, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    const u8 move_data[] = {0x10, 0x20, 0x30, 0x40, 0x50};

    log_info("--- RAW NAND Driver Test ---");

    /* 1. Chip ID 与几何参数 */
    chip_id = nandflash_get_flash_id();
    if (chip_id == 0 || chip_id == 0xFFFF) {
        _print_fail("Read Chip ID", -1);
        return -1;
    }
    printf("  Chip ID: 0x%04X  Page=%d  Block=%dKB  Capacity=%dMB\n",
           chip_id, nand_flash.page_size,
           nand_flash.block_size / 1024,
           nand_flash.capacity / (1024 * 1024));
    _print_pass("Read Chip ID + Geometry");

    /* 2. Block 擦除 */
    res = nand_flash_erase(0);
    if (res != 0) {
        _print_fail("Block Erase (addr=0)", res);
        return -1;
    }
    _print_pass("Block Erase");

    /* 3. Page 写入 + 读回 + 校验 */
    memset(_test_buf, 0, nand_flash.page_size);
    memcpy(_test_buf, pattern, sizeof(pattern));
    res = nand_flash_write_page(0, 0, _test_buf, nand_flash.page_size);
    if (res != 0) {
        _print_fail("Page Write (block=0,page=0)", res);
        return -1;
    }

    memset(_test_buf, 0, nand_flash.page_size);
    res = nand_flash_read_page(0, 0, 0, _test_buf, nand_flash.page_size);
    if (res != 0) {
        _print_fail("Page Read (block=0,page=0)", res);
        return -1;
    }
    if (memcmp(_test_buf, pattern, sizeof(pattern)) != 0) {
        _print_fail("Data Verify", -1);
        printf("    expected: ");
        printf_buf((u8 *)pattern, sizeof(pattern));
        printf("    got:      ");
        printf_buf(_test_buf, sizeof(pattern));
        return -1;
    }
    _print_pass("Page Write + Read + Verify");

    /* 4. 内部数据搬运测试
     * 使用 page_size 作为目标地址（page 1），避免对同一 page 二次编程。 */
    u32 move_dst_addr = nand_flash.page_size;
    memset(_test_buf, 0, nand_flash.page_size);
    res = nand_page_internal_data_move(0x00, move_dst_addr, 0, sizeof(move_data), (u8 *)move_data);
    if (res != 0) {
        _print_fail("Page Internal Data Move", res);
        return -1;
    }
    // 从目标 page 读回
    memset(_test_buf, 0, nand_flash.page_size);
    res = nand_flash_read(move_dst_addr, _test_buf, nand_flash.page_size);
    if (res != 0) {
        _print_fail("Read Moved Page", res);
        return -1;
    }
    // 验证替换数据在目标偏移 0 处
    if (memcmp(_test_buf, move_data, sizeof(move_data)) != 0) {
        _print_fail("Replacement Data Verify", -1);
        printf("    expected: ");
        printf_buf((u8 *)move_data, sizeof(move_data));
        printf("    got:      ");
        printf_buf(_test_buf, sizeof(move_data));
        return -1;
    }
    // 验证源地址数据未被破坏
    memset(_test_buf, 0, nand_flash.page_size);
    res = nand_flash_read(0x00, _test_buf, nand_flash.page_size);
    if (res != 0) {
        _print_fail("Read Source Page", res);
        return -1;
    }
    if (memcmp(_test_buf, pattern, sizeof(pattern)) != 0) {
        _print_fail("Source Data After Move", -1);
        return -1;
    }
    _print_pass("Page Internal Data Move + Verify");

    log_info("--- RAW Test: ALL PASS ---");
    return 0;
}
#endif /* NAND_TEST_RAW */

// ==================== FTL Test ====================
#if NAND_TEST_FTL
static int _test_ftl(void)
{
    int res;
    BLOCK_INFO bi;
    char tmp[64];

    log_info("--- FTL Test ---");

    /* 1. SPBK 信息转储 */
    SPBK_INFO *sp = &ftl_hdl.spbk;
    if (sp->spbk_page_size == 0 || sp->spbk_block_size == 0) {
        _print_fail("SPBK Valid Check", -1);
        return -1;
    }
    printf("  SPBK: page=%d  block=%dKB  mount=%d blocks  flash=%dMB\n",
           sp->spbk_page_size,
           sp->spbk_block_size / 1024,
           sp->spbk_mount_size,
           sp->spbk_flash_size * sp->spbk_block_size / (1024 * 1024));
    printf("  Hot: [%d, %d)  Cold: [%d, %d)  BlockInfo@%d\n",
           sp->spbk_hot_part_addr,
           sp->spbk_cold_part_addr,
           sp->spbk_cold_part_addr,
           sp->spbk_cold_part_exaddr,
           sp->spbk_block_info_addr);
    _print_pass("SPBK Info Dump");

    /* 2. 块信息查询 */
    // block 0 是 super block（保留区），不在 hot/cold 数据区内，
    // flag=0 要求验证逻辑映射关系会对 block 0 永远返回 FTL_FLASH_ERR。
    // 使用 flag=1 直接读取 block_info 条目，不做映射校验。
    res = ftl_get_block_info(0, &bi, 1);
    if (res != 0) {
        _print_fail("Block Info Query (block=0)", res);
        return -1;
    }
    printf("  Block[0]: logic_map=%d  status=0x%02X  erase_count=%d\n",
           bi.logic_map_addr, (u8)bi.page_status,
           bi.block_erase_num[0] | (bi.block_erase_num[1] << 8) | (bi.block_erase_num[2] << 16));
    _print_pass("Block Info Query");

    /* 3. 逻辑扇区读取（读文件系统扇区 0，验证 FAT BPB 签名）
     * ftl_dev_read 对 _ftl_read 的调用加了 hot_part_addr * block_size/512 偏移，
     * 测试直接调用 _ftl_read 需要手动加上该偏移。 */
    u32 fs_sector_0 = sp->spbk_hot_part_addr * sp->spbk_block_size / 512;
    memset(_test_buf, 0, 512);
    res = _ftl_read(_test_buf, fs_sector_0, 1);
    if (res != 0) {
        _print_fail("Sector 0 Read", res);
        return -1;
    }
    // FAT16/32 BPB 特征：偏移 0x36 处为 "FAT12"/"FAT16"/"FAT32"
    memcpy(tmp, _test_buf + 0x36, 8);
    tmp[8] = '\0';
    printf("  FS type: %s\n", tmp);
    _print_pass("Sector 0 Read (FAT BPB)");

    /* 4. 坏块计数 */
    printf("  Bad blocks: %d\n", ftl_hdl.err_block_num);
    _print_pass("Bad Block Count");

    log_info("--- FTL Test: ALL PASS ---");
    return 0;
}
#endif /* NAND_TEST_FTL */

// ==================== Filesystem Test ====================
#if NAND_TEST_FS
#if !TCFG_FATFS_WRITE_ENABLE
#error "NAND_TEST_FS requires TCFG_FATFS_WRITE_ENABLE"
#endif

#define TEST_ROOT_PATH  "storage/nandflash_ftl/C/"
#define TEST_FILENAME   TEST_ROOT_PATH"NTEST.TMP"
static const u8 _fs_pattern[] = "NAND_FS_TEST_2026";

static int _test_filesystem(void)
{
    FILE *fp = NULL;
    int res;
    int ret = -1;
    u8 rbuf[sizeof(_fs_pattern)] = {0};

    log_info("--- Filesystem Test ---");

    /* Ensure the smoke test is idempotent after an interrupted previous run. */
    fdelete_by_name(TEST_FILENAME);

    /* 1. 创建文件 + 写入 */
    fp = fopen(TEST_FILENAME, "w+");
    if (fp == NULL) {
        _print_fail("fopen (create)", -1);
        goto __exit;
    }
    res = fwrite((void *)_fs_pattern, 1, sizeof(_fs_pattern), fp);
    if (res != sizeof(_fs_pattern)) {
        _print_fail("fwrite", res);
        goto __exit;
    }
    res = fclose(fp);
    fp = NULL;
    if (res != 0) {
        _print_fail("fclose (write)", res);
        goto __exit;
    }
    _print_pass("File Create + Write");

    /* 2. 读取 + 校验 */
    memset(rbuf, 0, sizeof(rbuf));
    fp = fopen(TEST_FILENAME, "r");
    if (fp == NULL) {
        _print_fail("fopen (read)", -1);
        goto __exit;
    }
    res = fread(rbuf, 1, sizeof(_fs_pattern), fp);
    if (res != sizeof(_fs_pattern)) {
        _print_fail("fread", res);
        goto __exit;
    }
    fclose(fp);
    fp = NULL;
    if (memcmp(rbuf, _fs_pattern, sizeof(_fs_pattern)) != 0) {
        _print_fail("FS Data Verify", -1);
        printf("    expected: ");
        printf_buf((u8 *)_fs_pattern, sizeof(_fs_pattern));
        printf("    got:      ");
        printf_buf(rbuf, sizeof(_fs_pattern));
        goto __exit;
    }
    _print_pass("File Read + Verify");

    /* 3. 删除 + 验证删除 */
    res = fdelete_by_name(TEST_FILENAME);
    if (res != 0) {
        _print_fail("fdelete", res);
        goto __exit;
    }
    fp = fopen(TEST_FILENAME, "r");
    if (fp != NULL) {
        _print_fail("Verify Deletion (file still exists)", -1);
        goto __exit;
    }
    _print_pass("File Delete + Verify");

    log_info("--- FS Test: ALL PASS ---");
    ret = 0;

__exit:
    if (fp != NULL) {
        fclose(fp);
    }
    if (ret != 0) {
        fdelete_by_name(TEST_FILENAME);
    }
    return ret;
}
#endif /* NAND_TEST_FS */

// ==================== Entry Points ====================

// 入口 1：裸驱动测试（FTL init 之前调用，可安全擦写任意 Block）
void nand_test_run_raw(void)
{
#if NAND_TEST_RAW
    printf("\n");
    printf("========================================\n");
    printf("  NAND Flash RAW Driver Test\n");
    printf("========================================\n\n");

    int raw_pass = (_test_raw_driver() == 0);

    printf("\n========================================\n");
    printf("  RAW Test: %s\n", raw_pass ? "PASS" : "FAIL");
    printf("========================================\n\n");
#else
    printf("[NAND_TEST] RAW test disabled\n");
#endif
}

// 入口 2：FTL + FS 测试（mount 之后调用）
void nand_test_run_all(void)
{
    int ftl_pass = 1, fs_pass = 1;

    printf("\n");
    printf("========================================\n");
    printf("  NAND Flash FTL + FS Test Suite\n");
    printf("========================================\n\n");

#if NAND_TEST_FTL
    ftl_pass = (_test_ftl() == 0);
    printf("\n");
#else
    printf("  [DISABLED] FTL Test\n\n");
#endif

#if NAND_TEST_FS
    if (ftl_pass) {
        fs_pass = (_test_filesystem() == 0);
        printf("\n");
    } else {
        _print_skip("FS Test", "FTL not ready");
        fs_pass = 0;
    }
#else
    printf("  [DISABLED] FS Test\n\n");
#endif

    printf("========================================\n");
    printf("  Summary: FTL=%s  FS=%s\n",
           ftl_pass ? "PASS" : "FAIL",
           fs_pass ? "PASS" : "FAIL");
    printf("========================================\n\n");
}

#endif /* TCFG_NAND_TEST_ENABLE */
