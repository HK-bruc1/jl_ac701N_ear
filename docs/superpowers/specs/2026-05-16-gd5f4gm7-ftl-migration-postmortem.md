# GD5F4GM7UE 闭源 FTL → 开源 FTL 迁移：根因分析与决策记录

> **项目**: AC701N TWS 耳机 SDK  
> **目标**: GD5F4GM7UE (4Gb SPI NAND) 文件系统可读写  
> **结论**: 闭源 `ftl.a` 不兼容 4KB page 几何；迁移到开源 `nandflash_ftl.c` 成功  
> **日期**: 2026-05-16

---

## 1. 结论先行

**闭源 `ftl.a` 在 GD5F4GM7UE（4KB page / 256KB block / 2048 blocks）配置下无法完成 `ftl_format()` 操作，内部触发 Chip Exception 崩溃。** 根本原因是 `ftl.a` 的内存分配或格式化逻辑对 4KB page 几何不兼容（2KB page / 1024 blocks 的旧配置可正常运行）。

经过系统性逐层排查——从 SPI 通信到裸 NAND 驱动到 FTL 核心——确认裸 NAND 擦除/写入/读回全部正常后，决定用编译器可见的开源 `nandflash_ftl.c` 替代闭源 `ftl.a`。迁移完成后 FTL 格式化、FAT 挂载均成功。

---

## 2. 排查全链路（时间线）

### 2.1 编译与链接层

在 SPI 通信开始之前，首先需要让代码通过编译和链接。这一层暴露了多个与 earphone 工程默认配置不兼容的问题。

#### 问题 0-1: `nandflash.c` 缺少 SPI 命令宏

**症状**: 首次编译 `nandflash.c` 时产生 20+ 个错误——`GD_READ_ID`、`GD_WRITE_ENABLE`、`GD_BLOCK_ERASE`、`NAND_STATUS_OIP` 等全部未定义。

**排查**: 当前 earphone SDK 的 `nandflash.h` 只包含 `NAND_Result` 枚举、`nandflash_dev_platform_data` 结构体和 3 个函数声明，缺少 SPI NAND 所需的全部命令码和状态位宏定义。这些宏在原 soundbox 版本的 `nandflash.h` 中存在，但 earphone 工程的头文件是精简版。

**修复**: 向 `nandflash.h` 补充：
- 23 个 `GD_*` SPI NAND 命令码宏（`GD_READ_ID=0x9F`、`GD_WRITE_ENABLE=0x06`、`GD_BLOCK_ERASE=0xD8` 等）
- 8 个 Feature/Status 寄存器位定义宏（`NAND_STATUS_OIP`、`NAND_STATUS_E_FAIL` 等）
- 修正 3 个函数声明，使其返回值类型和参数签名与 `nandflash.c` 实现一致：`NAND_Result` → `int`，移除多余的 `offset` 参数

#### 问题 0-2: `spi_close` 链接失败

**症状**: 编译通过，但链接时报 `undefined reference to 'spi_close'`。

**排查**: `nandflash.c` 的 `_nandflash_close()` 中调用了 `spi_close()`。当前 earphone SDK 的 SPI 驱动头文件（`spi.h`）中只有 `spi_open()` 和 `spi_deinit()`，无 `spi_close()`。soundbox 版本的 `device.a` 可能包含此函数，但 earphone 版本不包含。

**修复**: `spi_close(_nandflash.spi_num)` → `spi_deinit(_nandflash.spi_num)`。两者参数相同（`hw_spi_dev`），语义等价（关闭/反初始化 SPI 外设）。

#### 问题 0-3: `genFileList.c` 守卫阻止 earphone 编译 NAND 源文件

**症状**: 在 earphone 工程中启用 `TCFG_NANDFLASH_DEV_ENABLE=1` 后，编译系统并未编译 `nandflash.c` 和 `ftl_device.c`。

**排查**: `genFileList.c` 第 1446-1452 行将 NAND 源文件包裹在 `#ifdef CONFIG_SOUNDBOX_CASE` 守卫内。earphone 工程使用的是 `CONFIG_EARPHONE_CASE`，因此这些源文件被排除。`device.a` 和 `ftl.a` 虽然已链接，但它们不提供 `nandflash_dev_ops` 和 `ftl_dev_ops` 符号。

