## nginx编译选项说明

| author | update |
| ------ | ------ |
| perrynzhou@gmail.com | 2020/05/24 |


- 下载nginx源代码
```
$ wget http://nginx.org/download/nginx-1.18.0.tar.gz
$ tar zxvf nginx-1.18.0.tar.gz && cd nginx-1.18.0
$ configure 后面可以跟着很多编译选项
```

- configure 编译选项说明
  - --prefix=path,制定nginx的安装目录
  - --wth-stream,启动stream模块，让nginx能够工作在4层网络，直接处理TCP/UDP协议
  - --with-threads,启用nginx线程池子，让nginx使用线程池来处理数据，“卸载”阻塞操作
  - --with-pcre=path,--with-openssl=path,虽然linux内置了这些库，但是有时候版本太低会导致bug.所以这2个编译选项能够制定pcre和openssl源代码，从而获取最新版本
  - --with-{xxx-moudle},--without-{xxx-module},用这些配置来决定启用和禁用哪些自带的功能模块
  - --build=name，这可以在版本信息中制定公司名称。构建日期，源代码版本号等信息
    ```
    ./configure --build="${USER} build at `date +%Y%m%d`"
    ```
  - --with-debug,启用nginx的调试模式
  - --with-ld-opt=options,这个选项指定编译链接时候的额外参数，可以链接其他第三方库，例如使用hemalloc
    ```
    ./configure -with-ld-opt="-ljemmalloc"
    ```
  - --add-module=path,指定nginx第三方模块的源代码，这样nginx可以把第三方模块像自带模块一样进行编译，这个选项可以在configure后面出现任意多次，从而达到一次为nginx添加任意多的第三方模块
  - --add-dynamic-module=patj,这个功能和--add-module功能相同，--add-dynamic-module分离nginx主可执行程序和模块，可以在启动时候灵活组合动态加载，方便、nginx更新，使用"make modules"只编译变动模块，而nginx核心代码无须重新编译

- 基本配置
  - 进程管理相关配置(全局域配置)
    - worker_processes {number}|auto,设置nginx能够启动的worker的进程数目，通常nginx的worker进程和CPU核心数相等时能获取最佳的性能，这个配置一般需要和worker_cpu_affinity指令配合使用
    - worker_cpu_affinity auto | {cpumask},指定worker进程运行在某个cpu核心上。
    ```
    //配置2个？nginx工作进程
    worker_processes 2 
    //这2个进程分别绑定在0和1核心的CPU上
    worker_cpu_affinity 0001 00010
    ```
    - master_process on|of,决定使用启用nginx进程池，默认是on,如果设置为off,nginx启动不会建立master进程，只会启动一个worker进程来处理哦请求.
    - daemon on|off,决定是否启用daemon方式运行nginx,默认是on,如果设置为off方便我们调试
    - working_directory {path},配置nginx工作目录，仅仅存放nginx的coredump文件
    - working_shutdown_timeout {time},当使用"nginx -s quit"要求终止运行,nginx将等待的时间，然后强制关闭进程，这个选项可以很好解决系统里面出现大量”is shutting down"状态的nginx进程问题
    - access_log,配置http的访问请求的日志
    - error_log,配置服务器错误日志
    ```
    //level级别有debug、info、notice、warn、error、crit、alert、emerg
    error_log file|sterrr level;
    ```
    - worker_connections,配置worker进程的最大连接数，默认是1024

