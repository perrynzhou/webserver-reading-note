
## http处理全过程追踪

| 作者 | 时间 |QQ技术交流群 |
| ------ | ------ |------ |
| perrynzhou@gmail.com |2020/10/15 |中国开源存储技术交流群(672152841) |

### http处理涉及的每个阶段

  ```
  //http处理按照枚举定义的值大小，从小到大的执行每个阶段设置的回调函数来处理该阶段的逻辑
  typedef enum {
      //请求头读取完成之后的阶段
      NGX_HTTP_POST_READ_PHASE = 0,
      //server内请求地址重写阶段
      NGX_HTTP_SERVER_REWRITE_PHASE,
      //配置文件查找阶段
      NGX_HTTP_FIND_CONFIG_PHASE,
      //location内请求地址重写阶段
      NGX_HTTP_REWRITE_PHASE,
      //请求地址重写完成之后的阶段
      NGX_HTTP_POST_REWRITE_PHASE,
      //访问权限检查准备阶段
      NGX_HTTP_PREACCESS_PHASE,
      //访问权限检查阶段
      NGX_HTTP_ACCESS_PHASE,
      //访问权限检查完成之后的阶段
      NGX_HTTP_POST_ACCESS_PHASE,
      //内容产生准备阶段，目前只有ngx_http_mirror_module_ctx和ngx_http_try_files_module_ctx 
      NGX_HTTP_PRECONTENT_PHASE,
      //内容产生阶段
      NGX_HTTP_CONTENT_PHASE,
      //日志模块处理阶段
      NGX_HTTP_LOG_PHASE
  } ngx_http_phases;
  ```

### 每个阶段处理调试

#### NGX_HTTP_POST_READ_PHASE阶段调试

- 核心函数
  ```
  NGX_HTTP_POST_READ_PHASE-->ngx_http_realip_handler
  
  //这个模块需要在编译nginx时候添加 --with-http_ssl_module --with-http_realip_module模块支持
  //例如这样的编译选项:CFLAGS="-ggdb3 -O0" ./configure --with-http_stub_status_module --with-http_ssl_module --with-http_realip_module
  ```
- 模拟客户端请求

```
curl 172.25.78.25:80/demo
```
- nginx针对该模块配置
```
 server {
        listen       80;
        server_name  localhost;
        set_real_ip_from 127.0.0.1;
        real_ip_header X-My-IP;
       //当用户请求 xxx:/demo时候，在客户端响应内容中输出 echo 的内容
        location /demo {
           set $addr $remote_addr;
           echo "from: $addr";
        }
      }
```

#### NGX_HTTP_SERVER_REWRITE_PHASE阶段处理函数

  ```
  NGX_HTTP_SERVER_REWRITE_PHASE-->ngx_http_rewrite_handler
  								ngx_http_core_rewrite_phase
  ```
  
#### NGX_HTTP_FIND_CONFIG_PHASE阶段处理函数

  ```
  NGX_HTTP_FIND_CONFIG_PHAS-->ngx_http_core_find_config_phase
  ```

  

#### NGX_HTTP_REWRITE_PHASE阶段处理函数

  ```
  NGX_HTTP_REWRITE_PHASE-->ngx_http_rewrite_handler
  ```

  

#### NGX_HTTP_POST_REWRITE_PHASE阶段处理函数

  ```
  NGX_HTTP_POST_REWRITE_PHASE-->ngx_http_core_post_rewrite_phase
  ```

  

#### NGX_HTTP_PREACCESS_PHASE阶段处理函数
  
  ```
  NGX_HTTP_PREACCESS_PHASE-->ngx_http_realip_handler
  ```
  
#### NGX_HTTP_ACCESS_PHASE阶段处理函数

  ```
  NGX_HTTP_ACCESS_PHASE-->ngx_http_core_access_phase
  						ngx_http_access_handler
  						ngx_http_auth_basic_handler
  						ngx_http_auth_request_handler
  ```

  

#### NGX_HTTP_POST_ACCESS_PHASE阶段处理函数

  ```
  NGX_HTTP_POST_ACCESS_PHASE-->ngx_http_core_post_access_phase
  ```

  