**修复**: 移除 `#ifdef CONFIG_SOUNDBOX_CASE` / `#endif` 外层守卫，保留内层 `#if TCFG_NANDFLASH_DEV_ENABLE`。

#### 问题 0-4: `_nandflash_open` 隐式声明

**症状**: 编译警告 `implicit declaration of function '_nandflash_open' is invalid in C99`。

**排查**: `_nandflash_init()`（定义在第 910 行）调用了 `_nandflash_open()`（定义在第 954 行）。在 C99 标准下，函数必须先声明后使用。两个函数之间没有前向声明。

**修复**: 在 `_nandflash_init()` 之前添加 `static int _nandflash_open(void *arg);`。

#### 问题 0-5: `dev_reg.c` 未编译导致 `dev_manager` 链接失败

**症状**: 启用 `TCFG_DEV_MANAGER_ENABLE=1` 后，链接时报 `undefined reference to 'dev_reg'`。

**排查**: `dev_manager.c` 引用了 `dev_reg[]` 数组（定义在 `dev_reg.c`），但 `genFileList.c` 第 1915 行将 `dev_reg.c` 限制为仅在 `TCFG_APP_MUSIC_EN || TCFG_MIX_RECORD_ENABLE` 时编译。TWS 耳机这两个条件都不满足。

**修复**: 扩展守卫为 `TCFG_APP_MUSIC_EN || TCFG_MIX_RECORD_ENABLE || TCFG_NANDFLASH_DEV_ENABLE`。

#### 问题 0-6: `app_config.h` 无条件覆盖板级配置

**症状**: 在 `board_ac701n_demo_cfg.h` 中将 `TCFG_DEV_MANAGER_ENABLE` 设为 1，但 `dev_manager` 任务仍未创建。

**排查**: `app_config.h` 第 201-207 行使用三个 `#define TCFG_DEV_MANAGER_ENABLE ...`（无条件），无论板级配置文件中设置了什么值都会被覆盖。TWS 耳机命中 else 分支（值为 0）。

**修复**: 用 `#ifndef TCFG_DEV_MANAGER_ENABLE` / `#endif` 包裹 `app_config.h` 中的定义块。板级配置文件在 include 链中先被处理，其值会被保留。

---

### 2.2 SPI 通信层

#### 问题 1: `_nandflash_open` 从未被调用
**症状**: 日志只有 `nandflash_init !` 和 `new partition`，无 `nandflash open`。

**排查**: 
- `_nandflash_open()` 由 `ftl_dev_open()` → `dev_open("nand_flash")` 调用
- `dev_manager_add()` 在 `dev_manager_task()` 中执行
- `dev_manager_task` 仅在 `TCFG_DEV_MANAGER_ENABLE = 1` 时创建

**根因**: TWS 耳机的 `app_config.h` 在 else 分支将 `TCFG_DEV_MANAGER_ENABLE` 硬编码为 0，且 board config 无法覆盖（无条件 `#define`）。

**修复**: 
1. `app_config.h` 加 `#ifndef TCFG_DEV_MANAGER_ENABLE` 守卫
2. `board_ac701n_demo_cfg.h` 显式设 `#define TCFG_DEV_MANAGER_ENABLE 1`

#### 问题 2: SPI 读取返回全 `0xFF`
**症状**: `_nandflash_open` 被调用后，`nand_flash_wait_ok()` 读取状态寄存器返回 `0xFF`，触发 `nand_flash prom error!`。

**排查**: `0xFF` 在 SPI 总线上表示 MISO 线未被芯片驱动——芯片没有响应命令。SPI NAND 上电后必须发送 Software Reset（0xFF）才能进入已知状态。

**修复**: 在 `spi_open` 后、状态读取前添加 `nand_flash_reset()`。

#### 问题 3: Reset 后仍然读不到状态
**症状**: 添加 reset 后，仍返回 `status:0xFF`。

**排查**: 对比参考 soundbox SDK 的初始化序列，发现缺失：
1. SPI pin 全部引脚驱动强度设置（当前仅设 DO/PC03）
2. D2/D3 引脚上拉（防浮空干扰总线）
3. `spi_open` / `spi_set_width` 执行顺序错误

