#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "mybuf.h"

#define RINGBUF_START(pbuf) ((unsigned char*)(pbuf->buf_start))

#define RINGBUF_END(pbuf) (pbuf->buf_start + pbuf->buf_size - 1)

#define RINGBUF_HEAD(pbuf) (pbuf->buf_head)

#define RINGBUF_TAIL(pbuf) (pbuf->buf_tail)

#define RINGBUF_SIZE(pbuf) (pbuf->buf_size)

#define RINGBUF_USED(pbuf) (RINGBUF_HEAD(pbuf) <= RINGBUF_TAIL(pbuf) ? RINGBUF_TAIL(pbuf) - RINGBUF_HEAD(pbuf) : RINGBUF_SIZE(pbuf) + RINGBUF_TAIL(pbuf) - RINGBUF_HEAD(pbuf))

#define RINGBUF_FREE(pbuf) (RINGBUF_HEAD(pbuf) <= RINGBUF_TAIL(pbuf) ? RINGBUF_SIZE(pbuf) + RINGBUF_HEAD(pbuf) - RINGBUF_TAIL(pbuf) : RINGBUF_HEAD(pbuf) - RINGBUF_TAIL(pbuf))

struct _ringbuf_ {
    pthread_mutex_t mutex;
    unsigned int buf_size;
    unsigned char * buf_head;
    unsigned char * buf_tail;
    unsigned char buf_start[1];
};

ringbuf* ringbuf_create(unsigned int size) {
    if (size > 0) {
        ringbuf *buf = malloc(sizeof(ringbuf) + size);
        if (buf == NULL) {
            return NULL;
        }
        buf->buf_size = size;
        buf->buf_head = buf->buf_tail = buf->buf_start;
        pthread_mutex_init(&buf->mutex, NULL);
        return buf;
    }
    return NULL;
}

int ringbuf_use(ringbuf* pbuf) {
    int buf_used;
    pthread_mutex_lock(&pbuf->mutex);
    if (pbuf->buf_head <= pbuf->buf_tail) 
    {
        buf_used = pbuf->buf_tail - pbuf->buf_head;
    }
    else {
        buf_used = pbuf->buf_size + pbuf->buf_tail - pbuf->buf_head;
    }
    pthread_mutex_unlock(&pbuf->mutex);
    return buf_used;
}

int ringbuf_free(ringbuf* pbuf) {
    int buf_free;
    pthread_mutex_lock(&pbuf->mutex);
    if (pbuf->buf_head <= pbuf->buf_tail) {
        buf_free = pbuf->buf_size + pbuf->buf_head - pbuf->buf_tail;
    } else {
        buf_free = pbuf->buf_head - pbuf->buf_tail;
    }
    pthread_mutex_unlock(&pbuf->mutex);
    return buf_free;
}

int ringbuf_push(ringbuf* pbuf, const void* data, int size) {
    pthread_mutex_lock(&pbuf->mutex);
    if (pbuf && data && size > 0 && size < (int)RINGBUF_FREE(pbuf)) {
        unsigned int tail_free = RINGBUF_END(pbuf) - RINGBUF_TAIL(pbuf) + 1;
        if (tail_free >= size) {
            memcpy(RINGBUF_TAIL(pbuf), (char *)data, size);
            RINGBUF_TAIL(pbuf) += size;
            if (RINGBUF_TAIL(pbuf) > RINGBUF_END(pbuf)) {
                RINGBUF_TAIL(pbuf) = RINGBUF_START(pbuf);
            } 
        }
        else {
                memcpy(RINGBUF_TAIL(pbuf),(char *)data, tail_free);
                unsigned int lev = size - tail_free;
                memcpy(RINGBUF_START(pbuf), (char *)data + tail_free, lev);
                RINGBUF_TAIL(pbuf) = RINGBUF_START(pbuf) + lev;
        }
        pthread_mutex_unlock(&pbuf->mutex);
        return 1;
    }
    pthread_mutex_lock(&pbuf->mutex);
    return 0; // failed!
}
int ringbuf_pop(ringbuf* pbuf,void* data,int size)
{
	pthread_mutex_lock(&pbuf->mutex);
	if(pbuf && data && size > 0 && size < (int)RINGBUF_USED(pbuf) )
	{
		unsigned int head_used = RINGBUF_END(pbuf) - RINGBUF_HEAD(pbuf) + 1;
		if(head_used >= size)
		{
			memcpy((char*)data,RINGBUF_HEAD(pbuf),size);
			RINGBUF_HEAD(pbuf) += size;
			if(RINGBUF_HEAD(pbuf) > RINGBUF_END(pbuf))
				RINGBUF_HEAD(pbuf) = RINGBUF_START(pbuf);
		}
		else
		{
			memcpy((char*)data,RINGBUF_HEAD(pbuf),head_used);
			unsigned int lev = size - head_used;
			memcpy((char*)data+head_used,RINGBUF_START(pbuf),lev);
			RINGBUF_HEAD(pbuf) = RINGBUF_START(pbuf) + lev;
		}
		pthread_mutex_unlock(&pbuf->mutex);
		return 1;// succeed
	}
	pthread_mutex_unlock(&pbuf->mutex);
	return 0; // failed
 
}
 
