# GD5F4GM7 NAND Flash 接入 实施计划 (v4)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 以厂商源文件（nandflash.c + ftl_device.c）为基础，通过运行时几何参数化支持 GD5F4GM7UE（512MB / 4KB page / 256KB block），并修改构建系统使 earphone 工程编译 NAND 源文件。

**Architecture:** 在现有源文件架构上做最小改动：
1. `nandflash.c` switch(read_id) 参数化 — 新增 page_size/block_size 运行时字段，替换硬编码 #define
2. `ftl_device.c` 通过 nandflash_get_ftl_info() 查询接口获取参数
3. `genFileList.c` 移除 CONFIG_SOUNDBOX_CASE 守卫使 earphone 也能编译

**Tech Stack:** C 预处理器宏配置, JL BR28 SDK, Pi32v2 工具链, SPI 单线半双工 (SPI_MODE_UNIDIR_1BIT)

**设计依据:** `docs/superpowers/specs/2026-05-15-gd5f4gm7-nandflash-design.md`（v4）

**文件清单:**

| 文件 | 操作 | 目的 |
|------|------|------|
| `SDK/apps/earphone/board/br28/board_ac701n_demo_cfg.h` | 修改 | 使能 NAND + SPI1 半双工 + 冲突清理 (Task 1) |
| `SDK/apps/earphone/device_config.c` | 修改 | 容量 128MB → 512MB (Task 2) |
| `SDK/apps/common/device/storage_device/nandflash/nandflash.c` | 修改 | 运行时几何参数化 + GD5F4GM7UE case + 查询接口 (Task 3) |
| `SDK/apps/common/include/nandflash.h` | 修改 | 新增查询接口声明 (Task 4) |
| `SDK/apps/common/device/storage_device/nandflash/ftl_device.c` | 修改 | 通过查询接口解耦硬编码参数 (Task 5) |
| `SDK/build/genFileList.c` | 修改 | 移除 CONFIG_SOUNDBOX_CASE 守卫 (Task 6) |

---

### Task 1: 板级配置 — SPI1 半双工 + NAND 使能 + 引脚冲突清理

**文件:**
- 修改: `SDK/apps/earphone/board/br28/board_ac701n_demo_cfg.h`

- [ ] **Step 1: 修改 SPI1 引脚和模式（约第 54-60 行）**

```c
// --- old_string ---
#define TCFG_HW_SPI1_ENABLE                 ENABLE_THIS_MOUDLE
#define TCFG_HW_SPI1_PORT_CLK               NO_CONFIG_PORT//IO_PORTA_00
#define TCFG_HW_SPI1_PORT_DO                NO_CONFIG_PORT//IO_PORTA_01
#define TCFG_HW_SPI1_PORT_DI                NO_CONFIG_PORT//IO_PORTA_02
#define TCFG_HW_SPI1_BAUD                   2000000L
#define TCFG_HW_SPI1_MODE                   SPI_MODE_BIDIR_1BIT
#define TCFG_HW_SPI1_ROLE                   SPI_ROLE_MASTER
// --- new_string ---
#define TCFG_HW_SPI1_ENABLE                 ENABLE_THIS_MOUDLE
#define TCFG_HW_SPI1_PORT_CLK               IO_PORTC_04
#define TCFG_HW_SPI1_PORT_DO                IO_PORTC_03
#define TCFG_HW_SPI1_PORT_DI                NO_CONFIG_PORT
#define TCFG_HW_SPI1_BAUD                   2000000L
#define TCFG_HW_SPI1_MODE                   SPI_MODE_UNIDIR_1BIT
#define TCFG_HW_SPI1_ROLE                   SPI_ROLE_MASTER
```

- [ ] **Step 2: 在 SPI2 配置段之后、SD 配置段之前插入 NAND Flash 使能（约第 68-70 行之间）**

找到：
```c
//*********************************************************************************//
//                                  SD 配置                                        //
//*********************************************************************************//
```

