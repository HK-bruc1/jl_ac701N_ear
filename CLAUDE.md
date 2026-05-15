# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an AC701N TWS (True Wireless Stereo) Bluetooth earphone SDK from JL (杰理/JieLi), built on the BR28 SoC platform with a Pi32v2 compiler toolchain. The SDK provides the complete firmware framework for Bluetooth earphone products.

## Build System

### Prerequisites

- **Windows**: Toolchain installed at `C:/JL/pi32/bin/` (clang.exe, pi32v2-lto-wrapper.exe, llvm-ar.exe)
- **Linux**: Toolchain installed at `/opt/jieli/pi32v2/bin/`

The build uses two stages:
1. **`make pre_build`** — Preprocesses `build/genFileList.c` to dynamically generate `build/fileList.mk`, selecting only the `.c` files enabled by current `TCFG_*` config macros.
2. **`make`** — Invokes `build/Makefile.mk` to compile all selected sources and link.

Many source files are conditionally compiled based on product type macros. For example, NAND flash source files are only compiled under `#ifdef CONFIG_SOUNDBOX_CASE` — this earphone project (`CONFIG_EARPHONE_CASE`) links them from pre-compiled `.a` libraries instead.

### Key targets

```bash
make              # Build firmware (SDK/sdk.elf)
make VERBOSE=1    # Build with full compiler output
make clean        # Remove build artifacts (objs/ and sdk.elf)
```

### Compiler flags

- Target: `-target pi32v2 -mcpu=r3`
- Optimization: `-Oz -flto` (optimize for size with link-time optimization)
- Config symbol gate: `CONFIG_RELEASE_ENABLE` plus many feature-toggling `TCFG_*` and `CONFIG_*` defines in the Makefile

### Build output

Build artifacts are placed in `SDK/cpu/br28/tools/`:
- `sdk.elf` — linked firmware ELF
- `sdk.map` — memory map
- `sdk.lst` — disassembly listing
- `uboot.boot` / `ota.bin` / `update.ufw` / `jl_isd.fw` — flashable firmware images

## Architecture

### Directory layout

```
AC701N/
├── SDK/                       # The SDK root
│   ├── Makefile               # Top-level makefile (configures toolchain, calls build/Makefile.mk)
│   ├── build/                 # Build scripts (genFileList.c, Makefile.mk, include_dir.txt)
│   ├── apps/                  # Application layer
│   │   ├── earphone/          # TWS earphone application (app_main.c, modes, audio, battery, BLE, UI)
│   │   └── common/            # Shared app modules (config, debug, dev_manager, cJSON, update, ui, music, le_audio)
│   ├── cpu/br28/              # CPU-specific code (startup, overlay, linker script sdk.ld, maskrom stubs)
│   ├── audio/                 # Audio subsystem
│   │   ├── framework/         # Audio streaming framework — nodes (processing blocks) and plugs (data sources)
│   │   │   ├── nodes/         # Audio processing nodes: agc, ns, dns, plc, volume, effect_dev*, etc.
│   │   │   └── plugs/source/  # Data sources: a2dp, esco, adc, iis, le_audio
│   │   ├── common/            # Shared audio utilities (volume, noise gate, frame adaptive, energy detection)
│   │   ├── CVP/               # Clear Voice Processing (AEC, multi-mic CVP algorithms)
│   │   ├── effect/            # Audio effects (EQ, bass/treble, pitch/speed, voice changer, spatial effects)
│   │   ├── anc/               # Active Noise Cancellation
│   │   ├── interface/         # Audio subsystem interfaces
│   │   └── cpu/br28/          # BR28-specific audio (DAI, DAC, VAD, audio decoder)
│   ├── interface/             # Hardware abstraction & protocol stack interfaces
│   │   ├── system/            # OS abstraction (task, timer, event, malloc) — wraps μC/OS or bare-metal
│   │   ├── driver/            # Peripheral drivers (GPIO, I2C, SPI, UART, power, trim)
│   │   ├── media/             # Media subsystem (codec, effects, encoder, audio base)
│   │   ├── btstack/           # Bluetooth protocol stack interfaces
│   │   ├── btctrler/          # Bluetooth controller/HCI layer
│   │   ├── update/            # OTA firmware update interfaces
│   │   └── utils/             # Utilities (filesystem, math, CRC, flash translation layer)
│   └── tools/                 # Build & download tools (compiler utils, merge-archives, isd_download)
├── src/                       # Per-project JSON configuration files
│   ├── 板级配置.json           # Board-level configuration (pin mux, GPIO)
│   ├── 电源配置.json           # Power configuration
│   ├── 蓝牙配置.json           # Bluetooth configuration (name, profiles)
│   ├── 按键配置.json           # Key/button configuration
│   ├── 音频配置.json           # Audio configuration
│   ├── 功能配置.json           # Feature configuration
│   ├── 升级配置.json           # OTA upgrade configuration
│   ├── 提示音.tone             # Prompt tone definitions
│   └── 音频流程/               # Audio flow graphs (.x6flow files for each mode: BT music, call, USB, ANC, etc.)
├── output/                    # Built firmware images (update.ufw, jl_isd.fw, jl_isd.bin, stream.bin)
├── cache/                     # Build tool cache
└── project.jlproj             # Project metadata (type, chip, SDK version, patch info)
```

