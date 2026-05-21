# SD NAND Flash 接入指南（与 SPI NAND 共存方案）

> **目标**：在同一套硬件 PCB（CLK=PC04, DAT0=PC03, CMD=PC05）上，通过编译期宏 `STORAGE_TYPE` 在 SPI NAND 和 SD NAND 之间切换，两套方案各自独立编译、互不污染。

---

## 1. 硬件接线

SD NAND 沿用了 SPI NAND 的同一组 GPIO，**仅语义不同**：

```
                   SPI NAND 模式                 SD NAND 模式
              ┌──────────────────┐       ┌──────────────────┐
  PC04 ───────┤ CLK  (SCLK)      │       │ CLK  (SD_CLK)    │
  PC03 ───────┤ DO   (MOSI/Data) │  →    │ DAT0 (SD_DATA0)  │
  PC05 ───────┤ CS   (Chip Sel)  │       │ CMD  (SD_CMD)    │
              └──────────────────┘       └──────────────────┘
```

| GPIO | SPI NAND 功能 | SD NAND 功能 | 说明 |
|:----:|:------------:|:-----------:|------|
| PC04 | SPI1 CLK | SD0 CLK | 时钟，两种协议共用 |
| PC03 | SPI1 DO（半双工收发） | SD0 DAT0 | 数据线，方向由硬件控制器自动切换 |
| PC05 | SPI1 CS（片选） | SD0 CMD | SPI 的片选变成 SD 的命令/响应线 |

> **硬件注意**：SD NAND 的 CMD 和 DAT0 需要通过 10KΩ 上拉到 VDD，DAT1~3 也必须上拉（即使未接 MCU）。详见 `SD_NANDFLASH接入JL平台可行性分析.md` 3.1.2 节。

---

## 2. 存储方案选择宏

在 `board_ac701n_demo_cfg.h` 中定义顶层选择宏：

```c
// board_ac701n_demo_cfg.h — 在包含 sdk_config.h 之前定义

// 存储方案选择：0=SPI NAND, 1=SD NAND
#define CONFIG_STORAGE_TYPE_SPI_NAND      0
#define CONFIG_STORAGE_TYPE_SD_NAND       1

#define CONFIG_STORAGE_TYPE               CONFIG_STORAGE_TYPE_SD_NAND
```

`CONFIG_STORAGE_TYPE` 是整个文件的**唯一开关**——改为 `CONFIG_STORAGE_TYPE_SPI_NAND` 即切回 SPI NAND。

---

## 3. 板级配置（board_ac701n_demo_cfg.h）

用条件编译控制 SPI 和 SD 两套配置，互斥生效：

