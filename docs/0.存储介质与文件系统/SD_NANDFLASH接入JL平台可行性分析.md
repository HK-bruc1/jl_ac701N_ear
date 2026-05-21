# 3. JL平台对SD NAND Flash的支持

> **本文档分析 SD NAND Flash（以 XCSD02G-MJ 为例）接入 JL AC701N/BR28 平台的技术方案。** SD NAND 是内部集成 SD2.0 控制器的 NAND Flash 芯片，对外呈现标准 SD 卡接口。与 SPI NAND 方案相比，SD NAND 无需主机端 FTL——坏块管理、磨损均衡、ECC 全部由芯片内部控制器处理，架构显著简化。

## 1. 能力边界总览

### 1.1 JL已提供的能力（开箱即用）

| 能力项 | 说明 | 实现形式 |
|--------|------|----------|
| SD/MMC 主机驱动 | SD 协议初始化、命令/响应收发、块读写 | 闭源 `sd_dev_ops`（在 `device.a` 中） |
| SD 卡检测 | CMD 检测、CLK 检测、IO 检测三种方式 | 框架代码（源码可见）：`sdmmc_cmd_detect`、`sdmmc_0_clk_detect`、`sdmmc_0_io_detect` |
| 1-bit / 4-bit 总线宽度 | 上电默认 1-bit，可通过 ACMD6 切换 4-bit | `TCFG_SD0_DAT_MODE` 配置宏 |
| VFS 文件系统挂载 | 将 SD 设备挂载为 FAT12/16/32/exFAT | 框架代码（源码可见） |
| USB Mass Storage | SD NAND 可以作为 U 盘暴露给 Host | 框架代码（源码可见） |
| OTA 固件升级 | 支持从 SD 卡读取升级文件 | 框架代码（源码可见） |
| 设备管理器集成 | 支持热插拔检测、mount/unmount 管理 | 框架代码（源码可见） |
| 强制在线模式 | 跳过插拔检测，直接挂载（焊死芯片场景） | `TCFG_SD_ALWAY_ONLINE_ENABLE` 宏 + `force_set_sd_online()` |

**一句话**：SD NAND 接入 JL 平台**不需要编写任何新驱动代码**。BR28 的 SD/MMC 主机控制器（`JL_SD0`）硬件 + `sd_dev_ops` 驱动已完整覆盖 SD2.0 协议。开发者只需配置引脚和宏即可使用。

### 1.2 与 SPI NAND 方案的关键差异

| 维度 | SPI NAND | SD NAND |
|------|----------|---------|
| 裸驱动 | `nandflash.c`（~1300 行，需从 soundbox SDK 获取） | `sd_dev_ops`（闭源，SDK 自带） |
| FTL 层 | 需要——`ftl.a` 或 `nandflash_ftl.c`（~1400 行） | **不需要**——芯片内置 |
| 设备注册 | 双设备（`nand_flash` + `nandflash_ftl`） | 单设备（`sd0`） |
| 文件系统 | FAT，需手动 `f_format()` 首次格式化 | FAT32，**出厂预格式化** |
| 坏块管理 | FTL 软件管理 | 芯片内部硬件 |
| 磨损均衡 | FTL 软件管理 | 芯片内部硬件 |
| ECC | FTL 软件管理 | 芯片内部 8bit/512B |
| 物理接口 | SPI（CLK/DO/DI/CS，4 线） | SDIO（CLK/CMD/DAT0，3 线 + 上拉） |
| 最高读速 | 取决于 SPI 时钟（2MHz → ~250KB/s） | 最高 21MB/s |
| 最高写速 | 取决于 SPI 时钟 + 擦除时间 | ~4.6MB/s |
| 驱动源码可见性 | ✅ 裸驱动 + FTL 源码可见（从 soundbox 获取后） | ❌ `sd_dev_ops` 闭源，在 `device.a` 中 |
| Host CPU 负载 | 高（FTL 磨损均衡/垃圾回收计算） | 低（仅块读写） |
| 初始化时间 | ~100ms（复位 + ID 识别 + FTL 扫描） | ~1s（SD 协议初始化 + FAT 挂载） |
| 首次使用流程 | 获取源文件 → 修复编译 → 配置 SPI → 格式化 → 挂载 | 配置引脚 → 使能宏 → 挂载（出厂已格式化） |

### 1.3 SD NAND vs SPI NAND 架构对比

