#ifndef TEST1_THREAD_POOL_H
#define TEST1_THREAD_POOL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

typedef struct thread_task{
    void *(*process) (int arg);//这是一个函数指针，参数和返回值都是指针
    int arg;
    struct thread_task *next;
}task;

typedef struct{
    pthread_mutex_t queue_lock;//互斥锁
    pthread_cond_t queue_ready;//条件变量
    task *queue_head;//任务队列
    int shutdown;//销毁标志
    pthread_t *threadid;//线程ID数组
    int thread_num;//线程数量
    int cur_queue_size;//当前任务数量
}thread_pool;


int pool_add_task(thread_pool *pool, void *(*process) (int arg), int arg);//添加任务

void *thread_routine(void *arg);//线程执行函数

thread_pool *pool_init(int thread_num);//初始化线程池

int pool_destroy(thread_pool *pool);//销毁线程池

#endif //TEST1_THREAD_POOL_H
