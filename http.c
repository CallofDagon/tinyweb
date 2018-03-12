#include "http.h"

void *do_request(int arg){
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;
    int fd = arg;
    void *ret = NULL;

    Rio_readinitb(&rio, fd);//重定向标准输出
    Rio_readlineb(&rio, buf, MAXLINE);
    //printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    //读取请求报头，拒绝GET以外的请求方法
    if(strcasecmp(method, "GET")){
        clienterror(fd, method, "501", "Not implemented", "not implement this method");
        return ret;
    }
    read_requesthdrs(&rio);//读取并忽略任何请求报头

    is_static = parse_uri(uri, filename, cgiargs);// 判断请求静态或动态内容，并解析出文件名和CGI参数
    if(stat(filename, &sbuf) < 0){//linux函数，判断文件是否存在，信息存入sbuf结构体中
        clienterror(fd, filename, "404", "Not found", "not find this file");
        close(fd);
        return ret;
    }
    if(is_static){
        if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)){//st_mode记录类型和权限，判断是普通文件且有权限读取
            clienterror(fd, filename, "403", "Forbidden", "not find this file");
            close(fd);
            return ret;
        }
        serve_static(fd, filename, sbuf.st_size);
    }else{
        if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)){
            clienterror(fd, filename, "403", "Forbidden", "not find this file");
            close(fd);
            return ret;
        }
        serve_dynamic(fd, filename, cgiargs);
    }
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg){
    char buf[MAXLINE], body[MAXBUF];
    //HTML写入字符串
    //响应主体
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n",body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Webserver</em>\r\n",body);

    //响应报头
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content_type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content_length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
    return ;
}

void read_requesthdrs(rio_t *rp){
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    while(strcmp(buf, "\r\n")){//直到读取到空文本行
        Rio_readlineb(rp, buf, MAXLINE);
        //printf("%s", buf);
    }
    return;
}//

int parse_uri(char *uri, char *filename, char *cgiargs){
    char *ptr;

    if(!strstr(uri, "cgi-bin")){//如果不包含动态内容
        strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);//相对文件名
        //如果/结尾则添加默认文件名
        if(uri[strlen(uri)-1] == '/')
            strcat(filename, "home.html");
        return 1;
    }else{
        ptr = index(uri, '?');//?的位置，分隔CGI参数和文件名
        if(ptr){
            strcpy(cgiargs, ptr+1);
            *ptr = '\0';
        } else
            strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        return 0;
    }
}

void serve_static(int fd, char *filename, int filesize){
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Webserver\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));
    //printf("Response headers:\n");
    //printf("%s", buf);

    srcfd = Open(filename, O_RDONLY, 0);
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);//将文件映射到一个srcp开始的私有只读虚拟内存空间
    Close(srcfd);//不需要描述符了，避免内存泄漏
    Rio_writen(fd, srcp, filesize);
    Munmap(srcp, filesize);//释放虚拟内存
}

void serve_dynamic(int fd, char *filename, char *cgiargs){
    char buf[MAXLINE], *emptylist[] = { NULL };
    //发送响应报头，剩余的部分由CGI程序发送
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Webserver\r\n");
    Rio_writen(fd, buf, strlen(buf));

    if(Fork() == 0){//子进程调用CGI
        setenv("QUERY_STRING", cgiargs, 1);//用CGI参数初始化环境变量
        Dup2(fd, STDOUT_FILENO);//重定向标准输出
        Execve(filename, emptylist, environ);//调用execve打开CGI程序
    }
    Wait(NULL);//父进程阻塞，等待回收子进程
};

void get_filetype(char *filename, char *filetype){
    if(strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if(strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if(strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if(strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}