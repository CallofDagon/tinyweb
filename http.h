#ifndef TEST1_HTTP_H
#define TEST1_HTTP_H

#include "csapp.h"

void *do_request(int arg);

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

void read_requesthdrs(rio_t *rp);

int parse_uri(char *uri, char *filename, char *cgiargs);

void serve_static(int fd, char *filename, int filesize);

void get_filetype(char *filename, char *filetype);

void serve_dynamic(int fd, char *filename, char *cgiargs);

#endif //TEST1_HTTP_H
