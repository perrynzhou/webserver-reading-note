/*************************************************************************
  > File Name: ngx_http_hello_world.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 三 10/28 21:24:01 2020
 ************************************************************************/

#include <stdio.h>
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

static u_char hello_world_message[] = "/*********** perrynzhou ********/\nhello world,this is first nginx module\n/*********** end ********/\n";
//"heelo_world这个指令的值定义"
struct ngx_http_world_loc_conf_s
{
  ngx_flag_t enable;
};
typedef struct ngx_http_world_loc_conf_s ngx_http_world_loc_conf_t;
static ngx_int_t ngx_http_hello_world_init(ngx_conf_t *cf);
static void *ngx_http_hello_world_create_loc_conf(ngx_conf_t *cf);
static ngx_int_t ngx_http_hello_world_handler(ngx_http_request_t *r);
static ngx_command_t ngx_http_hello_world_commands[] = {
    {//制定配置指令名称
     ngx_string("hello_world"),
     //配置指令的作用域
     NGX_HTTP_LOC_CONF | NGX_CONF_FLAG,
     //解析配置指令的函数指针
     ngx_conf_set_flag_slot,
     //配置指令具体数据的位置
     NGX_HTTP_LOC_CONF_OFFSET,
     //具体配置指令的值
     offsetof(ngx_http_world_loc_conf_t, enable),
     NULL},
    ngx_null_command,
};
/** 初始化注册函数 **/
static ngx_int_t ngx_http_hello_world_init(ngx_conf_t *cf)
{
  ngx_http_handler_pt *h;
  ngx_http_core_main_conf_t *conf;
  conf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);
  h = ngx_array_push(&conf->phases[NGX_HTTP_REWRITE_PHASE].handlers);
  *h = ngx_http_hello_world_handler;
  return NGX_OK;
}
/**模块继集成**/
static ngx_http_module_t ngx_http_hello_world_module_ctx = {
    //解析配置文件前被调用
    NULL,
    //解析配置文件后被调用
    ngx_http_hello_world_init,
    //创建http main域的配置结构
    NULL,
    //初始化http main域的配置结构
    NULL,
    //创建server域的配置结构
    NULL,
    //合并server域的配置结构
    NULL,
    //创建location域的配置结构
    ngx_http_hello_world_create_loc_conf,
    //合并location域的配置结构
    NULL,
};
/** 创建配置文件数据 **/
static void *ngx_http_hello_world_create_loc_conf(ngx_conf_t *cf)
{
  ngx_http_world_loc_conf_t *conf;
  conf = ngx_palloc(cf->pool, sizeof(ngx_http_world_loc_conf_t));
  if (conf == NULL)
  {
    return NULL;
  }
  conf->enable = NGX_CONF_UNSET;
  return conf;
}

/**配置指令**/
ngx_module_t ngx_http_hello_world_module = {
    //标准模块填充值
    NGX_MODULE_V1,
    //配置功能函数
    &ngx_http_hello_world_module_ctx,
    //配置指令数组
    ngx_http_hello_world_commands,
    //http模块必须的tag
    NGX_HTTP_MODULE,
    //init master
    NULL,
    //init module
    NULL,
    //init process
    NULL,
    //inbit thread
    NULL,
    //exit thread
    NULL,
    //exit process
    NULL,
    //exit master
    NULL,
    NGX_MODULE_V1_PADDING,
};
/** 定义处理函数 */
static ngx_int_t ngx_http_hello_world_handler(ngx_http_request_t *r)
{
  ngx_http_world_loc_conf_t *cf = ngx_http_get_module_loc_conf(r, ngx_http_hello_world_module);
  if (cf->enable)
  {
    ngx_buf_t *b;
    ngx_chain_t out;

    /* 设置Content-Type header. */
    r->headers_out.content_type.len = sizeof("text/plain") - 1;
    r->headers_out.content_type.data = (u_char *)"text/plain";

    /* 申请发送所需要内存 */
    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));

    out.buf = b;
    out.next = NULL; /* just one buffer */

    b->pos = hello_world_message;                                    /* first position in memory of the data */
    b->last = hello_world_message + sizeof(hello_world_message) - 1; /* last position in memory of the data */
    b->memory = 1;                                                   /* content is in read-only memory */
    b->last_buf = 1;                                                 /* there will be no more buffers in the request */

    /* Sending the headers for the reply. */
    r->headers_out.status = NGX_HTTP_OK; /* 200 status code */
    /* Get the content length of the body. */
    r->headers_out.content_length_n = sizeof(hello_world_message) - 1;
    ngx_http_send_header(r); /* Send the headers */

    /* Send the body, and return the status code of the output filter chain. */
    return ngx_http_output_filter(r, &out);
  }
  return 0;
}
