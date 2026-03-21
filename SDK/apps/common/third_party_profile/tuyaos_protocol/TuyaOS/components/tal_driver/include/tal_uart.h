/**
 * @file tal_uart.h
 * @brief This is tal_uart file
 * @version 1.0
 * @date 2021-08-24
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TAL_UART_H__
#define __TAL_UART_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define O_BLOCK 1 /* 阻塞读写 */
/* 通过注册回调区别是不是异步写 */
#define O_ASYNC_WRITE (1 << 1) /* 异步写 */
#define O_FLOW_CTRL (1 << 2) /* 流控使能 */

/* 用KCONFIG配置 */
#define O_TX_DMA (1 << 3) /* 用DMA发送 */
#define O_RX_DMA (1 << 4) /* 用DMA接收 */

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
typedef struct {
    UINT32_T  rx_buffer_size;
#ifdef CONFIG_TX_ASYNC
    UINT32_T  tx_buffer_size;
#endif
    UINT8_T open_mode;
    TUYA_UART_BASE_CFG_T base_cfg;
} TAL_UART_CFG_T;

/**
 * @brief uart irq callback
 *
* @param[in] port_id: uart port id, id index starts from 0
*                     in linux platform,
*                         high 16 bits aslo means uart type,
*                                   it's value must be one of the TUYA_UART_TYPE_E type
*                         the low 16bit - means uart port id
*                         you can input like this TUYA_UART_PORT_ID(TUYA_UART_SYS, 2)
 * @param[out] buff: data receive buff
 * @param[in] len: receive length
 *
 * @return none
 */
typedef VOID_T(*TAL_UART_IRQ_CB)(TUYA_UART_NUM_E port_id, VOID_T *buff, UINT16_T len);

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/

/**
 * @brief This API is used to init uart.
 *
 * @param[in] port_id: uart port id, id index starts from 0
 *                     in linux platform,
 *                     high 16 bits aslo means uart type,
 *                     it's value must be one of the TUYA_UART_TYPE_E type
 *                     the low 16bit - means uart port id
 *                     you can input like this TUYA_UART_PORT_ID(TUYA_UART_SYS, 2)
 * @param[in] *cfg: uart configure
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_uart_init(TUYA_UART_NUM_E port_id, TAL_UART_CFG_T *cfg);

/**
 * @brief This API is used to read data from uart.
 *
 * @param[in] port_id: uart port id, id index starts from 0
 *                     in linux platform,
 *                     high 16 bits aslo means uart type,
 *                     it's value must be one of the TUYA_UART_TYPE_E type
 *                     the low 16bit - means uart port id
 *                     you can input like this TUYA_UART_PORT_ID(TUYA_UART_SYS, 2)
 * @param[in] *cfg: uart configure
 *
 * @return >=0, the read size; < 0, read error
 */
INT32_T tal_uart_read(TUYA_UART_NUM_E port_id, UINT8_T *data, UINT32_T len);

/**
* @brief send data by uart
*
* @param[in] port_id: uart port id, id index starts from 0
*                     in linux platform,
*                         high 16 bits aslo means uart type,
*                                   it's value must be one of the TUYA_UART_TYPE_E type
*                         the low 16bit - means uart port id
*                         you can input like this TUYA_UART_PORT_ID(TUYA_UART_SYS, 2)
* @param[in] data: send data buffer
* @param[in] len: the send size
*
* @note This API is used to send data by uart.
*
* @return >=0, the write size; < 0, write error
*/

/**
 * @brief tal_uart_write
 *
 * @param[in] port_id: port_id
 * @param[in] *data: *data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
INT32_T tal_uart_write(TUYA_UART_NUM_E port_id, CONST UINT8_T *data, UINT32_T len);

/**
 * @brief This API is used to deinit uart.
 *
 * @param[in] port_id: uart port id, id index starts from 0
 *                     in linux platform,
 *                     high 16 bits aslo means uart type,
 *                     it's value must be one of the TUYA_UART_TYPE_E type
 *                     the low 16bit - means uart port id
 *                     you can input like this TUYA_UART_PORT_ID(TUYA_UART_SYS, 2)
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_uart_deinit(TUYA_UART_NUM_E port_id);

/**
 * @brief enable uart rx interrupt and register interrupt callback func
 *
* @param[in] port_id: uart port id, id index starts from 0
*                     in linux platform,
*                         high 16 bits aslo means uart type,
*                                   it's value must be one of the TUYA_UART_TYPE_E type
*                         the low 16bit - means uart port id
*                         you can input like this TUYA_UART_PORT_ID(TUYA_UART_SYS, 2)
 * @param[in] rx_cb: receive interrupt callback
 *
 * @return none
 */
VOID_T tal_uart_rx_reg_irq_cb(TUYA_UART_NUM_E port_id, TAL_UART_IRQ_CB rx_cb);


#ifdef __cplusplus
}
#endif

#endif /* __TAL_UART_H__ */
