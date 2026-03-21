/**
 * @file tuya_ble_port.h
 * @brief This is tuya_ble_port file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_PORT_H__
#define __TUYA_BLE_PORT_H__

#include "tuya_ble_type.h"
#include "tuya_ble_config.h"
#include "tkl_system.h"
#include "tal_ble_md5.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define tuya_ble_device_enter_critical()        TKL_ENTER_CRITICAL()
#define tuya_ble_device_exit_critical()         TKL_EXIT_CRITICAL()

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/

/**
 * @brief tuya_ble_gap_advertising_adv_data_update
 *
 * @param[in] p_ad_data: p_ad_data
 * @param[in] ad_len: ad_len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_gap_advertising_adv_data_update(UINT8_T CONST *p_ad_data, UINT8_T ad_len);

/**
 * @brief tuya_ble_gap_advertising_scan_rsp_data_update
 *
 * @param[in] *p_sr_data: *p_sr_data
 * @param[in] sr_len: sr_len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_gap_advertising_scan_rsp_data_update(UINT8_T CONST *p_sr_data, UINT8_T sr_len);

/**
 * @brief tuya_ble_gap_disconnect
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_gap_disconnect(VOID_T);

/**
 * @brief tuya_ble_gatt_send_data
 *
 * @param[in] *p_data: *p_data
 * @param[in] len: len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_gatt_send_data(CONST UINT8_T *p_data, UINT16_T len);

/**
 * @brief Function for update the device information characteristic value.
 *
 *@note      The device information characteristic uuid : 00000003-0000-1001-8001-00805F9B07D0
 *
 * @param[in] p_data: The pointer to the data to be updated.
 * @param[in] data_len: The length of the data to be updated.
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_device_info_characteristic_value_update(UINT8_T CONST *p_data, UINT8_T data_len);

/**
 * @brief tuya_ble_long_range_ext_adv_data_update
 *
 * @param[in] p_ad_data: p_data
 * @param[in] ad_len: data_len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_long_range_ext_adv_data_update(UINT8_T CONST *p_data, UINT8_T data_len);

/**
 * @brief Function for security request.
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_link_security_request(void);

/**
 * @brief Create a timer.
 *
 * @param[in] p_timer_id: a pointer to timer id address which can uniquely identify the timer.
 * @param[out] timeout_value_ms: Number of milliseconds to time-out event
 * @param[in] mode: mode
 * @param[out] timeout_handler: a pointer to a function which can be called when the timer expires.
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_timer_create(VOID_T **p_timer_id, UINT32_T timeout_value_ms, tuya_ble_timer_mode mode, tuya_ble_timer_handler_t timeout_handler);

/**
 * @brief Delete a timer.
 *
 * @param[in] timer_id: timer_id
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_timer_delete(VOID_T *timer_id);

/**
 * @brief Start a timer.
 *
 * @param[in] timer_id: timer_id
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_timer_start(VOID_T *timer_id);

/**
 * @brief Restart a timer.
 *
 * @note    If the timer has already started, it will start counting again.
 *
 * @param[in] timer_id: timer_id
 * @param[out] timeout_value_ms: New number of milliseconds to time-out event
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_timer_restart(VOID_T *timer_id, UINT32_T timeout_value_ms);

/**
 * @brief Stop a timer.
 *
 * @param[in] timer_id: timer_id
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_timer_stop(VOID_T *timer_id);

/**
 * @brief Function for delaying execution for a number of milliseconds.
 *
 * @param ms_time Number of milliseconds to wait.
 */
VOID_T tuya_ble_device_delay_ms(UINT32_T ms);

/**
 * @brief Function for delaying execution for a number of microseconds.
 *
 * @param us_time Number of microseconds to wait.
 */
VOID_T tuya_ble_device_delay_us(UINT32_T us);