- http配置
  - http配置模板
  ```
  http{
    upstream{
      //配置上游服务器
    }
    server {
      listen 80;
      localtion {
        ....
      }
    }
    server {
      listen 81;
      location {
        .....
      }
    }
  }
  ```
  - 基本配置,http模块常用有4个指令来配置http基本功能
  ```
  //配置域名服务器，如果不配置nginx将无法正确解析域名的IP地址，也无法访问后端服务
  resolver address [valid=time] [ipv6=on|off]

  //设置keepalove超时时间，默认是75S，通常有利于客户端复用HTTP长连接，提供服务器性能;如果希望服务器发送完数据就主动断开就把它设置为0
  keepalive_timeout timeout

  //配置access访问日志，日志格式是有log_format决定；其中buffer和flush选项制定磁盘写缓冲区大小和刷新时间
  access log [format [buffer=size]   [flush=time] [if=condition]];
  log_format name format_string;

  access_log /var/logs/nginx/access.log buffer=8k flush= 1s;

  ```

  - location配置
  ```
  // = 表示uri必须完全匹配
  // ~ 表示大小写敏感匹配
  // ~* 表示大小写不敏感匹配
  // ^~ 表示前缀匹配
  location [= 或者 ~ 或者 ~* 或者^~或者@] uri {....}
  ```

  - file配置
  ```
  //经过sever和location后，我们需要确定URI的处理方式，如果nginxy用作静态服务器，那么文件访问配置就简单了,需要指定存放路径和文件名称即可


  //设置请求资源的根目录,将以path作为起始路径在磁盘上查找文件.当请求/image/1.jpg时候，就返回文件/var/data/1.jpg的内容
  location /image{
    root /var/data/
  }
  ```
  - 反向代理,它位于客户端和真正服务器之间，接受客户端请求转发给后端，然后把后端的处理结果返回给客户端
  
  ```
  upstream backend {
    keepalive 32;
    zone shared_upstream_storage 64k;
    server 127.0.0.1:80;
  }
  ```

  - 负载均衡,它决定了nginx访问后端服务器集群时策略，根据系统实际场景选择合适的算法可以把流量尽可能均匀分配到每一台后盾服务器，优化资源利用率，目前nignx内置负载均衡算法、
    - round robin,分为普通轮询和带权限的轮询算法，默认算法，将请求按顺序轮流地分配到后端服务器上，它均衡地对待后端的每一台服务器，而不关心服务器实际的连接数和当前的系统负载
    ```
    //普通轮询算法
    stream {
      upstream web1 {       
        server 10.0.0.1 weight=5;       
        server 10.0.0.2 weight=10; 
      }
    }
    //加权轮询算法
    stream {
      upstream web2 {       
        server 10.0.0.1;       
        server 10.0.0.2; 
      }
    }
    ```
   
   
    - ip_hash,使用IP地址的散列算法，通过管理发送方IP和目的地IP地址的散列，将来自同一发送方的分组(或发送至同一目的地的分组)统一转发到相同服务器的算法。当客户端有一系列业务需要处理而必须和一个服务器反复通信时，该算法能够以流(会话)为单位，保证来自相同客户端的通信能够一直在同一服务器中进行处理。
     ```
     stream {
      upstream web {      
        ip_hash;      
        server 10.0.0.1:8080;       
        server 10.0.0.2:8080; 
      }
     }
    ```
    - least_conn,带权重的最少活跃连接算法,选择负载最轻的服务器，最小连接数算法比较灵活和智能，由于后端服务器的配置不尽相同，对于请求的处理有快有慢，它是根据后端服务器当前的连接情况，动态地选取其中当前
    ```
    stream {
    upstream web2 {
        least_conn;
        server 10.0.0.2:8080 max_fails=3 fail_timeout=5s;
        server 10.0.0.21:8080 max_fails=3 fail_timeout=5s;
    }
    ```

- 代理转发,在使用upstreamp{}配置了上游集群服务器均衡算法后，就可以在location(http)或者server(stream)里面设置proxy_pass等指令把客户端的请求转发到后端，实现反向代理功能

```
//转发http请求的location
location /index {
  //转发原始请求的host头部
  proxy_set_header Host host;
  //转发到upstream块定义的服务器集群
  proxy_pass http://web2;
}

//转发grpc的location
location /pass_to_grpc {
  //转发到grpc后端服务集群
  grpc_pass grpc_backend;
}
```