在它之前插入：
```c
//*********************************************************************************//
//                                 FLASH 配置                                      //
//*********************************************************************************//
// NAND Flash 使能（GD5F4GM7UE, SPI1 单线半双工）
// ECC_EN=1 (0xB0 bit4), 内部 8-bit ECC 开启
#define TCFG_NANDFLASH_DEV_ENABLE           1
#define TCFG_FLASH_DEV_SPI_HW_NUM           1
#define TCFG_FLASH_DEV_SPI_CS_PORT          IO_PORTC_05
#define TCFG_FLASH_DEV_FLASH_READ_WIDTH     1

// 当前项目不使用 NOR Flash / 内置录音。显式关闭以消除歧义。
// 注意：若后续开启离线 dlog，app_config.h 可能重新使能 TCFG_NORFLASH_DEV_ENABLE，
// 届时需重新检查 NAND 与 NOR 的引脚/资源规划。
#define TCFG_NORFLASH_SFC_DEV_ENABLE        DISABLE_THIS_MOUDLE
#define TCFG_NORFLASH_DEV_ENABLE            DISABLE_THIS_MOUDLE
#define TCFG_VIRFAT_FLASH_ENABLE            DISABLE_THIS_MOUDLE
#define FLASH_INSIDE_REC_ENABLE             DISABLE_THIS_MOUDLE
```

- [ ] **Step 3: 清理 NTC 引脚冲突（约第 31-33 行）**

```c
// --- old_string ---
#define NTC_DET_EN      0
#define NTC_POWER_IO    IO_PORTC_03
#define NTC_DETECT_IO   IO_PORTC_04
// --- new_string ---
#define NTC_DET_EN      0
#define NTC_POWER_IO    NO_CONFIG_PORT      // PC03 已被 SPI1 DATA 占用
#define NTC_DETECT_IO   NO_CONFIG_PORT      // PC04 已被 SPI1 CLK 占用
```

- [ ] **Step 4: 清理硬件 I2C 引脚冲突（约第 47-48 行）**

```c
// --- old_string ---
#define TCFG_HW_I2C0_CLK_PORT               IO_PORTC_04                             //硬件IIC  CLK脚选择
#define TCFG_HW_I2C0_DAT_PORT               IO_PORTC_05                             //硬件IIC  DAT脚选择
// --- new_string ---
#define TCFG_HW_I2C0_CLK_PORT               NO_CONFIG_PORT      // 避免与 PC04 (SPI1 CLK) 配置层冲突
#define TCFG_HW_I2C0_DAT_PORT               NO_CONFIG_PORT      // 避免与 PC05 (NAND CS) 配置层冲突
```

- [ ] **Step 5: 验证配置完整性**

Run: `grep -n "TCFG_NANDFLASH_DEV_ENABLE\|TCFG_FLASH_DEV_SPI\|SPI_MODE_UNIDIR_1BIT\|TCFG_NORFLASH_DEV_ENABLE\|NTC_DET_EN\|NTC_POWER_IO\|NTC_DETECT_IO\|TCFG_HW_I2C0" SDK/apps/earphone/board/br28/board_ac701n_demo_cfg.h`

Expected: 所有宏值与 Step 1-4 一致，且 `TCFG_NORFLASH_DEV_ENABLE` / `NTC_POWER_IO` 为 `DISABLE_THIS_MOUDLE` / `NO_CONFIG_PORT`。

- [ ] **Step 6: 提交**

```bash
git add SDK/apps/earphone/board/br28/board_ac701n_demo_cfg.h
git commit -m "$(cat <<'EOF'
config: enable GD5F4GM7 NAND flash on SPI1 half-duplex

- Enable TCFG_NANDFLASH_DEV_ENABLE with SPI1 (PC03/PC04/PC05)
- Set SPI1 to SPI_MODE_UNIDIR_1BIT (half-duplex)
- Clear NTC/HW I2C pin macros to avoid config-layer conflicts
- Disable NOR/SFC/rec flash per current project resource plan

Co-Authored-By: Claude Opus 4.7 <noreply@anthropic.com>
EOF
)"
```

---

### Task 2: 设备配置容量对齐

**文件:**
- 修改: `SDK/apps/earphone/device_config.c:171`

- [ ] **Step 1: 修改容量值**

```c
// --- old_string ---
    .size           = 128 * 1024 * 1024,
// --- new_string ---
    .size           = 512 * 1024 * 1024,
```