/**
 * @brief Function for RESET device.
 *
 * @param[in] param: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_device_reset(VOID_T);

/**
 * @brief Function for get mac addr.
 *
 * @param[in] *p_addr: *p_addr
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_gap_addr_get(tuya_ble_gap_addr_t *p_addr);

/**
 * @brief Function for update mac addr.
 *
 * @param[in] *p_addr: *p_addr
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_gap_addr_set(tuya_ble_gap_addr_t *p_addr);

/**
 * @brief Get ture random bytes .
 *
 * @note    SHOULD use TRUE random num generator
 *
 * @param[in] p_buf: pointer to data
 * @param[in] len: Number of bytes to take from pool and place in
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_rand_generator(UINT8_T *p_buf, UINT8_T len);

/**
 * @brief Function for get the unix timestamp.
 *
 * @note    timezone: 100 times the actual time zone
 *
 * @param[in] *timestamp: *timestamp
 * @param[in] *timezone: *timezone
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_rtc_get_timestamp(UINT32_T *timestamp, INT32_T *timezone);

/**
 * @brief Function for set the unix timestamp.
 *
 * @note    timezone: 100 times the actual time zone,Eastern eight zones:8x100
 *
 * @param[in] timestamp: timestamp
 * @param[in] timezone: timezone
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_rtc_set_timestamp(UINT32_T timestamp, INT32_T timezone);

/**
 * @brief Initialize the NV module.
 *
 * @param[in] none: none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_nv_init(VOID_T);

/**
 * @brief Erase data on flash.
 * @note This operation is irreversible.
 * @note This operation's units is different which on many chips.
 *
 * @param addr flash address
 * @param size erase bytes size
 *
 * @return result
 */
tuya_ble_status_t tuya_ble_nv_erase(UINT32_T addr, UINT32_T size);

/**
 * @brief Write data to flash.
 *
 * @note This operation must after erase. @see tuya_ble_nv_erase.
 *
 * @param addr flash address
 * @param p_data the write data buffer
 * @param size write bytes size
 *
 * @return result
 */
tuya_ble_status_t tuya_ble_nv_write(UINT32_T addr, CONST UINT8_T *p_data, UINT32_T size);

/**
 * @brief Read data from flash.
 * @note
 *
 * @param addr flash address
 * @param buf buffer to store read data
 * @param size read bytes size
 *
 * @return result
 */
tuya_ble_status_t tuya_ble_nv_read(UINT32_T addr, UINT8_T *p_data, UINT32_T size);

/**
 * @brief Initialize uart peripheral.
 * @note   UART_PARITY_NO_PARTY,UART_STOP_BITS_1,UART_WROD_LENGTH_8BIT;
 *         9600 baud rate.
 * @param No parameter.
 *
 * @return tuya_ble_status_t
 */
tuya_ble_status_t tuya_ble_common_uart_init(VOID_T);

/**
 * @brief Send data to uart.
 * @note
 *
 * @param p_data the send data buffer
 * @param len to send bytes size
 *
 * @return result
 */
tuya_ble_status_t tuya_ble_common_uart_send_data(CONST UINT8_T *p_data, UINT16_T len);

/**
 * \brief Create a new task and add it to the list of tasks that are ready to run.
 *
 * \param[out]  pp_handle  Used to pass back a handle by which the created task
 *                         can be referenced.
 * \param[in]   p_name     A descriptive name for the task.
 * \param[in]   p_routine  Pointer to task routine function that must be implemented
 *                         to never return.
 * \param[in]   p_param    Pointer parameter passed to the task routine function.
 * \param[in]   stack_size The size of the task stack that is specified as the number
 *                         of bytes.
 * \param[in]   priority   The priority at which the task should run. Higher priority
 *                         task has higher priority value.
 *
 * \return           The status of the task creation.
 * \retval TRUE      Task was created successfully and added to task ready list.
 * \retval FALSE     Task was failed to create.
 */
BOOL_T tuya_ble_os_task_create(VOID_T **pp_handle, CONST char *p_name, VOID_T(*p_routine)(VOID_T *), VOID_T *p_param, UINT16_T stack_size, UINT16_T priority);

