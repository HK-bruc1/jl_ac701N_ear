# GD5F4GM7 SPI NAND Flash 接入设计（v4）

> **项目**: AC701N TWS 耳机 SDK
> **目标芯片**: GigaDevice GD5F4GM7UE（3.3V，4Gb / 512MB SPI NAND Flash，Read ID = `C8h 94h`）
> **硬件前提**: SPI 单线半双工（330Ω 电阻桥接 SI/SO）
> **版本**: v4
> **日期**: 2026-05-16

### 版本变更历史

| 版本 | 日期 | 变更 |
|------|------|------|
| v1 | 2026-05-14 | 初稿，A1/A2/B 三方案对比 |
| v2 | 2026-05-15 | 敲定 A2 方案为主，补充验证计划 |
| v3 | 2026-05-15 | 纠正 A2 具体修改点、区分三层容量、量产初始化策略 |
| v4 | 2026-05-16 | **重大修正**：确认"A2 闭源栈"为幻觉，改为厂商源文件直接适配方案；纠正架构描述错误 |

---

## 1. 结论先行

当前 AC701N earphone 工程的 NAND Flash 支持链路：

- **源文件存在**：`nandflash.c`（裸驱动）、`ftl_device.c`（FTL 适配层）均为厂商提供的可见源码
- **不参与编译**：`genFileList.c` 当前仅在 `CONFIG_SOUNDBOX_CASE` 下编译这两个文件
- **已链接的 .a 库不提供 NAND 符号**：`device.a` 和 `ftl.a` 提供其他外设驱动和 FTL 算法核心，但**不包含** `nandflash_dev_ops` / `ftl_dev_ops`

因此接入 GD5F4GM7UE 需要：

1. 参数化 `nandflash.c`（当前硬编码 2KB page / 128KB block 几何）
2. 解耦 `ftl_device.c`（当前硬编码 F35SQA001G 参数）
3. 修改构建系统让 earphone 工程也编译这两个源文件
4. 板级配置使能 NAND + SPI1 半双工

### 设计边界

| 类别 | 含义 |
|------|------|
| **已由当前代码/资料证实** | 可以直接作为设计依据 |
| **合理推断** | 符合架构常识，但当前可见源码无法完全证明 |
| **必须实测后确认** | 不能仅凭配置或 Read ID 下结论 |

> **Read ID 正确只是必要条件，不是充分条件。**
>
> 它只能说明供电、SPI 通路和芯片身份基本正确；不能单独证明 page/block 几何参数、FTL 行为和跨边界读写都正确。

---

## 2. 证据基线

### 2.1 新 SDK 当前已确认的事实

| 事实 | 证据 |
|------|------|
| `genFileList.c` 只在 `CONFIG_SOUNDBOX_CASE` 下编译 `nandflash.c` + `ftl_device.c` | `SDK/build/genFileList.c:1446-1452` |
| `device.a` 和 `ftl.a` **不提供** `nandflash_dev_ops` / `ftl_dev_ops` 符号 | 全工程符号搜索：仅源文件 `.c` 中有定义 |
| `nandflash.c` 当前硬编码 `NAND_PAGE_SIZE=2048` / `NAND_BLOCK_SIZE=0x20000` | `SDK/apps/common/device/storage_device/nandflash/nandflash.c:22-24` |
| `ftl_device.c` 当前硬编码 `block_end=1024, page_size=2*1024` (F35SQA001G) | `SDK/apps/common/device/storage_device/nandflash/ftl_device.c:73-78` |
| 当前 `device_config.c` 中 NAND 分区大小仍是 128MB | `SDK/apps/earphone/device_config.c` |
| 当前工程默认未启用 USB MSD | `SDK/apps/earphone/board/br28/sdk_config.h` 中 `TCFG_USB_SLAVE_MSD_ENABLE = 0` |

### 2.2 GD5F4GM7UE 器件事实