```
SPI NAND 方案（需要 3 层）：
  App → VFS → FAT(fs.a) → FTL(nandflash_ftl.c) → SPI Driver(nandflash.c) → SPI HW → NAND 芯片

SD NAND 方案（只需 2 层）：
  App → VFS → FAT(fs.a) → SD Driver(sd_dev_ops) → SDIO HW(JL_SD0) → XCSD02G-MJ
```

SD NAND 少了一个 FTL 层，因为芯片内部 SD2.0 控制器已完整实现坏块管理、磨损均衡和 ECC。Host 端看到的是一个标准 FAT32 块设备。

---

## 2. 架构总览：从硬件到文件系统的分层

```
┌──────────────────────────────────────────────────┐
│                  应用层代码                        │
│  fopen / fread / fwrite / fclose / fdelete ...    │
│  fget_free_space("storage/sd0/C/", &free)        │
└────────────────────┬─────────────────────────────┘
                     │ VFS (Virtual File System)
                     │ fs.h / mount() / unmount()
┌────────────────────▼─────────────────────────────┐
│              FAT 文件系统层                        │
│  FAT12 / FAT16 / FAT32 / exFAT                    │
│  mount("sd0", "storage/sd0", "fat", ...)          │
│  ⚠️ 出厂预格式化，首次使用无需 f_format()         │
└────────────────────┬─────────────────────────────┘
                     │ dev_io_t 接口 (read/write/ioctrl)
┌────────────────────▼─────────────────────────────┐
│         SD 块设备驱动 (sd_dev_ops)                   │
│  闭源，在 device.a 中                              │
│  - SD 协议初始化 (CMD0/ACMD41/CMD2/CMD3/CMD7/CMD9) │
│  - 逻辑块读写 (512B 扇区)                          │
│  - 卡检测与状态管理                                 │
└────────────────────┬─────────────────────────────┘
                     │ SDIO 总线 (CLK, CMD, DAT0)
┌────────────────────▼─────────────────────────────┐
│        SD/MMC 主机控制器 (JL_SD0 寄存器)             │
│  BR28 硬件                                        │
└────────────────────┬─────────────────────────────┘
                     │ 物理引脚
┌────────────────────▼─────────────────────────────┐
│        XCSD02G-MJ SD NAND Flash 芯片              │
│  内置：SD2.0 控制器 + NAND 管理 (ECC/坏块/磨损)    │
└──────────────────────────────────────────────────┘
```

### 2.1 设备注册模型

SD NAND 在 JL 平台采用**单设备注册**模式——仅一个 `"sd0"` 设备，直接向上提供块设备接口：

```c
// device_config.c — 硬件设备注册（源码可见）
REGISTER_DEVICES(device_table) = {
#if TCFG_SD0_ENABLE
    { "sd0", &sd_dev_ops, (void *)&sd0_data },
#endif
    ...
};

// dev_reg.c — 逻辑盘注册（源码可见）
#if (TCFG_SD0_ENABLE)
    {
        /*logo*/         "sd0",
        /*name*/         "sd0",
        /*storage_path*/ "storage/sd0",
        /*root_path*/    "storage/sd0/C/",
        /*fs_type*/      "fat",
    },
#endif
```

与 SPI NAND 的"双设备"（`"nand_flash"` + `"nandflash_ftl"`）不同，SD NAND 只有一个逻辑设备。不需要中间 FTL 设备来桥接裸驱动和文件系统。

**文件系统直接挂载在 `"sd0"` 上。**

### 2.2 SD 协议初始化流程

SD NAND 的初始化流程与标准 SD 卡完全一致。SD 协议上电默认 1-bit 模式（仅 DAT0），无需任何特殊配置：

```
Step 1:  上电 → 发送 ≥74 个虚拟时钟周期（CMD 线保持高电平）
Step 2:  CMD0  (GO_IDLE_STATE)        → 复位到 Idle 状态
Step 3:  ACMD41 (SD_SEND_OP_COND)     → 查询工作电压范围，轮询 busy 位
Step 4:  CMD2  (ALL_SEND_CID)         → 获取 CID
Step 5:  CMD3  (SEND_RELATIVE_ADDR)   → 获取 RCA，每次上电随机变化
Step 6:  CMD7  (SELECT_CARD)          → 选中卡，进入传输状态
Step 7:  CMD9  (SEND_CSD)             → 获取 CSD（含容量信息）
Step 8:  ACMD6 (SET_BUS_WIDTH)        → [可选] 切换到 4-bit 模式
```

