#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".rcsp_interface.data.bss")
#pragma data_seg(".rcsp_interface.data")
#pragma const_seg(".rcsp_interface.text.const")
#pragma code_seg(".rcsp_interface.text")
#endif
#include "system/includes.h"
#include "rcsp_config.h"
#include "string.h"
#include "ble_user.h"
#include "JL_rcsp_protocol.h"
#include "JL_rcsp_api.h"
#include "app_ble_spp_api.h"
#include "btstack_rcsp_user.h"
#include "asm/cpu.h"

#if (THIRD_PARTY_PROTOCOLS_SEL&RCSP_MODE_EN)

#define BD_ADDR_TYPE_EDR_ATT    0xfb

static char *gap_device_name = "jl_rcsp_ble_test";
static u8 gap_device_name_len = 0;
static int g_rcsp_ble_conn_num = 0;
static int g_rcsp_adt_conn_num = 0;
static int g_rcsp_spp_conn_num = 0;

#define ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE 0x0003
#define ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE 0x0006
#define ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE 0x0008
#define ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE 0x0009
#define ATT_CHARACTERISTIC_2a05_01_VALUE_HANDLE 0x0056
#define ATT_CHARACTERISTIC_2a05_01_CLIENT_CONFIGURATION_HANDLE 0x0057

#if 0
#define rcsp_lib_puts(x)				puts(x)         //app_puts(RCSP_INTERFACE_LOG, x)
#define rcsp_lib_putchar(x)				putchar(x)      //app_putchar(RCSP_INTERFACE_LOG, x)
#define rcsp_lib_printf					printf          //app_log(RCSP_INTERFACE_LOG)
#define rcsp_lib_printf_buf(x, y)		put_buf(x, y)   //app_log_dump(RCSP_INTERFACE_LOG, x, y)
#else
#define rcsp_lib_puts(...)
#define rcsp_lib_putchar(...)
#define rcsp_lib_printf(...)
#define rcsp_lib_printf_buf(...)
#endif

extern const u8 adt_profile_support;
extern u8 rcsp_dual_support; // 是否支持一拖二
extern u8 rcsp_auth_support; // 是否开启认证

__attribute__((weak)) u8 rcsp_adt_support = 0; //edr att是否接入rcsp
void *bt_rcsp_spp_hdl = NULL;
void *rcsp_server_ble_hdl = NULL;
void *rcsp_server_edr_att_hdl = NULL;
void *bt_rcsp_spp_hdl1 = NULL;
void *rcsp_server_ble_hdl1 = NULL;
void *rcsp_server_edr_att_hdl1 = NULL;
#define EDR_ATT_HDL_UUID \
	(((u8)('E' + 'D' + 'R') << (1 * 8)) | \
	 ((u8)('A' + 'T' + 'T') << (0 * 8)))

/*****************************************
                 RCSP PROTOCOL
 *****************************************/
const u8 rcsp_link_key_data[16] = {0x06, 0x77, 0x5f, 0x87, 0x91, 0x8d, 0xd4, 0x23, 0x00, 0x5d, 0xf1, 0xd8, 0xcf, 0x0c, 0x14, 0x2b};

/**
 * @brief 获取当前ble连接设备的mac地址
 */
u8 *rcsp_get_ble_hdl_remote_mac_addr(u16 ble_con_handle)
{
    u16 con_hdl = app_ble_get_hdl_con_handle(rcsp_server_ble_hdl);
    if (ble_con_handle == con_hdl) {
        return app_ble_remote_mac_addr_get(rcsp_server_ble_hdl);
    }
    if (rcsp_dual_support) {
        u16 con_hdl1 = app_ble_get_hdl_con_handle(rcsp_server_ble_hdl1);
        if (ble_con_handle == con_hdl1) {
            return app_ble_remote_mac_addr_get(rcsp_server_ble_hdl1);
        }
    }

    return NULL;
}

// 获取当前已连接ble数目
u8 bt_rcsp_ble_conn_num(void)
{
    /* rcsp_lib_printf("%s, %s, %d, g_rcsp_ble_conn_num:%d\n", __FILE__, __FUNCTION__, __LINE__, g_rcsp_ble_conn_num); */
    return g_rcsp_ble_conn_num;
}
// 获取当前已连接edr att数目
u8 bt_rcsp_edr_att_conn_num(void)
{
    /* rcsp_lib_printf("%s, %s, %d, g_rcsp_adt_conn_num:%d\n", __FILE__, __FUNCTION__, __LINE__, g_rcsp_adt_conn_num); */
    return g_rcsp_adt_conn_num;
}

// 获取当前已连接spp数目
u8 bt_rcsp_spp_conn_num(void)
{
    /* rcsp_lib_printf("%s, %s, %d, g_rcsp_spp_conn_num:%d\n", __FILE__, __FUNCTION__, __LINE__, g_rcsp_spp_conn_num); */
    return g_rcsp_spp_conn_num;
}

// 获取当前已连接设备数目
u8 bt_rcsp_device_conn_num(void)
{
    u8 cnt = 0;
    cnt += bt_rcsp_ble_conn_num();
    cnt += bt_rcsp_spp_conn_num();
    /* rcsp_lib_printf("%s, %s, %d, cnt:%d\n", __FILE__, __FUNCTION__, __LINE__, cnt); */
    return cnt;
}

/**
 * @brief tws同步bt_handle信息
 */
_WEAK_ void rcsp_interface_bt_handle_tws_sync(void)
{
}

/**
 * @brief 获取tws同步的bt handle的总buf大小
 */
u16 rcsp_interface_tws_sync_buf_size()
{
    u16 ble_hdl_size = (u16)app_ble_hdl_core_data_size();
    u16 spp_hdl_size = (u16)app_spp_hdl_core_data_size();
    u16 buf_size = ble_hdl_size + spp_hdl_size;
    if (rcsp_dual_support) {
        buf_size *= 2;
    }
    return buf_size;
}

/**
 * @brief 用于tws同步，从机接收到主机的tws数据后同步到本地
 */