| 参数 | 值 |
|------|-----|
| 型号 | GD5F4GM7UE |
| 电压 | 3.3V（2.7V ~ 3.6V） |
| Read ID | `C8h 94h` (`0xC894`) |
| 容量 | 4Gb = 512MB |
| Page Size | 4096 bytes |
| Pages / Block | 64 |
| Block Size | 256KB |
| Total Blocks | 2048 |
| Internal ECC | 默认开启，8-bit / 512-byte data segment |
| 用户可用 OOB | ECC 开启时前 128 bytes 可编程 |

---

## 3. 架构

```
┌─────────────────────────────────────────────┐
│  应用层                                      │
│  fopen / fread / fwrite / fscan / f_format  │
├─────────────────────────────────────────────┤
│  VFS + FAT 文件系统                          │
│  实现来自 fs.a（闭源）                        │
├─────────────────────────────────────────────┤
│  FTL 适配层                                  │
│  ftl_device.c（源文件，需修改）               │
│  → 调用 ftl.a 闭源核心（磨损均衡/坏块/GC）     │
│  → 调用 nandflash_get_ftl_info() 获取参数    │
├─────────────────────────────────────────────┤
│  NAND 裸驱动                                 │
│  nandflash.c（源文件，需修改）                │
│  职责：Read ID / 页读写 / 块擦除 / ECC       │
│  → 运行时几何参数化（page_size/block_size）   │
│  → nandflash_get_ftl_info() 查询接口         │
├─────────────────────────────────────────────┤
│  SPI1 单线半双工                              │
│  CLK: PC04 / DATA: PC03 / CS: PC05           │
│  SPI_MODE_UNIDIR_1BIT                        │
├─────────────────────────────────────────────┤
│  GD5F4GM7UE                                  │
│  4Gb / 512MB / 4KB page / 256KB block         │
└─────────────────────────────────────────────┘
```

### 与参考版本（soundbox）的真实差异

| 维度 | 参考 soundbox SDK | 当前新 SDK 厂商源文件 |
|------|-------------------|----------------------|
| NAND 参数管理 | 独立 `nandflash_params.c` 查表 + `struct nand_flash_params_t` | `nandflash.c` 内 switch(read_id) 直接赋值 |
| 块大小计算 | 动态 `page_size * page_number` | 硬编码 `#define NAND_BLOCK_SIZE 0x20000` |
| FTL 参数传递 | `ftl_get_nand_info()` 查 params 表 | 硬编码在 `ftl_dev_open()` 中 |
| GD5F4GM7UE 支持 | 有（`nandflash_params.c:0xC894` 条目） | 无 |
| FTL 自动格式化 | 有（FTL! 标记检查） | 无 |

**策略**：使用当前厂商源文件架构（switch 参数化），不照搬参考版本的独立查表模式。

---

## 4. 具体修改

### 4.1 文件 1：`SDK/apps/common/device/storage_device/nandflash/nandflash.c`

**改动 1 — struct 扩展**：给 `struct nandflash_data` 加两个字段：

```c
struct nandflash_data {
    u8 ecc_mask;
    u8 ecc_err;
    u8 plane_select;
    u8 write_enable_position;
    u8 quad_mode_dummy_num: 4;
    u8 quad_mode_qe: 1;
    u16 block_number;
    u32 capacity;
    u16 page_size;                              // 新增
    u32 block_size;                             // 新增
};
```

**改动 2 — 宏移除**：删除 `#define NAND_PAGE_SIZE 2048` 和 `#define NAND_BLOCK_SIZE 0x20000`。

**改动 3 — 宏替换**（约 20 处）：所有 `NAND_PAGE_SIZE` → `nand_flash.page_size`，所有 `NAND_BLOCK_SIZE` → `nand_flash.block_size`。

**改动 4 — switch 初始值**：现有各 case 补充 `page_size` / `block_size` 赋值：

```c
case XT26G01C:
    // ...existing...
    nand_flash.page_size = 2048;
    nand_flash.block_size = 128 * 1024;
    break;
// ...所有现有 case 同样补充 ...
```

**改动 5 — GD5F4GM7UE case**：在 switch 中新增：

```c
case 0xC894: // GD5F4GM7UE
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
```

**改动 6 — feature 初始化**：switch 之后的 `nand_set_features(0xA0, 0x00)` 改为根据 ID 分派：