**修复**: 补齐完整初始化序列。原代码在 `spi_open` 之前调用了 `spi_set_width`，此时 SPI 控制器尚未激活，寄存器写入无效。纠正为 `spi_open` → `spi_set_width` 的正确顺序后，再补齐三项缺失：所有 6 个 SPI 引脚（CLK/DO/D1/D2/D3/CS）统一设为 64mA 驱动强度（原代码仅设了 DO 一个引脚）、D2/D3 引脚使能 10K 上拉（防止浮空干扰总线）、最后发送 `nand_flash_reset()`（Software Reset 命令 0xFF）。

---

### 2.3 裸 NAND 驱动层

在进入裸 NAND 验证之前，有一项关键的基础工作必须说明——**几何参数化**。原始 `nandflash.c` 使用硬编码宏 `NAND_PAGE_SIZE=2048`、`NAND_BLOCK_SIZE=0x20000`，所有地址计算、页/块遍历都基于这两个常量。GD5F4GM7UE 的几何是 4KB page / 256KB block，与硬编码完全不匹配。为此做了三项改动：

1. **将 `page_size` 和 `block_size` 改为 `nandflash_data` 结构体的运行时字段**，在 `_nandflash_open()` 的 Read ID switch 分支中按芯片型号赋值。所有 `NAND_PAGE_SIZE` / `NAND_BLOCK_SIZE` 引用全部改为 `nand_flash.page_size` / `nand_flash.block_size`。
2. **新增 `GD5F4GM7UE` (0xC894) case 分支**，配置 ECC 掩码、QE 位、block 数（2048）、capacity（512MB）、以及 4096 page_size / 256KB block_size。
3. **新增 `nandflash_get_ftl_info()` API**，使 FTL 层可以通过一次调用获取当前芯片的真实几何参数，替代原来在 `ftl_device.c` 中硬编码的 `page_size=2048, block_end=1024`。

这三项改动是后续 2.4 节 FTL 测试矩阵的基础——没有运行时参数化，就不可能在同一套代码里切换 4KB/2KB page、1024/2048 blocks 来隔离根因。

#### 验证点 1: Read ID
**结果**: `nandflash_read_id: 0xc894` ✅  
**含义**: SPI 通信正常，GD5F4GM7UE 芯片身份确认。

#### 验证点 2: Feature 寄存器
**结果**: 
```
A0 after set: 0x0    ← 写保护已解除
B0 after set: 0x11   ← ECC 已使能
```
**含义**: `nand_set_features()` + `nand_write_enable()` 生效。

#### 验证点 3: 裸 NAND 擦除/写入/读回
**测试代码**:
```c
log_info("raw erase blk0: %d", nand_flash_erase(0));
log_info("raw write pg0: %d", nand_flash_write_page(0, 0, wbuf, 256));
log_info("raw read pg0: %d, buf[0]=%02x buf[255]=%02x", ...);
```

**结果**:
```
raw erase blk0: 0        ← 擦除成功
raw write pg0: 0         ← 写入成功  
raw read pg0: 0, buf[0]=00 buf[255]=ff   ← 读回正确
```
**含义**: 裸 NAND 驱动完全正常，擦除/编程/读取三条路径均通过。

#### 硬件写保护排查：WP# 引脚

在裸 NAND 驱动验证过程中，写入失败还有一个"嫌疑人"——**GD5F4GM7 的硬件写保护引脚 WP#（pin 3 / IO2）**。

**背景知识**: GD5F4GM7UE 的 WP# 引脚如果为低电平，芯片会在硅片层阻止所有 PROGRAM 和 ERASE 操作。即使 Feature Register A0=0x00（软件解锁），WP# 为低也会彻底锁死写入。这是独立于软件写保护的硬件安全机制。

**排查过程**:
1. **怀疑阶段**: 观察到 `write_remap_table_err` 和 `block_inited_bad`，FAT mount 失败（`mbr err`）。但同时 Feature Register 已验证 A0=0x00（写保护已解除），Write Enable 也发送成功。写入失败的可能原因缩小到硬件层面。

2. **查阅原理图**: 用户查看硬件原理图，确认 GD5F4GM7 的 WP# 引脚（IO2）通过 **R53 电阻上拉到 VDD_3V**。这是一个标准设计——WP# 被外部电阻拉高，芯片始终处于可写入状态。

3. **结论**: **WP# 硬件写保护不是写入失败的原因。** 写入失败发生在软件层面——即 `ftl.a` 内部。

