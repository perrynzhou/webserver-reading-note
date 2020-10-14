/*************************************************************************
  > File Name: ngx_http_test_module.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 三 10/14 06:09:34 2020
 ************************************************************************/

#include<stdio.h>

/** 配置文件相关定义和解析 */
typedef struct {
  ngx_flag_t enable;
}ngx_http_test_loc_conf_t;

static ngx_command_t ngx_http_test_cmds[] = {
  {
    //制定配置指令名称
    ngx_string("test"),
    //配置指令的作用域
    NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
    //解析配置指令的函数指针
    ngx_conf_set_flag_slot,
    //配置指令具体数据的位置
    NGX_HTTP_LOC_CONF_OFFSET,
    //具体配置指令的值
    offsetof(ngx_http_test_conf_t,enable),
    NULL
  },
  ngx_null_command,
};

/** 创建配置文件数据 **/
static void *ng_http_test_create_loc_conf(ngx_conf *cf)
{
  ngx_http_test_loc_conf *conf;
  conf = ngx_paloc(cf->pool,sizeoof(ngx_http_test_loc_conf));
  if(conf==NULL)
  {
    return NULL;
  }
  conf->enable = NGX_CONF_UNSET;
  return conf;
}

/** 定义处理函数 */
static ngx_int_t ngx_http_test_handler(ngx_http_request *r)
{
  ngx_http_test_loc_conf *cf;
  cf = ngx_http_get_module_loc_conf(r,ngx_http_test_moule);
  if(cf->enable) {
    printf("hello test moddule disable\n");
    ngx_log_error(NGX_LOG_ERR,r->connection->log,0,"aa");
  }else{
       printf("hello test moddule\n");

  }
}

/** 初始化注册函数 **/
static ngx_int_t ngx_http_test_init(ngx_conf *cf)
{
  ngx_http_handler_pt *h;
  ngx_http_core_main_conf_t *conf;
  conf = ngx_http_conf_get_module_main_conf(cf,ngx_http_core_module);
  h = ngx_array_push(&conf->phases[NGX_HTTP_REWRITE_PHASE],handlers);
  *h= ngx_http_test_handler;
  return NGX_OK;
}

/**模块继集成**/
static ngx_http_test_module_ctx = {
  //解析配置文件前被调用
  NULL,
  //解析配置文件后被调用
  ngx_http_test_init,
  //创建http main域的配置结构
  NULL,
  //初始化http main域的配置结构
  NULL,
  //创建server域的配置结构
  NULL,
  //合并server域的配置结构
  NULL,
  //创建location域的配置结构
  ngx_http_test_create_loc_conf,
  //合并location域的配置结构
  NULL,
};

/**配置指令**/
ngx_module_t ngx_http_test_module = {
  //标准模块填充值
  NGX_MODULE_V1,
  //配置功能函数
  &ngx_http_test_module_ctx,
  //配置指令数组
  ngx_http_test_cmds,
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