> **注意**：上述序列为 SD2.0 标准流程。由于 `sd_dev_ops` 闭源，实际初始化序列由驱动内部实现决定。规格书 5.4 节还建议了 `ACMD13`（读 SD Status）和 `CMD10`（再次读 CID）等可选步骤。

**对于 1-bit 模式：Step 8 可省略。** 上电默认就是 1-bit，不需要发送 ACMD6。

### 2.3 模式选择机制

SD NAND 支持 SD 模式和 SPI 模式两种操作模式。DAT3 引脚在上电时的电平决定模式：

| DAT3 电平 | 发送 CMD0 后进入 | 说明 |
|-----------|:---:|------|
| 高电平（上拉） | **SD 模式** | DAT3 通过 10K~100KΩ 上拉到 VDD |
| 低电平 | SPI 模式 | 需要 Host 主动拉低 |

由于 SD NAND 焊死在 PCB 上，DAT3 只接上拉电阻（未接 GPIO），上电后自然为高电平，芯片**自动进入 SD 模式**——完全符合预期。

### 2.4 文件系统支持

SD NAND 只支持 `"fat"` 这一种文件系统类型，这与 SPI NAND 一致。但 SD NAND 有一个关键优势：**出厂预格式化**。

XCSD02G-MJ 出厂时已格式化为 FAT32（规格书 4.2 节），用户可直接使用 240MB（02G 型号）。这意味着：

- SPI NAND：首次使用需要 `f_format("nandflash_ftl", "fat", 4096)` 手动格式化
- SD NAND：挂载即用，不需要格式化步骤

### 2.5 文件系统-设备配对总表（含 SD NAND）

从 `dev_reg.c` 汇总 JL 平台所有存储设备与文件系统的配对关系：

| 设备 logo | 物理设备 | 文件系统 | 挂载点 | 根路径 | 首次格式化 |
|-----------|---------|----------|--------|--------|:---:|
| `SDFILE_DEV` ("sdfile") | 芯片内置 Flash | `sdfile` | `flash` | `flash/res/` | PC 端工具 |
| `"rec_sdfile"` | 芯片内置 Flash (录音) | `rec_sdfile` | `mnt/rec_sdfile` | `mnt/rec_sdfile/C/` | 不需要 |
| **`"sd0"`** | **SD NAND / SD 卡** | **`fat`** | **`storage/sd0`** | **`storage/sd0/C/`** | **出厂已格式化** |
| `"sd1"` | SD 卡 1 | `fat` | `storage/sd1` | `storage/sd1/C/` | 首次需格式化 |
| `"udisk0"` | USB U 盘 | `fat` | `storage/udisk0` | `storage/udisk0/C/` | 首次需格式化 |
| `"fat_nor"` | 外挂 NOR Flash (FAT) | `fat` | `storage/fat_nor` | `storage/fat_nor/C/` | 首次需格式化 |
| `"res_nor"` | 外挂 NOR Flash (资源) | `nor_sdfile` | `storage/res_nor` | `storage/res_nor/C/` | PC 端工具 |
| `"rec_nor"` | 外挂 NOR Flash (录音) | `rec_fs` | `storage/rec_nor` | `storage/rec_nor/C/` | 不需要 |
| `"vir_udisk0"` | 虚拟 U 盘 | `fat` | `storage/vir_udisk0` | `storage/vir_udisk0/C/` | 不需要 |
| `"virfat_flash"` | Flash 虚拟 FAT | `sdfile_fat` | `storage/virfat_flash` | `storage/virfat_flash/C/` | 不需要 |
| `"nandflash_ftl"` | SPI NAND Flash (FTL) | `fat` | `storage/nandflash_ftl` | `storage/nandflash_ftl/C/` | 首次需 `f_format()` |

---

## 3. 使用SD NAND需要做的事情

### 3.1 第一步：硬件连接

#### 3.1.1 最小连接方案（1-bit 模式）

SD NAND 在 1-bit 模式下只需 **5 个物理连接**：GND、CMD、VDD、CLK、DAT0。

```
         ┌─────────────────────┐
  DAT2 ──┤1 (上拉到 VDD)   8├── VDD (3.3V + 2.2μF 去耦)
  DAT3 ──┤2 (上拉到 VDD)   7├── DAT1 (上拉到 VDD)
   CLK ──┤3 (33Ω 串联)     6├── DAT0 → MCU GPIO (上拉)
   GND ──┤4                5├── CMD → MCU GPIO (上拉)
         └─────────────────────┘
```

#### 3.1.2 上拉电阻要求

