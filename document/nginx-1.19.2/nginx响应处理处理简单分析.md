## nginx响应处理处理简单分析

| author | update |
| ------ | ------ |
| perrynzhou@gmail.com | 2020/11/01 |

### 请求静态资源响应

- nginx 处理http请求流程:

  - 监听端口，设置回调函数ngx_http_init_connection()

  - 接受客户端连接，调用nginx_http_wait_request_handler()函数

  - 调用ngx_http_create_request创建请求对象

  - 接受数据，调用ngx_http_process_request_line解析请求行

  - 请求行接受完毕，调用ngx_http_process_request_headers()解析请求头

  - 请求头接受完毕后，调用ngx_http_process_request函数设置异步读写函数。

  - 调用ngx_http_handler开始真正的请求处理

  - 调用ngx_http_core_run_phasesa按照http阶段处理请求，这是http框架的核心，大部分http模块都在这里运行，最终产生响应内容

  - 调用ngx_http_send_header函数发送响应头，从函数指针ngx_http_top_header_filter开始，通过header filter模块链表过滤处理模块，最终发送处理过的响应头

  - 调用ngx_http_output_filter发送响应体，从哈数指针ngx_http_top_body_filter开始，通过body filter模块链过滤处理，最终也发送处理过的响应体

- nginx配置中禁用缓存
```
    location / {
            add_header Last-Modified $date_gmt;
            add_header Cache-Control 'no-store, no-cache, must-revalidate, proxy-revalidate, max-age=0';
            if_modified_since off;
            expires off;
            etag off;
            root   html;
            index  index.html index.htm;
        }

```
- nginx静态资源处理模块和函数说明如下：
```
//nginx静态资源处理的模块，比如请求nginx的默认index页面
ngx_module_t  ngx_http_static_module = {
    NGX_MODULE_V1,
    &ngx_http_static_module_ctx,           /* module context */
    NULL,                                  /* module directives */
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
//nginx静态模块的上下文信息，这里主要是配置静态模块在每个配置阶段的设置回调函数
static ngx_http_module_t  ngx_http_static_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_static_init,                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    NULL,                                  /* create location configuration */
    NULL                                   /* merge location configuration */
};

//nginx静态资源核心处理函数
static ngx_int_t ngx_http_static_handler(ngx_http_request_t *r){
	  /****** handle others ****/
	  //http响应头的header过滤
	  rc = ngx_http_send_header(r)
	  //http响应体中内容过滤
	  return ngx_http_output_filter(r, &out);
	  
}

//nginx静态资源处理的初始化函数，这里主要是设置ngx_http_static_handler
static ngx_int_t ngx_http_static_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_static_handler;

    return NGX_OK;
}

```

- ngx_http_send_header 函数分析
  - ngx_http_send_header根据nginx的内置或者配置来过滤请求响应的请求头,执行链路的总体分析
  
    ```
   	ngx_http_send_header:
   		ngx_http_top_header_filter(ngx_http_not_modified_header_filter)
   		/*****多个filter函数*****/
   		ngx_http_next_header_filter
   		ngx_http_header_filter
   	 		ngx_http_write_filter

    ```

  - 每个链路上的具体函数指针说明

    ```
    ngx_http_send_header(ngx_http_top_header_filter=ngx_http_not_modified_header_filter)
    ngx_http_not_modified_header_filter(ngx_http_next_header_filter=ngx_http_headers_filter)
    ngx_http_headers_filter(ngx_http_next_header_filter=ngx_http_userid_filter)
    ngx_http_userid_filter(ngx_http_next_header_filter=ngx_http_charset_header_filter)
    ngx_http_charset_header_filter(ngx_http_next_header_filter=ngx_http_ssi_header_filter)
    ngx_http_ssi_header_filter(ngx_http_next_header_filter=ngx_http_gzip_header_filter)
    ngx_http_gzip_header_filter(ngx_http_next_header_filter=ngx_http_range_header_filter)
    ngx_http_range_header_filter(ngx_http_next_header_filter=ngx_http_chunked_header_filter)
    	
    ngx_http_range_header_filter(ngx_http_next_header_filter=ngx_http_header_filter)
    ngx_http_header_filter() {
        out.buf = b;
        out.next = NULL;
        return ngx_http_write_filter()
    }
    ngx_http_write_filter()
    
    ```


- ngx_http_output_filter 函数分析

  - ngx_http_output_filter用于过滤请求体，第一次请求ngx_http_top_body_filter指针的函数，中间经过多次ngx_http_next_body_filter，最后达到ngx_http_write_filter。

    ```

    ngx_http_output_filter:
    	ngx_http_top_body_filter
    	//一个或者多个body filter函数
    	ngx_http_next_body_filter
    	.....................
    	ngx_http_write_filter
    
    ```

  - body的filter函数最终执行到ngx_http_write_filter函数，调用sendfile传输文件，至此整个静态资源的请求完成