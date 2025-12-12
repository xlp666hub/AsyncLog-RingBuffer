#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ring_buffer.h"

void rb_init(RingBuffer *rb,int capacity)
{
	rb->entries= (Log *)malloc(sizeof(Log) * capacity);//一次性申请一大块内存
	if((rb->entries) == NULL)
	{
		perror("malloc failed");
		exit(1);
	}

	//初始化状态
	rb->capacity = capacity;
	rb->is_running = true;
	rb->file_fd = 1;//stdout
	rb->head = 0;
	rb->tail = 0;
	rb->count = 0;

	//初始化锁和条件变量
	pthread_mutex_init(&(rb->mutex),NULL);
	pthread_cond_init(&(rb->cond_producer),NULL);
	pthread_cond_init(&(rb->cond_consumer),NULL);
}

void rb_push(RingBuffer *rb,const Log *entry)
{
	pthread_mutex_lock(&(rb->mutex));

	//如果缓存区满了，就先不要加
	while((rb->count) == (rb->capacity))
	{
		pthread_cond_wait(&(rb->cond_producer),&(rb->mutex));
	}

	memcpy(&(rb->entries[rb->tail]),entry,sizeof(Log));//新的日志放到队尾

	rb->tail = (rb->tail + 1) % rb->capacity;//移动队尾索引
	rb->count++;
	
	pthread_cond_signal(&(rb->cond_consumer));
	pthread_mutex_unlock(&(rb->mutex));
}

int rb_pop(RingBuffer *rb,Log *entry)
{
	pthread_mutex_lock(&(rb->mutex));

	//如果缓冲区为空，消费者睡觉
	while((rb->count == 0) && (rb->is_running == true))
	{
		pthread_cond_wait(&(rb->cond_consumer),&(rb->mutex));
	}

	if((rb->count == 0) && (rb->is_running == false))
	{
		pthread_mutex_unlock(&(rb->mutex));
		return 0;//消费者下班
	}

	memcpy(entry,&(rb->entries[rb->head]),sizeof(Log));

	rb->head = (rb->head + 1) % (rb->capacity);
	rb->count--;

	pthread_cond_signal(&(rb->cond_producer));
	pthread_mutex_unlock(&(rb->mutex));

	return 1;//正常返回
}

void rb_stop(RingBuffer *rb)
{
	pthread_mutex_lock(&(rb->mutex));

	rb->is_running = false;

	pthread_cond_broadcast(&(rb->cond_consumer));

	pthread_mutex_unlock(&(rb->mutex));
}

void rb_destroy(RingBuffer *rb)
{
	free(rb->entries);
	pthread_mutex_destroy(&(rb->mutex));
	pthread_cond_destroy(&(rb->cond_producer));
	pthread_cond_destroy(&(rb->cond_consumer));
}