这是 SD NAND 硬件连接中**最容易出问题的环节**。规格书多处强调（2.1 节、5.4 节、6.5 节）：CMD 和**所有 DAT 线**都必须通过 10K~100KΩ 上拉到 VDD。

**关键是 DAT1~3 即使不使用，也必须上拉。** DAT3 的上拉尤为关键——它不仅是数据线，还承担 SD 模式选择功能（高电平 = SD 模式）。如果 DAT3 因漏接上拉而浮空，芯片可能误入 SPI 模式，导致 SD 协议初始化失败。

| 信号 | 处理 | 备注 |
|------|------|------|
| CMD | 10KΩ 上拉到 VDD | 防止命令线悬浮，必接 |
| DAT0 | 10KΩ 上拉到 VDD | 防止数据线悬浮，必接 |
| DAT1 | 10KΩ 上拉到 VDD | **虽不使用，规格书要求上拉** |
| DAT2 | 10KΩ 上拉到 VDD | **虽不使用，规格书要求上拉** |
| DAT3 | 10KΩ 上拉到 VDD | **虽不使用，规格书要求上拉；此引脚上拉 = 选择 SD 模式** |
| CLK | 33Ω 串联 | 限流/阻抗匹配（规格书 2.1 节推荐 0~120Ω） |
| VDD | 2.2μF 去耦电容到 GND | 规格书 2.1 节建议 |

### 3.2 第二步：配置 SD0 引脚与时钟

在 `sdk_config.h` 中配置 SD0 的硬件参数。以下为推荐配置（引脚需按实际 PCB 连接调整）：

```c
// sdk_config.h — SD0 硬件配置
#define TCFG_SD0_ENABLE                  1                // 启用 SD0
#define TCFG_SD0_DAT_MODE                1                // 1-bit 单线模式
#define TCFG_SD0_DET_MODE                SD_CMD_DECT      // CMD 线检测
#define TCFG_SD0_CLK                     12000000         // 12MHz（数据传输频率）
#define TCFG_SD0_PORT_CMD                IO_PORTG_01      // 按实际硬件调整
#define TCFG_SD0_PORT_CLK                IO_PORTG_02      // 按实际硬件调整
#define TCFG_SD0_PORT_DA0                IO_PORTG_00      // 按实际硬件调整
#define TCFG_SD0_PORT_DA1                NO_CONFIG_PORT   // 1-bit 模式不接
#define TCFG_SD0_PORT_DA2                NO_CONFIG_PORT   // 1-bit 模式不接
#define TCFG_SD0_PORT_DA3                NO_CONFIG_PORT   // 1-bit 模式不接
#define TCFG_SD0_POWER_SEL               SD_PWR_NULL      // 不使用独立电源控制
#define TCFG_SDX_CAN_OPERATE_MMC_CARD    0                // 不支持 MMC
#define TCFG_KEEP_CARD_AT_ACTIVE_STATUS  0                // 空闲时允许卡进入 Standby 省电
```

#### 卡检测策略

JL SDK 提供三种检测方式（`sdmmc.h:9-11`）：

| 检测方式 | 宏 | 原理 | 适用场景 |
|---------|-----|------|---------|
| CMD 检测 | `SD_CMD_DECT` | 发送 CMD0 并等待响应 | **推荐**，无需额外引脚 |
| CLK 检测 | `SD_CLK_DECT` | 检测 CLK 引脚电平 | 需要 CLK 引脚功能复用 |
| IO 检测 | `SD_IO_DECT` | 检测指定 IO 引脚电平 | 需要额外检测引脚 |

对于焊死在 PCB 上的 SD NAND，推荐 `SD_CMD_DECT`。但更关键的是下一步的强制在线配置——CMD 检测依赖插拔事件，焊死芯片不会产生插拔事件。

#### 时钟策略

| 阶段 | 时钟范围 | 说明 |
|------|---------|------|
| 识别模式 (Identification) | 100 ~ 400 KHz | 初始化阶段，CMD0~CMD3。SD 驱动自动降速 |
| 数据传输模式 (Default Speed) | 0 ~ 25 MHz | 常规读写 |
| 数据传输模式 (High Speed) | 0 ~ 50 MHz | XCSD02G-MJ 最高支持 50MHz |

`TCFG_SD0_CLK = 12MHz` 是一个保守的数据传输频率。稳定后可提升至 25MHz。

### 3.3 第三步：开启强制在线模式（🔴 关键）