- [ ] **Step 2: 确认上下文**

Run: `grep -B5 -A5 "512 \* 1024 \* 1024" SDK/apps/earphone/device_config.c`

Expected: 该行在 `#if TCFG_NANDFLASH_DEV_ENABLE` 保护的 `nandflash_dev_data` 结构体内。

- [ ] **Step 3: 提交**

```bash
git add SDK/apps/earphone/device_config.c
git commit -m "$(cat <<'EOF'
config: align NAND platform size to GD5F4GM7 512MB capacity

Co-Authored-By: Claude Opus 4.7 <noreply@anthropic.com>
EOF
)"
```

---

### Task 3: nandflash.c — 运行时几何参数化 + GD5F4GM7UE 支持

**文件:**
- 修改: `SDK/apps/common/device/storage_device/nandflash/nandflash.c`

这是最核心的改动，分 7 个子步骤。

- [ ] **Step 1: 扩展 struct + 移除硬编码 #define（第 22-44 行）**

```c
// --- old_string ---
#define NAND_FLASH_TIMEOUT			1000000
#define MAX_NANDFLASH_PART_NUM      4
#define NAND_PAGE_SIZE 				2048
/* #define NAND_PAGE_SIZE 					2048+128 */
#define NAND_BLOCK_SIZE    			0x20000

#define XT26G01C                    0x0b11
#define XT26G02C                    0x2c24
#define XCSP4AAWH                   0x9cb1
#define F35SQA002G                  0xcd72
#define F35SQA001G                  0xcd71
#define F35SQA512M                  0xcd70
#define ZB35Q01C                  0x5e41


struct nandflash_data {
    u8 ecc_mask;
    u8 ecc_err;
    u8 plane_select;
    u8 write_enable_position;//写使能的位置，1表示在写入缓存区指令前进行写使能，0表示在写入缓存区指令后写使能
    u8 quad_mode_dummy_num: 4; //0XEB指令dummy数量
    u8 quad_mode_qe: 1; //featrue(0xb0):bit0:QE
    u16 block_number;
    u32 capacity;
};
// --- new_string ---
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
```

- [ ] **Step 2: 替换所有 NAND_PAGE_SIZE → nand_flash.page_size**

Step 1 已删除 `#define NAND_PAGE_SIZE`，现在用 replace_all 替换代码中所有残余出现（含注释）：

- `old_string`: `NAND_PAGE_SIZE` → `new_string`: `nand_flash.page_size` (replace_all=true)

- [ ] **Step 3: 替换所有 NAND_BLOCK_SIZE → nand_flash.block_size**

同上，Step 1 已删除 `#define NAND_BLOCK_SIZE`：

- `old_string`: `NAND_BLOCK_SIZE` → `new_string`: `nand_flash.block_size` (replace_all=true)

- [ ] **Step 3.5: 验证无残留**

Run: `grep -n "NAND_PAGE_SIZE\|NAND_BLOCK_SIZE" SDK/apps/common/device/storage_device/nandflash/nandflash.c`

Expected: 无输出。如有残留，说明 replace_all 遗漏了非连续 token，需手动修复。

- [ ] **Step 4: 为现有 7 个 switch case 补充 page_size/block_size 赋值**

每个 case 的 `nand_flash.capacity = ...;` 之后、`break;` 之前增加两行。逐 case 精确编辑：

XT26G01C (第 980 行之后):
```c
// --- old_string ---
            nand_flash.capacity = 128 * 1024 * 1024;
            break;
        case XT26G02C :
// --- new_string ---
            nand_flash.capacity = 128 * 1024 * 1024;
            nand_flash.page_size = 2048;
            nand_flash.block_size = 128 * 1024;
            break;
        case XT26G02C :
```

XT26G02C (第 990 行之后):
```c
// --- old_string ---
            nand_flash.capacity = 256 * 1024 * 1024;
            break;
        case XCSP4AAWH :
// --- new_string ---
            nand_flash.capacity = 256 * 1024 * 1024;
            nand_flash.page_size = 2048;
            nand_flash.block_size = 128 * 1024;
            break;
        case XCSP4AAWH :
```

