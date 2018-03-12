#ifndef TEST1_EPOLL_H
#define TEST1_EPOLL_H

#include <sys/epoll.h>
#include "thread_pool.h"
#include "non_block.h"
#include "http.h"

#define MAXEVENTS 1024

char hostname[MAXLINE], port[MAXLINE];
socklen_t clientlen;
struct sockaddr_storage clientaddr;

void epoll_add(int epollfd, int fd, int events);

void handle_events(int epoll_fd, int listen_fd, struct epoll_event* events,
                      int events_num, thread_pool* pool);

#endif //TEST1_EPOLL_H