**这是 SD NAND 接入中最容易踩的坑。** 对于可插拔的 SD 卡，SDK 依赖 `sdx_dev_detect_timer` 定时器轮询卡插入事件，检测到后才调用 `dev_manager_add("sd0")` 挂载。但 SD NAND 是**焊死在 PCB 上的**，不存在插拔动作。

如果不开启 `TCFG_SD_ALWAY_ONLINE_ENABLE`（`dev_manager.c:1029`），系统**永远不会主动调用 `dev_manager_add("sd0")`**，SD NAND 即使硬件连接正确也无法被注册和挂载。

在 `board_ac701n_demo_cfg.h` 中添加：

```c
// board_ac701n_demo_cfg.h — 须在 sdk_config.h 包含之前定义
#define TCFG_SD_ALWAY_ONLINE_ENABLE      1   // [关键] SD NAND 焊死在 PCB 上，无插拔事件
```

开启后，`dev_manager.c:1029-1047` 中的逻辑会：

```c
#if TCFG_SD_ALWAY_ONLINE_ENABLE
#if (defined(TCFG_SD0_ENABLE) && (TCFG_SD0_ENABLE))
    extern void force_set_sd_online(char *sdx);
    force_set_sd_online("sd0");          // 1. 绕过检测逻辑，强制标记 SD 为在线
    int err = dev_manager_add("sd0");    // 2. 立即挂载 SD 设备
#if (TCFG_SD_ALWAY_ONLINE_ENABLE && (TCFG_SD0_ENABLE || TCFG_SD1_ENABLE))
    extern void sdx_dev_detect_timer_del();
    sdx_dev_detect_timer_del();          // 3. 关闭检测定时器（已无必要）
#endif
    if (err != 0) {
        printf("sd add fail\n");
    }
#endif
#endif
```

> **补充**：当 `TCFG_SD_ALWAY_ONLINE_ENABLE` 关闭时，SD 挂载由检测定时器触发，失败后会通过 `dev_status.c:224` 重试最多 3 次。开启此宏后该重试路径被绕过——对焊死的 SD NAND 来说挂载失败即为硬件故障，重试无意义。

### 3.4 第四步：注册设备与挂载

设备注册分两层，SDK 已有完整代码，**无需修改**：

**第一层：硬件设备注册**（`device_config.c`）：

```c
REGISTER_DEVICES(device_table) = {
#if TCFG_SD0_ENABLE
    { "sd0", &sd_dev_ops, (void *)&sd0_data },
#endif
};
```

`devices_init()` 在启动时遍历 `device_table`，调用 `sd_dev_ops.init(sd0_data)` 完成硬件初始化（GPIO 功能复用、SD 控制器复位、中断注册）。

**第二层：逻辑盘注册**（`dev_reg.c`）：

```c
#if (TCFG_SD0_ENABLE)
    {
        /*logo*/         "sd0",
        /*name*/         "sd0",
        /*storage_path*/ "storage/sd0",
        /*root_path*/    "storage/sd0/C/",
        /*fs_type*/      "fat",
    },
#endif
```

`dev_manager_add("sd0")` 调用 `__dev_manager_add("sd0", 1)` 遍历 `dev_reg[]` 按 `logo` 字段匹配配置项，然后调用 `mount("sd0", "storage/sd0", "fat", ...)` 挂载。

**完整启动流程**：

```
系统上电
  ↓
devices_init()
  ├── sd_dev_ops.init(sd0_data)
  │   ├── sdmmc_0_port_init()         → 初始化 GPIO 引脚功能复用
  │   ├── SD 控制器硬件复位
  │   └── 注册 SD 中断处理
  ↓
dev_manager_init()
  ↓
dev_manager_task()
  ├── [TCFG_SD_ALWAY_ONLINE_ENABLE 路径] dev_manager.c:1029-1047
  │   ├── force_set_sd_online("sd0")    → 绕过插拔检测，强制标记在线
  │   ├── dev_manager_add("sd0")        → 挂载逻辑盘
  │   └── sdx_dev_detect_timer_del()    → 关闭检测定时器
  │
  ├── dev_manager_add("sd0") 内部:
  │   ├── __dev_manager_add("sd0", 1)
  │   │   └── 遍历 dev_reg[]，按 logo "sd0" 匹配配置项
  │   └── mount("sd0", "storage/sd0", "fat", ...)
  │       ├── sd_dev_ops.open("sd0")
  │       │   ├── 发送 ≥74 个虚拟时钟
  │       │   ├── CMD0 → ACMD41 → CMD2 → CMD3 → CMD7 → CMD9
  │       │   └── 卡就绪，接受块读写
  │       ├── FAT 超级块读取 → 验证 BPB 签名
  │       ├── FAT 缓存初始化
  │       └── 挂载成功 → 挂载点: storage/sd0/C/
  ↓
应用层可通过 fopen("storage/sd0/C/...") 访问
```

