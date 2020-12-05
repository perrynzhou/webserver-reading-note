## nginx模块概述


| 作者 | 时间 |QQ技术交流群 |
| ------ | ------ |------ |
| perrynzhou@gmail.com |2020/05/23 |中国开源存储技术交流群(672152841) |



### 基本介绍

- nginx所有源代码是以模块组织，其模块包括核心模块和功能模块。新增或者删除某个模块时候都需要重新编译nginx,重新生成二进制的文件

- nginx模块按照功能划分可以分为如下4大类

  ![nginx-module-overview](../images/nginx-module-overview.jpg)

  - handlers模块:协同完成客户端请求处理、产生响应数据、比如ngx_http_rewrite_module模块，用于处理客户端请求的地址重写；ngx_http_static_module模块，负责处理客户端的静态页面请求；ngx_http_log_module模块，负责记录用户日志。
  - filters模块:对handlers产生的响应数据做各种过滤，比如模块ngx_http_not_modified_filter_module对响应数据进行过滤检测，如果通过时间戳判断出前后两次请求的响应数据没有发生任何实质性的改变，那么直接响应 "304 Not Modified"的状态标识，让客户端使用本地缓存即可，而原本待发送的响应数据被清除
  - upstream模块:如果存在后端的这是服务器，nginx利用upstream模块充当反向代理的角色，对客户端发起的请求只负责进行转发(包括后端服务器的响应数据的回传)，比如ngx_http_proxy_module就是标准的upstream模块
  - load-balance模块:在nginx充当中间件代码角色时候，由于后端服务器一般是多台，对于一次请求希望达到每个后端服务器平均处理，这样load-balance就能起到很好的作用。ngx_http_upstream_ip_hash_module这样的load-balance模块实现不同的负载均衡算法

### nginx 模块结构体说明
```
typedef struct ngx_module_s          ngx_module_t;
struct ngx_module_s {
	//当前模块在同类模块中的序号
    ngx_uint_t            ctx_index;
    //单前模块在所有模块中的序号
    ngx_uint_t            index;
	//模块名称
    char                 *name;
	//模块版本
    ngx_uint_t            version;
    const char           *signature;
	//当前模块的私有数据，这里是void *，可以表示任何数据
    void                 *ctx;
    //当前模块配置项解析数组
    ngx_command_t        *commands;
    //模块类型
    ngx_uint_t            type;
    //初始化master进程时候调用,预留目前未被使用
    ngx_int_t           (*init_master)(ngx_log_t *log);
	//初始化module时候调用
    ngx_int_t           (*init_module)(ngx_cycle_t *cycle);
	//会在ngx_worker_process_init 调用
    ngx_int_t           (*init_process)(ngx_cycle_t *cycle);
    //初始化线程时候调用
    ngx_int_t           (*init_thread)(ngx_cycle_t *cycle);
    //线程退出时候调用
    void                (*exit_thread)(ngx_cycle_t *cycle);
    //worker进程退出时候调用，主要体现在ngx_worker_process_exit这个函数上
    void                (*exit_process)(ngx_cycle_t *cycle);
	//master进程退出时候调用，体现在ngx_master_process_exit函数里
    void                (*exit_master)(ngx_cycle_t *cycle);
};
```
### nginx 1.19.2模块名称和对应处理模块

