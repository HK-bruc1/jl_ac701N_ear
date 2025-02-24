#ifndef _APP_LE_AURACAST_H
#define _APP_LE_AURACAST_H

#include "typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BROADCAST_STATUS_DEFAULT = 0,
    BROADCAST_STATUS_SCAN_START,
    BROADCAST_STATUS_SCAN_STOP,
    BROADCAST_STATUS_START,
    BROADCAST_STATUS_STOP,
} BROADCAST_STATUS;

/**
 * @brief 获取auracast状态
 */
BROADCAST_STATUS le_auracast_status_get();

/**
 * @brief 关闭auracast功能（音频、扫描）
 */
void le_auracast_stop(void);

#ifdef __cplusplus
};
#endif

#endif