/**
 * \brief Remove a task from RTOS's task management. The task being deleted will be removed
 * from RUNNING, READY or WAITING state.
 *
 * \param[in] p_handle  The handle of the task to be deleted.
 *
 * \return           The status of the task deletion.
 * \retval TRUE      Task was deleted successfully.
 * \retval FALSE     Task was failed to delete.
 */
BOOL_T tuya_ble_os_task_delete(VOID_T *p_handle);

/**
 * \brief Suspend the task. The suspended task will not be scheduled and never get
 * any microcontroller processing time.
 *
 * \param[in] p_handle  The handle of the task to be suspended.
 *
 * \return           The status of the task suspension.
 * \retval TRUE      Task was suspended successfully.
 * \retval FALSE     Task was failed to suspend.
 */
BOOL_T tuya_ble_os_task_suspend(VOID_T *p_handle);

/**
 * \brief Resume the suspended task.
 *
 * \param[in] p_handle  The handle of the task to be resumed.
 *
 * \return           The status of the task resume.
 * \retval TRUE      Task was resumed successfully.
 * \retval FALSE     Task was failed to resume.
 */
BOOL_T tuya_ble_os_task_resume(VOID_T *p_handle);

/**
 * \brief   Creates a message queue instance. This allocates the storage required by the
 *          new queue and passes back a handle for the queue.
 *
 * \param[out]  pp_handle  Used to pass back a handle by which the message queue
 *                         can be referenced.
 *
 * \param[in]   msg_num    The maximum number of items that the queue can contain.
 *
 * \param[in]   msg_size   The number of bytes each item in the queue will require. Items
 *                         are queued by copy, not by reference, so this is the number of
 *                         bytes that will be copied for each posted item. Each item on the
 *                         queue must be the same size.
 *
 * \return           The status of the message queue creation.
 * \retval TRUE      Message queue was created successfully.
 * \retval FALSE     Message queue was failed to create.
 */
BOOL_T tuya_ble_os_msg_queue_create(VOID_T **pp_handle, UINT32_T msg_num, UINT32_T msg_size);

/**
 * \brief   Delete the specified message queue, and free all the memory allocated for
 *          storing of items placed on the queue.
 *
 * \param[in]   p_handle   The handle to the message queue being deleted.
 *
 * \return           The status of the message queue deletion.
 * \retval TRUE      Message queue was deleted successfully.
 * \retval FALSE     Message queue was failed to delete.
 */
BOOL_T tuya_ble_os_msg_queue_delete(VOID_T *p_handle);

/**
 * \brief    Peek the number of items sent and resided on the message queue.
 *
 * \param[in]   p_handle   The handle to the message queue being peeked.
 * \param[out]  p_msg_num  Used to pass back the number of items residing on the message queue.
 *
 * \return           The status of the message queue peek.
 * \retval TRUE      Message queue was peeked successfully.
 * \retval FALSE     Message queue was failed to peek.
 */
BOOL_T tuya_ble_os_msg_queue_peek(VOID_T *p_handle, UINT32_T *p_msg_num);

/**
 * \brief   Send an item to the back of the specified message queue. The item is
 *          queued by copy, not by reference.
 *
 * \param[in]   p_handle The handle to the message queue on which the item is to be sent.
 * \param[in]   p_msg    Pointer to the item that is to be sent on the queue. The referenced
 *                       item rather than pointer itself will be copied on the queue.
 * \param[in]   wait_ms  The maximum amount of time in milliseconds that the task should
 *                       block waiting for the item to sent on the queue.
 * \arg \c 0           No blocking and return immediately.
 * \arg \c 0xFFFFFFFF  Block infinitely until the item sent.
 * \arg \c others      The timeout value in milliseconds.
 *
 * \return           The status of the message item sent.
 * \retval TRUE      Message item was sent successfully.
 * \retval FALSE     Message item was failed to send.
 */
BOOL_T tuya_ble_os_msg_queue_send(VOID_T *p_handle, VOID_T *p_msg, UINT32_T wait_ms);