**此排查的关键价值**: 排除了硬件写保护后，写入失败的原因被精确锁定在 `ftl.a` 闭源库内部。如果没做这一步，后续决策（放弃 `ftl.a`）就缺乏硬件侧的信心支撑。

---

### 2.4 FTL 层（闭源 `ftl.a`）

裸 NAND 驱动已验证可用，接下来测试 `ftl.a` 能否在 GD5F4GM7UE 的正确几何参数下完成 FTL 初始化。

#### 策略：渐进缩小参数，隔离根因

`ftl.a` 的 `ftl_init()` 接收一个 `ftl_nand_flash` 结构体，其中 `page_size` 和 `block_end` 是两个关键几何参数。通过调整这两个值，可以区分以下三种可能性：

1. **通用内存不足** — 无论参数如何，系统 RAM 都不够 FTL 用 → 需要扩大 VM 池
2. **4KB page 不兼容** — 只要 `page_size=4096`，FTL 就崩溃 → `ftl.a` 内部硬编码了 2KB page 假设
3. **总块数过多** — `block_end=2048` 时元数据超内存，但 `block_end=1024` 时正常 → 可降容量使用

#### 测试矩阵与结果

| 轮次 | page_size | block_end | logic_blocks | 改动目的 | 结果 |
|------|-----------|-----------|-------------|---------|------|
| **C** | **4096** | 2048 | 1904 | 全容量，正确几何 | `ftl_init` 内存断言崩溃：`need_pages: 355, unused: 72` |
| **B** | **4096** | 1024 | 952 | 半容量，验证是否为总块数问题 | `ftl_init_ok`，但 `ftl_format()` 内 Chip Exception，`block_inited_bad: 0-7`，双 CPU 异常，系统软复位 |
| **A** | **2048** | 1024 | 1000 | 回退到旧芯片的 2KB page，验证是否为 page_size 问题 | `ftl_init_ok`，无崩溃，但 `write_remap_table_err` + `mbr err`，FAT mount 失败 |

#### 逐轮分析

**配置 C（4KB page / 2048 blocks）——全容量首发测试**

直接使用 GD5F4GM7UE 的正确几何参数。`ftl_init()` 在 `pmalloc` 分配阶段即崩溃：

```
need_pages: 0x00000163  (355 页)
pmalloc unused pages: 0x48  (72 页)
ASSERT-FAILD: 0 No enough physics memory
```

需 355 页，仅剩 72 页，缺口 283 页（约 71KB）。**但这不能区分是"绝对内存不足"还是"ftl.a 对 4KB page 的计算有 bug"。**

为此做了两方面的内存优化尝试：
- **FTL 侧减负**：将 `ftl_config` 中 `page_buf_num` 从 4 降至 1、`delayed_write_msec` 从 100 降至 0，尽可能缩小 FTL 内部的运行时缓冲区占用
- **系统侧扩容**：将 `TCFG_VM_SIZE` 从 40KB 翻倍至 80KB，给 `pmalloc` 提供更多物理页

即便如此，缺口仍远超可用内存。**需要下一轮测试来区分根因。**

**配置 B（4KB page / 1024 blocks）——缩小块数，排除总容量因素**

将 `block_end` 从 2048 减半至 1024（只用芯片前 256MB），`logic_block_num` 相应减至 952。如果这是纯内存不足问题，半容量应能通过 `ftl_init()`。

结果：**`ftl_init_ok`——初始化通过，无内存崩溃。** 说明不是系统绝对内存不够，而是 `ftl.a` 对 2048 blocks 的元数据分配超出了可用 RAM。

但随后 `ftl_format()` 内部崩溃：

```
[FTL]ftl_init_ok
[FTL_DEV]ftl first format start
[FTL]block_inited_bad: 0    ← 格式化为 8 个块分配元数据
[FTL]block_inited_bad: 1
...
[FTL]block_inited_bad: 7
exception_analyze_user_callback:
Chip Exception, Current CPU0     ← ftl.a 内部崩溃
```

崩溃地址在 `0x0016Axxx` 区域——属于 `ftl.a` 代码段。裸 NAND 驱动此时已回答完所有的 erase/write 回调且全部返回成功，但 `ftl.a` 内部逻辑触发异常。**由于 `ftl.a` 是闭源预编译库，无法查看崩溃点的源码，无法继续调试。**

