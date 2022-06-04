#include "ring_buffer.h"

/***************************************************************
Copyright © Guangzhou ITS Communications Equipments Co.,Ltd. 
            2001-2029. All rights reserved.
文件名		: ring_buffer.c
作者	  	: 
版本	   	: V1.0
描述	   	: 环形队列实现代码
其他	   	: 无
日志	   	: 
***************************************************************/

//初始化缓冲区
struct ring_buffer* ring_buffer_init(void *buffer, uint32_t size, pthread_mutex_t *f_lock)
{
    assert(buffer);
    struct ring_buffer *ring_buf = NULL;
    #if 0
    if (!is_power_of_2(size))
    {
    fprintf(stderr,"size must be power of 2.\n");
        return ring_buf;
    }
    #endif
    ring_buf = (struct ring_buffer *)malloc(sizeof(struct ring_buffer));
    if (!ring_buf)
    {
        Console_Error("Failed to malloc memory,errno:%u,reason:%s",
            errno, strerror(errno));
        return ring_buf;
    }
    memset(ring_buf, 0, sizeof(struct ring_buffer));
    ring_buf->buffer = buffer;
    ring_buf->size = size;
    ring_buf->in = 0;
    ring_buf->out = 0;
    ring_buf->f_lock = f_lock;
    return ring_buf;
}
struct ring_buffer* ring_buffer_alloc(uint32_t size, pthread_mutex_t *f_lock)
{
    void *buffer = NULL;
	struct ring_buffer *ret = NULL;
    #if 0
    if (!is_power_of_2(size)) {
        fprintf(stderr,"size must be power of 2.\n");
        return ret;
	}
    #endif
    buffer = (void *)malloc(size);
    if (!buffer)
    {
        Console_Error("Failed to malloc memory,errno:%u,reason:%s",
            errno, strerror(errno));
        return ret;
    }
    ret = ring_buffer_init(buffer, size,  f_lock);
    if(!ret)
    {
        if(buffer)
        {
            free(buffer);
            buffer = NULL;
        }
    }

    return ret;
}
struct ring_buffer* ring_buffer_config(uint32_t size, pthread_mutex_t *f_lock)
{
    struct ring_buffer *ring_buf = NULL;
    if (pthread_mutex_init(f_lock, NULL) != 0)
    {
        Console_Error( "Failed init mutex,errno:%u,reason:%s\n",
            errno, strerror(errno));
        return ring_buf;
    }
    ring_buf = ring_buffer_alloc(size, f_lock);
    if (!ring_buf)
    {
        Console_Error( "Failed to init ring buffer.\n");
        return ring_buf;
    }
    return ring_buf;
}


//释放缓冲区
void ring_buffer_free(struct ring_buffer *ring_buf)
{
    if (ring_buf)
    {
        if (ring_buf->buffer)
        {
            free(ring_buf->buffer);
            ring_buf->buffer = NULL;
        }
        free(ring_buf);
        ring_buf = NULL;
    }
}


//从缓冲区中取数据
uint32_t __ring_buffer_get(struct ring_buffer *ring_buf, void * buffer, uint32_t size)
{
    assert(ring_buf || buffer);
    uint32_t len = 0;
    size  = min(size, ring_buf->in - ring_buf->out);
    uint32_t RingbufOutRemainder = (ring_buf->out) % (ring_buf->size);
    /* first get the data from fifo->out until the end of the buffer */
    len = min(size, ring_buf->size - RingbufOutRemainder);
    memcpy(buffer, ring_buf->buffer + RingbufOutRemainder, len);
    /* then get the rest (if any) from the beginning of the buffer */
    memcpy(buffer + len, ring_buf->buffer, size - len);
    ring_buf->out += size;
    return size;
}
//向缓冲区中存放数据
uint32_t __ring_buffer_put(struct ring_buffer *ring_buf, void *buffer, uint32_t size)
{
    assert(ring_buf || buffer);
    uint32_t len = 0;
    //其实不用下面两句判断环形队列满
    // if(__ring_buffer_len(ring_buf) >= ring_buf->size)
    //     return 0;//环形队列已满
    size = min(size, ring_buf->size - ring_buf->in + ring_buf->out);//size变成环形缓存区最大可以字节数。
    uint32_t RingbufInRemainder = (ring_buf->in) % (ring_buf->size);

    /* first put the data starting from fifo->in to buffer end */
    len  = min(size, ring_buf->size - RingbufInRemainder);
    memcpy(ring_buf->buffer + RingbufInRemainder, buffer, len);
    /* then put the rest (if any) at the beginning of the buffer */
    memcpy(ring_buf->buffer, buffer + len, size - len);
    ring_buf->in += size;
    return size;
}

void __ring_buffer_reset(struct ring_buffer *ring_buf)
{
	ring_buf->in = ring_buf->out = 0;
}

void ring_buffer_reset(struct ring_buffer *ring_buf)
{
    pthread_mutex_lock(ring_buf->f_lock);
    __ring_buffer_reset(ring_buf);
    pthread_mutex_unlock(ring_buf->f_lock);

}




//缓冲区的长度
uint32_t ring_buffer_put(struct ring_buffer *ring_buf, void *buffer, uint32_t size)
{
    uint32_t ret;
    pthread_mutex_lock(ring_buf->f_lock);
    ret = __ring_buffer_put(ring_buf, buffer, size);
    pthread_mutex_unlock(ring_buf->f_lock);
    return ret;
}


uint32_t ring_buffer_get(struct ring_buffer *ring_buf, void *buffer, uint32_t size)
{
    uint32_t ret;
    pthread_mutex_lock(ring_buf->f_lock);
    ret = __ring_buffer_get(ring_buf, buffer, size);
    //buffer中没有数据
    if (ring_buf->in == ring_buf->out)
        ring_buf->in = ring_buf->out = 0;
    pthread_mutex_unlock(ring_buf->f_lock);
    return ret;
}

uint32_t __ring_buffer_len(const struct ring_buffer *ring_buf)
{
    return (ring_buf->in - ring_buf->out);
}

uint32_t ring_buffer_len(const struct ring_buffer *ring_buf)
{
    uint32_t len = 0;
    pthread_mutex_lock(ring_buf->f_lock);
    len = __ring_buffer_len(ring_buf);
    pthread_mutex_unlock(ring_buf->f_lock);
    return len;
}