XCSP4AAWH (第 1000 行之后):
```c
// --- old_string ---
            nand_flash.capacity = 512 * 1024 * 1024;
            break;
        case F35SQA512M :
// --- new_string ---
            nand_flash.capacity = 512 * 1024 * 1024;
            nand_flash.page_size = 2048;
            nand_flash.block_size = 128 * 1024;
            break;
        case F35SQA512M :
```

F35SQA512M (第 1010 行之后):
```c
// --- old_string ---
            nand_flash.capacity = 64 * 1024 * 1024;
            break;
        case F35SQA001G :
// --- new_string ---
            nand_flash.capacity = 64 * 1024 * 1024;
            nand_flash.page_size = 2048;
            nand_flash.block_size = 128 * 1024;
            break;
        case F35SQA001G :
```

F35SQA001G (第 1020 行之后):
```c
// --- old_string ---
            nand_flash.capacity = 128 * 1024 * 1024;
            break;
        case F35SQA002G :
// --- new_string ---
            nand_flash.capacity = 128 * 1024 * 1024;
            nand_flash.page_size = 2048;
            nand_flash.block_size = 128 * 1024;
            break;
        case F35SQA002G :
```

F35SQA002G (第 1030 行之后):
```c
// --- old_string ---
            nand_flash.capacity = 256 * 1024 * 1024;
            break;
        case ZB35Q01C:
// --- new_string ---
            nand_flash.capacity = 256 * 1024 * 1024;
            nand_flash.page_size = 2048;
            nand_flash.block_size = 128 * 1024;
            break;
        case ZB35Q01C:
```

ZB35Q01C (第 1040 行之后):
```c
// --- old_string ---
            nand_flash.capacity = 128 * 1024 * 1024;
            break;
        }

//power-up:need set featurea[A0]=00h
// --- new_string ---
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
```

- [ ] **Step 5: 添加 GD5F4GM7UE 的 feature 寄存器初始化**

```c
// --- old_string ---
//power-up:need set featurea[A0]=00h
        nand_set_features(0xA0, 0x00);
// --- new_string ---
//power-up:need set featurea[A0]=00h
        if (_nandflash.flash_id == GD5F4GM7UE) {
            // GD5F4GM7UE: ECC_EN=1 (0xB0 bit4), protect all blocks
            // 0xA0: BP3=0,BP2=0,BP1=1,BP0=1 → 0x1C
            // 0xB0: HSE=1,QE=0,ECC_EN=1 (bit4) → 0x11
            nand_set_features(0xA0, 0x1C);
            nand_set_features(0xB0, 0x11);
        } else {
            nand_set_features(0xA0, 0x00);
        }
```

注意：使用十六进制 `0x1C` / `0x11` 而非 `0b00011100` / `0b00010001`，避免旧工具链对二进制字面量的兼容问题。

- [ ] **Step 6: 在 nandflash_dev_ops 之前添加查询接口**

```c
// --- old_string ---
static int nandflash_dev_ioctl(struct device *device, u32 cmd, u32 arg)
{
    struct nandflash_partition *part = device->private_data;
    if (!part) {
        log_error("nandflash partition invalid\n");
        return -EFAULT;
    }
    return _nandflash_ioctl(cmd, arg, 512,  part);
}
const struct device_operations nandflash_dev_ops = {
// --- new_string ---
static int nandflash_dev_ioctl(struct device *device, u32 cmd, u32 arg)
{
    struct nandflash_partition *part = device->private_data;
    if (!part) {
        log_error("nandflash partition invalid\n");
        return -EFAULT;
    }
    return _nandflash_ioctl(cmd, arg, 512,  part);
}

#include "ftl_api.h"

void nandflash_get_ftl_info(struct ftl_nand_flash *ftl)
{
    ftl->block_begin = 0;
    ftl->block_end = nand_flash.block_number;
    ftl->logic_block_num = nand_flash.block_number * 93 / 100;
    ftl->page_num = 64;
    ftl->page_size = nand_flash.page_size;
    ftl->oob_offset = 0;
    ftl->oob_size = (nand_flash.page_size == 4096) ? 128 : 64;
    ftl->max_erase_cnt = 100000;
}

u16 nandflash_get_flash_id(void)
{
    return _nandflash.flash_id;
}

const struct device_operations nandflash_dev_ops = {
```