**配置 A（2KB page / 1024 blocks）——回退 page_size，验证几何兼容性**

将 `page_size` 从 4096 退回到旧芯片的 2048。这是关键测试——如果 2KB page 下 `ftl.a` 工作正常，则证明问题出在 4KB page 的兼容性上。

结果：**`ftl_init_ok`，`ftl_format()` 未崩溃，FAT mount 走到 `__fat_mount: capacity = 3e800`。** 但随后 `mbr err`——这是预期的，因为 2KB page / 128KB block 的几何与 GD5F4GM7UE 的物理结构不匹配，FTL 的块映射完全错位。此配置的"成功"无实际使用价值——它管理的是一个几何扭曲的假象。

#### 综合结论

三组测试排除了所有替代解释，精确定位到根因：

```
┌─────────────────────────────────────────────────────────┐
│  裸 NAND 驱动：erase/write/read 全部通过                │
│  Feature Register A0=0x00（写保护已解除）               │
│  WP# 硬件写保护：R53 上拉到 VDD_3V（已排除）            │
│  ↓                                                      │
│  问题唯一可能位置：ftl.a 闭源库内部                      │
│  ↓                                                      │
│  2KB page 配置 → ftl.a 正常运行（语义错误但逻辑正确）     │
│  4KB page 配置 → ftl.a 崩溃（无论全容量还是半容量）       │
│  ↓                                                      │
│  结论：ftl.a 不兼容 4KB page 几何                       │
└─────────────────────────────────────────────────────────┘
```

`ftl.a`（MD5 `bb781675...`）对 GD5F4GM7UE 的 4KB page / 256KB block 几何存在**根深蒂固的不兼容**——不是单纯的内存不足（半容量也不行），不是简单的格式化逻辑（2KB page 下格式化可完成），而是 `ftl.a` 内部硬编码了 2KB page 的假设，在 4KB page 输入下触发未定义行为。

---

### 2.5 决策：转向开源 FTL

#### 选项对比

| 维度 | 继续调试 `ftl.a` | 迁移 `nandflash_ftl.c` |
|------|-----------------|----------------------|
| 代码可见性 | 闭源，无法调试崩溃点 | 1393 行 C 源码，完全可见 |
| 调试手段 | 只能改入参、看日志 | 可加断点、改逻辑 |
| 几何适配 | 不兼容 4KB page（已实证） | `#define FTL_NAND_PAGE_SIZE 4096` 即可 |
| 风险 | 可能永远无法解决 | 首次集成有编译/链接风险（已解决） |
| 时间 | 不确定 | ~2 小时 |

#### 选择

**迁移到开源 `nandflash_ftl.c`**。理由：
1. 裸 NAND 驱动已全面验证可用，问题确定在 FTL 层
2. 闭源库无法调试，继续投入是黑盒试错
3. 开源 FTL 只需改两个 `#define` 即可适配 4KB 几何
4. 与现有 `device_operations` 接口完全兼容，`dev_manager` + `f_mount` 链路不变

---

## 3. 迁移实施

### 3.1 技术机制：如何断开 `ftl.a` 依赖

决定迁移后，需要回答一个关键问题：**怎样干净地替换掉闭源库，而不影响系统的其他部分？**

#### 旧架构（闭源 FTL）

```
device_config.c
  └── extern const struct device_operations ftl_dev_ops;  // 来自 nandflash.h
       │
       └── ftl_device.c  ← 唯一调用者
            ├── #include "ftl_api.h"
            ├── ftl_init(&flash, &config)     ──┐
            ├── ftl_format()                   ──┤
            ├── ftl_read_bytes(...)            ──┼── 全部来自 ftl.a
            ├── ftl_write_bytes(...)           ──┤
            └── ftl_uninit()                   ──┘
            └── const struct device_operations ftl_dev_ops = { ... };
```

`ftl_api.h` 声明了 5 个函数：`ftl_init`、`ftl_format`、`ftl_read_bytes`、`ftl_write_bytes`、`ftl_uninit`。这些符号由 `ftl.a` 提供（`Makefile.mk` 第 311 行链接）。`device_config.c` 通过 `extern const struct device_operations ftl_dev_ops` 引用 FTL 设备，实际 `ftl_dev_ops` 由 `ftl_device.c` 定义。

#### 验证：`ftl.a` 无其他调用者

执行全工程符号搜索：

