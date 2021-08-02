#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "mybuf.h"
 
#define BUF_SIZE (1024)
void* thread_put(void* args);
void* thread_get(void* args);
pthread_mutex_t mutex;
pthread_cond_t cond;
int main()
{
	ringbuf* buf = NULL;
	char tm[64];
	memset(tm,0x00,sizeof(tm));
	pthread_t pid[2];
	int i,ret;
	buf= ringbuf_create(BUF_SIZE); // 分配环形缓冲区,缓冲区自身带线程锁
	if (buf == NULL)
	{
		printf("Error creating ringbuf\n");
		return -1;
	}
	pthread_cond_init(&cond,NULL); // 初始化默认条件下的条件变量（另一个参数为NULL，否则该条件变量的特性将由参数中的属性值来决定），
	pthread_mutex_init(&mutex,NULL);
//	printf("buf start is %d end is %d\r\n",ringbuf_get_head(buf),ringbuf_get_tail(buf));
	printf("buf start is %d \r\n",ringbuf_get_head(buf));
 
#if	1 
	ret = pthread_create(&pid[i],NULL,thread_get,(void*)buf);
	if(ret!= 0)
	{
		printf("can not open \r\n");
	}
	ret = pthread_create(&pid[i],NULL,thread_put,(void*)buf);
	if(ret!= 0)
	{
		printf("can not open \r\n");
	}
	while(1)
	{
		sleep(4);
		printf("In \r\n");
		
		}
 
#endif
#if 0 
	strcpy(tm,"dsfaaa");
	ringbuf_push(buf,tm,strlen(tm));
	printf("buf size is %d\r\n",ringbuf_get_size(buf));
	
	memset(tm,0x00,sizeof(tm));
	ringbuf_pop(buf,tm,4);
	printf("from buf is %s\r\n",tm);
	ringbuf_destroy(buf);
#endif
	return 0;
 
}
void *thread_get(void* args)
{
	ringbuf *buf=(ringbuf*)args;
//	char *buf=(char*)args;
	char temp[100];
	memset(temp,0x00,sizeof(temp));
//	strcpy(temp,"wangguoxi");
	for(;;)
	{
		memset(temp,0x00,sizeof(temp));
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&cond,&mutex);
		ringbuf_pop(buf,temp,90);
//		strncpy(temp,buf,10);
		printf("start %d end is %d get is  %s \r\n",ringbuf_get_head(buf),ringbuf_get_tail(buf),temp);
		memset(buf,0x00,sizeof(buf));
		pthread_mutex_unlock(&mutex);
	}
}
 
 
void *thread_put(void* args)
{
	ringbuf *buf=(ringbuf*)args;
//	char *buf=(char*)args;
	int i;
	char temp[100];
	memset(temp,0x00,sizeof(temp));
	for(i=0;i<10;i++)
	strcpy(temp+i*9,"wangguoxi");
	printf("%s\r\n",temp);
	sleep(5);
	for(;;)
	{
		if(temp!=NULL)
		{
 
			pthread_mutex_lock(&mutex);
			ringbuf_push(buf,temp,strlen(temp));
			printf("head is %d\r\n",ringbuf_get_head(buf));
		//	strncpy(buf,temp,strlen(temp));
			pthread_mutex_unlock(&mutex);
			pthread_cond_signal(&cond); // 肯定是先写入然后再读取啦。
//			sleep(1);
		usleep(1);
		}
 
	}
}