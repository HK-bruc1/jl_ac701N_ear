#ifndef _NANDFLASH_TEST_H
#define _NANDFLASH_TEST_H

#include "app_config.h"

#if TCFG_NAND_TEST_ENABLE

// 子宏默认值：如果板级未定义则默认全开
#ifndef NAND_TEST_RAW
#define NAND_TEST_RAW    1
#endif
#ifndef NAND_TEST_FTL
#define NAND_TEST_FTL    1
#endif
#ifndef NAND_TEST_FS
#define NAND_TEST_FS     1
#endif

void nand_test_run_raw(void);  // 裸驱动测试入口（FTL init 之前调用）
void nand_test_run_all(void);  // FTL + FS 测试入口（mount 之后调用）

#endif // TCFG_NAND_TEST_ENABLE

#endif // _NANDFLASH_TEST_H
