/**@brief 仿照linux kfifo写的ring buffer
 *@atuher Anker  date:2013-12-18
* ring_buffer.h
 * */

#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <unistd.h>

#include "console_print.h"

//判断x是否是2的次方
#define is_power_of_2(x) ((x) != 0 && (((x) & ((x) - 1)) == 0))
//取a和b中最小值
#define min(a, b) (((a) < (b)) ? (a) : (b))

struct ring_buffer
{
    void      *buffer;     //缓冲区
    uint32_t     size;       //大小
    uint32_t     in;         //入口位置
    uint32_t       out;        //出口位置
    pthread_mutex_t *f_lock;    //互斥锁
};

extern struct ring_buffer *ring_buffer_init(void *buffer, uint32_t size, pthread_mutex_t *f_lock);

extern struct ring_buffer *ring_buffer_alloc(uint32_t size, pthread_mutex_t *f_lock);
extern struct ring_buffer *ring_buffer_config(uint32_t size, pthread_mutex_t *f_lock);


extern void ring_buffer_free(struct ring_buffer *ring_buf);

extern uint32_t __ring_buffer_get(struct ring_buffer *ring_buf, void *buffer, uint32_t size);

extern uint32_t __ring_buffer_put(struct ring_buffer *ring_buf, void *buffer, uint32_t size);

extern void __ring_buffer_reset(struct ring_buffer *ring_buf);
extern void ring_buffer_reset(struct ring_buffer *ring_buf);
extern uint32_t ring_buffer_put(struct ring_buffer *ring_buf, void *buffer, uint32_t size);
extern uint32_t ring_buffer_get(struct ring_buffer *ring_buf, void *buffer, uint32_t size);
extern uint32_t __ring_buffer_len(const struct ring_buffer *ring_buf);
extern uint32_t ring_buffer_len(const struct ring_buffer *ring_buf);

#endif