### Application structure (earphone app)

The `app_main.c` defines the RTOS task table (`task_info_table[]`) which creates all system tasks. Key tasks include:
- **app_core** — main application logic
- **btstack** / **btctrler** — Bluetooth protocol stack
- **jlstream** (plus 7 sub-tasks) — JL audio streaming framework
- **a2dp_dec** — A2DP audio decode (LHDC support optional)
- **aec** / **eff_mtask** — audio processing
- **file_dec** / **write_file** — file playback and recording
- **update** / **tws_ota** — OTA firmware updates

The app uses `apps/earphone/mode/` for state-machine based mode management:
- `mode/bt/` — Bluetooth modes (music playback, phone calls, background, TWS pairing)
- `mode/music/` — local music playback
- `mode/pc/` — PC/USB audio
- `mode/linein/` — line-in audio

### Configuration system

Feature configuration is **two-layered**:
1. **Board config header** (`apps/earphone/board/br28/board_config.h`) selects the board variant which includes `board_ac701n_demo_cfg.h` — this sets all `TCFG_*` macros to enable/disable features.
2. **JSON config files** in `src/` are processed by the JL configuration tool to generate runtime config binaries (e.g., `btcfg.bin`, `audiocfg.bin`).

Key configuration macros follow naming conventions:
- `TCFG_*` — feature toggles (e.g., `TCFG_USER_TWS_ENABLE`, `TCFG_PC_ENABLE`)
- `CONFIG_*` — build-time compile configs (e.g., `CONFIG_RELEASE_ENABLE`, `CONFIG_UCOS_ENABLE`)

### Audio pipeline

The audio subsystem uses a **node-based streaming architecture**:
- **Source plugs** (`audio/framework/plugs/source/`) feed data into the pipeline (A2DP, ESCO, ADC, local file, LE Audio)
- **Processing nodes** (`audio/framework/nodes/`) transform audio: volume, EQ effects, AGC, noise suppression, CVP/AEC, PLC, spatial effects
- The pipeline is defined at runtime by the `.x6flow` files in `src/音频流程/`, one per audio mode (BT music, BT call, USB Audio, ANC, etc.)

### Key patterns

- **RTOS**: The system runs μC/OS, wrapped through the `interface/system/` abstraction. Tasks have priorities (1-6, lower=higher priority), queues, and stack sizes defined in the task table.
- **Message passing**: Inter-task communication uses a message adapter pattern (`apps/earphone/message/adapter/`) dispatching events between subsystems (key, battery, audio player, BT stack, touch, TWS).
- **Memory management**: Uses custom memory sections (`#pragma bss_seg`, `data_seg`, `const_seg`, `code_seg`) per component for linker placement. System uses VM (Virtual Memory) with configurable pool sizes for persistent settings.
- **SDK libs**: Core libraries (BT stack, media, drivers) are pre-compiled `.a` archives stored in the toolchain path, not in this repo. Only the application and framework source is in the repo.
- **Dual-bank OTA**: The firmware supports dual-bank flash for safe OTA updates (update in background bank, swap on success).

