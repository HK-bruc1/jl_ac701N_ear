/**
 * @file tkl_queue.c
 * @brief This is tkl_queue file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2024-2024 Tuya Inc. All Rights Reserved.
 *
 */

#include "tkl_queue.h"
// #include "FreeRTOS.h"
// #include "FreeRTOSConfig.h"
#include "task.h"
// #include "os_msg_q.h"
// #include "semphr.h"
// #include "event_groups.h"
// #include "os_msg_q.h"


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
#include "circular_buf.h"
struct __tkl_queue {
    void *queue;
    int msg_size;
    OS_SEM q_sem;
    cbuffer_t cbuf;
    unsigned char *q_buff;
};

OPERATE_RET tkl_queue_create_init(TKL_QUEUE_HANDLE *queue, INT_T msgsize, INT_T msgcount)
{
    // os_q_create(*queue, msgcount * msgsize);
    struct __tkl_queue *q = malloc(sizeof(struct __tkl_queue));
    /* printf("tkl_queue_init 0x%x\n", (u32)q); */
    q->q_buff = malloc(msgsize * msgcount);
    cbuf_init(&q->cbuf, q->q_buff, msgsize * msgcount);
    q->queue = q;
    q->msg_size = msgsize;
    os_sem_create(&q->q_sem, 0);
    *queue = q;
    return OPRT_OK;
}

OPERATE_RET tkl_queue_post(CONST TKL_QUEUE_HANDLE queue, VOID_T *data, UINT_T timeout)
{
    // return os_q_post(queue, data);
    struct __tkl_queue *q = (struct __tkl_queue *)queue;
    /* printf("tkl_queue_post 0x%x\n", (u32)q); */
    cbuf_write(&q->cbuf, data, q->msg_size);
    os_sem_post(&q->q_sem);
    // printf("tkl_queue_post\n");
    return TRUE;
}

OPERATE_RET tkl_queue_fetch(CONST TKL_QUEUE_HANDLE queue, VOID_T *msg, UINT_T timeout)
{
    // return os_q_pend(queue, timeout, msg);
    struct __tkl_queue *q = (struct __tkl_queue *)queue;
    /* printf("tkl_queue_fetch 0x%x\n", (u32)q); */
    os_sem_pend(&q->q_sem, 0);
    cbuf_read(&q->cbuf, msg, q->msg_size);
    return TRUE;
}

VOID_T tkl_queue_free(CONST TKL_QUEUE_HANDLE queue)
{
}