- 生成nginx模块信息
```
$ nginx-1.19.2 && CFLAGS="-g -O0" ./configure
//在nginx-1.19.2下生成objs，这个目录由一个 ngx_modules.c,这个文件包括了nginx的所有模块名称和处理模块
$ ls
autoconf.err  Makefile  nginx  nginx.8  ngx_auto_config.h  ngx_auto_headers.h  ngx_modules.c  ngx_modules.o  src
[perrynzhou@linuxzhou ~/Debug/nginx-1.19.2/objs]$ 
```
- 模块信息,ngx_modules[]中模块的次序决定了该模块在nginx执行的顺序。当一个请求同时符合多个模块的处理规则时，nginx将按照ngx_modules[]中的顺序选择最靠前的模块优先处理。对于http过滤模块执行顺序正好相反，在http框架初始化时，在 ngx_modules[]中将过滤模块按照先后顺序向过滤链表中添加，但是每次都是添加到链表的头部，因此ngx_modules[]中越是靠后的过滤模块却会被首先处理。
```
//每一个模块处理的类型都是ngx_module_t
ngx_module_t *ngx_modules[] = {
    &ngx_core_module,
    &ngx_errlog_module,
    &ngx_conf_module,
    &ngx_regex_module,
    &ngx_events_module,
    &ngx_event_core_module,
    &ngx_epoll_module,
    &ngx_http_module,
    &ngx_http_core_module,
    &ngx_http_log_module,
    &ngx_http_upstream_module,
    &ngx_http_static_module,
    &ngx_http_autoindex_module,
    &ngx_http_index_module,
    &ngx_http_mirror_module,
    &ngx_http_try_files_module,
    &ngx_http_auth_basic_module,
    &ngx_http_access_module,
    &ngx_http_limit_conn_module,
    &ngx_http_limit_req_module,
    &ngx_http_geo_module,
    &ngx_http_map_module,
    &ngx_http_split_clients_module,
    &ngx_http_referer_module,
    &ngx_http_rewrite_module,
    &ngx_http_proxy_module,
    &ngx_http_fastcgi_module,
    &ngx_http_uwsgi_module,
    &ngx_http_scgi_module,
    &ngx_http_memcached_module,
    &ngx_http_empty_gif_module,
    &ngx_http_browser_module,
    &ngx_http_upstream_hash_module,
    &ngx_http_upstream_ip_hash_module,
    &ngx_http_upstream_least_conn_module,
    &ngx_http_upstream_random_module,
    &ngx_http_upstream_keepalive_module,
    &ngx_http_upstream_zone_module,
    &ngx_http_write_filter_module,
    &ngx_http_header_filter_module,
    &ngx_http_chunked_filter_module,
    &ngx_http_range_header_filter_module,
    &ngx_http_gzip_filter_module,
    &ngx_http_postpone_filter_module,
    &ngx_http_ssi_filter_module,
    &ngx_http_charset_filter_module,
    &ngx_http_userid_filter_module,
    &ngx_http_headers_filter_module,
    &ngx_http_copy_filter_module,
    &ngx_http_range_body_filter_module,
    &ngx_http_not_modified_filter_module,
    NULL
};

char *ngx_module_names[] = {
    "ngx_core_module",
    "ngx_errlog_module",
    "ngx_conf_module",
    "ngx_regex_module",
    "ngx_events_module",
    "ngx_event_core_module",
    "ngx_epoll_module",
    "ngx_http_module",
    "ngx_http_core_module",
    "ngx_http_log_module",
    "ngx_http_upstream_module",
    "ngx_http_static_module",
    "ngx_http_autoindex_module",
    "ngx_http_index_module",
    "ngx_http_mirror_module",
    "ngx_http_try_files_module",
    "ngx_http_auth_basic_module",
    "ngx_http_access_module",
    "ngx_http_limit_conn_module",
    "ngx_http_limit_req_module",
    "ngx_http_geo_module",
    "ngx_http_map_module",
    "ngx_http_split_clients_module",
    "ngx_http_referer_module",
    "ngx_http_rewrite_module",
    "ngx_http_proxy_module",
    "ngx_http_fastcgi_module",
    "ngx_http_uwsgi_module",
    "ngx_http_scgi_module",
    "ngx_http_memcached_module",
    "ngx_http_empty_gif_module",
    "ngx_http_browser_module",
    "ngx_http_upstream_hash_module",
    "ngx_http_upstream_ip_hash_module",
    "ngx_http_upstream_least_conn_module",
    "ngx_http_upstream_random_module",
    "ngx_http_upstream_keepalive_module",
    "ngx_http_upstream_zone_module",
    "ngx_http_write_filter_module",
    "ngx_http_header_filter_module",
    "ngx_http_chunked_filter_module",
    "ngx_http_range_header_filter_module",
    "ngx_http_gzip_filter_module",
    "ngx_http_postpone_filter_module",
    "ngx_http_ssi_filter_module",
    "ngx_http_charset_filter_module",
    "ngx_http_userid_filter_module",
    "ngx_http_headers_filter_module",
    "ngx_http_copy_filter_module",
    "ngx_http_range_body_filter_module",
    "ngx_http_not_modified_filter_module",
    NULL
};

```