```c
// board_ac701n_demo_cfg.h — 存储方案分段配置

//*********************************************************************************//
//                         SPI 硬件配置（SPI NAND 模式使用）                         //
//*********************************************************************************//
#if (CONFIG_STORAGE_TYPE == CONFIG_STORAGE_TYPE_SPI_NAND)
#define TCFG_HW_SPI1_ENABLE                 ENABLE_THIS_MOUDLE
#else
#define TCFG_HW_SPI1_ENABLE                 DISABLE_THIS_MOUDLE
#endif
#define TCFG_HW_SPI1_PORT_CLK               IO_PORTC_04      // 两种模式共用
#define TCFG_HW_SPI1_PORT_DO                IO_PORTC_03      // 两种模式共用
#define TCFG_HW_SPI1_PORT_DI                NO_CONFIG_PORT   // 半双工：DO 同时收发
#define TCFG_HW_SPI1_BAUD                   2000000L
#define TCFG_HW_SPI1_MODE                   SPI_MODE_UNIDIR_1BIT  // 单线半双工
#define TCFG_HW_SPI1_ROLE                   SPI_ROLE_MASTER

//*********************************************************************************//
//                           FLASH 存储方案（二选一）                                //
//*********************************************************************************//
#if (CONFIG_STORAGE_TYPE == CONFIG_STORAGE_TYPE_SPI_NAND)
// ---- SPI NAND 方案 ----
#define TCFG_NANDFLASH_DEV_ENABLE           ENABLE_THIS_MOUDLE
#define TCFG_FLASH_DEV_SPI_HW_NUM           1
#define TCFG_FLASH_DEV_SPI_CS_PORT          IO_PORTC_05      // SPI CS
#define TCFG_FLASH_DEV_FLASH_READ_WIDTH     1

#elif (CONFIG_STORAGE_TYPE == CONFIG_STORAGE_TYPE_SD_NAND)
// ---- SD NAND 方案 ----
#define TCFG_NANDFLASH_DEV_ENABLE           DISABLE_THIS_MOUDLE
// SD 硬件配置宏（供 sdk_config.h 和 device_config.c 使用）
#define TCFG_SD0_DEF_DAT_MODE               1               // 1-bit 单线模式
#define TCFG_SD0_DEF_CLK                    12000000        // 12MHz
#define TCFG_SD0_DEF_PORT_CMD               IO_PORTC_05     // ← CS 变 CMD
#define TCFG_SD0_DEF_PORT_CLK               IO_PORTC_04     // ← 同上
#define TCFG_SD0_DEF_PORT_DA0               IO_PORTC_03     // ← DO 变 DAT0
#define TCFG_SD0_DEF_PORT_DA1               NO_CONFIG_PORT  // 1-bit 不用
#define TCFG_SD0_DEF_PORT_DA2               NO_CONFIG_PORT
#define TCFG_SD0_DEF_PORT_DA3               NO_CONFIG_PORT
#define TCFG_SD0_DEF_POWER_SEL              SD_PWR_NULL

// [关键] SD NAND 焊死在 PCB 上，跳过插拔检测，直接强制在线挂载
#define TCFG_SD_ALWAY_ONLINE_ENABLE         ENABLE_THIS_MOUDLE

#endif // CONFIG_STORAGE_TYPE
```

---

## 4. SDK 配置（sdk_config.h）

需要修改 `sdk_config.h` 中的 SD0 配置段，使其接受 `board_ac701n_demo_cfg.h` 传入的配置宏：

```c
// sdk_config.h — 将原来的硬编码替换为宏引用

#define TCFG_SD0_ENABLE                   CONFIG_STORAGE_TYPE
                                                        // 0=SPI NAND 模式, 1=SD NAND 模式
#if TCFG_SD0_ENABLE
#define TCFG_SD0_DAT_MODE                 TCFG_SD0_DEF_DAT_MODE
#define TCFG_SD0_DET_MODE                 SD_CMD_DECT      // CMD 线检测
#define TCFG_SD0_CLK                      TCFG_SD0_DEF_CLK
#define TCFG_SD0_DET_IO                   NO_CONFIG_PORT
#define TCFG_SD0_DET_IO_LEVEL             0
#define TCFG_SD0_POWER_SEL                TCFG_SD0_DEF_POWER_SEL
#define TCFG_SDX_CAN_OPERATE_MMC_CARD     0               // 不支持 MMC
#define TCFG_KEEP_CARD_AT_ACTIVE_STATUS   0
#define TCFG_SD0_PORT_CMD                 TCFG_SD0_DEF_PORT_CMD
#define TCFG_SD0_PORT_CLK                 TCFG_SD0_DEF_PORT_CLK
#define TCFG_SD0_PORT_DA0                 TCFG_SD0_DEF_PORT_DA0
#define TCFG_SD0_PORT_DA1                 TCFG_SD0_DEF_PORT_DA1
#define TCFG_SD0_PORT_DA2                 TCFG_SD0_DEF_PORT_DA2
#define TCFG_SD0_PORT_DA3                 TCFG_SD0_DEF_PORT_DA3
#endif // TCFG_SD0_ENABLE
```

> **原理**：`TCFG_SD0_ENABLE` 的值等于 `CONFIG_STORAGE_TYPE`（SPI NAND 模式为 `0`，SD NAND 模式为 `1`）。`#if TCFG_SD0_ENABLE` 和 `#if 0` 效果相同——预处理器会将值为 `0` 的宏判为假，整个 SD 代码段被排除。

---

## 5. 设备注册（device_config.c）

