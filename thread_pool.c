#include "thread_pool.h"

thread_pool *pool_init(int thread_num){
    thread_pool *pool = (thread_pool *)malloc(sizeof(thread_pool));

    pthread_mutex_init(&(pool->queue_lock), NULL);
    pthread_cond_init(&(pool->queue_ready), NULL);

    pool->queue_head = NULL;

    pool->thread_num = thread_num;
    pool->cur_queue_size = 0;
    pool->shutdown = 0;

    pool->threadid = (pthread_t *)malloc(thread_num * sizeof(pthread_t));
    int i;
    for(i = 0; i < thread_num; i++){
        pthread_create(&(pool->threadid[i]), NULL, thread_routine, pool);
    }
    return pool;
}

int pool_add_task(thread_pool *pool, void *(*process) (int arg), int arg){
    //printf("加入任务\n");
    task *newtask = (task *)malloc(sizeof(task));//新建任务
    newtask->process = process;
    newtask->arg = arg;
    newtask->next = NULL;
    pthread_mutex_lock(&(pool->queue_lock));//对线程池操作时要锁定
    task *member = pool->queue_head;
    if(member != NULL){
        while(member->next != NULL){
            member = member->next;
        }
        member->next = newtask;
    }else{
        pool->queue_head = newtask;
    }

    if(pool->queue_head == NULL){
        //printf("任务添加失败，链表为空\n");
        return -1;
    }

    pool->cur_queue_size++;
    //printf("现在任务数%d\n",pool->cur_queue_size);
    pthread_mutex_unlock(&(pool->queue_lock));//解锁，唤醒线程
    pthread_cond_signal(&(pool->queue_ready));
    return 0;
}

void *thread_routine(void *arg){
    //printf("启动线程 %lu\n", pthread_self());
    thread_pool *pool = (thread_pool *)arg;
    while(1){

        //对线程池操作先锁定
        pthread_mutex_lock(&(pool->queue_lock));

        //如果任务数为0且不准备销毁线程池，则让线程阻塞并解锁，唤醒后会重新锁定
        while(pool->cur_queue_size == 0 && !pool->shutdown){
            //printf("没有任务，线程 %lu 阻塞\n", pthread_self());
            pthread_cond_wait (&(pool->queue_ready), &(pool->queue_lock));
        }
        //销毁线程池，解锁，关闭线程
        if(pool->shutdown){
            pthread_mutex_unlock(&(pool->queue_lock));
            //printf("线程 %lu 关闭\n", pthread_self());
            pthread_exit(NULL);
        }

        //printf("线程 %lu 执行工作\n", pthread_self());
        pool->cur_queue_size--;
        task *cur_task = pool->queue_head;
        pool->queue_head = cur_task->next;
        pthread_mutex_unlock(&(pool->queue_lock));//取出任务后先解锁再工作，这样不会影响到其他线程
        //执行任务并释放空间
        (*(cur_task->process)) (cur_task->arg);
        free(cur_task);
        cur_task = NULL;
    }
}

int pool_destroy(thread_pool *pool){
    if(pool->shutdown){
        return -1;//已经在执行销毁了，防止重复操作
    }
    pool->shutdown = 1;

    //唤醒并关闭所有线程
    pthread_cond_broadcast(&(pool->queue_ready));
    //阻塞，等待线程关闭并回收
    int i;
    for(i = 0; i < pool->thread_num; i++){
        pthread_join(pool->threadid[i], NULL);
    }
    //释放空间，未完成的任务也丢弃
    free(pool->threadid);
    task *head = NULL;
    while(pool->queue_head != NULL){
        head = pool->queue_head;
        pool->queue_head = pool->queue_head->next;
        free(head);
    }
    pthread_mutex_destroy(&(pool->queue_lock));
    pthread_cond_destroy(&(pool->queue_ready));
    free(pool);
    pool = NULL;
    return 0;
}