void ringbuf_destroy(ringbuf* pbuf)
{
	free(pbuf);
}
 
void ringbuf_clear(ringbuf* pbuf)
{
	pthread_mutex_lock(&pbuf->mutex);
	RINGBUF_HEAD(pbuf) = RINGBUF_TAIL(pbuf) = RINGBUF_START(pbuf);
	pthread_mutex_unlock(&pbuf->mutex);
}
 
void* ringbuf_top(ringbuf* pbuf,void* data,int size)
{
	pthread_mutex_lock(&pbuf->mutex);
	if( pbuf && data && size > 0 && size <= (int)RINGBUF_USED(pbuf))
	{
		unsigned int head_used = RINGBUF_END(pbuf) - RINGBUF_HEAD(pbuf)+ 1;
		if(head_used >= size)
		{
			data = RINGBUF_HEAD(pbuf);
		}
		else
		{
			memcpy((char*)data,RINGBUF_HEAD(pbuf),head_used);
			unsigned int lev = size - head_used;
			memcpy((char*)data+head_used,RINGBUF_START(pbuf),lev);
		}
		pthread_mutex_unlock(&pbuf->mutex);
		return data;
	}
	pthread_mutex_unlock(&pbuf->mutex);
	return NULL;
}
 
int  ringbuf_head_move(ringbuf* pbuf,int size)
{
 
	pthread_mutex_lock(&pbuf->mutex);
	if(pbuf && size > 0 && size <= (int)RINGBUF_USED(pbuf))
	{
		unsigned int head_used = RINGBUF_END(pbuf)-RINGBUF_HEAD(pbuf)+ 1;
		if( head_used >= size)
		{
			RINGBUF_HEAD(pbuf) += size;
			if(RINGBUF_HEAD(pbuf) > RINGBUF_END(pbuf))
				RINGBUF_HEAD(pbuf) = RINGBUF_HEAD(pbuf);
		}
		else
		{
			unsigned int lev = size - head_used;
			RINGBUF_HEAD(pbuf) = RINGBUF_START(pbuf) + lev;
		}
		pthread_mutex_unlock(&pbuf->mutex);
		return 1;
	}
	pthread_mutex_unlock(&pbuf->mutex);
	return 0;
}
 
int ringbuf_head_seek(ringbuf* pbuf,int  offset)
{
	if(offset >= 0)
	{
 
		pthread_mutex_lock(&pbuf->mutex);
		unsigned int head_used = RINGBUF_END(pbuf) - RINGBUF_HEAD(pbuf)+1;
		if( offset <= head_used )
		{
			RINGBUF_HEAD(pbuf) += offset;
		}
		else
		{
			unsigned int lev = offset - head_used;
			RINGBUF_HEAD(pbuf) = RINGBUF_START(pbuf) + lev;
		}
		pthread_mutex_unlock(&pbuf->mutex);
		return 1;
	}
	else
	{
 
		pthread_mutex_lock(&pbuf->mutex);
		unsigned int head_free = RINGBUF_HEAD(pbuf) - RINGBUF_START(pbuf) + 1;
		if( -offset <= head_free )
		{
			RINGBUF_HEAD(pbuf) += offset;
		}
		else
		{
			unsigned int lev = -(offset + head_free);
			RINGBUF_HEAD(pbuf) = RINGBUF_END(pbuf) - lev;
		}
		pthread_mutex_unlock(&pbuf->mutex);
		return 1;
	}
	return 0;
}
 
