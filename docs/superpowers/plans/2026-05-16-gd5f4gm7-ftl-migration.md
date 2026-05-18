# GD5F4GM7 闭源 FTL → 开源 FTL 迁移计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 用开源 `nandflash_ftl.c` 替代闭源 `ftl.a`，解决 `ftl.a` 在 GD5F4GM7UE 4KB page / 256KB block 几何下的 `ftl_format()` 崩溃问题。

**Architecture:** `nandflash_ftl.c` 是一个自包含的 FTL 实现（1393 行），提供 `ftl_dev_ops`（与当前 `ftl_device.c` 同名冲突）。它坐在裸 `nand_flash` 设备上，通过 `dev_bulk_read/write` + `dev_ioctl` 管理坏块/磨损均衡/FAT 格式化。核心改动：将硬编码 `FTL_NAND_PAGE_SIZE=2048` / `FTL_NAND_BLOCK_SIZE=0x20000` 改为 GD5F4GM7UE 的 4096 / 0x40000。

**Tech Stack:** C, JL BR28 SDK, Pi32v2 工具链

**设计依据:** `docs/superpowers/specs/2026-05-15-gd5f4gm7-nandflash-design.md`（v4）

**文件清单:**

| 文件 | 操作 | 目的 |
|------|------|------|
| `SDK/apps/common/include/nandflash_ftl.h` | 新建 | FTL 类型定义 + 几何 #define（GD5F4GM7UE 适配） |
| `SDK/apps/common/device/storage_device/nandflash/nandflash_ftl.c` | 新建 | 开源 FTL 实现（从参考版本拷贝 + include 路径调整） |
| `SDK/build/genFileList.c` | 修改 | 用 `nandflash_ftl.c` 替换 `ftl_device.c` |
| `SDK/build/Makefile.mk` | 修改 | 移除 `ftl.a` 链接（不再需要） |
| `SDK/apps/common/device/storage_device/nandflash/nandflash.c` | 修改 | 移除裸 NAND 诊断代码 |
| `SDK/apps/common/device/storage_device/nandflash/ftl_device.c` | 修改 | `#if 0` 禁用 `ftl_dev_ops`，保留为参考 |

---

### Task 1: 创建 nandflash_ftl.h（GD5F4GM7UE 几何适配）

**文件:**
- 新建: `SDK/apps/common/include/nandflash_ftl.h`

- [ ] **Step 1: 写入头文件**

```c
#ifndef __NANDFLASH_FTL_H__
#define __NANDFLASH_FTL_H__

// GD5F4GM7UE: 4KB page, 256KB block (64 pages/block)
#define FTL_NAND_PAGE_SIZE                     4096
#define FTL_NAND_BLOCK_SIZE                    0x40000

enum {
    FTL_OK = 0,
    FTL_DISK_ERR,
    FTL_FILE_ERR,
    FTL_BLOCK_ERR,
    FTL_FLASH_ERR,
    FTL_MOUNT_FAT_ERR,
    FTL_UPDATE_SPBK_ERR,
    FTL_SEARCH_ERR,
};

extern const struct device_operations ftl_dev_ops;

#define FTL_FREE         0xFF
#define FTL_DAMAGED      0x00
#define FTL_USED         0x01
#define FTL_CONSUMED     0x10
#define FTL_FAT          0x20
#define FTL_USED_LOG     0x40
#define FTL_SPBK         0xF8FFFFFFu
#define FTL_BLOCK_INFO   0xF4FFFFFFu
#define FTL_EX_FREE      0xFEFFFFFFu
#define FTL_BLOCK_BREAK  0xFFFFFFFFu

typedef struct _spbk_info {
    char spbk_logo[4];
    int spbk_page_size;
    int spbk_block_size;
    int spbk_flash_size;
    int spbk_mount_size;
    int spbk_hot_part_addr;
    int spbk_cold_part_addr;
    int spbk_cold_part_exaddr;
    int spbk_hot_part_exaddr;
    int spbk_block_info_logaddr;
    int spbk_block_info_addr;
    int spbk_block_info_len;
} SPBK_INFO;

typedef struct _block_info {
    int logic_map_addr;
    char page_status;
    char block_erase_num[3];
} BLOCK_INFO;

typedef struct _ftl_hdl {
    SPBK_INFO spbk;
    BLOCK_INFO block_info;
    void *hdev;
    char re_devname[16];
    char erase_all;
    char block_info_exchange;
    char err_block_num;
    int cur_block;
    int new_cold_sblock;
    int old_cold_exaddr;
} FTL_HDL;

#endif
```

