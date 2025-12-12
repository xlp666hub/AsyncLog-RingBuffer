#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <pthread.h>
#include <stdbool.h>

#define LOG_MSG_SIZE 128      //每条日志最大128字节

//每条日志用一个结构体存储
typedef struct{
	char data[LOG_MSG_SIZE];
}Log;

//环形缓存区管理器
typedef struct{
	Log *entries;  //指向malloc的大数组

	int capacity;  //总容量
	int count;     //当前元素数量

	//队列数据结构，从头部取，新内容加到队尾
	int head;      //读指针（大数组中元素的索引）
	int tail;      //写指针（大数组中元素的索引）

	//用两个条件变量避免惊群效应
	pthread_mutex_t mutex;
	pthread_cond_t cond_producer;//缓冲区满时，生产者要等待
	pthread_cond_t cond_consumer;//缓冲区为空时，消费者要等待

	bool is_running;

	int file_fd;//存放日志信息的文件的文件描述符
}RingBuffer;


void rb_init(RingBuffer *rb,int capacity);
void rb_push(RingBuffer *rb,const Log *entry);
int  rb_pop(RingBuffer *rb,Log *entry);
void rb_stop(RingBuffer *rb);
void rb_destroy(RingBuffer *rb);

#endif