```bash
grep -r "ftl_init\|ftl_format\|ftl_read_bytes\|ftl_write_bytes\|ftl_uninit" \
  apps/ audio/ cpu/ --include="*.c" | grep -v ftl_device.c
```

**结果为空**——整个 SDK 中，`ftl.a` 的 API 仅被 `ftl_device.c` 调用。这意味着一件事：**只要不再编译 `ftl_device.c`，`ftl.a` 的所有符号都不会被引用，可以安全地从链接列表中移除。**

#### 新架构（开源 FTL）

```
device_config.c
  └── extern const struct device_operations ftl_dev_ops;  // 同一行，不变
       │
       └── nandflash_ftl.c  ← 自包含，不依赖 ftl.a
            ├── 内部函数：ftl_init(), _ftl_init(), ftl_format(), _ftl_format()
            ├── 内部函数：ftl_dev_read(), ftl_dev_write(), ftl_dev_ioctrl()
            ├── 调用 dev_bulk_read/write/ioctl 操作底层 nand_flash 设备
            └── const struct device_operations ftl_dev_ops = { ... };
```

#### 切换机制：同符号名、不同提供者

两个 FTL 实现都定义了**同名的全局符号** `ftl_dev_ops`（`const struct device_operations`）：

| 文件 | 编译状态 | 提供 ftl_dev_ops？ |
|------|---------|-------------------|
| `ftl_device.c` | ❌ 不再编译（genFileList.c 已移除） | 是（但不参与链接） |
| `nandflash_ftl.c` | ✅ 参与编译（genFileList.c 新增） | 是（链接器找到的唯一实现） |
| `ftl.a` | ❌ 不再链接（Makefile.mk 已移除） | 否（ftl.a 提供的是 ftl_init/ftl_format 等函数，不直接提供 ftl_dev_ops） |

**不需要修改任何函数名。** 切换策略不是"改名绕开冲突"，而是：

1. **符号提供者层面** — 从 `genFileList.c` 移除 `ftl_device.c`、新增 `nandflash_ftl.c`。编译系统只编译后者，`ftl_dev_ops` 的唯一定义来自 `nandflash_ftl.c`。
2. **库依赖层面** — 从 `Makefile.mk` 移除 `ftl.a`。由于 `ftl.a` 的 API 无其他调用者，移除后零链接错误。
3. **设备注册层面** — `device_config.c` 第 200 行的 `{"nandflash_ftl", &ftl_dev_ops, ...}` 不变。这行只引用 `ftl_dev_ops` 的地址，不关心它来自哪个 `.c` 文件。

#### 验证：断链确认

编译通过后的符号解析检查：

```bash
# ftl_dev_ops 来自 nandflash_ftl.c，非 ftl_device.c
grep "ftl_dev_ops" sdk.map
# 预期：地址落在 nandflash_ftl.c 的 .text 段

# ftl.a 无残留符号
grep "ftl_init\|ftl_format\|ftl_read_bytes\|ftl_write_bytes" sdk.map | grep -v nandflash_ftl
# 预期：无匹配（ftl.a 的符号不再出现）
```

### 3.2 改动清单

| 文件 | 操作 | 说明 |
|------|------|------|
| `nandflash_ftl.h` | 新建 | FTL 类型定义，`FTL_NAND_PAGE_SIZE=4096`, `FTL_NAND_BLOCK_SIZE=0x40000` |
| `nandflash_ftl.c` | 新建 | 从参考 SDK 拷贝，修正 include 路径，添加 BPB_* 常量 |
| `genFileList.c` | 修改 | `ftl_device.c` → `nandflash_ftl.c` |
| `Makefile.mk` | 修改 | 移除 `ftl.a` 链接 |
| `nandflash.c` | 修改 | 清理裸 NAND 诊断代码 |
| `device_config.c` | 修改 | `arg` 参数从 NULL → `"nand_flash"` |
| `ftl_device.c` | 不变 | 保留为参考，不参与编译 |

### 3.3 遇到的编译问题

