//socket函数和RIO包的封装来自于CSAPP提供的源码
#include "thread_pool.h"
#include "csapp.h"
#include "epoll.h"
#include "http.h"
#include "non_block.h"

void *doit(int n){
    printf("%d\n",n);
    void *ret = NULL;
    return ret;
}

int main(){
    //初始配置
    int max_thread_num = 4;
    int listen_port = 1234;

    int listenfd, epollfd, events_num, connfd;
    struct epoll_event *events;//epoll得到的可用事件集合

    listenfd = Open_listenfd(listen_port);
    //printf("设置监听套接字为%d\n", listen_port);
    set_non_block(listenfd);//套接字设置为非阻塞

    thread_pool *pool = pool_init(max_thread_num);
    //printf("初始化线程池\n");

    epollfd = epoll_create1(0);
    events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * MAXEVENTS);

    //printf("初始化epoll %d\n", epollfd);

    epoll_add(epollfd, listenfd, (EPOLLIN | EPOLLET));//添加监听套接字

    while (1){
        events_num = epoll_wait(epollfd, events, MAXEVENTS, -1);
        //printf("获得事件%d个\n", events_num);

        handle_events(epollfd, listenfd, events, events_num, pool);
    }

}