- [ ] **Step 2: 提交**

```bash
git add SDK/apps/common/include/nandflash_ftl.h
git commit -m "$(cat <<'EOF'
ftl: add nandflash_ftl.h with GD5F4GM7UE geometry defines

- FTL_NAND_PAGE_SIZE = 4096 (4KB page)
- FTL_NAND_BLOCK_SIZE = 0x40000 (256KB block)
- SPBK_INFO, BLOCK_INFO, FTL_HDL type definitions

Co-Authored-By: Claude Opus 4.7 <noreply@anthropic.com>
EOF
)"
```

---

### Task 2: 拷贝并适配 nandflash_ftl.c

**文件:**
- 新建: `SDK/apps/common/device/storage_device/nandflash/nandflash_ftl.c`

- [ ] **Step 1: 拷贝源文件**

```bash
cp PIN_JL7018_V3.0.0_zenchord/SDK/apps/common/device/storage_device/nandflash/nandflash_ftl.c \
   SDK/apps/common/device/storage_device/nandflash/nandflash_ftl.c
```

- [ ] **Step 2: 修正 include 路径**

```c
// --- old_string (line 1) ---
#include "include/nandflash_ftl.h"
// --- new_string ---
#include "nandflash_ftl.h"
```

- [ ] **Step 3: 移除 virfat_flash.h 依赖（不需要）**

文件搜索 `virfat_flash` 使用：仅在一行注释中（`virfat_flash_erase`）。该 include 不提供 `nandflash_ftl.c` 实际使用的任何符号，可移除。

```c
// --- old_string ---
#include "virfat_flash.h"
// --- new_string (remove the line) ---
```

- [ ] **Step 4: 修正 clr_wdt 依赖**

`clr_wdt()` 在 `asm/wdt.h` 中定义，但 `system/includes.h` 可能已包含。添加显式 include：

```c
// --- old_string ---
#include "system/includes.h"
#include "virfat_flash.h"
// --- new_string ---
#include "system/includes.h"
#include "asm/wdt.h"
```

- [ ] **Step 5: 提交**

```bash
git add SDK/apps/common/device/storage_device/nandflash/nandflash_ftl.c
git commit -m "$(cat <<'EOF'
ftl: add open-source nandflash_ftl.c FTL implementation

Copied from reference soundbox SDK, adapted for GD5F4GM7UE:
- Include path corrected (nandflash_ftl.h in common include dir)
- Removed unused virfat_flash.h dependency

Co-Authored-By: Claude Opus 4.7 <noreply@anthropic.com>
EOF
)"
```

---

### Task 3: 保留 ftl_device.c 为参考（不修改，不编译）

**说明:** `nandflash_ftl.c` 也定义了 `ftl_dev_ops`，同名冲突。Task 4 将从 `genFileList.c` 移除 `ftl_device.c`，该文件不再参与编译。**无需 `#if 0`**——文件保持原样作为参考即可。

已验证：`ftl.a` 的 `ftl_init/ftl_format/ftl_read_bytes/ftl_write_bytes/ftl_uninit` 仅被 `ftl_device.c` 调用，无其他模块依赖。

---

### Task 4: 更新构建系统

**文件:**
- 修改: `SDK/build/genFileList.c`
- 修改: `SDK/build/Makefile.mk`

- [ ] **Step 1: genFileList.c — 用 nandflash_ftl.c 替换 ftl_device.c**

```c
// --- old_string ---
#if TCFG_NANDFLASH_DEV_ENABLE
c_SRC_FILES += \
	apps/common/device/storage_device/nandflash/nandflash.c \
	apps/common/device/storage_device/nandflash/ftl_device.c
#endif
// --- new_string ---
#if TCFG_NANDFLASH_DEV_ENABLE
c_SRC_FILES += \
	apps/common/device/storage_device/nandflash/nandflash.c \
	apps/common/device/storage_device/nandflash/nandflash_ftl.c
#endif
```

