/**
 * @file tuya_ble_event.c
 * @brief This is tuya_ble_event file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "string.h"

#include "tal_util.h"

#include "tuya_ble_type.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_api.h"
#include "tuya_ble_port.h"
#include "tuya_ble_main.h"
#include "tuya_ble_secure.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_sdk_version.h"
#include "tuya_ble_event.h"
#include "tuya_ble_log.h"

#if (!TUYA_BLE_USE_OS)

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
/**@brief Macro for checking if a queue is full. */
#define TUYA_BLE_SCHED_QUEUE_FULL() tuya_sched_queue_full()

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
STATIC  UINT8_T        *m_queue_event_data;     /**< Array for holding the queue event data. */
STATIC volatile UINT8_T m_queue_start_index;    /**< Index of queue entry at the start of the queue. */
STATIC volatile UINT8_T m_queue_end_index;      /**< Index of queue entry at the end of the queue. */
STATIC UINT16_T         m_queue_event_size;     /**< Maximum event size in queue. */
STATIC UINT16_T         m_queue_size;           /**< Number of queue entries. */

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




STATIC __TUYA_BLE_INLINE UINT8_T next_index(UINT8_T index)
{
    return (index < m_queue_size) ? (index + 1) : 0;
}

STATIC __TUYA_BLE_INLINE UINT8_T tuya_sched_queue_full(VOID_T)
{
    UINT8_T tmp = m_queue_start_index;
    return next_index(m_queue_end_index) == tmp;
}

UINT32_T tuya_ble_sched_init(UINT16_T event_size, UINT16_T queue_size, VOID_T *p_event_buffer)
{
    // Check that buffer is correctly aligned
    if (!tal_util_is_word_aligned(p_event_buffer)) {
        TUYA_BLE_LOG_ERROR("tuya_ble_sched_init error");
        return 1;
    }

    // Initialize event scheduler
    m_queue_event_data    = &((UINT8_T *)p_event_buffer)[0];
    m_queue_end_index     = 0;
    m_queue_start_index   = 0;
    m_queue_event_size    = event_size;
    m_queue_size          = queue_size;

    return 0;
}

UINT16_T tuya_ble_sched_queue_size_get(VOID_T)
{
    return m_queue_size;
}

UINT16_T tuya_ble_sched_queue_space_get(VOID_T)
{
    UINT16_T start = m_queue_start_index;
    UINT16_T end   = m_queue_end_index;
    UINT16_T free_space = m_queue_size - ((end >= start) ?
                                          (end - start) : (m_queue_size + 1 - start + end));
    return free_space;
}

UINT16_T tuya_ble_sched_queue_events_get(VOID_T)
{
    UINT16_T start = m_queue_start_index;
    UINT16_T end   = m_queue_end_index;
    UINT16_T number_of_events;
    if (m_queue_size == 0) {
        number_of_events = 0;
    } else {
        number_of_events = ((end >= start) ? (end - start) : (m_queue_size + 1 - start + end));
    }
    return number_of_events;
}

STATIC tuya_ble_status_t tuya_ble_sched_event_put(VOID_T CONST *p_event_data, UINT16_T event_data_size)
{
    tuya_ble_status_t err_code;

    if (event_data_size <= m_queue_event_size) {
        UINT16_T event_index = 0xFFFF;

        tuya_ble_device_enter_critical();

        if (!TUYA_BLE_SCHED_QUEUE_FULL()) {
            event_index       = m_queue_end_index;
            m_queue_end_index = next_index(m_queue_end_index);

        }

        tuya_ble_device_exit_critical();

        if (event_index != 0xFFFF) {
            // NOTE: This can be done outside the critical region since the event consumer will
            //       always be called from the main loop, and will thus never interrupt this code.

            if ((p_event_data != NULL) && (event_data_size > 0)) {
                memcpy(&m_queue_event_data[event_index * m_queue_event_size],
                       p_event_data,
                       event_data_size);

            } else {

            }

            err_code = TUYA_BLE_SUCCESS;
        } else {
            err_code = TUYA_BLE_ERR_NO_MEM;
        }
    } else {
        err_code = TUYA_BLE_ERR_INVALID_LENGTH;
    }

    return err_code;
}

VOID_T tuya_sched_execute(VOID_T)
{
    STATIC tuya_ble_evt_param_t tuya_ble_evt;
    tuya_ble_evt_param_t *evt;

    evt = &tuya_ble_evt;

    UINT8_T end_ix = m_queue_end_index;

    while (m_queue_start_index != end_ix) {
        // Since this function is only called from the main loop, there is no
        // need for a critical region here, however a special care must be taken
        // regarding update of the queue start index (see the end of the loop).
        UINT16_T event_index = m_queue_start_index;

        VOID_T *p_event_data;

        p_event_data = &(m_queue_event_data[event_index * m_queue_event_size]);

        memcpy(evt, p_event_data, SIZEOF(tuya_ble_evt_param_t));

        //TUYA_BLE_LOG_DEBUG("TUYA_RECEIVE_EVT-0x%04x, start index-0x%04x, end index-0x%04x\n", evt->hdr.event, m_queue_start_index, m_queue_end_index);

        tuya_ble_event_process(evt);

        // Event processed, now it is safe to move the queue start index,
        // so the queue entry occupied by this event can be used to store
        // a next one.
        m_queue_start_index = next_index(m_queue_start_index);
    }

}

VOID_T tuya_ble_event_queue_init(VOID_T)
{
    if ((SIZEOF(tuya_ble_evt_param_t)) > TUYA_BLE_EVT_SIZE) {
        TUYA_BLE_LOG_ERROR("ERROR!!TUYA_BLE_EVT_SIZE is not enough!");
        return;
    }

    TUYA_BLE_SCHED_INIT(TUYA_BLE_EVT_SIZE, TUYA_BLE_EVT_MAX_NUM);
}

tuya_ble_status_t tuya_ble_message_send(tuya_ble_evt_param_t *evt)
{
    return tuya_ble_sched_event_put(evt, m_queue_event_size);
}

#endif