void rcsp_interface_set_bt_hdl_with_tws_data(u8 *recieve_buf, u16 recieve_len)
{
    u16 buf_size = rcsp_interface_tws_sync_buf_size();
    if (recieve_len != buf_size) {
        rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
    u16 ble_hdl_size = (u16)app_ble_hdl_core_data_size();
    u16 spp_hdl_size = (u16)app_spp_hdl_core_data_size();
    u16 buf_index = 0;
    app_ble_hdl_core_data_set(rcsp_server_ble_hdl, recieve_buf);
    buf_index += ble_hdl_size;
    app_spp_hdl_core_data_set(bt_rcsp_spp_hdl, recieve_buf + buf_index);
    buf_index += spp_hdl_size;
    g_rcsp_ble_conn_num = 0;
    g_rcsp_adt_conn_num = 0;
    g_rcsp_spp_conn_num = 0;
    u16 ble_con_handle = app_ble_get_hdl_con_handle(rcsp_server_ble_hdl);
    if (ble_con_handle) {
        g_rcsp_ble_conn_num++;
    }
    u8 *spp_remote_addr = app_spp_get_hdl_remote_addr(bt_rcsp_spp_hdl);
    if (spp_remote_addr) {
        g_rcsp_spp_conn_num++;
    }
    if (rcsp_dual_support) {
        app_ble_hdl_core_data_set(rcsp_server_ble_hdl1, recieve_buf + buf_index);
        buf_index += ble_hdl_size;
        app_spp_hdl_core_data_set(bt_rcsp_spp_hdl1, recieve_buf + buf_index);
        u16 ble_con_handle1 = app_ble_get_hdl_con_handle(rcsp_server_ble_hdl1);
        if (ble_con_handle1) {
            g_rcsp_ble_conn_num++;
        }
        u8 *spp_remote_addr1 = app_spp_get_hdl_remote_addr(bt_rcsp_spp_hdl1);
        if (spp_remote_addr1) {
            g_rcsp_spp_conn_num++;
        }
    }
    if (adt_profile_support && rcsp_adt_support) {
        u16 adt_con_handle = app_ble_get_hdl_con_handle(rcsp_server_edr_att_hdl);
        if (adt_con_handle) {
            g_rcsp_adt_conn_num++;
        }
        if (rcsp_dual_support) {
            u16 adt_con_handle1 = app_ble_get_hdl_con_handle(rcsp_server_edr_att_hdl1);
            if (adt_con_handle1) {
                g_rcsp_adt_conn_num++;
            }
        }
    }

    // log打印验证数据是否正确
    /* struct _app_ble_core ble_hdl = {0}; */
    /* app_ble_hdl_core_data_get(rcsp_server_ble_hdl, (u8 *)&ble_hdl); */
    /* rcsp_lib_printf("%s, %s, %d, con_handle:%d\n", __FILE__, __FUNCTION__, __LINE__, ble_hdl.con_handle); */
    /* put_buf((u8 *)&ble_hdl, ble_hdl_size); */
    /* struct _app_spp_core spp_hdl = {0}; */
    /* app_spp_hdl_core_data_get(bt_rcsp_spp_hdl, (u8 *)&spp_hdl); */
    /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
    /* rcsp_lib_printf_buf((u8 *)&spp_hdl, spp_hdl_size); */
    /* if (rcsp_dual_support) { */
    /* struct _app_ble_core ble_hdl1 = {0}; */
    /* app_ble_hdl_core_data_get(rcsp_server_ble_hdl1, (u8 *)&ble_hdl1); */
    /* rcsp_lib_printf("%s, %s, %d, con_handle:%d\n", __FILE__, __FUNCTION__, __LINE__, ble_hdl1.con_handle); */
    /* rcsp_lib_printf_buf((u8 *)&ble_hdl1, ble_hdl_size); */
    /* struct _app_spp_core spp_hdl1 = {0}; */
    /* app_spp_hdl_core_data_get(bt_rcsp_spp_hdl1, (u8 *)&spp_hdl1); */
    /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
    /* rcsp_lib_printf_buf((u8 *)&spp_hdl1, spp_hdl_size); */
    /* } */
}

static u8 *tws_sync_buf = NULL;
void rcsp_interface_tws_sync_buf_content_free()
{
    if (tws_sync_buf) {
        free(tws_sync_buf);
        tws_sync_buf = NULL;
    }
}

/**
 * @brief rcsp主机同步bt handle数据给从机
 *
 * @param send_buf 外部malloc的一个指针，size根据rcsp_interface_tws_sync_buf_size获取
 */
void rcsp_interface_tws_sync_buf_content(u8 *send_buf)
{
    rcsp_interface_tws_sync_buf_content_free();
    u16 buf_size = rcsp_interface_tws_sync_buf_size();
    if (buf_size < 1) {
        return;
    }
    tws_sync_buf = malloc(buf_size);
    ASSERT(tws_sync_buf, "tws_sync_buf malloc fail!");
    u16 buf_index = 0;
    u16 ble_hdl_size = (u16)app_ble_hdl_core_data_size();
    u16 spp_hdl_size = (u16)app_spp_hdl_core_data_size();
    u8 *ble_hdl = zalloc(ble_hdl_size);
    ASSERT(ble_hdl, "rcsp tws sync, ble_buf malloc fail!");
    app_ble_hdl_core_data_get(rcsp_server_ble_hdl, ble_hdl);
    /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
    /* put_buf((u8 *)&ble_hdl, ble_hdl_size); */
    memcpy(tws_sync_buf, ble_hdl, ble_hdl_size);
    buf_index += ble_hdl_size;
    u8 *spp_hdl = zalloc(spp_hdl_size);
    ASSERT(spp_hdl, "rcsp tws sync, spp_buf malloc fail!");
    app_spp_hdl_core_data_get(bt_rcsp_spp_hdl, spp_hdl);
    /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
    /* rcsp_lib_printf_buf((u8 *)&spp_hdl, spp_hdl_size); */
    memcpy(tws_sync_buf + buf_index, spp_hdl, spp_hdl_size);
    buf_index += spp_hdl_size;
    if (rcsp_dual_support) {
        u8 *ble_hdl1 = ble_hdl;
        app_ble_hdl_core_data_get(rcsp_server_ble_hdl1, ble_hdl1);
        /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
        /* rcsp_lib_printf_buf((u8 *)&ble_hdl1, ble_hdl_size); */
        memcpy(tws_sync_buf + buf_index, ble_hdl1, ble_hdl_size);
        buf_index += ble_hdl_size;
        u8 *spp_hdl1 = spp_hdl;
        app_spp_hdl_core_data_get(bt_rcsp_spp_hdl1, spp_hdl1);
        /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
        /* rcsp_lib_printf_buf((u8 *)&spp_hdl1, spp_hdl_size); */
        memcpy(tws_sync_buf + buf_index, spp_hdl1, spp_hdl_size);
    }
    free(ble_hdl);
    free(spp_hdl);
    /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
    /* rcsp_lib_printf_buf(tws_sync_buf, buf_size); */
    memcpy(send_buf, tws_sync_buf, buf_size);
    rcsp_interface_tws_sync_buf_content_free();
}

/**
 * @brief 在连接或断开的时候，设置rcsp蓝牙连接信息
 *
 * @param con_handle ble连接或断开时输入
 * @param remote_addr spp连接或断开时输入
 * @param isconn true:连接；false:断开
 */
void bt_rcsp_set_conn_info(u16 con_handle, void *remote_addr, bool isconn)
{
    rcsp_lib_printf("%s, %s, %d, isconn:%d\n", __FILE__, __FUNCTION__, __LINE__, isconn);
    rcsp_lib_printf("hdl:0x%x, remote_addr:\n", (u32)con_handle);
    if (remote_addr) {
        rcsp_lib_printf_buf(remote_addr, 6);
    }
    if (con_handle) {
        u16 ble_con_handle = app_ble_get_hdl_con_handle(rcsp_server_ble_hdl);
        u16 ble_con_handle1 = 0;
        u16 adt_con_handle = 0;
        u16 adt_con_handle1 = 0;
        if (rcsp_dual_support) {
            ble_con_handle1 = app_ble_get_hdl_con_handle(rcsp_server_ble_hdl1);
        }
        if (adt_profile_support && rcsp_adt_support) {
            adt_con_handle = app_ble_get_hdl_con_handle(rcsp_server_edr_att_hdl);
            if (rcsp_dual_support) {
                adt_con_handle1 = app_ble_get_hdl_con_handle(rcsp_server_edr_att_hdl1);
            }
        }
        if (isconn) {
            rcsp_lib_printf("%s, %s, %d, b1:0x%x, b2:0x%x\n", __FILE__, __FUNCTION__, __LINE__, ble_con_handle, ble_con_handle1);
            if (ble_con_handle && (con_handle == ble_con_handle)) {
                rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
                /* app_ble_set_filter_con_handle(rcsp_server_ble_hdl, con_handle); */
                g_rcsp_ble_conn_num++;
            } else if (rcsp_dual_support && ble_con_handle1 && (con_handle == ble_con_handle1)) {
                rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
                /* app_ble_set_filter_con_handle(rcsp_server_ble_hdl1, con_handle); */
                g_rcsp_ble_conn_num++;
            } else if (adt_profile_support && rcsp_adt_support && adt_con_handle && (con_handle == adt_con_handle)) {
                rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
                /* app_ble_set_filter_con_handle(rcsp_server_ble_hdl1, con_handle); */
                g_rcsp_adt_conn_num++;
            } else if (adt_profile_support && rcsp_adt_support && adt_con_handle1 && (con_handle == adt_con_handle1)) {
                rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
                /* app_ble_set_filter_con_handle(rcsp_server_ble_hdl1, con_handle); */
                g_rcsp_adt_conn_num++;
            } else {
                printf("rcsp_server_ble_hdl is run out of!\n");
                return;
            }
            rcsp_protocol_bound(con_handle, NULL);
        } else {
            /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
            if (ble_con_handle == con_handle) {
                rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
                app_ble_set_filter_con_handle(rcsp_server_ble_hdl, 0);
                g_rcsp_ble_conn_num--;
            } else if (rcsp_dual_support && (ble_con_handle1 == con_handle)) {
                rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
                app_ble_set_filter_con_handle(rcsp_server_ble_hdl1, 0);
                g_rcsp_ble_conn_num--;
            } else if (adt_profile_support && rcsp_adt_support && adt_con_handle && (con_handle == adt_con_handle)) {
                rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
                app_ble_set_filter_con_handle(rcsp_server_edr_att_hdl, 0);
                g_rcsp_adt_conn_num--;
            } else if (rcsp_dual_support && adt_profile_support && rcsp_adt_support && adt_con_handle1 && (con_handle == adt_con_handle1)) {
                rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
                app_ble_set_filter_con_handle(rcsp_server_edr_att_hdl1, 0);
                g_rcsp_adt_conn_num--;
            }
            rcsp_protocol_reset_bound(con_handle, NULL);
        }
        if (rcsp_auth_support) {
            JL_rcsp_reset_bthdl_auth(con_handle, NULL);
        }
        if (rcsp_dual_support && (g_rcsp_ble_conn_num > 2)) {
            rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
            g_rcsp_ble_conn_num = 2;
        } else if (!rcsp_dual_support && (g_rcsp_ble_conn_num > 1)) {
            rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
            g_rcsp_ble_conn_num = 1;
        } else if (g_rcsp_ble_conn_num < 0) {
            rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
            g_rcsp_ble_conn_num = 0;
        }
        if (rcsp_dual_support && (g_rcsp_adt_conn_num > 2)) {
            rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
            g_rcsp_adt_conn_num = 2;
        } else if (!rcsp_dual_support && (g_rcsp_adt_conn_num > 1)) {
            rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
            g_rcsp_adt_conn_num = 1;
        } else if (g_rcsp_adt_conn_num < 0) {
            rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
            g_rcsp_adt_conn_num = 0;
        }
        /* u16 ble_con_hdl = app_ble_get_hdl_con_handle(rcsp_server_ble_hdl); */
        /* u16 ble_con_hdl1 = app_ble_get_hdl_con_handle(rcsp_server_ble_hdl1); */
        /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
        /* rcsp_lib_printf("ble_con_handle:%d, ble_con_handle1:%d\n", ble_con_hdl, ble_con_hdl1); */

    } else if (remote_addr) {
        u8 *spp_remote_addr = app_spp_get_hdl_remote_addr(bt_rcsp_spp_hdl);
        u8 *spp_remote_addr1 = NULL;
        if (rcsp_dual_support) {
            spp_remote_addr1 = app_spp_get_hdl_remote_addr(bt_rcsp_spp_hdl1);
        }
        /* if (spp_remote_addr) { */
        /* 	rcsp_lib_printf("%s, %s, %d, spp_remote_addr:", __FILE__, __FUNCTION__, __LINE__); */
        /* 	rcsp_lib_printf_buf(spp_remote_addr, 6); */
        /* } */
        /* if (spp_remote_addr1) {	 */
        /* 	rcsp_lib_printf("%s, %s, %d, spp_remote_addr1:", __FILE__, __FUNCTION__, __LINE__); */
        /* 	rcsp_lib_printf_buf(spp_remote_addr1, 6); */
        /* } */
        if (isconn) {
            if (!spp_remote_addr && (!spp_remote_addr1 || ((spp_remote_addr1 != NULL) && memcmp(remote_addr, spp_remote_addr1, 6)))) {
                rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
                app_spp_set_filter_remote_addr(bt_rcsp_spp_hdl, remote_addr);
                rcsp_protocol_bound(0, remote_addr);
                g_rcsp_spp_conn_num++;
            } else if (rcsp_dual_support && !spp_remote_addr1 && (!spp_remote_addr || ((spp_remote_addr != NULL) && memcmp(remote_addr, spp_remote_addr, 6)))) {
                rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
                app_spp_set_filter_remote_addr(bt_rcsp_spp_hdl1, remote_addr);
                rcsp_protocol_bound(0, remote_addr);
                g_rcsp_spp_conn_num++;
            } else {
                rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
                return;
            }
        } else {
            /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
            rcsp_protocol_reset_bound(0, remote_addr);
            // 底层已经清理了，这里可以不用清理
            g_rcsp_spp_conn_num = 0;
            if (spp_remote_addr) {
                rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
                g_rcsp_spp_conn_num++;
            }
            if (spp_remote_addr1) {
                rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
                g_rcsp_spp_conn_num++;
            }
        }
        if (rcsp_auth_support) {
            JL_rcsp_reset_bthdl_auth(0, remote_addr);
        }
        if (rcsp_dual_support && (g_rcsp_spp_conn_num > 2)) {
            rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
            g_rcsp_spp_conn_num = 2;
        } else if (!rcsp_dual_support && (g_rcsp_spp_conn_num > 1)) {
            rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
            g_rcsp_spp_conn_num = 1;
        } else if (g_rcsp_spp_conn_num < 0) {
            rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
            g_rcsp_spp_conn_num = 0;
        }
    }

    rcsp_interface_bt_handle_tws_sync();

}

/**
 *	@brief 用于外部接收ble/spp自定义数据使用
 *
 *	@param ble_con_hdl ble发送句柄
 *	@param remote_addr spp发送地址
 *	@param buf 接收数据
 *	@param len 接收数据的长度
 *	@param att_handle ble_con_hdl有值时，ble的特征值，一般是用户自定义的特征
 */
_WEAK_ void bt_rcsp_custom_recieve_callback(u16 ble_con_hdl, void *remote_addr, u8 *buf, u16 len, uint16_t att_handle)
{
}

/**
 *	@brief 用于外部自定义att_read_callback的使用
 *
 *	@param hdl ble的结构体句柄
 *	@param connection_handle ble的连接句柄
 *	@param att_handle ble特征值
 *	@param offset
 *	@param buffer 读特征的数据
 *	@param buffer_size 读特征的数据长度
 */
_WEAK_ uint16_t bt_rcsp_custom_att_read_callback(void *hdl, hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    return 0;
}

/**
 *	@brief 用于外部自定义att_write_callback的使用
 *
 *	@param hdl ble的结构体句柄
 *	@param connection_handle ble的连接句柄
 *	@param att_handle ble特征值
 *	@param transaction_mode
 *	@param offset
 *	@param buffer 写特征的数据
 *	@param buffer_size 写特征的数据长度
 */
_WEAK_ void bt_rcsp_custom_att_write_callback(void *hdl, hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
}

/**
 *	@brief 用于rcsp接收ble/spp数据使用
 *
 *	@param hdl ble/spp发送句柄
 *	@param remote_addr spp发送地址
 *	@param buf 接收数据
 *	@param len 接收数据的长度
 */
void bt_rcsp_recieve_callback(void *hdl, void *remote_addr, u8 *buf, u16 len)
{
    rcsp_lib_printf("===rcsp_rx(%d):", len);
    rcsp_lib_printf_buf(buf, len);
    /* rcsp_lib_printf("hdl:0x%x, remote_addr:\n", (u32)hdl); */
    /* if (remote_addr) { */
    /* rcsp_lib_printf_buf(remote_addr, 6); */
    /* } */

    // 获取监听hdl绑定的bthdl
    // 只有连接时绑定了handle才能处理数据
    if (hdl && (!remote_addr)) {
        u16 ble_con_handle = app_ble_get_hdl_con_handle(rcsp_server_ble_hdl);
        u16 ble_con_handle1 = 0;
        u16 adt_con_handle = 0;
        u16 adt_con_handle1 = 0;
        if (rcsp_dual_support) {
            ble_con_handle1 = app_ble_get_hdl_con_handle(rcsp_server_ble_hdl1);
        }
        if (adt_profile_support && rcsp_adt_support) {
            adt_con_handle = app_ble_get_hdl_con_handle(rcsp_server_edr_att_hdl);
            if (rcsp_dual_support) {
                adt_con_handle1 = app_ble_get_hdl_con_handle(rcsp_server_edr_att_hdl1);
            }
        }
        rcsp_lib_printf("ble_con_handle:%d, ble_con_handle1:%d\n", ble_con_handle, ble_con_handle1);
        if (ble_con_handle && (hdl == rcsp_server_ble_hdl)) {
            /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
            if (!JL_rcsp_get_auth_flag_with_bthdl(ble_con_handle, NULL)) {
                /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
                if (!rcsp_protocol_head_check(buf, len)) {
                    // 如果还没有验证，则只接收验证信息
                    rcsp_lib_printf("%d===rcsp_rx(%d):", __LINE__, len);
                    rcsp_lib_printf_buf(buf, len);
                    JL_rcsp_auth_recieve(ble_con_handle, NULL, buf, len);
                }
                return;
            }
            /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
            rcsp_lib_printf("%d===rcsp_rx(%d):", __LINE__, len);
            rcsp_lib_printf_buf(buf, len);
            JL_protocol_data_recieve(NULL, buf, len, ble_con_handle, NULL);
        } else if (ble_con_handle1 && (hdl == rcsp_server_ble_hdl1)) {
            /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
            if (!JL_rcsp_get_auth_flag_with_bthdl(ble_con_handle1, NULL)) {
                /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
                if (!rcsp_protocol_head_check(buf, len)) {
                    // 如果还没有验证，则只接收验证信息
                    rcsp_lib_printf("%d===rcsp_rx(%d):", __LINE__, len);
                    rcsp_lib_printf_buf(buf, len);
                    JL_rcsp_auth_recieve(ble_con_handle1, NULL, buf, len);
                }
                return;
            }
            /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
            rcsp_lib_printf("%d===rcsp_rx(%d):", __LINE__, len);
            rcsp_lib_printf_buf(buf, len);
            JL_protocol_data_recieve(NULL, buf, len, ble_con_handle1, NULL);
        } else if (adt_profile_support && adt_con_handle && (hdl == rcsp_server_edr_att_hdl)) {
            /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
            if (!JL_rcsp_get_auth_flag_with_bthdl(adt_con_handle, NULL)) {
                /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
                if (!rcsp_protocol_head_check(buf, len)) {
                    // 如果还没有验证，则只接收验证信息
                    rcsp_lib_printf("%d===rcsp_rx(%d):", __LINE__, len);
                    rcsp_lib_printf_buf(buf, len);
                    JL_rcsp_auth_recieve(adt_con_handle, NULL, buf, len);
                }
                return;
            }
            /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
            rcsp_lib_printf("%d===rcsp_rx(%d):", __LINE__, len);
            rcsp_lib_printf_buf(buf, len);
            JL_protocol_data_recieve(NULL, buf, len, adt_con_handle, NULL);
        } else if (adt_profile_support && adt_con_handle1 && (hdl == rcsp_server_edr_att_hdl1)) {
            /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
            if (!JL_rcsp_get_auth_flag_with_bthdl(adt_con_handle1, NULL)) {
                /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
                if (!rcsp_protocol_head_check(buf, len)) {
                    // 如果还没有验证，则只接收验证信息
                    rcsp_lib_printf("%d===rcsp_rx(%d):", __LINE__, len);
                    rcsp_lib_printf_buf(buf, len);
                    JL_rcsp_auth_recieve(adt_con_handle1, NULL, buf, len);
                }
                return;
            }
            /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
            rcsp_lib_printf("%d===rcsp_rx(%d):", __LINE__, len);
            rcsp_lib_printf_buf(buf, len);
            JL_protocol_data_recieve(NULL, buf, len, adt_con_handle1, NULL);
        }
    } else if (remote_addr) {
        u8 *spp_remote_addr = app_spp_get_hdl_remote_addr(bt_rcsp_spp_hdl);
        u8 *spp_remote_addr1 = NULL;
        if (rcsp_dual_support) {
            spp_remote_addr1 = app_spp_get_hdl_remote_addr(bt_rcsp_spp_hdl1);
        }
        if (spp_remote_addr) {
            rcsp_lib_printf("%s, %s, %d, spp_remote_addr:", __FILE__, __FUNCTION__, __LINE__);
            rcsp_lib_printf_buf(spp_remote_addr, 6);
        }
        if (spp_remote_addr1) {
            rcsp_lib_printf("%s, %s, %d, spp_remote_addr1:", __FILE__, __FUNCTION__, __LINE__);
            rcsp_lib_printf_buf(spp_remote_addr1, 6);
        }
        if ((spp_remote_addr && (hdl == bt_rcsp_spp_hdl)) || \
            (spp_remote_addr1 && (hdl == bt_rcsp_spp_hdl1))
           ) {
            if (!JL_rcsp_get_auth_flag_with_bthdl(0, remote_addr)) {
                if (!rcsp_protocol_head_check(buf, len)) {
                    // 如果还没有验证，则只接收验证信息
                    rcsp_lib_printf("%s, %s, %d", __FILE__, __FUNCTION__, __LINE__);
                    rcsp_lib_printf_buf(remote_addr, 6);
                    rcsp_lib_printf("%d===rcsp_rx(%d):", __LINE__, len);
                    rcsp_lib_printf_buf(buf, len);
                    JL_rcsp_auth_recieve(0, remote_addr, buf, len);
                }
                rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
                return;
            }
            rcsp_lib_printf("%s, %s, %d", __FILE__, __FUNCTION__, __LINE__);
            rcsp_lib_printf_buf(remote_addr, 6);
            rcsp_lib_printf("%d===rcsp_rx(%d):", __LINE__, len);
            rcsp_lib_printf_buf(buf, len);
            JL_protocol_data_recieve(NULL, buf, len, 0, remote_addr);
            bt_rcsp_custom_recieve_callback(0, remote_addr, buf, len, 0);
        }
    }
    /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
}

static uint16_t default_rcsp_send_att_handle = ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE;
static att_op_type_e default_rcsp_send_att_op_type = ATT_OP_AUTO_READ_CCC;

/**
 *	@brief 设置rcsp的ble的默认发送特征
 *
 *	@param att_handle ble_con_hdl有值时，可填用户自定义的特征, 为0是rcsp的特征值
 *	@param att_op_type 参考att_op_type_e枚举的排序，为0是rcsp的特征值
 */
void bt_rcsp_set_default_send_att_msg(uint16_t att_handle, att_op_type_e att_op_type)
{
    if (att_handle) {
        default_rcsp_send_att_handle = att_handle;
    } else {
        default_rcsp_send_att_handle = ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE;
    }
    if (att_op_type) {
        default_rcsp_send_att_op_type = att_op_type;
    } else {
        default_rcsp_send_att_op_type = ATT_OP_AUTO_READ_CCC;
    }
}

/**
 *	@brief 用于外部判断RCSP是否可以通过SPP发送数据
 *
 *	@return 1可以  0不可以
 *
 */
_WEAK_ u8 bt_rcsp_spp_can_send(void)
{
    //y_printf("bt_rcsp_spp_can_send\n");
    return 1;
}

/**
 *	@brief 用于发送rcsp的数据使用
 *
 *	@param ble_con_hdl ble发送句柄
 *	@param remote_addr spp发送地址	注：当ble_con_hdl与remote_addr都不填时，给所有的设备都发数据
 *	@param buf 发送的数据
 *	@param len 发送的数据长度
 *	@param att_handle ble_con_hdl有值时，可填用户自定义的特征, 为0是rcsp的特征值，或者是bt_rcsp_set_default_send_att_msg设置的
 *	@param att_op_type 参考att_op_type_e枚举的排序，为0是rcsp默认配置，或者是bt_rcsp_set_default_send_att_msg设置的
 */
int bt_rcsp_data_send(u16 ble_con_hdl, u8 *remote_addr, u8 *buf, u16 len, uint16_t att_handle, u8 att_op_type)
{
    if (ble_con_hdl) {
        rcsp_lib_printf("ble_con_hdl:%d\n", ble_con_hdl);
    }
    u8 _addr_temp[6] = {0};
    if (remote_addr && memcmp(remote_addr, _addr_temp, 6)) {
        rcsp_lib_printf("remote_addr(spp):\n");
        rcsp_lib_printf_buf(remote_addr, 6);
    } else {
        if (!ble_con_hdl) {
            return 0;
        }
    }
    rcsp_lib_printf("===rcsp_tx(%d):", len);
    rcsp_lib_printf_buf(buf, len);
    uint16_t send_att_handle = default_rcsp_send_att_handle;
    att_op_type_e send_att_op_type = default_rcsp_send_att_op_type;
    if (att_handle) {
        send_att_handle = att_handle;
        rcsp_lib_printf("att_handle:%d\n", att_handle);
    }
    if (att_op_type) {
        send_att_op_type = (att_op_type_e)att_op_type;
        rcsp_lib_printf("att_op_type:%d\n", att_op_type);
    }
    rcsp_lib_printf("send_att_handle:%d\n", send_att_handle);
    rcsp_lib_printf("send_att_op_type:%d\n", send_att_op_type);
    u16 ble_con_handle = 0, ble_con_handle1 = 0, adt_con_handle = 0;
    u8 *spp_remote_addr = NULL;
    u8 *spp_remote_addr1 = NULL;
    u16 adt_con_handle1 = 0;
    if (ble_con_hdl) {
        ble_con_handle = app_ble_get_hdl_con_handle(rcsp_server_ble_hdl);
        if (rcsp_dual_support) {
            ble_con_handle1 = app_ble_get_hdl_con_handle(rcsp_server_ble_hdl1);
        }
        if (adt_profile_support && rcsp_adt_support) {
            adt_con_handle = app_ble_get_hdl_con_handle(rcsp_server_edr_att_hdl);
            if (rcsp_dual_support) {
                adt_con_handle1 = app_ble_get_hdl_con_handle(rcsp_server_edr_att_hdl1);
            }
        }
        if (ble_con_hdl == ble_con_handle) {
            ble_con_handle1 = 0;
        } else if (ble_con_hdl == ble_con_handle1) {
            ble_con_handle = 0;
        } else {
            ble_con_handle = 0;
            ble_con_handle1 = 0;
        }
    }
    if (remote_addr && memcmp(remote_addr, _addr_temp, 6)) {
        spp_remote_addr = app_spp_get_hdl_remote_addr(bt_rcsp_spp_hdl);
        if (rcsp_dual_support) {
            spp_remote_addr1 = app_spp_get_hdl_remote_addr(bt_rcsp_spp_hdl1);
        }
        if (spp_remote_addr && !memcmp(remote_addr, spp_remote_addr, 6)) {
            spp_remote_addr1 = NULL;
        } else if (spp_remote_addr1 && !memcmp(remote_addr, spp_remote_addr1, 6)) {
            spp_remote_addr = NULL;
        } else {
            spp_remote_addr = NULL;
            spp_remote_addr1 = NULL;
        }
    }
    /* rcsp_lib_printf("111111ble_con_handle:%d, ble_con_handle1:%d\n", ble_con_handle, ble_con_handle1); */
    /* if (spp_remote_addr) { */
    /* rcsp_lib_printf("222222spp_remote_addr:\n"); */
    /* rcsp_lib_printf_buf(spp_remote_addr, 6); */
    /* } */
    /* if (spp_remote_addr1) { */
    /* rcsp_lib_printf("222222spp_remote_addr1:\n"); */
    /* rcsp_lib_printf_buf(spp_remote_addr1, 6); */
    /* } */
    if (spp_remote_addr && bt_rcsp_spp_can_send()) {
        if (!JL_rcsp_get_auth_flag_with_bthdl(0, spp_remote_addr)) {
            if (!rcsp_protocol_head_check(buf, len)) {
                // 如果还没有验证，则只发送验证信息
                /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
                app_spp_data_send(bt_rcsp_spp_hdl, buf, len);
            }
        } else {
            /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
            app_spp_data_send(bt_rcsp_spp_hdl, buf, len);
        }
    }
    if (ble_con_handle) {
        if (!JL_rcsp_get_auth_flag_with_bthdl(ble_con_handle, NULL)) {
            if (!rcsp_protocol_head_check(buf, len)) {
                // 如果还没有验证，则只发送验证信息
                /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
                app_ble_att_send_data(rcsp_server_ble_hdl, send_att_handle, buf, len, send_att_op_type);
            }
        } else {
            /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
            app_ble_att_send_data(rcsp_server_ble_hdl, send_att_handle, buf, len, send_att_op_type);
        }
    }
    if (rcsp_dual_support && spp_remote_addr1 && bt_rcsp_spp_can_send()) {
        if (!JL_rcsp_get_auth_flag_with_bthdl(0, spp_remote_addr1)) {
            if (!rcsp_protocol_head_check(buf, len)) {
                // 如果还没有验证，则只发送验证信息
                /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
                app_spp_data_send(bt_rcsp_spp_hdl1, buf, len);
            }
        } else {
            /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
            app_spp_data_send(bt_rcsp_spp_hdl1, buf, len);
        }
    }
    if (rcsp_dual_support && ble_con_handle1) {
        if (!JL_rcsp_get_auth_flag_with_bthdl(ble_con_handle1, NULL)) {
            if (!rcsp_protocol_head_check(buf, len)) {
                // 如果还没有验证，则只发送验证信息
                /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
                app_ble_att_send_data(rcsp_server_ble_hdl1, send_att_handle, buf, len, send_att_op_type);
            }
        } else {
            /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
            app_ble_att_send_data(rcsp_server_ble_hdl1, send_att_handle, buf, len, send_att_op_type);
        }
    }
    if (adt_profile_support && rcsp_adt_support && adt_con_handle) {
        if (!JL_rcsp_get_auth_flag_with_bthdl(adt_con_handle, NULL)) {
            if (!rcsp_protocol_head_check(buf, len)) {
                // 如果还没有验证，则只发送验证信息
                /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
                app_ble_att_send_data(rcsp_server_edr_att_hdl, send_att_handle, buf, len, send_att_op_type);
            }
        } else {
            /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
            app_ble_att_send_data(rcsp_server_edr_att_hdl, send_att_handle, buf, len, send_att_op_type);
        }
    }
    if (adt_profile_support && rcsp_adt_support && adt_con_handle1) {
        if (!JL_rcsp_get_auth_flag_with_bthdl(adt_con_handle1, NULL)) {
            if (!rcsp_protocol_head_check(buf, len)) {
                // 如果还没有验证，则只发送验证信息
                /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
                app_ble_att_send_data(rcsp_server_edr_att_hdl1, send_att_handle, buf, len, send_att_op_type);
            }
        } else {
            /* rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__); */
            app_ble_att_send_data(rcsp_server_edr_att_hdl1, send_att_handle, buf, len, send_att_op_type);
        }
    }
    return 0;
}

/**
 *	@brief 用于发送ble/spp自定义数据使用
 *
 *	@param ble_con_hdl ble发送句柄
 *	@param remote_addr spp发送地址	注：当ble_con_hdl与remote_addr都不填时，给所有的设备都发数据
 *	@param buf 发送的数据
 *	@param len 发送的数据长度
 *	@param att_handle ble_con_hdl有值时，可填用户自定义的特征, 为0是rcsp的特征值
 *	@param att_op_type 参考att_op_type_e枚举的排序
 */
void bt_rcsp_custom_data_send(u16 ble_con_hdl, u8 *remote_addr, u8 *buf, u16 len, uint16_t att_handle, att_op_type_e att_op_type)
{
    u8 _addr_temp[6] = {0};
    u8 flag = 0;
    if (remote_addr) {
        flag = memcmp(remote_addr, _addr_temp, 6);
    }
    if ((ble_con_hdl != 0) || flag) {
        bt_rcsp_data_send(ble_con_hdl, remote_addr, buf, len, att_handle, att_op_type);
    } else if (/*(ble_con_hdl==0)&& */ !remote_addr) {
        // 全发送
        u16 ble_con_handle = app_ble_get_hdl_con_handle(rcsp_server_ble_hdl);
        u8 *spp_remote_addr = app_spp_get_hdl_remote_addr(bt_rcsp_spp_hdl);
        u16 adt_con_handle = 0;
        if (adt_profile_support && rcsp_adt_support) {
            adt_con_handle = app_ble_get_hdl_con_handle(rcsp_server_edr_att_hdl);
        }
        if (adt_con_handle != 0) {
            bt_rcsp_data_send(adt_con_handle, NULL, buf, len, att_handle, att_op_type);
        }
        if (ble_con_handle != 0) {
            bt_rcsp_data_send(ble_con_handle, NULL, buf, len, att_handle, att_op_type);
        }
        if (spp_remote_addr) {
            bt_rcsp_data_send(0, spp_remote_addr, buf, len, 0, 0);
        }
        if (rcsp_dual_support) {
            u16 ble_con_handle1 = app_ble_get_hdl_con_handle(rcsp_server_ble_hdl1);
            if (ble_con_handle1) {
                bt_rcsp_data_send(ble_con_handle1, NULL, buf, len, att_handle, att_op_type);
            }
            u8 *spp_remote_addr1 = app_spp_get_hdl_remote_addr(bt_rcsp_spp_hdl1);
            if (spp_remote_addr1) {
                bt_rcsp_data_send(0, spp_remote_addr1, buf, len, 0, 0);
            }
            u16 adt_con_handle1 = 0;
            if (adt_profile_support && rcsp_adt_support) {
                adt_con_handle1 = app_ble_get_hdl_con_handle(rcsp_server_edr_att_hdl1);
            }
            if (adt_con_handle1 != 0) {
                bt_rcsp_data_send(adt_con_handle1, NULL, buf, len, att_handle, att_op_type);
            }
        }
    }
}

static void cbk_connect_handler(void *hdl, u8 state)
{
    rcsp_lib_printf("cbk_connect_handler hdl:0x%x, state:%d rcsp_server_ble_hdl:0x%x, rcsp_server_ble_hdl1:0x%x\n", (u32)hdl, state, (u32)rcsp_server_ble_hdl, (u32)rcsp_server_ble_hdl1);
}

/*****************************************
                    BLE
 *****************************************/

static uint16_t att_read_callback(void *hdl, hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    uint16_t  att_value_len = 0;
    uint16_t handle = att_handle;
    rcsp_lib_printf("read_callback, connection_handle:%0x, handle= 0x%04x, buffer= %08x\n", connection_handle, handle, (u32)buffer);

    switch (handle) {
    case ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE:
        att_value_len = strlen(gap_device_name);

        if ((offset >= att_value_len) || (offset + buffer_size) > att_value_len) {
            break;
        }

        if (buffer) {
            memcpy(buffer, &gap_device_name[offset], buffer_size);
            att_value_len = buffer_size;
            rcsp_lib_printf("\n------read gap_name: %s \n", gap_device_name);
        }
        break;

    case ATT_CHARACTERISTIC_2a05_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE:
        if (buffer) {
            buffer[0] = att_get_ccc_config(handle);
            buffer[1] = 0;
        }
        att_value_len = 2;
        break;

    default:
        u16 ble_con_handle = app_ble_get_hdl_con_handle(rcsp_server_ble_hdl);
        u16 ble_con_handle1 = app_ble_get_hdl_con_handle(rcsp_server_ble_hdl1);
        u16 adt_con_handle = 0;
        u16 adt_con_handle1 = 0;
        if (adt_profile_support && rcsp_adt_support) {
            adt_con_handle = app_ble_get_hdl_con_handle(rcsp_server_edr_att_hdl);
            adt_con_handle1 = app_ble_get_hdl_con_handle(rcsp_server_edr_att_hdl1);
        }
        if (ble_con_handle && (hdl == rcsp_server_ble_hdl)) {
            bt_rcsp_custom_recieve_callback(ble_con_handle, NULL, buffer, buffer_size, handle);
        } else if (adt_con_handle && (hdl == rcsp_server_edr_att_hdl)) {
            bt_rcsp_custom_recieve_callback(adt_con_handle, NULL, buffer, buffer_size, handle);
        } else if (adt_con_handle1 && (hdl == rcsp_server_edr_att_hdl1)) {
            bt_rcsp_custom_recieve_callback(adt_con_handle1, NULL, buffer, buffer_size, handle);
        } else if (ble_con_handle1 && (hdl == rcsp_server_ble_hdl1)) {
            bt_rcsp_custom_recieve_callback(ble_con_handle1, NULL, buffer, buffer_size, handle);
        }
        break;
    }
    rcsp_lib_printf("att_value_len= %d\n", att_value_len);
    uint16_t custom_att_value_len = bt_rcsp_custom_att_read_callback(hdl, connection_handle, att_handle, offset, buffer, buffer_size);
    if (custom_att_value_len) {
        att_value_len = custom_att_value_len;
    }
    return att_value_len;
}

static int att_write_callback(void *hdl, hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    u16 handle = att_handle;
    /* rcsp_lib_printf("write_callback, connection_handle:%0x, handle= 0x%04x\n", connection_handle, handle); */

    switch (handle) {
    case ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE:
        u16 tmp16 = BT_NAME_LEN_MAX;
        if ((offset >= tmp16) || (offset + buffer_size) > tmp16) {
            break;
        }

        if (offset == 0) {
            memset(gap_device_name, 0x00, BT_NAME_LEN_MAX);
        }
        memcpy(&gap_device_name[offset], buffer, buffer_size);
        rcsp_lib_printf("\n------write gap_name:");
        break;

    case ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE:
        /* rcsp_lib_printf("eeeeeeeeeeeeeeeeee %s, %s, %d, connection_handle:%d\n", __FILE__, __FUNCTION__, __LINE__, connection_handle); */
        if (rcsp_auth_support) {
            // 使能notify的时候reset auth保证app能重新连接
            // 处理APP断开后台还连接的情况
            JL_rcsp_reset_bthdl_auth(connection_handle, NULL);
        }

        /* set_ble_work_state(BLE_ST_NOTIFY_IDICATE); */
        rcsp_lib_printf("\n------write ccc:%04x, %02x\n", handle, buffer[0]);
        att_set_ccc_config(handle, buffer[0]);

        /* can_send_now_wakeup(); */
        JL_protocol_resume();

        break;

    case ATT_CHARACTERISTIC_2a05_01_CLIENT_CONFIGURATION_HANDLE:
        att_set_ccc_config(handle, buffer[0]);
        break;

    case ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE:
        /* rcsp_lib_printf("ble_rx(%d):\n", buffer_size); */
        /* rcsp_lib_printf_buf(buffer, buffer_size); */
        bt_rcsp_recieve_callback(hdl, NULL, buffer, buffer_size);

    // rcsp的特征也允许通过custom recieve接口返回给外面的开发者使用
    /* break; */
    default:
        u16 ble_con_handle = app_ble_get_hdl_con_handle(rcsp_server_ble_hdl);
        u16 ble_con_handle1 = app_ble_get_hdl_con_handle(rcsp_server_ble_hdl1);
        u16 adt_con_handle = 0;
        u16 adt_con_handle1 = 0;
        if (adt_profile_support && rcsp_adt_support) {
            adt_con_handle = app_ble_get_hdl_con_handle(rcsp_server_edr_att_hdl);
            adt_con_handle1 = app_ble_get_hdl_con_handle(rcsp_server_edr_att_hdl1);
        }
        if (ble_con_handle && (hdl == rcsp_server_ble_hdl)) {
            bt_rcsp_custom_recieve_callback(ble_con_handle, NULL, buffer, buffer_size, handle);
        } else if (ble_con_handle1 && (hdl == rcsp_server_ble_hdl1)) {
            bt_rcsp_custom_recieve_callback(ble_con_handle1, NULL, buffer, buffer_size, handle);
        } else if (adt_con_handle && (hdl == rcsp_server_edr_att_hdl)) {
            bt_rcsp_custom_recieve_callback(adt_con_handle, NULL, buffer, buffer_size, handle);
        } else if (adt_con_handle1 && (hdl == rcsp_server_edr_att_hdl1)) {
            bt_rcsp_custom_recieve_callback(adt_con_handle1, NULL, buffer, buffer_size, handle);
        }
        break;
    }
    bt_rcsp_custom_att_write_callback(hdl, connection_handle, att_handle, transaction_mode, offset, buffer, buffer_size);

    return 0;
}

extern void rcsp_user_cbk_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static void cbk_packet_handler(void *hdl, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    rcsp_user_cbk_packet_handler(packet_type, channel, packet, size);
}

extern void rcsp_user_cbk_sm_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static void cbk_sm_packet_handler(void *hdl, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    rcsp_user_cbk_sm_packet_handler(packet_type, channel, packet, size);
}



/*****************************************
                    SPP
 *****************************************/

extern int rcsp_user_spp_state_specific(u8 packet_type, u8 *spp_remote_addr);
extern void rcsp_user_event_spp_handler(u8 spp_status, u8 flag);
void bt_rcsp_spp_state_callback(void *hdl, void *remote_addr, u8 state)
{
    switch (state) {
    case SPP_USER_ST_CONNECT:
        printf("bt_rcsp rfcomm connect###########\n");

        bt_rcsp_set_conn_info(0, remote_addr, true);

        break;
    case SPP_USER_ST_DISCONN:
        printf("bt_rcsp spp comm disconnect#########\n");

        bt_rcsp_set_conn_info(0, remote_addr, false);

        break;
    };

    // rcsp_user_spp_state_specific需要上面绑定设备后才能被调用
    rcsp_user_spp_state_specific(state, remote_addr);
    // 需要app_core线程处理的操作放到以下函数
    rcsp_user_event_spp_handler(state, 1);
}

/*****************************************
             RCSP PROTOCOL INIT
 *****************************************/

void app_ble_callback_register(void *rcsp_server_ble_hdl);
void app_spp_callback_register(void *bt_rcsp_spp_hdl);
extern const char *bt_get_local_name();

_WEAK_ const char *rcsp_ble_name()
{
    return bt_get_local_name();
}

void bt_rcsp_interface_init(const uint8_t *rcsp_profile_data)
{
    gap_device_name = (char *)rcsp_ble_name();
    gap_device_name_len = strlen(gap_device_name);
    if (gap_device_name_len > BT_NAME_LEN_MAX) {
        gap_device_name_len = BT_NAME_LEN_MAX;
    }

    rcsp_lib_printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
    // spp init
    if (bt_rcsp_spp_hdl == NULL) {
        bt_rcsp_spp_hdl = app_spp_hdl_alloc(0x0);
        if (bt_rcsp_spp_hdl == NULL) {
            ASSERT(0, "bt_rcsp_spp_hdl alloc err !!\n");
            return;
        }
    }
    app_spp_callback_register(bt_rcsp_spp_hdl);

    // ble init
    if (rcsp_server_ble_hdl == NULL) {
        rcsp_server_ble_hdl = app_ble_hdl_alloc();
        if (rcsp_server_ble_hdl == NULL) {
            ASSERT(0, "rcsp_server_ble_hdl alloc err !!\n");
            return;
        }
    }
    app_ble_adv_address_type_set(rcsp_server_ble_hdl, 0);
    app_ble_profile_set(rcsp_server_ble_hdl, rcsp_profile_data);
    app_ble_callback_register(rcsp_server_ble_hdl);
    if (adt_profile_support && rcsp_adt_support) {
        if (rcsp_server_edr_att_hdl == NULL) {
            rcsp_server_edr_att_hdl = app_ble_hdl_alloc();
            if (rcsp_server_edr_att_hdl == NULL) {
                ASSERT(0, "rcsp_server_edr_att_hdl alloc err !!\n");
                return;
            }
        }
        app_ble_adv_address_type_set(rcsp_server_edr_att_hdl, 0);
        app_ble_att_connect_type_set(rcsp_server_edr_att_hdl, BD_ADDR_TYPE_EDR_ATT);
        app_ble_hdl_uuid_set(rcsp_server_edr_att_hdl, EDR_ATT_HDL_UUID);
        app_ble_profile_set(rcsp_server_edr_att_hdl, rcsp_profile_data);
        app_ble_callback_register(rcsp_server_edr_att_hdl);
        if (rcsp_dual_support) {
            if (rcsp_server_edr_att_hdl1 == NULL) {
                rcsp_server_edr_att_hdl1 = app_ble_hdl_alloc();
                if (rcsp_server_edr_att_hdl1 == NULL) {
                    ASSERT(0, "rcsp_server_edr_att_hdl1 alloc err !!\n");
                    return;
                }
            }
            app_ble_adv_address_type_set(rcsp_server_edr_att_hdl1, 0);
            app_ble_att_connect_type_set(rcsp_server_edr_att_hdl1, BD_ADDR_TYPE_EDR_ATT);
            app_ble_hdl_uuid_set(rcsp_server_edr_att_hdl1, EDR_ATT_HDL_UUID);
            app_ble_profile_set(rcsp_server_edr_att_hdl1, rcsp_profile_data);
            app_ble_callback_register(rcsp_server_edr_att_hdl1);
        }
    }

    // 是否支持一拖二
    if (rcsp_dual_support) {
        // spp init
        if (bt_rcsp_spp_hdl1 == NULL) {
            bt_rcsp_spp_hdl1 = app_spp_hdl_alloc(0x0);
            if (bt_rcsp_spp_hdl1 == NULL) {
                ASSERT(0, "bt_rcsp_spp_hdl1 alloc err !!\n");
                return;
            }
        }
        app_spp_callback_register(bt_rcsp_spp_hdl1);

        // ble init
        if (rcsp_server_ble_hdl1 == NULL) {
            rcsp_server_ble_hdl1 = app_ble_hdl_alloc();
            if (rcsp_server_ble_hdl1 == NULL) {
                ASSERT(0, "rcsp_server_ble_hdl1 alloc err !!\n");
                return;
            }
        }
        app_ble_adv_address_type_set(rcsp_server_ble_hdl, 0);
        app_ble_profile_set(rcsp_server_ble_hdl1, rcsp_profile_data);
        app_ble_callback_register(rcsp_server_ble_hdl1);
    }

    // protocol init
    JL_rcsp_auth_init(bt_rcsp_data_send, (u8 *)rcsp_link_key_data, NULL);

}



void bt_rcsp_interface_exit(void)
{
    if (app_ble_get_hdl_con_handle(rcsp_server_ble_hdl)) {
        app_ble_disconnect(rcsp_server_ble_hdl);
    }
    app_ble_hdl_free(rcsp_server_ble_hdl);
    rcsp_server_ble_hdl = NULL;

    if (app_spp_get_hdl_remote_addr(bt_rcsp_spp_hdl)) {
        app_spp_disconnect(bt_rcsp_spp_hdl);
    }
    app_spp_hdl_free(bt_rcsp_spp_hdl);
    bt_rcsp_spp_hdl = NULL;

    if (rcsp_dual_support) {
        if (app_ble_get_hdl_con_handle(rcsp_server_ble_hdl1)) {
            app_ble_disconnect(rcsp_server_ble_hdl1);
        }
        app_ble_hdl_free(rcsp_server_ble_hdl1);
        rcsp_server_ble_hdl1 = NULL;
        if (app_spp_get_hdl_remote_addr(bt_rcsp_spp_hdl1)) {
            app_spp_disconnect(bt_rcsp_spp_hdl1);
        }
        app_spp_hdl_free(bt_rcsp_spp_hdl1);
        bt_rcsp_spp_hdl1 = NULL;
    }
}


void app_ble_callback_register(void *rcsp_server_ble_hdl)
{
    app_ble_att_read_callback_register(rcsp_server_ble_hdl, att_read_callback);
    app_ble_att_write_callback_register(rcsp_server_ble_hdl, att_write_callback);
    app_ble_att_server_packet_handler_register(rcsp_server_ble_hdl, cbk_packet_handler);
    app_ble_hci_event_callback_register(rcsp_server_ble_hdl, cbk_packet_handler);
    app_ble_l2cap_packet_handler_register(rcsp_server_ble_hdl, cbk_packet_handler);
    app_ble_sm_event_callback_register(rcsp_server_ble_hdl, cbk_sm_packet_handler);
}

void app_spp_callback_register(void *bt_rcsp_spp_hdl)
{
    app_spp_recieve_callback_register(bt_rcsp_spp_hdl, bt_rcsp_recieve_callback);
    app_spp_state_callback_register(bt_rcsp_spp_hdl, bt_rcsp_spp_state_callback);
    app_spp_wakeup_callback_register(bt_rcsp_spp_hdl, NULL);
}

#endif

