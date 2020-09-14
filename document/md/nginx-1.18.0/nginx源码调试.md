### nginx 源码调试方法

#### nginx源码下载

```
wget http://nginx.org/download/nginx-1.19.2.tar.gz
```

#### 安装nginx依赖库(包括gcc/gdb)

```
yum install  -y gdb wget gcc gcc-c++   pcre pcre-devel  zlib zlib-devel openssl openssl-devel
```

#### 编译安装

```
tar zxvf nginx-1.18.0.tar.gz
// 执行CFLAGS="-ggdb3 -O0" ./configure 可以展开nginx中宏定义,这种定义nginx的二进制文件较大
cd nginx-1.18.0 && CFLAGS="-g -O0" ./configure
make -j2 && make install
```


#### 调试方法和技巧

- 修改对应的配置方式1
  
```
// vi /usr/local/nginx/conf/nginx.conf
worker_processes  1;
//每个worker进程都是fork出来的，然后成为了守护进程，不方便调试
//设置daemon off后，gdb进入work进程后使用set follow-fork-mode child，就直接进入工作进程
daemon off

```
- 修改对应配置方式2

```
//将监控进程和工作进程的逻辑全部合在一个进程里面
master_process off
```

- 开始调试

```
gdb --args /usr/local/nginx/sbin/nginx  -c /usr/local/nginx/conf/nginx.conf
(gdb)br main

//或者如下方式
gdb /usr/local/nginx/sbin/nginx  
(gdb)set args   -c /usr/local/nginx/conf/nginx.conf
```
- 查看宏定义
```
//编译时候使用CFLAGS="-ggdb3 -O0" ./configure &&make &&make install
(gdb) info macro NGX_OK
Defined at /home/perrynzhou/Source/nginx-1.18.0/src/core/ngx_core.h:37
  included at /home/perrynzhou/Source/nginx-1.18.0/src/core/nginx.c:9
#define NGX_OK 0
(gdb) macro expand NGX_OK
expands to: 0
```
#### 利用strace/pstack调试nginx

- 使用wget模拟客户端操作
```
[root@CentOS1 ~]# wget 127.0.0.1
--2020-08-22 17:45:29--  http://127.0.0.1/
Connecting to 127.0.0.1:80... connected.
HTTP request sent, awaiting response... 200 OK
Length: 612 [text/html]
Saving to: ‘index.html’

100%[================================================================================================================================>] 612         --.-K/s   in 0s      

2020-08-22 17:45:29 (121 MB/s) - ‘index.html’ saved [612/612]
```
- nginx中strace后的信息
```
[root@CentOS1 ~]# ps -ef|grep nginx
root     23346 20739  0 17:37 pts/0    00:00:00 gdb --args /usr/local/nginx/sbin/nginx -c /usr/local/nginx/conf/nginx.conf
root     23348 23346  0 17:37 pts/0    00:00:00 nginx: master process /usr/local/nginx/sbin/nginx -c /usr/local/nginx/conf/nginx.conf
nobody   23383 23348  0 17:43 pts/0    00:00:00 nginx: worker process
root     23392 17870  0 17:45 pts/1    00:00:00 grep --color=auto nginx
[root@CentOS1 ~]# strace -p 23383
strace: Process 23383 attached
epoll_wait(12, [{EPOLLIN, {u32=4160450576, u64=140737353838608}}], 512, -1) = 1
//接受客户端请求，wget过来的请求对应的文件描述符为7
accept4(10, {sa_family=AF_INET, sin_port=htons(60994), sin_addr=inet_addr("127.0.0.1")}, [112->16], SOCK_NONBLOCK) = 7
//把客户端请求文件描述符7添加到epoll中
epoll_ctl(12, EPOLL_CTL_ADD, 7, {EPOLLIN|EPOLLRDHUP|EPOLLET, {u32=4160451008, u64=140737353839040}}) = 0
epoll_wait(12, [{EPOLLIN, {u32=4160451008, u64=140737353839040}}], 512, 60000) = 1
//从客户端接受请求数据
recvfrom(7, "GET / HTTP/1.1\r\nUser-Agent: Wget"..., 1024, 0, NULL, NULL) = 107
stat("/usr/local/nginx/html/index.html", {st_mode=S_IFREG|0644, st_size=612, ...}) = 0
//打开请求内容
open("/usr/local/nginx/html/index.html", O_RDONLY|O_NONBLOCK) = 14
fstat(14, {st_mode=S_IFREG|0644, st_size=612, ...}) = 0
//往客户端写入响应
writev(7, [{iov_base="HTTP/1.1 200 OK\r\nServer: nginx/1"..., iov_len=238}], 1) = 238
//发送文件，这里采用了linux的0拷贝的技术
sendfile(7, 14, [0] => [612], 612)      = 612
write(8, "127.0.0.1 - - [22/Aug/2020:17:45"..., 96) = 96
close(14)                               = 0
setsockopt(7, SOL_TCP, TCP_NODELAY, [1], 4) = 0
epoll_wait(12, [{EPOLLIN|EPOLLRDHUP, {u32=4160451008, u64=140737353839040}}], 512, 65000) = 1
recvfrom(7, "", 1024, 0, NULL, NULL)    = 0
close(7)                                = 0
epoll_wait(12, 
```




#### nginx 请求处理规则

- 按照[ngx_modules.c](./document/nginx-1.18.0/ngx_modules.c)中定义的模块，当一个请求同时符合多个模块的处理规则时候，按照ngx_modules数组中的顺序选择最靠前的模块优先处理.

- 按照[ngx_modules.c](./document/nginx-1.18.0/ngx_modules.c)中定义的模块，针对http的过滤模块而言则是相反的，因为http框架在初始化时候，会在ngx_modules的数组中将过滤模块按先后顺序向filter list中添加，每次添加都是添加到表头，因此针对http模块，越是靠后的模块越是优先响应http.
