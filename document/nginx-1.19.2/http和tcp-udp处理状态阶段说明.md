## http和tcp-udp处理状态阶段说明

| author | update |
| ------ | ------ |
| perrynzhou@gmail.com | 2020/10/1 |

###  http请求处理状态机的11个阶段
- 状态机状态定义
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
- HTTP请求处理阶段

	- NGX_HTTP_POST_READ_PHASE阶段,当nginx成功接收到一个客户端请求后(accept()正确返回fd建立连接)，针对该请求所做的第一个动作就是读取客户端请求发过来的请求头内容。如果这个阶段设置有对应的回调函数，那么在读取并解析客户端请求头内容后会执行这些回调函数
	- NGX_HTTP_SERVER_REWRITE_PHASE阶段,这个和NGX_HTTP_POST_REWRITE_PHASE都属于地址重写,也都是针对rewrite模块而设计。NGX_HTTP_SERVER_REWRITE_PHASE用于server上下文里面的地址重写。NGX_HTTP_POST_REWRITE_PHASE用于location上下文里面的地址重写。nginx中的rewrite模块的相关指令既可以用在server上下文，也可以用在location上下文。在客户端请求被nginx接受后，首先做server的查找和定位,在定位server(如果未发现server就使用默认的server)后执行NGX_HTTP_SERVER_REWRITE_PHASE阶段上的回调函数，然后进入下一个阶段NGX_HTTP_FIND_CONFIG_PHASE。
	- NGX_HTTP_FIND_CONFIG_PHASE阶段,这个阶段不设置任何的回调函数,它们永远不会执行。该阶段完成的是nginx的特定任务，即location定位。只有把当前请求的对应location找到了，才能从该location上下文中取出更多的配置信息，在进行下一步处理NGX_HTTP_REWRITE_PHAS阶段。
	- NGX_HTTP_REWRITE_PHAS阶段，经过上一个阶段的处理在location里面按照地址重新规则进行地址重写。
	- NGX_HTTP_POST_REWRITE_PHASE阶段，这个阶段也是nginx的特定的任务，主要是检查当前请求是否做过过多的内部跳转，防止一个请求的处理在nginx内部跳转很多次甚至是死循环，因为每一次跳转都基本把所有的流程都走一遍，这个是非常消耗性能
	- NGX_HTTP_PREACCESS_PHASE、NGX_HTTP_ACCESS_PHASE、NGX_HTTP_POST_ACCESS_PHASE阶段，这三个阶段分别做权限检查的前期、中期、后期工作;其中后期是固定的，就是检查前面访问权限的结果，如果当前请求没有访问权限，那么直接返回状态403
	- NGX_HTTP_LOG_PHASE阶段,是专门对日志模块所设定的处理阶段

### TCP/UDP处理阶段的7个阶段(自上而下在每个阶段处理)

- post-accept阶段,接受客户端连接请求后的第一个阶段，模块ngx_stream_realip_module在这个阶段被调用
- pre-access阶段，访问处理阶段，模块ngx_stream_limit_conn_module在这个阶段被调用
- access阶段,访问处理阶段，模块ngx_stream_access_module在这个阶段被调用
- ssl阶段，TLS/SSL处理阶段,模块ngx_stream_ssl_module在这个阶段被调用
- preread阶段,数据预读阶段，将TCP/UDP会话数据的初始字节读入预读缓冲区，以允许ngx_stream_ssl_preread_stream之类模块在处理之前分析数据
- content阶段,数据处理阶段，将tcp/udp会话数据代理到上游服务器或将模块ngx_stream_return_module指定的值返回客户端
- log阶段，记录客户端会话处理结果的最后阶段，模块ngx_stream_log_module模块在这个阶段被调用