```c
if (_nandflash.flash_id == 0xC894) {
    nand_set_features(0xA0, 0b00011100);
    nand_set_features(0xB0, 0b00010001);
} else {
    nand_set_features(0xA0, 0x00);
}
```

**改动 7 — 查询接口**：在 `nandflash_dev_ops` 定义之前新增：

```c
#include "ftl_api.h"
void nandflash_get_ftl_info(struct ftl_nand_flash *ftl)
{
    ftl->block_begin = 0;
    ftl->block_end = nand_flash.block_number;
    // SPI NAND 典型预留 5%-10% 用于坏块替换与磨损均衡。
    // 若 ftl.a 内部已有额外预留池，此值可适当上调；
    // 当前取 93% 作为保守起点，对应 2048 blocks → ~1904 logic blocks
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
```

### 4.2 文件 2：`SDK/apps/common/include/nandflash.h`

新增声明：

```c
// FTL 参数查询接口
struct ftl_nand_flash;
void nandflash_get_ftl_info(struct ftl_nand_flash *ftl);
u16 nandflash_get_flash_id(void);
```

### 4.3 文件 3：`SDK/apps/common/device/storage_device/nandflash/ftl_device.c`

`ftl_dev_open()` 中删除硬编码的 flash 结构体，替换为：

```c
struct ftl_nand_flash flash;
nandflash_get_ftl_info(&flash);
flash.page_size_shift = get_first_one(flash.page_size);
flash.block_size_shift = get_first_one(flash.page_size * flash.page_num);
flash.page_read = ftl_port_page_read;
flash.page_write = ftl_port_page_write;
flash.erase_block = ftl_port_erase_block;

g_capacity = flash.logic_block_num << (flash.block_size_shift - 9);
```

`IOCTL_GET_ID` 改为调用 `nandflash_get_flash_id()` 返回真实 ID，不再写死 `0xcd71`。

### 4.4 文件 4：`SDK/build/genFileList.c`

第 1446-1452 行，删除 `#ifdef CONFIG_SOUNDBOX_CASE` / `#endif` 守卫（保留内层 `#if TCFG_NANDFLASH_DEV_ENABLE`）：

```diff
-#ifdef CONFIG_SOUNDBOX_CASE
 #if TCFG_NANDFLASH_DEV_ENABLE
 c_SRC_FILES += \
 	apps/common/device/storage_device/nandflash/nandflash.c \
 	apps/common/device/storage_device/nandflash/ftl_device.c
 #endif
-#endif
```

### 4.5 文件 5：`SDK/apps/earphone/board/br28/board_ac701n_demo_cfg.h`

**SPI1 单线半双工**（替换当前配置）：

```c
#define TCFG_HW_SPI1_ENABLE                 ENABLE_THIS_MOUDLE
#define TCFG_HW_SPI1_PORT_CLK               IO_PORTC_04
#define TCFG_HW_SPI1_PORT_DO                IO_PORTC_03
#define TCFG_HW_SPI1_PORT_DI                NO_CONFIG_PORT
#define TCFG_HW_SPI1_BAUD                   2000000L
#define TCFG_HW_SPI1_MODE                   SPI_MODE_UNIDIR_1BIT
#define TCFG_HW_SPI1_ROLE                   SPI_ROLE_MASTER
```

**NAND Flash 使能**（在 SPI 配置段之后插入）：

```c
#define TCFG_NANDFLASH_DEV_ENABLE           1
#define TCFG_FLASH_DEV_SPI_HW_NUM           1
#define TCFG_FLASH_DEV_SPI_CS_PORT          IO_PORTC_05
#define TCFG_FLASH_DEV_FLASH_READ_WIDTH     1

#define TCFG_NORFLASH_SFC_DEV_ENABLE        DISABLE_THIS_MOUDLE
#define TCFG_NORFLASH_DEV_ENABLE            DISABLE_THIS_MOUDLE
#define TCFG_VIRFAT_FLASH_ENABLE            DISABLE_THIS_MOUDLE
#define FLASH_INSIDE_REC_ENABLE             DISABLE_THIS_MOUDLE
```

**引脚冲突清理**：