### 3.5 第五步：通过 VFS 进行文件读写

与 SPI NAND 完全相同的接口。SD NAND 挂载后，通过标准 C 风格的文件操作访问：

```c
// === 写入文件 ===
FILE *fp = fopen("storage/sd0/C/test.txt", "w+");
if (fp) {
    fwrite("Hello SD NAND!", 1, 15, fp);
    fclose(fp);
}

// === 读取文件 ===
fp = fopen("storage/sd0/C/test.txt", "r");
if (fp) {
    char buf[64];
    int len = fread(buf, 1, sizeof(buf), fp);
    fclose(fp);
}

// === 删除文件 ===
fdelete_by_name("storage/sd0/C/test.txt");

// === 获取剩余空间 (单位: KB) ===
u32 free_space;
fget_free_space("storage/sd0/C/", &free_space);

// === 扫描目录 ===
struct vfscan *fs = fscan("storage/sd0/C/", "-tMP3 -r", 3);
if (fs && fs->file_number > 0) {
    FILE *file = fselect(fs, FSEL_FIRST_FILE, 0);
    // 使用 file ...
}
fscan_release(fs);
```

完整的 VFS 接口参考 `SDK/interface/utils/fs/fs.h`。

### 3.6 补充配置项

| 宏 | 推荐值 | 说明 |
|------|:---:|------|
| `TCFG_KEEP_CARD_AT_ACTIVE_STATUS` | 0 | 控制在空闲时是否保持卡活跃（`lib_driver_config.c:24`）。设为 1 减少唤醒延迟，设为 0 更省电。两种选择对 SD NAND 焊死场景均合理，按功耗/响应速度偏好取舍 |
| `TCFG_RECORD_FOLDER_DEV_ENABLE` | 按需 | 如果同时启用，系统会额外创建 `sd0_rec` 逻辑分区（`dev_reg.c:72-81`），指向 `storage/sd0/C/REC/` 录音专用文件夹 |

---

## 4. 集成点

### 4.1 USB MSD (U盘模式)

SD NAND 可以作为 USB 大容量存储设备暴露给 PC。前提是使能 `TCFG_USB_SLAVE_MSD_ENABLE = 1`（`sdk_config.h:109`）。SDK 在 `task_pc.c:255-261` 中自动将 `sd0` 注册为 USB mass storage 设备：

```c
#if TCFG_USB_SLAVE_MSD_ENABLE
#if (!TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0 && TCFG_SD0_ENABLE)\
     ||(TCFG_SD0_ENABLE && TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0 && TCFG_DM_MULTIPLEX_WITH_SD_PORT != 0)
    msd_register_disk("sd0", NULL);
#endif
```

**注意**：如果 USB DM 引脚与 SD DAT0 复用（`TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0`），需额外确认 `TCFG_DM_MULTIPLEX_WITH_SD_PORT` 配置。对于 SD NAND 焊死场景，通常不涉及此复用。

### 4.2 OTA 升级

支持从 SD 卡读取升级固件进行 OTA，设备类型为 `SD0_UPDATA`（`update.h:30`，值 `0x5A01`）。与之对应，SPI NAND 的设备类型为 `USER_NANDFLASH_UFW_UPDATA`（`update.h:46`）。

### 4.3 设备管理器

设备管理器的完整生命周期管理：

| 阶段 | 函数调用 | 说明 |
|------|---------|------|
| 初始化 | `dev_manager_init()` → `devices_init()` | 注册所有设备 |
| 挂载 | `dev_manager_add("sd0")` | 自动 mount FAT 文件系统 |
| 检查 | `dev_manager_online_check_by_logo("sd0", 1)` | 检查设备是否在线 |
| 获取根路径 | `dev_manager_get_root_path_by_logo("sd0")` | 返回 `"storage/sd0/C/"` |
| 卸载 | `dev_manager_unmount("sd0")` | 卸载文件系统 |

### 4.4 与 SPI NAND 的共存

两个方案可以通过编译期宏切换：