- [ ] **Step 2: Makefile.mk — 移除 ftl.a 链接**

找到 `cpu/br28/liba/ftl.a` 行，删除或注释：

```bash
# Verify: grep -n "ftl\.a" SDK/build/Makefile.mk
# Then remove or comment out that line
```

- [ ] **Step 3: 提交**

```bash
git add SDK/build/genFileList.c SDK/build/Makefile.mk
git commit -m "$(cat <<'EOF'
build: replace ftl_device.c with nandflash_ftl.c, remove ftl.a

- nandflash_ftl.c provides its own FTL implementation
- ftl.a is no longer needed (closed-source FTL core)
- ftl_device.c kept as reference but excluded from compilation

Co-Authored-By: Claude Opus 4.7 <noreply@anthropic.com>
EOF
)"
```

---

### Task 5: 清理 nandflash.c 诊断代码

**文件:**
- 修改: `SDK/apps/common/device/storage_device/nandflash/nandflash.c`

- [ ] **Step 1: 移除裸 NAND 诊断测试**

移除之前加入的 `raw erase/write/read` 测试代码块和 `A0 after set / B0 after set` 诊断日志，保留核心的 feature 寄存器初始化。

```c
// --- old_string ---
            log_info("A0 after set: 0x%x", nand_get_features(0xA0));
            nand_write_enable();
            nand_set_features(0xB0, 0x11);
            nand_flash_wait_ok(NAND_FLASH_TIMEOUT);
            log_info("B0 after set: 0x%x", nand_get_features(0xB0));

            // DIAG: raw NAND erase + write + read-back test
            {
                u8 wbuf[256], rbuf[256];
                for (int i = 0; i < 256; i++) wbuf[i] = i;
                int r;
                log_info("raw erase blk0: %d", nand_flash_erase(0));
                r = nand_flash_write_page(0, 0, wbuf, 256);
                log_info("raw write pg0: %d", r);
                r = nand_flash_read_page(0, 0, 0, rbuf, 256);
                log_info("raw read pg0: %d, buf[0]=%02x buf[255]=%02x",
                         r, rbuf[0], rbuf[255]);
            }
// --- new_string ---
            nand_write_enable();
            nand_set_features(0xB0, 0x11);
            nand_flash_wait_ok(NAND_FLASH_TIMEOUT);
```

- [ ] **Step 2: 提交**

```bash
git add SDK/apps/common/device/storage_device/nandflash/nandflash.c
git commit -m "$(cat <<'EOF'
nandflash: remove raw NAND diagnostic test code

Bare NAND driver verified: erase/write/read all pass.
Remove diagnostic logging added during ftl.a debugging.

Co-Authored-By: Claude Opus 4.7 <noreply@anthropic.com>
EOF
)"
```

---

### Task 6: 编译验证

- [ ] **Step 1: 清理并编译**

Run: `cd SDK/ && make clean && make 2>&1 | tee build.log`

Expected: `sdk.elf` 成功生成，无 `ftl.a` 相关链接错误，无 `ftl_dev_ops` 重复定义错误。

- [ ] **Step 2: 确认符号**

Run: `grep -E "ftl_dev_ops|nandflash_dev_ops|nandflash_ftl" SDK/cpu/br28/tools/sdk.map`

Expected: `ftl_dev_ops` 来自 `nandflash_ftl.c`，`nandflash_dev_ops` 来自 `nandflash.c`，无 `ftl.a` 符号。

---

### 后续硬件验证

编译通过后，烧录测试。预期日志应包含：

```
[nandFLASH]nandflash_read_id: 0xc894
[nandFLASH]nandflash open success !
nandflash_ftl add start
... FTL 内部日志 ...
__dev_manager_add, nandflash_ftl add ok
===nandflash_ftl mount ok===
```

开源 FTL 有自己的日志输出（`r_printf`/`y_printf`），可观察其初始化流程是否正常。