### Storage & filesystem architecture

The storage subsystem uses a layered device → FTL → filesystem → VFS model:

```
App Code (fopen / fread / fwrite)
  → VFS (fs.h — unified file interface, source visible)
    → Filesystem (fat / sdfile / nor_sdfile / rec_fs — mostly in fs.a, closed source)
      → FTL or raw device (dev_io_t {read, write, ioctrl})
        → SPI driver (norflash.c 源码可见; NAND 驱动在 device.a 闭源)
```

**Device registration flow**: `REGISTER_DEVICES(device_table)` in `apps/earphone/device_config.c` → `devices_init()` → `dev_reg[]` table defines logo/fs_type/storage_path → `dev_manager_add()` auto-mounts via `mount()`.

**Filesystem types** (defined in `dev_reg.c`):

| fs_type | Medium | Visibility |
|---------|--------|-----------|
| `fat` | SD, UDisk, NOR FAT, **NAND FTL** | `fs.a` (closed) |
| `sdfile` | Built-in flash (resources) | closed |
| `nor_sdfile` | External NOR (resources) | closed |
| `rec_fs` | External NOR (recording) | closed |
| `sdfile_fat` | Virtual FAT on NOR | **`virfat_flash.c` (source visible)** |

**NAND Flash specifics**: Uses a two-device model — `"nand_flash"` (raw, `nandflash_dev_ops` in `device.a`) + `"nandflash_ftl"` (FTL, `ftl_dev_ops` in `ftl.a`). Only FAT filesystem is supported on NAND (via FTL). Both drivers are closed-source pre-compiled libraries. NAND does NOT support `sdfile`/`nor_sdfile` resource filesystems.

**SPI flash pin-saving design**: For IO-constrained boards (TWS charging case etc.), SI and SO can be bridged with a 330Ω resistor into a single bidirectional data line. Uses `SPI_MODE_UNIDIR_1BIT` (half-duplex) — driver code doesn't change, SPI controller HW handles direction switching. Trade-off: Dual/Quad SPI multi-line reads become unavailable. See `5.存储介质与文件系统/1.JL平台对nandflash的支持.md` for details.

### Pre-compiled libraries

Many core subsystems are closed-source `.a` files in `cpu/br28/liba/`:

| Library | Contains |
|---------|---------|
| `btstack.a`, `btctrler.a` | Bluetooth protocol stack |
| `fs.a` | FAT/VFS filesystem implementations |
| `ftl.a` | NAND Flash Translation Layer |
| `device.a` | NAND/NOR flash device drivers, peripheral drivers |
| `media.a` | Audio codec/processing libraries |
| `system.a`, `cpu.a` | OS/kernel, CPU support |
| `update.a` | OTA update logic |

These are linked in `build/Makefile.mk`. Source code is NOT in this repo — only headers (in `apps/common/include/` and `interface/`) provide the public API. When investigating behavior, check the header files first, then look at analogous open-source code (e.g., `norflash.c` for flash driver patterns).

## Development workflow

- Edit source files in `SDK/apps/earphone/` and `SDK/audio/`
- Board-specific overrides go in `SDK/apps/earphone/board/br28/board_ac701n_demo_cfg.h`
- Build with `make` from the `SDK/` directory
- Firmware output appears in `SDK/cpu/br28/tools/`
- Flashing is done via the JL ISP tool (`isd_download.exe`) or the download batch script
- The `src/` JSON files are edited through the JL configuration GUI tool, not by hand, and generate the `output/` firmware

## Git branches

- `main` — base/public SDK
- `T2616` — current development branch (adds key programming/burning support)