int ringbuf_head_free(ringbuf* pbuf)
{
	unsigned int ret;
	pthread_mutex_lock(&pbuf->mutex);
	ret = RINGBUF_HEAD(pbuf) - RINGBUF_START(pbuf);
	pthread_mutex_unlock(&pbuf->mutex);
	return ret;
}
int ringbuf_tail_seek(ringbuf* pbuf,int offset)
{
	if(offset >= 0)
	{
 
		pthread_mutex_lock(&pbuf->mutex);
		unsigned int tail_free = RINGBUF_END(pbuf) - RINGBUF_TAIL(pbuf) + 1;
		if(offset <= tail_free)
		{
			RINGBUF_TAIL(pbuf) += offset;
		}
		else
		{
			unsigned int lev = offset - tail_free;
			RINGBUF_TAIL(pbuf) = RINGBUF_START(pbuf) + lev;
		}
		pthread_mutex_unlock(&pbuf->mutex);
		return 1;
	}
	else
	{
 
		pthread_mutex_lock(&pbuf->mutex);
		unsigned int tail_free = RINGBUF_TAIL(pbuf) - RINGBUF_START(pbuf) + 1;
		if(-offset <= tail_free)
		{
			RINGBUF_TAIL(pbuf) += offset;
		}
		else
		{
			unsigned int lev = -(offset + tail_free);
			RINGBUF_TAIL(pbuf) = RINGBUF_END(pbuf) - lev;
		}
		pthread_mutex_unlock(&pbuf->mutex);
		return 1;
	}
	return 0;
 
}
int ringbuf_rewind(ringbuf* pbuf)
{
 
	unsigned char *start_tail = NULL;
	unsigned int head_free = ringbuf_head_free(pbuf);
	if(head_free == 0)
	{
		return 0;
	}
 
	pthread_mutex_lock(&pbuf->mutex);
	if( RINGBUF_HEAD(pbuf) <= RINGBUF_TAIL(pbuf) )
	{
		memmove(RINGBUF_START(pbuf), RINGBUF_HEAD(pbuf), RINGBUF_USED(pbuf));
 
	}
	else
	{
		unsigned int tail_used = RINGBUF_TAIL(pbuf) - RINGBUF_HEAD(pbuf) +1;
		unsigned int head_used = RINGBUF_END(pbuf) - RINGBUF_HEAD(pbuf) + 1;
		start_tail = malloc(sizeof(char)*tail_used + 1);
		if(start_tail == NULL)
		{
			return ;
		}
		memcpy(start_tail,RINGBUF_START(pbuf),tail_used);
		start_tail[tail_used] = '\0';
		memmove(RINGBUF_START(pbuf), RINGBUF_HEAD(pbuf), head_used);
		memmove(RINGBUF_START(pbuf)+head_used+1,start_tail,tail_used);
		RINGBUF_HEAD(pbuf) = RINGBUF_START(pbuf);
		RINGBUF_TAIL(pbuf) = RINGBUF_START(pbuf) + RINGBUF_USED(pbuf);
	}
	pthread_mutex_unlock(&pbuf->mutex);
	return 1;
}
 
// 重新申请ringbuf 这个大小是新生成的大小 而不是扩展的大小
ringbuf* ringbuf_realloc(ringbuf* pbuf,int size)
{
	pthread_mutex_lock(&pbuf->mutex);
	unsigned int init_size = RINGBUF_SIZE(pbuf);
	if( init_size == size)
	{
		pthread_mutex_unlock(&pbuf->mutex);
		return pbuf;
	}
	unsigned int tail_pos = RINGBUF_TAIL(pbuf) - RINGBUF_START(pbuf);
	if( tail_pos > size)
	{
		ringbuf_rewind(pbuf);
		tail_pos = RINGBUF_TAIL(pbuf) - RINGBUF_START(pbuf);
		size = tail_pos > size ? tail_pos : size;
	}
	unsigned int head_pos = RINGBUF_HEAD(pbuf) - RINGBUF_START(pbuf);
	ringbuf* new_buf = realloc(pbuf ,sizeof(ringbuf) + size);
	RINGBUF_SIZE(new_buf) = size;
	RINGBUF_HEAD(new_buf) = RINGBUF_START(new_buf) + head_pos;
	RINGBUF_TAIL(new_buf) = RINGBUF_START(new_buf) + tail_pos;
	pthread_mutex_unlock(&pbuf->mutex);
	return new_buf;
}
 
int  ringbuf_tail_free(ringbuf* pbuf)
{
	int ret;
	pthread_mutex_lock(&pbuf->mutex);
	ret = RINGBUF_END(pbuf) - RINGBUF_TAIL(pbuf) + 1;
	pthread_mutex_unlock(&pbuf->mutex);
	return ret;
}
 
void* ringbuf_get_head(ringbuf* pbuf)
{
	void* ret;
 
	pthread_mutex_lock(&pbuf->mutex);
	ret = RINGBUF_HEAD(pbuf);
	pthread_mutex_unlock(&pbuf->mutex);
	return ret;
}
void* ringbuf_get_tail(ringbuf* pbuf)
{
	void* ret;
	pthread_mutex_lock(&pbuf->mutex);
	ret = RINGBUF_TAIL(pbuf);
	pthread_mutex_unlock(&pbuf->mutex);
	return ret;
}
 
int ringbuf_get_size(ringbuf* pbuf)
{	
	int ret;
	pthread_mutex_lock(&pbuf->mutex);
	ret = RINGBUF_SIZE(pbuf);
	pthread_mutex_unlock(&pbuf->mutex);
	return ret;
}