```c
// board_ac701n_demo_cfg.h
#define TCFG_STORAGE_TYPE_SPI_NAND    0
#define TCFG_STORAGE_TYPE_SD_NAND     1

#if TCFG_STORAGE_TYPE_SD_NAND
  #define TCFG_SD_ALWAY_ONLINE_ENABLE       ENABLE_THIS_MOUDLE
  #define TCFG_SD0_ENABLE                   ENABLE_THIS_MOUDLE
  #define TCFG_NANDFLASH_DEV_ENABLE         DISABLE_THIS_MOUDLE
#elif TCFG_STORAGE_TYPE_SPI_NAND
  #define TCFG_NANDFLASH_DEV_ENABLE         ENABLE_THIS_MOUDLE
  #define TCFG_SD0_ENABLE                   DISABLE_THIS_MOUDLE
#endif
```

运行时共存（同时挂载两种存储）理论上可行，但需确保 GPIO 无冲突——SPI1 的引脚和 SD0 的引脚分配到不同的 IO 端口组。

---

## 5. 常见问题

### Q1: SD NAND 和 SD 卡驱动是同一个吗？

是的。`sd_dev_ops` 是通用的 SD/MMC 块设备驱动，对 SD 卡、SD NAND、MMC 卡均使用相同的驱动代码。SD NAND 在协议层与标准 SD 卡完全一致。

### Q2: SD NAND 不需要 FTL，那坏块管理怎么办？

XCSD02G-MJ 芯片内部集成了完整的 NAND 管理控制器，包括 8bit/512B ECC 纠错、坏块管理、磨损均衡、垃圾回收。Host 端看到一个"完美"的块设备，完全不需要关心底层 NAND 物理特性。这是 SD NAND 相对 SPI NAND 最大的架构优势。

### Q3: SD NAND 是否需要手动格式化？

不需要。XCSD02G-MJ 出厂已格式化为 FAT32（规格书 4.2 节），用户可用容量 240MB（02G 型号）。直接挂载即可使用。

### Q4: 1-bit 模式性能够用吗？

写速度 ~4.6MB/s，读速度 ~21MB/s。对于 TWS 耳机的典型应用场景（蓝牙音频 ~320KB/s、固件升级、配置文件读写）完全足够。4-bit 模式需要额外 3 个 GPIO（DAT1~3），在 IO 紧张的小型设备上得不偿失。

### Q5: SD NAND 的 DAT1~3 不接 MCU，只上拉行不行？

可以，而且规格书明确支持这种配置（2.1 节）。上电后 SD NAND 默认使用 1-bit 模式（仅 DAT0），未使用的 DAT1~3 保持输入状态即可。但**上拉电阻必须接**——规格书强调"主机应通过 RDAT 将所有 DAT0-3 线路进行上拉，即使主机在 SD 模式下仅将 SD NAND 用作 1 位模式也是如此"。

### Q6: 闭源的 sd_dev_ops 怎么调试？

虽然 `sd_dev_ops` 在 `device.a` 中闭源，但 SD 协议是完全标准化的。调试时可使用逻辑分析仪同时捕获 CLK、CMD、DAT0 三条线的波形，对照 SD2.0 规范分析每一帧命令和响应。常见问题（上拉漏接、时钟频率过高、供电不稳）都可以从波形中直接定位。

### Q7: SD NAND 和 SPI NAND 可以同时使用吗？

理论上可以。两者使用不同的硬件接口（SDIO vs SPI），GPIO 不冲突的话可以共存。但需要评估是否有实际需求——SD NAND 已经提供了完整的存储能力，同时维护两套存储方案增加复杂度。

### Q8: SD NAND 初始化需要多长时间？

SD 协议初始化约 1 秒（规格书 6.2 节规定初始化超时值为 1 秒）。主要包括：
- ≥74 个虚拟时钟周期
- ACMD41 轮询 busy 位（芯片内部准备就绪）
- FAT 文件系统挂载（读取超级块、初始化 FAT 缓存）

相比 SPI NAND（~100ms 复位 + FTL 扫描），SD NAND 初始化略慢，但对用户感知无影响。

---

## 6. 关键文件清单