```c
#define NTC_DET_EN                          0
#define NTC_POWER_IO                        NO_CONFIG_PORT
#define NTC_DETECT_IO                       NO_CONFIG_PORT

#define TCFG_HW_I2C0_CLK_PORT               NO_CONFIG_PORT
#define TCFG_HW_I2C0_DAT_PORT               NO_CONFIG_PORT
```

### 4.6 文件 6：`SDK/apps/earphone/device_config.c`

将 NAND 分区容量从 128MB 对齐为 512MB：

```c
.size = 512 * 1024 * 1024,
```

---

## 5. 初始化与运行时数据流

### 5.1 正常挂载路径

```
上电
  ↓
app_main.c
  ↓
dev_manager_init()
  ↓
dev_manager_task()
  ↓
devices_init()
  ↓
注册 "nand_flash" 与 "nandflash_ftl"
  ↓
dev_manager_add("nandflash_ftl")
  ↓
mount("nandflash_ftl", "storage/nandflash_ftl", "fat", ...)
  ↓
FTL 初始化 + FAT 挂载
  ↓
挂载成功后通过 VFS 访问文件
```

### 5.2 首次初始化 / 格式化策略

必须拆开三层状态：

| 层级 | 关心的问题 | 典型操作 |
|------|------------|----------|
| Raw NAND | 芯片是否可识别、页读写/块擦除是否正确 | Read ID、raw page read/write、erase |
| FTL | 是否已有 FTL metadata、FTL 是否能初始化 | FTL init / mount |
| FAT | 是否已有可挂载文件系统 | `mount()` / `f_format()` |

量产推荐策略：

1. **生产/烧录阶段显式初始化**：由产测工装或专门 provisioning 命令触发，不在普通开机路径自动格式化
2. **正常运行阶段只尝试 mount**：运行期 mount 失败时只上报错误，不自动擦盘
3. **不通过弱信号自行推断空白介质**：不通过 mount 失败、首扇区全 `0xFF` 之类信号自动格式化

#### PoC 阶段的例外

实验板 bring-up 可保留显式调用的测试辅助函数：

```c
static int nandflash_format_for_poc(void)
{
    wdt_disable();
    int ret = f_format("nandflash_ftl", "fat", 4096);
    wdt_enable();
    if (ret) {
        return ret;
    }
    return dev_manager_mount("nandflash_ftl");
}
```

注意：512MB 全片格式化可能需要数秒。如果看门狗超时周期较短，`wdt_disable()`/`wdt_enable()` 足够；若未来改为更细粒度的看门狗策略，应在格式化循环内部周期性 `wdt_clear()` 喂狗。

### 5.3 需要避免的错误路径

```
mount 失败
  ↓
直接认定"未格式化"
  ↓
自动 f_format
  ↓
潜在误擦用户数据
```

---

## 6. 能力边界

### 6.1 性能基线

SPI 单线半双工 @ 2MHz 的理论峰值约 250KB/s，协议开销后实际预期：

| 操作 | 预期吞吐 | 说明 |
|------|----------|------|
| 顺序读 | ~150 KB/s | GD_READ_FROM_CACHE (0x03) + DMA |
| 顺序写 | ~80 KB/s | PROGRAM_LOAD + PROGRAM_EXECUTE |
| 随机读 | ~100 KB/s | 受 page read to cache 延迟影响 |
| 块擦除 | ~3 ms/block (typ) | 不影响稳态吞吐 |

对应用场景的影响：
- **音频播放**（128-320kbps MP3）：足够，CPU 端缓冲可吸收延迟
- **USB MSD 枚举与大文件传输**：体验受限，建议产品侧给出可接受下限
- **OTA 升级**：512MB 纯读约 58 分钟（524288KB / 150KB/s）；实际含擦除、校验和协议开销，保守估计 ~1.5h，增量升级可接受
- **FTL GC**：可能产生秒级暂停，需在日志中标记 GC 触发频率

### 6.2 `block_page_get()` 优化保证

宏改为运行时变量后，`block_page_get()` 中的除法/取模无法被编译器优化为移位。**约束条件**：所有芯片的 `page_size` 和 `block_size` 必须维持为 2 的幂次（当前所有支持芯片均满足：`page_size ∈ {2048, 4096}`，`block_size ∈ {128KB, 256KB}`）。如未来加入非 2 的幂次几何芯片，需改用 `__builtin_ctz()` 预计算 shift 值。