| 问题 | 原因 | 修复 |
|------|------|------|
| `use of undeclared identifier 'BPB_TotSec32'` 等 9 个错误 | `virfat_flash.h` 被移除但其中定义的 FAT BPB 偏移常量仍被使用 | 将 6 个 BPB_* `#define` 直接加入 `nandflash_ftl.c` |
| `implicit declaration of function 'clr_wdt'` | `clr_wdt()` 不在 `asm/wdt.h` 中，是各模块本地 `extern` 声明的 | 恢复 `extern void clr_wdt(void);` |
| `dev->fmnt = 0`（首次运行） | `device_config.c` 中 `arg = NULL`，FTL 不知道打开哪个裸设备 | `arg` 改为 `"nand_flash"` |

---

## 4. 最终验证结果

```
[nandFLASH]nandflash_read_id: 0xc894        ← SPI 通信 + Read ID
[nandFLASH]nandflash open success !          ← 裸 NAND 驱动就绪
>>>[test]:format okokokok                    ← FTL 格式化完成
__fat_mount: capacity = efe00                ← FAT 容量 = 983,040 扇区 ≈ 480MB
__dev_manager_add, nandflash_ftl add ok,
  dev->fmnt = 0x419290                       ← 文件系统挂载成功
```

| 验证项 | 结果 |
|--------|------|
| Read ID | `0xC894` ✅ |
| 裸 NAND 擦除 | 成功 ✅ |
| 裸 NAND 写入 | 成功 ✅ |
| 裸 NAND 读回 | 数据一致 ✅ |
| FTL 格式化 | `format okokokok` ✅ |
| FAT 挂载 | `fmnt = 0x419290`（有效句柄） ✅ |
| 系统稳定性 | 完整运行至蓝牙模式，无崩溃 ✅ |
| 首次启动耗时 | ~9.1 秒（全片擦除 2048 blocks） |
| 后续启动耗时 | < 1 秒（仅读 SPBK 元数据） |

---

## 5. 经验教训

### 5.1 走过的弯路（未成功的尝试）

以下尝试花费了时间但未解决问题，记录于此避免后来者重复踩坑：

| 尝试 | 目的 | 结果 | 教训 |
|------|------|------|------|
| 扩大 VM pool（40→80KB）+ 缩减 FTL 缓冲区（`page_buf_num` 4→1, `delayed_write_msec` 100→0） | 让 Config C 通过 `ftl_init` 内存分配 | `ftl_init` 仍报 `No enough physics memory`（缺 283 页） | `ftl.a` 对 4KB page × 2048 blocks 的元数据计算远超可用 RAM，纯内存扩容无法解决 |
| 半容量测试（4KB page / 1024 blocks / Config B） | 排除"总块数过多"是唯一原因 | `ftl_init` 通过但 `ftl_format` 内 Chip Exception 崩溃 | 问题不在容量而在 4KB page 兼容性，即使只用一半芯片也无法工作 |
| 回退 2KB page 几何（Config A） | 验证 `ftl.a` 在"错误几何"下是否可运行 | `ftl_init` / `ftl_format` 均未崩溃，但 FAT mount 失败（几何与实际芯片不匹配） | 证实了 `ftl.a` 只能在 2KB page 假设下运行，但这不是可用方案——管理的是几何扭曲的假象 |

三条弯路从不同角度指向同一结论：**`ftl.a` 闭源库与 4KB page 几何不兼容，唯一的出路是替换 FTL 实现。**

### 5.2 核心经验

1. **分层验证是定位闭源库问题的唯一手段。** 将问题从 SPI → 裸 NAND → FTL 逐层隔离，每一层用独立测试验证，最终精确定位到 `ftl.a` 的 `ftl_format()` 内部崩溃。

2. **裸驱动验证是决策基础。** `raw erase/write/read` 三项测试全部通过，才能确信问题不在底层，坚定地放弃闭源 FTL。

3. **编译期可见性决定调试效率。** 闭源 `ftl.a` 的崩溃点不可见、不可修改，而开源 `nandflash_ftl.c` 的每个状态都可追踪。

4. **框架层面的使能条件需要完整梳理。** `TCFG_DEV_MANAGER_ENABLE` 的三层覆盖（board config → app_config 无条件 #define → 编译守卫）耗费了大量排查时间。

5. **硬件写保护必须对照原理图确认。** SPI NAND 的 WP# 引脚提供独立的硬件写保护机制。在确认 A0=0x00（软件解锁）后写入仍失败时，查阅原理图确认 R53 上拉电阻排除了硬件锁死的可能性，将问题精确定位到 `ftl.a` 内部。软硬件双层验证是可靠决策的基础。