- [ ] **Step 7: 提交**

```bash
git add SDK/apps/common/device/storage_device/nandflash/nandflash.c
git commit -m "$(cat <<'EOF'
nandflash: runtime geometry parameterization for GD5F4GM7UE

- Replace hardcoded NAND_PAGE_SIZE/NAND_BLOCK_SIZE with runtime fields
- Add page_size/block_size to nandflash_data struct
- Add GD5F4GM7UE (0xC894) case with 4KB page/256KB block config
- Add GD5F4GM7UE feature register init (ECC_EN + protect)
- Add nandflash_get_ftl_info() / nandflash_get_flash_id() query APIs

Co-Authored-By: Claude Opus 4.7 <noreply@anthropic.com>
EOF
)"
```

---

### Task 4: nandflash.h — 新增查询接口声明

**文件:**
- 修改: `SDK/apps/common/include/nandflash.h`

- [ ] **Step 1: 在 NAND_Result 枚举后、nandflash_dev_platform_data 前插入声明**

```c
// --- old_string ---
	NAND_Result nand_flash_page_write(uint32_t block, uint32_t page, uint32_t offset, uint8_t *data, uint32_t data_size);
	NAND_Result nand_flash_page_read(uint32_t block, uint32_t page, uint32_t column_address, uint8_t *data, uint32_t data_size);
	NAND_Result nand_flash_erase(uint32_t addr);

	#endif
// --- new_string ---
	NAND_Result nand_flash_page_write(uint32_t block, uint32_t page, uint32_t offset, uint8_t *data, uint32_t data_size);
	NAND_Result nand_flash_page_read(uint32_t block, uint32_t page, uint32_t column_address, uint8_t *data, uint32_t data_size);
	NAND_Result nand_flash_erase(uint32_t addr);

	// FTL 参数查询接口
	struct ftl_nand_flash;
	void nandflash_get_ftl_info(struct ftl_nand_flash *ftl);
	u16 nandflash_get_flash_id(void);

	#endif
```

- [ ] **Step 2: 提交**

```bash
git add SDK/apps/common/include/nandflash.h
git commit -m "$(cat <<'EOF'
nandflash.h: declare FTL parameter query interface

- nandflash_get_ftl_info() — populate ftl_nand_flash struct
- nandflash_get_flash_id() — return detected chip ID

Co-Authored-By: Claude Opus 4.7 <noreply@anthropic.com>
EOF
)"
```

---

### Task 5: ftl_device.c — 通过查询接口解耦硬编码参数

**文件:**
- 修改: `SDK/apps/common/device/storage_device/nandflash/ftl_device.c`

- [ ] **Step 1: 替换硬编码 flash 参数为 nandflash_get_ftl_info() 调用**

`ftl_dev_open()` 函数中（第 62-95 行）:

```c
// --- old_string ---
static int ftl_dev_open(const char *name, struct device **device, void *arg)
{
    dev_open("nand_flash", NULL);

    struct ftl_nand_flash flash;
    flash.block_begin = 0;
    flash.block_end = 1024;
    flash.logic_block_num = 1000;
    flash.page_num = 64;
    flash.page_size = 2 * 1024;
    flash.oob_offset = 0;
    flash.oob_size = 64;
    flash.max_erase_cnt = 10 * 10000;
    flash.page_size_shift = get_first_one(flash.page_size);
    flash.block_size_shift = get_first_one(flash.page_size * flash.page_num);
    flash.page_read = ftl_port_page_read;
    flash.page_write = ftl_port_page_write;
    flash.erase_block = ftl_port_erase_block;

    g_capacity = flash.logic_block_num << (flash.block_size_shift - 9);

    struct ftl_config config = {
        .page_buf_num = 4,
        .delayed_write_msec = 100,
    };
    ftl_init(&flash, &config);
    *device = &ftl_device;
    return 0;
}
// --- new_string ---
static int ftl_dev_open(const char *name, struct device **device, void *arg)
{
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
        .page_buf_num = 4,
        .delayed_write_msec = 100,
    };
    ftl_init(&flash, &config);
    *device = &ftl_device;
    return 0;
}
```

