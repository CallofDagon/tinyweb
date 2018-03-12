#include "epoll.h"

void epoll_add(int epollfd, int fd, int events){
    struct epoll_event event;
    event.events = events;
    event.data.fd = fd;
    int ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    //printf("添加套接字%d %d\n", fd, ret);
}

void handle_events(int epollfd, int listenfd, struct epoll_event* events, int events_num, thread_pool* pool){
    int i, connfd;
    for(i = 0; i < events_num; i++){
        int fd = events[i].data.fd;
        //printf("处理套接字%d\n", fd);
        if(fd == listenfd){
            //printf("处理监听\n");

            clientlen = sizeof(clientaddr);
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);//填写了客户端套接字
            getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);//linux自带函数，将套接字结构转换成字符串
            printf("Accepted connection from (%s, %s)\n",hostname, port);

            set_non_block(connfd);

            epoll_add(epollfd, connfd, (EPOLLIN | EPOLLET | EPOLLONESHOT));
        }else{
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN))){
                close(fd);
                continue;
            }

            pool_add_task(pool, do_request, fd);
            //printf("已加入请求队列\n");
        }
    }
}