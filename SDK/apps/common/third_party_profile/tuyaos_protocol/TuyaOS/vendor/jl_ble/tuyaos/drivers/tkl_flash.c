/**
 * @file tkl_flash.c
 * @brief This is tkl_flash file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "tkl_flash.h"
#include "board.h"
#include "system/includes.h"
#include "user_cfg_id.h"
#include "tuya_ble_main.h"
#include "tuya_ble_protocol_callback.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/


/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/
typedef enum _FLASH_ERASER {
    CHIP_ERASER,
    BLOCK_ERASER,//4k
    SECTOR_ERASER,//64k
} FLASH_ERASER;

//
extern bool sfc_erase(FLASH_ERASER cmd, u32 addr);
extern u32 sdfile_cpu_addr2flash_addr(u32 offset);

#define USER_FILE_NAME       SDFILE_APP_ROOT_PATH"USERIF"
#define NV_MODE_FILE         0 //固定放在一个指定的区域，一般情况下不会被意外擦除，flash比较大的方案建议用这个
#define NV_MODE_VM           1 //用VM存，被意外擦除的概率比较高，比如升级的时候，flash空间不够的时候用这个
#define TUYA_BLE_NV_MODE     NV_MODE_VM
//使用文件的的方式保存数据，需要在ini文件添加下面的配置
/*
USERIF_ADR=AUTO;
USERIF_LEN=0x4000;
USERIF_OPT=1;
*/

FILE *code_fp = NULL;

struct vfs_attr code_attr = {0};
struct vfs_attr attr;

typedef struct __tuya_addr_to_vfs {
    u32 tuya_start_addr;//0
    u32 vfs_satrt_addr;//1
} tuya_addr_to_vfs;

static tuya_addr_to_vfs addr_sw;

uint8_t app_tuya_get_vm_id(u32 addr)
{
    uint8_t index;
    switch (addr) {
    case 0:
        index = CFG_USER_TUYA_INFO_AUTH;
        break;
    case 1:
        index = CFG_USER_TUYA_INFO_AUTH_BK;
        break;
    case 2:
        index = CFG_USER_TUYA_INFO_SYS;
        break;
    case 3:
        index = CFG_USER_TUYA_INFO_SYS_BK;
        break;
    default:
        return TUYA_BLE_ERR_INVALID_ADDR;
    }
    return index;
}

static uint8_t (*tuya_get_vm_id)(u32 addr) = app_tuya_get_vm_id;

void tuya_get_vm_id_register(uint8_t (*handler)(u32 addr))
{
    tuya_get_vm_id = handler;
}

static uint8_t tuya_get_vm_id_func(u32 addr)
{
    if (tuya_get_vm_id) {
        return tuya_get_vm_id(addr);
    }
    printf("tuya_get_vm_id no regitster");
    return 0;
}

OPERATE_RET tkl_flash_init(void)
{
#if (TUYA_BLE_NV_MODE == NV_MODE_FILE)
    if (code_fp) {
        return OPRT_OK;
    }
    code_fp = fopen(USER_FILE_NAME, "r+w");
    if (code_fp == NULL) {
        return OPRT_FILE_NOT_FIND;
    }

    fget_attrs(code_fp, &attr);
    if (attr.fsize < 2048) {
    }
    addr_sw.tuya_start_addr = 0;
    addr_sw.vfs_satrt_addr = attr.sclust;
#endif

    return OPRT_OK;
}

OPERATE_RET tkl_flash_read(UINT32_T addr, UCHAR_T *dst, UINT32_T size)
{
    tkl_flash_init();
#if (TUYA_BLE_NV_MODE == NV_MODE_FILE)
    FILE *read_fp = code_fp;
    if (code_fp == NULL) {
        return OPRT_FILE_NOT_FIND;
    }
    fseek(read_fp, addr, SEEK_SET);
    fread(read_fp, p_data, size);//文件模式读取自定义数据区
#else
    u8 index;
    switch (addr) {
    case TUYA_BLE_AUTH_FLASH_ADDR:
        index = tuya_get_vm_id_func(0);
        break;
    case TUYA_BLE_AUTH_FLASH_BACKUP_ADDR:
        index = tuya_get_vm_id_func(1);
        break;
    case TUYA_BLE_SYS_FLASH_ADDR:
        index = tuya_get_vm_id_func(2);
        break;
    case TUYA_BLE_SYS_FLASH_BACKUP_ADDR:
        index = tuya_get_vm_id_func(3);
        break;

    default:
        return OPRT_INVALID_PARM;
    }
    int ret = syscfg_read(index, dst, size);
    printf("tkl_flash_read ret:%d size:%d\n", ret, size);
    if (ret != size) {
        memset(dst, 0x00, size);
    }
#endif
    return OPRT_OK;
}

OPERATE_RET tkl_flash_write(UINT32_T addr, CONST UCHAR_T *src, UINT32_T size)
{
    printf("tkl_flash_write size:%d\n", size);
    put_buf(src, size);
    tkl_flash_init();
#if (TUYA_BLE_NV_MODE == NV_MODE_FILE)
    FILE *write_fp = code_fp;
    if (code_fp == NULL) {
        return OPRT_FILE_NOT_FIND;
    }
    int ret = 0;
    ret = fseek(write_fp, addr, SEEK_SET);
    int r = fwrite(write_fp, p_data, size);//更新数据到自定义区，写数据前需要擦除，确保处于FF状态才能成功写入
    if (r != size) {
    }
#else
    u8 index;
    switch (addr) {
    case TUYA_BLE_AUTH_FLASH_ADDR:
        index = tuya_get_vm_id_func(0);
        break;
    case TUYA_BLE_AUTH_FLASH_BACKUP_ADDR:
        index = tuya_get_vm_id_func(1);
        break;
    case TUYA_BLE_SYS_FLASH_ADDR:
        index = tuya_get_vm_id_func(2);
        break;
    case TUYA_BLE_SYS_FLASH_BACKUP_ADDR:
        index = tuya_get_vm_id_func(3);
        break;

    default:
        return OPRT_INVALID_PARM;
    }
    if (syscfg_write(index, src, size) != size) {
        printf("tuya vm_write error");
        return OPRT_KVS_WR_FAIL;
    }
#endif
    //发消息，用来给TWS同步app配对数据
    if (addr == TUYA_BLE_AUTH_FLASH_ADDR) {
        tuya_ble_custom_evt_send(APP_EVT_AUTH_FLASH_SAVE);
    } else if (addr == TUYA_BLE_SYS_FLASH_ADDR) {
        tuya_ble_custom_evt_send(APP_EVT_SYS_FLASH_SAVE);
    }
    return OPRT_OK;
}

OPERATE_RET tkl_flash_erase(UINT32_T addr, UINT32_T size)
{
    tkl_flash_init();
#if (TUYA_BLE_NV_MODE == NV_MODE_FILE)
    if (code_fp == NULL) {
        return OPRT_FILE_NOT_FIND;
    }
    u32 flash_addr = sdfile_cpu_addr2flash_addr(addr_sw.vfs_satrt_addr + addr);
    sfc_erase(SECTOR_ERASER, flash_addr);// 4k
#endif
    return OPRT_OK;
}

OPERATE_RET tkl_flash_lock(UINT32_T addr, UINT32_T size)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_flash_unlock(UINT32_T addr, UINT32_T size)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_flash_get_one_type_info(TUYA_FLASH_TYPE_E type, TUYA_FLASH_BASE_INFO_T *info)
{
    return OPRT_NOT_SUPPORTED;
}


