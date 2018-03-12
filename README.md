# tinyweb
编译方式
gcc -g non_block.c thread_pool.c epoll.c http.c csapp.c main.c -o web -lpthread

#sudo ./web 运行
默认端口 80， 线程数 10，可在main.c中修改
