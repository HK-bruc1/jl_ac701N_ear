# GD5F4GM7xExxG

# SPI-NAND Flash Datasheet

DS-GD5F4GM7-Rev1.0 | September 2025

---

## Contents

1. [Feature](#1-feature)
2. [General Description](#2-general-description)
   - 2.1 [Valid Part Numbers](#21-valid-part-numbers)
   - 2.2 [Connection Diagram](#22-connection-diagram)
   - 2.3 [Pin Description](#23-pin-description)
   - 2.4 [Block Diagram](#24-block-diagram)
3. [Memory Mapping](#3-memory-mapping)
4. [Array Organization](#4-array-organization)
5. [Device Operation](#5-device-operation)
   - 5.1 [SPI Modes](#51-spi-modes)
   - 5.2 [Hold Mode](#52-hold-mode)
   - 5.3 [Write Protection](#53-write-protection)
   - 5.4 [Power Off Timing](#54-power-off-timing)
   - 5.5 [Data Strobe (DQS) Signal](#55-data-strobe-dqs-signal-bga24-only)
6. [Commands Description](#6-commands-description)
7. [Write Operations](#7-write-operations)
   - 7.1 [Write Enable (WREN) (06h)](#71-write-enable-wren-06h)
   - 7.2 [Write Disable (WRDI) (04h)](#72-write-disable-wrdi-04h)
8. [Read Operations](#8-read-operations)
   - 8.1 [Page Read](#81-page-read)
   - 8.2 [Page Read to Cache (13h)](#82-page-read-to-cache-13h)
   - 8.3 [Read From Cache (03h or 0Bh)](#83-read-from-cache-03h-or-0bh)
   - 8.4 [Read From Cache x2 (3Bh)](#84-read-from-cache-x2-3bh)
   - 8.5 [Read From Cache x4 (6Bh)](#85-read-from-cache-x4-6bh)
   - 8.6 [Read From Cache Dual IO (BBh)](#86-read-from-cache-dual-io-bbh)
   - 8.7 [Read From Cache Quad IO (EBh)](#87-read-from-cache-quad-io-ebh)
   - 8.8 [Read From Cache Quad I/O DTR (EEh)](#88-read-from-cache-quad-io-dtr-eeh)
   - 8.9 [Read ID (9Fh)](#89-read-id-9fh)
   - 8.10 [Read UID](#810-read-uid)
   - 8.11 [Read Parameter Page](#811-read-parameter-page)
   - 8.12 [Read CASN Page](#812-read-casn-page)
9. [Program Operations](#9-program-operations)
   - 9.1 [Page Program](#91-page-program)
   - 9.2 [Program Load (PL) (02h)](#92-program-load-pl-02h)
   - 9.3 [Program Load x4 (PL x4) (32h)](#93-program-load-x4-pl-x4-32h)
   - 9.4 [Program Execute (PE) (10h)](#94-program-execute-pe-10h)
   - 9.5 [Internal Data Move](#95-internal-data-move)
   - 9.6 [Program Load Random Data (84h)](#96-program-load-random-data-84h)
   - 9.7 [Program Load Random Data x4 (C4h/34h)](#97-program-load-random-data-x4-c4h34h)
10. [Erase Operations](#10-erase-operations)
    - 10.1 [Block Erase (D8h)](#101-block-erase-d8h)
11. [Deep Power Down Mode](#11-deep-power-down-mode)
    - 11.1 [Deep Power-Down (B9h)](#111-deep-power-down-b9h)
    - 11.2 [Release from Deep Power-Down (ABh)](#112-release-from-deep-power-down-abh)
12. [Assistant Bad Block Management](#12-assistant-bad-block-management)
    - 12.1 [Bad Block Management (A1h)](#121-bad-block-management-a1h)
    - 12.2 [Read Bad Block Link Table (A5h)](#122-read-bad-block-link-table-a5h)
13. [Read ECC Status Command (7Ch)](#13-read-ecc-status-command-7ch)
14. [Reset Operations](#14-reset-operations)
    - 14.1 [Soft Reset (FFh)](#141-soft-reset-ffh)
    - 14.2 [Enable Power On Reset (66h) and Power On Reset (99h)](#142-enable-power-on-reset-66h-and-power-on-reset-99h)
15. [Feature Operations](#15-feature-operations)
    - 15.1 [Get Features (0Fh) and Set Features (1Fh)](#151-get-features-0fh-and-set-features-1fh)
    - 15.2 [Status Register and Driver Register](#152-status-register-and-driver-register)
    - 15.3 [OTP Region](#153-otp-region)
    - 15.4 [Block Protection](#154-block-protection)
    - 15.5 [Power Lock Down Protection](#155-power-lock-down-protection)
    - 15.6 [Internal ECC](#156-internal-ecc)
16. [Power On/Off Timing](#16-power-onoff-timing)
17. [Absolute Maximum Ratings](#17-absolute-maximum-ratings)
18. [Capacitance Measurement Conditions](#18-capacitance-measurement-conditions)
19. [DC Characteristic](#19-dc-characteristic)
20. [AC Characteristics](#20-ac-characteristics)
21. [Performance and Timing](#21-performance-and-timing)
22. [Ordering Information](#22-ordering-information)
23. [Package Information](#23-package-information)
24. [Revision History](#24-revision-history)

---

## 1. Feature

- 4Gb SLC NAND Flash
- Page Size
  - Internal ECC On (ECC_EN=1, default): Page Size: 4096-Byte + 128-Byte
  - Internal ECC Off (ECC_EN=0): Page Size: 4096-Byte + 256-Byte
- Standard, Dual, Quad SPI, DTR
  - Standard SPI: SCLK, CS#, SI, SO, WP#, HOLD#
  - Dual SPI: SCLK, CS#, SIO0, SIO1, WP#, HOLD#
  - Quad SPI: SCLK, CS#, SIO0, SIO1, SIO2, SIO3
  - DTR (Double Transfer Rate) Read: SCLK, CS#, SIO0, SIO1, SIO2, SIO3
- High Speed Clock Frequency
  - 3.3V: 133MHz for Standard/Dual/Quad SPI; 104MHz for DTR Quad SPI
  - 1.8V: 104MHz for Standard/Dual/Quad SPI; 80MHz for DTR Quad SPI
- Software/Hardware Write Protection
  - Write protect all/portion of memory via software
  - Register protection with WP# Pin
  - Power Lock Down Protection
- Single Power Supply Voltage
  - Full voltage range for 1.8V: 1.7V ~ 2.0V
  - Full voltage range for 3.3V: 2.7V ~ 3.6V
- Advanced security Features
  - 40K-Byte OTP Region
- Program/Erase/Read Speed
  - Page Program time: 340us typical
  - Block Erase time: 3ms typical
  - Page read time: 180us maximum
- Low Power Consumption
  - 3.3V: 35mA maximum active current; 35uA maximum standby current
  - 1.8V: 30mA maximum active current; 30uA maximum standby current
- Enhanced access performance
  - 4Kbyte cache for fast random read
- Advanced Feature for NAND
  - Factory good block 0~block 255
  - Deep Power Down (1.8V only)
- Reliability
  - P/E cycles with ECC: Typical 80K
  - Data retention: 10 Years
- Internal ECC
  - 8bits / 528byte

**Note:**
1. ECC is on by default, and it can be disabled by the user.
2. The P/E cycles with ECC will be 60K at 105 degrees C operation temperature.
3. The maximum standby current is 50uA at 105 degrees C.

---

## 2. General Description

SPI (Serial Peripheral Interface) NAND Flash provides an ultra-cost effective while high density non-volatile memory storage solution for embedded systems, based on an industry-standard NAND Flash memory core. It is an attractive alternative to SPI-NOR and standard parallel NAND Flash, with advanced features.

- Total pin count is 8, including VCC and GND
- Density is 4Gb
- Superior write performance and cost per bit over SPI-NOR
- Significantly lower cost than parallel NAND

This low-pin-count NAND Flash memory follows the industry-standard serial peripheral interface, and always remains the same pin out from one density to another. The command sets resemble common SPI-NOR command sets, modified to handle NAND specific functions and added new features. GigaDevice SPI NAND is an easy-to-integrate NAND Flash memory, with specified designed features to ease host management:

- User-selectable internal ECC. ECC parity is generated internally during a page program operation. When a page is read to the cache register, the ECC parity is detected and corrects the errors when necessary. The 128-byte spare area is available even when internal ECC is enabled. The device outputs corrected data and returns an ECC error status.
- Internal data move or copy back with internal ECC. The device can be easily refreshed and manage garbage collection task, without need of shift in and out of data. This command string can only be used on blocks with the same parity attribute, and it is invalid for blocks that have already executed the BBM command.
- Power on Read with internal ECC. The device will automatically read the first page of the first block to cache after power on, then the host can directly read data from cache for easy boot. Also, the data is promised to be correct by internal ECC when ECC enabled.

It is programmed and read in page-based operations, and erased in block-based operations. Data is transferred to or from the NAND Flash memory array, page by page, to a data register and a cache register. The cache register is closest to I/O control circuits and acts as a data buffer for the I/O data; the data register is closest to the memory array and acts as a data buffer for the NAND Flash memory array operation. The cache register functions as the buffer memory to enable page and random data READ/WRITE and copy back operations. These devices also use a SPI status register that reports the status of device operation.

### 2.1 Valid Part Numbers

Please contact GigaDevice regional sales for the latest product selection and available form factors.

| Product Number | Density | Voltage | Package Type | Temperature |
| :--- | :--- | :--- | :--- | :--- |
| GD5F4GM7REYIG | 4Gbit | 1.7V to 2.0V | WSON8 (8x6mm) | -40 degrees C to 85 degrees C |
| GD5F4GM7REWIG | 4Gbit | 1.7V to 2.0V | WSON8 (6x5mm) | -40 degrees C to 85 degrees C |
| GD5F4GM7REBIG | 4Gbit | 1.7V to 2.0V | TFBGA24 (5x5 Ball Array) | -40 degrees C to 85 degrees C |
| GD5F4GM7UEYIG | 4Gbit | 2.7V to 3.6V | WSON8 (8x6mm) | -40 degrees C to 85 degrees C |
| GD5F4GM7UEWIG | 4Gbit | 2.7V to 3.6V | WSON8 (6x5mm) | -40 degrees C to 85 degrees C |
| GD5F4GM7UEBIG | 4Gbit | 2.7V to 3.6V | TFBGA24 (5x5 Ball Array) | -40 degrees C to 85 degrees C |
| GD5F4GM7REYJG | 4Gbit | 1.7V to 2.0V | WSON8 (8x6mm) | -40 degrees C to 105 degrees C |
| GD5F4GM7UEYJG | 4Gbit | 2.7V to 3.6V | WSON8 (8x6mm) | -40 degrees C to 105 degrees C |

### 2.2 Connection Diagram

**Figure 2-1. Connection Diagram**

```
Top View (WSON8 8x6mm / 6x5mm):
         CS#  1  |oooooooo|  8  VCC
       SO/SIO1 2  |oooooooo|  7  HOLD#/SIO3
       WP#/SIO2 3  |oooooooo|  6  SCLK
             VSS 4  |oooooooo|  5  SI/SIO0

Top View (TFBGA24 5x5 ball array):
    A1  A2  A3  A4  A5
    NC  NC  NC  NC  NC
    B1  B2  B3  B4  B5
    NC  SCLK VSS VCC  NC
    C1  C2  C3  C4  C5
    NC  CS#  DQS  WP#(IO2) NC
    D1  D2  D3  D4  D5
    NC  SI(IO0) HOLD#(IO3) NC
    E1  E2  E3  E4  E5
    NC  NC  SO(IO1) NC  NC
```

### 2.3 Pin Description

| Pin Name | I/O | Description |
| :--- | :--- | :--- |
| CS# | I | Chip Select input, active low |
| SO/SIO1 | I/O | Serial Data Output / Serial Data Input Output 1 |
| WP#/SIO2 | I/O | Write Protect, active low / Serial Data Input Output 2 |
| SI/SIO0 | I/O | Serial Data Input / Serial Data Input Output 0 |
| SCLK | I | Serial Clock input |
| HOLD#/SIO3 | I/O | Hold Input / Serial Data Input Output 3 |
| VCC | Supply | Power Supply |
| VSS | Ground | Ground |
| NC | - | Not Connect, not internal connection; can be driven or floated. |
| DQS (only for BGA24) | O | Data Strobe Signal Output |

**Note:**
1. CS# must be driven high if chip is not selected. Please don't leave CS# floating any time after power is on.
2. If WP# and HOLD# are unused with QE=0, they are recommended to be driven high by the host, or an external pull-up resistor should be placed on the PCB in order to avoid allowing WP# and HOLD# driven low.
3. If SIO2 is unused with QE=1, it is recommended to be driven high or low by the host, or an external pull-up resistor should be placed on the PCB in order to avoid allowing SIO2 input to float.
4. If the DQS Function is not used, this pin must be floating.

### 2.4 Block Diagram

**Figure 2-2. Block Diagram**

```
                         HOLD#/SIO3, WP#/SIO2
                                  |
    SCLK ----+                    |
             |    +---------------+---------------+
    SI/SIO0 -+--->|               |               |
             |    |    Serial NAND Controller     |
    SO/SIO1 -+--->|               |               |
             |    |               |               |
             |    |   Cache       |   NAND Array  |
    DQS -----+--->|   Register    |   Memory Core |
             |    |               |               |
    VCC/VSS -+--->|   Data        |   Status      |
             |    |   Register    |   Register    |
             |    +---------------+---------------+
```

---

## 3. Memory Mapping

For 4G:

| Level | Range | Description |
| :--- | :--- | :--- |
| Block | 0 ~ 2047 | RA<16:6> |
| Page | 0 ~ 63 | RA<5:0> |
| Byte | 0 ~ 4351 | CA<12:0> |

**Note:**
1. CA: Column Address. The 13-bit address is capable of addressing from 0 to 8191 bytes; however, only bytes 0 through 4351 are valid. Bytes 4352 through 8191 of each page are "out of bounds," do not exist in the device, and cannot be addressed.
2. RA: Row Address. RA<5:0> selects a page inside a block, and RA<16:6> selects a block.

---

## 4. Array Organization

**Table 4. Array Organization**

| Each device has | Each block has | Each page has |
| :--- | :--- | :--- |
| 4Gb | 256K+16K bytes | 4K+256 bytes |
| 2048 x 64 pages | 64 pages | - |
| 2048 blocks | - | - |

**Figure 4. Array Organization**

```
SO <--> | Cache Register | 4096 | 128 | <---> SI
        | Data Register  | 4096 | 128 | <---> NAND Memory Core

1 page = (4K + 128 bytes)
1 block = (4K + 128 bytes) x 64 pages = (256K + 8K) bytes

Per device (4G): 2048 Blocks
For 4G: = (256K + 8K) bytes x 2048 blocks = 4Gb (Internal ECC = ON)

When Internal ECC = OFF:
1 page = (4K + 256 bytes)
1 block = (4K + 256 bytes) x 64 pages = (256K + 16K) bytes
For 4G: = (256K + 16K) bytes x 2048 blocks = 4Gb
```

**Note:**
1. When Internal ECC is enabled, the user can program the first 128 bytes of the entire 256 bytes spare area and the last 128 bytes of the whole spare area cannot be programmed, the user can read the entire 256 bytes spare area.
2. When Internal ECC is disabled, the user can read and program the entire 256 bytes spare area.

---

## 5. Device Operation

### 5.1 SPI Modes

SPI NAND supports two SPI modes:

- CPOL = 0, CPHA = 0 (Mode 0)
- CPOL = 1, CPHA = 1 (Mode 3)

Input data is latched on the rising edge of SCLK and data shifts out on the falling edge of SCLK for both modes. All timing diagrams shown in this data sheet are mode 0.

**Note:** While CS# is HIGH, keep SCLK at VCC or GND (determined by mode 0 or mode 3). Do not toggle SCLK until CS# is driven LOW.

We recommend that the user pull CS# high when the SPI flash is not in use, it may cause damage to the flash. When CS# is high and SCLK at VCC or GND state, the device is in idle state.

**Standard SPI**

SPI NAND Flash features a standard serial peripheral interface on 4 signals bus: Serial Clock (SCLK), Chip Select (CS#), Serial Data Input (SI) and Serial Data Output (SO).

**Dual SPI**

SPI NAND Flash supports Dual SPI operation when using the x2 and dual IO commands. These commands allow data to be transferred to or from the device at two times the rate of the standard SPI. When using the Dual SPI command, the SI and SO pins become bidirectional I/O pins: SIO0 and SIO1.

**Quad SPI**

SPI NAND Flash supports Quad SPI operation when using the x4 and Quad IO commands. These commands allow data to be transferred to or from the device at four times the rate of the standard SPI. When using the Quad SPI command, the SI and SO pins become bidirectional I/O pins: SIO0 and SIO1, and WP# and HOLD# pins become SIO2 and SIO3.

**DTR Quad SPI**

The device supports DTR Quad SPI operation when using the "DTR Quad I/O Fast Read" command. This command allows data to be transferred to and from the device at eight times the rate of the standard SPI, and data output will be latched on both the rising and falling edges of the serial clock. When using the DTR Quad SPI command the SI and SO pins become bidirectional I/O pins: IO0 and IO1, and WP# and HOLD# pins become IO2 and IO3. DTR Quad SPI commands require the Quad Enable bit (QE) in the Status Register to be set.

### 5.2 Hold Mode

The HOLD# function is only available when QE=0. If QE=1, The HOLD# function is disabled, the pin acts as dedicated data I/O pin.

The HOLD# signal goes low to stop any serial communications with the device, but doesn't stop the operation of reading, programming, or erasing in progress.

The operation of HOLD, requires CS# to remain low, and starts on the falling edge of the HOLD# signal, with SCLK signal being low (if SCLK is not low, HOLD operation will not start until SCLK being low). The HOLD condition ends on the rising edge of HOLD# signal with SCLK being low (If SCLK is not low, HOLD operation will not end until SCLK being low).

The SO is high impedance, both SI and SCLK are ignored during the HOLD operation, if CS# drives high during HOLD operation, it will reset the internal logic of the device. To re-start communication with chip, the HOLD# must be set to high and then CS# must be set to low.

### 5.3 Write Protection

**Software Protection:** The BP[2:0] bits in the feature register control the write protection of the flash array. With BP[2:0] set to certain value, the corresponding memory area is protected from program and erase operations. The BP bits can be changed with the Set Features command.

**Hardware Protection:** When WP# is low (active low), the feature register becomes read-only and all program/erase commands are ignored. The WP# pin must be driven high or connected to VCC to allow normal program/erase operations. The WP# function is only available when QE=0. If QE=1, the WP# function is disabled.

**Power Lock Down Protection:** The device supports a Power Lock Down feature that prevents software from modifying the Block Protection settings. When Power Lock Down is enabled, the block protection bits are frozen and can only be reset by a power cycle. See Section 15.5 for details.

### 5.4 Power Off Timing

Please do not turn off the power before Write/Erase operation is completed. Avoid writing or erasing data when the power is low. Power shortage and/or power failure before Write/Erase operation is completed will lead to loss of data or damage to the data.

### 5.5 Data Strobe (DQS) Signal (BGA24 Only)

The DQS signal is a data strobe output that is only available on the BGA24 package. It is used to provide a timing reference for data output during DTR Quad I/O read operations (EEh command). The DQS signal toggles synchronously with the data output and can be used by the host to accurately capture data.

When the DQS function is not used, this pin must be floating.

---

## 6. Commands Description

**Table 6-1. Commands Set**

| Command Name | Byte1 | Byte2 | Byte3 | Byte4 | Byte5 | Byte6 | Byte 7 |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Write Enable | 06h | | | | | | |
| Write Disable | 04h | | | | | | |
| Get Features | 0Fh | A7-A0 | | D7-D0 | Wrap(7) | | |
| Set Feature | 1Fh | A7-A0 | | D7-D0 | | | |
| Page Read (to cache) | 13h | A23-A16 | | A15-A8 | A7-A0 | | |
| Read From Cache | 03h/0Bh(8) | A15-A8 | | A7-A0(2) | Dummy(1) | D7-D0 | |
| Read From Cache x2 | 3Bh | A15-A8 | | A7-A0(2) | Dummy(1) | D7-D0 | |
| Read From Cache x4 | 6Bh | A15-A8 | | A7-A0(2) | Dummy(1) | D7-D0 | |
| Read From Cache Dual IO | BBh | A15-A8 | | A7-A0(2) | Dummy(1) | D7-D0 | |
| Read From Cache Quad IO | EBh | A15-A8 | | A7-A0(2) | Dummyx2(1) | D7-D0 | Dummy |
| Read From Cache Quad I/O DTR | EEh | A31-A24 | | A23-A16 | A15-A8 | A7-A0(2) | D7-D0 |
| Read ID(4) | 9Fh | Dummy | | MID | DID | | |
| Read parameter page | 13h | 00h | | 00h | 01h | | |
| Read CASN page | 13h | 00h | | 00h | 01h | | |
| Read UID | 13h | 00h | | 00h | 00h | | |
| Program Load | 02h | A15-A8 | | A7-A0(3) | D7-D0 | Next byte | |
| Program Load x4 | 32h | A15-A8 | | A7-A0(3) | D7-D0 | Next byte | |
| Program Execute | 10h | A23-A16 | | A15-A8 | A7-A0 | | |
| Program Load Random Data | 84h | A15-A8 | | A7-A0(3) | D7-D0 | Next byte | |
| Program Load Random Data x4 | C4h/34h | A15-A8 | | A7-A0(3) | D7-D0 | Next byte | |
| Block Erase(256K) | D8h | A23-A16 | | A15-A8 | A7-A0 | | |
| Reset(5) | FFh | | | | | | |
| Enable Power on Reset | 66h | | | | | | |
| Power on Reset(6) | 99h | | | | | | |
| Deep Power Down (1.8V only) | B9h | | | | | | |
| Release Deep Power Down (1.8V only) | ABh | | | | | | |
| ECC Status Read | 7Ch | Dummy | | D7-D0 | | | |
| Bad Block Management (Swap Blocks) | A1h | LBA | | LBA | PBA | PBA | |
| Read Bad Block Link Table | A5h | Dummy | LBA0 | LBA0 | PBA0 | PBA0 | LBA1 |

**Note:**
1. 03h/0Bh/3Bh/6Bh has 8 clock, 1 byte dummy. BBh has 4 clock, 1 byte dummy. EBh has 4 clock, 2 bytes dummy. EEh has 8 clock, 8 bytes dummy.
2. The A15-A0 (03h/0Bh/3Bh/6Bh) has 3-bit dummy & 13-bit column address.
3. The A31-A0 has 19-bit dummy & 13-bit column address.
4. MID is Manufacture ID (C8h for GigaDevice), DID is Device ID.
5. Reset command:
   - Reset will reset PAGE READ/PROGRAM/ERASE operation.
   - Reset will reset status register bits P_FAIL/E_FAIL/WEL/OIP/ECCS/ECCSE.
   - The reset function must be forbidden throughout OTP PGM and BBM PGM.
6. Power on reset: Retrieve status register and data in cache to power on status.
7. The output would be updated by real-time, until CS# is driven high.
8. Read UID/parameter/CASN page are same as page read to cache.

---

## 7. Write Operations

### 7.1 Write Enable (WREN) (06h)

The Write Enable (WREN) command is for setting the Write Enable Latch (WEL) bit. The Write Enable Latch (WEL) bit must be set prior to following operations that change the contents of the memory array:

- Page program
- OTP program/OTP protection
- Block erase
- BBM program

The WEL bit can be cleared after a reset command.

**Figure 7-1. Write Enable Sequence Diagram**

```
CS#  :  |_____________________________________|
        0   1   2   3   4   5   6   7
SCLK :  |____|    |____|    |____|    |____|    |____|    |____|    |____|    |____|
SI   :  |  0   0   0   0   0   1   1   0   |  (06h)
SO   :  |________High-Z________|
```

### 7.2 Write Disable (WRDI) (04h)

The Write Disable command is used to reset the Write Enable Latch (WEL) bit. The WEL bit is reset by following condition:

- Page program
- OTP program/OTP protection
- Block erase
- BBM program

**Figure 7-2. Write Disable Sequence Diagram**

```
CS#  :  |_____________________________________|
        0   1   2   3   4   5   6   7
SCLK :  |____|    |____|    |____|    |____|    |____|    |____|    |____|    |____|
SI   :  |  0   0   0   0   0   1   0   0   |  (04h)
SO   :  |________High-Z________|
```

---

## 8. Read Operations

### 8.1 Page Read

The PAGE READ (13h) command transfers the data from the NAND Flash array to the cache register. The command sequence is as follows:

- 13h (PAGE READ to cache)
- 0Fh (GET FEATURES command to read the status)
- 03h or 0Bh (Read from cache)/3Bh (Read from cache x2)/6Bh (Read from cache x4)/BBh (Read from cache dual IO)/EBh (Read from cache Quad IO)/EEh (Read from cache Quad IO DTR)

The PAGE READ command requires a 24-bit address. After the block/page addresses are registered, the device starts the transfer from the main array to the cache register, and is busy for tRD time. During this time, the GET FEATURE (0Fh) command can be issued to monitor the status. Followed the page read operation, the RANDOM DATA READ (03h/0Bh/3Bh/6Bh/BBh/EBh/EEh) command must be issued in order to read out the data from cache. The output data starts at the initial address specified in the command, once it reaches the ending boundary of the whole page section, the output will wrap around from the beginning boundary until CS# is pulled high to terminate this operation. Refer to the waveforms to view the entire READ operation.

**Note:**
1. The command 6Bh (Read from cache x4)/EBh (Read from cache Quad IO)/EEh (Read from cache Quad IO DTR) is only available with the QE bit is enabled.
2. When the user reads to the end of 128-Byte spare area, it doesn't wrap around from the beginning boundary and an additional 128-Byte ECC code will be read with Internal ECC enabled.

### 8.2 Page Read to Cache (13h)

The command page read to cache reads the data from the flash array to the cache register.

**Figure 8-1. Page Read to Cache Sequence Diagram**

```
Step 1: Issue Page Read command
CS#  :  |_________________________________________________________________________|
        0 1  2 3 4  5 6  7 8 9  10     28 29  30 31
SCLK :  |_|  |_|  |_|  |_|  |_|      |_|  |_|  |_|
SI   :  | Command |  24-bit address  |
        |   13h   | 23..3 2 1 0
SO   :  |________________High-Z_________________|

Step 2: Poll Status (Get Features)
CS#  :  |_________________________________________________________|
        0 1 2  3 4 5 6 7  8 9  10 11  12 13  14   15
SCLK :  |_|  |_|  |_|  |_|  |_|  |_|  |_|  |_|   |_|  |_|
SI   :  | Get Feature |  1 byte addr  |
        |     0Fh     |  7 6 5 4 3 2 1 0  (C0h)
SO   :  |____High-Z___|      Status data byte
                              7 6 5 4 3 2 1 0
                              |  |  |  |  |  |  |__ OIP
                              |  |  |  |  |  |_____ WEL
                              |  |  |  |  |________ E_FAIL
                              |  |  |  |___________ P_FAIL
                              |  |  |______________ ECCS
                              |  |_________________ ECCS
                              |____________________ Reserved

Step 3: Read Data from Cache (if OIP=0)
CS#  :  |_______________________________________________________________________________|
        16 17  18 19 20  21 22 23  24
SCLK :  |_|  |_|  |_|  |_|  |_|  |_|  |_|  |_|  |_|  |_|  |_|  |_|  |_|  |_|  |_|  |_|
SI   :  | Data byte address |
SO   :  |  Data byte 0  |  Data byte 1  |
        7 6 5 4 3 2 1 0  7 6 5 4 3 2 1 0
```

### 8.3 Read From Cache (03h or 0Bh)

The command sequence is shown below.

This command reads data from the cache register using the Standard SPI interface (1-1-1). The command is followed by a 16-bit column address (3 dummy bits + 13-bit column address) and 8 dummy clocks. After the dummy clocks, the device outputs data starting from the specified column address.

### 8.4 Read From Cache x2 (3Bh)

The command sequence is shown below.

The Read from Cache x2 command (3Bh) is similar to the Read from Cache command but with the capability to output the data bytes by two pins: SIO0 and SIO1. The command is followed by a 16-bit column address (3 dummy bits + 13-bit column address) and 8 dummy clocks. After the dummy clocks, the device outputs data on both SIO0 and SIO1 starting from the specified column address.

### 8.5 Read From Cache x4 (6Bh)

The Quad Enable bit (QE) of feature (B0[0]) must be set to enable the read from cache x4. The command sequence is shown below.

The Read from Cache x4 command (6Bh) is similar to the Read from Cache x2 command but with the capability to output the data bytes by four pins: SIO0, SIO1, SIO2, and SIO3. The command is followed by a 16-bit column address (3 dummy bits + 13-bit column address) and 8 dummy clocks. After the dummy clocks, the device outputs data on all four SIO pins starting from the specified column address.

### 8.6 Read From Cache Dual IO (BBh)

The Read from Cache Dual I/O command (BBh) is similar to the Read from Cache x2 command but uses both SIO0 and SIO1 for address input as well. The command is followed by 3 dummy bits and a 13-bit column address input on two pins, then 4 dummy clocks. After the dummy clocks, data is output on both SIO0 and SIO1.

### 8.7 Read From Cache Quad IO (EBh)

The Read from Cache Quad IO command is similar to the Read from Cache x4 command but uses all four SIO pins for address input as well. The command is followed by 3 dummy bits and a 13-bit column address input on four pins, then 4 dummy clocks and 2 dummy bytes. After the dummy clocks, data is output on all four SIO pins.

### 8.8 Read From Cache Quad I/O DTR (EEh)

The DTR Quad IO command enables Double Transfer Rate throughput on quad I/O of SPI in read mode. The Quad Enable (QE) bit of status register must be set to "1" before issuing this command.

The command is followed by a 32-bit address field (19 dummy bits + 13-bit column address), 8 dummy clocks, and 8 dummy bytes. Data is then output on all four IO pins at double the rate (latched on both rising and falling edges of SCLK).

**Note:** The DQS signal is only available on BGA24 package and only valid during EEh command.

### 8.9 Read ID (9Fh)

The READ ID command is used to identify the NAND Flash device.

- The READ ID command outputs the Manufacturer ID and the device ID.

**Figure 8-8. Read ID Sequence Diagram**

```
CS#  :  |_________________________________________________________________________|
        0 1  2 3 4  5 6  7 8  9 10 11 12  13 14 15 16  17 18 19 20 21 22 23
SCLK :  |_|  |_|  |_|  |_|  |_|  |_|  |_|  |_|  |_|  |_|  |_|  |_|  |_|  |_|
SI   :  | Command | Dummy byte |
        |   9Fh   |   FFh
SO   :  |____High-Z___|  Manufacturer ID  |  Device ID
                          C8h                94h (3.3V) / 84h (1.8V)
```

**Table 8-1. Read ID Table**

| MID | Manufacturer |
| :--- | :--- |
| C8h | GigaDevice |

| DID | Device | Voltage |
| :--- | :--- | :--- |
| 94h | GD5F4GM7UE | 3.3V (2.7V~3.6V) |
| 84h | GD5F4GM7RE | 1.8V (1.7V~2.0V) |

### 8.10 Read UID

The Read Unique ID function is used to retrieve the 16 bytes unique ID (UID) for the device. The UID is factory programmed and is unique for each device. The UID data may be stored within the OTP area and is read using the same sequence as Page Read to Cache.

To read the UID:
1. Set OTP_EN=1 in the Feature Register (B0h) to enable OTP access mode
2. Issue Page Read to Cache (13h) with address 00h 00h 00h
3. Read data from cache (32 bytes: 16 bytes UID + 16 bytes bit-wise complement)
4. Clear OTP_EN=0 to exit OTP access mode

**Table 8-2. Read UID Table**

| Device | UID Size | Complement Size | Total |
| :--- | :--- | :--- | :--- |
| 4G | 16 bytes | 16 bytes | 32 bytes |

### 8.11 Read Parameter Page

The Read Parameter Page function retrieves the data structure that describes the memory organization, timing parameters, and other behavioral parameters. This data structure enables the host processor to identify the specific GigaDevice SPI-NAND device and its parameters.

To read the Parameter Page:
1. Set OTP_EN=1 in the Feature Register (B0h) to enable OTP access mode
2. Issue Page Read to Cache (13h) with address 00h 00h 01h
3. Read 768 bytes from cache (Parameter Page data, repeated 3 times = 2304 bytes total)
4. Clear OTP_EN=0 to exit OTP access mode

**Parameter page table as follow (4G):**

| Byte | O/M | Description | 3.3V | 1.8V |
| :--- | :--- | :--- | :--- | :--- |
| 0-3 | M | Parameter page signature | 4Fh 4Eh 46h 49h | 4Fh 4Eh 46h 49h |
| | | Byte 0: 4Fh, "O" | | |
| | | Byte 1: 4Eh, "N" | | |
| | | Byte 2: 46h, "F" | | |
| | | Byte 3: 49h, "I" | | |
| 4-5 | M | Revision number | 00h 01h | 00h 01h |
| 6-7 | M | Feature supported | 00h 00h | 00h 00h |
| 8-9 | M | Optional commands supported | 00h 00h | 00h 00h |
| 10-31 | | Reserved | 00h | 00h |
| 32-43 | M | Device manufacturer | 47h 49h 47h 41h 44h 45h 56h 49h 43h 45h 20h 20h | 47h 49h 47h 41h 44h 45h 56h 49h 43h 45h 20h 20h |
| | | "GIGADEVICE " | | |
| 44-63 | M | Device model | 47h 44h 35h 46h 34h 47h 4Dh 37 | 47h 44h 35h 46h 34h 47h 4Dh 37 |
| | | "GD5F4GM7" | | |
| 64 | M | JEDEC manufacturer ID | C8h | C8h |
| 65-66 | O | Date code | 00h 00h | 00h 00h |
| 67-79 | | Reserved | 00h | 00h |
| 80-83 | M | Number of data bytes per page | 00h 10h 00h 00h (4096) | 00h 10h 00h 00h |
| 84-85 | M | Number of spare bytes per page | 80h 00h (128) | 80h 00h |
| 86-89 | M | Number of data bytes per partial page | 00h 10h 00h 00h | 00h 10h 00h 00h |
| 90-91 | M | Number of spare bytes per partial page | 80h 00h | 80h 00h |
| 92-95 | M | Number of pages per block | 00h 00h 40h 00h (64) | 00h 00h 40h 00h |
| 96-99 | M | Number of blocks per logical unit (LUN) | 00h 08h 00h 00h (2048) | 00h 08h 00h 00h |
| 100 | M | Number of logical units (LUNs) | 01h | 01h |
| 101 | M | Number of address cycles | 03h | 03h |
| 102 | M | Number of bits per cell | 01h (SLC) | 01h |
| 103-104 | M | Bad blocks maximum per LUN | C8h 00h (200) | C8h 00h |
| 105-106 | M | Block endurance | 05h 00h | 05h 00h |
| 107 | M | Guaranteed valid blocks at beginning of target | 01h | 01h |
| 108-109 | M | Block endurance for guaranteed valid blocks | 05h 00h | 05h 00h |
| 110 | M | Number of programs per page | 04h | 04h |
| 111 | M | Partial programming attributes | 00h | 00h |
| | | bit 4: 1 = partial page layout is partial page data followed by partial page spare | | |
| | | bit 1-0: 00 = partial page programming not supported | | |
| 112 | M | Number of bits ECC correctability | 08h (8 bits) | 08h |
| 113 | M | Number of interleaved address bits | 00h | 00h |
| 114 | M | Interleaved operation attributes | 00h | 00h |
| 115-127 | | Reserved | 00h | 00h |
| 128-129 | M | I/O pin capacitance per signal line | 00h 00h | 00h 00h |
| 130-131 | M | Timing mode support | 00h 00h | 00h 00h |
| 132-133 | M | Program cache timing mode support | 00h 00h | 00h 00h |
| 134-135 | M | tPROG (Maximum page program time) | 58h 02h (600us) | 58h 02h |
| 136-137 | M | tBERS (Maximum block erase time) | 10h 27h (10ms) | 10h 27h |
| 138-139 | M | tR (Maximum page read time) | B4h 00h (180us) | B4h 00h |
| 140-141 | M | tCCS (Minimum change column setup time) | 00h 00h | 00h 00h |
| 142-143 | M | Source synchronous timing mode support | 00h 00h | 00h 00h |
| 144-145 | M | tRmp (Source synchronous timing mode multiplier) | 00h 00h | 00h 00h |
| 146-147 | M | tADL (Programmable output drive impedance) | 00h 00h | 00h 00h |
| 148-149 | M | tCAD (Minimum data cache busy time) | 00h 00h | 00h 00h |
| 150-163 | | Reserved | 00h | 00h |
| 164-165 | M | Vendor specific revision number | 00h 01h | 00h 01h |
| 166-167 | M | Vendor specific | 00h 00h | 00h 00h |
| 168-173 | M | Vendor specific | 00h 00h 00h 00h 00h 00h | 00h 00h 00h 00h 00h 00h |
| 174-175 | M | Vendor specific | 00h 00h | 00h 00h |
| 176-177 | M | Vendor specific | 00h 00h | 00h 00h |
| 178-179 | M | Vendor specific | 00h 00h | 00h 00h |
| 180-181 | M | Vendor specific | 00h 00h | 00h 00h |
| 182-183 | M | Vendor specific | 00h 00h | 00h 00h |
| 184-185 | M | Vendor specific | 00h 00h | 00h 00h |
| 186-253 | | Vendor specific (Reserved) | 00h | 00h |
| 254-255 | M | Integrity CRC | CRC-16 | CRC-16 |

**Notes:**
1. "O" Stands for Optional, "M" for Mandatory.
2. The Integrity CRC (Cycling Redundancy Check) field is used to verify that the parameter page data was transferred correctly to the host.

### 8.12 Read CASN Page

The Read CASN Page function retrieves the data structure that describes the chip specific advanced setting and other behavioral parameters. This data structure enables the host processor to identify the specific GigaDevice SPI-NAND CASN device and its parameters.

To read the CASN Page:
1. Set OTP_EN=1 in the Feature Register (B0h) to enable OTP access mode
2. Issue Page Read to Cache (13h) with address 00h 00h 01h
3. Read bytes 768-1535 from cache (768 bytes CASN Page data)
4. Clear OTP_EN=0 to exit OTP access mode

**CASN page table as follow (4G):**

| Byte | O/M | Description | 3.3V | 1.8V |
| :--- | :--- | :--- | :--- | :--- |
| 0-3 | M | CASN page signature | 47h 44h 35h 46h | 47h 44h 35h 46h |
| | | Byte 0: 47h, "G" | | |
| | | Byte 1: 44h, "D" | | |
| | | Byte 2: 35h, "5" | | |
| | | Byte 3: 46h, "F" | | |
| 4-5 | M | Revision number | 00h 01h | 00h 01h |
| 6-7 | M | CASN parameters length | 00h 03h (768 bytes) | 00h 03h |
| 8-19 | M | Device model | 47h 44h 35h 46h 34h 47h 4Dh 37 00h 00h 00h 00h | 47h 44h 35h 46h 34h 47h 4Dh 37 00h 00h 00h 00h |
| 20-23 | M | Device capacity | 00h 00h 00h 04h (4Gb) | 00h 00h 00h 04h |
| 24 | M | Cell type | 01h (SLC) | 01h |
| 25 | M | Simultaneously programmed pages | 01h | 01h |
| 26-27 | M | ECC capability | 00h 08h (8 bits) | 00h 08h |
| 28-29 | M | Bad blocks maximum per LUN | 00h C8h (200) | 00h C8h |
| 30-31 | M | Block endurance | 00h 05h | 00h 05h |
| 32 | M | Guaranteed valid blocks at beginning | FFh (255) | FFh |
| 33-34 | M | Block endurance for guaranteed blocks | 00h 05h | 00h 05h |
| 35 | M | Number of bits per cell | 01h (SLC) | 01h |
| 36-37 | M | Number of address cycles | 00h 03h | 00h 03h |
| 38-39 | M | Number of bytes per page (main) | 10h 00h (4096) | 10h 00h |
| 40-41 | M | Number of spare bytes per page | 00h 80h (128) | 00h 80h |
| 42-43 | M | Number of pages per block | 00h 40h (64) | 00h 40h |
| 44-45 | M | Number of blocks per LUN | 08h 00h (2048) | 08h 00h |
| 46 | M | Number of LUNs | 01h | 01h |
| 47 | M | OTP pages | 0Ah (10 pages) | 0Ah |
| 48-49 | M | OTP page size | 10h 00h (4096) | 10h 00h |
| 50-51 | M | OTP spare size | 00h 80h (128) | 00h 80h |
| 52 | M | ECC type | 01h (Hamming/BCH) | 01h |
| 53-55 | M | Reserved | 00h 00h 00h | 00h 00h 00h |

| Byte | Description | 3.3V | 1.8V |
| :--- | :--- | :--- | :--- |
| 830-833 | luns per target | 00h 00h 01h 00h | 00h 00h 01h 00h |
| 850 | cmd 03h: SDR 1_1_1 read | 03h | 03h |
| 851 | bit7~4: address nbytes, bit3~0: dummy nbytes | 21h | 21h |
| 852 | cmd 0Bh: SDR 1_1_1 fast read | 0Bh | 0Bh |
| 853 | bit7~4: address nbytes, bit3~0: dummy nbytes | 21h | 21h |
| ... | (Additional SPI command descriptors) | ... | ... |
| 997 | status bytes (max=2) | 01h | 01h |
| 998 | status register mask0 | 00h | 00h |
| 999 | status register mask1 | 30h | 30h |
| 1000 | post process operator: 0=none, 1=& | 00h | 00h |

**Notes:**
1. The Integrity CRC (Cycling Redundancy Check) field is used to verify that the CASN page data was transferred correctly to the host. The CRC shall be calculated using the following 16-bit generator polynomial: G(X) = X^16 + X^15 + X^2 + 1.

---

## 9. Program Operations

### 9.1 Page Program

The PAGE PROGRAM operation sequence programs 1 byte to whole page bytes of data within a page. The typical program sequence is as follows:

- 02h (PROGRAM LOAD)/32h (PROGRAM LOAD x4) + data load (to cache)
- 10h (PROGRAM EXECUTE) + 24-bit address (cache to array)
- 0Fh (GET FEATURES command to read the status) + C0h

For random data load within a page, after the initial PROGRAM LOAD command, the PROGRAM LOAD RANDOM DATA (84h/C4h/34h) command can be issued to load data to specific column addresses within the cache before the PROGRAM EXECUTE command.

The device supports Partial Page Program, which allows multiple program operations within a single page (up to 4 partial programs when ECC is enabled). However, each partial program must be to a different area of the page, and the total data programmed cannot exceed the page size.

**Partial Page Program constraints:**

| ECC State | Max Partial Programs | Notes |
| :--- | :--- | :--- |
| ECC Enabled | 4 | Each partial must target different ECC segment |
| ECC Disabled | No limit | Full page can be programmed in multiple operations |

### 9.2 Program Load (PL) (02h)

The Program Load (02h) command is used to load data into the cache register. The command consists of an 8-bit Op code, followed by 3 dummy bits, and a 13-bit column address, then the data bytes to be programmed. Data is loaded sequentially into the cache register starting from the specified column address.

**Figure 9-1. Program Load Sequence Diagram**

```
CS#  :  |_________________________________________________________________________|
        0 1  2 3 4  5 6  7 8 9  10     22 23  24 25     31 32  33 34 35 36 37 38 39
SCLK :  |_|  |_|  |_|  |_|  |_|      |_|  |_|  |_|     |_|  |_|  |_|  |_|  |_|  |_|
SI   :  | Command | Col Address |    Data Byte 0    |    Data Byte 1    |
        |   02h   | 0 0 0 A12-A0 | D7-D0            | D7-D0
SO   :  |________________High-Z______________________________________________________|
```

### 9.3 Program Load x4 (PL x4) (32h)

The Program Load x4 command (32h) is similar to the Program Load command (02h) but with the capability to input the data bytes by four pins: SIO0, SIO1, SIO2, and SIO3. The Quad Enable (QE) bit must be set to enable this command.

The command consists of an 8-bit Op code, followed by 3 dummy bits, and a 13-bit column address, then the data bytes input on four pins.

### 9.4 Program Execute (PE) (10h)

After the data is loaded, a PROGRAM EXECUTE (10h) command must be issued to initiate the transfer of data from the cache register to the main array. PROGRAM EXECUTE consists of an 8-bit Op code, followed by a 24-bit address (3 address bytes). Upon completion, the Write Enable Latch (WEL) bit will be cleared.

**Figure 9-2. Program Execute Sequence Diagram**

```
CS#  :  |_________________________________________________________________________|
        0 1  2 3 4  5 6  7 8 9  10     28 29  30 31
SCLK :  |_|  |_|  |_|  |_|  |_|      |_|  |_|  |_|
SI   :  | Command |   24-bit Row Address   |
        |   10h   | A23-A16 | A15-A8 | A7-A0
SO   :  |________________High-Z_________________|
```

### 9.5 Internal Data Move

The INTERNAL DATA MOVE command sequence programs or replaces data in a page with existing data. The INTERNAL DATA MOVE command sequence is as follows:

- 13h (PAGE READ to cache) + 24-bit source address
- [Optional] 84h/C4h/34h (PROGRAM LOAD RANDOM DATA) for data modification
- 10h (PROGRAM EXECUTE) + 24-bit target address
- 0Fh (GET FEATURES command to read the status)

This command is useful for page refresh and garbage collection operations without the need to transfer data out of the device and back in.

**Important constraints:**
1. The INTERNAL DATA MOVE command can only be used between blocks with the same parity attribute (both even or both odd numbered blocks).
2. The command is invalid for blocks that have already executed the BBM command.

### 9.6 Program Load Random Data (84h)

The Program Load Random Data command programs or replaces data in a page with existing data. The command consists of an 8-bit Op code, followed by 3 dummy bits, and a 13-bit column address, then the data bytes. This command is used to modify data at a specific column address within the cache without affecting other data already loaded.

This command is typically used in combination with the INTERNAL DATA MOVE command to modify specific bytes within a page before programming.

### 9.7 Program Load Random Data x4 (C4h/34h)

The Program Load Random Data x4 command (C4h/34h) is similar to the Program Load Random Data command but with the capability to input the data bytes by four pins: SIO0, SIO1, SIO2, and SIO3. The Quad Enable (QE) bit must be set to enable this command.

---

## 10. Erase Operations

### 10.1 Block Erase (D8h)

The BLOCK ERASE (D8h) command is used to erase at the block level. The BLOCK ERASE command operates on one block at a time.

The command sequence for BLOCK ERASE is as follows:

1. 06h (WRITE ENABLE)
2. D8h (BLOCK ERASE) + 24-bit block address
3. 0Fh (GET FEATURES command to read the status) + C0h
4. Wait for OIP bit = 0 (erase complete)

After the block erase operation is complete, all bytes in the erased block will be set to FFh.

**Figure 10-1. Block Erase Sequence Diagram**

```
CS#  :  |_________________________________________________________________________|
        0 1  2 3 4  5 6  7 8 9  10     28 29  30 31
SCLK :  |_|  |_|  |_|  |_|  |_|      |_|  |_|  |_|
SI   :  | Command |   24-bit Row Address   |
        |   D8h   | A23-A16 | A15-A8 | A7-A0
SO   :  |________________High-Z_________________|
```

---

## 11. Deep Power Down Mode

### 11.1 Deep Power-Down (B9h)

Executing the Deep Power-Down (DP) command is the only way to put the device in the lowest consumption mode (the Deep Power-Down Mode). It can be used to reduce the standby current, especially for battery-powered applications.

The command sequence for Deep Power-Down is:

1. Drive CS# low
2. Send B9h command
3. Drive CS# high
4. Wait for tDP (max 3us)
5. Device enters Deep Power-Down mode

While in Deep Power-Down mode:
- Standby current is reduced to 1uA maximum (1.8V)
- All commands are ignored except ABh (Release from DPD), FFh (Reset), 66h (Enable Power-On Reset), and 99h (Power-On Reset)
- The device will automatically exit Deep Power-Down mode on power-up

**Note:** This command is only available for 1.8V devices.

### 11.2 Release from Deep Power-Down (ABh)

To release the device from the Power-Down state, the command is issued by driving CS# low, shifting the instruction code "ABh" and driving CS# high.

The command sequence for Release from Deep Power-Down is:

1. Drive CS# low
2. Send ABh command
3. Drive CS# high
4. Wait for tRES1 (max 50us)
5. Device returns to standby mode

After the device is released from Deep Power-Down mode, the normal command set is available.

**Note:** This command is only available for 1.8V devices. The device will also exit Deep Power-Down mode automatically when power is cycled.

---

## 12. Assistant Bad Block Management

As a NAND Flash, the device may have blocks that are invalid when shipped from the factory. The number of valid blocks (NVB) of the total available blocks are specified in the parameter page.

A bad block is defined as a block that contains one or more bad bits. Bad blocks do not affect the performance of good blocks because each block is independent with isolated transistor cells.

The invalid blocks are identified at the factory by the manufacturer. The factory tests the device under worst-case conditions and marks the blocks that fail as "bad." Blocks that are marked as bad should not be erased or programmed, as this may cause unpredictable results.

The factory bad block marking method:

| Check Point | Location | Value | Description |
| :--- | :--- | :--- | :--- |
| Factory Bad Block Mark | First page of each block, byte 4096 | Non-FFh | Block is bad |
| | | FFh | Block is good |

### 12.1 Bad Block Management (A1h)

The device offers a convenient method to manage the bad blocks logical address. It can be easily done by linking the bad block with the good block that exist at the shipment and after shipment.

The command sequence for Bad Block Management is:

1. 06h (WRITE ENABLE)
2. A1h (BAD BLOCK MANAGEMENT) + LBA (11-bit logical block address) + PBA (11-bit physical block address)
3. 0Fh (GET FEATURES command to read the status)
4. Wait for OIP bit = 0 (BBM operation complete)

After successful execution, the logical block address (LBA) will be redirected to the physical block address (PBA). The original PBA block will be marked as bad and is no longer accessible.

**Table 12-1. BBM Command Format**

| Byte | Description |
| :--- | :--- |
| Byte 1 | A1h (Command) |
| Byte 2 | LBA[10:8] (3 MSB of Logical Block Address) + 5-bit dummy |
| Byte 3 | LBA[7:0] (8 LSB of Logical Block Address) |
| Byte 4 | PBA[10:8] (3 MSB of Physical Block Address) + 5-bit dummy |
| Byte 5 | PBA[7:0] (8 LSB of Physical Block Address) |

**Notes:**
1. The BBM command can only be executed once per block. After a block has been mapped, it cannot be mapped again.
2. The BBM command is non-volatile. The mapping information is stored in a dedicated area and survives power cycles.
3. Maximum 40 BBM entries are supported per die.
4. The reset function must be forbidden throughout BBM program.

### 12.2 Read Bad Block Link Table (A5h)

The Read Bad Block Link Table command is used to read all the BBM entries that have been programmed.

The command sequence is:

1. Drive CS# low
2. Send A5h command + 8-bit dummy
3. Read LBA0, PBA0, LBA1, PBA1, ... (each entry is 4 bytes: 2 bytes LBA + 2 bytes PBA)
4. Drive CS# high

**Table 12-2. BBM Link Table Format**

| Entry | Bytes | Description |
| :--- | :--- | :--- |
| Entry 0 | Byte 1-2 | LBA0 (Logical Block Address 0) |
| | Byte 3-4 | PBA0 (Physical Block Address 0) |
| Entry 1 | Byte 5-6 | LBA1 (Logical Block Address 1) |
| | Byte 7-8 | PBA1 (Physical Block Address 1) |
| ... | ... | ... |
| Entry 39 | Byte 157-158 | LBA39 (Logical Block Address 39) |
| | Byte 159-160 | PBA39 (Physical Block Address 39) |

**LBA[15:14] status bits:**

| LBA[15] | LBA[14] | Status |
| :--- | :--- | :--- |
| 0 | 0 | Link available (not used) |
| 1 | 0 | Link used and valid |
| 1 | 1 | Link used but invalid |
| 0 | 1 | Reserved (NA) |

Unused entries will output 00h for both LBA and PBA fields.

**Note:** BBLS (Bad Block Link Status) in the Feature Status Register (F0h) bit 3 = 1 indicates that all 40 BBM entries have been used.

---

## 13. Read ECC Status Command (7Ch)

The Read ECC Status Command (7Ch) is used to monitor the device Internal ECC status after the read operation. The output data of the command is the same as the **4bits ECC Status (ECCS1, ECCS0, ECCSE1, ECCSE0)** in the Feature Register. The purpose of this command is to provide the user with a quick way to read the ECC Status instead of the complex Get Feature method. This command is only available with Internal ECC on (ECC_EN=1).

The command sequence is:

1. Drive CS# low
2. Send 7Ch command + 8-bit dummy
3. Read 1 byte ECC status data (bits 4-1 contain the 4-bit ECC status)
4. Drive CS# high

**Table 13-1. ECC Status Read Output**

| Bit | Name | Description |
| :--- | :--- | :--- |
| 7-5 | Reserved | Reserved (0) |
| 4 | ECCSE1 | ECC Status Extension bit 1 (from F0h[5]) |
| 3 | ECCSE0 | ECC Status Extension bit 0 (from F0h[4]) |
| 2 | ECCS1 | ECC Status bit 1 (from C0h[6]) |
| 1 | ECCS0 | ECC Status bit 0 (from C0h[5]) |
| 0 | Reserved | Reserved (0) |

The ECC status bits are the same as those in the Status Register (C0h[6:5]) and Feature Status Register (F0h[5:4]). See Section 15.2 for the complete ECC status mapping.

**Note:** When ECC is disabled (ECC_EN=0), this command returns undefined data.

---

## 14. Reset Operations

### 14.1 Soft Reset (FFh)

The Soft Reset command is used to reset the device and abort any ongoing operations. The reset command sequence is:

1. Drive CS# low
2. Send FFh command
3. Drive CS# high
4. Wait for tRST (5-500us depending on operation)
5. Device returns to idle state

After the reset command:
- All ongoing operations (PAGE READ/PROGRAM/ERASE) are aborted
- Status register bits P_FAIL/E_FAIL/WEL/OIP/ECCS/ECCSE are cleared
- Feature register settings are preserved (not affected by soft reset)
- The device returns to the standby state

**Important:** The reset function must be forbidden throughout OTP program and BBM program. Resetting during these operations may cause permanent damage to the device.

**Table 14-1. Reset Timing**

| Operation being reset | tRST (typical) |
| :--- | :--- |
| Standby / Read | 5 us |
| Program | 10 us |
| Erase | 500 us |

### 14.2 Enable Power On Reset (66h) and Power On Reset (99h)

The Enable Power On Reset and Power On Reset commands are used to reset the device to its power-on default state. This sequence resets both the status register and the feature register to their default values.

The command sequence is:

1. Drive CS# low
2. Send 66h command (Enable Power On Reset)
3. Drive CS# high
4. Drive CS# low
5. Send 99h command (Power On Reset)
6. Drive CS# high
7. Wait for tRST (5-10us)
8. Device returns to power-on default state

After the Power On Reset sequence:
- Status register bits P_FAIL/E_FAIL/WEL/OIP/ECCS/ECCSE are cleared
- **Feature register settings are reset to default values** (this is the key difference from Soft Reset)
  - B0h (Feature Register): OTP_PRT=0, OTP_EN=0, ECC_EN=1, QE=0
  - A0h (Protection Register): BP2=1, BP1=1, BP0=1, INV=0, CMP=0, BRWD=0
  - D0h (Driver Strength): DS_IO[1:0]=00 (100%)
  - 60h (Block Protection Lock): BPL=0
- The first page of the first block is automatically read to cache (Power-On Read)

**Note:** The 66h command must be immediately followed by the 99h command without any other commands in between.

---

## 15. Feature Operations

### 15.1 Get Features (0Fh) and Set Features (1Fh)

The Get Features and Set Features commands are used to read and write the device feature registers. These registers control various device behaviors including ECC, write protection, OTP access, and I/O configuration.

**Get Features (0Fh)**

The Get Features command is used to read the current value of a feature register.

Command sequence:
1. Drive CS# low
2. Send 0Fh command
3. Send 1-byte register address (A7-A0)
4. Read 1-byte register data (D7-D0)
5. Drive CS# high

**Set Features (1Fh)**

The Set Features command is used to write a new value to a feature register.

Command sequence:
1. Drive CS# low
2. Send 1Fh command
3. Send 1-byte register address (A7-A0)
4. Send 1-byte register data (D7-D0)
5. Drive CS# high

**Table 15-1. Feature Register Addresses**

| Register Address | Register Name | Type | Description |
| :--- | :--- | :--- | :--- |
| A0h | Protection Register | R/W | Block protection settings |
| B0h | Feature Register | R/W | OTP/ECC/Quad control |
| C0h | Status Register | RO | Device status |
| D0h | Driver Strength Register | R/W | I/O driver strength |
| F0h | Feature Status Register | RO | Extended ECC status |
| 60h | Block Protection Lock | R/W | Power lock down protection |

### 15.2 Status Register and Driver Register

**Table 15-2. Status Register (C0h)**

| Bit | Name | After POR / 66h+99h | After Reset (FFh) | Description |
| :--- | :--- | :--- | :--- | :--- |
| 7 | Reserved | 0 | 0 | Reserved (0) |
| 6 | ECCS1 | 0 | 0 | ECC Status bit 1 |
| 5 | ECCS0 | 0 | 0 | ECC Status bit 0 |
| 4 | Reserved | 0 | 0 | Reserved (0) |
| 3 | P_FAIL | 0 | 0 | Program Fail Flag |
| 2 | E_FAIL | 0 | 0 | Erase Fail Flag |
| 1 | WEL | 0 | 0 | Write Enable Latch |
| 0 | OIP | 0 | 0 | Operation In Progress |

**Status Register Bit Descriptions:**

| Bit | Description |
| :--- | :--- |
| **OIP (Operation In Progress)** | 0 = Device is ready; 1 = Device is busy (read/program/erase operation in progress) |
| **WEL (Write Enable Latch)** | 0 = Write disabled; 1 = Write enabled (set by 06h, cleared after program/erase or reset) |
| **E_FAIL (Erase Fail Flag)** | 0 = Erase operation passed; 1 = Erase operation failed |
| **P_FAIL (Program Fail Flag)** | 0 = Program operation passed; 1 = Program operation failed |
| **ECCS[1:0] (ECC Status)** | 00 = No error; 01 = 1-4 bit error corrected; 10 = Bit error not corrected; 11 = 8 bits corrected |

**Table 15-3. Feature Register (B0h)**

| Bit | Name | After POR / 66h+99h | After Reset (FFh) | Description |
| :--- | :--- | :--- | :--- | :--- |
| 7 | OTP_PRT | 0 (Before OTP Set) | No change | OTP Protect bit. Non-volatile. Once set to 1, OTP area is permanently locked read-only. |
| 6 | OTP_EN | 0 | No change | OTP Enable bit. Set to 1 to enable OTP area access. |
| 5 | Reserved | 0 | 0 | Reserved (0) |
| 4 | ECC_EN | 1 | No change | ECC Enable bit. 1 = ECC enabled (default); 0 = ECC disabled. |
| 3-1 | Reserved | 0 | 0 | Reserved (0) |
| 0 | QE | 0 | No change | Quad Enable bit. Set to 1 to enable Quad I/O operations. |

**Table 15-4. Driver Strength Register (D0h)**

| Bit | Name | After POR / 66h+99h | After Reset (FFh) | Description |
| :--- | :--- | :--- | :--- | :--- |
| 7-2 | Reserved | 0 | 0 | Reserved (0) |
| 1-0 | DS_IO[1:0] | 00 | No change | I/O Driver Strength. 00=100%(default), 01=75%, 10=50%, 11=25% |

**Table 15-5. Feature Status Register (F0h)**

| Bit | Name | After POR / 66h+99h | After Reset (FFh) | Description |
| :--- | :--- | :--- | :--- | :--- |
| 7-6 | Reserved | 0 | 0 | Reserved (0) |
| 5 | ECCSE1 | 0 | 0 | ECC Status Extension bit 1 |
| 4 | ECCSE0 | 0 | 0 | ECC Status Extension bit 0 |
| 3 | BPS | 1 | No change | Block Protection Status. 1 = Block is protected; 0 = Block is not protected |
| 2-0 | Reserved | 0 | 0 | Reserved (0) |

**Table 15-6. Protection Register (A0h)**

| Bit | Name | After POR / 66h+99h | After Reset (FFh) | Description |
| :--- | :--- | :--- | :--- | :--- |
| 7 | BRWD | 0 | No change | Block Register Write Disable. 1 = Protection register is read-only when WP#=LOW |
| 6-5 | Reserved | 0 | 0 | Reserved (0) |
| 4 | BP2 | 1 | No change | Block Protection bit 2 |
| 3 | BP1 | 1 | No change | Block Protection bit 1 |
| 2 | BP0 | 1 | No change | Block Protection bit 0 |
| 1 | INV | 0 | No change | Invert protection region. 1 = Invert the protection direction |
| 0 | CMP | 0 | No change | Complement protection. Used with BP[2:0] and INV for advanced protection |

**Table 15-7. Block Protection Lock Register (60h)**

| Bit | Name | After POR / 66h+99h | After Reset (FFh) | Description |
| :--- | :--- | :--- | :--- | :--- |
| 7-5 | Reserved | 0 | 0 | Reserved (0) |
| 4 | BPL | 0 | No change | Block Protection Lock. 1 = BP[2:0], INV, CMP, BRWD are locked until next power cycle |
| 3-1 | Reserved | 0 | 0 | Reserved (0) |
| 0 | Reserved | 0 | 0 | Must keep 0 |

### 15.3 OTP Region

The OTP (One-Time Programmable) region provides 40KB of user-programmable storage that can be written only once. The OTP region is organized as 10 pages, each with the same page size as the main array.

**OTP Access:**

To access the OTP region:
1. Set OTP_EN=1 in the Feature Register (B0h) using Set Features (1Fh) command
2. Perform read/program operations using the standard command set
3. Clear OTP_EN=0 to exit OTP access mode

**OTP Protection:**

The OTP region can be permanently locked by setting OTP_PRT=1 in the Feature Register (B0h). Once OTP_PRT is set:
- The OTP region becomes read-only
- No further programming is allowed
- This setting is non-volatile and cannot be reversed

**OTP State Table:**

| OTP_PRT | OTP_EN | State |
| :--- | :--- | :--- |
| 0 | 0 | Normal operation mode (OTP not accessible) |
| 0 | 1 | OTP accessible for read and program |
| 1 | 0 | OTP locked (read-only), normal operation |
| 1 | 1 | OTP locked (read-only), OTP mode |

**OTP Operation Sequence:**

1. **OTP Program:**
   - 1Fh + B0h: Set OTP_EN=1
   - 06h: Write Enable
   - 02h/32h: Program Load + data
   - 10h + OTP page address: Program Execute
   - 0Fh + C0h: Poll OIP until 0
   - 1Fh + B0h: Clear OTP_EN=0

2. **OTP Read:**
   - 1Fh + B0h: Set OTP_EN=1
   - 13h + OTP page address: Page Read to Cache
   - 0Fh + C0h: Poll OIP until 0
   - 03h/0Bh/3Bh/6Bh/BBh/EBh: Read From Cache
   - 1Fh + B0h: Clear OTP_EN=0

3. **OTP Lock (Permanent):**
   - 1Fh + B0h: Set OTP_EN=1 and OTP_PRT=1
   - 06h: Write Enable
   - 10h + any OTP page address: Program Execute (this locks OTP)
   - OTP_PRT will remain 1 permanently

**OTP Region Layout:**

| OTP Page | Page Address | Access |
| :--- | :--- | :--- |
| OTP Page 0 | 02h | User data (read/program) |
| OTP Page 1 | 03h | User data (read/program) |
| OTP Page 2 | 04h | User data (read/program) |
| OTP Page 3 | 05h | User data (read/program) |
| OTP Page 4 | 06h | User data (read/program) |
| OTP Page 5 | 07h | User data (read/program) |
| OTP Page 6 | 08h | User data (read/program) |
| OTP Page 7 | 09h | User data (read/program) |
| OTP Page 8 | 0Ah | User data (read/program) |
| OTP Page 9 | 0Bh | User data (read/program) |

**Notes:**
1. OTP space cannot be erased.
2. Once OTP_PRT is set to 1, it cannot be cleared. The OTP region is permanently read-only.
3. Do not use Soft Reset (FFh) command during OTP programming, as it may cause OTP damage.
4. Do not exit OTP mode (clear OTP_EN) when reading UID/Parameter Page/CASN Page.
5. When OTP_EN=1, the device accesses OTP area instead of main array.

### 15.4 Block Protection

The Block Protection feature allows the user to protect specific regions of the memory array from program and erase operations. The protection is controlled by the Protection Register (A0h) using the BP[2:0], INV, and CMP bits.

**Block Protection Configuration:**

The protection region is defined by the BP[2:0] bits in combination with the INV and CMP bits. The default state after power-up is BP2=BP1=BP0=1, which protects the entire memory array.

**Table 15-8. Block Protection Matrix for 4G Device**

| CMP | INV | BP2 | BP1 | BP0 | Protect Row Address Range | Protected Blocks | Description |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| X | X | 0 | 0 | 0 | None | 0 | None (all unlocked) |
| 0 | 0 | 0 | 0 | 1 | 1F800h~1FFFFh | 1 | Upper 1/64 |
| 0 | 0 | 0 | 1 | 0 | 1F000h~1FFFFh | 2 | Upper 1/32 |
| 0 | 0 | 0 | 1 | 1 | 1E000h~1FFFFh | 4 | Upper 1/16 |
| 0 | 0 | 1 | 0 | 0 | 1C000h~1FFFFh | 8 | Upper 1/8 |
| 0 | 0 | 1 | 0 | 1 | 18000h~1FFFFh | 16 | Upper 1/4 |
| 0 | 0 | 1 | 1 | 0 | 10000h~1FFFFh | 32 | Upper 1/2 |
| X | X | 1 | 1 | 1 | 00000h~1FFFFh | 2048 | All (default) |
| 0 | 1 | 0 | 0 | 1 | 00000h~007FFh | 1 | Lower 1/64 |
| 0 | 1 | 0 | 1 | 0 | 00000h~00FFFh | 2 | Lower 1/32 |
| 0 | 1 | 1 | 0 | 0 | 00000h~03FFFh | 8 | Lower 1/8 |
| 0 | 1 | 1 | 0 | 1 | 00000h~07FFFh | 16 | Lower 1/4 |
| 0 | 1 | 1 | 1 | 0 | 00000h~0FFFFh | 32 | Lower 1/2 |
| 1 | 0 | 1 | 1 | 0 | 00000h~0003Fh | 1 | Block 0 only |
| 1 | 1 | 0 | 0 | 1 | 00800h~1FFFFh | 2047 | All except lower 1/64 |
| 1 | 1 | 0 | 1 | 0 | 01000h~1FFFFh | 2046 | All except lower 1/32 |
| 1 | 1 | 1 | 0 | 0 | 04000h~1FFFFh | 2040 | All except lower 1/8 |
| 1 | 1 | 1 | 0 | 1 | 08000h~1FFFFh | 2016 | All except lower 1/4 |

**Note:** The Block Protection Status (BPS) bit in the Feature Status Register (F0h) indicates whether the currently selected block is protected (BPS=1) or not (BPS=0).

### 15.5 Power Lock Down Protection

The Power Lock Down Protection feature prevents software from modifying the Block Protection settings. When enabled, the block protection bits (BP[2:0], INV, CMP) and the Block Register Write Disable bit (BRWD) are locked and can only be unlocked by a power cycle.

**Power Lock Down Operation:**

1. Configure the desired block protection settings in the Protection Register (A0h)
2. Set BPL=1 in the Block Protection Lock Register (60h) using Set Features (1Fh) command
3. The protection settings are now locked and cannot be modified by software
4. To unlock, a power cycle is required (BPL is cleared to 0 on power-up)

**Block Protection Lock Register (60h):**

| Bit | Name | Description |
| :--- | :--- | :--- |
| 4 | BPL | Block Protection Lock. 1 = BP[2:0], INV, CMP, BRWD are locked until next power cycle |

**Note:** Once BPL is set to 1, it cannot be cleared by software. Only a power cycle will reset BPL to 0.

### 15.6 Internal ECC

The device incorporates an internal ECC (Error Correction Code) engine that provides 8-bit error correction per 528 bytes of data (512 bytes main + 16 bytes spare). The ECC is enabled by default (ECC_EN=1 in the Feature Register B0h).

**ECC Coverage:**

| Area | Size | ECC Coverage |
| :--- | :--- | :--- |
| Main Area | 4096 bytes (8 x 512 bytes) | Yes |
| Spare Area (first 128 bytes) | 128 bytes (8 x 16 bytes) | Yes |
| Spare Area (last 128 bytes) | 128 bytes | No (used for ECC parity) |

**ECC Operation:**

During Program operation:
1. The ECC engine calculates parity bits for each 528-byte segment
2. The parity data (16 bytes per segment) is stored in the last 128 bytes of the spare area

During Read operation:
1. The ECC engine reads the data and parity bits
2. The ECC engine detects and corrects errors (up to 8 bits per 528 bytes)
3. The corrected data is output to the host
4. The ECC status is reported in the Status Register (C0h[6:5]) and Feature Status Register (F0h[5:4])

**ECC Status Mapping:**

| ECCS1 | ECCS0 | ECCSE1 | ECCSE0 | Description |
| :--- | :--- | :--- | :--- | :--- |
| 0 | 0 | X | X | No bit errors detected |
| 0 | 1 | 0 | 0 | 1~4 bits error corrected |
| 0 | 1 | 0 | 1 | 5 bits error corrected |
| 0 | 1 | 1 | 0 | 6 bits error corrected |
| 0 | 1 | 1 | 1 | 7 bits error corrected |
| 1 | 1 | X | X | 8 bits error corrected |
| 1 | 0 | X | X | Bit errors not corrected (uncorrectable) |

**Notes:**
1. ECC is enabled by default (ECC_EN=1). It can be disabled by setting ECC_EN=0 in the Feature Register (B0h).
2. When ECC is enabled, the user can only program the first 128 bytes of the spare area. The last 128 bytes are used for ECC parity and cannot be programmed.
3. When ECC is disabled, the entire 256 bytes spare area is available for user programming.
4. The ECC status bits are valid only after a successful read operation.
5. ECCS/ECCSE are set to 00h after power-up or reset, and are updated after each read operation.
6. When ECC is disabled (ECC_EN=0), the ECC status bits are invalid.

---

## 16. Power On/Off Timing

**Figure 16-1. Power On/Off Timing**

```
Power On:
VCC
 |
 |     _______________
 |    /               |_________
 |___/
    |<-- tVSL -->|
                Device operational

Power Off:
VCC
_________
 |       |\
 |       | \
 |_______|  \_____
          |<->|
          VWI (Write Inhibit Voltage)

Power-On Reset:
VCC _____|          |_________________
         |< tVSL >|
CS# _____|__________|        |________
         |<-- tRST -->|

Power-Down:
VCC ________________|
                    |\
                    | \
                    |  \___
                       |<->|
                       tPWD (min 50us)
```

**Table 16-1. Power-On/Off Timing and Write Inhibit Threshold for 1.8V/3.3V**

| Symbol | Parameter | Min | Max | Unit |
| :--- | :--- | :--- | :--- | :--- |
| tVSL | VCC(min) to device operational | 3 | - | ms |
| tRST | Reset time after power-on | 5 | 10 | us |
| VWI | Write inhibit voltage | - | 1.5 (1.8V) / 2.5 (3.3V) | V |
| VPWD | Power-down voltage for initialization | - | 0.7 | V |
| tPWD | Power-down duration for initialization | 50 | - | us |

**Note:** Do not send any command before VCC reaches the minimum operating voltage. Do not power down during write/erase operations.

---

## 17. Absolute Maximum Ratings

**Table 17-1. Absolute Maximum Ratings**

| Parameter | Symbol | Value | Unit |
| :--- | :--- | :--- | :--- |
| Storage temperature | TSTG | -65 to +150 | degrees C |
| Ambient temperature with power applied | TA | -40 to +105 | degrees C |
| Input voltage with respect to ground (1) | VIN | -0.6 to VCC+0.4 | V |
| VCC supply voltage (1) | VCC | -0.6 to +4.6 | V |
| Output short circuit current (2) | IOS | 5 | mA |
| Electrostatic discharge voltage (Human Body Model) (3) | VESD | +/-4000 | V |

**Notes:**
1. The minimum DC input voltage is -0.6V. During voltage transitions, inputs may undershoot to -2.0V for periods of up to 20ns. The maximum DC voltage on output and I/O pins is VCC + 0.4V. During voltage transitions, outputs may overshoot to VCC + 2.0V for periods up to 20ns.
2. No more than one output may be shorted to ground at a time. Duration of the short circuit should not be greater than one second.
3. The minimum DC voltage on input or I/O pins is -0.6V. During voltage transitions, inputs may undershoot to -2.0V for periods of up to 20ns. The maximum DC voltage on input and I/O pins is VCC + 0.4V and may overshoot to VCC + 2.0V for periods up to 20ns.

---

## 18. Capacitance Measurement Conditions

**Table 18-1. Capacitance Measurement Conditions**

| Parameter | Symbol | Test Condition | Min | Max | Unit |
| :--- | :--- | :--- | :--- | :--- | :--- |
| Input capacitance | CIN | VIN = 0V, f = 1MHz | - | 6 | pF |
| Output capacitance | COUT | VOUT = 0V, f = 1MHz | - | 8 | pF |
| Input/Output capacitance | CI/O | VI/O = 0V, f = 1MHz | - | 8 | pF |

**Note:** These parameters are characterized and not 100% tested.

---

## 19. DC Characteristic

**Table 19-1. DC Characteristics for 3.3V (2.7V~3.6V)**

| Symbol | Parameter | Test Condition | Min | Typ | Max | Unit |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| VCC | Supply voltage | | 2.7 | 3.3 | 3.6 | V |
| ILI | Input leakage current | VIN = VCC or VSS | - | - | +/-2 | uA |
| ILO | Output leakage current | VOUT = VCC or VSS | - | - | +/-2 | uA |
| ICC1 | Standby current (CMOS) | CS# = VCC, f = 0 | - | 10 | 35 | uA |
| ICC2 | Operating current (Read) | f = 133MHz | - | 15 | 30 | mA |
| ICC3 | Operating current (Program) | | - | 15 | 30 | mA |
| ICC4 | Operating current (Erase) | | - | 15 | 30 | mA |
| ICC5 | Operating current (DTR Read) | f = 104MHz, Quad | - | 20 | 35 | mA |
| VIL | Input low voltage | | -0.5 | - | 0.2VCC | V |
| VIH | Input high voltage | | 0.8VCC | - | VCC+0.4 | V |
| VOL | Output low voltage | IOL = 1.6mA | - | - | 0.4 | V |
| VOH | Output high voltage | IOH = -100uA | VCC-0.2 | - | - | V |

**Table 19-2. DC Characteristics for 1.8V (1.7V~2.0V)**

| Symbol | Parameter | Test Condition | Min | Typ | Max | Unit |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| VCC | Supply voltage | | 1.7 | 1.8 | 2.0 | V |
| ILI | Input leakage current | VIN = VCC or VSS | - | - | +/-2 | uA |
| ILO | Output leakage current | VOUT = VCC or VSS | - | - | +/-2 | uA |
| ICC1 | Standby current (CMOS) | CS# = VCC, f = 0 | - | 5 | 30 | uA |
| ICC1-DPD | Deep power-down current | CS# = VCC, f = 0 | - | 1 | - | uA |
| ICC2 | Operating current (Read) | f = 104MHz | - | 10 | 20 | mA |
| ICC3 | Operating current (Program) | | - | 10 | 20 | mA |
| ICC4 | Operating current (Erase) | | - | 10 | 20 | mA |
| ICC5 | Operating current (DTR Read) | f = 80MHz, Quad | - | 15 | 30 | mA |
| VIL | Input low voltage | | -0.5 | - | 0.2VCC | V |
| VIH | Input high voltage | | 0.8VCC | - | VCC+0.4 | V |
| VOL | Output low voltage | IOL = 100uA | - | - | 0.2VCC | V |
| VOH | Output high voltage | IOH = -100uA | 0.8VCC | - | - | V |

**Notes:**
1. Typical values are measured at TA = 25 degrees C, VCC = 3.3V/1.8V.
2. Maximum values are measured at TA = 85 degrees C.
3. The maximum standby current is 50uA at 105 degrees C.

---

## 20. AC Characteristics

**Table 20-1. AC Characteristics for 3.3V (2.7V~3.6V)**

| Symbol | Parameter | Min | Typ | Max | Unit |
| :--- | :--- | :--- | :--- | :--- | :--- |
| FC1 | Serial clock frequency for Standard/Dual/Quad SPI | - | - | 133 | MHz |
| FC_DTR | Serial clock frequency for DTR Quad SPI | - | - | 104 | MHz |
| tCH | Serial clock high time | 3.375 | - | - | ns |
| tCL | Serial clock low time | 3.375 | - | - | ns |
| tSLCH | CS# active setup time | 5 | - | - | ns |
| tCHSL | CS# not active hold time | 5 | - | - | ns |
| tDVCH | Data in setup time | 2 | - | - | ns |
| tCHDX | Data in hold time | 2 | - | - | ns |
| tSHSL | CS# deselect time | 20 | - | - | ns |
| tCLQV | Clock low to output valid | - | - | 6.5 | ns |
| tCLQX | Output hold time | 1.5 | - | - | ns |
| tSHQZ | Output disable time | - | - | 20 | ns |

**Table 20-2. AC Characteristics for 1.8V (1.7V~2.0V)**

| Symbol | Parameter | Min | Typ | Max | Unit |
| :--- | :--- | :--- | :--- | :--- | :--- |
| FC1 | Serial clock frequency for Standard/Dual/Quad SPI | - | - | 104 | MHz |
| FC_DTR | Serial clock frequency for DTR Quad SPI | - | - | 80 | MHz |
| tCH | Serial clock high time | 4 | - | - | ns |
| tCL | Serial clock low time | 4 | - | - | ns |
| tSLCH | CS# active setup time | 5 | - | - | ns |
| tCHSL | CS# not active hold time | 5 | - | - | ns |
| tDVCH | Data in setup time | 2 | - | - | ns |
| tCHDX | Data in hold time | 2 | - | - | ns |
| tSHSL | CS# deselect time | 20 | - | - | ns |
| tCLQV | Clock low to output valid | - | - | 9 | ns |
| tCLQX | Output hold time | 2 | - | - | ns |
| tSHQZ | Output disable time | - | - | 20 | ns |

**Table 20-3. Deep Power-Down AC Characteristics for 1.8V**

| Symbol | Parameter | Min | Typ | Max | Unit |
| :--- | :--- | :--- | :--- |
| tDP | CS# high to Deep Power-Down | - | - | 3 | us |
| tRES1 | CS# high to Standby mode without Electronic Signature read | - | - | 50 | us |

**Notes:**
1. All AC parameters are measured with CL = 30pF.
2. Timing measurement reference level: Input = 0.5VCC, Output = 0.5VCC.

---

## 21. Performance and Timing

**Table 21-1. Performance and Timing**

| Symbol | Parameter | Typ | Max | Unit |
| :--- | :--- | :--- | :--- | :--- |
| tRD | Page read time (no ECC) | - | 25 | us |
| tRD_ECC | Page read time (with ECC) | 70 | 180 | us |
| tPROG | Page program time | 300 | 600 | us |
| tPROG_ECC | Page program time (with ECC) | 340 | 600 | us |
| tBERS | Block erase time | 3 | 10 | ms |
| tRST | Reset time (standby/read) | 5 | 10 | us |
| tRST_PGM | Reset time (program) | - | 10 | us |
| tRST_ERS | Reset time (erase) | - | 500 | us |

---

## 22. Ordering Information

**Table 22-1. Ordering Information**

```
GD 5 F 4 G M 7 [V] E [Pkg] [Temp] [Green]
  | | | | | |  |   |   |      |      |
  | | | | | |  |   |   |      |      +-- G = Pb Free & Halogen Free
  | | | | | |  |   |   |      |
  | | | | | |  |   |   |      +--------- I = -40~85 degrees C, J = -40~105 degrees C
  | | | | | |  |   |   |
  | | | | | |  |   |   +---------------- Y = WSON8 (8x6mm), W = WSON8 (6x5mm), B = TFBGA24 (5x5)
  | | | | | |  |   |
  | | | | | |  |   +-------------------- E = E Version
  | | | | | |  |
  | | | | | |  +------------------------ U = 3.3V (2.7~3.6V), R = 1.8V (1.7~2.0V)
  | | | | | |
  | | | | | +--------------------------- M7 Series
  | | | | |
  | | | | +----------------------------- 4G = 4Gb
  | | | |
  | | | +------------------------------- G = SLC NAND
  | | |
  | | +--------------------------------- F = SPI NAND Flash
  | |
  | +----------------------------------- 5 = SPI Interface
  |
  +------------------------------------- GigaDevice Prefix
```

**Table 22-2. Valid Order Part Numbers**

| Part Number | Voltage | Package | Temperature |
| :--- | :--- | :--- | :--- |
| GD5F4GM7UEYIG | 2.7V~3.6V | WSON8 (8x6mm) | -40 degrees C ~ 85 degrees C |
| GD5F4GM7UEWIG | 2.7V~3.6V | WSON8 (6x5mm) | -40 degrees C ~ 85 degrees C |
| GD5F4GM7UEBIG | 2.7V~3.6V | TFBGA24 (5x5) | -40 degrees C ~ 85 degrees C |
| GD5F4GM7REYIG | 1.7V~2.0V | WSON8 (8x6mm) | -40 degrees C ~ 85 degrees C |
| GD5F4GM7REWIG | 1.7V~2.0V | WSON8 (6x5mm) | -40 degrees C ~ 85 degrees C |
| GD5F4GM7REBIG | 1.7V~2.0V | TFBGA24 (5x5) | -40 degrees C ~ 85 degrees C |
| GD5F4GM7UEYJG | 2.7V~3.6V | WSON8 (8x6mm) | -40 degrees C ~ 105 degrees C |
| GD5F4GM7REYJG | 1.7V~2.0V | WSON8 (8x6mm) | -40 degrees C ~ 105 degrees C |

---

## 23. Package Information

### 23.1 WSON8 (8x6mm)

**Figure 23-1. WSON8 (8x6mm) Package Outline**

```
Top View:
  +-------------------+
  |  8           1    |  Pin 1: CS#
  |                   |  Pin 2: SO/SIO1
  |                   |  Pin 3: WP#/SIO2
  |                   |  Pin 4: VSS
  |                   |  Pin 5: SI/SIO0
  |                   |  Pin 6: SCLK
  |  5           4    |  Pin 7: HOLD#/SIO3
  +-------------------+  Pin 8: VCC

Dimensions (mm):
  A: 0.70 - 0.80 (total thickness)
  A1: 0.00 - 0.05 (stand-off)
  b: 0.35 - 0.45 (lead width)
  D: 8.00 BSC (body length)
  D2: 3.30 - 3.80 (exposed pad length)
  E: 6.00 BSC (body width)
  E2: 4.20 - 4.70 (exposed pad width)
  e: 1.27 BSC (pitch)
  L: 0.50 - 0.60 (lead length)
```

### 23.2 WSON8 (6x5mm)

**Figure 23-2. WSON8 (6x5mm) Package Outline**

```
Top View:
  +----------------+
  |  8        1    |  Pin 1: CS#
  |                |  Pin 2: SO/SIO1
  |                |  Pin 3: WP#/SIO2
  |                |  Pin 4: VSS
  |                |  Pin 5: SI/SIO0
  |                |  Pin 6: SCLK
  |  5        4    |  Pin 7: HOLD#/SIO3
  +----------------+  Pin 8: VCC

Dimensions (mm):
  A: 0.70 - 0.80
  A1: 0.00 - 0.05
  b: 0.30 - 0.40
  D: 6.00 BSC
  D2: 2.80 - 3.30
  E: 5.00 BSC
  E2: 3.30 - 3.80
  e: 1.27 BSC
  L: 0.40 - 0.50
```

### 23.3 TFBGA24 (5x5 Ball Array)

**Figure 23-3. TFBGA24 (5x5 Ball Array) Package Outline**

```
Top View:
  A   B   C   D   E
  o   o   o   o   o   1
  o   o   o   o   o   2
  o   o   o   o   o   3
  o   o   o   o   o   4
  o   o   o   o   o   5

Ball Assignment:
  A1-A5: NC
  B1: NC, B2: SCLK, B3: VSS, B4: VCC, B5: NC
  C1: NC, C2: CS#, C3: DQS, C4: WP#/SIO2, C5: NC
  D1: NC, D2: SI/SIO0, D3: HOLD#/SIO3, D4: NC, D5: NC
  E1: NC, E2: NC, E3: SO/SIO1, E4: NC, E5: NC

Dimensions (mm):
  A: 0.80 - 1.00
  A1: 0.15 - 0.25
  A2: 0.55 - 0.65
  b: 0.40 - 0.50 (ball diameter)
  D: 5.00 BSC
  E: 5.00 BSC
  e: 1.00 BSC (ball pitch)
```

---

## 24. Revision History

**Table 24-1. Revision History**

| Revision | Date | Description |
| :--- | :--- | :--- |
| Rev 1.0 | September 2025 | Initial release |

---

*Document: DS-GD5F4GM7-Rev1.0*
*Date: September 2025*
*GigaDevice Semiconductor Inc.*