### 6.3 出厂坏块识别

GD5F4GM7UE 出厂时，每个 block 的 page 0 / byte 0（或 page 0 的 OOB 区第一个字节）标记该 block 是否为坏块（非 `0xFF` 即坏块）。当前 `nandflash.c` 未显式扫描出厂坏块表。

**策略**：
- 若 `ftl.a` 闭源核心在 `ftl_init()` 时已自动扫描并跳过坏块，则裸驱动层无需额外处理
- **必须在 FTL 验证阶段确认**：手动读取某个 block 的 page 0 byte 0，验证 `ftl.a` 是否跳过非 `0xFF` 标记的 block
- 若 `ftl.a` 不处理，则需在 `nandflash_get_ftl_info()` 中传递坏块表，或裸驱动层在 open 时扫描并存储

### 6.4 USB MSD

- 框架支持已存在：`task_pc.c` 中 `TCFG_NANDFLASH_DEV_ENABLE` 下注册 `nandflash_ftl`
- 当前工程默认未启用：`TCFG_USB_SLAVE_MSD_ENABLE = 0`
- 若产品需要，需额外打开配置并验证枚举、双端 FAT 一致性

### 6.5 OTA

SDK 侧存在 NAND OTA 相关枚举入口（`USER_NANDFLASH_UFW_UPDATA`），但从当前可见源码还不能证明完整闭环可直接使用。若 OTA 在项目范围内，需单独定义并验证升级包入径、触发、成功/失败判定。

### 6.6 FTL 内部行为

`ftl.a` 闭源核心负责：逻辑/物理映射、磨损均衡、坏块管理、垃圾回收。其内部实现细节不可见，文档中应表述为**预期职责**。

---

## 7. 风险评估

| 风险 | 影响 | 应对 |
|------|------|------|
| 运行时几何参数化后，现有芯片 128KB block 寻址路径被误改 | 高 | 回归现有芯片支持（XT26G01C 等）的 block 操作 |
| `block_page_get()` 改为运行时除法，性能下降 | 中 | 约束所有 page_size/block_size 为 2 的幂次；非幂次芯片改用 `__builtin_ctz()` 预计算 shift |
| mount 失败时误触发自动格式化 | 高 | 受控初始化，不把 offline 等同 blank media |
| 单线 2MHz 性能瓶颈 | 中 | 实测顺序读写吞吐，评估 USB MSD/OTA 场景是否可接受 |
| 看门狗在格式化时触发 | 中 | 格式化前后显式处理 WDT；若超时周期短则改用循环内 `wdt_clear()` 喂狗 |
| 配置耦合重新打开 NOR | 中 | dlog 配置可能重新使能 `TCFG_NORFLASH_DEV_ENABLE`；NAND 配置纳入版本 diff 检查清单 |
| FTL over-provisioning 不足 (2%→已调至7%) | 中 | 已从 98% 调至 93%；仍需在运行期监控 bad block 增长速率 |
| `ftl.a` 未处理出厂坏块 | 高 | FTL 验证阶段必须确认：读取某个 block page 0 标记字节，验证 `ftl.a` 是否自动跳过 |
| 图形化配置器覆盖手动修改 | 中 | 任何配置更新后必须复核 NAND 相关宏；纳入版本 diff 检查清单 |
| `get_first_one()` 符号是否存在于当前 SDK | 低 | 编译验证阶段自动检查；若缺失用 `__builtin_ctz()` 替换 |

---

## 8. 工程实践要求

1. **NAND 宏纳入版本 diff 检查**：任何通过图形化配置器更新 `board_ac701n_demo_cfg.h` 后，必须复核以下宏是否被意外修改：`TCFG_NANDFLASH_DEV_ENABLE`、`TCFG_FLASH_DEV_SPI_*`、SPI1 引脚、`TCFG_NORFLASH_DEV_ENABLE`
2. **格式化权限控制**：`f_format("nandflash_ftl", ...)` 调用仅允许在工厂模式下通过显式命令触发，不得挂在普通开机路径
3. **Feature 位注释**：`nand_set_features(0xA0, 0b00011100)` 等寄存器写入必须在代码中注释各位含义（ECC-E / BUF / BP 等），便于后续芯片变更时维护
4. **`page_size` / `block_size` 维持 2 的幂次**：新增芯片 case 时，确保这两个值为 2 的幂次（如 2048, 4096, 8192），否则需同时实现 `__builtin_ctz()` 预计算路径