#### NGX_HTTP_PRECONTENT_PHASE阶段处理函数

  ```
  NGX_HTTP_PRECONTENT_PHASE-->ngx_http_mirror_handler
  							ngx_http_try_files_handler
  ```

  

#### NGX_HTTP_CONTENT_PHASE阶段处理函数

  ```
  NGX_HTTP_CONTENT_PHASE-->ngx_http_core_content_phase
  						 ngx_http_autoindex_handler
  						 ngx_http_dav_handler
  						 ngx_http_gzip_static_handler
  						 ngx_http_index_handler
  						 ngx_http_random_index_handler
  						 ngx_http_static_handler
  ```

  

#### NGX_HTTP_LOG_PHASE阶段处理函数

  ```
  NGX_HTTP_LOG_PHASE-->ngx_http_log_handler
  ```
  
### 调试的准备环境
- 下载echo-nginx-module模块
```
git clone https://github.com/openresty/echo-nginx-module.git
```

- 重新编译nginx
```
//NGX_HTTP_POST_READ_PHASE阶段需要在编译时候enable http_realip_module模块
CFLAGS="-ggdb3 -O0" ./configure --add-module=../echo-nginx-module/  --with-http_stub_status_module --with-http_ssl_module --with-http_realip_module

```


### gdb信息


- NGX_HTTP_POST_READ_PHASE阶段调试
```
Breakpoint 1, ngx_http_realip_handler (r=0x1fa0ea0) at src/http/modules/ngx_http_realip_module.c:144
144         rlcf = ngx_http_get_module_loc_conf(r, ngx_http_realip_module);
(gdb) bt
#0  ngx_http_realip_handler (r=0x1fa0ea0) at src/http/modules/ngx_http_realip_module.c:144
#1  0x0000000000466df9 in ngx_http_core_generic_phase (r=0x1fa0ea0, ph=0x1fb7aa0) at src/http/ngx_http_core_module.c:890
#2  0x0000000000466d98 in ngx_http_core_run_phases (r=0x1fa0ea0) at src/http/ngx_http_core_module.c:868
#3  0x0000000000466d06 in ngx_http_handler (r=0x1fa0ea0) at src/http/ngx_http_core_module.c:851
#4  0x000000000047576f in ngx_http_process_request (r=0x1fa0ea0) at src/http/ngx_http_request.c:2078
#5  0x00000000004740d4 in ngx_http_process_request_headers (rev=0x1fbeb00) at src/http/ngx_http_request.c:1480
#6  0x000000000047364c in ngx_http_process_request_line (rev=0x1fbeb00) at src/http/ngx_http_request.c:1151
#7  0x0000000000472208 in ngx_http_wait_request_handler (rev=0x1fbeb00) at src/http/ngx_http_request.c:500
#8  0x0000000000454aff in ngx_epoll_process_events (cycle=0x1f92240, timer=60000, flags=1) at src/event/modules/ngx_epoll_module.c:901
#9  0x00000000004455fa in ngx_process_events_and_timers (cycle=0x1f92240) at src/event/ngx_event.c:247
#10 0x0000000000452726 in ngx_worker_process_cycle (cycle=0x1f92240, data=0x0) at src/os/unix/ngx_process_cycle.c:740
#11 0x000000000044f6e9 in ngx_spawn_process (cycle=0x1f92240, proc=0x452678 <ngx_worker_process_cycle>, data=0x0, name=0x4effd7 "worker process", respawn=-3)
    at src/os/unix/ngx_process.c:199
#12 0x0000000000451957 in ngx_start_worker_processes (cycle=0x1f92240, n=1, type=-3) at src/os/unix/ngx_process_cycle.c:349
#13 0x000000000045117d in ngx_master_process_cycle (cycle=0x1f92240) at src/os/unix/ngx_process_cycle.c:130
#14 0x0000000000411cc6 in main (argc=3, argv=0x7ffe0cdfe0f8) at src/core/nginx.c:382
```