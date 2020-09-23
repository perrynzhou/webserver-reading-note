## ngx_http_static_module原理和分析


### 1.nginx handlers模块结构体说明

```
typedef struct ngx_module_s          ngx_module_t;
struct ngx_module_s {
    //当前模块在同类模块中的编号
    ngx_uint_t            ctx_index;
    //单个模块在全局模块中的序号
    ngx_uint_t            index;
    //模块名称
    char                 *name;

    ngx_uint_t            spare0;
    ngx_uint_t            spare1;

    ngx_uint_t            version;
    const char           *signature;
    //当前模块的私有数据，可以表示任何的数据
    void                 *ctx;
    //当前模块配置项解析数组
    ngx_command_t        *commands;
    //模块类型
    ngx_uint_t            type;
    //初始化master进程时候被调用
    ngx_int_t           (*init_master)(ngx_log_t *log);
    //初始化module时候被调用
    ngx_int_t           (*init_module)(ngx_cycle_t *cycle);
    //初始化worker进程时候被调用
    ngx_int_t           (*init_process)(ngx_cycle_t *cycle);
    //初始化线程时候被调用
    ngx_int_t           (*init_thread)(ngx_cycle_t *cycle);
    //线程退出时候被调用，
    void                (*exit_thread)(ngx_cycle_t *cycle);
    //worker进程退出时候被调用主要体现在ngx_worker_process_exit这个函数上
    void                (*exit_process)(ngx_cycle_t *cycle);
	//master进程退出时候被调用，主要体现在ngx_master_process_exit函数里
    void                (*exit_master)(ngx_cycle_t *cycle);
};
//设置http模块的配置文件解释时候回调函数
static ngx_http_module_t  ngx_http_core_module_ctx = {
    //解析http块配置时候调用
    ngx_http_core_preconfiguration,        /* preconfiguration */
    ngx_http_core_postconfiguration,       /* postconfiguration */

    ngx_http_core_create_main_conf,        /* create main configuration */
    ngx_http_core_init_main_conf,          /* init main configuration */

    ngx_http_core_create_srv_conf,         /* create server configuration */
    ngx_http_core_merge_srv_conf,          /* merge server configuration */

    ngx_http_core_create_loc_conf,         /* create location configuration */
    ngx_http_core_merge_loc_conf           /* merge location configuration */
};

ngx_module_t  ngx_http_core_module = {
    NGX_MODULE_V1,
    &ngx_http_core_module_ctx,             /* module context */
    ngx_http_core_commands,                /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};
```
### 2. nginx handler模块
- 对于客户端http请求，为了获取更强的控制力，nginx将整个过程分为多个阶段，每一个阶段由0个或者多个回调函数处理。ngx_http_static_module就是将自己的模块功能函数ngx_http_static_init挂载到NGX_HTTP_CONTENT_PHASE阶段
```
static ngx_http_module_t  ngx_http_static_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_static_init,                  /* postconfiguration */
	/******忽略*******/
};


ngx_module_t  ngx_http_static_module = {
    NGX_MODULE_V1,
    &ngx_http_static_module_ctx,           /* module context */
	/******忽略*******/
};
static ngx_int_t ngx_http_static_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;
    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);
     h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
    *h = ngx_http_static_handler;

    return NGX_OK;
}

```
###  3. 请求处理状态机的11个阶段
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
- 状态机状态解释

	- NGX_HTTP_POST_READ_PHASE阶段,当nginx成功接收到一个客户端请求后(accept()正确返回fd建立连接)，针对该请求所做的第一个动作就是读取客户端请求发过来的请求头内容。如果这个阶段设置有对应的回调函数，那么在读取并解析客户端请求头内容后会执行这些回调函数
	- NGX_HTTP_SERVER_REWRITE_PHASE阶段,这个和NGX_HTTP_POST_REWRITE_PHASE都属于地址重写,也都是针对rewrite模块而设计。NGX_HTTP_SERVER_REWRITE_PHASE用于server上下文里面的地址重写。NGX_HTTP_POST_REWRITE_PHASE用于location上下文里面的地址重写。nginx中的rewrite模块的相关指令既可以用在server上下文，也可以用在location上下文。在客户端请求被nginx接受后，首先做server的查找和定位,在定位server(如果未发现server就使用默认的server)后执行NGX_HTTP_SERVER_REWRITE_PHASE阶段上的回调函数，然后进入下一个阶段NGX_HTTP_FIND_CONFIG_PHASE。
	- NGX_HTTP_FIND_CONFIG_PHASE阶段,这个阶段不设置任何的回调函数,它们永远不会执行。该阶段完成的是nginx的特定任务，即location定位。只有把当前请求的对应location找到了，才能从该location上下文中取出更多的配置信息，在进行下一步处理NGX_HTTP_REWRITE_PHAS阶段。
	- NGX_HTTP_REWRITE_PHAS阶段，经过上一个阶段的处理在location里面按照地址重新规则进行地址重写。
	- NGX_HTTP_POST_REWRITE_PHASE阶段，这个阶段也是nginx的特定的任务，主要是检查当前请求是否做过过多的内部跳转，防止一个请求的处理在nginx内部跳转很多次甚至是死循环，因为每一次跳转都基本把所有的流程都走一遍，这个是非常消耗性能
	- NGX_HTTP_PREACCESS_PHASE、NGX_HTTP_ACCESS_PHASE、NGX_HTTP_POST_ACCESS_PHASE阶段，这三个阶段分别做权限检查的前期、中期、后期工作;其中后期是固定的，就是检查前面访问权限的结果，如果当前请求没有访问权限，那么直接返回状态403
	- NGX_HTTP_LOG_PHASE阶段,是专门对日志模块所设定的处理阶段