---

## 9. 验证计划

### 9.1 编译验证

```bash
cd SDK/
make clean
make
```

验收：`sdk.elf` 成功生成，无 NAND/FTL 相关链接错误。

### 9.2 Bring-up 基础验证

| 检查项 | 预期 | 说明 |
|--------|------|------|
| 供电 | NAND VCC 正常 | 先排除硬件问题 |
| SPI CLK | PC04 有 2MHz 时钟 | 与配置一致 |
| SPI CS | PC05 在事务期间正确拉低 | 片选正常 |
| Read ID | `0xC894` | 只证明身份与基础通信 |
| DATA 线 | PC03 能正确完成收发切换 | 验证单线半双工 |

### 9.3 Raw NAND 几何验证

| 测试 | 目的 |
|------|------|
| 单页写读 | 确认基础 page access 正常 |
| 页边界验证 | 通过 block/page/offset 组合验证 4KB page 语义 |
| 相邻 page 写读 | 验证 page 递增行为 |
| 相邻 block 写读/擦除 | 验证 64 pages/block 与 256KB block 语义 |
| 近尾 block 访问 | 验证 2048 blocks 上界 |
| 擦除后再写 | 验证 erase 与重编程路径 |

### 9.4 FTL 验证

| 测试 | 目的 |
|------|------|
| blank chip 首次打开 | 观察 FTL 是否能完成初始化 |
| 逻辑容量读取 | 区分 512MB raw capacity 与 FTL logical capacity |
| 多次 mount/unmount | 验证 FTL 元数据稳定性 |
| 多次重启后读回 | 验证 FTL 持久化 |
| 出厂坏块跳过 | 读取某 block page 0 byte 0，确认 `ftl.a` 自动跳过非 `0xFF` 标记的 block |

### 9.5 FAT 文件系统验证

| 步骤 | 操作 | 预期 |
|------|------|------|
| 1 | 受控执行 `f_format("nandflash_ftl", "fat", 4096)` | 返回 0 |
| 2 | 重新 mount | 成功 |
| 3 | 创建、写入、关闭小文件 | 成功 |
| 4 | 重启后重新打开并校验 | 内容一致 |
| 5 | 写入多文件、删除、重建 | FAT 元数据正常 |

### 9.6 验收门槛

1. Read ID 正确
2. raw NAND 几何验证通过
3. blank chip 上的 FTL 行为被观测并确认可用
4. FAT 文件读写通过
5. 生产侧初始化流程明确，普通运行期不会误擦数据

---

## 10. 附录：关键文件路径

| 文件 | 角色 |
|------|------|
| `SDK/apps/common/device/storage_device/nandflash/nandflash.c` | NAND 裸驱动（厂商源文件，需参数化修改） |
| `SDK/apps/common/device/storage_device/nandflash/ftl_device.c` | FTL 适配层（厂商源文件，需解耦硬编码参数） |
| `SDK/apps/common/include/nandflash.h` | NAND 公共头文件（新增查询接口声明） |
| `SDK/interface/utils/flash_translation_layer/ftl_api.h` | FTL API 头文件 |
| `SDK/build/genFileList.c:1446-1452` | 构建源文件列表（移除 SOUNDBOX_CASE 守卫） |
| `SDK/apps/earphone/board/br28/board_ac701n_demo_cfg.h` | 板级开关与 SPI 引脚 |
| `SDK/apps/earphone/device_config.c` | NAND 平台数据 |
| `SDK/apps/common/dev_manager/dev_reg.c` | `nandflash_ftl` FAT 注册项（已存在，无需修改） |
| `SDK/build/Makefile.mk:306,311` | device.a / ftl.a 链接（无需修改） |