SD0 注册代码**已存在，无需修改**（`device_config.c:17-51`）——条件编译自动根据 `TCFG_SD0_DET_MODE` 和 `TCFG_SD0_POWER_SEL` 选择正确的 `detect_func` 和 `power`：

```c
// device_config.c — 已有代码，条件编译自动生效（行号 17-51）
#if TCFG_SD0_ENABLE
SD0_PLATFORM_DATA_BEGIN(sd0_data) = {
    .port = {
        TCFG_SD0_PORT_CMD,   // → IO_PORTC_05
        TCFG_SD0_PORT_CLK,   // → IO_PORTC_04
        TCFG_SD0_PORT_DA0,   // → IO_PORTC_03
        TCFG_SD0_PORT_DA1,   // → NO_CONFIG_PORT
        TCFG_SD0_PORT_DA2,   // → NO_CONFIG_PORT
        TCFG_SD0_PORT_DA3,   // → NO_CONFIG_PORT
    },
    .data_width     = TCFG_SD0_DAT_MODE,   // → 1
    .speed          = TCFG_SD0_CLK,        // → 12000000
    .detect_mode    = TCFG_SD0_DET_MODE,   // → SD_CMD_DECT
    .priority       = 3,
#if (TCFG_SD0_DET_MODE == SD_IO_DECT)
    .detect_io      = TCFG_SD0_DET_IO,
    .detect_io_level = TCFG_SD0_DET_IO_LEVEL,
    .detect_func    = sdmmc_0_io_detect,
#elif (TCFG_SD0_DET_MODE == SD_CLK_DECT)
    .detect_io_level = TCFG_SD0_DET_IO_LEVEL,
    .detect_func    = sdmmc_0_clk_detect,
#else
    .detect_func    = sdmmc_cmd_detect,     // ← DET_MODE=SD_CMD_DECT 走此分支
#endif
#if (TCFG_SD0_POWER_SEL == SD_PWR_SDPG)
    .power          = sd_set_power,
#else
    .power          = NULL,                 // ← POWER_SEL=SD_PWR_NULL 走此分支
#endif
    SD0_PLATFORM_DATA_END()
};
#endif

REGISTER_DEVICES(device_table) = {
#if TCFG_SD0_ENABLE
    { "sd0", &sd_dev_ops, (void *)&sd0_data },
#endif
#if TCFG_NANDFLASH_DEV_ENABLE
    {"nand_flash",   &nandflash_dev_ops, (void *)&nandflash_dev_data},
    {"nandflash_ftl", &ftl_dev_ops,      (void *)"nand_flash"},
#endif
};
```

SPI NAND 和 SD NAND 的设备注册互斥——`CONFIG_STORAGE_TYPE` 决定哪个分支生效。

---

## 6. 逻辑盘注册（dev_reg.c）

**无需修改**——`dev_reg.c` 中两份注册项都已存在：

```c
// dev_reg.c:42-50 — SD NAND
#if (TCFG_SD0_ENABLE)
    { "sd0", "sd0", "storage/sd0", "storage/sd0/C/", "fat" },
#endif

// dev_reg.c:159-166 — SPI NAND
#if (defined TCFG_NANDFLASH_DEV_ENABLE && TCFG_NANDFLASH_DEV_ENABLE)
    { "nandflash_ftl", "nandflash_ftl", "storage/nandflash_ftl",
      "storage/nandflash_ftl/C/", "fat" },
#endif
```

条件编译保证只有一个生效。

---

## 7. 设备管理器（dev_manager.c）

**无需修改**。`dev_manager_task()` 中两个分支已经用条件编译隔离：

```c
// dev_manager.c — SPI NAND 路径（第 1084 行）
#if (defined TCFG_NANDFLASH_DEV_ENABLE && TCFG_NANDFLASH_DEV_ENABLE)
    dev_manager_add("nandflash_ftl");
#endif

// dev_manager.c — SD NAND 路径（第 1029 行）
#if TCFG_SD_ALWAY_ONLINE_ENABLE
#if (defined(TCFG_SD0_ENABLE) && (TCFG_SD0_ENABLE))
    force_set_sd_online("sd0");
    int err = dev_manager_add("sd0");
    sdx_dev_detect_timer_del();
#endif
#endif
```

