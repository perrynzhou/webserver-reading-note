
##  nginx框架功能介绍

| author | update |
| ------ | ------ |
| perrynzhou@gmail.com | 2020/11/03 |

### 请求处理的流程概览：

- 监听端口，接受客户端的连接请求

- 读取请求头，包括请求行和请求头

- 读取或者丢弃掉请求体

- 生成和发送响应头

- 生成和发送响应体
  
### 请求处理http基本流程如下:

- 监听端口，设置回调函数ngx_http_init_connection()

- 接受客户端连接，调用nginx_http_wait_request_handler()函数

- 调用ngx_http_create_request创建请求对象ngx_http_request_t,初始化里面的内存池、起始时间、配置数据结构，应用于整个http请求的生命周期
  - ngx_http_request_t结构中ctx非常重要，由于nginx是异步和多阶段处理的，很多时候http模块不节能在一次调用中完成请求处理，所以使用ctx字段来保存中间结果，当nginx再次调用模块时候可从ctx中取出上次的数据继续运行，这实际是一种checkpoint的机制。nginx通过把请求和ctx绑定在一起，就完美处理这种处理不同状态的请求
- 接受数据，调用ngx_http_process_request_line解析请求行

- 请求行接受完毕，调用ngx_http_process_request_headers()解析请求头

- 请求头接受完毕后，调用ngx_http_process_request函数设置异步读写函数。

- 调用ngx_http_handler开始真正的请求处理

- 调用ngx_http_core_run_phasesa按照http阶段处理请求，这是http框架的核心，大部分http模块都在这里运行，最终产生响应内容。这个阶段非常重要，它组织所有http模块，调用模块的处理函数，执行虚拟主机的查找、重定向、权限检查、内容产生等操作，是http框架运行机制中最根本的部分。

- 调用ngx_http_send_header函数发送响应头，从函数指针ngx_http_top_header_filter开始，通过header filter模块链表过滤处理模块，最终发送处理过的响应头

- 调用ngx_http_output_filter发送响应体，从哈数指针ngx_http_top_body_filter开始，通过body filter模块链过滤处理，最终也发送处理过的响应体

### 请求处理阶段

- 请求阶段定义
```
//http请求处理的11个阶段
typedef enum {
    //读取http头后，开始读取内容，请求头读取完成之后的阶段
    NGX_HTTP_POST_READ_PHASE = 0,
    //server改写uri,server内请求地址重写阶段
    NGX_HTTP_SERVER_REWRITE_PHASE,
    //查找匹配的location,配置文件查找阶段
    NGX_HTTP_FIND_CONFIG_PHASE,
    //location改写uri,location内请求地址重写阶段
    NGX_HTTP_REWRITE_PHASE,
    //请求地址重写完成之后的阶段
    NGX_HTTP_POST_REWRITE_PHASE,
    //访问权限检查准备阶段
    NGX_HTTP_PREACCESS_PHASE,
    //检查访问权限，实现权限控制，访问权限检查阶段
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

// TCP和UDP处理的7个阶段
typedef enum {
   // post-accept阶段,接受客户端连接请求后的第一个阶段，模块ngx_stream_realip_module在这个阶段被调用
    NGX_STREAM_POST_ACCEPT_PHASE = 0,
    // pre-access阶段，访问处理阶段，模块ngx_stream_limit_conn_module在这个阶段被调用
    NGX_STREAM_PREACCESS_PHASE,
    // access阶段,访问处理阶段，模块ngx_stream_access_module在这个阶段被调用
    NGX_STREAM_ACCESS_PHASE,
    // ssl阶段，TLS/SSL处理阶段,模块ngx_stream_ssl_module在这个阶段被调用
    NGX_STREAM_SSL_PHASE,
    // preread阶段,数据预读阶段，将TCP/UDP会话数据的初始字节读入预读缓冲区，以允许ngx_stream_ssl_preread_stream之类模块在处理之前分析数据
    NGX_STREAM_PREREAD_PHASE,
    // content阶段,数据处理阶段，将tcp/udp会话数据代理到上游服务器或将模块ngx_stream_return_module指定的值返回客户端
    NGX_STREAM_CONTENT_PHASE,
    // log阶段，记录客户端会话处理结果的最后阶段，模块ngx_stream_log_module模块在这个阶段被调用
    NGX_STREAM_LOG_PHASE
} ngx_stream_phases;
```
- 阶段说明
  - ngx_http_phasesd定义了http框架处理的11个阶段，11个阶段中有3个阶段是有http框架使用，用户不能注册开发模块在这些阶段处理请求，即使注册了也会被框架忽略，这3个阶段分别是NGX_HTTP_FIND_CONFIG_PHASE、NGX_HTTP_POST_REWRITE_PHASE、NGX_HTTP_POST_ACCESS_PHASE，所以我们介入开发的只有8个模块。
  
  ### 处理引擎
  - nginx的模块ngx_http_core_module管理所有的handler模块，它定义请求的处理阶段，把所有的handler模块组织程一条流水线，通过一个引擎来驱动模块处理http请求