| 文件路径 | 角色 | 是否需要修改 |
|---------|------|:---:|
| `SDK/apps/earphone/board/br28/sdk_config.h` | SD0 引脚、时钟、总线宽度等硬件配置 | ✅ 修改（使能 + 配置引脚） |
| `SDK/apps/earphone/board/br28/board_ac701n_demo_cfg.h` | 板级功能宏总开关：`TCFG_SD_ALWAY_ONLINE_ENABLE` | ✅ 新增此宏 |
| `SDK/apps/earphone/device_config.c` | `sd0_data` 平台数据 + `device_table` 注册 | 无需修改（已有 `#if TCFG_SD0_ENABLE` 分支） |
| `SDK/apps/common/dev_manager/dev_reg.c` | `sd0` 逻辑盘注册（文件系统类型、路径） | 无需修改（已有 SD0 注册项） |
| `SDK/apps/common/dev_manager/dev_manager.c` | 设备管理器：`TCFG_SD_ALWAY_ONLINE_ENABLE` 分支 | ❌ 只读参考，不修改 |
| `SDK/interface/driver/cpu/br28/asm/sdmmc.h` | `sdmmc_platform_data` 结构体 + `SD0_PLATFORM_DATA` 宏 | ❌ 只读参考，不修改 |
| `SDK/interface/driver/device/sdmmc/sdmmc.h` | SD 检测模式/速度等级常量 | ❌ 只读参考，不修改 |
| `SDK/interface/utils/fs/fs.h` | VFS 文件系统接口 | ❌ 只读参考，不修改 |
| `SDK/cpu/periph_demo/sd_test.c` | SD 读写测试 demo（`#if 0` 默认禁用） | 参考实现 |
| `SDK/cpu/br28/liba/device.a` | `sd_dev_ops` 闭源驱动所在库 | ❌ 预编译库 |
| `SDK/cpu/br28/liba/fs.a` | FAT 文件系统实现 | ❌ 预编译库 |

### 修改汇总

| # | 文件 | 修改内容 |
|---|------|---------|
| 1 | `sdk_config.h` | `TCFG_SD0_ENABLE` 改为 `1`；配置 `TCFG_SD0_PORT_CMD/CLK/DA0` 为实际引脚；`TCFG_SD0_DAT_MODE=1`；`TCFG_SD0_CLK=12000000` |
| 2 | `board_ac701n_demo_cfg.h` | 新增 `#define TCFG_SD_ALWAY_ONLINE_ENABLE 1` |
| 3 | 无其他修改 | `device_config.c`、`dev_reg.c` 已有完整 SD0 支持代码，条件编译守卫自动生效 |

**总计：2 个文件的配置级修改，0 行新驱动代码。**

---

## 7. 硬件调试检查清单

如果 SD NAND 初始化失败，按以下顺序排查：

1. **供电**：VDD 引脚电压是否在 2.7~3.6V 范围？2.2μF 去耦电容是否已焊接？
2. **上拉电阻**：CMD、DAT0~3 是否全部通过 10K~100KΩ 上拉到 VDD？特别注意 DAT3——如果没有上拉，芯片可能误入 SPI 模式
3. **CLK 串联电阻**：CLK 线上是否串接了 33Ω 左右电阻？
4. **引脚映射**：`TCFG_SD0_PORT_CMD/CLK/DA0` 是否与 PCB 实际连接一致？
5. **GPIO 冲突**：这 3 个 IO 是否被其他功能（如 SPI、UART、LED）占用？
6. **`TCFG_SD_ALWAY_ONLINE_ENABLE`**：是否已在 `board_ac701n_demo_cfg.h` 中定义为 1？
7. **逻辑分析仪**：抓 CLK/CMD/DAT0 波形，确认 ≥74 个虚拟时钟 + CMD0 + ACMD41 序列已发出且收到了正确的响应

---

## 参考资料

- XCSD02G-MJ 规格书 v1.0 (2025-10-30), 深圳市芯存科技有限公司
- SD Specifications Part 1: Physical Layer Simplified Specification, Version 2.0
- JL AC701N SDK 源码: `SDK/interface/driver/cpu/br28/asm/sdmmc.h`
- JL AC701N SDK 源码: `SDK/apps/common/dev_manager/dev_reg.c`
- JL AC701N SDK 源码: `SDK/apps/common/dev_manager/dev_manager.c`
- JL AC701N SDK 源码: `SDK/apps/earphone/device_config.c`
- JL AC701N SDK 源码: `SDK/cpu/periph_demo/sd_test.c`
- 存储介质与文件系统关系: `docs/0.存储介质与文件系统/0.存储介质与文件系统的关系.md`
- SPI NAND Flash 接入分析: `docs/0.存储介质与文件系统/SPI_NANDFLASH接入JL平台可行性分析.md`
- 闭源 FTL 到开源 FTL 迁移: `docs/0.存储介质与文件系统/2.JL平台从闭源FTL到开源FTL迁移分析.md`