- [ ] **Step 2: 修正 IOCTL_GET_ID 返回真实 chip ID**

`ftl_dev_ioctl()` 函数中:

```c
// --- old_string ---
    case IOCTL_GET_ID:
        *((u32 *)arg) = 0xcd71;
        break;
// --- new_string ---
    case IOCTL_GET_ID:
        *((u32 *)arg) = nandflash_get_flash_id();
        break;
```

- [ ] **Step 3: 提交**

```bash
git add SDK/apps/common/device/storage_device/nandflash/ftl_device.c
git commit -m "$(cat <<'EOF'
ftl_device: decouple hardcoded params via nandflash_get_ftl_info()

- Replace hardcoded flash geometry with runtime query
- IOCTL_GET_ID returns actual chip ID instead of fixed 0xcd71

Co-Authored-By: Claude Opus 4.7 <noreply@anthropic.com>
EOF
)"
```

---

### Task 6: genFileList.c — 移除 CONFIG_SOUNDBOX_CASE 守卫

**文件:**
- 修改: `SDK/build/genFileList.c:1446-1452`

- [ ] **Step 1: 删除 #ifdef / #endif 守卫**

```c
// --- old_string ---
#ifdef CONFIG_SOUNDBOX_CASE
#if TCFG_NANDFLASH_DEV_ENABLE
c_SRC_FILES += \
	apps/common/device/storage_device/nandflash/nandflash.c \
	apps/common/device/storage_device/nandflash/ftl_device.c
#endif
#endif
// --- new_string ---
#if TCFG_NANDFLASH_DEV_ENABLE
c_SRC_FILES += \
	apps/common/device/storage_device/nandflash/nandflash.c \
	apps/common/device/storage_device/nandflash/ftl_device.c
#endif
```

- [ ] **Step 2: 提交**

```bash
git add SDK/build/genFileList.c
git commit -m "$(cat <<'EOF'
build: compile NAND source files for all product types

Remove CONFIG_SOUNDBOX_CASE guard so earphone projects
can use nandflash.c and ftl_device.c source files.

Co-Authored-By: Claude Opus 4.7 <noreply@anthropic.com>
EOF
)"
```

---

### Task 7: 编译验证

- [ ] **Step 1: 清理并编译**

Run: `cd SDK/ && make clean && make 2>&1 | tee build.log`

Expected: `sdk.elf` 成功生成，无 NAND/FTL/FAT 相关链接错误。

- [ ] **Step 2: 确认关键符号存在**

Run: `grep -c "nandflash_dev_ops\|ftl_dev_ops\|nandflash_get_ftl_info\|nandflash_get_flash_id\|get_first_one" SDK/cpu/br28/tools/sdk.map`

Expected: 5 个符号均可查到，`nandflash_*` 和 `get_first_one` 均已链接。若 `get_first_one` 缺失，需在 `ftl_device.c` 中改用 `__builtin_ctz()` 替换。

- [ ] **Step 3: 速查关键符号绑定**

Run: `grep -E "nandflash_get_ftl_info|nandflash_get_flash_id|nandflash_dev_ops|ftl_dev_ops" SDK/cpu/br28/tools/sdk.map`

Expected: `.map` 中能看到所有符号地址（已解析），无 `UND` 或 `*UND*` 标记。

---

### 后续验证（不在本次代码改动范围）

上述 7 个 Task 完成编译验证后，需在硬件上进行 bring-up 验证。详见 spec §9 验证计划。

关键验证路径：
1. Read ID → 确认 `0xC894` 被正确识别
2. Raw NAND 几何 → page/block 边界测试
3. FTL 初始化 → blank chip 首次 mount
4. FAT 读写 → f_format + 文件创建 + 重启持久化
5. 现有芯片回归 → 若手边有 XT26G01C/F35SQA001G 等旧芯片，验证几何参数化后读写/擦除仍正常（`page_size=2048` / `block_size=128*1024` 应等价于原硬编码宏）