/**
 * \brief   Receive an item from the specified message queue. The item is received by
 *          copy rather than by reference, so a buffer of adequate size must be provided.
 *
 * \param[in]   p_handle The handle to the message queue from which the item is to be received.
 * \param[out]  p_msg    Pointer to the buffer into which the received item will be copied.
 *                       item rather than pointer itself will be copied on the queue.
 * \param[in]   wait_ms  The maximum amount of time in milliseconds that the task should
 *                       block waiting for an item to be received from the queue.
 * \arg \c 0           No blocking and return immediately.
 * \arg \c 0xFFFFFFFF  Block infinitely until the item received.
 * \arg \c others      The timeout value in milliseconds.
 *
 * \return           The status of the message item received.
 * \retval TRUE      Message item was received successfully.
 * \retval FALSE     Message item was failed to receive.
 */
BOOL_T tuya_ble_os_msg_queue_recv(VOID_T *p_handle, VOID_T *p_msg, UINT32_T wait_ms);

/**
 * @brief If undefine TUYA_BLE_SELF_BUILT_TASK ,application should provide the task to sdk to process the event.
 *          SDK will use this port to send event to the task of provided by application.
 *
 * @param[in] evt: the message data point to be send.
 * @param[in] wait_ms: The maximum amount of time in milliseconds that the task should
 *                     block waiting for an item to be received from the queue.
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
BOOL_T tuya_ble_event_queue_send_port(tuya_ble_evt_param_t *evt, UINT32_T wait_ms);

/**
 * @brief 128 bit AES ECB encryption on speicified plaintext and keys
 *
 * @note   least significant octet of encrypted data corresponds to encypted[0]
 *
 * @param[in] key: keys to encrypt the plaintext
 * @param[in] input: specifed plain text to be encypted
 * @param[in] input_len: byte length of the data to be descrypted, must be multiples of 16
 * @param[out] output: output buffer to store encrypted data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
BOOL_T tuya_ble_aes128_ecb_encrypt(UINT8_T *key, UINT8_T *input, UINT16_T input_len, UINT8_T *output);

/**
 * @brief 128 bit AES ECB decryption on speicified encrypted data and keys
 *
 * @note   least significant octet of encrypted data corresponds to encypted[0]
 *
 * @param[in] key: keys to decrypt the data
 * @param[in] input: specifed encypted data to be decypted
 * @param[in] input_len: byte length of the data to be descrypted, must be multiples of 16
 * @param[out] output: output buffer to store plain data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
BOOL_T tuya_ble_aes128_ecb_decrypt(UINT8_T *key, UINT8_T *input, UINT16_T input_len, UINT8_T *output);

/**
 * @brief 128 bit AES CBC encryption on speicified plaintext and keys
 *
 * @note   least significant octet of encrypted data corresponds to encypted[0]
 *
 * @param[in] key: keys to encrypt the plaintext
 * @param[in] iv: initialization vector (IV) for CBC mode
 * @param[in] input: specifed plain text to be encypted
 * @param[in] input_len: byte length of the data to be descrypted, must be multiples of 16
 * @param[out] output: output buffer to store encrypted data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
BOOL_T tuya_ble_aes128_cbc_encrypt(UINT8_T *key, UINT8_T *iv, UINT8_T *input, UINT16_T input_len, UINT8_T *output);

/**
 * @brief 128 bit AES CBC descryption on speicified plaintext and keys
 *
 * @note   least significant octet of encrypted data corresponds to encypted[0]
 *
 * @param[in] key: keys to decrypt the data
 * @param[in] iv: initialization vector (IV) for CBC mode
 * @param[in] input: specifed encypted data to be decypted
 * @param[in] input_len: byte length of the data to be descrypted, must be multiples of 16
 * @param[out] output: output buffer to store plain data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
BOOL_T tuya_ble_aes128_cbc_decrypt(UINT8_T *key, UINT8_T *iv, UINT8_T *input, UINT16_T input_len, UINT8_T *output);

/**
 * @brief MD5 checksum
 *
 * @param[in] input: specifed plain text to be encypted
 * @param[in] input_len: byte length of the data to be encypted
 * @param[out] output: output buffer to store md5 result data,output data len is always 16
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
BOOL_T tuya_ble_md5_crypt(UINT8_T *input, UINT16_T input_len, UINT8_T *output);

/**
 * @brief MD5 loop crypt
 *
 * @param[in] ctx: mbedtls md5 context
 * @param[in] step: the step of crypt, for details see ENUM_MD5_CRYPE_LOOP_STEP
 * @param[in] input: specifed plain text to be encypted
 * @param[in] input_len: byte length of the data to be encypted
 * @param[out] output: output buffer to store md5 result data,output data len is always 16
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
BOOL_T tuya_ble_md5_crypt_loop(mbedtls_md5_context *ctx, ENUM_MD5_CRYPE_LOOP_STEP step, UINT8_T *input, UINT16_T input_len, UINT8_T *output);

/**
 * @brief SHA256 checksum
 *
 * @param[in] input: specifed plain text to be encypted
 * @param[in] input_len: byte length of the data to be encypted
 * @param[out] output: output buffer to store sha256 result data,output data len is always 32
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
BOOL_T tuya_ble_sha256_crypt(CONST UINT8_T *input, UINT16_T input_len, UINT8_T *output);

/**
 * @brief tuya_ble_ecc_sign_secp256r1
 *
 * @param[in] *p_sk: *p_sk
 * @param[in] p_data: p_data
 * @param[in] data_size: data_size
 * @param[in] *p_sig: *p_sig
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_ecc_sign_secp256r1(CONST UINT8_T *p_sk, CONST UINT8_T *p_data, UINT32_T data_size, UINT8_T *p_sig);

/**
 * @brief tuya_ble_ecc_verify_secp256r1
 *
 * @param[in] *p_pk: *p_pk
 * @param[in] p_hash: p_hash
 * @param[in] hash_size: hash_size
 * @param[in] *p_sig: *p_sig
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
tuya_ble_status_t tuya_ble_ecc_verify_secp256r1(CONST UINT8_T *p_pk, CONST UINT8_T *p_hash, UINT32_T hash_size, CONST UINT8_T *p_sig);

/**
 * @brief This function calculates the full generic HMAC
 *        on the input buffer with the provided key.
 *
 * @param[in] key: The HMAC secret key.
 * @param[in] key_len: The length of the HMAC secret key in Bytes.
 * @param[in] input: specifed plain text to be encypted
 * @param[in] input_len: byte length of the data to be encypted
 * @param[out] output: output buffer to store the result data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
BOOL_T tuya_ble_hmac_sha1_crypt(CONST UINT8_T *key, UINT32_T key_len, CONST UINT8_T *input, UINT32_T input_len, UINT8_T *output);

/**
 * @brief This function calculates the full generic HMAC
 *        on the input buffer with the provided key.
 *
 * @param[in] key: The HMAC secret key.
 * @param[in] key_len: The length of the HMAC secret key in Bytes.
 * @param[in] input: specifed plain text to be encypted
 * @param[in] input_len: byte length of the data to be encypted
 * @param[out] output: output buffer to store the result data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
BOOL_T tuya_ble_hmac_sha256_crypt(CONST UINT8_T *key, UINT32_T key_len, CONST UINT8_T *input, UINT32_T input_len, UINT8_T *output);

/**
 * \brief Allocate a memory block with required size.
 *
 * \param[in] size Required memory size.
 *
 * \return The address of the allocated memory block. If the address is NULL, the
 *         memory allocation failed.
 */
VOID_T *tuya_ble_port_malloc(UINT32_T size);

/**
 * \brief    Free a memory block that had been allocated.
 *
 * \param[in]   pv     The address of memory block being freed.
 *
 * \return     None.
 */
VOID_T tuya_ble_port_free(VOID_T *pv);


#ifdef __cplusplus
}
#endif

#endif /* __TUYA_BLE_PORT_H__ */