---

## 8. 应用层访问路径

两种方案的访问路径不同，但应用代码可以通过条件编译或运行时判断适配：

```c
// SPI NAND 路径
#define STORAGE_ROOT_PATH   "storage/nandflash_ftl/C/"

// SD NAND 路径
#define STORAGE_ROOT_PATH   "storage/sd0/C/"

// 方案一：编译期路径
#if (CONFIG_STORAGE_TYPE == CONFIG_STORAGE_TYPE_SD_NAND)
  #define STORAGE_ROOT_PATH "storage/sd0/C/"
#else
  #define STORAGE_ROOT_PATH "storage/nandflash_ftl/C/"
#endif

// 文件操作（两种方案完全相同）
FILE *fp = fopen(STORAGE_ROOT_PATH "test.txt", "w+");
fwrite("data", 1, 4, fp);
fclose(fp);
```

> 也可以使用 `dev_manager_get_root_path_by_logo("sd0")` 或 `dev_manager_get_root_path_by_logo("nandflash_ftl")` 在运行时获取根路径，避免编译期绑死。

---

## 9. 修改文件汇总

| # | 文件 | 性质 | 修改内容 |
|---|------|:---:|---------|
| 1 | `board_ac701n_demo_cfg.h` | **修改** | 新增 `CONFIG_STORAGE_TYPE` 选择宏；SPI1 和 FLASH 段加条件编译；SD NAND 分支新增所有 SD 硬件配置宏 |
| 2 | `sdk_config.h` | **修改** | SD0 段配置值改为引用 `TCFG_SD0_DEF_*` 宏；`TCFG_SD0_ENABLE` 改为 `CONFIG_STORAGE_TYPE`（随方案切换自动为 0 或 1） |
| 3 | `device_config.c` | 不修改 | SD0 注册代码已存在，条件编译自动生效 |
| 4 | `dev_reg.c` | 不修改 | 两份注册项已存在，条件编译自动生效 |
| 5 | `dev_manager.c` | 不修改 | 两份挂载路径已存在，条件编译自动生效 |

**总计：修改 2 个文件，不写任何新驱动代码。**

---

## 10. 切换方案

```
当前用 SPI NAND？                    → CONFIG_STORAGE_TYPE = CONFIG_STORAGE_TYPE_SPI_NAND
准备切 SD NAND？                    → CONFIG_STORAGE_TYPE = CONFIG_STORAGE_TYPE_SD_NAND

make clean && make                  → 编译出对应方案的固件
```

**注意事项**：
- 切换方案后建议 `make clean`，确保旧方案的 `.o` 文件和宏缓存不残留
- SD NAND 出厂已 FAT32 格式化，**不需要** `f_format()`
- SD NAND 模式下的 `storage/sd0/C/` 与 SPI NAND 模式下的 `storage/nandflash_ftl/C/` 是两个独立的命名空间，应用层路径宏需同步切换

---

## 11. 调试检查清单

SD NAND 模式下若挂载失败，按以下顺序排查：

1. **上拉电阻** — CMD 和 DAT0~3 是否全部通过 10KΩ 上拉到 VDD？（DAT1~3 即使不接 MCU 也必须上拉）
2. **CLK 串联电阻** — CLK 线上是否串接 33Ω 左右电阻？
3. **宏配置** — `CONFIG_STORAGE_TYPE` 是否为 `CONFIG_STORAGE_TYPE_SD_NAND`？
4. **`TCFG_SD_ALWAY_ONLINE_ENABLE`** — 是否确认为 `ENABLE_THIS_MOUDLE`？
5. **SPI1 关闭** — SPI NAND 模式下 `TCFG_HW_SPI1_ENABLE` 是否为 `DISABLE_THIS_MOUDLE`？（避免 GPIO 冲突）
6. **逻辑分析仪** — 抓 CLK/CMD/DAT0 三条线波形，确认 ≥74 个虚拟时钟 + CMD0 序列已正常发出并收到响应
