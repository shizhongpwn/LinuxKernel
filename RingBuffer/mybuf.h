#ifndef _RINGBUF_HEADER_
#define _RINGBUF_HEADER_
#ifdef _cplusplus

extern "C" {
#endif 

typedef struct _ringbuf_ ringbuf;

ringbuf* ringbuf_create(unsigned int size);

int ringbuf_push(ringbuf* pbuf, const void* data, int size); // 向缓冲区放入数据

int ringbuf_pop(ringbuf* pbuf, void* data, int size); // 从缓冲区中取出数据

void* ringbuf_top(ringbuf* pbuf, void * data, int size); //从缓冲中取数据 但是不修改缓冲的大小

int ringbuf_head_move(ringbuf * pbuf, int size); // 移动缓冲的头指针
 
int ringbuf_head_seek(ringbuf* pbuf,int size);  //

ringbuf* ringbuf_realloc(ringbuf* pbuf,int size); // 重新申请缓冲区的大小，size 的大小是的最终大小

int ringbuf_get_size(ringbuf* pbuf); // 获得buf的大小

#if 1

void* ringbuf_get_head(ringbuf* pbuf); // 获得缓冲区的开始

void* ringbuf_get_tail(ringbuf* pbuf); //获得缓冲区的尾部

void ringbuf_destroy(ringbuf* pbuf); // 销毁缓冲区

#endif

#ifdef __cplusplus
}
#endif

#endif


