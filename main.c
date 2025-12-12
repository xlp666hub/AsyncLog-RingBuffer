#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ring_buffer.h"

#define FILE_NAME "log.txt"

void log_push(RingBuffer *rb,const char *msg)
{
	Log log_entry = {0};

	snprintf(log_entry.data,LOG_MSG_SIZE,"%s",msg);//最推荐的函数
		
	rb_push(rb,&log_entry);
}

void *pthread_producer1(void *arg)
{
	RingBuffer *rb = (RingBuffer *)arg;

	for(int i=0;i<1000;i++)
	{
		char buf[64];

		snprintf(buf, sizeof(buf), "Producer1 code:%d\n", i);

		printf("Producer1 pushing:%s\n",buf);

		log_push(rb,buf);

		usleep(100);
	}
	return NULL;
}

void *pthread_producer2(void *arg)
{
	RingBuffer *rb = (RingBuffer *)arg;

	for(int i=0;i<1000;i++)
	{
		char buf[64];

		snprintf(buf, sizeof(buf), "Producer2 code:%d\n", i);

		printf("Producer2 pushing:%s\n",buf);

		log_push(rb,buf);

		usleep(100);
	}
	return NULL;
}

void *pthread_consumer(void *arg)
{
	RingBuffer *rb = (RingBuffer *)arg;
	Log entry;
	while(rb_pop(rb,&entry))
	{
		write(rb->file_fd,entry.data,strlen(entry.data));
	}
	printf("消费者线程：优雅退出\n");
	return NULL;
}

int main()
{
	RingBuffer rb;

	rb_init(&rb,10);

	rb.file_fd = open(FILE_NAME,O_RDWR|O_CREAT|O_APPEND,0644);
	if(rb.file_fd == -1)
	{
		perror("文件打开失败");
		return -1;
	}

	pthread_t consumer,producer1,producer2;
	pthread_create(&consumer,NULL,pthread_consumer,(void *)&rb);
	pthread_create(&producer1,NULL,pthread_producer1,(void *)&rb);
	pthread_create(&producer2,NULL,pthread_producer2,(void *)&rb);

	pthread_join(producer1,NULL);
	pthread_join(producer2,NULL);
	printf("所有生产者已经结束任务\n");

	rb_stop(&rb);
	pthread_join(consumer,NULL);
	printf("消费者已经退出\n");

	close(rb.file_fd);
	rb_destroy(&rb);
	return 0;
}

